// Package docx extracts the text structure of a WordprocessingML (.docx)
// document: paragraphs, headings, list items and tables.
//
// Format reference: ECMA-376 Part 1 (WordprocessingML) and Part 2 (Open
// Packaging Conventions).
package docx

import (
	"archive/zip"
	"encoding/xml"
	"errors"
	"fmt"
	"io"
	"os"
	"path"
	"strings"
)

// Document is the text content of a .docx file in reading order.
type Document struct {
	Blocks []Block
}

// Block is a *Paragraph or a *Table.
type Block interface{ block() }

// Paragraph is a paragraph's text with its structural role resolved from
// styles and numbering definitions.
type Paragraph struct {
	Text    string // run text; line breaks are "\n"
	Heading int    // 0 for body text, 1..9 for a heading level
	List    *List  // nil unless the paragraph is a list item
}

// List describes a list item.
type List struct {
	Level   int // nesting level, 0-based
	Ordered bool
}

// Table is a grid of cells. Merged cells are expanded into empty cells, so
// every row lists one cell per grid column it covers.
type Table struct {
	Rows [][]Cell
}

// Cell holds the paragraphs and nested tables of a table cell.
type Cell struct {
	Blocks []Block
}

func (*Paragraph) block() {}
func (*Table) block()     {}

// Open parses the .docx file at name.
func Open(name string) (*Document, error) {
	f, err := os.Open(name)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	st, err := f.Stat()
	if err != nil {
		return nil, err
	}
	return Parse(f, st.Size())
}

// Parse parses a .docx package of the given size read from r.
func Parse(r io.ReaderAt, size int64) (*Document, error) {
	z, err := zip.NewReader(r, size)
	if err != nil {
		return nil, fmt.Errorf("docx: %w", err)
	}
	files := make(map[string]*zip.File, len(z.File))
	for _, f := range z.File {
		files[f.Name] = f
	}

	rootRels, err := relationships(files, "")
	if err != nil {
		return nil, err
	}
	main := files[rootRels["officeDocument"]]
	if main == nil {
		return nil, errors.New("docx: no main document part")
	}
	rels, err := relationships(files, main.Name)
	if err != nil {
		return nil, err
	}
	st, err := parseStyles(files[rels["styles"]])
	if err != nil {
		return nil, err
	}
	num, err := parseNumbering(files[rels["numbering"]])
	if err != nil {
		return nil, err
	}

	rc, err := main.Open()
	if err != nil {
		return nil, fmt.Errorf("docx: %s: %w", main.Name, err)
	}
	defer rc.Close()
	p := &parser{d: xml.NewDecoder(rc), styles: st, numbering: num}
	doc, err := p.document()
	if err != nil {
		return nil, fmt.Errorf("docx: %s: %w", main.Name, err)
	}
	return doc, nil
}

// relationships returns the internal targets of part's relationships as
// package paths, keyed by the last segment of the relationship type URI
// (e.g. "styles"). The empty part name selects the package root.
func relationships(files map[string]*zip.File, part string) (map[string]string, error) {
	dir, name := path.Split(part)
	var rels struct {
		Rel []struct {
			Type       string `xml:"Type,attr"`
			Target     string `xml:"Target,attr"`
			TargetMode string `xml:"TargetMode,attr"`
		} `xml:"Relationship"`
	}
	if err := decodePart(files[dir+"_rels/"+name+".rels"], &rels); err != nil {
		return nil, err
	}
	m := make(map[string]string)
	for _, r := range rels.Rel {
		if r.TargetMode == "External" {
			continue
		}
		target := path.Join(dir, r.Target)
		if strings.HasPrefix(r.Target, "/") {
			target = r.Target[1:]
		}
		m[path.Base(r.Type)] = target
	}
	return m, nil
}

// decodePart unmarshals the XML part f into v. A missing part leaves v unchanged.
func decodePart(f *zip.File, v any) error {
	if f == nil {
		return nil
	}
	rc, err := f.Open()
	if err != nil {
		return fmt.Errorf("docx: %s: %w", f.Name, err)
	}
	defer rc.Close()
	if err := xml.NewDecoder(rc).Decode(v); err != nil {
		return fmt.Errorf("docx: %s: %w", f.Name, err)
	}
	return nil
}

// val is the common shape of WordprocessingML property elements: <w:x w:val="..."/>.
type val struct {
	Val string `xml:"val,attr"`
}

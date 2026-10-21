package docx

import (
	"encoding/xml"
	"errors"
	"io"
	"strconv"
	"strings"
)

// transparent elements wrap content without changing its meaning, so their
// children are treated as children of the enclosing element. Any other
// element that isn't explicitly handled is skipped with all its content:
// deletions, field instructions, text boxes, properties, etc.
var transparent = map[string]bool{
	"r":          true,
	"hyperlink":  true,
	"sdt":        true,
	"sdtContent": true,
	"customXml":  true,
	"smartTag":   true,
	"fldSimple":  true,
	"ins":        true,
	"moveTo":     true,
	"dir":        true,
	"bdo":        true,
}

// parser streams document.xml. Elements are matched by local name only, so
// both Transitional and Strict namespaces are accepted.
type parser struct {
	d         *xml.Decoder
	styles    styles
	numbering numbering
}

func (p *parser) document() (*Document, error) {
	for {
		tok, err := p.d.Token()
		if err == io.EOF {
			return nil, errors.New("no document body")
		}
		if err != nil {
			return nil, err
		}
		if e, ok := tok.(xml.StartElement); ok && e.Name.Local == "body" {
			doc := &Document{}
			err := p.children(func(e xml.StartElement) error { return p.block(e, &doc.Blocks) })
			return doc, err
		}
	}
}

// children calls f for each child element of the element being read, looking
// through transparent wrappers, and returns after its end tag. f must consume
// the element it is given, e.g. with p.d.Skip.
func (p *parser) children(f func(xml.StartElement) error) error {
	depth := 0
	for {
		tok, err := p.d.Token()
		if err != nil {
			return err
		}
		switch t := tok.(type) {
		case xml.StartElement:
			if transparent[t.Name.Local] {
				depth++
			} else if err := f(t); err != nil {
				return err
			}
		case xml.EndElement:
			if depth == 0 {
				return nil
			}
			depth--
		}
	}
}

// block appends the paragraph or table e to dst and skips any other element.
func (p *parser) block(e xml.StartElement, dst *[]Block) error {
	switch e.Name.Local {
	case "p":
		para, err := p.paragraph()
		if err != nil {
			return err
		}
		*dst = append(*dst, para)
	case "tbl":
		t, err := p.table()
		if err != nil {
			return err
		}
		*dst = append(*dst, t)
	default:
		return p.d.Skip()
	}
	return nil
}

func (p *parser) paragraph() (*Paragraph, error) {
	var props pPr
	var text strings.Builder
	err := p.children(func(e xml.StartElement) error {
		switch e.Name.Local {
		case "pPr":
			return p.d.DecodeElement(&props, &e)
		case "t":
			var s string
			err := p.d.DecodeElement(&s, &e)
			text.WriteString(s)
			return err
		case "tab":
			text.WriteByte(' ')
		case "br", "cr":
			if typ := attr(e, "type"); typ != "page" && typ != "column" {
				text.WriteByte('\n')
			}
		case "noBreakHyphen":
			text.WriteByte('-')
		case "drawing":
			text.WriteRune(Picture)
		}
		return p.d.Skip()
	})
	if err != nil {
		return nil, err
	}

	para := &Paragraph{Text: text.String()}
	var styleID string
	if props.Style != nil {
		styleID = props.Style.Val
	}
	if props.OutlineLvl != nil {
		para.Heading = outlineHeading(props.OutlineLvl.Val)
	} else {
		para.Heading = p.styles.heading(styleID)
	}

	numID, ilvl := p.styles.numPr(styleID)
	if np := props.NumPr; np != nil {
		if np.NumID != nil {
			numID = np.NumID.Val
		}
		if np.ILvl != nil {
			ilvl = np.ILvl.Val
		}
	}
	if numID != "" && numID != "0" {
		if ilvl == "" {
			ilvl = "0"
		}
		level, _ := strconv.Atoi(ilvl)
		para.List = &List{Level: level, Ordered: p.numbering.ordered(numID, ilvl)}
	}
	return para, nil
}

func (p *parser) table() (*Table, error) {
	t := &Table{}
	err := p.children(func(e xml.StartElement) error {
		if e.Name.Local != "tr" {
			return p.d.Skip()
		}
		row, err := p.row()
		t.Rows = append(t.Rows, row)
		return err
	})
	return t, err
}

func (p *parser) row() ([]Cell, error) {
	var cells []Cell
	err := p.children(func(e xml.StartElement) error {
		if e.Name.Local != "tc" {
			return p.d.Skip()
		}
		c, span, err := p.cell()
		cells = append(cells, c)
		for range span - 1 {
			cells = append(cells, Cell{})
		}
		return err
	})
	return cells, err
}

// cell parses a table cell and returns it with the number of grid columns it
// spans. A cell continuing a vertical merge is returned empty.
func (p *parser) cell() (c Cell, span int, err error) {
	span = 1
	merged := false
	err = p.children(func(e xml.StartElement) error {
		if e.Name.Local != "tcPr" {
			return p.block(e, &c.Blocks)
		}
		var pr struct {
			GridSpan *val `xml:"gridSpan"`
			VMerge   *val `xml:"vMerge"`
		}
		if err := p.d.DecodeElement(&pr, &e); err != nil {
			return err
		}
		if pr.GridSpan != nil {
			// Word tables have at most 63 columns; the cap keeps hostile files cheap.
			if n, err := strconv.Atoi(pr.GridSpan.Val); err == nil && n > 1 && n <= 64 {
				span = n
			}
		}
		merged = pr.VMerge != nil && pr.VMerge.Val != "restart"
		return nil
	})
	if merged {
		c = Cell{}
	}
	return c, span, err
}

func attr(e xml.StartElement, local string) string {
	for _, a := range e.Attr {
		if a.Name.Local == local {
			return a.Value
		}
	}
	return ""
}

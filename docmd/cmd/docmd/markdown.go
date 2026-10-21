package main

import (
	"io"
	"strings"
	"unicode/utf8"

	"docmd/docx"
)

type options struct {
	Indent      int  // spaces per list nesting level
	AlignTables bool // pad table cells so columns line up in the source
}

func writeMarkdown(w io.Writer, d *docx.Document, opt options) error {
	m := &mdWriter{opt: opt}
	for _, b := range d.Blocks {
		switch b := b.(type) {
		case *docx.Paragraph:
			m.paragraph(b)
		case *docx.Table:
			m.table(b)
		}
	}
	if m.buf.Len() > 0 {
		m.buf.WriteByte('\n')
	}
	_, err := io.WriteString(w, m.buf.String())
	return err
}

type mdWriter struct {
	opt     options
	buf     strings.Builder
	inList  bool   // the last block written was a list item
	parents []bool // orderedness of the current list item's ancestors, by level
}

// start separates a new block from the previous one. Consecutive list items
// are separated by a single newline so they form one list.
func (m *mdWriter) start(listItem bool) {
	if m.buf.Len() > 0 {
		if listItem && m.inList {
			m.buf.WriteString("\n")
		} else {
			m.buf.WriteString("\n\n")
		}
	}
	if !listItem || !m.inList {
		m.parents = m.parents[:0]
	}
	m.inList = listItem
}

func (m *mdWriter) paragraph(p *docx.Paragraph) {
	lines := textLines(p.Text)
	if len(lines) == 0 {
		return
	}
	switch {
	case p.Heading > 0:
		m.start(false)
		m.buf.WriteString(strings.Repeat("#", min(p.Heading, 6)) + " ")
		m.buf.WriteString(strings.Join(lines, " "))
	case p.List != nil:
		m.start(true)
		indent := m.listIndent(p.List)
		marker := "- "
		if p.List.Ordered {
			marker = "1. "
		}
		m.buf.WriteString(strings.Repeat(" ", indent) + marker)
		m.buf.WriteString(strings.Join(lines, "\\\n"+strings.Repeat(" ", indent+len(marker))))
	default:
		m.start(false)
		m.buf.WriteString(strings.Join(lines, "\\\n"))
	}
}

// listIndent returns the indentation of a list item and records it as the
// parent of deeper items.
func (m *mdWriter) listIndent(l *docx.List) int {
	if len(m.parents) > l.Level {
		m.parents = m.parents[:l.Level]
	}
	for len(m.parents) < l.Level {
		m.parents = append(m.parents, false)
	}
	indent := 0
	for _, ordered := range m.parents {
		if ordered {
			// CommonMark only nests items under "1. " when they are
			// indented to its content, 3 columns in.
			indent += max(m.opt.Indent, 3)
		} else {
			indent += m.opt.Indent
		}
	}
	m.parents = append(m.parents, l.Ordered)
	return indent
}

func (m *mdWriter) table(t *docx.Table) {
	var rows [][]string
	cols := 0
	for _, r := range t.Rows {
		row := make([]string, len(r))
		for i, c := range r {
			row[i] = cellText(c.Blocks)
		}
		rows = append(rows, row)
		cols = max(cols, len(row))
	}
	if cols == 0 {
		return
	}
	widths := make([]int, cols)
	if m.opt.AlignTables {
		for i := range widths {
			widths[i] = 3 // the "---" separator
		}
		for _, row := range rows {
			for i, s := range row {
				widths[i] = max(widths[i], utf8.RuneCountInString(s))
			}
		}
	}

	m.start(false)
	line := func(cells []string, pad string) {
		m.buf.WriteString("|")
		for i := range cols {
			s := ""
			if i < len(cells) {
				s = cells[i]
			}
			m.buf.WriteString(" " + s + strings.Repeat(pad, max(widths[i]-utf8.RuneCountInString(s), 0)) + " |")
		}
	}
	line(rows[0], " ")
	m.buf.WriteString("\n")
	sep := make([]string, cols)
	for i := range sep {
		sep[i] = "---"
	}
	line(sep, "-")
	for _, row := range rows[1:] {
		m.buf.WriteString("\n")
		line(row, " ")
	}
}

// cellText renders the content of a table cell on a single line, joining
// paragraphs and line breaks with <br>. Nested tables are flattened.
func cellText(blocks []docx.Block) string {
	var parts []string
	for _, b := range blocks {
		switch b := b.(type) {
		case *docx.Paragraph:
			for _, l := range strings.Split(b.Text, "\n") {
				if l = strings.TrimSpace(l); l != "" {
					parts = append(parts, escape(l, inlineSpecial+"|"))
				}
			}
		case *docx.Table:
			for _, r := range b.Rows {
				for _, c := range r {
					if s := cellText(c.Blocks); s != "" {
						parts = append(parts, s)
					}
				}
			}
		}
	}
	return strings.Join(parts, "<br>")
}

// textLines splits paragraph text at line breaks into escaped, trimmed,
// non-empty lines.
func textLines(text string) []string {
	var lines []string
	for _, l := range strings.Split(text, "\n") {
		if l = strings.TrimSpace(l); l != "" {
			lines = append(lines, escapeLine(l))
		}
	}
	return lines
}

// inlineSpecial lists the characters that can start inline Markdown syntax.
const inlineSpecial = "\\`*_[]<>"

func escape(s, special string) string {
	var b strings.Builder
	for _, r := range s {
		if r == docx.Picture {
			b.WriteString("*[picture]*")
			continue
		}
		if strings.ContainsRune(special, r) {
			b.WriteByte('\\')
		}
		b.WriteRune(r)
	}
	return b.String()
}

// escapeLine escapes s so that, at the start of a line, it can't be read as
// a heading, quote, list item, thematic break or code fence.
func escapeLine(s string) string {
	s = escape(s, inlineSpecial)
	if strings.ContainsRune("#>-+=~", rune(s[0])) {
		return "\\" + s
	}
	digits := len(s) - len(strings.TrimLeft(s, "0123456789"))
	if digits > 0 && digits < len(s) && (s[digits] == '.' || s[digits] == ')') {
		return s[:digits] + "\\" + s[digits:]
	}
	return s
}

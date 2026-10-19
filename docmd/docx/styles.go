package docx

import (
	"archive/zip"
	"strconv"
	"strings"
)

// pPr holds the paragraph properties docx cares about, from a paragraph or a style.
type pPr struct {
	Style      *val `xml:"pStyle"`
	OutlineLvl *val `xml:"outlineLvl"`
	NumPr      *struct {
		ILvl  *val `xml:"ilvl"`
		NumID *val `xml:"numId"`
	} `xml:"numPr"`
}

type style struct {
	name    string // lowercased; stays English in localized documents, unlike the ID
	basedOn string
	pPr     pPr
}

// styles maps paragraph style IDs to their definitions.
type styles map[string]*style

func parseStyles(f *zip.File) (styles, error) {
	var x struct {
		Style []struct {
			Type    string `xml:"type,attr"`
			ID      string `xml:"styleId,attr"`
			Name    val    `xml:"name"`
			BasedOn val    `xml:"basedOn"`
			PPr     pPr    `xml:"pPr"`
		} `xml:"style"`
	}
	if err := decodePart(f, &x); err != nil {
		return nil, err
	}
	s := make(styles)
	for _, st := range x.Style {
		if st.Type == "paragraph" {
			s[st.ID] = &style{strings.ToLower(st.Name.Val), st.BasedOn.Val, st.PPr}
		}
	}
	return s, nil
}

// find returns the first style in the basedOn chain starting at id for which
// f returns true, or nil.
func (s styles) find(id string, f func(*style) bool) *style {
	// The bound guards against basedOn cycles in malformed documents.
	for range 32 {
		st := s[id]
		if st == nil {
			return nil
		}
		if f(st) {
			return st
		}
		id = st.basedOn
	}
	return nil
}

// heading returns the heading level that style id gives a paragraph, or 0.
func (s styles) heading(id string) int {
	level := 0
	s.find(id, func(st *style) bool {
		if st.pPr.OutlineLvl != nil {
			level = outlineHeading(st.pPr.OutlineLvl.Val)
			return true
		}
		if n, ok := strings.CutPrefix(st.name, "heading "); ok {
			if l, err := strconv.Atoi(n); err == nil && l >= 1 && l <= 9 {
				level = l
				return true
			}
		}
		return false
	})
	return level
}

// outlineHeading converts a w:outlineLvl value (0..8, 9 = body text) to a heading level.
func outlineHeading(v string) int {
	l, err := strconv.Atoi(v)
	if err != nil || l < 0 || l > 8 {
		return 0
	}
	return l + 1
}

// numPr returns the numbering ID and level inherited from style id.
func (s styles) numPr(id string) (numID, ilvl string) {
	st := s.find(id, func(st *style) bool { return st.pPr.NumPr != nil })
	if st == nil {
		return "", ""
	}
	if np := st.pPr.NumPr; np.NumID != nil {
		numID = np.NumID.Val
	}
	if np := st.pPr.NumPr; np.ILvl != nil {
		ilvl = np.ILvl.Val
	}
	return numID, ilvl
}

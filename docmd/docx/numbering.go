package docx

import "archive/zip"

// numbering maps a numbering instance (w:numId) and level (w:ilvl) to its
// number format, e.g. "bullet" or "decimal".
type numbering map[string]map[string]string

func parseNumbering(f *zip.File) (numbering, error) {
	var x struct {
		AbstractNum []struct {
			ID  string `xml:"abstractNumId,attr"`
			Lvl []struct {
				ILvl   string `xml:"ilvl,attr"`
				NumFmt *val   `xml:"numFmt"`
				// Word puts newer formats in markup-compatibility blocks,
				// with a classic format as the fallback.
				Fallback *val `xml:"AlternateContent>Fallback>numFmt"`
			} `xml:"lvl"`
		} `xml:"abstractNum"`
		Num []struct {
			ID            string `xml:"numId,attr"`
			AbstractNumID val    `xml:"abstractNumId"`
		} `xml:"num"`
	}
	if err := decodePart(f, &x); err != nil {
		return nil, err
	}
	formats := make(map[string]map[string]string)
	for _, a := range x.AbstractNum {
		lvls := make(map[string]string)
		for _, l := range a.Lvl {
			switch {
			case l.NumFmt != nil:
				lvls[l.ILvl] = l.NumFmt.Val
			case l.Fallback != nil:
				lvls[l.ILvl] = l.Fallback.Val
			}
		}
		formats[a.ID] = lvls
	}
	n := make(numbering)
	for _, num := range x.Num {
		n[num.ID] = formats[num.AbstractNumID.Val]
	}
	return n, nil
}

// ordered reports whether level ilvl of numbering instance numID is numbered
// rather than bulleted. Unknown definitions count as bulleted.
func (n numbering) ordered(numID, ilvl string) bool {
	switch n[numID][ilvl] {
	case "", "bullet", "none":
		return false
	}
	return true
}

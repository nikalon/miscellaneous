# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

docmd is a CLI that converts `.docx` files to Markdown, keeping only text content (headings, paragraphs, lists, tables). Go standard library only — do not add external modules.

## Commands

```sh
go build ./cmd/docmd                          # build the CLI
go vet ./... && go test ./...                 # check everything
go test ./cmd/docmd -run 'TestCases/lists'    # run a single test case
go test ./cmd/docmd -update                   # regenerate want.md goldens; review with git diff
```

## Architecture

- `docx/` — **reusable library** (meant to be imported by other projects): parses a .docx into a small semantic model (`Document` → `Paragraph`/`Table` blocks). It resolves Word's machinery (style IDs, `basedOn` chains, numbering definitions) into meaning — `Heading` level, `List{Level, Ordered}` — so consumers never see OOXML. Keep it free of Markdown concerns.
  - Parts are located via OPC relationships (`_rels/.rels` → main document → its `.rels` for styles/numbering), not hardcoded paths.
  - `document.xml` is streamed with `xml.Decoder` (order of paragraphs/tables matters); property elements (`pPr`, `tcPr`) are decoded with `DecodeElement`. Elements are matched by local name only (Transitional and Strict namespaces).
  - `body.go`'s `transparent` map lists wrapper elements whose children are processed as if unwrapped; **every other unhandled element is skipped entirely** (deletions, field instructions, drawings, text boxes). New content support usually means adding a case there.
  - Headings: `outlineLvl` wins, else style name `heading N` (names stay English in localized docs; style IDs don't).
- `cmd/docmd/` — CLI plus the tool-specific Markdown writer (`markdown.go`, unexported, not meant for reuse). Formatting options (`-indent`, `-align-tables`) live in `options`; their defaults exist only in the flag definitions in `main.go`.

## Testing

End-to-end tests only, by design — internals are expected to change, so don't add unit tests of internal functions. `TestCases` calls `run()` (the CLI entry point) in-process for each `cmd/docmd/testdata/<program>/<case>/`, where `<program>` names the application that produced the input (`libreoffice`; `invalid` holds broken files for error cases). Test inputs are real `.docx` files only — no hand-written OOXML.

- `input.docx`: the document under test
- `args`: optional extra flags; the runner always prepends `-indent 2 -align-tables=false` so changing a CLI default never breaks cases — only cases about indentation/alignment set those flags
- `want.md` expected output, or `want.err` with a substring of the expected stderr for failure cases

LibreOffice cases are generated: edit `input.fodt` (flat ODT source) and run `testdata/libreoffice/convert.sh [case...]` (uses `soffice`, or the Flatpak if absent), then `-update` and review the Markdown diff. Before trusting new output, check the generated `word/document.xml`/`styles.xml` — LibreOffice silently drops style attributes it doesn't like (every `.fodt` needs the `office:master-styles` block for parent/list/outline style attributes to survive), so a "wrong" output may be a faithful conversion of a wrong input.

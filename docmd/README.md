# docmd

docmd converts Word `.docx` files to Markdown. It keeps the text content only: headings, paragraphs, lists and tables. Pictures become a `*[picture]*` placeholder; formatting, comments, footnotes and text boxes are dropped.

It's a single self-contained binary written in Go, using only the standard library.

## Building

Requires Go 1.26 or later.

```sh
go build ./cmd/docmd
```

This produces a `docmd` binary (`docmd.exe` on Windows) in the current directory. To build for another platform, set `GOOS`/`GOARCH`:

```sh
GOOS=windows GOARCH=amd64 go build ./cmd/docmd
```

Run the tests with:

```sh
go vet ./... && go test ./...
```

## Usage

```
docmd [flags] input.docx...
```

Each input is written next to it with a `.md` extension, so `report.docx` becomes `report.md`:

```sh
docmd report.docx            # writes report.md
docmd *.docx                 # converts every .docx in the directory
docmd -o - report.docx       # prints to stdout
docmd -o notes.md report.docx
```

Wildcards also work in Windows `cmd.exe` and PowerShell, where docmd expands `*` and `?` itself.

| Flag | Default | Description |
| --- | --- | --- |
| `-o file` | | Write to `file` instead of `input.md` (`-` for stdout). Only with a single input. |
| `-indent n` | `2` | Spaces per list nesting level. |
| `-align-tables` | `true` | Pad table cells so columns line up. Use `-align-tables=false` to disable. |

Flags can go before, between or after the inputs. If some inputs fail, the rest are still converted and docmd exits with status 1.

## TODO

- Include a picture's alt text in the placeholder, e.g. `*[picture: A bar chart of sales by region]*`.

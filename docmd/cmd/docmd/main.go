// Command docmd converts .docx files to Markdown.
package main

import (
	"bytes"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"docmd/docx"
)

func main() {
	os.Exit(run(os.Args[1:], os.Stdout, os.Stderr))
}

func run(args []string, stdout, stderr io.Writer) int {
	fs := flag.NewFlagSet("docmd", flag.ContinueOnError)
	fs.SetOutput(stderr)
	fs.Usage = func() {
		fmt.Fprintln(stderr, "usage: docmd [flags] input.docx...")
		fs.PrintDefaults()
	}
	out := fs.String("o", "", "write to `file` instead of input.md (\"-\" for stdout); only with a single input")
	var opt options
	fs.IntVar(&opt.Indent, "indent", 2, "spaces per list nesting level")
	fs.BoolVar(&opt.AlignTables, "align-tables", true, "pad table cells so columns line up")

	// Accept flags before, between and after inputs.
	var inputs []string
	for {
		if err := fs.Parse(args); err != nil {
			return 2
		}
		if fs.NArg() == 0 {
			break
		}
		inputs = append(inputs, expand(fs.Arg(0))...)
		args = fs.Args()[1:]
	}
	switch {
	case len(inputs) == 0:
		fs.Usage()
		return 2
	case *out != "" && len(inputs) > 1:
		fmt.Fprintln(stderr, "docmd: -o requires a single input")
		return 2
	case opt.Indent < 0:
		fmt.Fprintln(stderr, "docmd: -indent must not be negative")
		return 2
	}

	status := 0
	for _, in := range inputs {
		if err := convert(in, *out, opt, stdout); err != nil {
			fmt.Fprintf(stderr, "docmd: %s: %v\n", in, err)
			status = 1
		}
	}
	return status
}

// expand does the wildcard expansion that Windows shells, unlike Unix ones,
// leave to the program. A pattern matching nothing is kept as is.
func expand(arg string) []string {
	if runtime.GOOS == "windows" && strings.ContainsAny(arg, "*?") {
		if matches, _ := filepath.Glob(arg); len(matches) > 0 {
			return matches
		}
	}
	return []string{arg}
}

func convert(in, out string, opt options, stdout io.Writer) error {
	if !strings.EqualFold(filepath.Ext(in), ".docx") {
		return errors.New("not a .docx file")
	}
	doc, err := docx.Open(in)
	if err != nil {
		return err
	}
	if out == "-" {
		return writeMarkdown(stdout, doc, opt)
	}
	if out == "" {
		out = strings.TrimSuffix(in, filepath.Ext(in)) + ".md"
	}
	var buf bytes.Buffer
	if err := writeMarkdown(&buf, doc, opt); err != nil {
		return err
	}
	return os.WriteFile(out, buf.Bytes(), 0o644)
}

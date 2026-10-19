package main

import (
	"bytes"
	"flag"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

var update = flag.Bool("update", false, "rewrite want.md files with the current output")

// TestCases runs the CLI on each testdata/<program>/<case> directory, where
// <program> names the application that produced the case's input.docx. A case
// also holds an optional args file with extra flags, and the expected want.md
// output or a want.err substring of the expected error.
func TestCases(t *testing.T) {
	dirs, err := filepath.Glob("testdata/*/*")
	if err != nil {
		t.Fatal(err)
	}
	for _, dir := range dirs {
		if fi, err := os.Stat(dir); err != nil || !fi.IsDir() {
			continue
		}
		t.Run(filepath.ToSlash(strings.TrimPrefix(dir, "testdata"+string(filepath.Separator))), func(t *testing.T) {
			input := filepath.Join(dir, "input.docx")
			// Pin the formatting options so changing a default doesn't break cases.
			args := []string{"-indent", "2", "-align-tables=false"}
			if b, err := os.ReadFile(filepath.Join(dir, "args")); err == nil {
				args = append(args, strings.Fields(string(b))...)
			}
			args = append(args, "-o", "-", input)

			var stdout, stderr bytes.Buffer
			code := run(args, &stdout, &stderr)

			if want, err := os.ReadFile(filepath.Join(dir, "want.err")); err == nil {
				if code == 0 || !strings.Contains(stderr.String(), strings.TrimSpace(string(want))) {
					t.Fatalf("exit code %d, stderr %q; want failure containing %q", code, stderr.String(), want)
				}
				return
			}
			if code != 0 {
				t.Fatalf("exit code %d: %s", code, stderr.String())
			}
			wantFile := filepath.Join(dir, "want.md")
			if *update {
				if err := os.WriteFile(wantFile, stdout.Bytes(), 0o644); err != nil {
					t.Fatal(err)
				}
				return
			}
			want, err := os.ReadFile(wantFile)
			if err != nil {
				t.Fatal(err)
			}
			if got := stdout.String(); got != string(want) {
				t.Errorf("output differs from %s\n--- got ---\n%s--- want ---\n%s", wantFile, got, want)
			}
		})
	}
}

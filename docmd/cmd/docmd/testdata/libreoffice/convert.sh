#!/bin/sh
# Regenerates each case's input.docx from its input.fodt (LibreOffice flat ODT)
# by saving it with LibreOffice Writer. Usage: ./convert.sh [case...]
set -eu
cd "$(dirname "$0")"

if command -v soffice >/dev/null; then
	soffice() { command soffice "$@"; }
else
	soffice() { flatpak run --filesystem="$PWD" org.libreoffice.LibreOffice "$@"; }
fi

[ $# -gt 0 ] || set -- */
for c in "$@"; do
	c=${c%/}
	soffice --headless --convert-to docx:"MS Word 2007 XML" --outdir "$PWD/$c" "$PWD/$c/input.fodt"
done

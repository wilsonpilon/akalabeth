#!/bin/bash
# Builds dist/akalabeth.dsk (720 KB, MSX-DOS 2) with msxdisk.exe, from out/akalabeth.com
HERE="$(cd "$(dirname "$0")" && pwd)"
PROJ="$HERE/.."
ROOT="$PROJ/../../.."                       # E:/akalabeth
DOS="$PROJ/../../tools/build/DOS"
DIST="$ROOT/dist"
MSXDISK="$ROOT/msxdisk.exe"
mkdir -p "$DIST/stage" && cd "$DIST/stage" || exit 1
rm -f ./* ../akalabeth.dsk
cp "$PROJ/out/akalabeth.com" AKALABET.COM
printf 'AKALABET\r\n' > AUTOEXEC.BAT
cp "$DOS/COMMAND2.COM" "$DOS/MSXDOS2.SYS" .
"$MSXDISK" create ../akalabeth.dsk
"$MSXDISK" add ../akalabeth.dsk AUTOEXEC.BAT AKALABET.COM COMMAND2.COM MSXDOS2.SYS
cp AKALABET.COM ..
cd .. && rm -rf stage
"$MSXDISK" list akalabeth.dsk -l

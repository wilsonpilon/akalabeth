#!/bin/bash
# usage: runtim.sh <label>   (builds release for the world test, debug 4 for the dungeon test)
SP=/d/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad
H=/e/akalabeth/MSXgl/projects/akalabeth/akalabeth.h
OUT=$SP/timing_$1.txt; : > $OUT
for pair in "0 scenT1" "4 scenT2"; do
  set -- $pair
  sed -i -E "s/#define AK_DEBUG\t\t[0-9]+/#define AK_DEBUG\t\t$1/" $H
  (cd /e/akalabeth/MSXgl/projects/akalabeth && bash build.sh 2>&1 | grep -i "error\|undefined" | grep -v DSK)
  cd $SP && ./mkkw.sh >/dev/null && cp /e/akalabeth/MSXgl/projects/akalabeth/out/akalabeth.com dos2/AKA.COM
  rm -f steps.log
  powershell -NoProfile -ExecutionPolicy Bypass -File run.ps1 -Scn $2 -Disk dos2 -Ext msxdos2 > /dev/null
  cat steps.log >> $OUT
done
sed -i -E "s/#define AK_DEBUG\t\t[0-9]+/#define AK_DEBUG\t\t0/" $H
cat $OUT

#!/bin/bash
MAP=/e/akalabeth/MSXgl/projects/akalabeth/out/akalabeth.map
g() { grep -i " $1 " $MAP | head -1 | awk '{print $1}'; }
KW=$(g _g_keywait); DT=$(g _g_drawtime); PR=$(g _g_prof); G0=$(g _ui_getkey); G1=$(g _ui_getkey_upper); TK=$(g _g_sndTicks); HT=$(g _g_hudtime); TT=$(g _g_turntime)
echo "set KW 0x${KW:4}; set DT 0x${DT:4}; set PR 0x${PR:4}; set GK0 0x${G0:4}; set GK1 0x${G1:4}; set TK 0x${TK:4}; set HT 0x${HT:4}; set TT 0x${TT:4}" > /d/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/kw.tcl
cat /d/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/kw.tcl

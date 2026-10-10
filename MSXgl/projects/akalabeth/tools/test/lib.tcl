set SH "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/shots"
catch { set throttle off } r
catch { set maxframeskip 0 } r
catch { set minframeskip 0 } r
proc shot {name} { global SH; screenshot "$SH/$name.png" }
proc arrow {dir} {
  array set m {left 0x10 up 0x20 down 0x40 right 0x80}
  keymatrixdown 8 $m($dir)
  after time 0.15 "keymatrixup 8 $m($dir)"
}
proc at {t script} { after time $t $script }
proc vram {name} { global SH; save_debuggable VRAM "[file dirname $SH]/$name.vram" }
# Wait until the game switches the VDP to SCREEN 5, then run the given script (times inside are relative to that moment)
proc wait_game {script} {
  if {[catch {set r0 [vdpreg 0]; set r1 [vdpreg 1]}]} { after time 0.25 [list wait_game $script]; return }
  if {$r0 == 6 && ($r1 & 0x40)} { uplevel #0 $script } else { after time 0.25 [list wait_game $script] }
}
proc t {dt script} { after time $dt $script }
proc esc_press {} { keymatrixdown 7 0x04; after time 0.15 "keymatrixup 7 0x04" }

source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {shot X0_start 0}
  {arrow up}
  {arrow up}
  {arrow up}
  {shot X1_after3 0}
  {key "w"}
  {shot X2_saved 0}
  {arrow up}
  {arrow up}
  {shot X3_moved 0}
  {key "l"}
  {shot X4_loaded 0}
}
wait_game start_steps
after time 100000 {exit}

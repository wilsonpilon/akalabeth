source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {key "ffff"}
  {key "q"}
  {arrow right}
  {arrow right}
  {arrow down}
  {shot R4_a}
  {key "w"}
  {shot R4_saved}
  {arrow right}
  {arrow right}
  {arrow right}
  {shot R4_moved}
  {key "l"}
  {shot R4_loaded}
  {key "h"}
  {shot R4_help}
  {key " "}
  {shot R4_end}
}
wait_game start_steps
after time 100000 {exit}

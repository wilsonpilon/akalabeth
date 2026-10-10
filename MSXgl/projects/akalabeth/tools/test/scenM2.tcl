source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {key "p"}
  {shot M2_start 0}
  {arrow up}
  {shot M2_move 0}
  {key " "}
  {shot M2_attacked 0}
  {key " "}
  {key "a"}
  {key "r"}
  {shot M2_strike 0}
  {key " "}
  {shot M2_after 0}
}
wait_game start_steps
after time 100000 {exit}

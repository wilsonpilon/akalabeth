source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {key "p"}
  {shot M1_start 0}
  {arrow up}
  {shot M1_fall_msgs 0}
  {key " "}
  {shot M1_level2 0}
}
wait_game start_steps
after time 100000 {exit}

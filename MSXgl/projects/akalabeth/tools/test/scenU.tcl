source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {shot U0_start 0}
  {arrow up}
  {arrow up}
  {shot U1_door 0}
  {arrow up}
  {arrow up}
  {arrow up}
  {shot U2_chest_ahead 0}
  {arrow up}
  {shot U3_hole_here 0}
  {key "\r"}
  {shot U4_level3 0}
  {key "s"}
  {shot U5_status 0}
  {key "\r"}
  {shot U6_back 0}
}
wait_game start_steps
after time 100000 {exit}

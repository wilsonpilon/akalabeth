source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {key "h"}
  {shot R2_help_ingame}
  {key " "}
  {shot R2_back}
  {key "\r"}
  {shot R2_outro}
  {key " "}
  {shot R2_knight}
}
wait_game start_steps
after time 100000 {exit}

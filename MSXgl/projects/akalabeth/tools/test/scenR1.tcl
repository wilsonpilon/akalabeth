source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {shot R_intro 300}
  {key " "}
  {shot R_title 300}
  {key "h"}
  {shot R_help 300}
  {key " "}
  {shot R_title2 300}
}
wait_game start_steps
after time 100000 {exit}

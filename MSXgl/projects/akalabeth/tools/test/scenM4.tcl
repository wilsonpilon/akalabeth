source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "m"}
  {esc}
  {shot M4_prompt 0}
  {key "n"}
  {shot M4_no 0}
  {esc}
  {key "y"}
  {shot M4_dos 3}
}
wait_game start_steps
after time 100000 {exit}

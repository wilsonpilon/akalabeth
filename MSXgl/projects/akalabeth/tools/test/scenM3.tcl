source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "m"}
  {key "p"}
  {key "a"}
  {key "m"}
  {shot M3_menu 0}
  {key "1"}
  {shot M3_ladder_msg 0}
  {key " "}
  {shot M3_ladder_here 0}
  {key "\r"}
  {shot M3_up_msg 0}
  {key " "}
  {shot M3_level1 0}
  {key "a"}
  {key "m"}
  {key "2"}
  {key " "}
  {shot M3_ladder_down 0}
  {key "\r"}
  {key " "}
  {shot M3_back_level2 0}
  {esc}
  {shot M4_quit_prompt 0}
  {key "y"}
  {shot M4_dos 0}
}
wait_game start_steps
after time 100000 {exit}

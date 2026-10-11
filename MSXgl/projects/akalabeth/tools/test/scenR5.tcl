source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {shot R5_start}
  {arrow up}
  {arrow up}
  {key "a"}
  {key "r"}
  {key " "}
  {shot R5_fight}
  {arrow left}
  {arrow left}
  {arrow up}
  {arrow up}
  {psg d1}
  {sleep 2}
  {psg d2}
  {key "m"}
  {sleep 1}
  {shot R5_nomusic}
  {key "m"}
  {sleep 3}
  {psg d3}
  {key "h"}
  {sleep 1}
  {shot R5_help}
  {key " "}
  {sleep 1}
  {shot R5_back}
}
wait_game start_steps
after time 100000 {exit}

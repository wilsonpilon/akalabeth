source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key " "}
  {key "7"}
  {key "3"}
  {key "y"}
  {key "f"}
  {shot R14_rat}
  {mx 7 0x40}
  {ty "m"}
  {sleep 1}
  {shot R14_m}
  {mx 7 0x40}
  {ty "u"}
  {sleep 1}
  {mx 7 0x40}
  {ty "d"}
  {sleep 1}
  {mx 7 0x40}
  {ty "p"}
  {sleep 1}
  {mx 7 0x40}
  {ty "t"}
  {sleep 1}
  {mx 7 0x40}
  {ty "e"}
  {sleep 1}
  {shot R14_e}
  {mx 7 0x40}
  {ty "g"}
  {sleep 1}
  {mx 7 0x40}
  {ty "h"}
  {sleep 1}
  {mx 7 0x40}
  {ty "f"}
  {sleep 2}
  {shot R14_end}
}
wait_game start_steps
after time 100000 {exit}

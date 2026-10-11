source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {sleep 12}
  {mx 7 0x08}
  {sleep 3}
  {ty " "}
  {sleep 3}
  {ty "7"}
  {mx 7 0x80}
  {ty "3"}
  {mx 7 0x80}
  {ty "y"}
  {ty "f"}
  {sleep 4}
  {shot R11_rat}
  {mx 7 0x40}
  {ty "m"}
  {sleep 2}
  {shot R11_m}
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
  {sleep 2}
  {shot R11_all}
  {mx 7 0x40}
  {ty "g"}
  {sleep 2}
  {shot R11_gold}
}
rename exit exit_orig
proc exit {} {
  set f [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/chk.txt" a]
  puts $f "cheat=[peek 0xCE18] t=[machine_info time]"; close $f
  exit_orig
}
wait_game start_steps
after time 100000 {exit}

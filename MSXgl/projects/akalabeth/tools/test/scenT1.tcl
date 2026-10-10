source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {key "ffff"}
  {key "q"}
  {arrow right}
  {tim world_move1}
  {arrow right}
  {tim world_move2}
  {arrow down}
  {tim world_move3}
}
wait_game start_steps
after time 100000 {exit}

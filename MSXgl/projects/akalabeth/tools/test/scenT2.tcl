source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::steps {
  {until 1}
  {key " "}
  {key "7\r"}
  {key "3\r"}
  {key "y"}
  {key "f"}
  {tim dungeon_start}
  {arrow up}
  {tim dungeon_fwd1}
  {arrow up}
  {tim dungeon_fwd2}
  {arrow right}
  {tim dungeon_turn}
  {arrow up}
  {tim dungeon_fwd3}
}
wait_game start_steps
after time 100000 {exit}

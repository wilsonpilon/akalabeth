source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib2.tcl"
set ::pf [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/pcs.txt" w]
set ::on 0
proc samp {} {
  if {$::on} { puts $::pf [format %04X [reg PC]] }
  after time 0.001 samp
}
proc start_samp {} { set ::on 1; runner }
rename exit exit_orig
proc exit {} { close $::pf; exit_orig }
set ::steps {
  {sleep 8}
  {arrow up}
  {arrow up}
  {arrow left}
  {arrow up}
  {arrow right}
  {arrow up}
  {arrow down}
  {arrow up}
  {arrow up}
  {arrow left}
  {arrow up}
}
after time 1 samp
after time 9 {set ::on 1}
wait_game start_steps
after time 100000 {exit}

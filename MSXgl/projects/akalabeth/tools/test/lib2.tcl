source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/lib.tcl"
source "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/kw.tcl"
set ::steps {}
set ::waitstart 0
proc cnt {} { return [peek $::KW] }
# steps: {key "text"} {shot name ?delay?} {sleep secs} {arrow dir} {vram name}
proc runner {} {
  if {[llength $::steps] == 0} { exit }
  set s [lindex $::steps 0]
  set ::steps [lrange $::steps 1 end]
  set kind [lindex $s 0]
  set lf [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/trace.log" a]; puts $lf "[format %.1f [machine_info time]] step=$s cnt=[cnt]"; close $lf
  switch $kind {
    key   { set text [lindex $s 1]
            set n [string length $text]
            set c0 [cnt]
            type $text
            wait_idle [expr {($c0 + $n) & 255}] [machine_info time] }
    arrow { set c0 [cnt]; arrow [lindex $s 1]
            wait_idle [expr {($c0 + 1) & 255}] [machine_info time] }
    shot  { set d 0.8; if {[llength $s] > 2} { set d [lindex $s 2] }
            after realtime 0.8 "shot [lindex $s 1]; after realtime 0.4 runner" }
    vram  { after time 0.8 "vram [lindex $s 1]; runner" }
    sleep { after time [lindex $s 1] runner }
    esc   { set c0 [cnt]; esc_press; wait_idle [expr {($c0 + 1) & 255}] [machine_info time] }
    psg   { set f [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/steps.log" a]
            set r {}; for {set i 0} {$i < 11} {incr i} { lappend r [format %02X [debug read "PSG regs" $i]] }
            puts $f "[lindex $s 1] emu=[format %.2f [machine_info time]] ticks=[expr {[peek $::TK] + 256*[peek [expr {$::TK+1}]]}] psg=$r draw=[expr {[peek $::DT] + 256*[peek [expr {$::DT+1}]]}] prof=[prof]"; close $f; runner }
    tim   { set f [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/steps.log" a]
            puts $f "[lindex $s 1]: turn=[expr {[peek $::TT] + 256*[peek [expr {$::TT+1}]]}] draw=[expr {[peek $::DT] + 256*[peek [expr {$::DT+1}]]}] hud=[expr {[peek $::HT] + 256*[peek [expr {$::HT+1}]]}] frames"; close $f; runner }
    until { wait_cnt [lindex $s 1] [machine_info time] }
    pl { pl; runner }
    settle { settle_wait 0 [machine_info time] }
    sample { sampler; runner }
    dump  { dump_samples; runner }
    log   { set f [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/steps.log" a]
            puts $f "[lindex $s 1]: drawtime=[expr {[peek $::DT] + 256*[peek [expr {$::DT+1}]]}] frames prof=[prof]"; close $f; runner }
  }
}
proc wait_idle {expected t0} {
  if {[cnt] == $expected || ([machine_info time] - $t0) > 12} { after time 0.1 runner } else { after time 0.1 [list wait_idle $expected $t0] }
}
proc start_steps {} { after time 0.5 runner }

proc prof {} { set r {}; for {set i 0} {$i < 5} {incr i} { lappend r [expr {[peek [expr {$::PR+2*$i}]] + 256*[peek [expr {$::PR+2*$i+1}]]}] }; return $r }
set ::pcs {}
proc sampler {} {
  if {[llength $::pcs] >= 1200} { return }
  lappend ::pcs [reg PC]
  after time 0.0041 sampler
}
proc dump_samples {} {
  set f [open "D:/temp/user/claude/E--akalabeth/4b906f04-2e13-4ac8-afe1-1fc63e17fd4d/scratchpad/samples.txt" w]
  foreach s $::pcs { puts $f $s }
  close $f
}

# idle = nearly all recent PC samples are inside ui_getkey (game blocked waiting for a key)
set ::hist {}
proc settle_wait {n t0} {
  set pc [reg PC]
  lappend ::hist [expr {$pc >= $::GK0 && $pc < $::GK1}]
  if {[llength $::hist] > 14} { set ::hist [lrange $::hist 1 end] }
  set hits 0; foreach h $::hist { incr hits $h }
  if {([llength $::hist] == 14 && $hits >= 12) || ([machine_info time] - $t0) > 60} { set ::hist {}; after time 0.5 runner } else { after time 0.053 [list settle_wait $n $t0] }
}

proc wait_cnt {v t0} {
  if {[cnt] == $v || ([machine_info time] - $t0) > 40} { after time 0.3 runner } else { after time 0.1 [list wait_cnt $v $t0] }
}

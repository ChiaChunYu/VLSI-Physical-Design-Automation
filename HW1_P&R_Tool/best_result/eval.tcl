puts "+------------------------------------------------+"
puts "|                                                |"
puts "|    This script is used for PDA HW1 grading.    |"
puts "|                                                |"
puts "+------------------------------------------------+"

setMultiCpuUsage -localCpu 32 -cpuPerRemoteHost 1 -remoteHost 0 -keepLicense true
setDistributeHost -local

# Restore the best design
restoreDesign best_result.enc.dat.tar.gz misty

# Check slack time
puts "\[Info\] Extracting slack time..."
redirect -variable timing_report "report_timing"
regexp {= Slack Time\s+(-?[\d.]+)} $timing_report all slack

# Check DRC violations
puts "\[Info\] Extracting DRC violations..."
redirect -variable drc_report "verify_drc"
regexp {  Verification Complete :\s+([\d]+)\s+Viols.} $drc_report all drc

# Report clock period
puts "\[Info\] Extracting clock period..."
set clock_period [get_attribute [get_clocks clk] period]

# Report wirelength and area
puts "\[Info\] Extracting wirelength and area..."
summaryReport -noHtml -outfile summary.rpt
redirect -variable summary_report "cat summary.rpt"
regexp {Total wire length:\s+([\d.]+)\s+um} $summary_report all wirelength
regexp {Total area of Chip:\s+([\d.]+)\s+um\^2} $summary_report all area

# Dump results
puts ""
puts "--- Summary of the Current Result ---"
puts " Slack Time:         $slack"
puts " DRC Violations:     $drc"
puts " Clock Period:       $clock_period"
puts " Total area of chip: $area"
puts " Total wire length:  $wirelength"
puts "-------------------------------------"
exit

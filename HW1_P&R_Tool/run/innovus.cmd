#######################################################
#                                                     
#  Innovus Command Logging File                     
#  Created on Sat Sep 20 17:36:43 2025                
#                                                     
#######################################################

#@(#)CDS: Innovus v21.13-s100_1 (64bit) 03/04/2022 14:32 (Linux 3.10.0-693.el7.x86_64)
#@(#)CDS: NanoRoute 21.13-s100_1 NR220220-0140/21_13-UB (database version 18.20.572) {superthreading v2.17}
#@(#)CDS: AAE 21.13-s034 (64bit) 03/04/2022 (Linux 3.10.0-693.el7.x86_64)
#@(#)CDS: CTE 21.13-s042_1 () Mar  4 2022 08:38:36 ( )
#@(#)CDS: SYNTECH 21.13-s014_1 () Feb 17 2022 23:50:03 ( )
#@(#)CDS: CPE v21.13-s074
#@(#)CDS: IQuantus/TQuantus 20.1.2-s656 (64bit) Tue Nov 9 23:11:16 PST 2021 (Linux 2.6.32-431.11.2.el6.x86_64)

set_global _enable_mmmc_by_default_flow      $CTE::mmmc_default
suppressMessage ENCEXT-2799
win
setMultiCpuUsage -localCpu 8 -cpuPerRemoteHost 1 -remoteHost 0 -keepLicense true
setDistributeHost -local
setDesignMode -process 7 -node N7
set init_gnd_net VSS
set init_lef_file {../lef/asap7_tech_4x_201209.lef ../lef/asap7sc7p5t_28_L_4x_220121a.lef ../lef/asap7sc7p5t_28_SL_4x_220121a.lef}
set init_verilog ../design/misty.v
set init_mmmc_file ../scripts/mmmc.view
set init_pwr_net VDD
init_design
setDesignMode -bottomRoutingLayer 2
setDesignMode -topRoutingLayer 7
clearGlobalNets
globalNetConnect VDD -type pgpin -pin VDD -instanceBasename *
clearGlobalNets
globalNetConnect VDD -type pgpin -pin VDD -instanceBasename *
globalNetConnect VSS -type pgpin -pin VSS -instanceBasename *
getIoFlowFlag
setIoFlowFlag 0
floorPlan -coreMarginsBy die -site asap7sc7p5t -r 1.0 0.66 6.22 6.22 6.22 6.22
uiSetTool select
getIoFlowFlag
fit
add_tracks -snap_m1_track_to_cell_pins
add_tracks -mode replace -offsets {M5 vertical 0}
deleteAllFPObjects
addWellTap -cell TAPCELL_ASAP7_75t_L -cellInterval 12.960 -inRowOffset 1.296
setPinAssignMode -pinEditInBatch true
editPin -fixOverlap 1 -unit MICRON -spreadDirection clockwise -side Left -layer 3 -spreadType center -spacing 0.72 -pin {{data_in[0]} {data_in[1]} {data_in[2]} {data_in[3]} {data_in[4]} {data_in[5]} {data_in[6]} {data_in[7]} {data_in[8]} {data_in[9]} {data_in[10]} {data_in[11]} {data_in[12]} {data_in[13]} {data_in[14]} {data_in[15]} {data_in[16]} {data_in[17]} {data_in[18]} {data_in[19]} {data_in[20]} {data_in[21]} {data_in[22]} {data_in[23]} {data_in[24]} {data_in[25]} {data_in[26]} {data_in[27]} {data_in[28]} {data_in[29]} {data_in[30]} {data_in[31]} {data_in[32]} {data_in[33]} {data_in[34]} {data_in[35]} {data_in[36]} {data_in[37]} {data_in[38]} {data_in[39]} {data_in[40]} {data_in[41]} {data_in[42]} {data_in[43]} {data_in[44]} {data_in[45]} {data_in[46]} {data_in[47]} {data_in[48]} {data_in[49]} {data_in[50]} {data_in[51]} {data_in[52]} {data_in[53]} {data_in[54]} {data_in[55]} {data_in[56]} {data_in[57]} {data_in[58]} {data_in[59]} {data_in[60]} {data_in[61]} {data_in[62]} {data_in[63]} {data_in[64]} {data_in[65]} {data_in[66]} {data_in[67]} {data_in[68]} {data_in[69]} {data_in[70]} {data_in[71]} {data_in[72]} {data_in[73]} {data_in[74]} {data_in[75]} {data_in[76]} {data_in[77]} {data_in[78]} {data_in[79]} {data_in[80]} {data_in[81]} {data_in[82]} {data_in[83]} {data_in[84]} {data_in[85]} {data_in[86]} {data_in[87]} {data_in[88]} {data_in[89]} {data_in[90]} {data_in[91]} {data_in[92]} {data_in[93]} {data_in[94]} {data_in[95]} {data_in[96]} {data_in[97]} {data_in[98]} {data_in[99]} {data_in[100]} {data_in[101]} {data_in[102]} {data_in[103]} {data_in[104]} {data_in[105]} {data_in[106]} {data_in[107]} {data_in[108]} {data_in[109]} {data_in[110]} {data_in[111]} {data_in[112]} {data_in[113]} {data_in[114]} {data_in[115]} {data_in[116]} {data_in[117]} {data_in[118]} {data_in[119]} {data_in[120]} {data_in[121]} {data_in[122]} {data_in[123]} {data_in[124]} {data_in[125]} {data_in[126]} {data_in[127]} clk nreset data_rdy key_rdy EncDec}
editPin -fixOverlap 1 -unit MICRON -spreadDirection clockwise -side Right -layer 3 -spreadType center -spacing 0.576 -pin {{data_out[0]} {data_out[1]} {data_out[2]} {data_out[3]} {data_out[4]} {data_out[5]} {data_out[6]} {data_out[7]} {data_out[8]} {data_out[9]} {data_out[10]} {data_out[11]} {data_out[12]} {data_out[13]} {data_out[14]} {data_out[15]} {data_out[16]} {data_out[17]} {data_out[18]} {data_out[19]} {data_out[20]} {data_out[21]} {data_out[22]} {data_out[23]} {data_out[24]} {data_out[25]} {data_out[26]} {data_out[27]} {data_out[28]} {data_out[29]} {data_out[30]} {data_out[31]} data_valid key_valid busy {data_out[32]} {data_out[33]} {data_out[34]} {data_out[35]} {data_out[36]} {data_out[37]} {data_out[38]} {data_out[39]} {data_out[40]} {data_out[41]} {data_out[42]} {data_out[43]} {data_out[44]} {data_out[45]} {data_out[46]} {data_out[47]} {data_out[48]} {data_out[49]} {data_out[50]} {data_out[51]} {data_out[52]} {data_out[53]} {data_out[54]} {data_out[55]} {data_out[56]} {data_out[57]} {data_out[58]} {data_out[59]} {data_out[60]} {data_out[61]} {data_out[62]} {data_out[63]} data_valid key_valid busy}
editPin -snap TRACK -pin *
setPinAssignMode -pinEditInBatch false
legalizePin
set sprCreateIeRingOffset 1.0
set sprCreateIeRingThreshold 1.0
set sprCreateIeRingJogDistance 1.0
set sprCreateIeRingLayers {}
set sprCreateIeRingOffset 1.0
set sprCreateIeRingThreshold 1.0
set sprCreateIeRingJogDistance 1.0
set sprCreateIeRingLayers {}
set sprCreateIeStripeWidth 10.0
set sprCreateIeStripeThreshold 1.0
set sprCreateIeStripeWidth 10.0
set sprCreateIeStripeThreshold 1.0
set sprCreateIeRingOffset 1.0
set sprCreateIeRingThreshold 1.0
set sprCreateIeRingJogDistance 1.0
set sprCreateIeRingLayers {}
set sprCreateIeStripeWidth 10.0
set sprCreateIeStripeThreshold 1.0
setAddRingMode -ring_target default -extend_over_row 0 -ignore_rows 0 -avoid_short 0 -skip_crossing_trunks none -stacked_via_top_layer Pad -stacked_via_bottom_layer M1 -via_using_exact_crossover_size 1 -orthogonal_only true -skip_via_on_pin {  standardcell } -skip_via_on_wire_shape {  noshape }
addRing -nets {VDD VSS} -type core_rings -follow core -layer {top M7 bottom M7 left M6 right M6} -width {top 2.176 bottom 2.176 left 2.176 right 2.176} -spacing {top 0.384 bottom 0.384 left 0.384 right 0.384} -offset {top 0.384 bottom 0.384 left 0.384 right 0.384} -center 0 -threshold 0 -jog_distance 0 -snap_wire_center_to_grid None
addStripe -skip_via_on_wire_shape blockring -direction horizontal -set_to_set_distance 2.16 -skip_via_on_pin Standardcell -stacked_via_top_layer M1 -layer M2 -width 0.072 -nets VDD -stacked_via_bottom_layer M1 -start_from bottom -snap_wire_center_to_grid None -start_offset -0.044 -stop_offset -0.044
addStripe -skip_via_on_wire_shape blockring -direction horizontal -set_to_set_distance 2.16 -skip_via_on_pin Standardcell -stacked_via_top_layer M1 -layer M2 -width 0.072 -nets VSS -stacked_via_bottom_layer M1 -start_from bottom -snap_wire_center_to_grid None -start_offset 1.036 -stop_offset -0.044
addStripe -skip_via_on_wire_shape Noshape -set_to_set_distance 12.960 -skip_via_on_pin Standardcell -stacked_via_top_layer Pad -spacing 0.360 -xleft_offset 0.360 -layer M3 -width 0.936 -nets {VDD VSS} -stacked_via_bottom_layer M2 -start_from left
addStripe -skip_via_on_wire_shape Noshape -direction horizontal -set_to_set_distance 21.6 -skip_via_on_pin Standardcell -stacked_via_top_layer M7 -spacing 0.864 -layer M4 -width 0.864 -nets {VDD VSS} -stacked_via_bottom_layer M3 -start_from bottom
setSrouteMode -reset
setSrouteMode -viaConnectToShape { noshape }
sroute -connect { corePin } -layerChangeRange { M1(1) M7(1) } -blockPinTarget { nearestTarget } -floatingStripeTarget { blockring padring ring stripe ringpin blockpin followpin } -deleteExistingRoutes -allowJogging 0 -crossoverViaLayerRange { M1(1) Pad(10) } -nets { VDD VSS } -allowLayerChange 0 -targetViaLayerRange { M1(1) Pad(10) }
editPowerVia -add_vias 1 -orthogonal_only 0
place_opt_design
routeDesign
editPowerVia -delete_vias 1 -top_layer 7 -bottom_layer 6
editPowerVia -delete_vias 1 -top_layer 6 -bottom_layer 5
editPowerVia -delete_vias 1 -top_layer 5 -bottom_layer 4
editPowerVia -delete_vias 1 -top_layer 4 -bottom_layer 3
editPowerVia -delete_vias 1 -top_layer 3 -bottom_layer 2
editPowerVia -delete_vias 1 -top_layer 2 -bottom_layer 1
editPowerVia -add_vias 1
report_timing
setAnalysisMode -analysisType onChipVariation
optDesign -postRoute
report_timing
verify_drc
report_timing > timing.rpt
summaryReport -noHtml -outfile summary.rpt
verify_drc > drc.rpt
saveDesign -tgz ../best_result/best_result.enc

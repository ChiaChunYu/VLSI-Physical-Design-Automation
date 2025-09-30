# ####################################################################

set sdc_version 2.0

# Set the current design
current_design misty

create_clock -name "clk" -period 1843 [get_ports clk]

set_input_delay -clock clk 1  [all_inputs]

set_output_delay -clock clk 925  [all_outputs]



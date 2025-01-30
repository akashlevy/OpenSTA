read_liberty asap7_invbuf.lib.gz
read_liberty ../examples/sky130hd_tt.lib.gz
read_verilog mixed_units.v
link_design mixed_units
create_clock -name clk -period 10
set_input_delay -clock clk 0 [all_inputs -no_clocks]
set_output_delay -clock clk 0 [all_outputs]
report_checks -fields {capacitance slew input_pin} -endpoint_path_count 10000 -group_path_count 10000

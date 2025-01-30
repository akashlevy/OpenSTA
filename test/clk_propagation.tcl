set sta_propagate_all_clocks 1
# set sta_propagate_gated_clock_enable 1
read_liberty asap7_invbuf.lib.gz
read_liberty asap7_seq.lib.gz
read_verilog clk_propagation.v
link_design clk_propagation
create_clock -period 100 -name clk {clk}
set_input_delay -clock clk 0 [all_inputs -no_clocks]
set_output_delay -clock clk 0 [all_outputs]
report_checks

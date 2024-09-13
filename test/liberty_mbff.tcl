read_liberty liberty_mbff.lib
read_verilog liberty_mbff.v
link_design liberty_mbff
create_clock -name clk -period 0 clk
puts [all_registers]
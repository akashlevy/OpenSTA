# unit_noinvbuf reg1 asap7
read_liberty asap7_invbuf.lib.gz
read_liberty asap7_seq.lib.gz
read_liberty asap7_simple.lib.gz
read_verilog reg1_asap7.v
link_design top
create_clock -name clk -period 500 {clk1 clk2 clk3}
sta::set_delay_calculator unit_noinvbuf
report_checks -fields {input_pins slew} -format full_clock -through u1

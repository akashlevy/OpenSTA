// Liberty file test: multi-bit flip-flop (MBFF)
module liberty_mbff (
  input wire clk,
  input wire scan_en,
  input wire scan_d,
  input wire [3:0] d,
  output wire [3:0] q1,
  output wire [3:0] q2,
  output wire [3:0] q3,
  output wire [1:0] q4
);

  MBFF mbff (
    .clk(clk),
    .scan_en(scan_en),
    .scan_d(scan_d),
    .d(d),
    .q(q1)
  );

  MBFF_simple mbff_simple (
    .clk(clk),
    .d(d),
    .q(q2)
  );

  shift_reg_ff shift_reg_ff_inst (
    .D(d[0]),
    .CP(clk),
    .Q0(q3[0]),
    .Q1(q3[1]),
    .Q2(q3[2]),
    .Q3(q3[3])
  );

  counter_ff counter_ff_inst (
    .reset(d[0]),
    .CP(clk),
    .Q0(q4[0]),
    .Q1(q4[1])
  );

endmodule
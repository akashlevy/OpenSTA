// Liberty file test: multi-bit flip-flop (MBFF)
module liberty_mbff (
  input wire clk,
  input wire scan_en,
  input wire scan_d,
  input wire [3:0] d,
  output wire [3:0] q,
  output wire [3:0] q2
);

  MBFF mbff (
    .clk(clk),
    .scan_en(scan_en),
    .scan_d(scan_d),
    .d(d),
    .q(q)
  );

  MBFF_simple mbff_simple (
    .clk(clk),
    .d(d),
    .q(q2)
  );

endmodule
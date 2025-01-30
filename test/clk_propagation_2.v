module clk_propagation(
	input  wire clk,
	input  wire data,
	output wire data_out
);

	wire clk_inv;
	wire clk_inv_inv;

	// Invert clock
	CKINVDCx8_ASAP7_75t_R inv_clk (
		.A(clk),
		.Y(clk_inv)
	);
	
	// Invert clock again
	CKINVDCx8_ASAP7_75t_R inv_clk (
		.A(clk_inv),
		.Y(clk_inv_inv)
	);

	// Posedge DFF uses inverted clock for negedge functionality
	DFFHQx4_ASAP7_75t_R dff_negedge (
		.CLK(clk_inv_inv),
		.D(data),
		.Q(data_out)
	);

endmodule
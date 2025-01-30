module mixed_units (
	input  wire inA,
	input  wire inB,
	output wire outY
);

wire net_1;

// Example ASAP7 inverter
INVx1_ASAP7_75t_R invA (
	.A (inA),
	.Y (net_1)
);

// Example SKY130 2-input AND
sky130_fd_sc_hd__and2_1 andB (
	.A (net_1),
	.B (inB),
	.X (outY)
);

endmodule
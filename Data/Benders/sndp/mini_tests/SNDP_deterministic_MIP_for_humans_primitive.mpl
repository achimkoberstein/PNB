TITLE
	SNDP_deterministic_MIP_for_humans_simple;

VARIABLES
	x_c24;
	x_c34;
	x_a12;
	x_b12;
	x_a13;
	x_b13;
	x_a32;
	x_a33;

BINARY VARIABLES
	yc2;
	yc3;

MAX
	13*(x_c24 + x_c34) - 2*x_a12 - 2*x_b12 - 2*x_a13 - 2*x_b13 - 1*x_a32 - 2*x_c24 - 3*x_c34 - 2000*yc2 - 2000*yc3;

SUBJECT TO
	x_a12 + x_a32 = 2*x_c24;
	x_a13 + x_a33 = 2*x_c34;
	x_b12 = 3*x_c24;
	x_b13 = 3*x_c34;
	x_c24 + x_c34 <= 5000;
	x_c24 <= 5000*yc2;
	x_c34 <= 5000*yc3

BOUNDS
	yc2 <= 1;
	yc3 <= 1;

END
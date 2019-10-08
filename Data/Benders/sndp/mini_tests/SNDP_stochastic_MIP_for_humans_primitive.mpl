TITLE
	SNDP_deterministic_MIP_for_humans_primitive;

STOCHASTIC

SCENARIO
	SCEN := 1..3;

PROBABILITIES
	Prob[SCEN] := (0.25, 0.5, 0.25);

RANDOM DATA
	Demand[SCEN] := (2000, 5000, 8000);

STAGE1 BINARY VARIABLES
	yc2;
	yc3;

STAGE2 VARIABLES
	x_c24;
	x_c34;
	x_a12;
	x_b12;
	x_a13;
	x_b13;
	x_a32;
	x_a33;

MIN	! should be min for solver
	13*(x_c24 + x_c34) - 2*x_a12 - 2*x_b12 - 2*x_a13 - 2*x_b13 - 1*x_a32 - 2*x_c24 - 3*x_c34 - 2000*yc2 - 2000*yc3;

SUBJECT TO
	x_a12 + x_a32 = 2*x_c24;
	x_a13 + x_a33 = 2*x_c34;
	x_b12 = 3*x_c24;
	x_b13 = 3*x_c34;
	x_c24 + x_c34 <= Demand;
	x_c24 <= 5000*yc2;
	x_c34 <= 5000*yc3

END
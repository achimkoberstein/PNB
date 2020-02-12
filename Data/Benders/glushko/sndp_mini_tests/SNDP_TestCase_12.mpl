!Properties from TestCase_0 that are present in this test case:
! - no matrix A
! - only bin vars on the first stage
! - standard bounds for 1st stage cont vars (if any)
! - only vector rhs is stochastic

!Properties specific for this TestCase:
! feasibility cuts - assume that according to the spedition contract at least 100 items of product C should be transported total on arcs 24 and 34

!These properties of are present in all TestCases:
! - MIN obj - always the case for smps files
! - 2 stages - only such problems are in scope of the research
! - only cont vars on the 2nd stage - we will solve the relaxation otherwise
! - >= constraints in subproblems - we do not care about subproblems
! - bin and cont (if any) vars on the first - int vars are prohibited 

TITLE
	SNDP_TestCase_12;

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
	! now includes costs for prebooked transport
	- 13*(x_c24 + x_c34) + 2*x_a12 + 2*x_b12 + 2*x_a13 + 2*x_b13 + 1*x_a32 + 2*x_c24 + 3*x_c34 + 2000*yc2 + 2000*yc3;

SUBJECT TO
	x_a12 + x_a32 >= 2*x_c24;
	-x_a12 + -x_a32 >= -2*x_c24;

	x_a13 + x_a33 >= 2*x_c34;
	-x_a13 + -x_a33 >= -2*x_c34;

	x_b12 >= 3*x_c24;
	-x_b12 >= -3*x_c24;

	x_b13 >= 3*x_c34;
	-x_b13 >= -3*x_c34;

	! normal signs in this constraints
	x_c24 + x_c34 <= Demand;
	x_c24 <= 5000*yc2;
	x_c34 <= 5000*yc3;

	! new constraints of minimum transportation amount
	x_c24 + x_c34 >= 100;

END
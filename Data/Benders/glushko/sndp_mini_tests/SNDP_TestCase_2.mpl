!Properties from TestCase_0 that are present in this test case:
! - no matrix A
! - only bin vars on the first stage
! - standard bounds for 1st stage cont vars (if any)
! - only vector rhs is stochastic

!Properties specific for this TestCase:
! - 3 bin var (+1 in comparison to Test_Case_0). Imagine that we can also open the facility in node 1

!These properties of are present in all TestCases:
! - MIN obj - always the case for smps files
! - 2 stages - only such problems are in scope of the research
! - only cont vars on the 2nd stage - we will solve the relaxation otherwise
! - >= constraints in subproblems - we do not care about subproblems
! - bin and cont (if any) vars on the first - int vars are prohibited 

TITLE
	SNDP_TestCase_2;

STOCHASTIC

SCENARIO
	SCEN := 1..3;

PROBABILITIES
	Prob[SCEN] := (0.25, 0.5, 0.25);

RANDOM DATA
	Demand[SCEN] := (2000, 5000, 8000);

STAGE1 BINARY VARIABLES
	yc1;	! was not present in the original problem
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
	x_c14;	! now we can delivery from node 1 to node 4
	x_a11;	! and we can use available resources
	x_b11;

MIN	! should be min for solver, was updated
	- 13*(x_c24 + x_c34 + x_c14) + 2*x_a12 + 2*x_b12 + 2*x_a13 + 2*x_b13 + 1*x_a32 + 2*x_c24 + 3*x_c34 + 2000*yc2 + 2000*yc3 + 2000*yc1;
	!- 13*(x_c24 + x_c34) + 2*x_a12 + 2*x_b12 + 2*x_a13 + 2*x_b13 + 1*x_a32 + 2*x_c24 + 3*x_c34 + 2000*yc2 + 2000*yc3;

SUBJECT TO
	x_a12 + x_a32 >= 2*x_c24;
	-x_a12 + -x_a32 >= -2*x_c24;

	x_a13 + x_a33 >= 2*x_c34;
	-x_a13 + -x_a33 >= -2*x_c34;

	x_a11 >= 2*x_c14;
	-x_a11 >= -2*x_c14;	! new pair of constraints

	x_b12 >= 3*x_c24;
	-x_b12 >= -3*x_c24;

	x_b13 >= 3*x_c34;
	-x_b13 >= -3*x_c34;

	x_b11 >= 3*x_c14;
	-x_b11 >= -3*x_c14;	! new pair of constraints

	-x_c14 - x_c24 - x_c34 >= -Demand;	! one more var in demand constraint
	-x_c14 >= -5000*yc1;	! new constraint
	-x_c24 >= -5000*yc2;
	-x_c34 >= -5000*yc3;

END
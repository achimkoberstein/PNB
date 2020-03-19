!Properties from TestCase_0 that are present in this test case:
! - only bin vars on the first stage
! - standard bounds for 1st stage cont vars (if any)

!Properties specific for this TestCase:
! - matrix A is not empty - assume that there is a budget for opening the production locations
! of 3500 Euro (hence, both locations CANNOT be opened simultaniously. In the original problem they do) 
! - vector rhs and objective are stochastic: TransportationPrice_b13 is stochastic now
! - feasibility cuts - assume that according to the spedition contract at least 100 items of product C should be transported total on arcs 24 and 34
! - MIP (bin and cont) vars on the 2nd stage - we will solve the relaxation
! - bin 2nd stage variable controls wheter route 3->4 is used and fee is payed

!These properties of are present in all TestCases:
! - MIN obj - always the case for smps files
! - 2 stages - only such problems are in scope of the research
! - >= constraints in subproblems - we do not care about subproblems
! - bin and cont (if any) vars on the first - int vars are prohibited

! Solution value: -10750
! Solution value  2nd stage relaxation: -10750 (the same)


TITLE
	SNDP_TestCase_15;

STOCHASTIC

SCENARIO
	SCEN := 1..3;

PROBABILITIES
	Prob[SCEN] := (0.25, 0.5, 0.25);

RANDOM DATA
	Demand[SCEN] := (2000, 5000, 8000);
	TransportationPrice_b13[SCEN] := (1, 3, 5);		! in the original problems it is deterministic with the value 2

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

STAGE2 BINARY VARIABLES
	z_c34;		! - 1 - if product c transported 3->4 and we pay the fee of 100, 0 - otherwise

MIN	! should be min for solver
	! now includes costs for prebooked transport and cost of 3->4
	- 13*(x_c24 + x_c34) + 2*x_a12 + 2*x_b12 + 2*x_a13 + TransportationPrice_b13*x_b13 + 1*x_a32 + 2*x_c24 + 3*x_c34 + 2000*yc2 + 2000*yc3 + z_c34*100;

SUBJECT TO
	! new budget constraint
	2000*yc2 +2000*yc3 <= 3500;

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

	! product c must be transported on 3->4
	x_c34 <= z_c34*10000;

END
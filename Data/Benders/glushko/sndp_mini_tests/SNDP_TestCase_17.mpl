!Properties from TestCase_0 that are present in this test case:
! - standard bounds for 1st stage cont vars (if any)
! - only vector rhs is stochastic

!Properties specific for this TestCase:
! matrix A is not empty - we have smth like a budget that we need definitely use: == transformed to two constraints: <= and <=
! assume we need to prebook transport (capacity k_c24 and k_c34) in the 1st stage for the routes: xc24 and xc34. Prebooking tranport cost money
! mixed-binary 1st stage
! mixed-binary 2nd stage

!These properties of are present in all TestCases:
! - MIN obj - always the case for smps files
! - 2 stages - only such problems are in scope of the research
! - only cont vars on the 2nd stage - we will solve the relaxation otherwise
! - >= constraints in subproblems - we do not care about subproblems
! - bin and cont (if any) vars on the first - int vars are prohibited

! Solution value: -900
! Solution value 2nd stage relaxation: -985
! Solution value 1st and 2nd stage relaxation: -3229.375

TITLE
	SNDP_TestCase_17;

STOCHASTIC

SCENARIO
	SCEN := 1..3;

PROBABILITIES
	Prob[SCEN] := (0.25, 0.5, 0.25);

RANDOM DATA
	Demand[SCEN] := (2000, 5000, 8000);

STAGE1 VARIABLES    ! this is the transport (capacity) which we want to prebook
	k_c24;
	k_c34;

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
	- 13*(x_c24 + x_c34) + 2*x_a12 + 2*x_b12 + 2*x_a13 + 2*x_b13 + 1*x_a32 + 2*x_c24 + 3*x_c34 + 2000*yc2 + 2000*yc3 + 2*(k_c24 + k_c34) + z_c34*100;

SUBJECT TO
	! new constraints: both locations should be opened
	2*(k_c24 + k_c34) + 2000*yc2 +2000*yc3 <= 5000;
	-2*(k_c24 + k_c34) + -2000*yc2 -2000*yc3 <= -5000;

	x_a12 + x_a32 >= 2*x_c24;
	-x_a12 + -x_a32 >= -2*x_c24;

	x_a13 + x_a33 >= 2*x_c34;
	-x_a13 + -x_a33 >= -2*x_c34;

	x_b12 >= 3*x_c24;
	-x_b12 >= -3*x_c24;

	x_b13 >= 3*x_c34;
	-x_b13 >= -3*x_c34;

	-x_c24 + -x_c34 >= -Demand;
	-x_c24 >= -5000*yc2;
	-x_c34 >= -5000*yc3;

	! new constraints related to the prebooked transport on stage 1
	-x_c24 >= -k_c24;
	-x_c34 >= -k_c34;

	! product c must be transported on 3->4
	x_c34 <= z_c34*10000;

!BOUNDS
!	0 <= yc2 <= 1;
!	0 <= yc3 <= 1;
!	0 <= z_c34 <= 1;

END
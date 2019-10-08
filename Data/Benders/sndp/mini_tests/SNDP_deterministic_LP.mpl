TITLE
	SNDP_deterministic_LP;

INDEX
	VAR_INDEX := (_A12, _B12, _A13, _B13, _A32, _A33, _C24, _C34) : 4;
	BIN_VAR_INDEX := (_C2, _C3) : 3;
	CONSTR_INDEX := 1..11;

DATA
	Cost[VAR_INDEX] := (2, 2, 2, 2, 1, 0, -11, -10);		!vector qT, transportation costs, selling price (signs are reverted in order to transform to MIN problem)
	Fixed_Cost[BIN_VAR_INDEX] := (2000, 2000);			!vector c, set up costs (signs are reverted in order to transform to MIN problem)
	T[CONSTR_INDEX, BIN_VAR_INDEX] := (	0,		0
						0,		0
						0,		0
						0,		0
						0,		0
						0,		0
						0,		0
						0,		0
						0,		0
						5000,		0
						0,		5000);	!matrix T (technological in 2 stage model)
	W[CONSTR_INDEX, VAR_INDEX] := (	1, 0, 0, 0, 1, 0, -2, 0,
					-1, 0, 0, 0, -1, 0, 2, 0,
					0, 0, 1, 0, 0, 1, 0, -2,
					0, 0, -1, 0, 0, -1, 0, 2,
					0, 1, 0, 0, 0, 0, -3, 0,
					0, -1, 0, 0, 0, 0, 3, 0,
					0, 0, 0, 1, 0, 0, 0, -3,
					0, 0, 0, -1, 0, 0, 0, 3,
					0, 0, 0, 0, 0, 0, -1, -1,
					0, 0, 0, 0, 0, 0, -1, 0,
					0, 0, 0, 0, 0, 0, 0, -1);	!matrix W (recourse in 2 stage model)
	!H := (-2000-5000-8000)/3;
	h[CONSTR_INDEX] := (0, 0, 0, 0, 0, 0, 0, 0, -8000, 0, 0);

VARIABLES
	x[VAR_INDEX];
	y[BIN_VAR_INDEX];	

!BINARY VARIABLES
!	y[BIN_VAR_INDEX];

MACROS
	Master = sum(BIN_VAR_INDEX: Fixed_Cost * y);
	Subproblem = sum(VAR_INDEX: Cost * x);

MIN									!MAX was transformed to MIN
	Objective = Master + Subproblem;

SUBJECT TO
    	Constr[CONSTR_INDEX]: sum(VAR_INDEX: W * x) >= h - sum(BIN_VAR_INDEX: T * y);

BOUNDS
	0<=y<=1;

END
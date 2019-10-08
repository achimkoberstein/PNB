TITLE
    
    DakotaModel_implicit;
    ! implicit formulation of the stochastic model (with special MPL syntax)
    ! Objective value -1730.0
    ! MAX objective function changed to MIN

STOCHASTIC

SCENARIO
    
    SCEN := 1..3;

INDEX

    PRODUCTS := (Desk, Table, Chair);
    RESOURCES := (Lumber, Finishing, Carpentry);

PROBABILITIES
    
    Prob[SCEN] := (0.3, 0.4, 0.3);

RANDOM DATA

    Demand[PRODUCTS, SCEN] := (50, 150, 250
                               20, 110, 250
                               200, 225, 500);

DATA

    Price[PRODUCTS] := (60, 40, 10);
    Cost[RESOURCES] := (2, 4, 5.2);
    Consumption[RESOURCES, PRODUCTS] := (8, 6, 1
					4, 2, 1.5
					2, 1.5, .5);

STAGE1 VARIABLES

   Purchase[RESOURCES];

STAGE2 VARIABLES

   Production[PRODUCTS];

MIN

    Profit = -(sum(PRODUCTS: Price * Production) - sum(RESOURCES: Cost * Purchase));

SUBJECT TO

    ResourceConstraint[RESOURCES]: sum(PRODUCTS: Consumption * Production) <= Purchase;
    DemandConstraint[PRODUCTS]: Production <= Demand;

END
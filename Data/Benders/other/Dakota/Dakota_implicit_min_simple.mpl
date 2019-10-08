TITLE
    
    DakotaModel_implicit_simple;
    ! implicit formulation of the stochastic model (with special MPL syntax)
    ! Objective value -1800.0
    ! MAX objective function changed to MIN
    ! 2 instead of 3 scenarios, products, and resources
    
STOCHASTIC

SCENARIO
    
    SCEN := 1..2;

INDEX

    PRODUCTS := (Desk, Table);
    RESOURCES := (Lumber, Finishing);

PROBABILITIES
    
    Prob[SCEN] := (0.6, 0.4);

RANDOM DATA

    Demand[PRODUCTS, SCEN] := (50, 150
                               20, 110);

DATA

    Price[PRODUCTS] := (60, 40);
    Cost[RESOURCES] := (2, 4);
    Consumption[RESOURCES, PRODUCTS] := (8, 6
					4, 2);

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
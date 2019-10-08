TITLE
    
    DakotaModel_implicit_simple_x_constrained;
    ! implicit formulation of the stochastic model (with special MPL syntax)
    ! Objective value -2320.0
    ! MAX objective function changed to MIN
    ! 2 instead of 3 scenarios, products, and resources
    ! 1st stage resource constraint added

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

    ResourceUB[RESOURCES] := (3500, 1500);   ! not present in original problem, but needed for Excel. Does not change optimal solution
    Price[PRODUCTS] := (-70, -40);         ! in original problem it was 60, 40
    Cost[RESOURCES] := (2, 4);
    Consumption[RESOURCES, PRODUCTS] := (8, 6
					4, 2);

STAGE1 VARIABLES

   Purchase[RESOURCES];

STAGE2 VARIABLES

   Production[PRODUCTS];

MACRO

   PurchaseCosts = sum(RESOURCES: Cost * Purchase);
   Sales = sum(PRODUCTS: Price * Production)

MIN

    !Profit = sum(RESOURCES: Cost * Purchase) + sum(PRODUCTS: Price * Production);
    Profit = PurchaseCosts + Sales;

SUBJECT TO       ! during MPS generation it is important to keep the order of everythin: 1st stage, 2nd stage

    ResourceUBConstraint[RESOURCES]: Purchase <= ResourceUB;   ! not present in original problem
    ResourceConstraint[RESOURCES]: sum(PRODUCTS: Consumption * Production) <= Purchase;
    DemandConstraint[PRODUCTS]: Production <= Demand;

END
TITLE
    
    DakotaModel_explicit_simple;
    ! explicit formulation of the stochastic model (w/o special MPL syntax)
    ! Objective value -2320.0
    ! MAX objective function changed to MIN
    ! 2 instead of 3 scenarios, products, and resources

INDEX

    SCEN := 1..2;
    PRODUCTS := (Desk, Table);
    RESOURCES := (Lumber, Finishing);

DATA

    Price[PRODUCTS] := (-70, -40);			!signes are reverted to have MIN problems for 1st and 2nd stage
    Cost[RESOURCES] := (2, 4);
    Consumption[RESOURCES, PRODUCTS] := (8, 6
					4, 2);
    Demand[PRODUCTS, SCEN] := (50, 150
				20, 110);
    Prob[SCEN] := (.6, .4);

VARIABLES

   Purchase[RESOURCES];
   Production[PRODUCTS, SCEN];

MACROS

  PurchaseCosts = sum(RESOURCES: Cost * Purchase);            !1st stage
  Sales = sum(PRODUCTS, SCEN: Prob * Price * Production);     !2nd stage

MIN

    Profit = PurchaseCosts + Sales;

SUBJECT TO

    ResourceConstraint[RESOURCES, SCEN]: sum(PRODUCTS: Consumption * Production) <= Purchase;
    DemandConstraint[PRODUCTS, SCEN]: Production <= Demand;

END
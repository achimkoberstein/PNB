TITLE
    
    DakotaModel_deterministic_simple;
    ! Objective value -4540.0
    ! MAX objective function changed to MIN
    ! 2 instead of 3 scenarios, products, and resources

INDEX

    PRODUCTS := (Desk, Table);
    RESOURCES := (Lumber, Finishing);

DATA

    Price[PRODUCTS] := (-70, -40);			!signes are reverted to have MIN problems for 1st and 2nd stage
    Cost[RESOURCES] := (2, 4);
    Consumption[RESOURCES, PRODUCTS] := (8, 6
					4, 2);
    Demand[PRODUCTS] := (90, 56);			!expected demand

VARIABLES

   Purchase[RESOURCES];
   Production[PRODUCTS];

MACROS

  PurchaseCosts = sum(RESOURCES: Cost * Purchase);     !1st stage
  Sales = sum(PRODUCTS: Price * Production);           !2nd stage

MIN                                                   !MIN was transformed to MAX

    Profit = PurchaseCosts + Sales;

SUBJECT TO

    ResourceConstraint[RESOURCES]: sum(PRODUCTS: Consumption * Production) <= Purchase;
    DemandConstraint[PRODUCTS]: Production <= Demand;

END
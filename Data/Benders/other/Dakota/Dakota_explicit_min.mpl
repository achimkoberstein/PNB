TITLE
    
    DakotaModel_explicit;
    ! explicit formulation of the stochastic model (w/o special MPL syntax)
    ! Objective value -1730.0
    ! MAX objective function changed to MIN

INDEX

    SCEN := 1..3;
    PRODUCTS := (Desk, Table, Chair);
    RESOURCES := (Lumber, Finishing, Carpentry);

DATA

    Price[PRODUCTS] := (60, 40, 10);
    Cost[RESOURCES] := (2, 4, 5.2);
    Consumption[RESOURCES, PRODUCTS] := (8, 6, 1
					4, 2, 1.5
					2, 1.5, .5);
    Demand[PRODUCTS, SCEN] := (50, 150, 250
				20, 110, 250
				200, 225, 500);
    Prob[SCEN] := (.3, .4, .3);

VARIABLES

   Purchase[RESOURCES];
   Production[PRODUCTS, SCEN];

MIN

    Profit = -(sum(SCEN, PRODUCTS: Prob (Price * Production)) - sum(RESOURCES: Cost * Purchase));

SUBJECT TO

    ResourceConstraint[RESOURCES, SCEN]: sum(PRODUCTS: Consumption * Production) <= Purchase;
    DemandConstraint[PRODUCTS, SCEN]: Production <= Demand;

END
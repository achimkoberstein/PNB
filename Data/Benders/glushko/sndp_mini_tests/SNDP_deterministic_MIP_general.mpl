TITLE
	SNDP_deterministic_general

DATA
	NrOfLocations := 4;	! last location is a final location (market)
	NrOfProducts := 3;	! last product is an end product
	SalesPrice := 13;
	Demand	:= 5000;
	PlantCost := 2000;
	PlantCapacity := 5000;
	M := 10000000;

INDEX
	location := 1..NrOfLocations;
	start := location;
	finish := location;
	product := 1..NrOfProducts;
	material[product] WHERE (product <= NrOfProducts-1); 	! all except the last one are materials
	route[start, finish] := 
		1, 1, 0,	! production cost
		1, 2, 2,
		1, 3, 2,
		1, 4, M,
		2, 1, M,
		2, 2, 0,	! production cost
		2, 3, M,
		2, 4, 2,
		3, 1, M,
		3, 2, 1,
		3, 3, 0,	! production cost
		3, 4, 3,
		4, 1, M,
		4, 2, M,
		4, 3, M,
		4, 4, M];

DATA
	ShipCost[start, finish] := [ ! make it really sparse
		1, 1, 0,	! production cost
		1, 2, 2,
		1, 3, 2,
		1, 4, M,
		2, 1, M,
		2, 2, 0,	! production cost
		2, 3, M,
		2, 4, 2,
		3, 1, M,
		3, 2, 1,
		3, 3, 0,	! production cost
		3, 4, 3,
		4, 1, M,
		4, 2, M,
		4, 3, M,
		4, 4, M];
	Capacity[product, location] := [
		1, 1, M,
		1, 3, M,
		2, 1, M,
		3, 2, PlantCapacity,
		3, 3, PlantCapacity];
	MaterialReq[material] := (2, 3);

DECISION VARIABLES
	Ship[product,start,finish] WHERE (ShipCost[start, finish] < M-1);

BINARY VARIABLES
	OpenProduction[location] WHERE (Capacity[product:=NrOfProducts, location] > 0);

MACROS
	TotalRevenue := SUM(product=NrOfProducts, start, finish=NrOfLocations:  SalesPrice * Ship);
	FixedCost := SUM(location: PlantCost * OpenProduction);
	TotalShipCost := SUM(product,start,finish: ShipCost * Ship);

MAX
	Profit = TotalRevenue - FixedCost - TotalShipCost;

SUBJECT TO
	ProductionConstr[product,location] -> ProdConstr:
		Ship[product,start:=location,finish:=location] <= Capacity[product,location];

	!BalanceConstr[product,location] -> BalConstr:
	!	SUM(finish<>location: Ship[product,start:=location,finish]) <= SUM(start: Ship[product,start,finish:=location]); ! output <= input + production
	
	BalanceConstr[product,location] -> BalConstr:
		SUM(finish<>location: Ship[product,start:=location,finish]) <= Ship[product,start:=location,finish:=location]; ! output <= production (no transhipment via nodes)

	DemandConstr[product=NrOfProducts, finish=NrOfLocations] -> DemConstr: SUM(product=NrOfProducts,start,finish=NrOfLocations: Ship) <= Demand;
	
	BOMConstr[location, material] WHERE (Capacity[product:=NrOfProducts, location] > 0):
		MaterialReq[material] * Ship[product:=NrOfProducts, start:=location, finish:=location] = SUM(start: Ship[product:=material, start, finish:=location]);
	
	PlantConstr[location] WHERE (Capacity[product:=NrOfProducts, location] > 0):
		Ship[product:=NrOfProducts, start:=location, finish:=location] <= Capacity[product:=NrOfProducts, location] * OpenProduction[location];

	
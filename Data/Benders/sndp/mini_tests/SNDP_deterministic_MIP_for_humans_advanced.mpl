TITLE
	SNDP_deterministic_MIP_for_humans;

INDEX
	Product := (A, B, C);
	Resource[Product] := (A, B);
	EndProduct[Product] := (C); 
	
	Location := (1, 2, 3, 4);
	PlantLocation(Location) := (2, 3);
	From := Location;
	To := Location;

	Route[From, To] := ((1, 2),
						(1, 3),
						(3, 2), 
						(2, 4), 
						(3, 4));

DATA
	Cost[Route] := (2, 2, 1, 2, 3);
	FixedCost[PlantLocation] := (2000, 2000);
	Req[Resource, EndProduct] := (2,
								  3);
	Price[EndProduct] := (13);
	Demand[Location] := (0, 0, 0, 5000);

VARIABLES
	x[Product, Route];	! transportation

BINARY VARIABLES
	y[PlantLocation];	! build the plant

MACROS
	FixedCost = sum(PlantLocation: FixedCost * y);
	TransportCost = sum(Product, Route: Cost * x);
	Sales = sum(EndProduct, Route:  Price * x);			!NB: the end product can be transported to the final destination only

	Objective = Sales - TransportCost - FixedCost;

SUBJECT TO
    	ProdReq[Route]: sum(Route: x) == 2 * sum(Route: x);

END
TITLE
  LSFRP_Instance_30

STOCHASTIC

SCENARIO
   sc2:= 1..100;

INDEX
  i := INDEXFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Nodes30.dat");				# visitations
  i2 := i;								# node index
  j := i;       							# node index
  k := i;
  Orig[i];								# Origins for func index O
  Dest[i];								# Destinations for func index D
  a[i, j] := INDEXFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/ArcsArranged30.dat");  		# arc index
  s := INDEXFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Ships30.dat");				#ship index	
  d:=INDEXFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/DemandIndex30.dat");			#Demands
  q:=INDEXFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Equipment30.dat");			#Set of Equipment 
  dvis[i2, d] := INDEXFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/mVis30.dat");

PROBABILITIES
  Prob[sc2]:= ALLEQUAL;  

RANDOM DATA
  delayy_temp[i,j IN a,sc2] := SPARSEFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/com_del_voronoirandom30.dat");
  Amountdemand_temp[d,sc2]:=SPARSEFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/com_dem_voronoirandom30.dat");  

  delayy[i,j IN a,sc2] := delayy_temp[i,j IN a,sc2] + 1e-10;
  Amountdemand[d,sc2]:= Amountdemand_temp[d,sc2];

 
DATA
  M1 := sum(i,j IN a: ABS(delayy)) + 100;
  !M1 := 1000000;
  !M2 := 1000000;
  M2 := sum(i,j IN a: ABS(delayy)) + 100;

  !Amountdemand[d]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/AmountDemand30.dat");
  O[s] := DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/OrigShip30.dat");			#origin of ships
  D[s] := 0;								#destination of ships	
  OriginDemand[d]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/OriginDemand30.dat");
  DestinationDemand[d]:= DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/DestinationDemand30.dat");
  Demandtype[d]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/TypeDemand30.dat");
  EqDemandType[q]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/EquipmentDemandType30.dat");

  EqOrigin[q]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/EquipmentSurplus30.dat");
  EqDestination[q]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/EquipmentDeficit30.dat");

  cPort[s,i]:=	DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/PortFee30.dat");		#port fees
  cSail[s,a]:= DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/SailCostArranged30.dat");  	#fixed sailing cost
  totalFixedCost[s,a]:= cSail+cPort[s,i:=a.j];

  !cSailVar[s,a]:= DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/VariableCostArranged30.dat");	#variable Sailing Cost	
  cSailVar[i,j IN a, s]:= SPARSEFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/VarBunker30.dat");	#variable Sailing Cost														
  !dmin[s,a]:=	DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/MinDurationArranged30.dat");
  !dmax[s,a]:= 	DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/MaxDurationArranged30.dat"); 
  !dminn[s,i,j] := 0;
  sink:=0;  	
  af[a]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/FlexArcsArranged30.dat");			#Arc Flexible or inflexible?
  i2f[i]:= DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/i2f30.dat");			#Node flexible or inflexible?									
  udc[s] := DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/CapacityDry30.dat");		# ship capacity dry containers
  urf[s] := DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/CapacityReefer30.dat");		# ship capacity reefer containers
 ! tEnter[i]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Entertime30.dat");
 ! tExit[i]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Exittime30.dat");
 ! tPilot[i]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Pilottime30.dat");
 ! tMv[s,i]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Movetime30.dat");
  r[d] := DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/RevenueDemand30.dat");		#Revenue per TEU cargo
  cmvO[d]:= DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/MoveCostOrigin30.dat");
  cmvD[d]:= DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/MoveCostDestination30.dat");	# move cost per TEU per visitation
  NetRevenue[d]:= r - cmvO - cmvD ;					#Revenue per TEU minus Move Cost
  rEquipment[q]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/EquipmentRevenue30.dat");
  cmvEqO[q]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/MoveCostOriginEquipment30.dat");
  cmvEqD[q]:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/MoveCostDestinationEquipment30.dat");
  NetRevenueEq[q]:= rEquipment - cmvEqO - cmvEqD ;
  M:=DATAFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/M30.dat");
  penalty[i] := SPARSEFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/Penalty30.dat");
  dDiff[i,j IN a, s]:=SPARSEFILE("C:/Users/achim/Documents/ViaCloud/SMPS/generatedFiles/DDiff30.dat");


STAGE1 VARIABLES
BINARY VARIABLES
  yy[s,i,j IN a];

STAGE2 VARIABLES
  x[d,s];		# shipping amount Cargo
  xEq[q,i,j IN a];   		# Shipping Amount Equipment		
  de[i,j IN a];		# Delay on Arc
  st[i,j IN a, s];	# Time Saved by vessel by sailing faster than min possible speed	

 
MODEL
  MIN Total_Revenue = -(SUM(d,s: NetRevenue * x) + SUM(q,i: NetRevenueEq * xEq[i,j:=EqDestination[q]])
             - SUM(s,i,j IN a: cSailVar * st)- SUM(i,j IN a: penalty[i:=j] * de)
             - SUM(s,a: totalFixedCost * yy[s,i:=a.i,j:=a.j]));

SUBJECT TO

  OneShipPerNode[j<>sink]: 
	SUM(s,i IN a : yy[s,i,j]) <= 1;  

  AllShipsLeaveStart[s,i=O]:
	SUM(j IN a: yy[s,i,j])=1;	

  AllShipsArriveatSink[s,j=D]:                       
	SUM(i IN a: yy[s,i,j])=1;

  FlowofShips[s,i2]:                                
	SUM(i IN a[j:=i2]: yy[s,i,j:=i2]) - SUM(j IN a[i:=i2]: yy[s,i:=i2,j]) = IIF(i2=O,-1,IIF(i2=D,1,0));

  FlowofEquipment[q,i2<>sink] WHERE i2<> EqOrigin AND i2<>EqDestination:
	SUM(i IN a[j:=i2]: xEq[i,j:=i2]) - SUM(j IN a[i:=i2]: xEq[i:=i2,j]) =0;

  !ShippingAmountLeavesNodes[d,s]:
  ShAmLeNo[d,s]:
	 x <= Amountdemand * SUM(j: yy[i:=OriginDemand[d],j]);

  !ShippingAmountArrivesNodes[d,s]:
  ShAmArNo[d,s]:
	 x <= Amountdemand * SUM(i: yy[i, j:=DestinationDemand[d]]);

  SmallerDemand[d,s]:
     x<=Amountdemand ;

  CapacityDry[s,i2<>sink]:  	
	SUM(d IN dvis: x[d,s]) + SUM(q, i in a[j:=i2]: xEq[q,i,j:=i2])<= udc;

  EquipmentRestriction[i,j IN a]:
    SUM(q: xEq[i,j]) <= SUM(s: udc * yy[s,i,j]); 

  SetDelay[i,j IN a, k IN a[i:=k,j:=i]]:
	(de[i:=k,j:=i]) - SUM(s: st[i,j])
	+ SUM(s: delayy * yy) <= de + (M1 * (1-SUM(s: yy[i:=k,j:=i]))) + (M1 * (1-SUM(s: yy[i,j])));

 SetDelayStartPoint[s,i=O[s],j IN a]:
	yy * delayy - st <= de; 
	!yy*(1-M1) * delayy - st <= de; 


!  MaxTimeSaving[i,j IN a, s]:         !!!!!!!! Achtung, gehört eigentlich ins Modell, wg. Workaround abgeschaltet
!    st<=dDiff * yy;

  MaxDelay[i,j IN a]:
    de<= SUM (s: yy * M2);
  
BOUNDS
  x >=0;
  xEq >=0;

END  
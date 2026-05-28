```cpp
MP_model m(new OsiCpxSolverInterface());  //1. Create the model and attach the solver from the OSI framework

//2. Define stages
enum { stage1, stage2, numStages };  //1st stage – planting
MP_stage T(numStages);               // 2nd stage – selling/purchasing crops

//3. Define set of crops and set of scenarios
enum { wheat, corn, sbeets, numCrops }; 
MP_set C(numCrops);   //set of crops

enum { low, normal, high, numScenarios };
MP_scenario_set scen(numScenarios);   //set of scenarios

//4. Define scenario probabilities. 
MP_data prob(scen); //In this problem each scenario has probability 1/3.

prob(low) = 1.0/3;
prob(normal) = 1.0/3;
prob(high) = 1.0/3;
m.setProbabilities(prob);

//5. Define data – planting cost, selling price, purchasing price, minimum requirement
MP_data plantingCost(C);   plantingCost.setName("plantingCost");
plantingCost(wheat) = 150;  plantingCost(corn) = 230;   plantingCost(sbeets) = 260;

MP_data sellingPrice(C); sellingPrice.setName("sellingPrice_below_quota");
sellingPrice(wheat) = 170;   sellingPrice(corn) = 150;  sellingPrice(sbeets) = 36;

MP_data sellingPrice_aQ(C); sellingPrice_aQ.setName("sellingPrice_above_quota");
sellingPrice_aQ(wheat) = -myInfty;  sellingPrice_aQ(corn) = -myInfty;   sellingPrice_aQ(sbeets) = 10;

MP_data purchPrice(C); purchPrice.setName("purchasingPrice");
purchPrice(wheat) = 238;  purchPrice(corn) = 210;  purchPrice(sbeets) = myInfty;

MP_data minReq(C); minReq.setName("minimumRequirement");
minReq(wheat) = 200;  minReq(corn) = 240;  minReq(sbeets) = 0;

double quota_val[3] = { myInfty, myInfty, 6000.0 };
MP_data quota(&quota_val[0], C);
quota.setName("quota");


//6. Define random data – crop yield
double scenYield[1][numCrops][numScenarios] =
{         // Second Stage , First Stage is always omitted
        { //Scen:   L    M     H
	                 {2 , 2.5 , 3},     // wheat
	                {2.4 , 3 , 3.6}, // corn
	                 {16 , 20 , 24}   // sugar beets
        }
};
MP_random_data yield(&scenYield[0][0][0], T, C);

//7. Define variables.
MP_variable x(C);      // acres planted
MP_variable w(T, C);   // crop sold below quota
MP_variable z(T, C);   // crop sold above quota
MP_variable y(T, C);   // crop purchased

//8. Define constraints.
MP_constraint landConstraint; landConstraint.setName("landConstr");
MP_constraint cattleConstraint(T, C); cattleConstraint.setName("cattleConstr");
MP_constraint quotaConstraint(T, C); quotaConstraint.setName("quotaConstr");

//total land cannot exceed available area
landConstraint = sum(C, x(C)) <= totalArea; 

//Production plus purchases must satisfy minimum demand.
cattleConstraint(T + 1, C) =  yield(T, C) * x(C) - w(T, C) - z(T, C) + y(T, C) >= minReq(C);

//Sales below quota are limited.
quotaConstraint(T + 1, C) = w(T, C) <= quota(C);

//9. Define the objective.
//The goal is to minimize the total cost while meeting the requirements.
m.setObjective(sum(C, plantingCost(C)* x(C))
- sum(C, sellingPrice(C) * w(T + 1, C) + sellingPrice_aQ(C) * z(T + 1, C) - purchPrice(C) * y(T + 1, C)));


//10. Solve
//Before solving, the model must be attached to the solver.
m.attach();
//The stochastic model is solved using a Parallel Nested Benders solver.
OsiCpxSolverInterface osi_cpx;
OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, m.getSmi());
m.setSolver(m.getSmi()->getOsiSolverInterface());
m.setStochasticSolver(&nested_benders);
m.StochSolver->initialSolve();

//11. Retrieve the solution
//After solving, the optimal objective value and decision variables can be displayed.
std::cout << nested_benders.getObjValue() << std::endl;

x.display();  w.display();   z.display();  y.display();
```

## Core parameters of the PNB:

__PNB_BENDERS__ 
Choice of the method of solving. Possible basic options are:

PNB_INT_ALGORITHM_DETEQ = 0
PNB_INT_ALGORITHM_BENDERS = 1;
PNB_INT_ALGORITHM_REGULARIZATION = 2;
PNB_INT_ALGORITHM_LEVEL = 3;
PNB_INT_ALGORITHM_LEVEL_ODA = 4;
PNB_INT_ALGORITHM_BENDERS_ODA = 5;
PNB_INT_ALGORITHM_REGULARIZATION_ODA = 6;
PNB_INT_ALGORITHM_LEVEL_MANHATTAN = 7;
PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA = 8;
PNB_INT_ALGORITHM_LEVEL_INFINITY = 9;
PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA = 10;
PNB_INT_ALGORITHM_SOLVER_BENDERS = 11;  -  use built-in Benders algorithm in solver (available in CPLEX)
PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT = 12;

__PNB_NUM_CPU__
Number of threads used. 0 (default value) means  taking what hardware concurrency offers.

__PNB_PRINT_SUBPROBLEMS__
If == true, prints .lp file.

__PNB_USE_START_SOLUTION__
Choice of start solutions. Possible options are:

PNB_INT_NO_START_SOLUTION = 0;
PNB_INT_EV_SOLUTION_FROM_PNB = 1;  -  find EV solution using ev_pnb, use cuts from ev_pnb.  TODO: DOES NOT WORK PROPERLY YET
PNB_INT_USE_EV_SOLUTION = 2;  -  benders started using ev solution from osi solver
PNB_INT_USE_SUPPLIED_SOLUTION = 3;
PNB_INT_EV_SOLUTION_FROM_PNB_WITH_EV = 4;  -  find EV solution using ev_pnb, use cuts from ev_pnb. ev_pnb is also started with ev solution from osi solver  TODO: DOES NOT WORK PROPERLY YET

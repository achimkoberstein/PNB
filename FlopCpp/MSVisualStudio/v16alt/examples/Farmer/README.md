## Two-Stage Farmer Problem Example

This example demonstrates how to build and solve a two-stage stochastic programming model using the library.

## Problem Description

The farmer must decide:

- How many acres of each crop to plant (first stage)
- How much crop to sell or purchase under different yield scenarios (second stage)

__The stochastic model contains:__

- 3 crops
- 3 yield scenarios
- recourse decisions
- quota constraints


__1. Create the Model__

```cpp
MP_model m(new OsiCpxSolverInterface());
```

__2. Define stages__

```cpp
enum { stage1, stage2, numStages };  
MP_stage T(numStages);               
```
- Stage 1: planting decisions
- Stage 2: selling and purchasing decisions


__3. Define sets__

```cpp
enum { wheat, corn, sbeets, numCrops }; 
MP_set C(numCrops);   //set of crops

enum { low, normal, high, numScenarios };
MP_scenario_set scen(numScenarios);   //set of scenarios
```


__4. Define scenario probabilities__

```cpp
MP_data prob(scen); 
prob(low) = 1.0/3;
prob(normal) = 1.0/3;
prob(high) = 1.0/3;
m.setProbabilities(prob);
```

__5. Define data__ 

```cpp
MP_data plantingCost(C); 
plantingCost(wheat) = 150;
plantingCost(corn) = 230;
plantingCost(sbeets) = 260;
```
Additional parameters include:
- selling prices
- purchasing prices
- crop quotas
- minimum requirements


__6. Define random yield data__

```cpp
double scenYield[1][numCrops][numScenarios] =
{         // Second Stage , First Stage is always omitted
        { //Scen:   L    M     H
                    {2 , 2.5 , 3},     // wheat
	                {2.4 , 3 , 3.6}, // corn
                    {16 , 20 , 24}   // sugar beets
        }
};
MP_random_data yield(&scenYield[0][0][0], T, C);
```


__7. Define variables__

```cpp
MP_variable x(C);      // acres planted
MP_variable w(T, C);   // crop sold below quota
MP_variable z(T, C);   // crop sold above quota
MP_variable y(T, C);   // crop purchased
```


__8. Define constraints.__

```cpp
MP_constraint landConstraint; 
landConstraint = sum(C, x(C)) <= totalArea; 

MP_constraint cattleConstraint(T, C); 
cattleConstraint(T + 1, C) =  yield(T, C) * x(C) - w(T, C) - z(T, C) + y(T, C) >= minReq(C); //Production plus purchases must satisfy minimum demand.

MP_constraint quotaConstraint(T, C);
quotaConstraint(T + 1, C) = w(T, C) <= quota(C); //Sales below quota are limited.
```

__9. Define objective__

```cpp
m.setObjective(sum(C, plantingCost(C)* x(C))
- sum(C, sellingPrice(C) * w(T + 1, C) + sellingPrice_aQ(C) * z(T + 1, C) - purchPrice(C) * y(T + 1, C)));
```

__10. Add parameters, solve__

```cpp
m.attach(); //Before solving, the model must be attached to the solver.

//The stochastic model is solved using a Parallel Nested Benders solver.
OsiCpxSolverInterface osi_cpx;

OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, m.getSmi());
m.setSolver(m.getSmi()->getOsiSolverInterface());
m.setStochasticSolver(&nested_benders);


parallel_benders::Parameter benders_param;
benders_param.init();
benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS);
m.StochSolver->setParameters(benders_param);
m.StochSolver->initialSolve();
```

__11. Retrieve the solution__

After solving, the optimal objective value and decision variables can be displayed.

```cpp
std::cout << nested_benders.getObjValue() << std::endl;
x.display();  w.display();   z.display();  y.display();
```

## Core parameters of the PNB:

__PNB_BENDERS__ 

Choice of the method of solving. Possible basic options are:
| Name | Value |Description|
|------|-------------|------|
|PNB_INT_ALGORITHM_DETEQ| 0 ||
|PNB_INT_ALGORITHM_BENDERS| 1 ||
|PNB_INT_ALGORITHM_REGULARIZATION| 2 ||
|PNB_INT_ALGORITHM_LEVEL|3||
|PNB_INT_ALGORITHM_LEVEL_ODA|4||
|PNB_INT_ALGORITHM_BENDERS_ODA|5||
|PNB_INT_ALGORITHM_REGULARIZATION_ODA|6||
|PNB_INT_ALGORITHM_LEVEL_MANHATTAN|7||
|PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA|8||
|PNB_INT_ALGORITHM_LEVEL_INFINITY|9||
|PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA|10||
|PNB_INT_ALGORITHM_SOLVER_BENDERS |11|use built-in Benders algorithm in solver (available in CPLEX)|
|PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT | 12 ||


__PNB_USE_START_SOLUTION__

| Name | Value | Description|
|------|-------------|-----------|
|PNB_INT_NO_START_SOLUTION|0| |
|PNB_INT_EV_SOLUTION_FROM_PNB|1|find EV solution using ev_pnb, use cuts from ev_pnb.|
|PNB_INT_USE_EV_SOLUTION|2|benders started using ev solution from osi solver|
|PNB_INT_USE_SUPPLIED_SOLUTION|3| |
|PNB_INT_EV_SOLUTION_FROM_PNB_WITH_EV|4|find EV solution using ev_pnb, use cuts from ev_pnb. ev_pnb is also started with ev solution from osi solver|


__PNB_WARM_START__

| Name | Value | Description|
|------|-------------|-----------|
|PNB_INT_NO_WARM_START|0| |
|PNB_INT_WARM_START_SCENARIO_WISE|1|WarmStart with solution from the previous iteration|
|PNB_INT_WARM_START_STAGE_WISE|2|WarmStart with solution from the first scenario (in the same iteration)|


__Other parameters__

|Parameter Name | Range | Description|
|------|-------------|-----------|
|__PNB_AGGREGATED_OPTIMALITY_CUTS__|Integer| |
|__PNB_NUM_CPU__|Integer|Number of threads used. 0 (default value) means  taking what hardware concurrency offers.|
|__PNB_PRINT_SUBPROBLEMS__|Boolean|Print the .lp files during solving the problems|



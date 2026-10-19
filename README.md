## Introduction
SFlopC++ is an open-source stochastic programming modeling framework built as an extension of FlopC++, a C++ library for algebraic modeling of optimization problems, with a stochastic modeling interface (SMI).
Offers a range of advanced acceleration techniques for multi-stage stochastic problems (NestedBenders):
- Level decomposition (Euclidean, manhattan, infinity distance)
- Regularized decomposition
- On-Demand Accuracy approach

Implemented for CPLEX, Gurobi and CLP solvers (interaction via OsiSolverInterface)

## Solvers
- IBM ILOG CPLEX (v. 22.2.0)
- Gurobi Optimizer (v. 12.0.3)
- CLP (COIN-OR Linear Programming Solver) (v. 1.14)

> Note: Commercial solvers (CPLEX, Gurobi) require a valid license.


## Solver licenses
### CPLEX
| License | Who it's for | Limits | Link |
|---|---|---|---|
| Community Edition (free) | Anyone, for learning and small models | 1000 variables and 1000 constraints | https://www.ibm.com/products/ilog-cplex-optimization-studio (use "Try free edition") |
| Academic (free) | Students, faculty, and staff at degree-granting institutions | None | https://www.ibm.com/academic/topic/data-science ([tutorial](https://www.youtube.com/watch?v=BVWPhseMf-A&t=191s)) |
| Paid | Production and commercial use | None | https://www.ibm.com/products/ilog-cplex-optimization-studio (use "Buy now") |

Make sure you install the Academic Initiative build, not the Community Edition,
or you will hit the size limits above (CPLEX error 1016).

### Gurobi
| License | Who it's for | Limits | Link |
|---|---|---|---|
| Commercial trial (free) | Companies evaluating Gurobi | 30-day trial | https://www.gurobi.com/free-trial/ |
| Academic (free) | Students, faculty, and staff at degree-granting institutions | None | https://www.gurobi.com/academics |
| Paid | Production and commercial use | None | https://www.gurobi.com/pricing/ |


__PNB is tested and supported for Windows 10 x64 and Visual Studio 2026.__

## Prerequisites:
Windows 10 x64, Visual Studio 2026

## Quick start:
1. 	Clone the repository: git clone https://github.com/pavloglushko/PNB.
2. 	Open it in Visual Studio 2026, move to "easy_configuration" branch.
3. Open solution FlopCpp.sln (FlopCpp/MSVisualStudio/v16alt/FlopCpp.sln). Here you will see the list of example optimization problems.	They can also be found in the following [folder](./FlopCpp/MSVisualStudio/v16alt/examples), but the launch of examples is made only through FlopCpp.sln.
4. Build the solution.
5. Right-click "Farmer" project, select "Set as starting project".
   Confirm that:
   - Solution Configuration = "Debug"
   - Solution Platforms = "x64
   Run the project. In case of unsuccessful compilation/linking, proceed with steps 6-8.
6. In case you installed CPLEX or Gurobi license -> Go to "Property Manager" ("View" -> "Other Windows" -> "Property Manager"). 
   Add one or both of the following files from the directory FlopCpp\MSVisualStudio\v16alt\ProjectConfiguration to the project:
   - 3_ProjectProperties.props for use of the CPLEX solver
   - 3_ProjectProperties_withGrb.props for use of the Gurobi solver
7. Check project dependencies and references. Both should include:
   - libflopcpp
   - ParallelNestedBenders
   - libOsiCpx
   - libOsiGrb 
   - libOsiClp
8. Run the project.
## Starting a new project:
1. Open solution FlopCpp.sln, right-click on it -> "Add" -> "New Project".
2. Add an empty project in the directory FlopCpp/MSVisualStudio/v16alt/examples.
3. Steps 5-8 of Quick start for your project.


## Core Concepts:
__MP_stage__
A dedicated set which contains all stages of the problems, and is used to index every constraint and variable at a stage other than the first. 

__MP_scenario_set__ 
A set which contains all scenarios. Useful, if scenario-wise data is retrieved by external tools, e.g., simulation results or a database. 

__RandomVariable__
 Models random variables. Needs distribution information and methods to discretize continuous distributions. If scenario information is given it contains a data entry for every scenario. Distribution information can be given in different ways, e.g., a typical normal distribution with mean µ and standard deviation σ or an empirical distribution with probability-value pairs.

__MP_random_data__ 
A random parameter. Can combine normal parameters (MP_data), random variables and random parameters with the usual FlopC++ algebraic operations, e.g., floor, +, ·. Smi can be extended with the following items to enable a better integration with the modeling language and possible solvers. 

__SMPS support, read/write methods__
Smi can read and write SMPS files. This allows interchanging models and testing different solvers that do not use Smi as input. Is done with the methods SmiScnModel::writeSmps() and SmiScnModel::readSmps()

```cpp
MP_model someModel(newOsiClpSolverInterface());

//...
// Specify the model here
//...

someModel.attach(someModel.Solver);
OsiStochasticParallelNestedBendersSolverInterface pnb(someModel.Solver,
someModel.getSmi());
pnb.initialSolve();

// Do some solution processing
```


## Stochastic examples:

| Example | Description | Useful for |
|----------|----------|--------------|
|[Farmers' problem](./FlopCpp/MSVisualStudio/v16alt/examples/Farmer)|   [Birge & Louveaux(2011), document provided by the McGillUniversity](https://cgm.cs.mcgill.ca/~avis/courses/567/notes/stoch1.pdf)| Explanation of basic example, details about the PNB solver check README)|   
|[Dakota problem](./FlopCpp/MSVisualStudio/v16alt/examples/Dakota)|[Higle (2005), p.31](https://pubsonline.informs.org/doi/pdf/10.1287/educ.1053.0016)|     |
|[Investment problem](./FlopCpp/MSVisualStudio/v16alt/examples/InvestmentProblem)|[Birge & Louveaux(2011), document provided by the McGillUniversity](https://cgm.cs.mcgill.ca/~avis/courses/567/notes/stoch1.pdf)|example of a multi-stage problem (4 stages)|
|[Multicommodity Network Design problem](./FlopCpp/MSVisualStudio/v16alt/examples/StochasticMNF)|[Rahmaniani et.al (2017)](https://www.cirrelt.ca/documentstravail/cirrelt-2017-22.pdf)|shows how to achieve full recourse in the objective with the help of a surplus variable|
|[Capacitated Facility Location problem](./FlopCpp/MSVisualStudio/v16alt/examples/Bodur_StochasticCFLP)|  [Bodur et.al (2016), p.15](https://optimization-online.org/wp-content/uploads/2014/03/4263.pdf)   |      |
|[SAPHIR problem](./FlopCpp/MSVisualStudio/v16alt/examples/SAPHIR)|     |     |
|[SCDP problem](./FlopCpp/MSVisualStudio/v16alt/examples/SCDP)|    |     |   

## Deterministic examples:
- [Sudoku problem](./FlopCpp/MSVisualStudio/v16alt/examples/Sudoku)
- [Multicommodity Network Design problem](./FlopCpp/MSVisualStudio/v16alt/examples/MulticommodityNetworkDesign)
- [Capacitated Facility Location problem](./FlopCpp/MSVisualStudio/v16alt/examples/CapacitatedFacilityLocation)


## Examples in progress:

| Example | Description | Useful for |
|----------|----------|--------------|
|[Bikesharing problem](./FlopCpp/MSVisualStudio/v16alt/examples/Bikesharing)|    |     |
|[CapaOpt problem](./FlopCpp/MSVisualStudio/v16alt/examples/CapaOpt)|    |    |
|[FleetAssignment problem](./FlopCpp/MSVisualStudio/v16alt/examples/FleetAssignment)| [Mancel & Mora-Camino (2006)](https://enac.hal.science/hal-00938765v1/document)    |    |
|[Network Interdiction problem](./FlopCpp/MSVisualStudio/v16alt/examples/StochasticNetworkInterdiction)|   [Bodur et.al (2016), p.18](https://optimization-online.org/wp-content/uploads/2014/03/4263.pdf)    |     |
|[SCDP_simple problem](./FlopCpp/MSVisualStudio/v16alt/examples/SCDP_simple)|    |    |



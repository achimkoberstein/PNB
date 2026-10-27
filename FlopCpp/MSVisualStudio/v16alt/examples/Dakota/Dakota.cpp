#include <iostream>

#include "flopc.hpp"
#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"

#include "SmiScnModel.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"


using namespace flopc;

int main(int argc, char* argv[]) {
#ifdef USE_GUROBI
	MP_model dakota(new OsiGrbSolverInterface());
#else 
	MP_model dakota(new OsiCpxSolverInterface());
#endif
	
	enum { stage1, stage2, numStages};
	MP_stage T(numStages);
	T.setName("T");

	enum { desk, table, chair, numProducts };
	MP_set P(numProducts);
	P.setName("P");

	enum { lumber, finishing, carpentry, numResources };
	MP_set R(numResources); 
	R.setName("R");

	enum { low, normal, high, numScenarios };
	MP_scenario_set scen(numScenarios); 
	scen.setName("Scen");

	MP_data prob(scen); 
	prob.setName("prob");
	prob(low) = 0.3;
	prob(normal) = 0.4;
	prob(high) = 0.3;
	dakota.setProbabilities(prob);

	double scenDemand[1][numProducts][numScenarios] =
	{// Second Stage , First Stage is always omitted
		{ // L M H
			{50 , 150 , 250} , // desk
			{20 , 110 , 250} , // t a b l e
			{200 , 225 , 500}  // chair
		}
	};
	MP_random_data demand(&scenDemand[0][0][0], T, P);
	demand.setName("demand");

	MP_data resourceCost(R);
	resourceCost.setName("resourceCost");
	resourceCost(lumber) = 2;
	resourceCost(finishing) = 4;
	resourceCost(carpentry) = 5.2;

	MP_data resourceReq(P, R);
	resourceReq.setName("resourceReq");
	resourceReq(desk, lumber) = 8;
	resourceReq(desk, finishing) = 4;
	resourceReq(desk, carpentry) = 2;
	resourceReq(table, lumber) = 6;
	resourceReq(table, finishing) = 2;
	resourceReq(table, carpentry) = 1.5;
	resourceReq(chair, lumber) = 1;
	resourceReq(chair, finishing) = 1.5;
	resourceReq(chair, carpentry) = 0.5;

	double prices[3] = { 60.0, 40.0, 10.0 };
	MP_data sellingPrice(&prices[0], P);
	sellingPrice.setName("sellingPrice");

	MP_variable x(R); x.setName("x");		// amount of resources
	MP_variable y(T, P); y.setName("y");	// produced units

	MP_constraint demandConstraint(T, P); demandConstraint.setName("demandConstr");
	MP_constraint productionConstraint(T, R); productionConstraint.setName("productionConstr");

	demandConstraint(T + 1, P) = y(T, P) <= demand(T, P);
	productionConstraint(T + 1, R) = sum(P, resourceReq(P, R) * y(T, P)) <= x(R);

	dakota.setObjective(					// convert into minimization problem
		- sum(P, y(T + 1, P) * sellingPrice(P))
		+ sum(R, x(R) * resourceCost(R)));

//	dakota.attach(dakota.Solver);
	dakota.attach();

	std::cout << "Writing SMPS" << std::endl;

	dakota.getSmi()->writeSmps(".\\Output\\Dakota_smps", false, false);


	double cpxtime = CoinCpuTime();
	dakota.solve(MP_model::MINIMIZE);
	cpxtime = CoinCpuTime() - cpxtime;

	dakota.Solver->writeMps(".\\Output\\Dakota");

	x.display("Display x:\n");
	y.display("Display y:\n");

	std::cout << "Optimal solution:" << std::endl;
    std::cout << "x[lumber] level scenario = " << x.levelScenario(0,lumber) << std::endl;
	std::cout << "x.level(finishing) = " << x.level(lumber) << std::endl;
	std::cout << "x[lumber] level = " << x(lumber).level() << std::endl;
	std::cout << "x[finishing] level scenario = " << x.levelScenario(0, finishing) << std::endl;
	std::cout << "x.level(finishing) = " << x.level(finishing) << std::endl;
	std::cout << "x[finishing].level = " << x(finishing).level() << std::endl;
	std::cout << "x[carpentry] level scenario = " << x.levelScenario(0, carpentry) << std::endl;
	std::cout << "x.level(carpentry) = " << x.level(carpentry) << std::endl;
	std::cout << "x[carpentry].level = " << x(carpentry).level() << std::endl;

	std::cout << "y[desk] scen low = " << y.levelScenario(low, stage2, desk) << std::endl;
	std::cout << "y[desk] scen low = " << y(stage2, desk).levelScenario(low) << std::endl;
	std::cout << "y[desk] scen normal = " << y.levelScenario(normal, stage2, desk) << std::endl;
	std::cout << "y[desk] scen normal = " << y(stage2, desk).levelScenario(normal) << std::endl;
	std::cout << "y[desk] scen high = " << y.levelScenario(high, stage2, desk) << std::endl;
	std::cout << "y[desk] scen high = " << y(stage2, desk).levelScenario(high) << std::endl;

	std::cout << "y[table] scen low = " << y.levelScenario(low, stage2, table) << std::endl;
	std::cout << "y[table] scen low = " << y(stage2, table).levelScenario(low) << std::endl;
	std::cout << "y[table] scen normal = " << y.levelScenario(normal, stage2, table) << std::endl;
	std::cout << "y[table] scen normal = " << y(stage2, table).levelScenario(normal) << std::endl;
	std::cout << "y[table] scen high = " << y.levelScenario(high, stage2, table) << std::endl;
	std::cout << "y[table] scen high = " << y(stage2, table).levelScenario(high) << std::endl;

	std::cout << "y[chair] scen low = " << y.levelScenario(low, stage2, chair) << std::endl;
	std::cout << "y[chair] scen low = " << y(stage2, chair).levelScenario(low) << std::endl;
	std::cout << "y[chair] scen normal = " << y.levelScenario(normal, stage2, chair) << std::endl;
	std::cout << "y[chair] scen normal = " << y(stage2, chair).levelScenario(normal) << std::endl;
	std::cout << "y[chair] scen high = " << y.levelScenario(high, stage2, chair) << std::endl;
	std::cout << "y[chair] scen high = " << y(stage2, chair).levelScenario(high) << std::endl;

	// ---------------------------------------
	// ---------- use PNB solver -------------
	// ---------------------------------------

	dakota.Solver->writeMps(".\\Output\\Dakota");

#ifdef USE_GUROBI
	OsiGrbSolverInterface osi_solver;
#else
	OsiCpxSolverInterface osi_solver;
#endif 
	
	OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_solver, dakota.getSmi()); //side effect: smi osi solver object is recreated (old one is deleted)
	dakota.setSolver(dakota.getSmi()->getOsiSolverInterface()); // therefore model osi solver object needs to be reset

	dakota.setStochasticSolver(&nested_benders);

	dakota.Solver->writeMps(".\\Output\\Dakota");

	// Declare parameter file
	parallel_benders::Parameter benders_param;
	benders_param.init();
	benders_param.parameter_.put(parallel_benders::PNB_BENDERS, 1);
//	benders_param.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION, parallel_benders::PNB_INT_USE_EV_SOLUTION);
	benders_param.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL);

	// To set a parameter look in NestedBendersTest.cpp
	dakota.StochSolver->setParameters(benders_param);

	double pnbtime = CoinCpuTime();
	dakota.StochSolver->initialSolve();
	pnbtime = CoinCpuTime() - pnbtime;

	std::cout << "Objective-Value " << nested_benders.getObjValue() << std::endl;
//	std::cout << "Cplex time = " << cpxtime << "     Benders time = " << pnbtime << std::endl;

	dakota.StochSolver->printSolutionToFile("PNB_solution.txt");

	x.display("x");
	y.display("y");

	dakota.getSmi()->printout();
}
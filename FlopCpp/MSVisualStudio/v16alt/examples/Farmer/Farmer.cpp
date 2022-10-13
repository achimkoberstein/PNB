#include <iostream>

#include "flopc.hpp"
#include "OsiCpxSolverInterface.hpp"

#include "SmiScnModel.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"


using namespace flopc;

int main(int argc, char* argv[]) {
	const double myInfty = 1e9;

	MP_model m(new OsiCpxSolverInterface());

	enum { stage1, stage2, numStages };
	MP_stage T(numStages);
	T.setName("T");

	enum { wheat, corn, sbeets, numCrops };
	MP_set C(numCrops);
	C.setName("C");

	enum { low, normal, high, numScenarios };
//	enum { normal, numScenarios };
	MP_scenario_set scen(numScenarios);
	scen.setName("Scen");

	MP_data prob(scen);
	prob.setName("prob");
//	prob(normal) = 1.0;
	prob(low) = 1.0/3;
	prob(normal) = 1.0/3;
	prob(high) = 1.0/3;
	m.setProbabilities(prob);

	double totalArea = 500.0;

	double scenYield[1][numCrops][numScenarios] =
	{// Second Stage , First Stage is always omitted
		{ // scen L M H
			{2 , 2.5 , 3} , // wheat
			{2.4 , 3 , 3.6} , // corn
			{16 , 20 , 24}  // sugar beets
		}
	};
	//double scenYield[1][numCrops][numScenarios] =
	//{// Second Stage , First Stage is always omitted
	//	{ // scen L M H
	//		{2.5} , // wheat
	//		{3} , // corn
	//		{20}  // sugar beets
	//	}
	//};
	MP_random_data yield(&scenYield[0][0][0], T, C);
	yield.setName("yield");

	MP_data plantingCost(C);
	plantingCost.setName("plantingCost");
	plantingCost(wheat) = 150;
	plantingCost(corn) = 230;
	plantingCost(sbeets) = 260;

	MP_data sellingPrice(C);
	sellingPrice.setName("sellingPrice_below_quota");
	sellingPrice(wheat) = 170;
	sellingPrice(corn) = 150;
	sellingPrice(sbeets) = 36;

	MP_data sellingPrice_aQ(C);
	sellingPrice_aQ.setName("sellingPrice_above_quota");
	sellingPrice_aQ(wheat) = -myInfty;
	sellingPrice_aQ(corn) = -myInfty;
	sellingPrice_aQ(sbeets) = 10;

	MP_data purchPrice(C);
	purchPrice.setName("purchasingPrice");
	purchPrice(wheat) = 238;
	purchPrice(corn) = 210;
	purchPrice(sbeets) = myInfty;

	MP_data minReq(C);
	minReq.setName("minimumRequirement");
	minReq(wheat) = 200;
	minReq(corn) = 240;
	minReq(sbeets) = 0;

	double quota_val[3] = { myInfty, myInfty, 6000.0 };
	MP_data quota(&quota_val[0], C);
	quota.setName("quota");

	MP_variable x(C); x.setName("x");		// acres of land devoted to crop c
	MP_variable w(T, C); w.setName("w");	// crop sold at good price (below quota)
	MP_variable z(T, C); z.setName("z");	// crop sold at bad price (above quota)
	MP_variable y(T, C); y.setName("y");	// crop purchased

	MP_constraint landConstraint; landConstraint.setName("landConstr");
	MP_constraint cattleConstraint(T, C); cattleConstraint.setName("cattleConstr");
	MP_constraint quotaConstraint(T, C); quotaConstraint.setName("quotaConstr");

	landConstraint = sum(C, x(C)) <= totalArea;
	cattleConstraint(T + 1, C) = yield(T, C) * x(C) - w(T, C) - z(T, C) + y(T, C) >= minReq(C);
	quotaConstraint(T + 1, C) = w(T, C) <= quota(C);

	m.setObjective(					// convert into minimization problem
		sum(C, plantingCost(C)* x(C))
		- sum(C, sellingPrice(C) * w(T + 1, C) + sellingPrice_aQ(C) * z(T + 1, C) - purchPrice(C) * y(T + 1, C)));

	//	m.attach(m.Solver);
	m.attach();
	m.Solver->writeLp(".\\Farmer1");

	std::cout << "Writing SMPS" << std::endl;

	m.getSmi()->writeSmps(".\\Farmer_smps", false, false);

	double cpxtime = CoinCpuTime();
	m.solve(MP_model::MINIMIZE);
	cpxtime = CoinCpuTime() - cpxtime;

	m.Solver->writeMps(".\\Farmer");
	m.Solver->writeLp(".\\Farmer");

	cout << "Optimal objective: " << m.Solver->getObjValue() << endl;
	x.display("Display x:\n");
	w.display("Display w:\n");
	z.display("Display z:\n");
	y.display("Display y:\n");

/*
	// ---------------------------------------
	// ---------- use PNB solver -------------
	// ---------------------------------------

	OsiCpxSolverInterface osi_cpx;
	OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, m.getSmi());

	m.setStochasticSolver(&nested_benders);

	// Declare parameter file
	parallel_benders::Parameter benders_param;
	benders_param.init();
	benders_param.parameter_.put(parallel_benders::PNB_BENDERS, 0);
	//	benders_param.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION, parallel_benders::PNB_INT_USE_EV_SOLUTION);
	benders_param.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL);

	// To set a parameter look in NestedBendersTest.cpp
	m.StochSolver->setParameters(benders_param);

	double pnbtime = CoinCpuTime();
	m.StochSolver->initialSolve();
	pnbtime = CoinCpuTime() - pnbtime;

	std::cout << "Objective-Value " << nested_benders.getObjValue() << std::endl;
	//	std::cout << "Cplex time = " << cpxtime << "     Benders time = " << pnbtime << std::endl;

	m.StochSolver->printSolutionToFile("PNB_solution.txt");

	x.display("Display x:\n");
	w.display("Display w:\n");
	z.display("Display z:\n");
	y.display("Display y:\n");
*/
	m.getSmi()->printout();
}
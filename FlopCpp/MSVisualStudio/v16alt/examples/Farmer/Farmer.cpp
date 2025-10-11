#include <iostream>

#include "flopc.hpp"
#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"

#include "SmiScnModel.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"


using namespace flopc;

int main(int argc, char* argv[]) {
	const double myInfty = 1e5;

	MP_model m(new OsiGrbSolverInterface());

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
	m.Solver->writeLp(".\\Farmer");

	std::cout << "Writing SMPS" << std::endl;
	//m.getSmi()->writeSmps(".\\Farmer_smps", false, false);

	// ---------------------------------------
	// ---------- use PNB solver -------------
	// ---------------------------------------

	OsiGrbSolverInterface osi_grb;
	OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_grb, m.getSmi()); //side effect: smi osi solver object is recreated (old one is deleted)
	m.setSolver(m.getSmi()->getOsiSolverInterface()); // therefore model solver object needs to be reset

	m.setStochasticSolver(&nested_benders);

	// Declare parameter file
	parallel_benders::Parameter benders_param;
	benders_param.init();
	benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS);

    //	benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS_ODA);
	// benders_param.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION, parallel_benders::PNB_INT_USE_EV_SOLUTION);
//	benders_param.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION, true);  //CRASHES
	benders_param.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL);
	benders_param.parameter_.put(parallel_benders::PNB_PRINT_SUBPROBLEMS, true);

	// To set a parameter look in NestedBendersTest.cpp
	m.StochSolver->setParameters(benders_param);

	double pnbtime = CoinCpuTime();
	m.StochSolver->initialSolve();
	pnbtime = CoinCpuTime() - pnbtime;

	std::cout << "Objective-Value " << nested_benders.getObjValue() << std::endl;

	m.StochSolver->printSolutionToFile("PNB_solution.txt");

	x.display("Display x:\n");
	w.display("Display w:\n");
	z.display("Display z:\n");
	y.display("Display y:\n");

	m.getSmi()->printout();
	

/*
	MP_model m(new OsiCpxSolverInterface);
	const double myInfty = 1e5;

	enum { wheat, corn, sbeets, numCrops };
	MP_set C(numCrops);
	C.setName("C");

	double totalArea = 500.0;

	MP_data yield(C);
	yield(wheat) = 2.5;
	yield(corn) = 3;
	yield(sbeets) = 20;

	//Planting cost for crops
	MP_data plantingCost(C);
	plantingCost.setName("plantingCost");
	plantingCost(wheat) = 150;
	plantingCost(corn) = 230;
	plantingCost(sbeets) = 260;

	MP_data sellingPrice(C); //Selling price for crops below the quota
	sellingPrice.setName("sellingPrice_below_quota");
	sellingPrice(wheat) = 170;
	sellingPrice(corn) = 150;
	sellingPrice(sbeets) = 36;

	MP_data sellingPrice_aQ(C);	//Selling price for crops above the quota 
	sellingPrice_aQ.setName("sellingPrice_above_quota");
	sellingPrice_aQ(wheat) = -myInfty;
	sellingPrice_aQ(corn) = -myInfty;
	sellingPrice_aQ(sbeets) = 10;

	MP_data purchPrice(C); //Purchasing price for for crops.
	purchPrice.setName("purchasingPrice");
	purchPrice(wheat) = 238;
	purchPrice(corn) = 210;
	purchPrice(sbeets) = myInfty;

	MP_data minReq(C); //Minimal requirement of yields for Farmer
	minReq.setName("minimumRequirement");
	minReq(wheat) = 200;
	minReq(corn) = 240;
	minReq(sbeets) = 0;

	//Quota for crops - applicable only for sugar beets (6000)
	double quota_val[3] = { myInfty, myInfty, 6000.0 };
	MP_data quota(&quota_val[0], C);
	quota.setName("quota");

	MP_variable x(C); x.setName("x"); // acres of land devoted to crop c
	MP_variable w(C); w.setName("w"); // crop sold at good price (below quota)
	MP_variable z(C); z.setName("z"); // crop sold at bad price (above quota)
	MP_variable y(C); y.setName("y"); // crop purchased

	MP_constraint landConstraint; landConstraint.setName("landConstr");
	MP_constraint cattleConstraint(C); cattleConstraint.setName("cattleConstr");
	MP_constraint quotaConstraint(C); quotaConstraint.setName("quotaConstr");

	//Sum of all land assigned to crops should be not more than 500 acres
	landConstraint = sum(C, x(C)) <= totalArea;

	//For each crop: yield - crop sold (below & above quota)
	//+ crop purchased >= minimal requirement
	cattleConstraint(C) = yield(C) * x(C) - w(C) - z(C) + y(C) >= minReq(C);

	//For each crop: amount of sold below quota should not exceed quota
	quotaConstraint(C) = w(C) <= quota(C);

	m.setObjective(	//minimization problem
		sum(C, plantingCost(C) * x(C)) - sum(C, sellingPrice(C) * w(C)
			+ sellingPrice_aQ(C) * z(C) - purchPrice(C) * y(C)));

	m.attach(m.Solver);

	m.Solver->writeLp(".\\Farmer1");

	double cpxtime = CoinCpuTime();
	m.solve(MP_model::MINIMIZE);
	cpxtime = CoinCpuTime() - cpxtime;

	m.Solver->writeMps(".\\Farmer");
	m.Solver->writeLp(".\\Farmer");

	cout << "Optimal objective: " << m.Solver->getObjValue() << endl;
	x.display("Display x:\n");
	//w.display("Display w:\n");
	//z.display("Display z:\n");
	//y.display("Display y:\n");
*/


}
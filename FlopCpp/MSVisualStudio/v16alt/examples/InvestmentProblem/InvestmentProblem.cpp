#include <iostream>

#include "flopc.hpp"
#include "OsiCpxSolverInterface.hpp"

#include "SmiScnModel.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"

using namespace flopc;


void runInvestmentModel() {

    MP_model investmentModel(new OsiCpxSolverInterface());
    MP_data start, goal;
    start() = 55; goal() = 80;

    //int numStage = 4; int numScen = 8;
    enum { numStage = 4 };
    enum { numScen = 8 };
    enum { asset1, asset2, numAssets };

    MP_stage T(numStage);
    MP_set assets(numAssets);
    MP_scenario_set scen(numScen);


    double scenarios[numStage - 1][numAssets][numScen] =
    { // stage 2
        {{1.25,1.25,1.25,1.25,1.06,1.06,1.06,1.06}, //asset1 
        {1.14,1.14,1.14,1.14,1.12,1.12,1.12,1.12}} //asset2 
    ,
        // stage 3
           {{1.25,1.25,1.06,1.06,1.25,1.25,1.06,1.06}, //asset1 
           {1.14,1.14,1.12,1.12,1.14,1.14,1.12,1.12}} //asset2 
       ,
        // stage 4
           {{1.25,1.06,1.25,1.06,1.25,1.05,1.25,1.06}, //asset1 
           {1.14,1.12,1.14,1.12,1.14,1.12,1.14,1.12}} //asset2 
    };

    MP_random_data returns(&scenarios[0][0][0], T, assets);

    returns.display("returns");

    MP_variable x(T, assets), wealth(T), shortage(T), overage(T);


    MP_constraint initialWealthConstr;
    MP_constraint allocationConstr(T);
    MP_constraint returnConstr(T);
    MP_constraint goalConstr(T);

    initialWealthConstr() = sum(assets, x(0, assets)) == start();
    allocationConstr(T) = sum(assets, x(T, assets)) == wealth(T);
    returnConstr(T).such_that(T > 0) = sum(assets, returns(T, assets) * x(T - 1, assets)) == wealth(T);
    //   returnConstr(T) = sum(assets, returns(T, assets) * x(T, assets)) == wealth(T);
    //   returnConstr(T+1) = sum(assets, returns(T+1, assets) * x(T, assets)) == wealth(T+1);
    goalConstr(T.last()) = wealth(T.last()) == goal() + overage(T.last()) - shortage(T.last());

    MP_expression valueFunction(-4.0 * shortage(T.last()) + 1.0 * overage(T.last()));

    investmentModel.setObjective(valueFunction);
    investmentModel.attach();
    //   investmentModel.getSmi()->writeSmps(".\\InvModel_smps", false, false);
    investmentModel.solve(MP_model::MAXIMIZE);
    investmentModel.Solver->writeLp(".\\InvModel");

}

int main(int argc, char* argv[]) {
    cout << "Start" << endl;

    runInvestmentModel();

    cout << "End" << endl;
}
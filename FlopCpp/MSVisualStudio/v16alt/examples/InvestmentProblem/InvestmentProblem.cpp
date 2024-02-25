#include <iostream>

#include "flopc.hpp"
#include "OsiCpxSolverInterface.hpp"

#include "SmiScnModel.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"

using namespace flopc;

int main(int argc, char* argv[]) {

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


    double scenarios[numScen - 1][numAssets][numScen] =
    { { // stage 2
          {1.25,1.25,1.25,1.25,1.06,1.06,1.06,1.06}, //asset1 - 1.155
          {1.14,1.14,1.14,1.14,1.16,1.16,1.16,1.16} //asset2 - 1.15
      },
      { // stage 3
          {1.21,1.21,1.07,1.07,1.15,1.15,1.06,1.06}, //asset1 - 1.1225
          {1.17,1.17,1.12,1.12,1.18,1.18,1.12,1.12} //asset2 - 1.1475
      },
      { // stage 4
          {1.26,1.07,1.25,1.06,1.05,1.06,1.05,1.06}, //asset1 - 1.1075
          {1.13,1.14,1.15,1.12,1.17,1.15,1.14,1.12} //asset2 - 1.14
      }
    };

    MP_random_data returns(&scenarios[0][0][0], T, assets);
    MP_variable x(T, assets), wealth(T), shortage(T), overage(T);


    MP_constraint initialWealthConstr, returnConstr(T, assets);
    MP_constraint allocationConstr(T,assets), goalConstr;

    initialWealthConstr() = sum(assets, x(0, assets)) == start();
    allocationConstr(T,assets) = sum(assets, x(T, assets)) == wealth(T);
    returnConstr(T,assets) = sum(assets, returns(T, assets) * x(T - 1, assets)) == wealth(T);
    goalConstr() = wealth(T.last()) == goal() + overage(T.last()) - shortage(T.last());

    MP_expression valueFunction(-1.3 * shortage(T.last()) + 1.1 * overage(T.last()));

    investmentModel.setObjective(valueFunction);
    investmentModel.attach();
    investmentModel.Solver->writeLp(".\\InvModel");
    investmentModel.getSmi()->writeSmps(".\\InvModel_smps", false, false);
    investmentModel.solve(MP_model::MAXIMIZE);

   

}
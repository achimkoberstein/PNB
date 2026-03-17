#include "flopc.hpp"
using namespace flopc;
using std::cout;
using std::endl;
#include "SmiScnModel.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"

#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include<vector>


int main(int argc, char* argv[]) {


#ifdef USE_GUROBI
    MP_model m(new OsiGrbSolverInterface());
#else 
    MP_model m(new OsiCpxSolverInterface());
#endif

    //std::string fname = "r01.1.dow";
    std::string fname = "simpleProblem.dow";
    std::ifstream file(fname.c_str());

    std::string line;
    std::getline(file, line);

    int numnodes, numarcs, numcoms;
    file >> numnodes >> numarcs >> numcoms;
    cout << "numnodes = " << numnodes << endl;
    cout << "numarcs = " << numarcs << endl;
    cout << "numcoms = " << numcoms << endl;

    MP_set K(numcoms); //commodities
    MP_set N(numnodes); //nodes

    using Array5 = std::array<double, 5>;
    std::vector<Array5> arcData(numarcs);

    int end_node, start_node, transportation_cost, capacity, design_cost, not_used_column;

    for (int i = 0; i < numarcs; i++) {
        file >> end_node >> start_node >> transportation_cost >> capacity >> design_cost >> not_used_column >> not_used_column;
        cout << " " << end_node << " " << start_node << " " << transportation_cost << " " << capacity << " " << design_cost << endl;
        arcData[i][0] = end_node;
        arcData[i][1] = start_node;
        arcData[i][2] = transportation_cost;
        arcData[i][3] = capacity;
        arcData[i][4] = design_cost;
    }

    using Array2 = std::array<double, 2>;
    std::vector<Array2> commodityData(numcoms);

    int origin, destination;
    for (int i = 0; i < numcoms; i++) {
        file >> origin >> destination >> not_used_column;

        cout << origin << " " << destination << endl;

        commodityData[i][0] = origin - 1;
        commodityData[i][1] = destination - 1;
    }
    file.close();




    //std::string scenfname = "r01-0.2-16";
    std::string scenfname = "simpleScenarioSet.2-2";
    std::ifstream scenfile(scenfname.c_str());

    enum { stage1, stage2, numStages };
    MP_stage T(numStages);
    T.setName("T");

    int data;
    scenfile >> data;
    int numScenarios = data;

    cout << numScenarios << endl;
    MP_scenario_set scen(numScenarios);
    scen.setName("Scen");

    MP_data prob(scen);
    prob.setName("prob");


    MP_subset<2> A(N, N); //arcs
    for (int i = 0; i < numarcs; i++) {
        A.insert(arcData[i][0] - 1, arcData[i][1] - 1);
    }
    // A.display();

    MP_data C(A); //transportation costs
    MP_data U(A); //capacity
    MP_data F(A); //design costs

    for (int i = 0; i < numarcs; i++) {
        C(A(arcData[i][0] - 1, arcData[i][1] - 1)) = arcData[i][2];
        U(A(arcData[i][0] - 1, arcData[i][1] - 1)) = arcData[i][3];
        F(A(arcData[i][0] - 1, arcData[i][1] - 1)) = arcData[i][4];
    }

    //C.display();

    MP_data O(K); //origins
    MP_data D(K); //destinations

    for (int k = 0; k < numcoms; k++) {
        O(k) = commodityData[k][0];
        D(k) = commodityData[k][1];
    }



    std::vector<double> scenDemand((numStages - 1) * numnodes * numcoms * numScenarios);

    //MISTAKE HERE
    vector<double> probability(numScenarios);
    vector<vector<double>> stochasticDemand(numScenarios, vector<double>(numcoms));

    double pr, stochD;
    for (int i = 0; i < numScenarios; i++) {
        scenfile >> pr;
        probability[i] = pr;
        prob(i) = pr;

        for (int k = 0; k < numcoms; k++) {
            scenfile >> stochD;
            stochasticDemand[i][k] = stochD;
        }
    }

    for (int n = 0; n < numnodes; n++) {
        for (int k = 0; k < numcoms; k++) {
            for (int i = 0; i < numScenarios; i++) {
                if (n == O(k)) {
                    scenDemand[((0 * numnodes + n) * numcoms + k) * numScenarios + i] = stochasticDemand[i][k];
                }
                else if (n == D(k)) {
                    scenDemand[((0 * numnodes + n) * numcoms + k) * numScenarios + i] = -stochasticDemand[i][k];
                }
                else {
                    scenDemand[((0 * numnodes + n) * numcoms + k) * numScenarios + i] = 0;
                }
                cout << scenDemand[((0 * numnodes + n) * numcoms + k) * numScenarios + i] << " ";
            }
            cout << "|";
        }
    }

    m.setProbabilities(prob);
    cout << "SCEN DEMAND:" << endl;


    MP_random_data demand(&scenDemand[0], T, N, K);
    demand.setName("demand");


    file.close();

    //demand.display();

    MP_variable	x(T, A, K); //transportation cost variable
    MP_binary_variable	y(A); //design cost variable

    x.setName("x");
    y.setName("y");


    MP_constraint  demand_constraint(T, N, K); //demand constraint for each node should be satisfied      
    MP_constraint  capacity_constraint(T, A);

    MP_index i, j, k;


    demand_constraint(T + 1, i, k) = sum(N(j), x(T, A(i, j), k)) - sum(N(j), x(T, A(j, i), k)) == demand(T, i, k);
    capacity_constraint(T + 1, A) = sum(K, x(T, A, K)) <= y(A) * U(A);

    m.setObjective(sum(K, sum(A, x(T + 1, A, K) * C(A))) + sum(A, F(A) * y(A)));

    m.attach();
    //m.Solver->writeLp(".\\StochasticNetworkDesign");
    m.Solver->writeLp(".\\simpleStochasticNetworkDesign");
    std::cout << "Writing SMPS" << std::endl;

    //m.getSmi()->writeSmps(".\\StochasticNetworkDesign_smps", false, false);
    m.getSmi()->writeSmps(".\\simpleStochasticNetworkDesign_smps", true, true);

    // ---------------------------------------
    // ---------- use PNB solver -------------
    // ---------------------------------------

#ifdef USE_GUROBI
    OsiGrbSolverInterface osi_solver;
#else
    OsiCpxSolverInterface osi_solver;
#endif 

    OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_solver, m.getSmi()); //side effect: smi osi solver object is recreated (old one is deleted)
    m.setSolver(m.getSmi()->getOsiSolverInterface()); // therefore model solver object needs to be reset

    m.setStochasticSolver(&nested_benders);

    // Declare parameter file
    parallel_benders::Parameter benders_param;
    benders_param.init();
   //benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_DETEQ);
    benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS);
    benders_param.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL);

    benders_param.parameter_.put(parallel_benders::PNB_PRINT_SUBPROBLEMS, 1);

    // To set a parameter look in NestedBendersTest.cpp
    m.StochSolver->setParameters(benders_param);

    double pnbtime = CoinCpuTime();
    m.StochSolver->initialSolve();
    pnbtime = CoinCpuTime() - pnbtime;

    std::cout << "Objective-Value " << nested_benders.getObjValue() << std::endl;
    //x.display();
    y.display();
    
}

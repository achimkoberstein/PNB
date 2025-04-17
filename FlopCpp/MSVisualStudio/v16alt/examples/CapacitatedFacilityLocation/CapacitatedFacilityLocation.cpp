#include "flopc.hpp"
using namespace flopc;
using std::cout;
using std::endl;
#include "SmiScnModel.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"

#include <OsiCpxSolverInterface.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include<vector>


static double HUGE_COST = 10000.00;
static int NULL_DEMAND = 0;

int main(int argc, char* argv[]) {
    //MP_model m(new OsiCpxSolverInterface);
    OsiSolverInterface* osi_solver = new OsiCpxSolverInterface();
    MP_model m(osi_solver);


    std::string fname = "1Cap10.txt";
    std::ifstream file(fname.c_str());

   

    std::string line;
    for (int i = 0; i < 3; i++) {
        std::getline(file, line);
        cout << line << endl;
    }
    
    int dimension, fixed_cost, capacity;
    file >> dimension >> fixed_cost >> capacity;
    cout << "dimension = " << dimension << endl;
    cout << "fixed cost = " << fixed_cost << endl;
    cout << "capacity = " << capacity << endl;

    //cost needs to be made as scenarios!!!!! (maybe)
    std::getline(file, line);
    cout << line << endl;
    std::getline(file, line);
    cout << line << endl;




    MP_set facilities(dimension);
    MP_set clients(dimension);
    
    int facility, client, demand;
    double transportation_cost;
    
    int problem_size = dimension * dimension;
    
    using Array4 = std::array<double, 4>;





    //Fill the array with the null-values for all the connections betweeen facilities and clients
    std::vector<Array4> arcData(problem_size);
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            arcData[i * dimension + j][0] = i + 1;
            arcData[i * dimension + j][1] = j + 1;
            arcData[i * dimension + j][2] = HUGE_COST;
            arcData[i * dimension + j][3] = NULL_DEMAND;

        }
    }

    while (file >> facility >> client >> transportation_cost >> demand) {
        arcData[(facility - 1) * dimension + (client - 1)][2] = transportation_cost;
        arcData[(facility - 1) * dimension + (client - 1)][3] = demand;
    }

    file.close();


    //enum { stage1, stage2, numStages };
    //MP_stage T(numStages);
    //T.setName("T");

    MP_subset<2> A(facilities, clients); //arcs
    for (int i = 0; i < problem_size; i++) {
        A.insert(arcData[i][0] - 1, arcData[i][1] - 1);
    }

    MP_data G(A); //transportation costs
    MP_data P(A); //demand

    for (int i = 0; i < problem_size; i++) {
        G(A(arcData[i][0] - 1, arcData[i][1] - 1)) = arcData[i][2];
        P(A(arcData[i][0] - 1, arcData[i][1] - 1)) = arcData[i][3];
    }

    MP_binary_variable	x(facilities, clients); //whether facility i delivers something to client j
    MP_binary_variable	y(facilities); //which facilities to open


    x.setName("x");
    y.setName("y");

    MP_constraint  demand_constraint(clients); // every client is serviced only by one facility??       
    MP_constraint  capacity_constraint(facilities);


    MP_index i, j, k;
    MP_data dc;
    dc() = 1; //to make sure that only one binary variable is activated

    //
    demand_constraint(j) = sum(facilities(i), x(i, j)) == dc();
    capacity_constraint(i) = sum(clients(j), P(A(i,j))*x(i,j)) <= capacity*y(i);


    demand_constraint.setName("dcons"); // These names are not going there for some reason!!!!
    capacity_constraint.setName("ccons");


    //instead of fixed costs - opening costs??
    m.setObjective( sum(facilities(i), fixed_cost*y(i)) +  sum(facilities(i), sum(clients(j), G(A(i,j))*x(i,j))) );

    m.attach();

    double cpxtime = CoinCpuTime();
    m.solve(MP_model::MINIMIZE);
    cpxtime = CoinCpuTime() - cpxtime;


    m.Solver->writeMps(".\\solution");
    m.Solver->writeLp(".\\solution");

    cout << "Optimal objective: " << m.Solver->getObjValue() << endl;

    //PROBLEM: display() method doesn't work on this task
    x.display("Display x:\n");
    y.display("Display y:\n");



/*
    //std::string scenfname = "r01-0.2-64";
    std::string scenfname = "simple_scenarioset.2-200";
    std::ifstream scenfile((folder + scenfname).c_str());

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
    MP_variable       z(T);  // surplus variable to achieve full recourse

    x.setName("x"); // These names are going to SMPS files!!!!
    y.setName("y");
    z.setName("z");

    MP_constraint  demand_constraint(T, N, K); //demand constraint for each node should be satisfied      
    MP_constraint  capacity_constraint(T, A);


    MP_index i, j, k;

    demand_constraint(T + 1, i, k) = sum(N(j), x(T, A(i, j), k)) - sum(N(j), x(T, A(j, i), k)) == demand(T, i, k);
    capacity_constraint(T + 1, A) = sum(K, x(T, A, K)) <= y(A) * U(A) + z(T);


    demand_constraint.setName("dcons"); // These names are not going there for some reason!!!!
    capacity_constraint.setName("ccons");

    m.setObjective(sum(K, sum(A, x(T.last(), A, K) * C(A))) + sum(A, F(A) * y(A)) + 1000 * z(T.last()));

    m.attach();
    m.Solver->writeLp((folder + ".\\200_simple_lp").c_str());
    //m.Solver->writeLp(".\\simpleProblemTest");
    std::cout << "Writing SMPS" << std::endl;

    //m.getSmi()->writeSmps(".\\StochasticNetworkDesign_smps", false, false);
    m.getSmi()->writeSmps((folder + ".\\200_simple_smps").c_str(), true, false);

    // ---------------------------------------
    // ---------- use PNB solver -------------
    // ---------------------------------------

    OsiCpxSolverInterface osi_cpx;
    OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, m.getSmi()); //side effect: smi osi solver object is recreated (old one is deleted)
    m.setSolver(m.getSmi()->getOsiSolverInterface()); // therefore model solver object needs to be reset

    m.setStochasticSolver(&nested_benders);

    // Declare parameter file
    parallel_benders::Parameter benders_param;
    benders_param.init();
    benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_DETEQ);
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
    */

}
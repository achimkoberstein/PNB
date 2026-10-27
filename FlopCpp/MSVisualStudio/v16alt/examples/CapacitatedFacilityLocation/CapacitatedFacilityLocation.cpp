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


static double HUGE_COST = 10000.00;
static int NULL_DEMAND = 0;

int main(int argc, char* argv[]) {
#ifdef USE_GUROBI
    MP_model m(new OsiGrbSolverInterface());
#else 
    MP_model m(new OsiCpxSolverInterface());
#endif



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

    demand_constraint(j) = sum(facilities(i), x(i, j)) == dc();
    capacity_constraint(i) = sum(clients(j), P(A(i,j))*x(i,j)) <= capacity*y(i);

    demand_constraint.setName("dcons");
    capacity_constraint.setName("ccons");

    m.setObjective( sum(facilities(i), fixed_cost*y(i)) +  sum(facilities(i), sum(clients(j), G(A(i,j))*x(i,j))) );
    m.attach();

    double cpxtime = CoinCpuTime();
    m.solve(MP_model::MINIMIZE);
    cpxtime = CoinCpuTime() - cpxtime;

    m.Solver->writeMps(".\\solution");
    m.Solver->writeLp(".\\solution");

    cout << "Optimal objective: " << m.Solver->getObjValue() << endl;
   // x.display("Display x:\n");
    y.display("Display y:\n");

}
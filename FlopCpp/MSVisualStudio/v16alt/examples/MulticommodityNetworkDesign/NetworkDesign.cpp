#include "flopc.hpp"
#include "CoinTime.hpp"
using namespace flopc;
using namespace std;
#include <OsiCpxSolverInterface.hpp>
#include <OsiGrbSolverInterface.hpp>
#include <iostream>
#include <fstream>
#include <string>



int main(int argc, char* argv[]) {
    //MP_model::getDefaultModel().setSolver(new OsiCpxSolverInterface);
    //MP_model m(new OsiCpxSolverInterface);
    MP_model m(new OsiGrbSolverInterface);

    //std::string fname = "simple_example.dow";
    std::string fname = "r01.1.dow";
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
        cout<< " " << end_node << " " << start_node << " " << transportation_cost << " " << capacity << " " << design_cost << endl;
        arcData[i][0] = end_node;
        arcData[i][1] = start_node;
        arcData[i][2] = transportation_cost;
        arcData[i][3] = capacity;
        arcData[i][4] = design_cost;
    }

    using Array3 = std::array<double, 3>;
    std::vector<Array3> commodityData(numcoms);

    int origin, destination, demand;
    for (int i = 0; i < numcoms; i++) {
        file >> origin >> destination >> demand;

        cout << origin << " " << destination << " " << demand << endl;

        commodityData[i][0] = origin - 1;
        commodityData[i][1] = destination - 1;
        commodityData[i][2] = demand;
    }
    file.close();


    MP_subset<2> A(N, N); //arcs
    for (int i = 0; i < numarcs; i++) {
        A.insert(arcData[i][0] - 1, arcData[i][1] - 1);
    }
    A.display();

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
    MP_data d(K); //demand

    for (int k = 0; k < numcoms; k++) {
        O(k) = commodityData[k][0];
        D(k) = commodityData[k][1];
        d(k) = commodityData[k][2];
    }

    MP_data demand_cons(K,N);
  
    for (int i = 0; i < numnodes; i++) {
        for (int k = 0; k < numcoms; k++) {
            if (i == O(k)) {
                demand_cons(k,i) = d(k);
            }
            else if (i == D(k)) {
                demand_cons(k,i) = -d(k);
            }
            else {
                demand_cons(k,i) = 0;
            }
        }
    }

    file.close();
    O.display();
    D.display();
    d.display();
    
    demand_cons.display();

    MP_variable	x(A,K); //transportation cost variable
    MP_binary_variable	y(A); //design cost variable
 
    x.setName("x");
    x.setName("y");

   
    MP_constraint
        demand_constraint(N, K), //demand constraint for each node should be satisfied      
        capacity_constraint(A, K);

    MP_index i, j, k;
    
    demand_constraint(i, k) = sum(N(j), x(A(i, j), k)) - sum(N(j), x(A(j, i), k)) == demand_cons(k,i);    
    capacity_constraint(A, K) = sum(K, x(A, K)) <= y(A) * U(A);  

    

    m.setObjective(sum(K, sum(A, x(A, K) * C(A))) + sum(A, F(A) * y(A)));
    m.attach(m.Solver);

    //m.Solver->writeLp(".\\Network1");
    double solve_time = CoinCpuTime();
    m.solve(MP_model::MINIMIZE);
    solve_time = CoinCpuTime() - solve_time;

    std::cout << "Solve time = " << solve_time << std::endl;
    

    m.Solver->writeMps(".\\Network");
    m.Solver->writeLp(".\\Network");
    cout << "Optimal objective: " << m.Solver->getObjValue() << endl;
    x.display();
    //y.display();

}


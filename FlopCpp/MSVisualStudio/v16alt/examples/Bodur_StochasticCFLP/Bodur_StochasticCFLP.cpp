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


std::vector<double> split(const std::string& str, char delimiter) {
    std::vector<double> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(stod(str.substr(start, end - start)));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    tokens.push_back(stod(str.substr(start)));
    return tokens;
}


double getMaxValue(const std::vector<double>& values) {
    if (values.empty()) {
        throw std::invalid_argument("Vector is empty.");
    }
    return *std::max_element(values.begin(), values.end());
}




int main(int argc, char* argv[]) {

    static int maxScenarios = 5000;
    //Use first K rows for scenarios!
    static int numScenarios = 250;
    static int numCustomers = 50;



    OsiSolverInterface* osi_solver = new OsiCpxSolverInterface();
    MP_model m(osi_solver);

    enum { stage1, stage2, numStages };
    MP_stage ST(numStages);
    ST.setName("ST");
    MP_scenario_set scen(numScenarios);
    scen.setName("Scen");


    std::string folder = "Data/";
    //std::string fname = "cap101.dat";
    std::string fname = "cap102.dat";
    //std::string fname = "cap103.dat";
    //std::string fname = "cap104.dat";
    

    std::ifstream file((folder + fname).c_str());
    std::string line;


    std::getline(file, line);
    line = line.substr(1, line.length() - 2);
    std::vector<double> f_values = split(line, ',');
    

    int numFacilities = f_values.size();
    MP_set I(numFacilities); // I - facilities
    MP_set J(numCustomers);  // J - customers


    MP_data F(I);
    for (int i = 0; i < numFacilities; i++) {
        F(i) = f_values[i];
    }



    std::getline(file, line);
    line = line.substr(1, line.length() - 2);
    std::vector<double> q_values = split(line, ',');
    cout << q_values.size() << endl;

    MP_data Q(I, J);

   
    for (int i = 0; i < numFacilities; i++) {
        for (int j = 0; j < numCustomers; j++) {
            cout << q_values[i * numCustomers + j] << endl;
            Q(i, j) = q_values[i * numCustomers + j];
        }
    }
   
  

    std::vector<double> t_array((numStages - 1) * (numFacilities + numCustomers) * numFacilities);
    for (int i = 0; i < numFacilities + numCustomers; i++) {
        std::getline(file, line);
        line = line.substr(1, line.length() - 3);
        if (i == 0) { line = line.substr(1, line.length()); }
       
        std::vector<double> t_values = split(line, ',');
        for (int j = 0; j < numFacilities; j++) {
            t_array[(0 * numFacilities + j) * (numFacilities + numCustomers) + i] = t_values[j];
        }
    }


    std::vector<double> w_array((numStages - 1) * (numFacilities + numCustomers) * (numFacilities * numCustomers));
    for (int i = 0; i < numFacilities + numCustomers; i++) {
        std::getline(file, line);
        line = line.substr(1, line.length() - 3);
        if (i == 0) { line = line.substr(1, line.length()); }

        std::vector<double> w_values = split(line, ',');
        for (int j = 0; j < numFacilities * numCustomers; j++) {
            w_array[(0 * (numFacilities * numCustomers) + j) * (numFacilities + numCustomers) + i] = w_values[j];
        }
    }


    //Taking only first K rows of the scenario set
    std::vector<double> lambda_array((numStages - 1) * numScenarios * (numCustomers));
    std::vector<double> lambda_sum_array(numScenarios);
    for (int i = 0; i < numScenarios; i++) {
        std::getline(file, line);
        line = line.substr(1, line.length() - 3);
        if (i == 0) { line = line.substr(1, line.length()); }

        std::vector<double> lambda_values = split(line, ',');
       
        double individual_sum = 0.0;
        for (int j = 0; j < numCustomers; j++) {
            lambda_array[(0 * (numCustomers) + j) * numScenarios + i] = lambda_values[j];
            individual_sum += lambda_values[j];
        }
        lambda_sum_array[i] = individual_sum;
    }

    for (int i = 0; i < maxScenarios - numScenarios; i++) {
        std::getline(file, line);
    }
    MP_random_data lambda(&lambda_array[0], ST, J);



    std::vector<double> ub_array((numStages - 1) * numFacilities);
    std::getline(file, line);
    line = line.substr(1, line.length() - 2);
    cout << line << endl;
    std::vector<double> ub_values = split(line, ',');
    for (int i = 0; i < numFacilities; i++) {
        ub_array[i] = ub_values[i];
    }



    //A -> s_i (capacity of a facility)
    MP_data S(I);
    std::getline(file, line);
    line = line.substr(2, line.length() - 4);
    std::vector<double> s_values = split(line, ',');
    for (int i = 0; i < numFacilities; i++) {
        S(i) = s_values[i];
    }



    MP_binary_variable x(I); //1 if facility I is opened
    MP_variable y(ST, I, J); //how much facility I gives to client J in scenario K


    //f - yes
    //x - yes
    //q_ij - yes but it is same for one scenario
    //y - yes
    //s_i - yes
    //lambda_j - yes

    lambda.display();


    MP_constraint demand_constraint(ST, J);
    MP_constraint opened_facilities_constraint(ST, I);
    MP_constraint recourse_constraint;

    MP_index i, j;


    //For some reason the function maximum(ST, sum(J(j), lambda(ST, j))); does not return the max element, but just the average
    //It doesn't affect the solution, the result is correct, but the .lp version of the problem looks incorrect.
    MP_data max_lambda;
    max_lambda() = *max_element(lambda_sum_array.begin(), lambda_sum_array.end());



    demand_constraint(ST + 1, j) = sum(I(i), y(ST, i, j)) >= lambda(ST, j);
    opened_facilities_constraint(ST + 1, i) = sum(J(j), y(ST, i, j)) <= S(i) * x(i);
    //recourse_constraint = sum(I(i), S(i) * x(i)) >= maximum(ST, sum(J(j), lambda(ST, j)));
    recourse_constraint = sum(I(i), S(i) * x(i)) >= max_lambda();

    m.setObjective(sum(I(i), F(i)* x(i)) + sum(ST, sum(I(i), sum(J(j), Q(i, j) * y(ST + 1, i, j)))));
    m.attach();
    m.Solver->writeLp(".\\simpleProblemTest");


    OsiCpxSolverInterface osi_cpx;
    OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, m.getSmi()); //side effect: smi osi solver object is recreated (old one is deleted)
    m.setSolver(m.getSmi()->getOsiSolverInterface()); // therefore model solver object needs to be reset

    m.setStochasticSolver(&nested_benders);

    // Declare parameter file
    parallel_benders::Parameter benders_param;
    benders_param.init();
    benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_DETEQ);
    benders_param.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL);

    //benders_param.parameter_.put(parallel_benders::PNB_PRINT_SUBPROBLEMS, 1);

    // To set a parameter look in NestedBendersTest.cpp
    m.StochSolver->setParameters(benders_param);

    double pnbtime = CoinCpuTime();
    m.StochSolver->initialSolve();
    pnbtime = CoinCpuTime() - pnbtime;

    std::cout << "Objective-Value " << nested_benders.getObjValue() << std::endl;

    x.display();
   // y.display();


    cout << "kek" << endl;


}
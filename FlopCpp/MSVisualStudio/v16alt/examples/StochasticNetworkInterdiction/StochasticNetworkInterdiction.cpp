#include "flopc.hpp"
using namespace flopc;
using std::cout;
using std::endl;
#include "SmiScnModel.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
#include "CoinTime.hpp"


#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include<vector>





//snipno : {3, 4}
int SNIP_NO = 3;

//b : {30, 40, 50, 60, 70, 80, 90}
static int B = 30;

//c: cost, is always equal to 1
static int C = 1;

//instanceNo: {0, 1, 2, 3, 4}
static int instanceNo = 0;





int main(int argc, char* argv[]) {


    /*int B = 2;

    int numNodes = 10;
    int numArcs = 12;

    int numScenarios = 4;
    int binaryVariables = 3;

    int no_vendors_num_lines = 9;
    int with_vendors_num_lines = 3;
    */
    
    
    std::string folder = "Data/";
    //std::string folder = "Data_simple/";

    
    int numNodes = 783;
    int numArcs = 2586;

    int numScenarios = 456;
    int binaryVariables = 320;

    int no_vendors_num_lines = 2266;
    int with_vendors_num_lines = 320;
    

    //Note: The nodes are not indexed in a consecutive manner. They take values 
    //from the set {1,2,...,38,91,92,...183,191,192,...830,851,852,...,862}.
    //1-38 - origin nodes
    //851-862 - destination nodes
    //91-183,181-830 - connection nodes???
    //UPDATE: I made them consecutive because otherwise MP_set doesn't take them.
    
    std::map<int, int> mapping;

    // Define the source and target ranges
    struct Range {
        int src_start;
        int src_end;
        int tgt_start;
    };

    
    Range ranges[] = {
        {1, 38, 1},       // 1-38 -> 1-38
        {91, 183, 39},    // 91-183 -> 39-131
        {191, 830, 132},  // 191-830 -> 132-771
        {851, 862, 772}   // 851-862 -> 772-783
    };

    
    /*Range ranges[] = {
        {1, 10, 1},       // 1-10 -> 1-10
    };*/

    // Fill the map
    for (const auto& r : ranges) {
        for (int i = 0; i <= r.src_end - r.src_start; ++i) {
            int src = r.src_start + i;
            int tgt = r.tgt_start + i;
            mapping[src] = tgt;
        }
    }


    OsiSolverInterface* osi_solver = new OsiCpxSolverInterface();
    MP_model m(osi_solver);
    cout << std::setprecision(14);


    


    //Set of arcs A/D (arcs on which vendors are NOT allowed to be placed
    //Every row represents an arc (i,j) of A/D and consists of i, j, r_ij
    std::string no_vendors_fname = "arcgain" + std::to_string(instanceNo)  + ".txt";
    std::ifstream arcs_no_vendors_file((folder + no_vendors_fname).c_str());


    //Provides the set D
    //Every row represents an arc (i,j) of D and consists of i, j, r_ij and q_ij
    //For snipno = 3, q_ij = 0.1*r_ij 
    //For snipno = 4, q_ij = 0, for all (i,j) of D.
    std::string with_vendors_fname = "intd_arc" + std::to_string(instanceNo)  + ".txt";
    cout << with_vendors_fname << endl;
    std::ifstream arcs_with_vendors_file((folder + with_vendors_fname).c_str());


    cout << no_vendors_num_lines << endl;
    cout << with_vendors_num_lines << endl;
    cout << no_vendors_num_lines + with_vendors_num_lines << endl;

    //Reading from the beginning of the files
    arcs_no_vendors_file.seekg(0, ios_base::beg);
    arcs_with_vendors_file.seekg(0, ios_base::beg);


    MP_set N(numNodes);




    int start_node, end_node;
    double probability_with_vendor, probability_without_vendor;

    using Array3 = std::array<double, 3>;
    std::vector<Array3> arcData_noVendors(no_vendors_num_lines);


    using Array4 = std::array<double, 4>;
    std::vector<Array4> arcData_withVendors(with_vendors_num_lines);



    int count = 0;
    while (arcs_no_vendors_file >> start_node >> end_node >> probability_without_vendor) {
        //cout << " " << start_node << " " << end_node << " " << " " << probability_without_vendor << endl;
        arcData_noVendors[count][0] = mapping[start_node];
        arcData_noVendors[count][1] = mapping[end_node];
        arcData_noVendors[count][2] = probability_without_vendor;
        count = count + 1;
    }
    arcs_no_vendors_file.close();


    count = 0;
    while (arcs_with_vendors_file >> start_node >> end_node >> probability_without_vendor >> probability_with_vendor) {
        //cout << " " << start_node << " " << end_node << " " << probability_without_vendor << " " << probability_with_vendor << endl;
        arcData_withVendors[count][0] = mapping[start_node];
        arcData_withVendors[count][1] = mapping[end_node];
        arcData_withVendors[count][2] = probability_without_vendor;
        arcData_withVendors[count][3] = probability_with_vendor;
        count = count + 1;
    }
    arcs_with_vendors_file.close();




    //Set A/D
    MP_subset<2> A(N, N);
    for (int i = 0; i < no_vendors_num_lines; i++) {
        A.insert(arcData_noVendors[i][0] - 1, arcData_noVendors[i][1] - 1);
    }

    MP_data R(A); //probability of avoiding detection with sensor installed

    for (int i = 0; i < no_vendors_num_lines; i++) {
        R(A(arcData_noVendors[i][0] - 1, arcData_noVendors[i][1] - 1)) = arcData_noVendors[i][2];
    }


    //Set D
    MP_subset<2> D(N, N);
    for (int i = 0; i < with_vendors_num_lines; i++) {
        //cout << arcData_withVendors[i][0] - 1 << " , " << arcData_withVendors[i][1] - 1 << endl;
        D.insert(arcData_withVendors[i][0] - 1, arcData_withVendors[i][1] - 1);
    }

    MP_data R_d(D); //probability of avoiding detection with sensor installed
    MP_data Q_d(D); //probability of avoiding detection without sensor installed

    for (int i = 0; i < with_vendors_num_lines; i++) {
        R_d(D(arcData_withVendors[i][0] - 1, arcData_withVendors[i][1] - 1)) = arcData_withVendors[i][2];
    }


    if (SNIP_NO == 1) {
        for (int i = 0; i < with_vendors_num_lines; i++) {
            Q_d(D(arcData_withVendors[i][0] - 1, arcData_withVendors[i][1] - 1)) = arcData_withVendors[i][3];
        }
    }
    else if(SNIP_NO == 3) {
        for (int i = 0; i < with_vendors_num_lines; i++) {
            Q_d(D(arcData_withVendors[i][0] - 1, arcData_withVendors[i][1] - 1)) = 0.1 * arcData_withVendors[i][2];
        }
    }
    else if (SNIP_NO == 4) {
        for (int i = 0; i < with_vendors_num_lines; i++) {
            Q_d(D(arcData_withVendors[i][0] - 1, arcData_withVendors[i][1] - 1)) = 0;
        }
    }
    else {}

    //R.display();

    //------------------------------------------------------------------
    //---------------------------Scenario set: -------------------------
    //------------------------------------------------------------------

    std::string scen_fname = "Scenarios.txt";
    std::ifstream scenfile((folder + scen_fname).c_str());

    enum { stage1, stage2, numStages };
    MP_stage T(numStages);
    T.setName("T");

    MP_scenario_set scen(numScenarios);
    scen.setName("Scen");



std::vector<double> dest_array((numStages - 1)* numNodes* numScenarios);
std::vector<double> or_array((numStages - 1)* numNodes* numScenarios);

for (int n = 0; n < numNodes; n++) {
    for (int i = 0; i < numScenarios; i++) {        
        dest_array[(0 * numNodes + n) * numScenarios + i] = 0;
        or_array[(0 * numNodes + n) * numScenarios + i] = 0;
    }
}




MP_data prob(scen);

MP_subset<1> origins(N);
MP_subset<1> destinations(N);

int or , dest;
double pr;
count = 0;
while (scenfile >> or >> dest >> pr) {
    cout << " " << or << " " << dest << " " << pr << endl;

    origins.insert((mapping[or] - 1));
    destinations.insert((mapping[dest] - 1));


    dest_array[(mapping[dest] - 1) * numScenarios + count] = 1;
    or_array[(mapping[or] -1) * numScenarios + count] = 1;    
    prob(count) = pr;

    count = count + 1;
}
scenfile.close();

//origins.display();
//destinations.display();


for (int n = 0; n < numNodes; n++) {
    for (int i = 0; i < numScenarios; i++) {
        if (or_array[(0 * numNodes + n) * numScenarios + i] > 0) {
            cout << "Node " << n << ", scenario " << i << ": " << endl;
            cout << "Origin:" << or_array[(0 * numNodes + n) * numScenarios + i] << endl;
        } 
    }
}


for (int n = 0; n < numNodes; n++) {
    for (int i = 0; i < numScenarios; i++) {
        if (dest_array[(0 * numNodes + n) * numScenarios + i] > 0) {
            cout << "Node " << n << ", scenario " << i << ": " << endl;
            cout << "Destination:" << dest_array[(0 * numNodes + n) * numScenarios + i] << endl;
        }
    }
}
prob.display();
m.setProbabilities(prob);


MP_random_data destination(&dest_array[0], T, N);
destination.setName("destination");

MP_random_data origin(&or_array[0], T, N);
origin.setName("origin");





std::string psi_fname = "psi.txt";
std::ifstream psifile((folder + psi_fname).c_str());

//psi.txt provides KxN matrix for psi values
// in our case, K is equal to 456, N is equal to 783.
//vector<vector<double>> psi_array(numScenarios, vector<double>(numNodes));
std::vector<double> psi_array((numStages - 1) * numNodes * numScenarios);
std::string psi_str;
double psi_val;

for (int i = 0; i < numScenarios; i++) {
    for (int j = 0; j < numNodes; j++) {
        psifile >> psi_str;
        if (i == 0 && j == 0) { psi_str = psi_str.substr(1, psi_str.length()); }
        if (j == 0) { psi_str = psi_str.substr(1, psi_str.length()); }

        std::stringstream psistream(psi_str);
        psistream >> psi_val;


        psi_array[(0 * numNodes + j) * numScenarios + i] = psi_val;

        if (psi_array[(0 * numNodes + j) * numScenarios + i] == 0) {
            cout << psi_str << endl;
            cout << "Null value at (" << i << ", " << j << ");\n";
        }
    }
}
psifile.close();


for (int j = 0; j < numNodes; j++) {
    double sum = 0;
    for (int i = 0; i < numScenarios; i++){
        
        sum = sum + psi_array[(0 * numNodes + j) * numScenarios + i];
    }
    double avg = sum / numScenarios;
    sum = 0;
    cout << "Node " << j << ", average: " << avg << endl;
}


  
    cout << "SCEN DEMAND:" << endl;

    MP_random_data psi(&psi_array[0], T, N);
    psi.setName("psi");



   MP_constraint budget_constraint;           //total cost of installing sensors does not exceed budget       
   MP_constraint lower_bound_constraint(T, destinations);
   //MP_constraint upper_bound_constraint(T, N);      //for every scenario, probability that the robber can go 
                                                   //from t_k to t_k is equal to 1 
    
    MP_constraint probability_constraint2(T, D, destinations);   //pi is at least the probability that the robber can travel from
    MP_constraint probability_constraint3(T, A, destinations);   //i to j undetected times the probability that the robber can 
    MP_constraint probability_constraint4(T, D, destinations);   //travel from j to t^k undetected
    
    MP_index i, j, d;
  

    MP_binary_variable x(D); //first stage - whether vendor is going to be placed or not
    x.setName("x");

    /*MP_variable pi(T, N, N); //probability that the evader can travel from i to t_k undetected
    pi.setName("pi");

    budget_constraint = sum(D, x(D) * C) <= B; //15b

    lower_bound_constraint(T + 1, i) = pi(T, i) >= lower_bound(T, i);
    upper_bound_constraint(T + 1, i) = pi(T, i) <= upper_bound(T, i);

    probability_constraint2(T + 1, D(i, j)) = pi(T, i) >= Q_d(D(i, j)) * pi(T, j);
    probability_constraint3(T + 1, A(i, j)) = pi(T, i) >= R(A(i, j)) * pi(T, j);
    probability_constraint4(T + 1, D(i, j)) = pi(T, i) - R_d(D(i, j)) * pi(T, j) >= -(R_d(D(i, j)) - Q_d(D(i, j))) * psi(T, j) * x(D(i, j));

    m.setObjective(sum(N, delta(T.last(), N)* pi(T.last(), N)));*/


    MP_variable pi(T, N, N); //probability that the evader can travel from i to j undetected
    pi.setName("pi");



    budget_constraint = sum(D, x(D) * C) <= B; //15b
    lower_bound_constraint(T + 1, destinations(d)) = pi(T, d, d) == 1; //Probability of travelling from i to i is 1.

    probability_constraint2(T + 1, D(i, j), destinations(d)) = pi(T, i, d) * destination(T, d) >= Q_d(D(i, j)) * pi(T, j, d) * destination(T, d);
    probability_constraint3(T + 1, A(i, j), destinations(d)) = pi(T, i, d) * destination(T, d) >= R(A(i, j)) * pi(T, j, d) * destination(T, d);
    probability_constraint4(T + 1, D(i, j), destinations(d)) = (pi(T, i, d) - R_d(D(i, j)) * pi(T, j, d)) * destination(T, d) >= (-(R_d(D(i, j)) - Q_d(D(i, j))) * psi(T, j) * x(D(i, j))) * destination(T, d);


    m.setObjective(sum(origins(i), sum(destinations(d), origin(T.last(), i) * destination(T.last(), d) * pi(T.last(), i, d))));


   
    
    

    //PROBLEM: We need to minimize sum of scenarios probabilities
    m.attach();
   //m.Solver->writeLp(".\\simpleProblemTest");


  
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
    //benders_param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT);
    //benders_param.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL);
   


    // To set a parameter look in NestedBendersTest.cpp
    m.StochSolver->setParameters(benders_param);

    double pnbtime = CoinCpuTime();
    m.StochSolver->initialSolve();
    pnbtime = CoinCpuTime() - pnbtime;

    std::cout << "Objective-Value " << nested_benders.getObjValue() << std::endl;
    x.display();


    cout << "kek" << endl;
}

#ifndef BendersNodeData_h__
#define BendersNodeData_h__

#include <vector>
#include <map>

#include <boost/dynamic_bitset.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/scoped_ptr.hpp>


class CoinPackedMatrix;
class CoinPackedVector;
class CoinWarmStart;
class OsiRowCut;
class SmiNodeData;

namespace parallel_benders{

    // Parameter names go in here. We use std::string as parameter
    // We start with integer parameters
    const std::string PNB_BENDERS = "PNB_BENDERS";          //
    const std::string PNB_WARM_START = "PNB_WARM_START";    //
    const std::string PNB_AGGREGATED_OPTIMALITY_CUTS = "PNB_AGGREGATED_OPTIMALITY_CUTS";    //
    const std::string PNB_SCENARIOS_PRO_CUT = "PNB_SCENARIOS_PRO_CUT" ;            //
    const std::string PNB_RELAXATION = "PNB_RELAXATION";                   //
    const std::string PNB_DYNAMIC_AGGREGATION = "PNB_DYNAMIC_AGGREGATION";          //
    const std::string PNB_TRUST_REGION = "PNB_TRUST_REGION";                 //
	const std::string PNB_NUM_CPU = "PNB_NUM_CPU"; // 0 = take what hardware_concurrency offers you, otherwise equal to the number of CPU we would like to use.
	const std::string PNB_CPU_USED = "PNB_CPU_USED"; // indicates how many CPU we used ( we have to set that from the solver)
	const std::string PNB_TREE_WALK_STRATEGY = "PNB_TREE_WALK_STRATEGY"; // Different Strategies. 0 = FastForw/FastBack, 1 = FastForw, 2 = FastBack, 3 = EpsFastForw, 4 = EpsFastBack, 5 = FastForw/FastBackWith2nd.Iteration
	const std::string PNB_TREE_WALK_SECOND_ITERATION_STAGE = "PNB_TREE_WALK_SECOND_ITERATION_STAGE";
	//const std::string PNB_TREE_WALK_STRATEGY_STAGE = "PNB_TREE_WALK_STRATEGY_STAGE"; // up to which stage do we want to follow the given strategy? else we resort to Fastpass

    // Here come the double parameters
    const std::string PNB_TOLERANCE_ZERO = "PNB_TOLERANCE_ZERO";             //
    const std::string PNB_TOLERANCE_INTEGER = "PNB_TOLERANCE_INTEGER";       //
    const std::string PNB_EPSILON_ABSOLUTE = "PNB_EPSILON_ABSOLUTE";           //
    const std::string PNB_EPSILON_RELATIVE = "PNB_EPSILON_RELATIVE";           // 


	// Some fixed integer to avoid magic number
	const int PNB_INT_FASTPASS = 0;
	const int PNB_INT_FAST_FORWARD = 1;
	const int PNB_INT_FAST_BACKWARD = 2;
	const int PNB_INT_EPSILON_FAST_FORWARD = 3;
	const int PNB_INT_EPSILON_FAST_BACKWARD = 4;
	const int PNB_INT_SECOND_ITERATION = 5;
	//const int PNB_INT_


// We can make it a struct anyway?!
struct BendersNodeData
{
public:
    BendersNodeData();
    BendersNodeData(SmiNodeData* nodeData);
    ~BendersNodeData();

    void set_col_solution(const double* colSolution);
    const double* col_solution();
    void set_dual_solution(const double* dualSolution);
    const double* dual_solution();
    void set_warm_start(CoinWarmStart* coinWarmStart);
    const CoinWarmStart* warm_start();

    // Information about the solution
    double* col_solution_;
    double* dual_solution_;
    double* next_stage_approximation_;
    double dual_value_variable_bounds_;
    bool feasible_;
    

    int cols_;
    int rows_;
    int num_thetas_;
    boost::dynamic_bitset<> active_cuts_; // Has to have the size of num_thetas_
    CoinWarmStart* warm_start_;

    SmiNodeData* smi_node_data_;
    int scenario_index_;
    int branch_scenario_;
    int num_node_in_stage_;
    int child_node_start_;
    double probability_;
    double conditional_probability_;

    double solution_value_;
    std::vector<OsiRowCut* > optimality_cuts_;
    std::vector<OsiRowCut* > feasibility_cuts_;
    // Multicut Aggregation Strategies..
    int num_scen_pro_cut_;

    std::vector< std::vector<int> > nodes_in_cut_; // Stores which subnodes of current node are aggregated to a cut.

    // Values for cut aggregation
    std::vector<double> current_cut_coefficients_;
    double current_cut_rhs_;

    CoinPackedMatrix* matrix_;
    double* column_lower_;
    double* column_upper_;
    double* objective_;
    double* row_lower_;
    double* row_upper_;
};

// Methods that work on a BendersNodeData
void createRoundRobinAggregation(BendersNodeData* benders_node_data, int num_child, int num_cuts, int num_subpropblems_per_cut);

struct BendersStageData {

    BendersStageData();
    ~BendersStageData();

    // Store problem data for each stage
    int num_columns_;
    int num_rows_;
    int stage_; // Just to be sure
    int num_thetas_; // Number of variables that represent the approximation of the next stage

    // Store information about stochasticity in stage
    bool stochastic_matrix_w_;
    bool stochastic_matrix_t_;
    bool stochastic_rhs_;
    bool stochastic_objective_;
    bool stochastic_bounds_;

    // Store pointers to matrices W and / or T if they do not change ?
    std::vector<int> column_indices_;
    std::vector<int> row_indices_;
    // Coin Warm Start object
    CoinWarmStart* warm_start_;

};

struct RowOrderedSparseMatrix{

    RowOrderedSparseMatrix(const std::vector<double>& elements, const std::vector<int>& indices, const std::vector<int>& start);
    
    // Store the matrix as vectors
    std::vector<double> elements_;
    std::vector<int> indices_;
    std::vector<int> start_;
};

struct RowOrderedSparseMatrixPointer{

    RowOrderedSparseMatrixPointer(const double* elements, const int* indices, const int* start);

    // Store the matrix as vectors
    const double* elements_;
    const int* indices_;
    const int* start_;
};

class Timer {
public:
    explicit Timer(int stgs) : stages_(stgs),start_time_(stages_,0.0),overall_time_(stages_,0.0) {}
    ~Timer() { }

    void Start(int stg);
    void Stop(int stg);

    double GetTime(int stg) { return overall_time_[stg]; }

    double GetAggregatedTime();

    std::string GetFullTimeString();

private:
    int stages_;
    std::vector<double> start_time_;
    std::vector<double> overall_time_;
};


struct Parameter {
    // Set default values for all strings we have..
    void init();


    //// Set values you want to set
    //void SetParameterValue(std::string parameter, double value){
    //    parameter_map_[parameter] = value;
    //}
    //double GetParameterValue(std::string parameter){
    //    return parameter_map_[parameter];
    //}

    // Stores every parameter as a std::string with a value that is stored as either double or int..
    boost::property_tree::ptree parameter_;
};

} // end namespace parallel_benders
#endif // BendersNodeData_h__
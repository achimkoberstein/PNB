#ifndef BendersNodeData_h__
#define BendersNodeData_h__

#include <vector>
#include <map>

#include <boost/dynamic_bitset.hpp>
#include <boost/scoped_ptr.hpp>


class CoinPackedMatrix;
class CoinPackedVector;
class CoinWarmStart;
class OsiRowCut;
class SmiNodeData;

namespace parallel_benders{


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

struct TimerStruct {
    void init(int numStages);

    std::string GetHeadingString();
    std::string GetTimeString();

    boost::scoped_ptr<Timer> total;
    boost::scoped_ptr<Timer> solver;
    boost::scoped_ptr<Timer> problemGeneration;
    boost::scoped_ptr<Timer> aggCut;
    boost::scoped_ptr<Timer> optiCut;
    boost::scoped_ptr<Timer> feasCut;
}; 

struct Parameter {
    // Set default values
    void init();


    //// Set values you want to set
    //void SetParameterValue(std::string parameter, double value){
    //    parameter_map_[parameter] = value;
    //}
    //double GetParameterValue(std::string parameter){
    //    return parameter_map_[parameter];
    //}

    // Stores every parameter as a std::string with a value that is stored as a double
    std::map<std::string,double> parameter_map_;
};

} // end namespace parallel_benders
#endif // BendersNodeData_h__
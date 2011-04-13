#ifndef BendersNodeData_h__
#define BendersNodeData_h__

#include <vector>



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
    CoinWarmStart* warm_start_;

    SmiNodeData* smi_node_data_;
    int scenario_index_;
    int branch_scenario_;
    int num_node_in_stage_;
    int child_node_start_;
    double probability_;
    double conditional_probability_;

    double solution_value_;
    double theta_;
    std::vector<OsiRowCut* > optimality_cuts_;
    std::vector<OsiRowCut* > feasibility_cuts_;

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

} // end namespace parallel_benders
#endif // BendersNodeData_h__
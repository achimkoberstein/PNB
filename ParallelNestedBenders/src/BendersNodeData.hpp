#ifndef BendersNodeData_h__
#define BendersNodeData_h__

#include <vector>

class CoinPackedMatrix;
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

    //void set_cols(const int cols);
    //void set_rows(const int rows);
    //const int cols();
    //const int rows();
    void set_col_solution(const double* colSolution);
    const double* col_solution();
    void set_dual_solution(const double* dualSolution);
    const double* dual_solution();
    void set_warm_start(CoinWarmStart* coinWarmStart);
    const CoinWarmStart* warm_start();


public:
    double* col_solution_;
    double* dual_solution_;
    int cols_;
    int rows_;
    CoinWarmStart* warm_start_;

    SmiNodeData* smi_node_data_;
    int scenario_index_;
    int branch_scenario_;	
    double conditional_probability_;

    double solution_value_;
    double theta_;
    std::vector<OsiRowCut* > optimality_cuts_;
    std::vector<OsiRowCut* > feasibility_cuts_;

    //CoinPackedMatrix* matrixT; Not sure what this is good for
    //double* dclo;
    //double* dcup;
    //double* dobj;
    //double* drlo;
    //double* drup;
};

} // end namespace parallel_benders
#endif // BendersNodeData_h__
#include "BendersNodeData.hpp"

#include "CoinWarmStart.hpp"
#include "OsiRowCut.hpp"
#include "SmiLinearData.hpp"
#include "SmiScnData.hpp"

using namespace std;

namespace parallel_benders{

BendersNodeData::BendersNodeData():col_solution_(NULL),dual_solution_(NULL),cols_(0),rows_(0),warm_start_(NULL),smi_node_data_(NULL),scenario_index_(-1),branch_scenario_(-1),optimality_cuts_(NULL),feasibility_cuts_(NULL)

{
}

BendersNodeData::BendersNodeData(SmiNodeData* nodeData):col_solution_(NULL),dual_solution_(NULL),cols_(0),rows_(0),warm_start_(NULL),smi_node_data_(nodeData),scenario_index_(-1),branch_scenario_(-1),optimality_cuts_(NULL),feasibility_cuts_(NULL)

{
}

BendersNodeData::~BendersNodeData(void)
{
    for (unsigned int i = 0; i < this->optimality_cuts_.size(); i++) {
        delete optimality_cuts_[i];
    }

    for (unsigned int i = 0; i < this->feasibility_cuts_.size(); i++) {
        delete feasibility_cuts_[i];
    }

    delete [] this->col_solution_;

    delete [] this->dual_solution_;

    delete this->warm_start_;

    //delete matrixT;

    //if (smi_node_data_->getColLowerLength()) delete [] dclo;
    //if (smi_node_data_->getColUpperLength()) delete [] dcup;
    //if (smi_node_data_->getObjectiveLength()) delete [] dobj;
    //if (smi_node_data_->getRowLowerLength()) delete [] drlo;
    //if (smi_node_data_->getRowUpperLength()) delete [] drup;
}

void BendersNodeData::set_col_solution(const double* colSolution)
{
    int n = smi_node_data_->getCore()->getNumCols(smi_node_data_->getStage()) + 1; //+1 for THETA

    if (this->col_solution_ == NULL)
    {
        this->col_solution_ = new double[n];
    }

    if (optimality_cuts_.empty())
        memcpy(this->col_solution_, colSolution, (n-1)*sizeof(double));
    else
        memcpy(this->col_solution_, colSolution, n*sizeof(double));
}

void BendersNodeData::set_dual_solution(const double* dualSolution) {
    int n = smi_node_data_->getCore()->getNumRows(smi_node_data_->getStage()) + optimality_cuts_.size()+feasibility_cuts_.size();

    if (this->dual_solution_ == NULL) {
        // first time, allocate memory
        this->dual_solution_ = new double[n];
    } else {
        delete [] this->dual_solution_;
        this->dual_solution_ = new double[n];
    }

    //realloc(this->dualSolution, n*sizeof(double));

    memcpy(this->dual_solution_, dualSolution, n*sizeof(double));
}

const double* BendersNodeData::col_solution(void)
{
    return this->col_solution_;
}

const double* BendersNodeData::dual_solution(void)
{
    return this->dual_solution_;
}

void BendersNodeData::set_warm_start(CoinWarmStart* coinWarmStart)
{
    delete this->warm_start_;
    this->warm_start_ = coinWarmStart;
}

const CoinWarmStart* BendersNodeData::warm_start(void)
{
    return this->warm_start_;
}


BendersStageData::BendersStageData()
: num_columns_(0),num_rows_(0),
stochastic_matrix_w_(false),stochastic_matrix_t_(false),stochastic_rhs_(false),stochastic_objective_(false),stochastic_bounds_(false)
{

}

BendersStageData::~BendersStageData()
{
}
} // end namespace parallel benders
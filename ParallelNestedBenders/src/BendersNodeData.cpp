#include "BendersNodeData.hpp"

#include <math.h>

#include <sstream>

#include "CoinTime.hpp"
#include "CoinHelperFunctions.hpp"
#include "CoinWarmStart.hpp"
#include "OsiRowCut.hpp"
#include "SmiLinearData.hpp"
#include "SmiScnData.hpp"

using namespace std;

namespace parallel_benders{

BendersNodeData::BendersNodeData():col_solution_(NULL),dual_solution_(NULL),next_stage_approximation_(NULL),dual_value_variable_bounds_(0.0),feasible_(true),
cols_(0),rows_(0),warm_start_(NULL),smi_node_data_(NULL),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(NULL),feasibility_cuts_(NULL),num_scen_pro_cut_(0),
matrix_(NULL),column_lower_(NULL),column_upper_(NULL),objective_(NULL),row_lower_(NULL),row_upper_(NULL)

{
}

BendersNodeData::BendersNodeData(SmiNodeData* nodeData):col_solution_(NULL),dual_solution_(NULL),next_stage_approximation_(NULL),dual_value_variable_bounds_(0.0),feasible_(true),
cols_(0),rows_(0),warm_start_(NULL),smi_node_data_(nodeData),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(NULL),feasibility_cuts_(NULL),num_scen_pro_cut_(0),
matrix_(NULL),column_lower_(NULL),column_upper_(NULL),objective_(NULL),row_lower_(NULL),row_upper_(NULL)
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

    delete [] this->next_stage_approximation_;

    delete this->warm_start_;

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
: num_columns_(0),num_rows_(0),stage_(0),num_thetas_(1),
stochastic_matrix_w_(false),stochastic_matrix_t_(false),stochastic_rhs_(false),stochastic_objective_(false),stochastic_bounds_(false),
column_indices_(0),row_indices_(0),warm_start_(NULL)
{

}

BendersStageData::~BendersStageData()
{
}

RowOrderedSparseMatrix::RowOrderedSparseMatrix(const std::vector<double>& elements, const std::vector<int>& indices, const std::vector<int>& start):
elements_(elements),indices_(indices),start_(start)
{

}

RowOrderedSparseMatrixPointer::RowOrderedSparseMatrixPointer(const double* elements, const int* indices, const int* start):
elements_(elements),indices_(indices),start_(start)
{

}

void Timer::Start( int stg )
{
    start_time_[stg] = CoinCpuTime();
}

void Timer::Stop( int stg )
{
    overall_time_[stg] += CoinCpuTime() - start_time_[stg];
}

double Timer::GetAggregatedTime()
{
    double t = 0;
    for (int i = 0; i < stages_; i++) {
        t += overall_time_[i];
    }
    return t;
}

std::string Timer::GetFullTimeString()
{
    std::stringstream s;
    s << GetAggregatedTime() << "; ";
    for (int i = 0; i < stages_; i++) {
        s << GetTime(i) << "; ";
    }
    return s.str();
}

std::string TimerStruct::GetTimeString()
{
    std::stringstream s;
    s << total->GetAggregatedTime() << solver->GetAggregatedTime() << problemGeneration->GetAggregatedTime() 
        << optiCut->GetAggregatedTime() << feasCut->GetAggregatedTime() << aggCut->GetAggregatedTime() << std::endl;

    return s.str();
}

void TimerStruct::init( int numStages )
{
    total.reset(new Timer(1));
    solver.reset(new Timer(numStages));
    problemGeneration.reset(new Timer(numStages));
    optiCut.reset(new Timer(numStages));
    aggCut.reset(new Timer(numStages));
    feasCut.reset(new Timer(numStages));
}

std::string TimerStruct::GetHeadingString()
{
    return "WallClock; Solver; BuildProblem; OptimalityCut; FeasibilityCut; AggregatCuts";
}

void Parameter::init()
{
    // DetEq or Benders
    parameter_map_["Benders"] = 1.0; // 0: No Benders, but detEq.
    // WarmStart
    parameter_map_["WarmStart"] = 1.0; // 0 = no WarmStart, 1 = WarmStart with solution from previous iteration, 2 = WarmStart with solution from previous scenario

    // Aggregation of Cuts ( or single-multi cut )
    parameter_map_["AggregatedOptimalityCuts"] = 1.0;// Integer value: How many cuts do we want to have per stage. Static! If 0, use ScenariosProCut to determine number of cuts.
    parameter_map_["ScenariosProCut"] = 0.0; // Integer value: How many scenarios do we want in a cut? If 0 = use AggregatedOptimalityCuts
    parameter_map_["Relaxation"] = 1.0; // Solved LP relaxation instead of MIP problem, if we have a MIP problem
    parameter_map_["DynamicAggregation"] = 0.0; // Do we use Dynamic aggregation?
    //parameter_map_["TrustRegion"] = 0; // Do we want to use TrustRegion TODO: Implement this

    // Tolerances
    parameter_map_["ZeroTolerance"] = std::pow(10.0,-8);
    parameter_map_["IntegerTolerance"] = std::pow(10.0,-6);
    parameter_map_["AbsoluteEpsilon"] = std::pow(10.0,-6);
    parameter_map_["RelativeEpsilon"] = std::pow(10.0,-6);
}
} // end namespace parallel benders
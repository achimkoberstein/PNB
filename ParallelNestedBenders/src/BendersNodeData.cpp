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
cols_(0),rows_(0),num_thetas_(0),active_cuts_(0),warm_start_(NULL),smi_node_data_(NULL),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(NULL),feasibility_cuts_(NULL),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),opt_cut_start_index_(0),feas_cut_start_index_(0),
matrix_(NULL),column_lower_(NULL),column_upper_(NULL),objective_(NULL),row_lower_(NULL),row_upper_(NULL)

{
}

BendersNodeData::BendersNodeData(SmiNodeData* nodeData):col_solution_(NULL),dual_solution_(NULL),next_stage_approximation_(NULL),dual_value_variable_bounds_(0.0),feasible_(true),
cols_(0),rows_(0),num_thetas_(0),active_cuts_(0),warm_start_(NULL),smi_node_data_(nodeData),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(NULL),feasibility_cuts_(NULL),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),opt_cut_start_index_(0),feas_cut_start_index_(0),
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
    if ( start_time_[stg] != 0)
        overall_time_[stg] += CoinCpuTime() - start_time_[stg];
    start_time_[stg] = 0;
}

double Timer::GetAggregatedTime() const
{
    double t = 0;
    for (int i = 0; i < stages_; i++) {
        t += overall_time_[i];
    }
    return t;
}

std::string Timer::GetFullTimeString() const
{
    std::stringstream s;
    s << GetAggregatedTime() << "; ";
    for (int i = 0; i < stages_; i++) {
        s << GetTime(i) << "; ";
    }
    return s.str();
}

void Parameter::init()
{
    // DetEq or Benders
    parameter_.put(PNB_BENDERS,1); // 0: No Benders (detEq), 1 Benders
    
	// WarmStart
    parameter_.put(PNB_WARM_START, 1); // 0 = no WarmStart, 1 = WarmStart with solution from previous iteration, 2 = WarmStart with solution from previous scenario

    //// Aggregation of Cuts ( or single-multi cut )
    parameter_.put(PNB_AGGREGATED_OPTIMALITY_CUTS, 1);// Integer value: How many cuts do we want to have per stage. Static! If 0, use ScenariosProCut to determine number of cuts.
    parameter_.put(PNB_SCENARIOS_PRO_CUT, 0); // Integer value: How many scenarios do we want in a cut? If 0 = use AggregatedOptimalityCuts
	parameter_.put(PNB_DYNAMIC_AGGREGATION, 0); // Do we use Dynamic aggregation? And which strategy?
	// MIP Stuff
    parameter_.put(PNB_RELAXATION, 0); // Solved LP relaxation instead of MIP problem, if we have a MIP problem
	// Multistage strategies
    parameter_.put(PNB_TREE_WALK_STRATEGY,0);
	parameter_.put(PNB_TREE_WALK_STRATEGY_STAGE,1); // Only applicable, if we use TREE_WALK_STRATEGY=5. FirstStage = 0.
    parameter_.put(PNB_TREE_WALK_STRATEGY_EPSILON,std::pow(10.0,-6)); 
    parameter_.put(PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_CASCADE,false);
    parameter_.put(PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_REPEAT,1);
	// Parallel Parameters
	parameter_.put(PNB_NUM_CPU,0);
	parameter_.put(PNB_CPU_USED,0);
	// Acceleration Techniques
    parameter_.put(PNB_TRUST_REGION, 0); // Do we want to use TrustRegion TODO: Implement this


    //// Tolerances
    parameter_.put(PNB_TOLERANCE_ZERO, std::pow(10.0,-8));
	parameter_.put(PNB_TOLERANCE_INTEGER, std::pow(10.0,-6));
    parameter_.put(PNB_EPSILON_ABSOLUTE, std::pow(10.0,-6));
    parameter_.put(PNB_EPSILON_RELATIVE, std::pow(10.0,-6));
}

void createRoundRobinAggregation( BendersNodeData* benders_node_data, int num_child, int num_cuts, int num_subproblems_per_cut )
{
	std::vector<std::vector<int> >& node_in_cut_vector(benders_node_data->nodes_in_cut_);
	if ( node_in_cut_vector.empty() ){
		// Fill the vector according to the number of cuts according to a "strategy"
		// It may well be the case, that more information is needed, as we go along
		// Round-Robin
		int overall_start_index = benders_node_data->child_node_start_;
		int overall_end_index = overall_start_index+num_child; 
		for (int i = 0; i < num_cuts; i++){
			node_in_cut_vector.push_back(std::vector<int>());
			node_in_cut_vector.back().reserve(num_subproblems_per_cut);
			for ( int j = overall_start_index+i*num_subproblems_per_cut; j < overall_start_index+(i+1)*num_subproblems_per_cut; j++){
				if ( j < overall_end_index){ // necessary for the last cut, as it may not fit
					node_in_cut_vector.back().push_back(j);
				}
			}
		}
	}
	else{
		// Rearrange it. 

		//TransformOptimalityCutAggregation( benders_node_data, node_in_cut_vector );
		assert(false);
	}
}

} // end namespace parallel benders
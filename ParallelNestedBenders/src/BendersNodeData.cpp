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

BendersNodeData::BendersNodeData():col_solution_(NULL),dual_solution_(NULL),reduced_cost_(NULL),next_stage_approximation_(NULL),dual_value_variable_bounds_(0.0),feasible_(true),
cols_(0),rows_(0),num_thetas_(0),active_cuts_(0),warm_start_(NULL),smi_node_data_(NULL),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),cut_information_(*this),optimality_cuts_(NULL),feasibility_cuts_(NULL),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),opt_cut_start_index_(0),feas_cut_start_index_(0),
matrix_(NULL),column_lower_(NULL),column_upper_(NULL),objective_(NULL),row_lower_(NULL),row_upper_(NULL),no_staircase_constraints_()

{
}

BendersNodeData::BendersNodeData(SmiNodeData* nodeData):col_solution_(NULL),dual_solution_(NULL),reduced_cost_(NULL),next_stage_approximation_(NULL),dual_value_variable_bounds_(0.0),feasible_(true),
cols_(0),rows_(0),num_thetas_(0),active_cuts_(0),warm_start_(NULL),smi_node_data_(nodeData),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),cut_information_(*this),optimality_cuts_(NULL),feasibility_cuts_(NULL),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),opt_cut_start_index_(0),feas_cut_start_index_(0),
matrix_(NULL),column_lower_(NULL),column_upper_(NULL),objective_(NULL),row_lower_(NULL),row_upper_(NULL),no_staircase_constraints_()
{

}

BendersNodeData::~BendersNodeData(void)
{
    //for (cut_container::iterator it = optimality_cuts_.begin(); it != optimality_cuts_.end(); ++it) {
    //    delete *it; 
    //}
    //for (cut_container::iterator it = feasibility_cuts_.begin(); it != feasibility_cuts_.end(); ++it) {
    //    delete *it; 
    //}

    delete [] this->col_solution_;

    delete [] this->dual_solution_;

    delete [] this->next_stage_approximation_;

    delete [] this->reduced_cost_;

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
stochastic_matrix_w_(false),stochastic_matrix_t_(false),stochastic_rhs_(false),stochastic_objective_(false),stochastic_bounds_(false),non_zero_bounds_(false),
column_indices_(0),row_indices_(0),warm_start_(NULL),osi_solver_map_(),no_staircase_constraints_()
{

}

BendersStageData::~BendersStageData()
{
    delete warm_start_;
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

int Timer::GetTimeStages() const
{
    return stages_;
}
void Parameter::init()
{
    // DetEq or Benders
    parameter_.put(PNB_BENDERS,1); // 0: No Benders (detEq), 1 Benders
    
	// WarmStart
    parameter_.put(PNB_WARM_START, 1); // 0 = no WarmStart, 1 = WarmStart with solution from previous iteration, 2 = WarmStart with solution from previous scenario (in the same iteration)

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
    // Stage Aggregation
    parameter_.put(PNB_STAGE_AGGREGATION,false);
    parameter_.put(PNB_STAGE_AGGREGATION_START,0);
    parameter_.put(PNB_STAGE_AGGREGATION_END,0);
    parameter_.put(PNB_STAGE_AGGREGATION_DIRECTION,true); // Direction == true : Aggregate from start
    // Start solution
    parameter_.put(PNB_USE_START_SOLUTION,0); // 0: no start solution, 1 EV solution, 2 ? not yet implemented
	// Parallel Parameters
	parameter_.put(PNB_NUM_CPU,0);
	parameter_.put(PNB_CPU_USED,0);
    // Maximal solution time
    parameter_.put(PNB_MAXIMAL_SOLUTION_TIME,3600); // In Minutes
	// Acceleration Techniques
    parameter_.put(PNB_TRUST_REGION, 0); // Do we want to use TrustRegion TODO: Implement this
    //// Tolerances: We should use the same interger and zero tolerances as the underyling LP-solver
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


CutManager::CutManager( BendersNodeData& benders_node) : node_data_ref_(benders_node),cut_informations_(),cut_counter_(0)
{
   
}


CutManager::~CutManager()
{

}

void CutManager::CreateCut( int iteration )
{
    cut_informations_.push_back( CutInformation() );
    cut_informations_.back().cut_creation_iteration_ = iteration;
    cut_counter_++;
       
}

CutInformation& CutManager::GetCut( int index )
{
    return cut_informations_.at(index);
}

void CutManager::DeleteCut( int index )
{
    cut_informations_.erase(cut_informations_.begin()+index);
    cut_counter_--;
}

int CutManager::NumCuts()
{
    return cut_counter_;
}
CutInformation::CutInformation() : cut_creation_iteration_(0),active_iteration_counter_(0),inactive_iteration_counter_(0),iteration_forward_active_vector_(),iteration_backward_active_vector_()
{

}

double computeEuclideanNorm( OsiRowCut* cut )
{
    double sum = 0;
    int length = cut->row().getNumElements();
    const double* elements = cut->row().getElements();
    for ( int i = 0; i < length; i++){
        sum += elements[i]*elements[i];
    }
    return sqrt(sum);
}

double computeCutViolation( OsiRowCut* cut, double* solution, double infinity ){
    int i;
    double sum = 0.0;
    const int* column = cut->row().getIndices();
    int number = cut->row().getNumElements();
    const double* element = cut->row().getElements();
    for ( i = 0;  i < number;  i++ ) {
        int colIndx = column[i];
        sum += solution[colIndx] * element[i];
    }
    // Test for infinity and stuff
    if ( sum > cut->ub() )
        return sum-cut->ub();
    else if ( sum < cut->lb() )
        return cut->lb()-sum;
    else if ( cut->ub() >= infinity ) // Cut is not violated!
        return cut->lb()-sum;
    else if ( cut->lb() <= -infinity) // Cut is not violated
        return sum-cut->ub();
    else // Both values are set, we do not know what to do with this!
        return 0.0;
}

CutStuff::CutStuff( int iteration, boost::shared_ptr<OsiRowCut>& cut_pointer): cut_creation_iteration_(iteration),aggregate_number_(-1),cut_(cut_pointer),active_value(1.0)
{
}

bool is_active( const boost::shared_ptr<CutStuff>& value )
{
    return value->active_value < 0;
}
} // end namespace parallel benders
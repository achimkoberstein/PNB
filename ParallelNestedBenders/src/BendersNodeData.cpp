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

BendersNodeData::BendersNodeData():col_solution_(),dual_solution_(),incumbent_col_solution_(),reduced_cost_(),next_stage_approximation_(),discrepancy_(nullptr),was_under_threshold_(),dual_value_variable_bounds_(0.0),feasible_(true),unbounded_(false),unbounded_row_(-1),lower_bound_(-1e20),upper_bound_(1e20),
mip_solution_(nullptr),unbounded_problem_(false),unbounded_extreme_ray_(nullptr),unbounded_right_hand_side_(nullptr),solve_ray_right_hand_side_(false),cols_(0),rows_(0),num_thetas_(0),active_aggregates_(0),warm_start_(),warm_start_added_(false),level_warm_start_(nullptr),level_warm_start_cstat_(nullptr),level_warm_start_rstat_(nullptr),level_warm_start_dnorm_(nullptr),level_warm_start_length_(0),level_regularized_solution_(),level_regularized_start_row_(-1),
smi_node_data_(nullptr),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(),feasibility_cuts_(),last_iteration_start_optimality_(optimality_cuts_.end()),last_iteration_start_feasibility_(feasibility_cuts_.end()),level_opt_iterator_(optimality_cuts_.end()),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),alternating_intra_cut_adaptive_(true),
matrix_(nullptr),column_lower_(nullptr),column_upper_(nullptr),objective_(nullptr),row_lower_(nullptr),row_upper_(nullptr),no_staircase_constraints_(),d_square_cut_t_matrix_(nullptr),d_square_cut_rhs_(),solver_rhs_(nullptr),can_safely_reuse_(true),oda_optimality_cuts_(),oda_cut_evaluation_(),oda_lambdas_(),build_time_(0.0),solution_time_(0.0)

{

}

BendersNodeData::BendersNodeData(SmiNodeData* nodeData):col_solution_(),dual_solution_(),incumbent_col_solution_(),reduced_cost_(),next_stage_approximation_(),discrepancy_(nullptr),was_under_threshold_(),dual_value_variable_bounds_(0.0),feasible_(true),unbounded_(false),unbounded_row_(-1),lower_bound_(-1e20),upper_bound_(1e20),
mip_solution_(nullptr),unbounded_problem_(false),unbounded_extreme_ray_(nullptr),unbounded_right_hand_side_(nullptr),solve_ray_right_hand_side_(false),cols_(0),rows_(0),num_thetas_(0),active_aggregates_(0),warm_start_(nullptr),warm_start_added_(false),level_warm_start_(nullptr),level_warm_start_cstat_(nullptr),level_warm_start_rstat_(nullptr),level_warm_start_dnorm_(nullptr),level_warm_start_length_(0),level_regularized_solution_(),level_regularized_start_row_(-1),
smi_node_data_(nodeData),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(),feasibility_cuts_(),last_iteration_start_optimality_(optimality_cuts_.end()),last_iteration_start_feasibility_(feasibility_cuts_.end()),level_opt_iterator_(optimality_cuts_.end()),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),alternating_intra_cut_adaptive_(true),
matrix_(nullptr),column_lower_(nullptr),column_upper_(nullptr),objective_(nullptr),row_lower_(nullptr),row_upper_(nullptr),no_staircase_constraints_(),d_square_cut_t_matrix_(nullptr),d_square_cut_rhs_(),solver_rhs_(nullptr),can_safely_reuse_(true),oda_optimality_cuts_(),oda_cut_evaluation_(),oda_lambdas_(),build_time_(0.0),solution_time_(0.0)
{
    
}

BendersNodeData::~BendersNodeData(void)
{

    delete [] d_square_cut_t_matrix_;

    delete [] solver_rhs_;

    delete [] discrepancy_;


}

void BendersNodeData::CleanUnusedMemory( bool direction )
{
    unbounded_ = false;
    unbounded_problem_ = false;
    unbounded_extreme_ray_.reset(nullptr);
    unbounded_right_hand_side_.reset(nullptr);
    if ( direction ){ // We go forward, we do not need dual values. We recompute them, if needed
        dual_solution_.clear();
        reduced_cost_.clear();
    }
    else { // We go backward, we do not need the column solution;
        col_solution_.clear();
        next_stage_approximation_.clear();
    // Needed in aggregate_cut method, for already solved problems. This means, we can get rid of it, if we go backwards.

    }
}

BendersStageData::BendersStageData()
: num_columns_(0),num_rows_(0),stage_(0),num_thetas_(1),upper_bound_(1e20),lower_bound_(-1e20),
stochastic_matrix_w_(false),stochastic_matrix_t_(false),stochastic_rhs_(false),stochastic_objective_(false),stochastic_bounds_(false),non_zero_bounds_(false),
column_indices_(0),row_indices_(0),warm_start_(nullptr),problem_was_solved_in_this_iteration_(false),osi_solver_map_(),no_staircase_constraints_(),changed_aggregation_level_(false),num_d_square_cuts_(0),d_square_matrix_(nullptr),non_integer_scenarios(),violated_index_(-1),d_square_lambda_0_1_(nullptr), d_square_lambda_1_1_(nullptr),d_square_lambda_0_2_(0),d_square_lambda_1_2_(0),d_square_pi_(nullptr),d_square_disjunctive_indices_(),solve_binary_subproblems_(false),use_level_(false)
{
	added_optimality_cuts_ = 0;
	added_feasibility_cuts_ = 0;
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
    assert( start_time_[stg] == 0.0);
    start_time_[stg] = CoinCpuTime();

}

void Timer::Stop( int stg )
{
    if ( start_time_[stg] != 0.0)
        overall_time_[stg] += CoinCpuTime() - start_time_[stg];
    start_time_[stg] = 0.0;
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

void Timer::AddTime( int stg, double time )
{
    if ( start_time_[stg] == 0.0 )
        overall_time_[stg] += time;
    else
        assert(false);
}
void Parameter::init()
{
    // DetEq or Benders
    parameter_.put(PNB_BENDERS,PNB_INT_ALGORITHM_BENDERS); // 0: No Benders (detEq), 1 Benders
    
	// WarmStart
    parameter_.put(PNB_WARM_START, 1); // 0 = no WarmStart, 1 = WarmStart with solution from previous iteration, 2 = WarmStart with solution from previous scenario (in the same iteration)

    parameter_.put(PNB_LP_METHOD,PNB_INT_DUAL_SIMPLEX);
    parameter_.put(PNB_FEAS_CUT_LP_METHOD,PNB_INT_DUAL_SIMPLEX);
    //// Aggregation of Cuts ( or single-multi cut )
    parameter_.put(PNB_AGGREGATED_OPTIMALITY_CUTS, 1);// Integer value: How many cuts do we want to have per stage. Static! If 0, use ScenariosProCut to determine number of cuts.
	parameter_.put(PNB_DYNAMIC_AGGREGATION, false); // Do we use Dynamic aggregation? And which strategy?
	// MIP Stuff
    parameter_.put(PNB_RELAXATION, 1); // Give stage from which we should solve the relaxed problem! (Standard is: Allow integers in first-stage, but relax later stages!
	// Multistage strategies
    parameter_.put(PNB_TREE_WALK_STRATEGY,0);
	parameter_.put(PNB_TREE_WALK_STRATEGY_STAGE,0); // Applicable, if we use TREE_WALK_STRATEGY=5. FirstStage = 0. Also Applicable for TREE_WALK_STRATEGY=6 (if set to 0, we autoset dynamic protocol)
    parameter_.put(PNB_TREE_WALK_STRATEGY_EPSILON, 1e-06); 
    // Feas cut generation
    parameter_.put(PNB_HANDLE_FEAS_CUTS,PNB_INT_FEAS_CUT_ONLY_ONE);
    // Stage Aggregation
    parameter_.put(PNB_STAGE_AGGREGATION,false);
    parameter_.put(PNB_STAGE_AGGREGATION_START,0);
    parameter_.put(PNB_STAGE_AGGREGATION_END,0);
    parameter_.put(PNB_STAGE_AGGREGATION_DIRECTION,true); // Direction == true : Aggregate from start
    //Bounds in matrix
    parameter_.put(PNB_BOUNDS_IN_MATRIX,false);
    // Intra Cut Aggregation
    parameter_.put(PNB_CUT_CONSOLIDATION,false);
    // Intra cut aggregation strategy
    parameter_.put(PNB_CUT_CONSOLIDATION_CUT_COMPUTATION,true);
    parameter_.put(PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD,5);
    parameter_.put(PNB_CUT_CONSOLIDATION_PARTIAL_THRESHOLD,0.1);
    // Start solution
    parameter_.put(PNB_USE_START_SOLUTION,0); // 0: no start solution
	// Parallel Parameters
	parameter_.put(PNB_NUM_CPU,0);
	parameter_.put(PNB_CPU_USED,0);
    // Maximal solution time
    parameter_.put(PNB_MAXIMAL_SOLUTION_TIME,3600); // In Minutes
    // Parameter for Level and ODA
    parameter_.put(PNB_LEVEL_LAMBDA,0.25);
    parameter_.put(PNB_ODA_KAPPA,0.5);
    parameter_.put(PNB_STAGE_LEVEL,0); // Up to which stage should we solve level problems? default is only the first stage (parameter holds also for regularization)
    //// Tolerances: We should use the same interger and zero tolerances as the underyling LP-solver
    parameter_.put(PNB_TOLERANCE_ZERO, 1e-06);
	parameter_.put(PNB_TOLERANCE_INTEGER,  1e-06); 
    parameter_.put(PNB_EPSILON_ABSOLUTE,  1e-06);
    parameter_.put(PNB_EPSILON_RELATIVE,  1e-06);
    // Solution management
    parameter_.put(PNB_COMPUTE_PRIMAL_SOLUTIONS,PNB_INT_SOLUTION_PRIMAL_NO);
    parameter_.put(PNB_COMPUTE_DUAL_SOLUTIONS,PNB_INT_SOLUTION_DUAL_NO);
    parameter_.put(PNB_COMPUTE_OBJECTIVE_VALUES,false);
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
        // Create oda_nodes_in_cut Aggregate Vector!
        benders_node_data->oda_aggregates_.resize(node_in_cut_vector.size());
        int count = 0;
        for ( std::vector<int>& vec : benders_node_data->oda_aggregates_ )
            vec.push_back(count++);
        assert( count == node_in_cut_vector.size());

        // For now we just add all aggregates into it!
        // 
        //benders_node_data->oda_aggregates_.push_back(std::vector<int>(node_in_cut_vector.size()));
        //for (int i = 0; i < node_in_cut_vector.size();i++)
        //    benders_node_data->oda_aggregates_[0][i] = i;
	}
	else{
        // Do not do it!
        assert(false);
	}
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

double CutStuff::active_threshold_ = 1.0;
int CutStuff::strike_threshold_ = -1;
CutStuff::CutStuff( int iteration, std::shared_ptr<OsiRowCut>& cut_pointer): cut_creation_iteration_(iteration),not_active_iterations_(0),aggregate_number_(-1),cut_(cut_pointer),active_value(0.0),current_index_in_solver_(-1),to_use_(true)
{
}

bool is_active( const std::shared_ptr<CutStuff>& value )
{
    return value->active_value < 0;
}

bool comparator_less_int_double_pair( const std::pair<int,double>& l, const std::pair<int,double>& r )
{
    return l.second < r.second; 
}
} // end namespace parallel benders
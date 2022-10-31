#include "BendersNodeData.hpp"

#include <math.h>

#include <sstream>
#include <boost/property_tree/json_parser.hpp>

#include "CoinTime.hpp"
#include "CoinHelperFunctions.hpp"
#include "CoinWarmStart.hpp"
#include "OsiRowCut.hpp"
#include "SmiLinearData.hpp"
#include "SmiScnData.hpp"

using namespace std;

namespace parallel_benders{

BendersNodeData::BendersNodeData():col_solution_(),dual_solution_(),incumbent_col_solution_(),reduced_cost_(),next_stage_approximation_(),discrepancy_(nullptr),was_under_threshold_(),dual_value_variable_bounds_(0.0),feasible_(true),unbounded_(false),unbounded_row_(-1),bounded_theta_(false),lower_bound_(-1e20),upper_bound_(1e20),
mip_solution_(nullptr),unbounded_problem_(false),unbounded_extreme_ray_(nullptr),unbounded_right_hand_side_(nullptr),solve_ray_right_hand_side_(false),cols_(0),rows_(0),num_thetas_(0),active_aggregates_(0),warm_start_(nullptr),warm_start_added_(false),level_warm_start_(nullptr),level_warm_start_cstat_(nullptr),level_warm_start_rstat_(nullptr),level_warm_start_dnorm_(nullptr),level_warm_start_length_(0),level_regularized_solution_(),level_regularized_start_row_(-1),
smi_node_data_(nullptr),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(),feasibility_cuts_(),epigraph_cuts_(),domain_cuts_(),lp_round_optimality_cuts_(),lp_round_feasibility_cuts_(),lp_round_epigraph_cuts_(),lp_round_str_epigraph_cuts_counter_(0), lp_round_plain_epigraph_cuts_counter_(0),lp_round_domain_cuts_(),last_iteration_start_optimality_(optimality_cuts_.end()),last_in_lp_cgp_start_optimality_(optimality_cuts_.end()),last_round_start_epigraph_(epigraph_cuts_.end()),last_in_lp_cgp_start_epigraph_(epigraph_cuts_.end()),last_round_start_domain_(domain_cuts_.end()),last_in_lp_cgp_problem_start_domain_(domain_cuts_.end()),last_iteration_start_feasibility_(feasibility_cuts_.end()),last_in_lp_cgp_problem_start_feasibility_(feasibility_cuts_.end()),alpha_normalized_(false),level_opt_iterator_(optimality_cuts_.end()),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),alternating_intra_cut_adaptive_(true),
matrix_(nullptr),column_lower_(nullptr),column_upper_(nullptr),objective_(nullptr),row_lower_(nullptr),row_upper_(nullptr),no_staircase_constraints_(),d_square_cut_t_matrix_(nullptr),d_square_cut_rhs_(),solver_rhs_(nullptr),can_safely_reuse_(true),oda_optimality_cuts_(),oda_cut_evaluation_(),oda_lambdas_(),oda_iterations_(),
lp_dual_cgp_matrix_(nullptr), lp_dual_cgp_column_lower_(), lp_dual_cgp_column_upper_(), lp_dual_cgp_objective_(), lp_dual_cgp_row_lower_(), lp_dual_cgp_row_upper_(), lp_dual_cgp_elements_defined_(false),
cvar_cuts_(),cvar_current_cut_coefficients_(),cvar_current_cut_rhs_(0.0),cvar_cut_index_(0),build_time_(0.0),solution_time_(0.0),cut_generation_time_(0.0)

{

}
/*

smi_noded_data_ pointer is copied using the shallow copy
OsiRowCut is present deep down in all cut containers and  wrapped in shared_ptr, not cloned
*/
BendersNodeData::BendersNodeData(const BendersNodeData& nodeData) :col_solution_(nodeData.col_solution_), dual_solution_(nodeData.dual_solution_), incumbent_col_solution_(nodeData.incumbent_col_solution_), reduced_cost_(nodeData.reduced_cost_), next_stage_approximation_(nodeData.next_stage_approximation_), discrepancy_(nullptr), was_under_threshold_(nodeData.was_under_threshold_), dual_value_variable_bounds_(nodeData.dual_value_variable_bounds_), feasible_(nodeData.feasible_), unbounded_(nodeData.unbounded_), unbounded_row_(nodeData.unbounded_row_), bounded_theta_(nodeData.bounded_theta_), lower_bound_(nodeData.lower_bound_), upper_bound_(nodeData.upper_bound_),
mip_solution_(nodeData.mip_solution_ ? std::make_unique<double[]>(nodeData.cols_) : nullptr), unbounded_problem_(nodeData.unbounded_problem_), unbounded_extreme_ray_(nullptr), unbounded_right_hand_side_(nullptr), solve_ray_right_hand_side_(nodeData.solve_ray_right_hand_side_), cols_(nodeData.cols_), rows_(nodeData.rows_), num_thetas_(nodeData.num_thetas_), active_aggregates_(nodeData.active_aggregates_), warm_start_(nodeData.warm_start_ == nullptr ? nullptr : nodeData.warm_start_->clone()), warm_start_added_(nodeData.warm_start_added_), level_warm_start_(nodeData.level_warm_start_ == nullptr ? nullptr : nodeData.level_warm_start_->clone()), level_warm_start_cstat_(nullptr), level_warm_start_rstat_(nullptr), level_warm_start_dnorm_(nullptr), level_warm_start_length_(nodeData.level_warm_start_length_), level_regularized_solution_(nodeData.level_regularized_solution_), level_regularized_start_row_(nodeData.level_regularized_start_row_),
smi_node_data_(nodeData.smi_node_data_), scenario_index_(nodeData.scenario_index_), branch_scenario_(nodeData.branch_scenario_), num_node_in_stage_(nodeData.num_node_in_stage_), child_node_start_(nodeData.child_node_start_),
probability_(nodeData.probability_), conditional_probability_(nodeData.conditional_probability_), optimality_cuts_(), feasibility_cuts_(), epigraph_cuts_(), domain_cuts_(),lp_round_optimality_cuts_(),lp_round_feasibility_cuts_(),lp_round_epigraph_cuts_(),lp_round_str_epigraph_cuts_counter_(0),lp_round_plain_epigraph_cuts_counter_(0),lp_round_domain_cuts_(),last_iteration_start_optimality_(optimality_cuts_.end()), last_in_lp_cgp_start_optimality_(optimality_cuts_.end()), last_round_start_epigraph_(epigraph_cuts_.end()), last_in_lp_cgp_start_epigraph_(epigraph_cuts_.end()), last_round_start_domain_(domain_cuts_.end()), last_in_lp_cgp_problem_start_domain_(domain_cuts_.end()), last_iteration_start_feasibility_(feasibility_cuts_.end()), last_in_lp_cgp_problem_start_feasibility_(feasibility_cuts_.end()), alpha_normalized_(nodeData.alpha_normalized_), level_opt_iterator_(optimality_cuts_.end()), num_scen_pro_cut_(nodeData.num_scen_pro_cut_), nodes_in_cut_(nodeData.nodes_in_cut_), changed_aggregation_level_(nodeData.changed_aggregation_level_), alternating_intra_cut_adaptive_(nodeData.alternating_intra_cut_adaptive_),
matrix_(nodeData.matrix_ ? new CoinPackedMatrix(*nodeData.matrix_) : nullptr), column_lower_(nullptr), column_upper_(nullptr), objective_(nullptr), row_lower_(nullptr), row_upper_(nullptr), no_staircase_constraints_(nodeData.no_staircase_constraints_), d_square_cut_t_matrix_(nodeData.d_square_cut_t_matrix_ ? new CoinPackedMatrix(*nodeData.d_square_cut_t_matrix_) : nullptr), d_square_cut_rhs_(nodeData.d_square_cut_rhs_), solver_rhs_(nullptr), can_safely_reuse_(nodeData.can_safely_reuse_), oda_optimality_cuts_(nodeData.oda_optimality_cuts_), oda_cut_evaluation_(nodeData.oda_cut_evaluation_), oda_lambdas_(nodeData.oda_lambdas_), oda_iterations_(nodeData.oda_iterations_),
lp_dual_cgp_matrix_(nullptr), lp_dual_cgp_column_lower_(nodeData.lp_dual_cgp_column_lower_), lp_dual_cgp_column_upper_(nodeData.lp_dual_cgp_column_upper_), lp_dual_cgp_objective_(nodeData.lp_dual_cgp_objective_), lp_dual_cgp_row_lower_(nodeData.lp_dual_cgp_row_lower_), lp_dual_cgp_row_upper_(nodeData.lp_dual_cgp_row_upper_), lp_dual_cgp_elements_defined_(nodeData.lp_dual_cgp_elements_defined_),
cvar_cuts_(), cvar_current_cut_coefficients_(nodeData.cvar_current_cut_coefficients_), cvar_current_cut_rhs_(nodeData.cvar_current_cut_rhs_), cvar_cut_index_(nodeData.cvar_cut_index_), build_time_(nodeData.build_time_), solution_time_(nodeData.solution_time_), cut_generation_time_(nodeData.cut_generation_time_)

{
    // we assume that stage_data_[current_stage_] - needed length of mip_solution is equal to cols_ and the same for rows_
    if (nodeData.mip_solution_) {
        assert(false && "Never tested");
        std::copy_n(nodeData.mip_solution_.get(), nodeData.cols_, mip_solution_.get());
    }
    if (nodeData.column_lower_) {
        assert(false && "Never tested");
        column_lower_ = new double[nodeData.cols_];
        std::copy_n(nodeData.column_lower_, nodeData.cols_, column_lower_);
    }
    if (nodeData.column_upper_) {
        assert(false && "Never tested");
        column_upper_ = new double[nodeData.cols_];
        std::copy_n(nodeData.column_upper_, nodeData.cols_, column_upper_);
    }
    if (nodeData.objective_) {
        assert(false && "Never tested");
        objective_ = new double[nodeData.cols_];
        std::copy_n(nodeData.objective_, nodeData.cols_, objective_);
    }
    if (nodeData.row_lower_) {
        assert(false && "Never tested");
        row_lower_ = new double[nodeData.rows_];
        std::copy_n(nodeData.row_lower_, nodeData.rows_, row_lower_);
    }
    if (nodeData.row_upper_) {
        assert(false && "Never tested");
        row_upper_ = new double[nodeData.rows_];
        std::copy_n(nodeData.row_upper_, nodeData.rows_, row_upper_);
    }
    if (nodeData.lp_dual_cgp_elements_defined_) {
        assert(false && "Never tested. Do we actually copy the matrix");
        lp_dual_cgp_matrix_ = make_shared<CoinPackedMatrix>(*nodeData.lp_dual_cgp_matrix_);
        lp_dual_cgp_elements_defined_ = true;
    }

    BendersNodeData::cut_container::const_iterator it = nodeData.optimality_cuts_.begin();
    for (; it != nodeData.optimality_cuts_.end(); ++it) {
        optimality_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
        if (nodeData.last_iteration_start_optimality_ == it) {
            last_iteration_start_optimality_ = --optimality_cuts_.end();
        }
        if (nodeData.last_in_lp_cgp_start_optimality_ == it) {
            last_in_lp_cgp_start_optimality_ = --optimality_cuts_.end();
        }
        if (nodeData.level_opt_iterator_ == it) {
            level_opt_iterator_ = --optimality_cuts_.end();
        }
    }

    it = nodeData.feasibility_cuts_.begin();
    for (; it != nodeData.feasibility_cuts_.end(); ++it) {
        feasibility_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
        if (nodeData.last_iteration_start_feasibility_ == it) {
            last_iteration_start_feasibility_ = --feasibility_cuts_.end();
        }
        if (nodeData.last_in_lp_cgp_problem_start_feasibility_ == it) {
            last_in_lp_cgp_problem_start_feasibility_ = --feasibility_cuts_.end();
        }
    }

    it = nodeData.epigraph_cuts_.begin();
    for (; it != nodeData.epigraph_cuts_.end(); ++it) {
        epigraph_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
        if (nodeData.last_round_start_epigraph_ == it) {
            last_round_start_epigraph_ = --epigraph_cuts_.end();
        }
        if (nodeData.last_in_lp_cgp_start_epigraph_ == it) {
            last_in_lp_cgp_start_epigraph_ = --epigraph_cuts_.end();
        }
    }

    it = nodeData.domain_cuts_.begin();
    for (; it != nodeData.domain_cuts_.end(); ++it) {
        domain_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
        if (nodeData.last_round_start_domain_ == it) {
            last_round_start_domain_ = --domain_cuts_.end();
        }
        if (nodeData.last_in_lp_cgp_start_epigraph_ == it) {
            last_in_lp_cgp_problem_start_domain_ = --domain_cuts_.end();
        }
    }
    
    it = nodeData.lp_round_optimality_cuts_.begin();
    for (; it != nodeData.lp_round_optimality_cuts_.end(); ++it) {
        lp_round_optimality_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
    }
    it = nodeData.lp_round_feasibility_cuts_.begin();
    for (; it != nodeData.lp_round_feasibility_cuts_.end(); ++it) {
        lp_round_feasibility_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
    }
    it = nodeData.lp_round_epigraph_cuts_.begin();
    for (; it != nodeData.lp_round_epigraph_cuts_.end(); ++it) {
        lp_round_epigraph_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
    }
    it = nodeData.lp_round_domain_cuts_.begin();
    for (; it != nodeData.lp_round_domain_cuts_.end(); ++it) {
        lp_round_domain_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
    }
    lp_round_str_epigraph_cuts_counter_ = nodeData.lp_round_str_epigraph_cuts_counter_.load();
    lp_round_plain_epigraph_cuts_counter_ = nodeData.lp_round_str_epigraph_cuts_counter_.load();

    it = nodeData.cvar_cuts_.begin();
    for (; it != nodeData.cvar_cuts_.end(); ++it) {
        cvar_cuts_.push_back(std::make_shared<CutStuff>(*it->get()));
    }

    assert(nodeData.discrepancy_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(nodeData.unbounded_extreme_ray_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(nodeData.unbounded_right_hand_side_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(nodeData.level_warm_start_cstat_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(nodeData.level_warm_start_rstat_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(nodeData.level_warm_start_dnorm_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(nodeData.solver_rhs_ == nullptr && "Implement copy of this member and its initialization in the member list.");
}

BendersNodeData::BendersNodeData(SmiNodeData * smiNodeData):col_solution_(),dual_solution_(),incumbent_col_solution_(),reduced_cost_(),next_stage_approximation_(),discrepancy_(nullptr),was_under_threshold_(),dual_value_variable_bounds_(0.0),feasible_(true),unbounded_(false),unbounded_row_(-1),bounded_theta_(false),lower_bound_(-1e20),upper_bound_(1e20),
mip_solution_(nullptr),unbounded_problem_(false),unbounded_extreme_ray_(nullptr),unbounded_right_hand_side_(nullptr),solve_ray_right_hand_side_(false),cols_(0),rows_(0),num_thetas_(0),active_aggregates_(0),warm_start_(nullptr),warm_start_added_(false),level_warm_start_(nullptr),level_warm_start_cstat_(nullptr),level_warm_start_rstat_(nullptr),level_warm_start_dnorm_(nullptr),level_warm_start_length_(0),level_regularized_solution_(),level_regularized_start_row_(-1),
smi_node_data_(smiNodeData),scenario_index_(-1),branch_scenario_(-1),num_node_in_stage_(0),child_node_start_(0),
probability_(0.0),conditional_probability_(0.0),optimality_cuts_(),feasibility_cuts_(),epigraph_cuts_(),domain_cuts_(),lp_round_optimality_cuts_(),lp_round_feasibility_cuts_(),lp_round_epigraph_cuts_(),lp_round_str_epigraph_cuts_counter_(0),lp_round_plain_epigraph_cuts_counter_(0),lp_round_domain_cuts_(),last_iteration_start_optimality_(optimality_cuts_.end()),last_in_lp_cgp_start_optimality_(optimality_cuts_.end()),last_round_start_epigraph_(epigraph_cuts_.end()),last_in_lp_cgp_start_epigraph_(epigraph_cuts_.end()),last_round_start_domain_(domain_cuts_.end()),last_in_lp_cgp_problem_start_domain_(domain_cuts_.end()),last_iteration_start_feasibility_(feasibility_cuts_.end()),last_in_lp_cgp_problem_start_feasibility_(feasibility_cuts_.end()),alpha_normalized_(false),level_opt_iterator_(optimality_cuts_.end()),num_scen_pro_cut_(0),nodes_in_cut_(),changed_aggregation_level_(false),alternating_intra_cut_adaptive_(true),
matrix_(nullptr),column_lower_(nullptr),column_upper_(nullptr),objective_(nullptr),row_lower_(nullptr),row_upper_(nullptr),no_staircase_constraints_(),d_square_cut_t_matrix_(nullptr),d_square_cut_rhs_(),solver_rhs_(nullptr),can_safely_reuse_(true),oda_optimality_cuts_(),oda_cut_evaluation_(),oda_lambdas_(),oda_iterations_(),
lp_dual_cgp_matrix_(nullptr), lp_dual_cgp_column_lower_(), lp_dual_cgp_column_upper_(), lp_dual_cgp_objective_(), lp_dual_cgp_row_lower_(), lp_dual_cgp_row_upper_(), lp_dual_cgp_elements_defined_(false),
cvar_cuts_(),cvar_current_cut_coefficients_(),cvar_current_cut_rhs_(0.0),cvar_cut_index_(0),build_time_(0.0),solution_time_(0.0),cut_generation_time_(0.0)
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

void BendersNodeData::Clear(int delete_cuts_start_it, bool keep_warm_start)
{
    col_solution_.clear();
    dual_solution_.clear();
    incumbent_col_solution_.clear();
    reduced_cost_.clear();
    next_stage_approximation_.clear();

    if (bounded_theta_) {
        assert(false && "What should we do here?");
    }
    if (unbounded_ && unbounded_row_ > 0) {
        assert(false && "What to do with solver?");
        //osi_solver->deleteRows(1, &benders_node_pointer->unbounded_row_);
        //SolveSolverInterface(osi_solver);
        unbounded_row_ = -1;
        // Add this to all methods that resolve the problem in one way or another!
    }
    unbounded_ = false;
    unbounded_problem_ = false;
    feasible_ = true;

    delete matrix_; matrix_ = nullptr;
    if (smi_node_data_->getColLowerLength())
        delete[] column_lower_;
    column_lower_ = nullptr;

    if (smi_node_data_->getColUpperLength())
        delete[] column_upper_;
    column_upper_ = nullptr;

    if (smi_node_data_->getObjectiveLength())
        delete[] objective_;
    objective_ = nullptr;

    delete[] row_upper_; row_upper_ = nullptr;
    delete[] row_lower_; row_lower_ = nullptr;

    // Delete cuts
    if (delete_cuts_start_it > -1) {
        for (auto container : { &optimality_cuts_, &epigraph_cuts_, &domain_cuts_, &feasibility_cuts_ }) {
            // we assume here that cuts are ordered according to the iteration
#ifndef NDEBUG
            int max_creation_iteration = -1;
            for (cut_container::iterator it = container->begin(); it != container->end(); it++) {
                if ((*it)->cut_creation_iteration_ < max_creation_iteration) {
                    assert(false && "Cuts in the container are not ordered according to the creation iteration.");
                }
                else {
                    max_creation_iteration = (*it)->cut_creation_iteration_;
                }
            }
#endif // !NDEBUG
            for (cut_container::iterator it = container->begin(); it != container->end(); it++) {
                if ((*it)->cut_creation_iteration_ >= delete_cuts_start_it) {
                    container->erase(it, container->end());
                    break;
                }
            }
        }
    }

    last_iteration_start_optimality_ = optimality_cuts_.end();
    last_in_lp_cgp_start_optimality_ = optimality_cuts_.end();
    level_opt_iterator_ = optimality_cuts_.end();
    last_round_start_epigraph_ = epigraph_cuts_.end();
    last_in_lp_cgp_start_epigraph_ = epigraph_cuts_.end();
    last_round_start_domain_ = domain_cuts_.end();
    last_in_lp_cgp_problem_start_domain_ = domain_cuts_.end();
    last_iteration_start_feasibility_ = feasibility_cuts_.end();
    last_in_lp_cgp_problem_start_feasibility_ = feasibility_cuts_.end();

    delete[] discrepancy_; discrepancy_ = nullptr;
    was_under_threshold_.reset();
    dual_value_variable_bounds_ = 0.0;
    lower_bound_ = -1e20;
    upper_bound_ = 1e20;
    assert(mip_solution_ == nullptr && "Smth to do with warm start");
    //mip_solution_(nullptr);
    unbounded_extreme_ray_ = nullptr;
    unbounded_right_hand_side_ = nullptr;
    solve_ray_right_hand_side_ = false;
    
    assert(active_aggregates_.all() && "What to do with non active aggregates. E.g., if we added cuts??? and want to resolve the problem");
    //active_aggregates_(0);
    
    // Warm start
    if (!keep_warm_start) {
        warm_start_.reset();
    }
    warm_start_added_ = false;

    assert(level_warm_start_ == nullptr && level_warm_start_cstat_ == nullptr && level_warm_start_rstat_ == nullptr 
        && level_warm_start_dnorm_ == nullptr && level_warm_start_length_ == 0);
    //level_warm_start_(nullptr);
    //level_warm_start_cstat_(nullptr);
    //level_warm_start_rstat_(nullptr);
    //level_warm_start_dnorm_(nullptr);
    //level_warm_start_length_(0);
    //level_regularized_solution_();
    //level_regularized_start_row_(-1);
    
#ifdef SPEC_CASES
    LOG_IF(WARNING, changed_aggregation_level_) << "Just to note : changed_aggregation_level_ == true after we Solved() the PNB CGP and want to solve it again." << std::endl;
#endif
    delete d_square_cut_t_matrix_; d_square_cut_t_matrix_ = nullptr;
    d_square_cut_rhs_.clear();
    delete solver_rhs_; solver_rhs_ = nullptr;
    can_safely_reuse_ = true;
    assert(oda_optimality_cuts_.empty() && "What to do with ODA?");
    //oda_optimality_cuts_() //??? should we clear() it
    //oda_cut_evaluation_() //??? should we clear() it
    assert(oda_lambdas_.empty() && "What to do with ODA?");
    // oda_lambdas_()
    assert(oda_iterations_.empty() && "What to do with ODA?");
    //oda_iterations_()
    assert(cvar_cuts_.empty() && "What to do with CVaR?");
    //cvar_cuts_()
    cvar_current_cut_coefficients_.clear();
    cvar_current_cut_rhs_ = 0.0;
    cvar_cut_index_ = 0;

    lp_dual_cgp_matrix_.reset();
    lp_dual_cgp_column_lower_.clear();
    lp_dual_cgp_column_upper_.clear();
    lp_dual_cgp_objective_.clear();
    lp_dual_cgp_row_lower_.clear();
    lp_dual_cgp_row_upper_.clear();

    // Timers
    build_time_ = 0.0;
    solution_time_ = 0.0;
    cut_generation_time_ = 0.0;
}

void BendersNodeData::getDualCGP_Data(std::shared_ptr<const CoinPackedMatrix>& matrix, const std::vector<double>*& column_lower, const std::vector<double>*& column_upper, const std::vector<double>*& objective, const std::vector<double>*& row_lower, const std::vector<double>*& row_upper) const
{
    matrix = lp_dual_cgp_matrix_;
    column_lower = &lp_dual_cgp_column_lower_;
    column_upper = &lp_dual_cgp_column_upper_;
    objective = &lp_dual_cgp_objective_;
    row_lower = &lp_dual_cgp_row_lower_;
    row_upper = &lp_dual_cgp_row_upper_;
}

void BendersNodeData::getMutableDualCGP_Data(std::shared_ptr<CoinPackedMatrix>& matrix, std::vector<double>*& column_lower, std::vector<double>*& column_upper, std::vector<double>*& objective, std::vector<double>*& row_lower, std::vector<double>*& row_upper)
{
    if (!lp_dual_cgp_matrix_) {
        lp_dual_cgp_matrix_ = make_shared<CoinPackedMatrix>();
    }
    matrix = lp_dual_cgp_matrix_;
    column_lower = &lp_dual_cgp_column_lower_;
    column_upper = &lp_dual_cgp_column_upper_;
    objective = &lp_dual_cgp_objective_;
    row_lower = &lp_dual_cgp_row_lower_;
    row_upper = &lp_dual_cgp_row_upper_;
}

BendersStageData::BendersStageData()
: num_columns_(0),num_rows_(0),stage_(0),num_thetas_(1),
stochastic_matrix_w_(false),stochastic_matrix_t_(false),stochastic_rhs_(false),stochastic_objective_(false),stochastic_bounds_(false),non_zero_bounds_(false),
column_indices_(0),row_indices_(0),warm_start_(nullptr),problem_was_solved_in_this_iteration_(false),no_staircase_constraints_(),changed_aggregation_level_(false),num_d_square_cuts_(0),d_square_matrix_(nullptr),non_integer_scenarios(),violated_index_(-1),d_square_lambda_0_1_(nullptr), d_square_lambda_1_1_(nullptr),d_square_lambda_0_2_(0),d_square_lambda_1_2_(0),d_square_pi_(nullptr),d_square_disjunctive_indices_(),solve_binary_subproblems_(false),use_level_(false),
added_optimality_cuts_(0), added_cgp_optimality_cuts_(0), added_epigraph_cuts_(0), added_str_epigraph_cuts_(0), added_plain_epigraph_cuts_(0), lp_cgp_counter_(0), added_domain_cuts_(0), added_feasibility_cuts_(0), added_cgp_feasibility_cuts_(0)
{
	
}
BendersStageData::BendersStageData(const BendersStageData& stageData)
    : num_columns_(stageData.num_columns_), num_rows_(stageData.num_rows_), stage_(stageData.stage_), num_thetas_(stageData.num_thetas_),
    stochastic_matrix_w_(stageData.stochastic_matrix_w_), stochastic_matrix_t_(stageData.stochastic_matrix_t_), stochastic_rhs_(stageData.stochastic_rhs_), stochastic_objective_(stageData.stochastic_objective_), stochastic_bounds_(stageData.stochastic_bounds_), non_zero_bounds_(stageData.non_zero_bounds_),
    column_indices_(stageData.column_indices_), row_indices_(stageData.row_indices_), warm_start_(stageData.warm_start_ == nullptr ? nullptr : stageData.warm_start_->clone()), problem_was_solved_in_this_iteration_(stageData.problem_was_solved_in_this_iteration_), no_staircase_constraints_(stageData.no_staircase_constraints_), changed_aggregation_level_(stageData.changed_aggregation_level_), num_d_square_cuts_(stageData.num_d_square_cuts_), d_square_matrix_(stageData.d_square_matrix_ ? new CoinPackedMatrix(*stageData.d_square_matrix_) : nullptr), non_integer_scenarios(stageData.non_integer_scenarios), violated_index_(stageData.violated_index_), d_square_lambda_0_1_(nullptr), d_square_lambda_1_1_(nullptr), d_square_lambda_0_2_(stageData.d_square_lambda_0_2_), d_square_lambda_1_2_(stageData.d_square_lambda_1_2_), d_square_pi_(nullptr), d_square_disjunctive_indices_(stageData.d_square_disjunctive_indices_), solve_binary_subproblems_(stageData.solve_binary_subproblems_), use_level_(stageData.use_level_),
    added_optimality_cuts_(stageData.added_optimality_cuts_.load()), added_cgp_optimality_cuts_(stageData.added_cgp_optimality_cuts_.load()), added_epigraph_cuts_(stageData.added_epigraph_cuts_.load()), added_str_epigraph_cuts_(stageData.added_str_epigraph_cuts_.load()), added_plain_epigraph_cuts_(stageData.added_plain_epigraph_cuts_.load()), lp_cgp_counter_(stageData.lp_cgp_counter_.load()), added_domain_cuts_(stageData.added_domain_cuts_.load()), added_feasibility_cuts_(stageData.added_feasibility_cuts_.load()), added_cgp_feasibility_cuts_(stageData.added_cgp_feasibility_cuts_.load())
{
    if (stageData.d_square_pi_) {
        assert(false && "Never tested");
        d_square_pi_ = new double[stageData.num_columns_];
        std::copy_n(stageData.d_square_pi_, stageData.num_columns_, d_square_pi_);
    }

    assert(stageData.d_square_lambda_1_1_ == nullptr && "Implement copy of this member and its initialization in the member list.");
    assert(stageData.d_square_lambda_0_1_ == nullptr && "Implement copy of this member and its initialization in the member list.");

}

BendersStageData::~BendersStageData()
{
}

void BendersStageData::Clear(bool keep_warm_start) {
    
    //??? what is with thetas
    //num_thetas_(1)
    
    //WarmStart
    if (!keep_warm_start) {
        assert(false && "Never tested: during the initialization we do this: warm_start_.reset(osi_solver_->getEmptyWarmStart()); Is it necessary?");
        warm_start_.reset(); 
    }
    problem_was_solved_in_this_iteration_ = false;
       
    changed_aggregation_level_ = false;
    
    // d_square stuff - not implemented
    assert(num_d_square_cuts_ == 0 && d_square_matrix_ == nullptr);
    //num_d_square_cuts_(0), d_square_matrix_(nullptr), non_integer_scenarios(), violated_index_(-1), d_square_lambda_0_1_(nullptr), d_square_lambda_1_1_(nullptr), d_square_lambda_0_2_(0), d_square_lambda_1_2_(0), d_square_pi_(nullptr), d_square_disjunctive_indices_(), solve_binary_subproblems_(false)
    
    // level is never used in pnb
    assert(!use_level_);
    //use_level_(false)

    added_optimality_cuts_ = 0;
	added_cgp_optimality_cuts_ = 0;
    added_epigraph_cuts_ = 0;
    added_str_epigraph_cuts_ = 0;
    added_plain_epigraph_cuts_ = 0;
    lp_cgp_counter_ = 0;
    added_domain_cuts_ = 0;
    added_feasibility_cuts_ = 0;
	added_cgp_feasibility_cuts_ = 0;
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
    start_time_[stg] = Now();

}

void Timer::Stop( int stg )
{
    if ( start_time_[stg] != 0.0)
        overall_time_[stg] = stg;
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
    s << GetAggregatedTime() << ";";
    for (int i = 0; i < stages_; i++) {
        s << GetTime(i) << ";";
    }
    return s.str();
}

int Timer::GetTimeStages() const
{
    return stages_;
}

void Timer::AddTime( int stg, double time )
{
    std::lock_guard<std::mutex>unique_lock(overall_time_mutex_);
    if ( start_time_[stg] == 0.0 )
        overall_time_[stg] += time;
    else
        assert(false);
}

void Timer::Reset() {
    start_time_.assign(start_time_.size(), 0.0);
    overall_time_.assign(overall_time_.size(), 0.0);
}

double Timer::Now()
{
    std::chrono::duration<double> now = wall_clock::now().time_since_epoch();
    return now.count();
}

void Parameter::init()
{
    // Save .lp files on disc (many files are saved on disk and might take a lot of space)
	parameter_.put(PNB_PRINT_SUBPROBLEMS,false);

	// Append instance info to .csv file
	parameter_.put(PNB_SAVE_INSTANCE_SPECS, false);
	
    // DetEq or Benders
    parameter_.put(PNB_BENDERS,PNB_INT_ALGORITHM_BENDERS); // 0: No Benders (detEq), 1 Benders
    
	// WarmStart
    parameter_.put(PNB_WARM_START, PNB_INT_NO_WARM_START);

    parameter_.put(PNB_LP_METHOD,PNB_INT_DUAL_SIMPLEX);
    parameter_.put(PNB_FEAS_CUT_LP_METHOD,PNB_INT_DUAL_SIMPLEX);
    //// Aggregation of Cuts ( or single-multi cut )
    parameter_.put(PNB_AGGREGATED_OPTIMALITY_CUTS, 1);// Integer value: How many cuts do we want to have per stage. Static! If 0, use ScenariosProCut to determine number of cuts.
	parameter_.put(PNB_DYNAMIC_AGGREGATION, false); // Do we use Dynamic aggregation? And which strategy?
	// MIP Stuff
    parameter_.put(PNB_RELAXATION, 0); // Give stage from which we should solve the relaxed problem! (default: 0 - no relaxation, standard: 2 - allow integers in first-stage, but relax later stages!)
    parameter_.put(PNB_USE_FIRST_PHASE, false);
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
    parameter_.put(PNB_USE_START_SOLUTION,PNB_INT_NO_START_SOLUTION);
	// Parallel Parameters
	parameter_.put(PNB_NUM_CPU,1);
	parameter_.put(PNB_CPU_USED,0);
    // Maximal solution time
    parameter_.put(PNB_MAXIMAL_SOLUTION_TIME,3600); // In Seconds
    // Parameter for Level and ODA
    parameter_.put(PNB_LEVEL_LAMBDA,0.5);
    parameter_.put(PNB_ODA_KAPPA,0.5);
    parameter_.put(PNB_ODA_AGGREGATES,1); // 1 Aggregates is single-cut case (default), 0 is same number of aggregates as number of aggregates, and some number inbetween is also OK.
    parameter_.put(PNB_STAGE_LEVEL,0); // Up to which stage should we solve level problems? default is only the first stage (parameter holds also for regularization)
    //// Tolerances: We should use the same integer and zero tolerances as the underlying LP-solver
    parameter_.put(PNB_TOLERANCE_ZERO, 1e-06);
	parameter_.put(PNB_TOLERANCE_INTEGER,  1e-06); 
    parameter_.put(PNB_EPSILON_ABSOLUTE,  1e-06);
    parameter_.put(PNB_EPSILON_RELATIVE,  1e-06);
    // CVaR params
    parameter_.put(PNB_CVAR_BETA,0.1); // Confidence level
    parameter_.put(PNB_CVAR_RHO,0.0); // CVaR value that we would like to reach
    parameter_.put(PNB_CVAR_WITH_FIRST_STAGE,false); // CVaR value that we would like to reach
    // Solution management
    parameter_.put(PNB_COMPUTE_PRIMAL_SOLUTIONS,PNB_INT_SOLUTION_PRIMAL_NO);
    parameter_.put(PNB_COMPUTE_DUAL_SOLUTIONS,PNB_INT_SOLUTION_DUAL_NO);
    parameter_.put(PNB_COMPUTE_OBJECTIVE_VALUES,false);
	//D2 algorithm
	SetParameterValue(PNB_AUTO_D2_ALGORITHM, false);
	// LiftAndProject Cuts
	parameter_.put(PNB_LIFT_AND_P_METHOD, PNB_LIFT_AND_P_METHOD_EPIGRAPH_STRENGTHENED);
    parameter_.put(PNB_CONT_PHASE_IT_LIMIT, 0);
	parameter_.put(PNB_LP_ROUNDS_LIMIT, 0);
	parameter_.put(PNB_LP_SWITCHES_LIMIT, 1);
	parameter_.put(PNB_LP_MAJOR_IT_LIMIT, 1);
	parameter_.put(PNB_LP_CUTS_FOR_FRACT, 100);
	parameter_.put(PNB_CGP_EPSILON_RELATIVE, 1e-06);
	parameter_.put(PNB_LP_ALLOW_TEST_CASES, 11111111111);
    parameter_.put(PNB_LP_PARALLEL_MODE, PNB_PARAL_MODE_ALL_SEQUENTIAL);
    parameter_.put(PNB_LP_USE_CGP_PNB_CUTS, PNB_CGP_CUTS_NONE);

	// Fill in constants
	// PNB_BENDERS
	consts_["PNB_INT_ALGORITHM_DETEQ"] = PNB_INT_ALGORITHM_DETEQ;
	consts_["PNB_INT_ALGORITHM_BENDERS"] = PNB_INT_ALGORITHM_BENDERS;
	consts_["PNB_INT_ALGORITHM_REGULARIZATION"] = PNB_INT_ALGORITHM_REGULARIZATION;
	consts_["PNB_INT_ALGORITHM_LEVEL"] = PNB_INT_ALGORITHM_LEVEL;
	consts_["PNB_INT_ALGORITHM_LEVEL_ODA"] = PNB_INT_ALGORITHM_LEVEL_ODA;
	consts_["PNB_INT_ALGORITHM_BENDERS_ODA"] = PNB_INT_ALGORITHM_BENDERS_ODA;
	consts_["PNB_INT_ALGORITHM_REGULARIZATION_ODA"] = PNB_INT_ALGORITHM_REGULARIZATION_ODA;
	consts_["PNB_INT_ALGORITHM_LEVEL_MANHATTAN"] = PNB_INT_ALGORITHM_LEVEL_MANHATTAN;
	consts_["PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA"] = PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA;
	consts_["PNB_INT_ALGORITHM_LEVEL_INFINITY"] = PNB_INT_ALGORITHM_LEVEL_INFINITY;
	consts_["PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA"] = PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA;
	consts_["PNB_INT_ALGORITHM_SOLVER_BENDERS"] = PNB_INT_ALGORITHM_SOLVER_BENDERS;
	consts_["PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT"] = PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT;

	// Variants with CVaR risk constraint
	consts_["PNB_INT_ALGORITHM_DETEQ_RISK"] = PNB_INT_ALGORITHM_DETEQ_RISK;
	consts_["PNB_INT_ALGORITHM_BENDERS_RISK"] = PNB_INT_ALGORITHM_BENDERS_RISK;
	consts_["PNB_INT_ALGORITHM_LEVEL_RISK"] = PNB_INT_ALGORITHM_LEVEL_RISK;
	consts_["PNB_INT_ALGORITHM_BENDERS_ODA_RISK"] = PNB_INT_ALGORITHM_BENDERS_ODA_RISK;
	consts_["PNB_INT_ALGORITHM_LEVEL_ODA_RISK"] = PNB_INT_ALGORITHM_LEVEL_ODA_RISK;
	consts_["PNB_INT_ALGORITHM_BENDERS_RISK_ALPHA"] = PNB_INT_ALGORITHM_BENDERS_RISK_ALPHA;
	consts_["PNB_INT_ALGORITHM_BENDERS_ODA_RISK_ALPHA"] = PNB_INT_ALGORITHM_BENDERS_ODA_RISK_ALPHA;
		
	// PNB_LP_METHOD
	consts_["PNB_INT_PRIMAL_SIMPLEX"] = PNB_INT_PRIMAL_SIMPLEX;
	consts_["PNB_INT_DUAL_SIMPLEX"] = PNB_INT_DUAL_SIMPLEX;
	consts_["PNB_INT_BARRIER"] = PNB_INT_BARRIER;
	consts_["PNB_INT_BARRIER_WITH_CROSSOVER"] = PNB_INT_BARRIER_WITH_CROSSOVER;
	consts_["PNB_INT_CONCURRENT_MASTER_STANDARD"] = PNB_INT_CONCURRENT_MASTER_STANDARD;

    //PNB_LP_PARALLEL_MODE
    consts_["PNB_PARAL_MODE_ALL_SEQUENTIAL"] = 0;
    consts_["PNB_PARAL_CGP_SUBPROBLEMS"] = 1;
    consts_["PNB_PARAL_FRACTS"] = 2;
    consts_["PNB_PARAL_FRACTS_AND_CGP_SUBPROBLEMS"] = 3;
    
	//PNB_USE_START_SOLUTION
	consts_["PNB_INT_NO_START_SOLUTION"] = PNB_INT_NO_START_SOLUTION;
	consts_["PNB_INT_EV_SOLUTION_FROM_PNB"] = PNB_INT_EV_SOLUTION_FROM_PNB;
	consts_["PNB_INT_USE_EV_SOLUTION"] = PNB_INT_USE_EV_SOLUTION;
	consts_["PNB_INT_USE_SUPPLIED_SOLUTION"] = PNB_INT_USE_SUPPLIED_SOLUTION;
	consts_["PNB_INT_EV_SOLUTION_FROM_PNB_WITH_EV"] = PNB_INT_EV_SOLUTION_FROM_PNB_WITH_EV;

    //PNB_WARM_START
    consts_["PNB_INT_NO_WARM_START"] = PNB_INT_NO_WARM_START;
    consts_["PNB_INT_WARM_START_SCENARIO_WISE"] = PNB_INT_WARM_START_SCENARIO_WISE; // WarmStart with solution from the previous iteration
    consts_["PNB_INT_WARM_START_STAGE_WISE"] = PNB_INT_WARM_START_STAGE_WISE; // WarmStart with solution from the first scenario (in the same iteration)

    //PNB_LP_USE_CGP_PNB_CUTS
    consts_["PNB_CGP_CUTS_NONE"] = PNB_CGP_CUTS_NONE;
    consts_["PNB_CGP_CUTS_EFFICIENT_AND_CONSOLIDATE"] = PNB_CGP_CUTS_EFFICIENT_AND_CONSOLIDATE;
    consts_["PNB_CGP_CUTS_EFFICIENT"] = PNB_CGP_CUTS_EFFICIENT;
    consts_["PNB_CGP_CUTS_CONSOLIDATE"] = PNB_CGP_CUTS_CONSOLIDATE;
    consts_["PNB_CGP_CUTS_ALL"] = PNB_CGP_CUTS_ALL;
}

void Parameter::update_from_json_string( std::string json_string ) {

	std::stringstream ss(json_string);
	boost::property_tree::ptree temp;
	boost::property_tree::read_json(ss, temp);
	
	for (auto leaf : temp) {
		
		std::string value = leaf.second.data();

		// Get (if any) int / bool value of the const string
		unordered_map<string, int>::iterator it = consts_.find(value);
		if (it != consts_.end())
			leaf.second.put_value(it->second);
		// convert booleans
		else if ( value == "true" )
			leaf.second.put_value(true);
		else if ( value == "false" )
			leaf.second.put_value(false);
		// else the value should be int
		else {
			if (value.find(".") != std::string::npos) { // float
				leaf.second.put_value(stof(value)); // convert to float
			}
			else {
				try
				{
					leaf.second.put_value(stoi(value)); // convert to int
				}
                catch (std::out_of_range& e)
                {
                    leaf.second.put_value(value); // save as a string (large integer);
                }
				catch (std::invalid_argument & e)
				{
					LOG(FATAL) << "Values in parameter_ should be int, float or bool. Check Parameter::consts_";
				}
			}
		}
		parameter_.put_child(leaf.first, leaf.second);
	}
}

void createRoundRobinAggregation( BendersNodeData* benders_node_data, int num_child, int num_cuts, int num_subproblems_per_cut, int oda_aggregates )
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
        // Set a parameter value here!
         benders_node_data->oda_aggregates_.resize(oda_aggregates);
        if ( oda_aggregates == 1 ){
            
            // Add all aggregates in there explicitly!
            for( int i = 0; i < node_in_cut_vector.size(); i++)
                benders_node_data->oda_aggregates_[0].push_back(i);
        }
        else if ( oda_aggregates == num_cuts) {
            int count = 0;
            for ( std::vector<int>& vec : benders_node_data->oda_aggregates_ )
                vec.push_back(count++);
            assert( count == node_in_cut_vector.size());
        }
        else if ( oda_aggregates > 0 && oda_aggregates < num_cuts ){
            // We have num_cuts aggregates and oda_aggregates many oda aggregates.
            for (int i = 0; i < num_cuts;i++)
                benders_node_data->oda_aggregates_[i % oda_aggregates].push_back(i);
        }
        else{
            assert(false);
        }


        // For now we just add all aggregates into it!
        // 
        //benders_node_data->oda_aggregates_.push_back(std::vector<int>(node_in_cut_vector.size()));
        //for (int i = 0; i < node_in_cut_vector.size();i++)
        //    benders_node_data->oda_aggregates_[0][i] = i;
	}
	else{
        // Do not do it!
        //assert(false);
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
CutStuff::CutStuff( int iteration, int thread_id, std::unique_ptr<OsiRowCut>& cut_pointer, int type, int num_cols, int num_theta): cut_creation_iteration_(iteration),cut_creation_thread_(thread_id),not_active_iterations_(0),aggregates_(),cut_(std::move(cut_pointer)),type_(type), num_cols_(num_cols),active_value(0.0),current_index_in_solver_(-1),to_use_(true),column_set_(parallel_benders::PNB_COL_SET_NONE)
{
    LOG_IF(FATAL, thread_id <= -2) << "We use -2 thread id for the non implemented code parts.";
    updateAggregData(num_theta);
	assert(is_consistent());
}

CutStuff::CutStuff(const CutStuff& another)
    : cut_creation_iteration_(another.cut_creation_iteration_),cut_creation_thread_(another.cut_creation_thread_),not_active_iterations_(another.not_active_iterations_), aggregates_(another.aggregates_), cut_(new OsiRowCut(*another.cut_)), type_(another.type_), num_cols_(another.num_cols_), active_value(another.active_value), current_index_in_solver_(another.current_index_in_solver_), to_use_(another.to_use_), column_set_(another.column_set_)
{
    assert(is_consistent());
}

const std::vector<int> CutStuff::getAggregates() const
{
    // it is rather expensive
    std::vector<int> aggregates_vector{};
    aggregates_vector.reserve(getNumberAggregates());
    
    int aggregate_index = aggregates_.find_first();
    while (aggregate_index != boost::dynamic_bitset<>::npos) {
        aggregates_vector.push_back(aggregate_index);
        aggregate_index = aggregates_.find_next(aggregate_index);
    }
    return aggregates_vector;
}

int CutStuff::getNumTheta() const
{
    return aggregates_.size();
}

int CutStuff::getNumberAggregates() const
{
    return aggregates_.count();
}

int CutStuff::getAggregateType() const
{   
    int num_aggregates = getNumberAggregates();
    int num_theta = getNumTheta();
    
    if (num_aggregates == 0) {
        return CutStuff::AGGREGATE_OVER_NONE;
    }
	if (num_aggregates == 1) {
        return CutStuff::AGGREGATE_OVER_ONE;
    }
    if (num_aggregates == num_theta) {
        return CutStuff::AGGREGATE_OVER_ALL;
    }
    if (num_aggregates > 1) {
        return CutStuff::AGGREGATE_OVER_PARTIAL;
    }
}

CutStuff& CutStuff::operator+=(const CutStuff& second_cut_stuff)
{
    
    LOG_IF(FATAL, num_cols_ != second_cut_stuff.num_cols_);
    LOG_IF(FATAL, getNumTheta() != second_cut_stuff.getNumTheta());
    LOG_IF(FATAL, cut_creation_iteration_ != second_cut_stuff.cut_creation_iteration_) << "Do you want to consolidate the cuts from different iterations?";
    LOG_IF(FATAL, not_active_iterations_ != 0 || second_cut_stuff.not_active_iterations_ != 0) << "How should we update it?";
    LOG_IF(FATAL, type_ != second_cut_stuff.type_);
    LOG_IF(FATAL, column_set_ != second_cut_stuff.column_set_);
    LOG_IF(FATAL, active_value != 0.0 || second_cut_stuff.active_value != 0.0) << "How should we update it?";
    LOG_IF(FATAL, current_index_in_solver_ != -1 || second_cut_stuff.current_index_in_solver_ != -1) << "How should we update it?";
    LOG_IF(FATAL, to_use_ != second_cut_stuff.to_use_) << "How should we update it?";

    const OsiRowCut& first = getCut();
    const OsiRowCut& second = second_cut_stuff.getCut();
    LOG_IF(FATAL, first == second);

    const CoinPackedVector& first_vector(first.row());
    const CoinPackedVector& second_vector(second.row());
    
    std::vector<double> new_elements(max(first_vector.getMaxIndex(), second_vector.getMaxIndex())+1, 0.0);
    for (int i = 0; i < first_vector.getNumElements(); i++) {
        int index = first_vector.getIndices()[i];
        double element = first_vector.getElements()[i];
        new_elements[index] = element;
    }

    for (int i = 0; i < second_vector.getNumElements(); i++) {
        int index = second_vector.getIndices()[i];
        double element = second_vector.getElements()[i];
        new_elements[index] += element;
        LOG_IF(FATAL, index >= num_cols_ && new_elements[index] != 1.0) << "Theta should always have value 1.0. We cannot have several cuts with the same aggregate(s).";
    }

    // Sparsify
    int max_num_non_zeros = min(first_vector.getNumElements()+second_vector.getNumElements(), int(new_elements.size()));
    std::vector<int>new_indices;
    new_indices.reserve(max_num_non_zeros);
    int index(0);
    for (int i = 0; i < new_elements.size(); i++) {
        double element = new_elements[i];
        if (element != 0.0) {
            new_indices.push_back(i);
            new_elements[index] = element;
            index++;
        }
    }
    
    // Create cut
    OsiRowCut* new_cut = new OsiRowCut();
    new_cut->setLb(first.lb() + second.lb());
    new_cut->setUb(first.ub());
    new_cut->setRow(index, new_indices.data(), new_elements.data()); // copy
    std::unique_ptr<OsiRowCut> cut_pointer(new_cut);
    setCut(cut_pointer);

    assert(is_consistent()); // might be not consistent if we get elements with 0.0 values
    return *this;
}

void CutStuff::updateAggregData(int num_theta)
{
    if ((type_ == PNB_CUT_TYPE_FEASIBILITY || type_ == PNB_CUT_TYPE_DOMAIN) && num_theta != 0) {
        throw runtime_error("Num theta should be 0 for the cuts w/o thetas: feasibility and domain cuts.");
    }
    const OsiRowCut& cut = getCut();
    int num_elems = cut.row().getNumElements();
    aggregates_.reset();
    aggregates_.resize(num_theta);
    for (int i = num_elems; i > 0; i--) {
        if (cut.row().getIndices()[i - 1] >= num_cols_) {
            int current_theta_index = cut.row().getIndices()[i - 1] - num_cols_;
            aggregates_.set(current_theta_index);
        }
        else {
            break;
        }
    }
}

const OsiRowCut& CutStuff::getCut() const
{
    return *cut_;
}

OsiRowCut& CutStuff::getMutableCut() const
{
    return *cut_;
}

void CutStuff::setCut(std::unique_ptr<OsiRowCut>& new_cut)
{
    cut_ = std::move(new_cut);
    updateAggregData(getNumTheta());
}

void CutStuff::Dump(const std::string& fname) const
{
    const CoinPackedVector& row = getCut().row();
	
    std::ofstream file_out;
	file_out.open(fname.c_str(), ios::trunc);
	// indices
    file_out << row.getIndices()[0];
	for (int i = 1; i < row.getNumElements(); i++) {
		file_out << "," << row.getIndices()[i];
	}
    file_out << '\n';

    // elements
    file_out << std::setprecision(20) << row.getElements()[0];
    for (int i = 1; i < row.getNumElements(); i++) {
        file_out << "," << std::setprecision(20) << row.getElements()[i];
    }
    file_out << '\n';
	
	// ub and lb
	file_out << std::setprecision(20) << getCut().ub() << '\n';
	file_out << std::setprecision(20) << getCut().lb() << '\n';

    // num_cols, num_thetas, iteration, type
    file_out << num_cols_ << '\n' << getNumTheta() << '\n' << cut_creation_iteration_ << '\n' << type_ << '\n';
	file_out.close();
}

void CutStuff::Print() const
{
    getCut().print();
    std::string type_str;
    switch (type_) {
    case PNB_CUT_TYPE_OPTIMALITY:
        type_str = "optimality";
        break;
    case PNB_CUT_TYPE_FEASIBILITY:
        type_str = "feasibility";
        break;
    case PNB_CUT_TYPE_EPIGRAPH:
        type_str = "L&P epigraph";
        break;
    case PNB_CUT_TYPE_DOMAIN:
        type_str = "L&P domain";
        break;
    case PNB_CUT_TYPE_CVAR:
        type_str = "CVaR";
        break;
    default:
        throw std::invalid_argument("Unknown cut type");
    }

    std::string column_set_str;
    switch (column_set_) {
    case PNB_COL_SET_NONE:
        column_set_str = "none";
        break;
    case PNB_COL_SET_PRIME:
        column_set_str = "prime";
        break;
    case PNB_COL_SET_DOUBLE_PRIME:
        column_set_str = "double-prime";
        break;
    default:
        throw std::invalid_argument("Unknown column set");
    }

    std::cout << "- Creation Iteration: " << cut_creation_iteration_ << "\n- Type: " << type_str << "\n- Column set: " << column_set_str << std::endl;
    std::cout << "- To use: " << to_use_ << "\n- Not active iterations: " << not_active_iterations_ << "\n- Active value: " << active_value << "\n- Index in solver: " << current_index_in_solver_ << std::endl;
}

bool CutStuff::is_consistent() const
{
	std::bitset<CutStuff::InconsistencyType::Count> inconsistencies;
	
	//The row element vector does not have duplicate indices
    //The row element vector indices are >= 0
    const OsiRowCut& cut = getCut();
    if (!cut.consistent()) {
		inconsistencies.set(CutStuff::InconsistencyType::InconstRow);
    }
    
    int num_theta = getNumTheta();
  
    //Check the cut values
    std::vector<int> calculated_aggregates(0);
    if (cut.row().getNumElements() > num_cols_ + getNumberAggregates()) {
		inconsistencies.set(CutStuff::InconsistencyType::TooManyElems);
    }
    bool started_theta(false);
    for (int i = 0; i < cut.row().getNumElements(); i++) {
        double val = cut.row().getElements()[i];
        int ind = cut.row().getIndices()[i];
        if (val == 0.0) { //no zero elements in row PackedVector
 			inconsistencies.set(CutStuff::InconsistencyType::ZeroRowElems);
        }
        if (ind >= num_cols_) { // it is theta
            calculated_aggregates.push_back(ind-num_cols_);
            if (val != 1.0) { //thetas should have 1.0 value or be ommited in the PackedVector
				inconsistencies.set(CutStuff::InconsistencyType::ThetaNotOne);
            }
            started_theta = true;
        }
        else if (started_theta) { // thetas should go after other coefs
			inconsistencies.set(CutStuff::InconsistencyType::ThetaBeforeCoef);
        }
    }
    std::sort(calculated_aggregates.begin(), calculated_aggregates.end());

    //Error in members
    if (calculated_aggregates.size() != getNumberAggregates()) {
		inconsistencies.set(CutStuff::InconsistencyType::WrongAggreg);
    }
    if (calculated_aggregates != getAggregates()) {
		inconsistencies.set(CutStuff::InconsistencyType::WrongAggreg);
    }
    if (getNumberAggregates() > num_theta) {
		inconsistencies.set(CutStuff::InconsistencyType::TooManyAggreg);
    }
    if (cut_creation_iteration_ < 0) {
		inconsistencies.set(CutStuff::InconsistencyType::InvalidCutCreation);
    }
    if (column_set_ < PNB_COL_SET_NONE || column_set_ > PNB_COL_SET_DOUBLE_PRIME) {
		inconsistencies.set(CutStuff::InconsistencyType::InvalidColumnSet);
    }
    if (type_ < parallel_benders::PNB_CUT_TYPE_OPTIMALITY || type_ > PNB_CUT_TYPE_CVAR) {
		inconsistencies.set(CutStuff::InconsistencyType::InvalidCutType);
    }

    //Epigraph and Domain cuts are always PNB_COL_SET_NONE
    if ((type_ == parallel_benders::PNB_CUT_TYPE_EPIGRAPH || type_ == parallel_benders::PNB_CUT_TYPE_DOMAIN) && column_set_ != PNB_COL_SET_NONE) {
		inconsistencies.set(CutStuff::InconsistencyType::WrongColumnSet);
    }
    
    //Aggregate type
    int aggregate_type = getAggregateType();
    if ((type_ == parallel_benders::PNB_CUT_TYPE_FEASIBILITY || type_ == parallel_benders::PNB_CUT_TYPE_DOMAIN) && aggregate_type != CutStuff::AGGREGATE_OVER_NONE) {
		inconsistencies.set(CutStuff::InconsistencyType::WrongAggregateFeasDomain);
    }
    else if (type_ == parallel_benders::PNB_CUT_TYPE_OPTIMALITY && aggregate_type == CutStuff::AGGREGATE_OVER_NONE) {
		inconsistencies.set(CutStuff::InconsistencyType::WrongAggregateOpt);
    }
    else if (type_ == parallel_benders::PNB_CUT_TYPE_EPIGRAPH) {
        if (num_theta == 1 && aggregate_type != CutStuff::AGGREGATE_OVER_ONE)
			inconsistencies.set(CutStuff::InconsistencyType::WrongAggregateEpigr);
        if (num_theta > 1 && aggregate_type != CutStuff::AGGREGATE_OVER_ALL)
			inconsistencies.set(CutStuff::InconsistencyType::WrongAggregateEpigr);
    }
    else if (type_ == parallel_benders::PNB_CUT_TYPE_CVAR) {
        throw runtime_error("Not implemented.");
    }

	bool consistent = inconsistencies.none();

	return consistent;
}

double CutStuff::AbsoluteViolation(const std::vector<double>& col_solution, const std::vector<double>& theta_values)
{
    assert(is_consistent());
    LOG_IF(FATAL, column_set_ != PNB_COL_SET_NONE && (col_solution.size() % 2 != 0 || col_solution.size()/2 != num_cols_)) << "Solution size should be even.";
    LOG_IF(FATAL, column_set_ != PNB_COL_SET_NONE && theta_values.size() % 2 != 0) << "Theta values size should be even.";
    
    //solution_column_shift is used to get columns from double prime column set in PNB CGP
    int solution_column_shift = (column_set_ == PNB_COL_SET_DOUBLE_PRIME ? num_cols_ : 0);

    double theta_value(0.0);
    for (auto theta_index : getAggregates()) {
        theta_index += column_set_ == PNB_COL_SET_DOUBLE_PRIME ? theta_values.size()/2 : 0;
        theta_value += theta_values[theta_index];
    }
    LOG_IF(FATAL, (type_ == PNB_CUT_TYPE_FEASIBILITY || type_ == PNB_CUT_TYPE_DOMAIN) && theta_value != 0.0);

    const CoinPackedVector& row = getCut().row();
    const double* elements = row.getElements();
    const int* indices = row.getIndices();
    int num_elements = row.getNumElements();
    double sum = 0.0;
    for (int i = 0; i < num_elements; i++) {
        if (indices[i] < num_cols_) { // we need only columns (not theta)
            sum += col_solution[indices[i] + solution_column_shift] * elements[i];
        }
        else {
            break;
        }
    }

    return getCut().lb() - sum - theta_value; // Violation = cut lower bound - cut_vector * column_solution - Theta
}

double CutStuff::RelativeViolation(const std::vector<double>& col_solution, const std::vector<double>& theta_values)
{
    const OsiRowCut& cut = getCut();
    if (cut.lb() == 0.0) {
        return -1.0;
    }
    return AbsoluteViolation(col_solution, theta_values)/abs(cut.lb());
}

double CutStuff::DistanceViolation(const std::vector<double>& col_solution, const std::vector<double>& theta_values)
{
    return AbsoluteViolation(col_solution, theta_values)/getCut().row().twoNorm();
}

bool CutStuff::isAggregate(int aggregate) const {

    if (aggregate + 1 > getNumTheta()) {
        throw std::invalid_argument("Aggregate number is out of theta range");
    }
    return aggregates_.test(aggregate);
}

void CutStuff::setAggregate(int aggregate)
{
    int num_theta = getNumTheta();
    OsiRowCut& cut = getMutableCut();
    if (aggregate+1 > num_theta) {
        throw std::invalid_argument("Aggregate number is out of theta range");
    }
    if (isAggregate(aggregate)) {
        assert(cut.row()[num_cols_ + aggregate] == 1.0);
        return;
    }
    assert(cut.row()[num_cols_ + aggregate] == 0.0);
    cut.mutableRow().insert(num_cols_ + aggregate, 1.0);

    updateAggregData(num_theta);
}

void CutStuff::resetAggregates(int num_theta /* = -1*/ )
{
    int old_num_aggregs = getNumberAggregates();
    OsiRowCut& cut = getMutableCut();
    CoinPackedVector& row = cut.mutableRow();
    row.truncate(row.getNumElements() - old_num_aggregs);

    if (num_theta == -1) {
        num_theta = getNumTheta();
    }
    updateAggregData(num_theta);
}

std::shared_ptr<CutStuff> LoadCutStuff(const std::string& fname, int thread_id)
{
    std::fstream file;
    file.open(fname.c_str()); // should be the list of relative paths to .sto (.stoch) files
    LOG_IF(FATAL, !file.is_open()) << "Could not open " << fname;

    std::string line;
    int line_number = 0;
    std::vector<int> cut_indices{};
    std::vector<double> cut_elements{};
    double ub(0);
    double lb(0);
    int num_cols(0), num_thetas(0), iteration(-1), cut_type(-1);
    while (getline(file, line, '\n')) {
        assert(line_number < 8); // indices, coefs, ub, lb, num_cols, num_thetas, iteration, type
        int from = 0;
        int to = 0;
        int len = 0;
        if (line_number == 0) { // indices
            std::stringstream ss(line);
            for (int i; ss >> i;) {
                cut_indices.push_back(i);
                if (ss.peek() == ',')
                    ss.ignore();
            }
        }
        else if (line_number == 1) { // coefs
            std::stringstream ss(line);
            for (double i; ss >> i;) {
                cut_elements.push_back(i);
                if (ss.peek() == ',')
                    ss.ignore();
            }
        }
        else if (line_number == 2) { // ub
            ub = std::stod(line);
        }
        else if (line_number == 3) { // lb
            lb = std::stod(line);
        }
        else if (line_number == 4) { // num_cols
            num_cols = std::stoi(line);
        }
        else if (line_number == 5) { // num_thetas
            num_thetas = std::stoi(line);
        }
        else if (line_number == 6) { // iteration
            iteration = std::stoi(line);
        }
        else if (line_number == 7) { // type
            cut_type = std::stoi(line);
        }
        ++line_number;
    }
    file.close();

    OsiRowCut* cut = new OsiRowCut();
    cut->setLb(lb);
    cut->setUb(ub);
    cut->setRow(cut_indices.size(), cut_indices.data(), cut_elements.data(), false);
    std::unique_ptr<OsiRowCut> cut_pointer(cut);

    std::shared_ptr<CutStuff> cut_stuff(new CutStuff(iteration, thread_id, cut_pointer, cut_type, num_cols, num_thetas));

    LOG_IF(FATAL, !cut_stuff->is_consistent());

    return cut_stuff;
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
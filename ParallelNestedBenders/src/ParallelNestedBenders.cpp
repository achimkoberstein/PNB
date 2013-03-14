#include "ParallelNestedBenders.hpp"

#include <assert.h>
#include <array>
#include <stdlib.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <utility> // std::pair

#include <mutex>

#include <math.h>
#include <sstream>


#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/multi_array.hpp"
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/math/special_functions/round.hpp>

#define GOOGLE_GLOG_DLL_DECL // Has to be done to prevent linking issues!
#include <glog/logging.h>
// Include all the other files

#include "CoinHelperFunctions.hpp"
#include "CoinPackedMatrix.hpp"
#include "CoinWarmStart.hpp"
#include "CoinWarmStartBasis.hpp"
#include "OsiRowCut.hpp"
#include "OsiSolverInterface.hpp"
#include "OsiCpxSolverInterface.hpp"
#include "SmiScnModel.hpp"
#include "SmiCoreCombineRule.hpp"
#include "CoinTime.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"
#include "ilcplex/cpxconst.h"
//#include "ilcplex/cplexcheck.h"
//#include "check.c"
#include "OsiCpxSolverInterface.hpp"


// Gurobi stuff
/*#include "OsiGrbSolverInterface.hpp"
extern "C" {
#include "gurobi_c.h"
}*/


namespace parallel_benders {

    void ParallelNestedBenders::AdjustProblem()
    {
        num_stages_ = core_->getNumStages();
        num_scenarios_ = smi_->getNumScenarios(); // The leaf nodes do not work correctly!

        if ( bounds_in_matrix_ ){
            smi_->moveBoundsIntoMatrix();
            //smi_->writeSmps("no_bounds",true,false);
            bounds_in_matrix_ = false;
        }
        // Check for integrality right away
        if (( core_->getIntegerLength() || core_->getBinaryLength()) ){ // Set boolean variable (For now)
            contains_integer_ = true;
            // Set stage wise integers
            // All binary are integer, but not all integer are binary
            const int* integer_indices = core_->getIntegerIndices();
            for (int i = 0; i < num_stages_;i++){
                if ( !core_->getIntCols(i).empty() )
                    integer_problems_[i] = true;
            } 

            //store_primal_and_dual_ = false; // No dual values can be stored for an integer problem..
            store_cut_information_ = false; // Therefore no cut aggregation


        }
        typedef std::map<int,std::vector< std::pair<int,double> > > non_staircase_row_map;
        non_staircase_row_map non_staircase_rows;
        // Check for non-staircase structure
        for (int stage = 2; stage < num_stages_; stage++){
            // Go through the core node
            SmiNodeData* current_node = core_->getNode(stage);
            // Go through all the rows. 
            if ( current_node->getNumMatrixElements() ){
                // We split the matrix entries into W and T 
                for ( int j = core_->getRowStart(stage); j < core_->getRowStart(stage)+core_->getNumRows(stage); j++){
                    const double* row_elements = current_node->getRowElements(j);
                    const int* row_indices = current_node->getRowIndices(j);
                    int row_length = current_node->getRowLength(j);
                    bool non_staircase_row = false;
                    for ( int k = 0; k < row_length; k++ ){
                        if ( row_indices[k] < core_->getColStart(stage-1) ){ // Column index is from 2 stages before the current stage. This means, we have a non-staircase structure. Store all the indices in a std::map[]
                            non_staircase_row = true;
                            break;
                        }
                    }
                    if ( non_staircase_row ){
                        non_staircase_problem_ = true;
                        for ( int kk = 0; kk < row_length; kk++)
                            non_staircase_rows[j].push_back(make_pair(row_indices[kk],row_elements[kk]));
                    }
                }
            }
        }
        //// What do we want to do?
        //if ( non_staircase_problem_ ){ // We have non_staircase_rows! How do we handle these?
        //    // Iterate over all rows
        //    BOOST_FOREACH( non_staircase_row_map::value_type vec_ref,non_staircase_rows){
        //        for (int i = 0; i < vec_ref.second.size(); i++){
        //            // Do something here!
        //            LOG(FATAL) << "Not yet implemented.";
        //        }
        //    }
        //    // We have to go through it from bottom to top.
        //    // If we have a non-staircase row in the last stage, we have to find the coefficient of the lowest stage.
        //    // Then we have to insert new rows with free variables ( and most likely stochastic coefficients.. ) in all stages lower than the last stage.
        //    // Because it would be necessary to change the whole SmiTree to handle Non-Staircase structure, we can add non-staircase handling behaviour inside our algorithm.
        //    // We have to manipulate the last stage in the following way: insert the new column, and insert the new row.
        //    // 

        //}
        // 1. Check for stochasticity of the model 
        for (int i = 0; i < num_stages_; i++)
            thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AdjustProblemInStage,this,i) );
        thread_pool_.wait(0);

        // If we have a non-staircase problem, handle it!


    }

    void ParallelNestedBenders::AdjustProblemInStage( int stage )
    {
        BendersStageData* benders_stage_ptr = stage_data_[stage];
        // Set stage dependent data
        benders_stage_ptr->stage_ = stage;
        benders_stage_ptr->num_columns_ = core_->getNumCols(stage);
        benders_stage_ptr->num_rows_ = core_->getNumRows(stage);

        // Initialize upper and lower bounds
        benders_stage_ptr->upper_bound_ = infinity_;
        benders_stage_ptr->lower_bound_ = -infinity_;


        // Build core T and W matrix
        // What do I need? Assign column to either W or T matrix, so we can keep it organized.
        // 

        // Reserve place. FIXME: This is necessary for CPLEX. Cplex needs pointers to the first and the last index ( which is one over the end, just like iterators )
        // We fill these arrays, so that cplex can use it to change the RHS, Columns bound and objective function coefficients
        if ( ! benders_stage_ptr->column_indices_.empty () )
            benders_stage_ptr->column_indices_.clear();
        if ( ! benders_stage_ptr->row_indices_.empty () )
            benders_stage_ptr->row_indices_.clear();

        benders_stage_ptr->column_indices_.reserve(benders_stage_ptr->num_columns_);
        benders_stage_ptr->row_indices_.reserve(benders_stage_ptr->num_rows_);
        for ( int i = 0; i < benders_stage_ptr->num_columns_+1; i++ ){
            benders_stage_ptr->column_indices_.push_back(i);            
        }
        for ( int i = 0; i < benders_stage_ptr->num_rows_+1; i++ ){
            benders_stage_ptr->row_indices_.push_back(i);            
        }
        // End of CPLEX necessary adjustments
        bool no_staircase = false;
        bool staircase_in_stage = false;
        // We split the matrix entries into W and T 
        if ( stage > 1){
            for ( int j = core_->getRowStart(stage); j < core_->getRowStart(stage)+benders_stage_ptr->num_rows_; j++){
                const double* row_elements = core_->getNode(stage)->getRowElements(j);
                const int* row_indices = core_->getNode(stage)->getRowIndices(j);
                int row_length = core_->getNode(stage)->getRowLength(j);
                for ( int k = 0; k < row_length; k++ ){
                    if ( row_indices[k] < core_->getColStart(stage-1))
                        no_staircase = true;
                    if ( no_staircase )
                        break;
                }
                if ( no_staircase ){
                    staircase_in_stage = true; no_staircase = false; non_staircase_problem_ = true;
                    // Choose a std::mutex to use here TODO
                    console_mutex_.lock();
                    for (int kk = 0; kk < row_length; kk++){
                        if ( core_->getColStage(row_indices[kk]) < stage-1 ) // Only for these coefficients we need constraints..
                            non_staircase_coefficients_[row_indices[kk]] = max(non_staircase_coefficients_[row_indices[kk]],stage);
                    } 
                    console_mutex_.unlock();
                }
            }
        }

        // Look at core data to get values for the matrices..
        for ( auto it = tree_nodes_by_stage_[stage].begin(); it != tree_nodes_by_stage_[stage].end(); ++it){
            // Look at current node
            BendersNodeData* benders_node_ptr = (*it)->getDataPtr();
            SmiNodeData* node_ptr = benders_node_ptr->smi_node_data_;
            if ( !node_ptr->isCoreNode()){ // Makes only sense for non-core nodes
                // Find stochastic T and W matrix, but only until we found out out if we have a stochastic_matrix_t 
                if ( node_ptr->getNumMatrixElements() && !( benders_stage_ptr->stochastic_matrix_w_ && benders_stage_ptr->stochastic_matrix_t_ && !staircase_in_stage) ){
                    // We split the matrix entries into W and T 
                    for ( int j = core_->getRowStart(stage); j < core_->getRowStart(stage)+benders_stage_ptr->num_rows_; j++){
                        const double* row_elements = node_ptr->getRowElements(j);
                        const int* row_indices = node_ptr->getRowIndices(j);
                        int row_length = node_ptr->getRowLength(j);
                        for ( int kk = 0; kk < row_length; kk++){
                            if ( row_indices[kk] >= core_->getColStart(stage))
                                benders_stage_ptr->stochastic_matrix_w_ = true;
                            else if ( stage > 0 && row_indices[kk] >= core_->getColStart(stage-1))
                                benders_stage_ptr->stochastic_matrix_t_ = true;
                            else{
                                no_staircase = true;
                            }

                        }

                        if ( no_staircase ){
                            // We store the row_indices of the current row and all the indices
                            non_staircase_stochastic_problem_ = true; no_staircase = false; non_staircase_problem_ = true;
                            //no_staircase_indicator[j] = true;
                            //non_staircase.push_back(std::vector<std::pair<int,double> > () ); 
                            //non_staircase.back().push_back(make_pair<int,double>(j,0)); // We store at first the index of the row.
                            //for (int kk = 0; kk < row_length; kk++){
                            //    non_staircase.back().push_back(make_pair<int,double>(row_indices[kk],row_elements[kk]));
                            //} // What we also need: Coefficients of the core matrix.
                        }// end if
                    }//end inner for
                }//end if


                if (!( node_ptr->getCoreCombineRule() == SmiCoreCombineReplace::Instance() || node_ptr->getCoreCombineRule() == SmiCoreCombineAdd::Instance())){
                    console_mutex_.lock();
                    LOG(FATAL) << "The supplied SMPS format is not understood. You used something that is not REPLACE in your stoch file.";
                    console_mutex_.unlock();
                }

                //benders_node_ptr->no_staircase_constraints_ = non_staircase;

                // Are there random bounds?
                if ( node_ptr->getLength(node_ptr->getCloStart()) || node_ptr->getLength(node_ptr->getCupStart()) )
                    benders_stage_ptr->stochastic_bounds_ = true;

                // Are there random objective function values?
                if ( node_ptr->getLength(node_ptr->getObjStart()) )
                    benders_stage_ptr->stochastic_objective_ = true;

                // Are there random right hand sides?
                if ( node_ptr->getLength(node_ptr->getRloStart()) || node_ptr->getLength(node_ptr->getRupStart()) )
                    benders_stage_ptr->stochastic_rhs_ = true;
            }//end non-core node check
        }//end for

        if ( !benders_stage_ptr->stochastic_bounds_ ){// If we have no stochastic bounds, we might want to check for non_zero_bounds
            // Also check if we have bounds other than >= 0 and <= 0 ( important for feasibility cut generation )
            const double* col_lower = core_->getDenseColLower(stage);
            const double* col_upper = core_->getDenseColUpper(stage);
            for (int i = 0; i < benders_stage_ptr->num_columns_; i++){
                if ( (!(col_lower[i] == -infinity_ || col_lower[i] == 0)) || (!(col_upper[i] == 0 || col_upper[i] == infinity_)) ){
                    benders_stage_ptr->non_zero_bounds_ = true;
                    break;
                }
            }
        }
        else
            benders_stage_ptr->non_zero_bounds_ = true;
        // In the non-staircase case we now have all the stuff we need.

        if ( integer_problems_[stage] ){
            // Initialize an array with the coltype of every variable
            benders_stage_ptr->coltype_array_.resize(benders_stage_ptr->num_columns_,'C');
            std::vector<int> int_cols(core_->getIntCols(stage));
            for ( std::vector<int>::iterator it = int_cols.begin(); it != int_cols.end() ; ++it){
                // Depends on upper / and or lower bounds, but at first we only solve binary problems..
                if ( core_->getDenseColUpper(stage)[*it] == 1.0 && core_->getDenseColLower(stage)[*it] == 0.0)
                    benders_stage_ptr->coltype_array_[*it] = 'B';
                else 
                    benders_stage_ptr->coltype_array_[*it] = 'I';
            }
        }
        
    }

    ParallelNestedBenders::ParallelNestedBenders( SmiScnModel* smi_model_ptr ) :
    smi_(smi_model_ptr),core_(smi_->getCore()),osi_solver_(nullptr),osi_solver_map_(),tree_(),tree_nodes_by_stage_(),stage_data_(),
        num_stages_(core_->getNumStages()),num_scenarios_(smi_->getNumScenarios()),infinity_(1e20),contains_integer_(false),integer_problems_(core_->getNumStages()),parameter_(),
        zero_tolerance_(0.0),integer_tolerance_(0.0),absolute_epsilon_(0.0),relative_epsilon_(0.0),
        current_iteration_(0),insubstantial_iteration_(0),max_iteration_(5000),minor_iteration_(0),major_iteration_(0),feasible_iteration_(0),lower_bound_(-infinity_),upper_bound_(infinity_),old_lower_bound_(-infinity_),
        current_stage_(0),non_staircase_problem_(false),forward_(true),infeasible_problem_(false),unbounded_problem_(false),infeasible_problems_(smi_->getNumScenarios()),added_optimality_cuts_(num_stages_,0),added_feasibility_cuts_(num_stages_,0),num_added_feasibility_cuts_(0),store_cut_information_(false),adaptive_(false),aggregation_scheme_(0),no_aggregation_(true),cut_consolidation_add_cut_(true),cut_consolidation_strike_threshold_(5),cut_consolidation_partial_threshold_(0.0),regularization_(false),regularized_solution_value_(infinity_),regularized_sigma_(1.0),regularized_gamma_(0.75),regularized_initialized_cols_(false),regularized_initialized_rows_(false),regularized_start_row_(0),start_solution_(0),d_square_(false),d_square_translation_term_(0),timer_(num_stages_+1),wall_clock_solution_time_(1),detailed_timer_(4),solution_times_(4),maximal_solution_time_(0),solution_code_(0),
        num_cpu_(0),console_mutex_(),solver_map_mutex_(),stage_warm_start_mutex_(),upper_bound_mutex_(),infeasible_problem_mutex_(),osi_solver_mutex_(),add_cut_counter_mutex_(),stage_lower_upper_bounds_mutex_(),misc_mutex_(),thread_pool_(0),
        warm_start_(false),warm_start_scenarios_(false),warm_start_stage_(false),only_one_feasibility_cut_(true),generate_feasibility_and_optimality_cuts_(false),store_primal_and_dual_(false),
        sequencing_protocol_(0),second_iteration_stage_(0),strategy_epsilon_(0.0),
        stage_lower_upper_bounds_(),old_lower_upper_bounds_(),temporary_stage_lower_bound_(-infinity_),temporary_stage_upper_bound_(infinity_),compute_stage_bounds_(false),at_last_iteration_(false),cuts_added_last_round_(0),theta_approximations_everywhere_(false),enforce_direction_(false),enforce_stage_(0),level_decomp_(false),level_quadratic_new_variables_(false),level_euclidean_(false),level_manhattan_(false),level_infinity_(false),
        oda_method_(false),level_lambda_(0.5),oda_kappa_(0.2),oda_gap_(infinity_),oda_lower_(-infinity_),oda_upper_(infinity_),oda_first_stage_(-infinity_),oda_phi_i_(-infinity_),oda_tilde_q_(-infinity_),oda_chosen_optimality_cut_by_scenario_(),oda_current_tilde_q_s_(),critical_iteration_(false),uncritical_iteration_counter_(0),maximal_number_of_uncritical_iterations_(0),critical_iteration_counter_(0),last_critical_iteration_gap_(infinity_),
        warm_up_(false),warm_up_solution_(nullptr),regularized_solution_(nullptr),regularized_update_incumbent_(false),stage_aggregation_(false),begin_aggregation_stage_(0),end_aggregation_stage_(0),aggregate_from_start_(true),seperate_first_stage_solver_(true),seperate_stage_solver_(false),solve_relaxation_(PNB_INT_NO_RELAXATION),debug_test_duplicate_indices_(false),pretty_printing_(false),pretty_printing_verbose_(true),compute_primal_solutions_for_stage_(PNB_INT_SOLUTION_PRIMAL_NO),compute_dual_solutions_for_stage_(PNB_INT_SOLUTION_DUAL_NO),compute_objective_values_(false),final_solution_run_(false),first_stage_original_col_lower_(nullptr),first_stage_original_col_upper_(nullptr),fixed_solution_(nullptr),fixed_first_stage_(false),non_staircase_coefficients_(),optimality_cut_counter_(0),intra_cut_aggregation_time_(0),adaptive_cut_time_(0),current_solution_value_(0.0),d_square_c_triple_timer_(0.0),d_square_c_rhs_timer_(0.0),solution_similiarity_counter_(0),max_similarity_iteration_(10)
    {

    }

    void ParallelNestedBenders::Init()
    {
        // Do not sync console output (makes no difference)
        //ios_base::sync_with_stdio(true);
        // Enable Buffering
        // 
        bool debug_first_stage = false;
        int buffer_size = 16384;
        if ( !debug_first_stage)
            setvbuf(stdout, nullptr, _IOFBF, buffer_size); // Full Bufferung, IOLBF = Line Buffering, IONBF = No Buffering

        num_stages_ = core_->getNumStages();
        num_scenarios_ = smi_->getNumScenarios();
        integer_problems_ = boost::dynamic_bitset<>(num_stages_);
        infeasible_problems_ =  boost::dynamic_bitset<>(num_scenarios_);
        timer_.Start(num_stages_);
        // Initialize osiSolver with cplex, if no solver is given
        if (smi_->getOsiSolverInterface() == nullptr)
            osi_solver_ = new OsiCpxSolverInterface();
        else
            osi_solver_ = smi_->getOsiSolverInterface()->clone();
        // Load empty problem (i.e. reset the solver interface
        osi_solver_->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
        // Cplex specific settings come later

        // Set infinity to solver infinity
        infinity_ = osi_solver_->getInfinity();
        lower_bound_ = -infinity_;
        upper_bound_ = infinity_;
        old_lower_bound_ = lower_bound_;
        regularized_solution_value_ = infinity_;

        int num_theta = 1; // Single cut as default
        int num_scen_pro_cut = 0; // Single cut as default.

        // Iterate over property map..
        boost::property_tree::ptree& param(parameter_.parameter_);

        // Specify if we want to keep solutions or not
        compute_primal_solutions_for_stage_ = param.get(PNB_COMPUTE_PRIMAL_SOLUTIONS,PNB_INT_SOLUTION_PRIMAL_NO);
        compute_dual_solutions_for_stage_ = param.get(PNB_COMPUTE_DUAL_SOLUTIONS,PNB_INT_SOLUTION_DUAL_NO);
        compute_objective_values_ = param.get(PNB_COMPUTE_OBJECTIVE_VALUES,false);
        // Memory switch: Compute solutions and store them during algorithm?
        if ( compute_primal_solutions_for_stage_ == PNB_INT_SOLUTION_PRIMAL_ALL )
            compute_primal_solutions_for_stage_ = num_stages_-1;
        if ( compute_dual_solutions_for_stage_ == PNB_INT_SOLUTION_DUAL_ALL )
            compute_dual_solutions_for_stage_ = num_stages_-1;
        if ( compute_primal_solutions_for_stage_ >= num_stages_)
            compute_primal_solutions_for_stage_ = num_stages_-1;
        assert(compute_primal_solutions_for_stage_ < num_stages_);

        // Tolerances
        zero_tolerance_ = param.get(PNB_TOLERANCE_ZERO,0.0);
        integer_tolerance_ = param.get(PNB_TOLERANCE_INTEGER,0.0);
        absolute_epsilon_ = param.get(PNB_EPSILON_ABSOLUTE,0.0);
        relative_epsilon_ = param.get(PNB_EPSILON_RELATIVE,0.0);

        // Solution time
        maximal_solution_time_ = param.get(PNB_MAXIMAL_SOLUTION_TIME,3600); // Maximal solution time is 1 hour by default!
        // WarmStart
        int warm_start_version = param.get(PNB_WARM_START,0);
        if ( warm_start_version > 0 )
            warm_start_ = true;
        if ( warm_start_version == 1 )
            warm_start_scenarios_ = true;
        if ( warm_start_version == 2 )
            warm_start_stage_ = true;

        // Problem formulation
        bounds_in_matrix_ = param.get(PNB_BOUNDS_IN_MATRIX,false);

        // LP method
        lp_method_ = param.get(PNB_LP_METHOD,PNB_INT_DUAL_SIMPLEX);
        feas_cut_lp_method_ = param.get(PNB_FEAS_CUT_LP_METHOD,PNB_INT_DUAL_SIMPLEX);

        // Multicut Aggregation
        int value = param.get(PNB_AGGREGATED_OPTIMALITY_CUTS,1);
        if ( value <= 0 ){ // We use the multi cut method
            // Use something you like
            num_theta = num_scenarios_; // With this it is ensured that the multicut method is used.
        }
        else
            num_theta = value;

        // Adaptive aggregation
        adaptive_ = param.get(PNB_DYNAMIC_AGGREGATION,false);

        // Intra-Cut Aggregation
        store_cut_information_ = param.get(PNB_CUT_CONSOLIDATION,false);
        cut_consolidation_add_cut_ = param.get(PNB_CUT_CONSOLIDATION_CUT_COMPUTATION,true);
        cut_consolidation_strike_threshold_ = param.get(PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD,5);
        cut_consolidation_partial_threshold_ = param.get(PNB_CUT_CONSOLIDATION_PARTIAL_THRESHOLD,0.0);

        // Warm Up Period
        start_solution_ = param.get(PNB_USE_START_SOLUTION,0);
        // Start_solution=0 Nothing
        if ( start_solution_ > PNB_INT_NO_START_SOLUTION)// At the moment we only have EV solution 
            warm_up_ = true;

        // Choice of algorithm
        int algorithm = param.get(PNB_BENDERS,PNB_INT_ALGORITHM_BENDERS); // Use Benders as default
        if ( algorithm == PNB_INT_ALGORITHM_REGULARIZATION )
            regularization_ = true;
        if ( algorithm == PNB_INT_ALGORITHM_REGULARIZATION_ODA ){
            regularization_ = true;
            oda_method_ = true;
        }
        if ( algorithm == PNB_INT_ALGORITHM_LEVEL ){
            level_decomp_ = true;
            level_euclidean_ = true;
        }
        if ( algorithm == PNB_INT_ALGORITHM_LEVEL_ODA ){
            level_decomp_ = true;
            oda_method_ = true;
            level_euclidean_ = true;
        }
        if ( algorithm == PNB_INT_ALGORITHM_BENDERS_ODA ){
            oda_method_ = true;
        }
        if ( algorithm == PNB_INT_ALGORITHM_LEVEL_MANHATTAN ){
            level_decomp_ = true;
            level_manhattan_ = true;
            level_quadratic_new_variables_ = false;
        }
        if ( algorithm == PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA ){
            level_decomp_ = true;
            level_manhattan_ = true;
            level_quadratic_new_variables_ = false;
            oda_method_ = true;
        }
        if ( algorithm == PNB_INT_ALGORITHM_LEVEL_INFINITY ){
            level_decomp_ = true;
            level_infinity_ = true;
            level_quadratic_new_variables_ = false;
        }
        if ( algorithm == PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA){
            level_decomp_ = true;
            level_infinity_ = true;
            level_quadratic_new_variables_ = false;
            oda_method_ = true;
        }
        level_lambda_ = param.get(PNB_LEVEL_LAMBDA,0.5);
        oda_kappa_ = param.get(PNB_ODA_KAPPA,0.2);

        if ( level_decomp_ && (level_lambda_ <= 0.0 || level_lambda_ >= 1.0)){
            // We do not want to use level_decomp.
            LOG(FATAL) << "Wrong parameter: Lambda out of specification : " << level_lambda_;
        }
        if ( oda_method_ ){
            if ( level_decomp_ ){
				// TODO: Reenable after first test set run, see if anything breaks..
                //if ( oda_kappa_ <= 0 || oda_kappa_ >= (1-level_lambda_)*(1-level_lambda_))
                //    LOG(FATAL) << "Level parameter are wrong. 0 < lambda < 1, 0 < kappa < (1-lambda)^2.";
            }
            if ( oda_kappa_ <= 0.0 || oda_kappa_ >= 1.0)
                LOG(FATAL) << "Kappa is out of range : " << oda_kappa_;
        }
        if ( num_stages_ > 2){
            if ( level_decomp_ || oda_method_)
                LOG(FATAL) << "Level Decomposition is not available for Multi-Stage Problems.";
        }

        // Regularization
        if ( regularization_ ){
            compute_stage_bounds_ = true;
            if ( start_solution_ == PNB_INT_NO_START_SOLUTION ){
                LOG(WARNING) << "Using EEV solution as default start solution, as none was specified";
                warm_up_ = true; // We need EEV start solution!
                start_solution_ = PNB_INT_USE_EV_SOLUTION;
            }
            seperate_first_stage_solver_ = true;
        }

        // Tree walking procedure
        sequencing_protocol_ = param.get(PNB_TREE_WALK_STRATEGY,0);
        second_iteration_stage_ = param.get(PNB_TREE_WALK_STRATEGY_STAGE,0);
        assert( second_iteration_stage_ >= 0);
        if ( sequencing_protocol_ == PNB_INT_SECOND_ITERATION ){ // In this case we can either disable the second iteration OR set it to lowest possible level
            if ( second_iteration_stage_ == 0 || second_iteration_stage_ >= num_stages_-1){
                sequencing_protocol_ = PNB_INT_FASTPASS;
                param.put(PNB_TREE_WALK_STRATEGY,PNB_INT_FASTPASS); // 0 is not a viable option, num_stages-1 neither, so we switch to fastpass if this is specified.
            }
            else
                param.put(PNB_TREE_WALK_STRATEGY_STAGE,second_iteration_stage_); // Update parameter to reflect that we care.
            // TODO: If we aggregate stages, we do not care about second_iteration_stage!
        }
        strategy_epsilon_ = param.get(PNB_TREE_WALK_STRATEGY_EPSILON,0.0); // Needed for epsilon-variants!
        if ( strategy_epsilon_ < relative_epsilon_ )
            strategy_epsilon_ = relative_epsilon_;

        switch (sequencing_protocol_){
            case PNB_INT_FAST_BACKWARD:
                strategy_epsilon_ = relative_epsilon_;
            case PNB_INT_FAST_FORWARD:
                strategy_epsilon_ = relative_epsilon_;
            case PNB_INT_EPSILON_FAST_BACKWARD:
            case PNB_INT_EPSILON_FAST_FORWARD:
            case PNB_INT_FASTPASS_WITH_FASTBACK:
                compute_stage_bounds_ = true;
                store_primal_and_dual_ = true;
                break;
            default:
                break;

        }
        // 
        // Feasibility problems
        int feasibility_cut_handling = param.get(PNB_HANDLE_FEAS_CUTS,PNB_INT_FEAS_CUT_ONLY_ONE);
        switch ( feasibility_cut_handling){
            case PNB_INT_FEAS_CUT_ONLY_ONE:
                only_one_feasibility_cut_ = true;
                break;
            case PNB_INT_FEAS_CUT_ALL:
                only_one_feasibility_cut_ = false;
                break;
            case PNB_INT_FEAS_CUT_WITH_OPT:
                only_one_feasibility_cut_ = false;
                generate_feasibility_and_optimality_cuts_ = true;
                break;
            default:
                only_one_feasibility_cut_ = true;
                break;
        }


        // Thread related stuff
        num_cpu_ = param.get(PNB_NUM_CPU,0);
        int available_cpus = boost::thread::hardware_concurrency();
        if ( num_cpu_ <= 0 )
            num_cpu_ = available_cpus;
        else
            num_cpu_ = std::min(num_cpu_,available_cpus);
        assert( num_cpu_ > 0);
        param.put(PNB_CPU_USED,num_cpu_);
        thread_pool_ = boost::threadpool::pool(num_cpu_); // Can we resize the threadpool?

        // Set first stage solver to own instance
        seperate_first_stage_solver_ = true;

        // create our benders node objects and build the tree
        // Initialize benders node data objects and tree!
        // 
        InitializeBendersNodeDataTree(num_scen_pro_cut);

        // Create Benders Stage data
        stage_data_.reserve(num_stages_);
        for (int i = 0; i < num_stages_; i++){
            stage_data_.push_back(new BendersStageData);
            // Set empty warm start object
            if ( warm_start_ && warm_start_stage_ )
                stage_data_.back()->warm_start_.reset(osi_solver_->getEmptyWarmStart());
            // Set number of thetas per stage, if we want to do it statically
            stage_data_.back()->num_thetas_ = num_theta;
        }

        // Multistage Aggregation
        stage_aggregation_ = param.get(PNB_STAGE_AGGREGATION,false);
        if ( stage_aggregation_ ){
            begin_aggregation_stage_ = param.get(PNB_STAGE_AGGREGATION_START,0);
            end_aggregation_stage_ = param.get(PNB_STAGE_AGGREGATION_END,0);
            if ( end_aggregation_stage_ == PNB_INT_STAGE_AGGREGATION_LAST_STAGE )
                end_aggregation_stage_ = num_stages_-1;
            if ( begin_aggregation_stage_ == PNB_INT_STAGE_AGGREGATION_LAST_STAGE ){
                begin_aggregation_stage_ = num_stages_-1;
                stage_aggregation_ = false;
            }
            if ( begin_aggregation_stage_ < PNB_INT_STAGE_AGGREGATION_LAST_STAGE )
                begin_aggregation_stage_ = std::max(0, num_stages_ - (PNB_INT_STAGE_AGGREGATION_LAST_STAGE-begin_aggregation_stage_) );
            if ( begin_aggregation_stage_ == 0 && end_aggregation_stage_ == num_stages_-1 ){
                stage_aggregation_ = false;
                LOG(ERROR) << "You specified stage aggregation that would result in the deterministic equivalent. We therefore disable stage aggregation for this run!";
            }
            aggregate_from_start_ = param.get(PNB_STAGE_AGGREGATION_DIRECTION,true);
        }

            // Initialize all upper and lower bounds
        stage_lower_upper_bounds_.reserve(num_stages_);
        for (int i = 0; i < num_stages_;i++){
            stage_lower_upper_bounds_.emplace_back( std::make_pair(-infinity_,infinity_));
            old_lower_upper_bounds_.emplace_back( std::make_pair(-infinity_,infinity_));
        }
        stage_lower_upper_bounds_[num_stages_-1].first = 0; // We do not have another stage in the last one.
        

        // Set values depending on others
        if ( adaptive_ ){
            store_primal_and_dual_ = true;
            // We need to set an aggregation_scheme
            aggregation_scheme_ = PNB_INT_AGGREGATION_STRATEGY_BY_INACTIVE_ITERATION;
            //seperate_first_stage_solver_ = false; // TODO: Until we worked out how to deal with first stage problem directly, we turn it of
        }
        if ( store_cut_information_){
            store_primal_and_dual_ = true; 
        }

        // Assign timer 
        build_problem_timer_ = 0;
        solve_problem_timer_ = 1;
        cut_generation_timer_ = 2;

        // Solve LP relaxation instead IP problem?
        solve_relaxation_ = param.get(PNB_RELAXATION,PNB_INT_NO_RELAXATION);


        // Set options for solver!
        SetSolverOptions(osi_solver_);
        // Unsilence first stage solver!
        if ( debug_first_stage )
            osi_solver_->messageHandler()->setLogLevel(2);
        // Use only one thread per cplex instance (except for the first sage

        // Initialize all benders node data objects with the correct cut aggregation policy. TODO
        timer_.Stop(num_stages_);
    }

    void ParallelNestedBenders::Solve()
    {
        if ( thread_pool_.size() == 0){
            solution_code_ = PNB_SOLUTION_PARAMETER_PROBLEM;
            return;
        }
        int setup_timer = num_stages_;
        timer_.Start(setup_timer);
        AdjustProblem();
        // We can stop here, if we have integer variables on more than the second stage..
        if ( integer_problems_[0] ){ // In the first stage.
            LOG(INFO) << "Your problem contains first-stage integer variables.";
        }
        std::vector<int> new_columns(num_stages_,0);
        if ( non_staircase_problem_ ){
            // We need to generate the non-staircase constraints
            LOG(INFO) << "Transforming to a non-staircase problem..";
            smi_->transformToStaircaseStructure(non_staircase_coefficients_);
            non_staircase_problem_ = false;
            non_staircase_stochastic_problem_ = false;
            non_staircase_coefficients_.clear();
            // Accomodate the changes we did in the transformation step!
            AdjustProblem();
            assert( !non_staircase_problem_ );
            assert( !non_staircase_stochastic_problem_);
            LOG(INFO)<< "Done.";
        }
        if ( integer_problems_[0] && solve_relaxation_ == 0 )
            integer_problems_[0] = false;
        for (int i = 1; i < num_stages_;i++){
            LOG_IF(WARNING,integer_problems_[i]) << "Your problem contains integer variables at stage " << i+1 << ".";
            if ( solve_relaxation_ >= i ){
                integer_problems_[i] = false; // Ensure we do solve the LP relaxation! (for second stage?)
                LOG(INFO) << "We solve the LP relaxation for stage"  << i << ".";
            }

            // TODO: If we want D_square_ we have to remove the && false statement
            if ( integer_problems_[i] ){ // If we have second stage integer problems, use the D^2 algorithm. (actually we might need binary variables for that and some other stuff.. but lets try it anyway..
                // We could enable the D^2 algorithm (with LaporteLouveaux Cuts) only if we have no mixed-integer first stage!
                const std::vector<int>& binary_cols(core_->getBinaryCols(0)); // Get binary cols of first stage!
                if ( binary_cols.size() == stage_data_[0]->num_columns_ )
                {
                    d_square_ = true;
                    store_primal_and_dual_ = true; // We need primal solution after we solved the subproblems!
                    // If we want to solve the actual MIPS, we need to set integer_problems_[i] to true, else to flase
                    integer_problems_[i] = false;
                    // We need a translation term! (Solve second stage LP problem and adjust!) D^2 requires theta to be >= 0.
                    d_square_translation_term_ = 20000; // Is taken for LaporteLoveaux Cuts as the minimum value of second stage problems!
                    d_square_w_iteration_.resize(stage_data_[1]->num_columns_,-1);
                    LOG(INFO) << "Enabling D^2 algorithm.";
                }
                else {
                    // We have a problem with first-stage mixed-integer and second stage integers. We can not solve such a problem!
                    LOG(ERROR) << "You tried to solve a problem with mixed-integer or pure integer first stage and mixed-integers at another stage. We can not solve this, only the relaxation. You can specify the stage from which you would like to relax the integer constraints with param PNB_RELAXATION. First Stage = 0";
                    solution_code_ = PNB_SOLUTION_PARAMETER_PROBLEM;
                    return;
                }
            }
        }
        // Stage Aggregation!
        if ( stage_aggregation_){
            //smi_->writeSmps("beforeStageAgg",true,false);
            smi_->aggregateStages(begin_aggregation_stage_,end_aggregation_stage_, aggregate_from_start_); // Aggregate stage 0 and 1
            /*smi_->writeSmps("stageAggreg",true,false);*/
            num_stages_ = core_->getNumStages();
            num_scenarios_ = smi_->getNumScenarios();
            InitializeBendersNodeDataTree(tree_nodes_by_stage_[0][0]->getDataPtr()->num_scen_pro_cut_); // Reinitialize the whole tree, based on the updated smi_ tree
            AdjustProblem(); // We need to set the important properties again!
            // Best way would be to just recreate the benders_node_data tree from scratch!
        }

        // Some tests for sequencing protocols.
        if ( num_stages_ == 2 ){ // No need for sequencing protocols in the two-stage case
            sequencing_protocol_ = 0;
        }
        if ( second_iteration_stage_ > num_stages_-2 && sequencing_protocol_ == PNB_INT_SECOND_ITERATION) // Fix second iteration stage by disabling it!
            sequencing_protocol_ = PNB_INT_FASTPASS;
        bool made_one_turn = false;

        // Initialize solver instance for every thread!
        current_stage_ = 1;
        for ( int i = 0; i < num_cpu_*4; i++ ){ // Should suffice to get every thread its own solver..
            // Schedule the tasks for this stage
            thread_pool_.schedule(boost::bind(&ParallelNestedBenders::SetSolverByThread,this));
        }
        thread_pool_.wait(0); // We wait until all threads are done
        current_stage_ = 0;

        // Compute or Use initial solutions!
        HandleInitialSolutionProcedures();

        double old_gap = 0;
        double gap = 0;
        int gap_counter = 0;
        int gap_counter_maximum = 1000;

        // Variables needed for dynamic strategy PNB_INT_FASTPASS_WITH_FASTBACK
        double threshold = 1;
        bool time_not_measured = true;
        double initial_time = 0;
        std::vector<double> initial_time_for_stage(num_stages_,0.0);
        int critical_stage = second_iteration_stage_; // The minimum critical stage, which is always viable is the second stage
        if ( critical_stage > 0 ){// If we have a critical stage that is not trivial, we say that we already have measured the time, i.e. do not set critical stage!
            time_not_measured = false;
        }
        double percent_threshold = 0.1;

        // Variables for pretty printing
        int last_major_iteration = 0;
        int last_current_iteration = 0;
        double last_lower_bound = lower_bound_;
        double last_upper_bound = upper_bound_;
        double last_gap = infinity_;

        timer_.Stop(setup_timer);
        wall_clock_solution_time_.Start(0);
        // Print initial solution line
        std::cout << left << std::setw(7) << "It." << std::setw(16) << "Lower Bound: " << std::setw(16) << "Upper Bound: " << std::setw(16) << "Relative Gap: " << std::endl; // This has to be done only once!
        // Do initialization stuff necessary for solve process
        bool not_stop_loop = false;
        while ( true  ) 
        {
            if ( !regularization_)
                not_stop_loop = lower_bound_ < upper_bound_ &&
                std::abs(upper_bound_-lower_bound_) > absolute_epsilon_ &&  
                RelativeGapPresent(upper_bound_,lower_bound_,infinity_,relative_epsilon_) &&
                current_iteration_-insubstantial_iteration_ < max_iteration_ &&
                solution_similiarity_counter_ < max_similarity_iteration_;
            else
            { // Regularization!
                if ( regularized_solution_value_ >= infinity_)
                    not_stop_loop = true;
                else if ( current_stage_ == 0){
                    double solution_qx = 0;
                    double ten_to_minus_ten = 1e-10;
                    double ten_to_minus_eight = 1e-8;
                    for (int i = 0; i < num_stages_; i++)
                        solution_qx += stage_lower_upper_bounds_[i].second;
                    not_stop_loop = (std::abs(regularized_solution_value_-solution_qx)/ ( std::abs(regularized_solution_value_)+ten_to_minus_ten) > ten_to_minus_eight);
                    // Compute Other Gap
                    if ( lower_bound_ > -infinity_ ){
                        double regularized_gap = std::abs(regularized_solution_value_-lower_bound_)/(std::abs(regularized_solution_value_)+ten_to_minus_ten);
                        LOG(INFO) << "Current regularized gap " << regularized_gap;
                        if ( start_solution_ == PNB_INT_COMPUTE_EV_SOLUTION )
                            if ( current_iteration_ <= 2 )
                                not_stop_loop = true;
                    }
                    else // If lower_bound is <= -infinity, we have not yet added any optimality cuts to the problem, so we can not stop!
                        not_stop_loop = true;
                }

            }
            if ( !not_stop_loop ){ // We stop the loop!
                LOG_IF(INFO,solution_similiarity_counter_ >= max_similarity_iteration_) << "Stopped due to solution similarity for " << solution_similiarity_counter_ << " iterations.";
                if ( solution_similiarity_counter_ >= max_similarity_iteration_ )
                    solution_code_ = PNB_SOLUTION_SIMILARITY_LIMIT;
                if ( current_iteration_-insubstantial_iteration_ >= max_iteration_ )
                    solution_code_ = PNB_SOLUTION_ITERATION_LIMIT;
                if ( upper_bound_ < lower_bound_-zero_tolerance_ )
                    solution_code_ = PNB_SOLUTION_UPPER_LESS_LOWER_BOUND;
                break;
            }
            wall_clock_solution_time_.Stop(0);


            // Check if we already went over the time limit
            if ( ((int) wall_clock_solution_time_.GetTime(0)) > maximal_solution_time_ ){
                LOG(ERROR) << "We stopped solving the problem after " << wall_clock_solution_time_.GetTime(0)/60 << " minutes.";
                solution_code_ = PNB_SOLUTION_TIME_LIMIT;
                return; // We stop solving the problem
            }
            wall_clock_solution_time_.Start(0);
            timer_.Start(current_stage_);
            // Reset warm start indicator
            stage_data_[current_stage_]->problem_was_solved_in_this_iteration_ = false;
            if ( upper_bound_ < lower_bound_ ){
                if ( !regularization_)
                    LOG(INFO) << "Upper Bound is lower than lower Bound. Iteration: " << current_iteration_;
            }
            if ( old_lower_bound_ > (lower_bound_ + ParallelNestedBenders::zero_tolerance_) && old_lower_bound_ > -infinity_  && !infeasible_problem_){
                if ( !regularization_)
                    LOG(INFO) << "New Lower bound " << lower_bound_ << " is less than old lower bound " << old_lower_bound_;
            }
            if ( !infeasible_problem_ ){
                // Count if gap is similar
                if ( current_stage_ == 0 ){
                    old_gap = gap;
                    gap = abs(upper_bound_-lower_bound_)/ (abs(lower_bound_)+1e-10); // Be consistent!
                    if ( upper_bound_ >= infinity_ || lower_bound_ <= -infinity_ )
                        gap = infinity_;
                    if ( regularization_ ){
                        double solution_qx = 0;
                        double ten_to_minus_ten = 1e-10;
                        for (int i = 0; i < num_stages_; i++)
                            solution_qx += stage_lower_upper_bounds_[i].second;
                        gap = std::abs(regularized_solution_value_-solution_qx)/( std::abs(regularized_solution_value_)+ten_to_minus_ten);
                    }
                    if (CompareForEquality(old_gap,gap,zero_tolerance_) )
                        gap_counter++;
                    else{
                        gap_counter = 0;
                        solution_similiarity_counter_ = 0;
                    }
                    if ( gap_counter > gap_counter_maximum ){
                        LOG(INFO) << "Algorithm stops as gap does not narrow anymore for " << gap_counter << " successive iterations.";
                        solution_code_ = PNB_SOLUTION_GAP_NONIMPROVING;
                        break;
                    }
                    // Count critical iterations!
                    critical_iteration_ = false;
                    if ( (1-level_lambda_)*last_critical_iteration_gap_ > gap ){
                        maximal_number_of_uncritical_iterations_ = std::max(maximal_number_of_uncritical_iterations_,uncritical_iteration_counter_);
                        LOG(INFO) << "Critical iteration after " << uncritical_iteration_counter_;
                        uncritical_iteration_counter_ = 0;
                        last_critical_iteration_gap_ = gap;
                        critical_iteration_ = true;
                        critical_iteration_counter_++;
                    }
                }
            }
            else
                gap_counter = 0;

            if ( current_stage_ == 0 ){ // Things to do if we are in the first stage
                current_iteration_++;
                minor_iteration_++;
                uncritical_iteration_counter_++;
                warm_up_ = false; // We do not need it anymore, when we reached the first stage!
            }
            if ( sequencing_protocol_ == PNB_INT_FAST_FORWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_FORWARD){
                if ( current_stage_ == 0)
                    major_iteration_++;
            }
            else {
                if ( current_stage_ == num_stages_-1)// We only have a major iteration, if we are at the last stage!? Depends on sequencing protocol? Full sweep is major iteration.
                    major_iteration_++;
            }

            // Change direction depending on last or first stage. This holds for all sequencing protocols.
            if ( current_stage_ == 0 ){
                forward_ = true;
                if ( !infeasible_problem_ )
                    feasible_iteration_++;
                // If we had infeasible problems in the last backward run, we added all the feas cuts and can start fresh
                infeasible_problem_ = false;
            }
            else if ( current_stage_ == num_stages_ -1)
                forward_ = false;

            if ( current_stage_ == enforce_stage_ )
                enforce_direction_ = false;

            // Cautious or FastBackward : Always go back, but not when no new cuts got generated!
            if ( theta_approximations_everywhere_ && (sequencing_protocol_ == PNB_INT_FAST_BACKWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_BACKWARD) ){
                if ( current_stage_ > 0 && current_stage_ < num_stages_-1){
                    forward_ = false;
                    cuts_added_last_round_ = added_optimality_cuts_[current_stage_-1]; // count how many cuts were present before this iteration!
                }

            }

            // Try the shuffle or FastForward Strategy
            if ( (sequencing_protocol_ == PNB_INT_FAST_FORWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_FORWARD) && !forward_ && !enforce_direction_){
                if ( current_stage_ > 0 && current_stage_ < num_stages_-1){// We can update the upper bound, because we do not do that elsewhere if we use this strategy.
                    // Decide wether we want to go forward again 
                    // We want to test if the lower bound for this stage is close enough to the upper bound
                    // We compute the absolute difference here between approximation and actual results!, not the discrepancy!
                    double current_upper_bound = 0;
                    for (int i = current_stage_; i < num_stages_; i++)
                        current_upper_bound += stage_lower_upper_bounds_[i].second;
                    if ( RelativeGapPresent(current_upper_bound,stage_lower_upper_bounds_[current_stage_].first,infinity_,strategy_epsilon_*std::min(std::abs(lower_bound_),std::abs(upper_bound_))) ){// If true, values are not equal. 
                        forward_ = true; // We want to move forward again, with the newest cuts..
                        // Check for nearby values
                        if ( std::abs(old_lower_upper_bounds_[current_stage_].first-stage_lower_upper_bounds_[current_stage_].first) < absolute_epsilon_ 
                            && std::abs(old_lower_upper_bounds_[current_stage_].second-stage_lower_upper_bounds_[current_stage_].second) < absolute_epsilon_  ){
                                forward_ = false;
                        }
                        if ( forward_ ){
                            current_iteration_++; // Lets see the impact of this.
                        }
                    }
                    else {
                        // We have to check if old_lower_bound and so are the same. if yes, we must go forward to reach something new, don't we?
                    }
                }
            }

            if ( current_stage_ == num_stages_-1 ){// We are in last stage
                at_last_iteration_ = true;
                if ( sequencing_protocol_ == PNB_INT_FASTPASS_WITH_FASTBACK)
                    made_one_turn = true; // Needed to indicate that we made a whole major iteration!
            }

            // Initialize lower/upper bounds for this stage.
            temporary_stage_lower_bound_ = 0;
            temporary_stage_upper_bound_ = 0;

            if ( !pretty_printing_ ){
                std::string direction(forward_ ? "F " : "B ");
                std::cout << minor_iteration_ << "Iteration " << direction << current_iteration_ << "." << current_stage_ << std::endl;
            }
            if ( current_stage_ == 0) // lower_bound_ is only computed in the first stage!
                old_lower_bound_ = lower_bound_;

            if ( d_square_ && current_stage_ == 1 && current_iteration_ > 1 ){
                if ( old_lower_bound_ == lower_bound_ ){
                    // Compare real solutions here!
                    bool not_the_same = true;
                    for (int i = 0; i < old_first_stage_solution_.size(); i++){
                        if ( !CompareForEquality(old_first_stage_solution_[i],i < stage_data_[0]->num_columns_ ? tree_nodes_by_stage_[0][0]->getDataPtr()->col_solution_[i] : tree_nodes_by_stage_[0][0]->getDataPtr()->next_stage_approximation_[i-stage_data_[0]->num_columns_],zero_tolerance_ )){
                            d_square_lower_bound_counter_ = 0;
                            not_the_same = false;
                            break;
                        }
                    }
                    if ( not_the_same )
                        d_square_lower_bound_counter_++;
                }
                else {
                    d_square_lower_bound_counter_ = 0;
                }
                if ( d_square_lower_bound_counter_ > 1 ){
                    // No difference between lower and upper bound
                    // Solve MIP subproblems!
                    stage_data_[current_stage_]->solve_binary_subproblems_ = true;
                    integer_problems_[current_stage_] = true;
                    d_square_lower_bound_counter_ = 0;
                }
                else {
                    stage_data_[current_stage_]->solve_binary_subproblems_ = false;
                    integer_problems_[current_stage_] = false;
                }
            }
            /************************************************************************/
            /* ODA-Level Decomposition                                              */
            /************************************************************************/
            if ( oda_method_ && current_stage_ > 0 && added_optimality_cuts_[0] > 0 ){
                oda_chosen_optimality_cut_by_scenario_.clear(); oda_current_tilde_q_s_.clear();
                oda_chosen_optimality_cut_by_scenario_.resize(num_scenarios_,0);
                oda_current_tilde_q_s_.resize(num_scenarios_,0.0);
                int num_nodes_oda = tree_nodes_by_stage_[current_stage_].size();
                // Store time needed to compute the oda_stuff in addition to normal Benders!
                solution_times_.Start(2);
                for ( int i = 0; i < num_nodes_oda; i++ ){
                    // Schedule the tasks for this stage
                    thread_pool_.schedule(boost::bind(&ParallelNestedBenders::BuildAndSolveOnDemandAccuracyProblem,this,tree_nodes_by_stage_[current_stage_][i]));
                    //BuildAndSolveOnDemandAccuracyProblem(tree_nodes_by_stage_[current_stage_][i]);
                }
                thread_pool_.wait(0); // We wait until all threads are done
                oda_tilde_q_ = std::accumulate(oda_current_tilde_q_s_.begin(),oda_current_tilde_q_s_.end(),oda_first_stage_);
                solution_times_.Stop(2);

            }
            // Find out if we still need to solve the second stage subproblems when using oda ( with everything entailled )
            bool solve_subproblems = true;
            if ( gap_counter < 1 && oda_method_ && current_stage_ > 0 && added_optimality_cuts_[0]> 0 ){
                if ( !level_decomp_) // We need to set oda_phi_i, which is not done for methods not using level_decomp!
                    oda_phi_i_ = lower_bound_;
                /*               LOG(INFO) << "Tilde Q : " << oda_tilde_q_;
                LOG(INFO) << "Phi_" << current_iteration_ << " : " << oda_phi_i_;
                LOG(INFO) << "Upper Bound : " << upper_bound_;
                LOG(INFO) << "Tilde Q : " << oda_tilde_q_ << " >= " << oda_kappa_*oda_phi_i_ + (1-oda_kappa_)*upper_bound_;*/

                if (oda_tilde_q_ >= oda_kappa_*oda_phi_i_ + (1-oda_kappa_)*upper_bound_ ){
                    solution_times_.Start(2);
                    LOG(INFO) << " Subproblems do not get solved, we add our own cuts.";
                    // Do not solve subproblems, but add aggregated cut(s)
                    for (int i = 0; i < tree_nodes_by_stage_[0][0]->getDataPtr()->oda_aggregates_.size(); i++){
                        thread_pool_.schedule(boost::bind(&ParallelNestedBenders::GenerateAggregatedCuts,this,tree_nodes_by_stage_[0][0],i));
                    }
                    thread_pool_.wait(0);
                    solution_times_.Stop(2);
                    solve_subproblems = false;
                    timer_.Stop(current_stage_);
                    current_stage_--; 
                    assert(current_stage_ == 0);
                    insubstantial_iteration_++;
                    major_iteration_--; // We accidentaly incremented the major_iteration counter!
                    continue; // Skip the rest of the loop (we go to the first stage)
                }
            }

            /************************************************************************/
            /* Solve the subproblems                                                */
            /************************************************************************/
            int num_nodes = tree_nodes_by_stage_[current_stage_].size();
            for ( int i = 0; i < num_nodes; i++ ){
                // Schedule the tasks for this stage
                thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]));
            }
            thread_pool_.wait(0); // We wait until all threads are done

            // Update the temporary stage lower/upper bounds!
            for ( auto node : tree_nodes_by_stage_[current_stage_]){
                temporary_stage_lower_bound_ += node->getDataPtr()->lower_bound_*node->getDataPtr()->probability_;
                temporary_stage_upper_bound_ += node->getDataPtr()->solution_value_*node->getDataPtr()->probability_;
            }

            bool can_update_upper_bound = !warm_up_;
            if ( d_square_ && current_stage_==1){
                // Solve C^3-SLP
                if ( stage_data_[current_stage_]->violated_index_ >= 0){
                    can_update_upper_bound = false;
                    //    BuildAndSolveCTripleProblem();
                    //    // Clear non-integer scenarios
                    //    stage_data_[current_stage_]->d_square_disjunctive_indices_.push_back(stage_data_[current_stage_]->violated_index_);
                    //    stage_data_[current_stage_]->non_integer_scenarios.clear();
                    //    stage_data_[current_stage_]->violated_index_ = -1;

                    //    // Before we resolve the problems, we need to reset temporary bounds
                    //
                    // temporary_stage_lower_bound_ = 0.0;
                    // temporary_stage_upper_bound_ = 0.0;
                    //
                    //    int num_nodes = tree_nodes_by_stage_[current_stage_].size();
                    //    for ( int i = 0; i < num_nodes; i++ ){
                    //        // Schedule the tasks for this stage
                    //        thread_pool_.schedule(boost::bind(&ParallelNestedBenders::BuildAndSolveRHSProblem,this,tree_nodes_by_stage_[current_stage_][i]));
                    //    }
                    //    thread_pool_.wait(0); // We wait until all threads are done
                    //    stage_data_[current_stage_]->num_d_square_cuts_++; // We generated a D^2-C^3 cut
                    //    stage_data_[current_stage_]->row_indices_.push_back(stage_data_[current_stage_]->row_indices_.back()+1); // We added a permanent cut (increase row indices)


                    //    std::pair<boost::thread::id, OsiSolverInterface* > id_solver_pair;
                    //    BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
                    //        //id_solver_pair.second->reset(); // This method throws exception with Cplex
                    //        id_solver_pair.second->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
                    //        // Workaround: Load empty problem. 
                    //        assert(id_solver_pair.second->getNumCols() == 0);
                    //    }
                    //    // Resolve all the problems!
                    //    for ( int i = 0; i < num_nodes; i++ ){
                    //        // Schedule the tasks for this stage
                    //        thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]));
                    //    }
                    //    thread_pool_.wait(0); // We wait until all threads are done
                    //    
                    if ( stage_data_[current_stage_]->violated_index_ < 0)
                        can_update_upper_bound = true;
                }
                if ( stage_data_[current_stage_]->solve_binary_subproblems_ ){//|| stage_data_[current_stage_]->violated_index_ < 0 ) { 
                    // Now we need to add a cut to the master-problem, from LaporteLoveaux
                    // We need finite lower bound for the second stage, this is supposed to be our translation amount (of course with a negative sign)
                    // We need the current expected value of second stage problems, this is temporary_stage_upper_bound
                    // We need the first stage integer (binary) cols and the current first stage integer solution!

                    double cut_rhs = 0;
                    double lower_bound_for_second_stage = -d_square_translation_term_;
                    double current_expected_second_stage_value_function = temporary_stage_upper_bound_;
                    double cut_coefficient_scalar = current_expected_second_stage_value_function-lower_bound_for_second_stage;
                    std::vector<int> integer_cols = core_->getIntCols(current_stage_-1); // Assume they are sorted!

                    //if ( integer_cols.size() != stage_data_[current_stage_-1]->num_columns_ ){
                    //    // We also have non-binary ( i.e. continuous )variables in the first stage.
                    //    // Compute obj value part of continuous variables
                    //    double continuous_part = 0.0;
                    //    const double * obj_coefficients = seperate_first_stage_solver_ ? osi_solver_->getObjCoefficients() : core_->getDenseObjCoefficients(current_stage_-1);
                    //    for (int variable_index = 0; variable_index < stage_data_[current_stage_-1]->num_columns_; variable_index++){
                    //        if ( std::find(integer_cols.begin(),integer_cols.end(),variable_index) != integer_cols.end() )
                    //            continue;
                    //        else {
                    //            assert(tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->col_solution_ != nullptr);
                    //            continuous_part += obj_coefficients[variable_index]*tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->col_solution_[variable_index];
                    //        }
                    //    }
                    //    cut_coefficient_scalar -= continuous_part;
                    //}
                    std::vector<double> cut_coefficients;
                    cut_coefficients.reserve(integer_cols.size());
                    // Get first stage solution and compute cut coefficients
                    std::vector<double>& col_solution = tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->col_solution_;
                    int count_binaries_valued_one = -1;
                    // Laporte-Louveaux Paper 1993, we have - and + exchanged, because we move these values to the other side of >= compared with the paper
                    for ( std::vector<int>::iterator it  = integer_cols.begin();it < integer_cols.end(); ++it){
                        if ( CompareForEquality(col_solution[*it], 1,integer_tolerance_) ){
                            cut_coefficients.push_back(-cut_coefficient_scalar);
                            count_binaries_valued_one++;
                        }
                        else if ( CompareForEquality(col_solution[*it],0,integer_tolerance_) )
                            cut_coefficients.push_back(+cut_coefficient_scalar);
                        else
                            LOG(INFO) << "Not at bound: " << col_solution[*it];
                    }
                    // Compute RHS
                    cut_rhs += lower_bound_for_second_stage;
                    cut_rhs -= cut_coefficient_scalar*count_binaries_valued_one;
                    if ( d_square_ )
                        cut_rhs += d_square_translation_term_;

                    // Get number of theta variables for first stage
                    int num_thetas = tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->num_thetas_;
                    for (int i = 0; i < num_thetas; i++){
                        integer_cols.push_back(stage_data_[current_stage_-1]->num_columns_+i);
                        cut_coefficients.push_back(1.0);
                    }
                    assert( integer_cols.size() == cut_coefficients.size());

                    // Add cut
                    std::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
                    cut_pointer->setLb(cut_rhs);
                    cut_pointer->setUb(infinity_);
                    cut_pointer->setRow(integer_cols.size(),&integer_cols[0],&cut_coefficients[0]);
                    std::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
                    cut_stuff->aggregate_number_ = -1;
                    cut_stuff->to_use_ = true;
                    tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->optimality_cuts_.push_back(cut_stuff);
                    if ( seperate_first_stage_solver_ && current_stage_ == 1 && tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->last_iteration_start_optimality_ == tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->optimality_cuts_.end())
                        tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->last_iteration_start_optimality_ = --tree_nodes_by_stage_[current_stage_-1][0]->getDataPtr()->optimality_cuts_.end();
                    LOG(INFO) << "Current Expected value of second stage value function: " << current_expected_second_stage_value_function;
                }

                // Clear non-integer scenario
                stage_data_[current_stage_]->non_integer_scenarios.clear();
                stage_data_[current_stage_]->violated_index_ = -1;
            }

            bool update_upper_bound = false;
            if ( current_stage_ == num_stages_-1 && infeasible_problems_.none() && !warm_up_ ){// Update upper bound only, when we have no infeasible problems!
                theta_approximations_everywhere_ = true; // Only needed in case of FAST_BACKWARD and EPSILON_FAST_BACKWARD and FASTPASS_WITH_FASTBACK
                // This ensures that we get back to the first stage when we finally reached the last stage! Important only for FastBackward protocols.
                if ( sequencing_protocol_ == PNB_INT_EPSILON_FAST_BACKWARD || sequencing_protocol_ == PNB_INT_FAST_BACKWARD){
                    enforce_stage_ = 0;
                    enforce_direction_ = true;
                }
            }

            // Update stage wise upper/lower bounds, only if we did not had infeasible problems.
            if ( !infeasible_problem_ && infeasible_problems_.none() ){
                old_lower_upper_bounds_[current_stage_].first = stage_lower_upper_bounds_[current_stage_].first;
                old_lower_upper_bounds_[current_stage_].second = stage_lower_upper_bounds_[current_stage_].second;
                stage_lower_upper_bounds_[current_stage_].first = temporary_stage_lower_bound_;
                stage_lower_upper_bounds_[current_stage_].second = temporary_stage_upper_bound_;

                if ( current_stage_ == num_stages_-1 && can_update_upper_bound ){
                    // Update upper bound of problem
                    double sum_stage_solutions = 0;
                    for (int i = 0; i < num_stages_; i++)
                        sum_stage_solutions += stage_lower_upper_bounds_[i].second;
                    if ( upper_bound_ > sum_stage_solutions && can_update_upper_bound){
                        upper_bound_ = sum_stage_solutions;
                        update_upper_bound = true;
                        solution_similiarity_counter_ = 0;
                    }
                    // Update node-wise upper bounds! (only needed for Level Decomposition in Multi-Stage Problems)
                    if ( level_decomp_ && num_stages_ > 2)
                        for ( int temp_stage = num_stages_-2; temp_stage >= 0; temp_stage--){
                            for ( auto node : tree_nodes_by_stage_[temp_stage] ){
                                double temp_upper_bound = node->getDataPtr()->solution_value_;
                                // Get child node and all its siblings!
                                auto child_node = node->getChild();
                                temp_upper_bound += child_node->getDataPtr()->conditional_probability_* (temp_stage == num_stages_-2 ? child_node->getDataPtr()->solution_value_ : child_node->getDataPtr()->temporary_upper_bound_);
                                while ( child_node->hasSibling() ){
                                    child_node = child_node->getSibling();
                                    temp_upper_bound += child_node->getDataPtr()->conditional_probability_* (temp_stage == num_stages_-2 ? child_node->getDataPtr()->solution_value_ : child_node->getDataPtr()->temporary_upper_bound_);
                                }
                                node->getDataPtr()->temporary_upper_bound_ = temp_upper_bound;
                                if ( node->getDataPtr()->upper_bound_ > temp_upper_bound)
                                    node->getDataPtr()->upper_bound_ = temp_upper_bound;
                            }
                        }
                    // Check for solution similarity
                    // No similarity check for FF and epsilonFF ( has to be done in another way, if any)
                    if ( !(sequencing_protocol_ == PNB_INT_FAST_FORWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_FORWARD)){
                        // Count how long it is equal!
                        if ( CompareForEquality(sum_stage_solutions,current_solution_value_,relative_epsilon_) ){
                            // Increase counter
                            if ( !d_square_ )
                                solution_similiarity_counter_++;
                        }
                        else{
                            // Reset counter
                            solution_similiarity_counter_ = 0;
                        }
                        current_solution_value_ = sum_stage_solutions;
                    }
                }
                

                // Discrepancy: expected value of next stage with approximation for later stages - approximation of later stages at this stage.
                if ( current_stage_ < num_stages_-1 && current_stage_ > 0  ){ 
                    // Discrepancy makes sense on the way forward, but only for values actually solved!
                    double discrepancy = stage_lower_upper_bounds_[current_stage_].first + (stage_lower_upper_bounds_[current_stage_-1].second-stage_lower_upper_bounds_[current_stage_-1].first);
                    // What is the absolute gap?
                    double absolute_gap = upper_bound_-lower_bound_;
                    if ( lower_bound_ == -infinity_ || upper_bound_ == infinity_)
                        threshold = 1;
                    else
                    {
                        // Find number of significant bits of absolute_gap. Threshold is 10^(number of digits of absolute gap - 2)
                        // Example: Absolute Gap is 10. Then log10(10) is 1. Threshold is 10^-1 = 0.1
                        threshold = std::pow(10,std::log10(absolute_gap)-1);
                    }
                    if ( sequencing_protocol_ == PNB_INT_FASTPASS_WITH_FASTBACK ){
                        if (discrepancy > threshold && theta_approximations_everywhere_ && forward_ && made_one_turn){
                            forward_ = false; // Discrepancy is greater than treshold, so we do not go forward. Only if we are at stages, that are higher than the critical stage, we do not change the direction.
                        }
                        else if ( forward_ ) // Discrepancy is lower than threshold.
                            // We must decide if we want to check for discrepancy violations also at this stage, or not by setting made_one_turn!
                            // We decided the critical stage by the initial time it took to compute these stages!
                            if ( current_stage_ == critical_stage ) // We make a full sweep from here by setting made_one_turn to false
                                made_one_turn = false; 

                    }

                    if ( (sequencing_protocol_ == PNB_INT_EPSILON_FAST_BACKWARD || sequencing_protocol_ == PNB_INT_FAST_BACKWARD) && theta_approximations_everywhere_ && !enforce_direction_){
                        if ( current_stage_ > 0 && current_stage_ < num_stages_-1){
                            if ( discrepancy > (strategy_epsilon_/(num_stages_-1)) * std::min(std::abs(lower_bound_),std::abs(upper_bound_)) ){ // If discrepancy is greater than threshold, we want to reduce this gap by building new cuts!
                                forward_ = false;
                                enforce_stage_ = 0;
                                enforce_direction_ = true;
                            }
                            else 
                                forward_ = true;
                        }
                    }
                    // This can, at least in part, only reasonably be done, if we have not yet resolved the current stage, as the x of this stage has changed then!
                }
            }

            // In case we updated the upper bound, we now need to store all column solutions!
            if ( update_upper_bound) {
                for (int temp_stage = 0; temp_stage <= compute_primal_solutions_for_stage_; temp_stage++){
                    for ( auto node = tree_nodes_by_stage_[temp_stage].begin(); node != tree_nodes_by_stage_[temp_stage].end(); ++node){
                        // Move solution from col_solution to incumbent_col_solution!
                        StoreValuesFromSolverInVector((*node)->getDataPtr()->incumbent_col_solution_,&(*node)->getDataPtr()->col_solution_[0],stage_data_[temp_stage]->num_columns_);
                        // Store scenario wise solution value!
                        (*node)->getDataPtr()->incumbent_solution_value_ = (*node)->getDataPtr()->solution_value_*(*node)->getDataPtr()->probability_;
                    }
                }
            }

            if ( pretty_printing_ ){
                // Format output so that it looks nice
                if ( (major_iteration_ > last_major_iteration || (pretty_printing_verbose_ && current_iteration_ > last_current_iteration)) && current_stage_ == 0 ){
                    last_major_iteration = major_iteration_;
                    last_current_iteration = current_iteration_;
                    last_lower_bound = lower_bound_;
                    last_upper_bound = upper_bound_;
                    last_gap = std::abs(last_upper_bound-last_lower_bound) / std::abs(last_upper_bound+1e-10);
                    std::cout << "\n";
                }

                //ostringstream os;
                std::string direction_string("F");
                std::string iteration_string = boost::lexical_cast<std::string>(pretty_printing_verbose_ ? major_iteration_:last_major_iteration) + ( pretty_printing_verbose_ ? +"."+boost::lexical_cast<std::string>(current_iteration_) : "");
                if ( !forward_ )
                    direction_string = "B";
                std::cout << "\r" << left << std::setw(10) << iteration_string << std::setw(16) << last_lower_bound << std::setw(16) << last_upper_bound << std::setw(16) << last_gap << std::setw(5) << direction_string << current_stage_;


                // Output at which stage we are currently like this. Put current mode up Front, followed by current stage. F3, F4 F5, B6 . But let it overwrite in the same position of the stream. Is this possible? TODO
            }
            else {
                if ( regularization_ ){
                    if ( current_stage_ == num_stages_-1 ){
                        std::cout << "Current Incumbent solution " << regularized_solution_value_ << "\n";
                        double current_solution = 0;
                        for (int i = 0; i < num_stages_; i++)
                            current_solution += stage_lower_upper_bounds_[i].second;
                        double regularized_tolerance = 1e-10;
                        std::cout << "Current Iteration solution " << current_solution << "\n";
                        std::cout << "Relative Gap: " << abs(regularized_solution_value_-current_solution)/(abs(regularized_solution_value_)+regularized_tolerance ) << "\n";
                        std::cout << "Lower Bound: " << lower_bound_ << std::endl;
                    }
                }
                else if ( current_stage_ == 0){
                    std::cout << "Lower Bound:  " << lower_bound_ << "\n";
                    std::cout << "Upper Bound:  " << upper_bound_ << "\n";
                    std::cout << "Relative Gap: " << abs(upper_bound_-lower_bound_)/abs(upper_bound_+1e-10)  << std::endl;

                }

            }

            // If we had unbounded problems, we have to stop the algorithm.
            //if ( unbounded_problem_ ){
            //    if ( infeasible_problems_.any() ){
            //        unbounded_problem_ = false;
            //    }
            //    //else if ( current_stage_ == 0 && !tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_.empty() ){// Lets see if we already have optimality cuts added to the problem?!
            //    //    LOG(ERROR) << "Your problem got unbounded in stage " << current_stage_+1 << ".";
            //    //    solution_code_ = PNB_SOLUTION_UNBOUNDED;
            //    //    break;
            //    //}
            //    //else
            //    //    unbounded_problem_ = false;
            //}

            /*******************************************************************************/
            /*  Aggregate optimality cuts for nodes at the stage before the current stage  */
            /*******************************************************************************/
            if ( !forward_ && infeasible_problems_.none() ){ // TODO: if generate_feas_and_opt_cuts we need to change this
                // We need to aggregate optimality cuts for the parent node
                for ( int i = 0; i < tree_nodes_by_stage_[current_stage_-1].size(); i++)
                    // Schedule the tasks for this stage
                    thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AggregateOptimalityCuts,this,tree_nodes_by_stage_[current_stage_-1][i]));
                thread_pool_.wait(0); // We wait until all threads are done
                // If we added no cuts, we might want to notice that

                //if ( num_cpu_ > 1 ) {
                //    // Try to restore a deterministic ordering for the latest iterations!
                //    CutStuffLessComparison myobject;
                //    for ( int i = 0; i < tree_nodes_by_stage_[current_stage_-1].size(); i++){
                //        tree_nodes_by_stage_[current_stage_-1][i]->getDataPtr()->optimality_cuts_.sort(myobject);
                //    }
                //}
            }

            // If we have an infeasible subproblem, we already added the cuts to the corresponding parent problems.
            if ( infeasible_problems_.any() ){
                // Set boolean variable to indicate that during this backward run, we do not generate optimality cuts!.
                infeasible_problem_ = true;
                if ( current_stage_ == 0){
                    LOG(INFO) << "First stage problem is infeasible.";
                    // We have to stop the algorithm, our master problem is infeasible..
                    solution_code_ = PNB_SOLUTION_INFEASIBLE;
                    break;
                }
                // Change direction
                forward_ = false;
                // Reset infeasible_problems_ set
                infeasible_problems_.reset();
                // Add generated feasibility cuts to cut counter
                added_feasibility_cuts_[current_stage_-1] += num_added_feasibility_cuts_;
                // Reset num_added_feasibility_cuts_
                num_added_feasibility_cuts_ = 0;
                // If we encounter an infeasible problem, we go straight back to the first stage!
                enforce_direction_ = true;
                enforce_stage_ = 0;
                // Set bounds on problems to zero!
                if ( compute_stage_bounds_){
                    for (int i = current_stage_; i < num_stages_;i++){
                        stage_lower_upper_bounds_[i].second = -infinity_;
                    }
                }
            }

            // When we did not added any cuts, we have to go forward! (except last stage of course and when all problems were feasible!)
            if ( (sequencing_protocol_ == PNB_INT_EPSILON_FAST_BACKWARD || sequencing_protocol_ == PNB_INT_FAST_BACKWARD )&& theta_approximations_everywhere_ && !forward_){
                if ( current_stage_ > 0 && current_stage_ < num_stages_-1 && cuts_added_last_round_ == added_optimality_cuts_[current_stage_-1] && !infeasible_problem_ && !enforce_direction_){
                    forward_ = true;
                    LOG_IF(INFO,sequencing_protocol_==PNB_INT_EPSILON_FAST_BACKWARD) << "Going forward to next stage because of no new cuts!.";
                }
            }


            // We change the stage, so we should clear all of the osiSolverInterfaces we used
            std::pair<boost::thread::id, OsiSolverInterface* > id_solver_pair;
            BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
                //id_solver_pair.second->reset(); // This method throws exception with Cplex
                id_solver_pair.second->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
                // Workaround: Load empty problem. 
                assert(id_solver_pair.second->getNumCols() == 0);
            }



            // Depending on if we store primal and dual solution, we need to delete the memory allocated for its storage, to lower memory requirements! Only if we are not in the last run to compute solutions we want to return!.
            for (int i = 0; i < tree_nodes_by_stage_[current_stage_].size(); i++){
                tree_nodes_by_stage_[current_stage_][i]->getDataPtr()->CleanUnusedMemory(forward_);
            }

            stage_data_[current_stage_]->changed_aggregation_level_ = false; // Necessary for adaptive_ methods!


            // Decide the critical stage in case of PNB_INT_FASTPASS_WITH_FASTBACK strategy
            if ( sequencing_protocol_ == PNB_INT_FASTPASS_WITH_FASTBACK && time_not_measured && major_iteration_ > 0 && current_stage_ == 0 && theta_approximations_everywhere_){ // Store the time it takes to compute certain stages!
                // Store the time in a vector
                initial_time = timer_.GetAggregatedTime();
                for (int i = 0; i< num_stages_; i++)
                    initial_time_for_stage[i] = timer_.GetTime(i);
                time_not_measured = false;
                bool changed_bouncing_stage = false;
                LOG(INFO) << "Overall initial time" << initial_time;
                double overall_time = initial_time_for_stage[0];
                LOG(INFO) << "Percentage stage 0 " << initial_time_for_stage[0]/initial_time;
                for (int i = 1; i < num_stages_;i++){ // Assumption: Later stages take longer than earlier stages
                    overall_time+= initial_time_for_stage[i];
                    LOG(INFO) << "Percentage stage " << i << " " << initial_time_for_stage[i]/initial_time;
                    if ( overall_time/initial_time > percent_threshold ){
                        critical_stage = i;
                        changed_bouncing_stage = true;
                        break;
                    }
                }
                // Do some result checking
                if ( changed_bouncing_stage ){ // If we changed the bouncing stage
                    if ( critical_stage == num_stages_-1 && num_stages_ > 2){ // If we chose last stage as bouncing stage, our threshold was probably to high
                        //LOG(INFO)<< "Your chosen threshold for the tree walk strategy was set to high. Last stage was chosen.";
                    }
                }
                if ( !changed_bouncing_stage ){ // If we did not changed the bouncing stage
                    LOG(INFO)<< "Your chosen threshold for the tree walk strategy was set to high.";
                }
                //critical_stage = num_stages_-1;
                LOG(INFO) << "Bouncing stage is stage " << critical_stage;
            }
            timer_.Stop(current_stage_);
            // Change next stage, depending on where we are heading (forward or backward)
            forward_ ? current_stage_++ : current_stage_--;
        }
        timer_.Stop(current_stage_);
        if ( pretty_printing_ )
            std::cout << "\n";
        // Output the final values once again!
        std::cout << "Algorithm finished with status code " << solution_code_ << ", final values below.\n"; 
        std::cout << "Lower Bound:  " << lower_bound_ << "\n";
        std::cout << "Upper Bound:  " << upper_bound_ << "\n";
        std::cout << "Relative Gap: " << abs(upper_bound_-lower_bound_)/abs(upper_bound_+1e-10)  << "\n";
        std::cout << "Major Iterations: " << major_iteration_ << "\n";
        std::cout << "Overall Iterations: " << current_iteration_ << "\n";
        if ( insubstantial_iteration_ > 0)
            std::cout << "Insubstantial Iterations: " << insubstantial_iteration_ << "\n";
        int cut_counter = 0;
        for (int i = 0; i < num_stages_; i++){
            cut_counter += added_optimality_cuts_[i];
        }
        LOG(INFO) << "Added " << cut_counter << " optimality cuts.";
        cut_counter = 0;
        for (int i = 0; i < num_stages_; i++){
            cut_counter += added_feasibility_cuts_[i];
        }
        LOG(INFO) << "Added " << cut_counter << " feasibility cuts.";
        // We finished solving
        if (!infeasible_problem_){
            std::cout.setf(std::ios::fixed);
            std::cout << "Solution: " << upper_bound_ << std::endl;
            //LOG(INFO) << "Solution: " << upper_bound_;
            std::cout.unsetf(std::ios::fixed);
        }
        else{
            upper_bound_ = infinity_;
            LOG(INFO) << "Problem is infeasible.";
        }
		// For unbounded problem, set upper_bound_ to -infinity


        LOG_IF(INFO,store_cut_information_) << "Spent " << intra_cut_aggregation_time_ << " seconds to perform cut consolidation (over all CPU's)";
        LOG_IF(INFO,adaptive_) << "Spent " << adaptive_cut_time_ << " seconds to aggregate aggregates.";
        LOG_IF(WARNING,current_iteration_-insubstantial_iteration_ >= max_iteration_) << "Algorithm exceeded maximum number of iterations.";
        LOG_IF(WARNING,max_similarity_iteration_ < solution_similiarity_counter_) << "Algorithm stopped, because solution did not changed for " << solution_similiarity_counter_ << " consecutive iterations.";
        LOG_IF(INFO,d_square_) << "Spent " << d_square_c_triple_timer_ << " seconds to compute C^3 problems.";
        LOG_IF(INFO,d_square_) << "Spent "  << d_square_c_rhs_timer_ << " seconds to compute D^2 RHS problems.";

    }


    void ParallelNestedBenders::HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Check if we already got infeasible subproblems only, if we generate either feas or opt cuts
        if ( !generate_feasibility_and_optimality_cuts_ && only_one_feasibility_cut_ )
            if ( infeasible_problems_.any() )
                return; 

        //// Check if we got unbounded subproblems
        //if ( unbounded_problem_ )
        //    return; // There is no need to actually solve other problems, as we have unbounded problems..

        // For debugging reasons only!
        //std::unique_lock<std::mutex> u_lock(debug_mutex_);

        OsiSolverInterface* osi_solver = GetSolverByThread();

        if ( current_stage_ == 0){
            detailed_timer_.Start(0);
        }
#ifdef COMPUTING_TIMER
        double current_time = CoinCpuTime();
        double elapsed_build_time = 0;
#endif // COMPUTING_TIMER


        if ( !seperate_first_stage_solver_ || current_stage_ > 0 || (current_stage_ == 0 && current_iteration_ == 1)) 
            BuildSubproblem(osi_solver, benders_tree_node_pointer);
        AppendColsToSubproblem(osi_solver,benders_tree_node_pointer);
        AppendCutsToSubproblem(osi_solver,benders_tree_node_pointer);

        if ( current_stage_ == 0){
            detailed_timer_.Stop(0);
            solution_times_.Start(0);
        }
#ifdef COMPUTING_TIMER
        if ( current_stage_ == 1){
            elapsed_build_time = CoinCpuTime()-current_time;
            current_time = CoinCpuTime();
        }
#endif // COMPUTING_TIMER


        PrintSubproblem(osi_solver,benders_tree_node_pointer->scenario());
        SolveSubproblem(osi_solver,benders_tree_node_pointer);

        // In case of unbounded parent problem, we exchange the right-hand-side of the solver and resolve the problem (point problem)!
        if ( benders_tree_node_pointer->getDataPtr()->unbounded_problem_ ){
            // Exchange right-hand-side of problem! Resolve!
            // Only works for problems without cuts, I suppose?!
            for (int i = 0; i < stage_data_[current_stage_]->num_rows_;i++)
                osi_solver->setRowBounds(i,benders_tree_node_pointer->getDataPtr()->row_lower_[i],benders_tree_node_pointer->getDataPtr()->row_upper_[i]);
            benders_tree_node_pointer->getDataPtr()->solve_ray_right_hand_side_ = false;
            PrintSubproblem(osi_solver,benders_tree_node_pointer->scenario(),"point");
            SolveSubproblem(osi_solver,benders_tree_node_pointer);
            // What about store data?
            // Try to store this solution
            //StoreData(osi_solver,benders_tree_node_pointer);
            LOG(INFO) << "Debug Breakpoint";
        }

        if ( current_stage_ == 0){
            solution_times_.Stop(0);
        }
#ifdef COMPUTING_TIMER
        if ( current_stage_ == 1){
            double elapsed_solution_time = CoinCpuTime()-current_time;
            std::unique_lock<std::mutex> lock_timer(debug_mutex_);
            detailed_timer_.AddTime(3,elapsed_build_time);
            solution_times_.AddTime(3,elapsed_solution_time);
        }
#endif // COMPUTING_TIMER

        // We only need to store data if we have a feasible solution for the subproblem.
        if ( !infeasible_problems_[benders_tree_node_pointer->scenario()]){
            StoreData(osi_solver,benders_tree_node_pointer);
            // Check for degeneracy and stuff, if we like it..
            if ( !forward_ || (store_primal_and_dual_ && current_stage_ > 0))
                GenerateOptimalityCut(benders_tree_node_pointer);
            if ( !benders_tree_node_pointer->getDataPtr()->unbounded_ ){ // Only if problem was not bounded by special row, we want to call these methods and only if we have true LPs, no MIPS
                if ( adaptive_ && (forward_ && current_stage_ < num_stages_-1 || current_stage_ == 0 ) ) // Analyse optimality cuts of this node!
                    AnalyseOptimalityCuts(osi_solver,benders_tree_node_pointer);
                if ( store_cut_information_ && (forward_ && current_stage_ < num_stages_-1 || current_stage_ == 0 ))
                    IntraIterationCutAggregation(osi_solver,benders_tree_node_pointer);
            }
            if ( level_decomp_ && current_stage_ == 0 ){
                BuildAndSolveLevelProblem(osi_solver,benders_tree_node_pointer);
            }
        }
        DeleteSubproblem(osi_solver,benders_tree_node_pointer);

    }

    void ParallelNestedBenders::SolveSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer  ){
        // Warm Start Handling
        // Depending on Warm Start behaviour, use the corresponding entry
        // If WarmstartForScenario or WarmstartForStage do the following
        //PrintSubproblem(osi_solver,benders_tree_node_pointer->scenario());

        SetWarmStart(osi_solver,benders_tree_node_pointer);

                //double time = CoinCpuTime();
        if ( regularization_ && current_stage_ == 0)
            BuildAndSolveRegularizedProblem(osi_solver,benders_tree_node_pointer);
        else {
            SolveSolverInterface(osi_solver);
        }
        //LOG(INFO) << "Solution time: " << CoinCpuTime()-time;

        // Result/Error Handling: Introduce a global variable, when feasibility problems occurs..
        // Check for errors
        if ( !osi_solver->isProvenOptimal() ){
            // We need to check for errors in this case
            if ( osi_solver->isAbandoned() ){
                //Problem
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
                if ( cpx_ptr ){
                    // Get ENV and LP ptr
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                    if ( CPXgetstat(env_ptr,lp_ptr) == 0 ){
                        // Try a resolve
                        CPXdualopt(env_ptr,lp_ptr);
                    }
                    if ( CPXgetstat(env_ptr,lp_ptr) == 0){
                        console_mutex_.lock();
                        LOG(FATAL) << "Solution process abandoned. Cplex returned with status code " << CPXgetstat(env_ptr,lp_ptr);
                        CPXwriteprob(env_ptr,lp_ptr,"abandonedProblem","lp");
                        console_mutex_.unlock();
                    }
                }
            }
            else if ( osi_solver->isProvenDualInfeasible() ){
                // Primal problem is unbounded. => dual infeasible
                // If we are in a problem with theta variables, we might have success in restricting the theta variables
                if ( current_stage_ < num_stages_-1){
                    // Put bound on theta variable via row, column bounds do not cut it ( do not know why )
                    // Get a primal ray! Store the ray! Look at ray!
                    //if ( integer_problems_[current_stage_] ){
                    //    // Solve LP problem, but we need to switch it back to MIP afterwards!
                    //    osi_solver->initialSolve();
                    //    assert( osi_solver->isProvenDualInfeasible());
                    //}
                    //benders_tree_node_pointer->getDataPtr()->unbounded_extreme_ray_.reset(osi_solver->getPrimalRays(1)[0]);

                    //// Normalize values
                    //for (int counter = 0; counter < stage_data_[current_stage_]->num_columns_; counter++){
                    //    if ( CompareWithZero(benders_tree_node_pointer->getDataPtr()->unbounded_extreme_ray_[counter]) )
                    //        benders_tree_node_pointer->getDataPtr()->unbounded_extreme_ray_[counter] = 0.0;
                    //    benders_tree_node_pointer->getDataPtr()->unbounded_extreme_ray_[counter] *= -1.0;
                    //}

                    int start_theta = stage_data_[current_stage_]->num_columns_;
                    bool unbounded_theta = true;
                    //// Find out if the problem is unbounded in the direction of the theta variable.
                    //for (int count_theta = start_theta; count_theta < start_theta+benders_tree_node_pointer->getDataPtr()->num_thetas_;count_theta++){
                    //    if ( benders_tree_node_pointer->getDataPtr()->unbounded_extreme_ray_[count_theta] != 0.0 ){
                    //        unbounded_theta = true; break;
                    //    }
                    //}
                    benders_tree_node_pointer->getDataPtr()->unbounded_ = true;
                    int num_thetas = benders_tree_node_pointer->getDataPtr()->num_thetas_;
                    if ( num_thetas && unbounded_theta){
                        // TODO Inspect the unbounded extreme ray to see if we need this extra row!
                        // 
                        boost::scoped_array<int> extra_indices(new int[num_thetas]);
                        boost::scoped_array<double> extra_elements(new double[num_thetas]);
                        for (int i = 0; i < num_thetas; i++){
                            extra_indices[i] = stage_data_[current_stage_]->num_columns_+i;
                            extra_elements[i] = 1;
                        }
                        // we bound all thetas with one extra row?
                        // We could also bound every theta itself ( or set variable bounds do not work right now.. ) 
                        osi_solver->addRow(num_thetas,extra_indices.get(),extra_elements.get(),-10000000000000, infinity_); // We assume that will work. I mean: Half infinity? 
                        PrintSubproblem(osi_solver,benders_tree_node_pointer->scenario(),"adjusted");
                        // We need to remove this row again, when we finished everything with this problem!
                        benders_tree_node_pointer->getDataPtr()->unbounded_row_ = osi_solver->getNumRows()-1;
                        // Switch preprocessing and advanced start off
                        OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
                        if ( osi_cpx != nullptr ){
                            CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
                            //CPXsetintparam(env_ptr,CPX_PARAM_ADVIND,CPX_OFF);
                            CPXsetintparam(env_ptr,CPX_PARAM_PREIND,CPX_OFF);
                            SolveSolverInterface(osi_solver);
                            //CPXsetintparam(env_ptr,CPX_PARAM_ADVIND,CPX_ON);
                            CPXsetintparam(env_ptr,CPX_PARAM_PREIND,CPX_ON);
                        }
                        else 
                            SolveSolverInterface(osi_solver);
                        // Depending on the solution to this
                    }
                    else {
                        // We can not handle this correctly for now
                        LOG(FATAL) << "Your problem got unbounded.";
                    }

                    if ( osi_solver->isProvenDualInfeasible()){
                        if ( forward_ ){
                            std::vector<SmiTreeNode<BendersNodeData*>*>* children = benders_tree_node_pointer->getChildren();
                            for ( std::vector<SmiTreeNode<BendersNodeData*>*>::iterator it = children->begin(); it != children->end(); ++it){
                                (*it)->getDataPtr()->unbounded_problem_ = true;
                            }
                        }
                    }
                }
                // TODO: This section should not be necessary anyway..
                //else if ( !osi_solver->isProvenOptimal()){
                //    if ( osi_solver->isProvenDualInfeasible() ){
                //        if ( seperate_first_stage_solver_ && current_stage_ == 0) // Strange behaviour may results if we use the seperate_first_stage_solver!
                //            seperate_first_stage_solver_ = false;
                //        console_mutex_.lock();
                //        LOG(ERROR)<< "Problem for scenario " << benders_tree_node_pointer->scenario() << " in stage " << current_stage_ << " is primal unbounded.";
                //        LOG(ERROR) << "This can point to an error in your model/data. This may also happen, when your original problem was meant as an maximization problem. Switching off seperate first stage solver.";
                //        SolveSolverInterface(osi_solver);
                //        if ( osi_solver->isProvenDualInfeasible() ){
                //            // We can not work with that in a reasonable manner..
                //            upper_bound_ = infinity_;
                //            unbounded_problem_ = true; 
                //            osi_solver->writeLp("unboundedProblem");
                //            osi_solver->writeMps("unboundedProblem");
                //        }
                //        console_mutex_.unlock();

                //    }
                //}
            }
            else if ( osi_solver->isProvenPrimalInfeasible() ){
                // Dual problem is unbounded: Generate feasibility cut if we are not in the first stage
                infeasible_problem_mutex_.lock();
                if ( only_one_feasibility_cut_ && infeasible_problems_.any() ){ 
                    infeasible_problems_[benders_tree_node_pointer->scenario()] = true;
                    infeasible_problem_mutex_.unlock();
                    return;
                }
                infeasible_problems_[benders_tree_node_pointer->scenario()] = true;
                infeasible_problem_mutex_.unlock();
                if ( current_stage_ > 0 ){
                    //LOG(INFO) << "Generate feasibility cut for scenario " << benders_tree_node_pointer->scenario() << " in stage " << current_stage_;
                    GenerateFeasibilityCut( osi_solver, benders_tree_node_pointer ); 
                }
                else{
                    // Introduce a debug switch asap for problem output.. 
                    osi_solver->writeLp("master_infeasible");
                    OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
                    if ( cpx_ptr ){
                        // Get ENV and LP ptr
                        CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                        CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                        CPXwriteprob(env_ptr,lp_ptr,"master_infeasible_cplex.lp","lp");

                    }
                    LOG(ERROR) << "Your master problem is infeasible.";
                }

            }
            else if ( !osi_solver->isProvenOptimal() ){
                // Get cplex status
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    // Get ENV and LP ptr
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                    int stat = CPXgetstat(env_ptr,lp_ptr);
                    LOG_IF(ERROR, !(stat == CPXMIP_OPTIMAL || stat == CPXMIP_OPTIMAL_TOL)) << "Cplex returned solution process with " << stat;
                    CPXwriteprob(env_ptr,lp_ptr,"not_optimal","lp");
                }
            }
        }// if not provenOptimal end
        else { // Proven Optimal: 
            StoreWarmStart(osi_solver,benders_tree_node_pointer);
        }
    }

    void ParallelNestedBenders::StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer ){
        // Store Column Solution
        // We need to copy the solution, as we erase the osiSolverInterface
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();

        // Copy theta variables
        int num_theta = osi_solver->getNumCols()-stage_data_[current_stage_]->num_columns_;
        assert(num_theta >= 0);

        // Store values for next stage approximation and column solution 
        // Store old_lower_bound solution too
        if ( d_square_ && current_stage_ == 0 ){

            old_first_stage_solution_.resize(stage_data_[current_stage_]->num_columns_+num_theta,0.0);

            if ( !benders_node_pointer->col_solution_.empty() )
                memcpy(&old_first_stage_solution_[0],&benders_node_pointer->col_solution_[0],sizeof(double)*(stage_data_[current_stage_]->num_columns_));
            if ( !benders_node_pointer->next_stage_approximation_.empty())
                memcpy(&old_first_stage_solution_[0]+stage_data_[current_stage_]->num_columns_,&benders_node_pointer->next_stage_approximation_[0],sizeof(double)*(num_theta));
        }

        // Retrieve and store the theta variables
        if ( num_theta ){
            if ( (regularization_ || level_decomp_ && level_quadratic_new_variables_)&& current_stage_ == 0){
                num_theta -= stage_data_[current_stage_]->num_columns_;
            }
            else if ( level_decomp_ && level_manhattan_ ){
                num_theta -= stage_data_[current_stage_]->num_columns_;
            }
            else if ( level_decomp_ && level_infinity_ ){
                num_theta -= 1;
            }
            assert(num_theta >= 0);
            StoreValuesFromSolverInVector(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,num_theta);
            if (  benders_node_pointer->unbounded_ && num_theta > 0){
                for (int i = 0; i < num_theta;i++)
                    benders_node_pointer->next_stage_approximation_[i] = -infinity_;
            }
            if ( benders_node_pointer->optimality_cuts_.empty() )
                num_theta = 0;
        }


        // We need the column solution for the next stage problem!
        // We want to differentiate between column solution and theta variables 
        StoreValuesFromSolverInVector(benders_node_pointer->col_solution_,osi_solver->getColSolution(),stage_data_[current_stage_]->num_columns_);

        //// If we have integer variables, we might want to set these variables to pure integer values! (because of tolerances)
        if ( integer_problems_[current_stage_] ){
            // Get integer cols
            const std::vector<int>& integer_cols = core_->getIntCols(current_stage_);
            for (int i = 0; i < integer_cols.size();i++){
                benders_node_pointer->col_solution_[integer_cols[i]] = boost::math::round(benders_node_pointer->col_solution_[integer_cols[i]]); 
            }
        }

        if ( d_square_ ){
            StoreValuesFromSolverInArray(benders_node_pointer->solver_rhs_,osi_solver->getRightHandSide(),osi_solver->getNumRows());
            // Store Column Solution multiplied by probability in the expected_y_values in stage_data!
            // Check For Integrality
            if ( current_stage_ == 1){
                std::vector<int> binaries = core_->getIntCols(current_stage_);
                std::vector<std::pair<int,double> > fractional_variables;
                int index_for_fractional_max = -1;
                for (int i = 0; i < binaries.size(); i++){
                    if ( !CompareForEquality(benders_node_pointer->col_solution_[binaries[i]],boost::math::round(benders_node_pointer->col_solution_[binaries[i]]),integer_tolerance_) ){
                        double fractional = 0.5-std::abs(benders_node_pointer->col_solution_[binaries[i]]-0.5);
                        fractional_variables.push_back(make_pair(binaries[i],fractional));
                        if (index_for_fractional_max >= 0 && fractional_variables[index_for_fractional_max].second < fractional ){
                            index_for_fractional_max = fractional_variables.size()-1;
                        }
                        else
                            index_for_fractional_max = 0;
                    }
                }
                if ( index_for_fractional_max >= 0){
                    // Found non-integer that should be integer?
                    // Mark as failing!
                    misc_mutex_.lock();
                    stage_data_[current_stage_]->non_integer_scenarios.push_back(benders_tree_node_pointer->scenario());
                    if ( stage_data_[current_stage_]->violated_index_ < 0) //TODO: Beware, magic number
                        stage_data_[current_stage_]->violated_index_ = fractional_variables[index_for_fractional_max].first; // should be index_for_fractional_max
                    misc_mutex_.unlock();
                }
            }
        }

        double solution_value = 0; // Compute the value of the c*x solution without all thetas!
        const double* obj_coefs = osi_solver->getObjCoefficients();
        for (int i = 0; i < stage_data_[current_stage_]->num_columns_;i++){
            solution_value += benders_node_pointer->col_solution_[i]*obj_coefs[i];
        }
        benders_node_pointer->solution_value_ = solution_value;

        if ( oda_method_ && current_stage_ == 0)
            oda_first_stage_ = solution_value;

        if ( num_theta ){
            benders_node_pointer->lower_bound_ = solution_value;
            for ( double theta_value : benders_node_pointer->next_stage_approximation_){ // Add all thetas to the c*x solution to get lower bound for node at this stage
                benders_node_pointer->lower_bound_ += theta_value;
            }
            if ( d_square_ )
                lower_bound_ -= d_square_translation_term_;
        }

        if ( current_stage_ == 0){ // This works in the multithreaded cases, as we have only one thread for the first stage problem{
            // We are again at stage 0, test for new upper bound!
            old_lower_bound_ = lower_bound_; // Set old lower bound to last lower bound. If we added a feasibility cut lately, we have to take special care.
            if ( !warm_up_ )
                lower_bound_ = benders_node_pointer->lower_bound_;
            if (num_theta == 0 || benders_node_pointer->unbounded_){// We have no thetas yet or whole problem is unbounded!
                lower_bound_ = -infinity_;
                old_lower_bound_ = -infinity_;
            }
        }

        // Store dual solutions
        if ( !forward_ || store_primal_and_dual_ ){
            // Store Dual Solution ( only if we move backward ) ( or we need it anyway )
            if ( integer_problems_[current_stage_] ){// Dual solution for MIP-first stage makes no sense to us
                if ( store_primal_and_dual_ == true && current_stage_ > 0){
                    // Compute dual information from solution we got!
                    std::vector<double> dual(osi_solver->getNumRows());
                    std::vector<double> reduced(osi_solver->getNumCols());
                    ComputeDualSolutionForMipProblem(osi_solver,&dual[0],&reduced[0],&benders_node_pointer->col_solution_[0]);
                    StoreValuesFromSolverInVector(benders_node_pointer->dual_solution_,&dual[0],osi_solver->getNumRows());
                    StoreValuesFromSolverInVector(benders_node_pointer->reduced_cost_,&reduced[0],osi_solver->getNumCols());
                }
            }
            else{ // In case of non-integer problems
                StoreValuesFromSolverInVector(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),osi_solver->getNumRows());
                if ( current_stage_ > 0 ) {
                    // Compute the values: \lambda * lower_bound - \mu * upper_bound
                    // Compute the resulting values right away, as we have the osi_solver_interface at hand.
                    if ( stage_data_[current_stage_]->non_zero_bounds_ ){
                        // Store Reduced Cost ( needed for dual values of primal variable bounds )
                        StoreValuesFromSolverInVector(benders_node_pointer->reduced_cost_,osi_solver->getReducedCost(),osi_solver->getNumCols());
                    }
                }
            }

            if ( (stage_data_[current_stage_]->non_zero_bounds_ || integer_problems_[current_stage_] ) && !benders_node_pointer->reduced_cost_.empty()){
                std::vector<double>& dual_values_variable_bounds = benders_node_pointer->reduced_cost_;
                const double *  variable_lower_bounds = integer_problems_[current_stage_] ? &benders_node_pointer->col_solution_[0] : osi_solver->getColLower();
                const double *  variable_upper_bounds = integer_problems_[current_stage_] ? &benders_node_pointer->col_solution_[0] : osi_solver->getColUpper();
                double computed_dual_value_for_variable_bounds = 0;
               std::vector<double>& column_solution = benders_node_pointer->col_solution_;
                assert(!column_solution.empty());
                for (int i = 0; i< stage_data_[current_stage_]->num_columns_; i++){
                    // Look at variable value and their bounds
                    if ( !CompareWithZero(column_solution[i]) && !CompareWithZero(dual_values_variable_bounds[i]) ){ // If variable is zero, it is either at one of its bounds ( then we do not want to worry ) or it is basic and we also do not have to worry..

                        // We have to look at bounds and multiply with correct one. Reduced Costs = Dual logical variables
                        if ( CompareForEquality( column_solution[i], variable_lower_bounds[i],zero_tolerance_) )
                            computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_lower_bounds[i];
                        else if ( CompareForEquality( column_solution[i], variable_upper_bounds[i],zero_tolerance_))
                            computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_upper_bounds[i]; 
                    }
                }
                benders_node_pointer->dual_value_variable_bounds_ = computed_dual_value_for_variable_bounds;
            }
        }
    }

    void ParallelNestedBenders::BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // TODO: Store not only rhs_upper and rhs_lower but also row sense, so we can call OsiCpx chgrhs instead of setRowSetBounds (Minor performance TODO)
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        SmiNodeData* node_pointer = benders_node_pointer->smi_node_data_; // Stochastic node pointer
        SmiNodeData* deterministic_node_pointer = core_->getNode(current_stage_); // Deterministic node (there is one per stage)
        bool reuse_solver = true;
        // If we do not want to reuse the solver, we have to compute the matrix // Other condition for not reusing: || osi_solver->getNumCols() != stage_pointer->num_columns_+ benders_node_pointer->num_thetas_
        if ( node_pointer->hasMatrix() || osi_solver->getNumCols() == 0  || !reuse_solver ){ // We have to load the matrix / change the matrix => load the problem
            reuse_solver = false;
            // See if we really have a w_matrix here!! TODO
            // We get the deterministic matrix first ( T and W together ).
            // We only need the W part, as h-Tx is the RHS.
            benders_node_pointer->matrix_ = deterministic_node_pointer->generateCoinPackedWMatrix();
            if ( node_pointer->hasMatrix()){
                RowOrderedSparseMatrixPointer stochastic_matrix(CreateRowOrderedSparseMatrixPointer( node_pointer ));
                // We only need to look at the W entries
                for (int row = 0; row < stage_pointer->num_rows_; row++){
                    for (int column_index = stochastic_matrix.start_[row]; column_index < stochastic_matrix.start_[row+1]; column_index++){
                        if ( stochastic_matrix.indices_[column_index] >= core_->getColStart(current_stage_) )
                            benders_node_pointer->matrix_->modifyCoefficient(row,stochastic_matrix.indices_[column_index]-core_->getColStart(current_stage_),stochastic_matrix.elements_[column_index],true);
                    }
                }
            }
        }
        else if ( stage_pointer->stochastic_matrix_w_ && !node_pointer->hasMatrix() ){
            // We still have to reload the matrix as it might have be changed in the past, and this is not what we want..
            reuse_solver = false;
            benders_node_pointer->matrix_ = deterministic_node_pointer->generateCoinPackedWMatrix();
        }

        // Do a quick matrix check, only if we do not reuse the solver. Necessary for problems that do not have a w_matrix!
        if ( !reuse_solver &&benders_node_pointer->matrix_->getMinorDim() == 0){
            benders_node_pointer->matrix_->reverseOrdering(); // We have an empty matrix, it is col ordered, so we need to reverse that ordering!
            benders_node_pointer->matrix_->setDimensions(0,stage_pointer->num_columns_);
            //// Insert at least empty entries for our columns (why is this necessary?)
            //for (int i = 0; i < stage_pointer->num_columns_; i++)
            //    benders_node_pointer->matrix_->modifyCoefficient(0,i,0,true);
        }

        if (  node_pointer->getColLowerLength()) {
            benders_node_pointer->column_lower_ = new double[stage_pointer->num_columns_];
            memcpy(benders_node_pointer->column_lower_, core_->getDenseColLower(current_stage_), stage_pointer->num_columns_*sizeof(double));
            if ( stage_pointer->stochastic_bounds_ )
                for (int i = 0; i < node_pointer->getColLowerLength(); i++) {
                    benders_node_pointer->column_lower_[node_pointer->getColLowerIndices()[i] - core_->getColStart(current_stage_)] = node_pointer->getColLowerElements()[i];
                }
        }
        else 
            benders_node_pointer->column_lower_ = const_cast<double*> (core_->getDenseColLower(current_stage_));

        // Column upper bounds
        if ( node_pointer->getColUpperLength()) {
            benders_node_pointer->column_upper_ = new double[stage_pointer->num_columns_];
            memcpy(benders_node_pointer->column_upper_, core_->getDenseColUpper(current_stage_), stage_pointer->num_columns_*sizeof(double));
            if ( stage_pointer->stochastic_bounds_ )
                for (int i = 0; i < node_pointer->getColUpperLength(); i++) {
                    benders_node_pointer->column_upper_[node_pointer->getColUpperIndices()[i] - core_->getColStart(current_stage_)] = node_pointer->getColUpperElements()[i];
                }
        }
        else
            benders_node_pointer->column_upper_ = const_cast<double*> (core_->getDenseColUpper(current_stage_));

        // If we have a fixed_solution we change the bounds to the fixed solution!
        if ( current_stage_ == 0 && fixed_first_stage_ ){
            // Delete the old ones, otherwise: memory leak
            if ( node_pointer->getColLowerLength())
                delete [] benders_node_pointer->column_lower_;
            if ( node_pointer->getColUpperLength())
                delete [] benders_node_pointer->column_upper_;
            benders_node_pointer->column_upper_ = fixed_solution_.get();
            benders_node_pointer->column_lower_ = fixed_solution_.get();
        }

        // Objective function coefficients
        if ( node_pointer->getObjectiveLength()) {
            benders_node_pointer->objective_ = new double[stage_pointer->num_columns_];
            memcpy(benders_node_pointer->objective_, core_->getDenseObjCoefficients(current_stage_), stage_pointer->num_columns_*sizeof(double));
            if ( stage_pointer->stochastic_objective_ )
                for (int i = 0; i < node_pointer->getObjectiveLength(); i++) {
                    benders_node_pointer->objective_[node_pointer->getObjectiveIndices()[i] - core_->getColStart(current_stage_)] = node_pointer->getObjectiveElements()[i];
                }
        }
        else
            benders_node_pointer->objective_ = const_cast<double*> (core_->getDenseObjCoefficients(current_stage_));

        // Build RHS via row lower and row upper. We have to add T-matrix with previous stage solutions
        int num_rows = stage_pointer->num_rows_;
        int number_of_allocated_rows = num_rows;

        // If we have an unbounded problem in the previous stage ( and we really would like to solve this problem ) we have to generate an unbounded problem right hand side!
        if ( num_rows > 0 ){
            if ( d_square_ && current_stage_ == 1 && !stage_pointer->solve_binary_subproblems_)
                number_of_allocated_rows += stage_pointer->num_d_square_cuts_;
            benders_node_pointer->row_lower_ = new double[number_of_allocated_rows];
            benders_node_pointer->row_upper_ = new double[number_of_allocated_rows];
            if ( benders_node_pointer->unbounded_problem_ ){ // This is wrong, because we have equality rows instead of inequality rows?
                benders_node_pointer->unbounded_right_hand_side_.reset(new double[number_of_allocated_rows]);
                CoinFillN(benders_node_pointer->unbounded_right_hand_side_.get(),number_of_allocated_rows,0.0);
                benders_tree_node_pointer->getDataPtr()->solve_ray_right_hand_side_ = true; // We solve this problem first!
            }
            memcpy(benders_node_pointer->row_lower_, core_->getDenseRowLower(current_stage_), num_rows*sizeof(double));
            if ( stage_pointer->stochastic_rhs_ ){
                for (int i = 0; i < node_pointer->getRowLowerLength(); i++) {
                    assert(node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_) >= 0);
                    assert(node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_) < num_rows);
                    benders_node_pointer->row_lower_[node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowLowerElements()[i];
                }
            }
            memcpy(benders_node_pointer->row_upper_, core_->getDenseRowUpper(current_stage_), num_rows*sizeof(double));
            if  ( stage_pointer->stochastic_rhs_){
                for (int i = 0; i < node_pointer->getRowUpperLength(); i++) {
                    benders_node_pointer->row_upper_[node_pointer->getRowUpperIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowUpperElements()[i];
                }
            }

        }

        // Add solution from previous problem, i.e. form right hand side of the problem
        if ( current_stage_ > 0 ){
            // Get solution from predecessor in tree
            double* unbounded_rhs = nullptr;
            double * column_solution = benders_tree_node_pointer->getParent()->getDataPtr()->col_solution_.empty() ? nullptr : &benders_tree_node_pointer->getParent()->getDataPtr()->col_solution_[0]; // Is of length num_columns of previous stage
            if ( warm_up_ ){ // We want the column solution of the warm_up_ solution! This is the warm_up_solution_ with an offset according the number of cols until current_stage-2.
                int offset_warmup = 0;
                for (int i = 0; i < current_stage_-1; i++)
                    offset_warmup += stage_data_[i]->num_columns_;
                column_solution = warm_up_solution_.get()+offset_warmup;
            }
            if ( benders_node_pointer->unbounded_problem_ )
                unbounded_rhs = benders_tree_node_pointer->getParent()->getDataPtr()->unbounded_extreme_ray_.get();

            if ( column_solution != nullptr){
                // Add T-matrix together with stochastic entries to the problem if there are any
                // We need the T-matrix from the previous stage asap
                RowOrderedSparseMatrixPointer stochastic_matrix(CreateRowOrderedSparseMatrixPointer( node_pointer ));
                RowOrderedSparseMatrixPointer deterministic_matrix(CreateRowOrderedSparseMatrixPointer( deterministic_node_pointer ));
                // Combine these two matrices into one ( see Benders.cpp for example ).
                int current_stage_columns = core_->getColStart(current_stage_);
                int previous_stage_columns = core_->getColStart(current_stage_-1);
                int stochastic_column_index = 0;
                bool stochastic_element_in_row = false;
                // Now we need the loop. Do go on.
                for (int row = 0; row < stage_pointer->num_rows_; row++){ // For every row we have to calculate the correct value that has to added to rlo or rup
                    double value = 0;
                    double unbounded_value = 0;
                    if ( node_pointer->hasMatrix() )// We only need stochastic T matrix here
                        stochastic_element_in_row = (stochastic_matrix.start_[row+1]-stochastic_matrix.start_[row]) ? true : false;

                    // We iterate over the row of the deterministic matrix
                    for ( int column_index = deterministic_matrix.start_[row]; column_index < deterministic_matrix.start_[row+1];column_index++){
                        // We have to jump over all W indices
                        while ( node_pointer->hasMatrix() && stochastic_element_in_row  ){ // Jump over all consecutive W column entries in this row (As columns are ordered, we should skip the rest of the matrix anyway // TODO: Ensure this

                            if (  stochastic_column_index < stochastic_matrix.start_[row+1] && stochastic_matrix.indices_[stochastic_column_index] >= current_stage_columns )
                                stochastic_column_index++;
                            else if (stochastic_column_index==stochastic_matrix.start_[row+1]) { // If the number of stochastic column indices > current_stage_columns (aka W-entries) equals length of stochastic matrix entries in this row, this is not a row for the T-matrix
                                stochastic_element_in_row = false;
                                break;
                            }
                            else
                                break;
                        }
                        // Only handle columns of the T-matrix
                        if ( deterministic_matrix.indices_[column_index] < current_stage_columns){

                            // If we have an element in the stochastic matrix, we use it instead
                            if ( stochastic_element_in_row && deterministic_matrix.indices_[column_index] == stochastic_matrix.indices_[stochastic_column_index] ){
                                assert( stochastic_matrix.indices_[stochastic_column_index]-previous_stage_columns < stage_data_[current_stage_-1]->num_columns_ && stochastic_matrix.indices_[stochastic_column_index]-previous_stage_columns >= 0);
                                value += stochastic_matrix.elements_[stochastic_column_index]*column_solution[deterministic_matrix.indices_[column_index]-previous_stage_columns];
                                //if ( unbounded_rhs != nullptr)
                                //    unbounded_value += stochastic_matrix.elements_[stochastic_column_index]*unbounded_rhs[deterministic_matrix.indices_[column_index]-previous_stage_columns];
                                stochastic_column_index++;
                            }

                            else {  // We assume Staircase-Problems. 
                                assert( deterministic_matrix.indices_[column_index]-previous_stage_columns < stage_data_[current_stage_-1]->num_columns_ && deterministic_matrix.indices_[column_index]-previous_stage_columns >= 0 );
                                value += deterministic_matrix.elements_[column_index]*column_solution[deterministic_matrix.indices_[column_index]-previous_stage_columns];
                                //if ( unbounded_rhs != nullptr)
                                //    unbounded_value += deterministic_matrix.elements_[column_index]*unbounded_rhs[deterministic_matrix.indices_[column_index]-previous_stage_columns];
                            }
                        }

                    }
                    if ( node_pointer->hasMatrix())
                        assert( stochastic_column_index == stochastic_matrix.start_[row+1]); // We added all stochastic elements of this row
                    // Substract from rlo or rup or even the solver interface.. depends on what we are doing
                    // TODO: Equality constraints.. 
                    if ( benders_node_pointer->row_lower_[row] <= -infinity_ ){// 
                        if ( benders_node_pointer->row_upper_[row] >= infinity_)
                            assert(false);
                        else
                            benders_node_pointer->row_upper_[row] -= value; // row <= ub
                    }
                    else if ( benders_node_pointer->row_upper_[row] >= infinity_ )
                        benders_node_pointer->row_lower_[row] -= value; // lb <= row
                    else {
                        //if (benders_node_pointer->row_lower_[row] == benders_node_pointer->row_upper_[row])

                        benders_node_pointer->row_lower_[row] -= value; // lb == row == ub, we can not handle lb <= row <= ub with one real inequality ( have to split up to two rows somewhere..
                        benders_node_pointer->row_upper_[row] -= value;
                    }
                    // TODO: reenable if necessary
                    //if ( unbounded_rhs != nullptr ){
                    //    benders_node_pointer->unbounded_right_hand_side_[row] -= unbounded_value;
                    //}
                }
            }
        }

        // If we want to solve the problem without d_square_stuff (as MIP for example, we need another boolean here..)
        if ( d_square_ && current_stage_ == 1 && !reuse_solver && !stage_data_[current_stage_]->solve_binary_subproblems_){
            if ( stage_pointer->d_square_matrix_ != nullptr)
                benders_node_pointer->matrix_->bottomAppendPackedMatrix(*stage_pointer->d_square_matrix_);
        }
        if ( d_square_ && current_stage_ == 1 && stage_pointer->num_d_square_cuts_ > 0 && !stage_data_[current_stage_]->solve_binary_subproblems_){
            // Correct RHS values
            double* t_array = new double[stage_pointer->num_d_square_cuts_];
            assert( benders_node_pointer->d_square_cut_t_matrix_ != nullptr);
            assert( benders_node_pointer->d_square_cut_t_matrix_->getNumRows() == stage_pointer->num_d_square_cuts_);
            benders_node_pointer->d_square_cut_t_matrix_->times(&benders_tree_node_pointer->getParent()->getDataPtr()->col_solution_[0],t_array);
            for (int i = stage_pointer->num_rows_; i < stage_pointer->num_rows_+stage_pointer->num_d_square_cuts_; i++){
                benders_node_pointer->row_upper_[i] = infinity_;

                double d_square_cut_rhs_value = benders_node_pointer->d_square_cut_rhs_[i-stage_pointer->num_rows_]-t_array[i-stage_pointer->num_rows_]; 
                // Compute the T_matrix elements
                benders_node_pointer->row_lower_[i] = d_square_cut_rhs_value;
            }
            delete [] t_array;
        }

        // Load the basic problem
        // We call this method if we just constructed the matrix for the first time
        if ( !reuse_solver ){
            if ( !benders_node_pointer->unbounded_problem_  )
                osi_solver->loadProblem(*benders_node_pointer->matrix_,benders_node_pointer->column_lower_,benders_node_pointer->column_upper_,benders_node_pointer->objective_,benders_node_pointer->row_lower_,benders_node_pointer->row_upper_);
            else
                osi_solver->loadProblem(*benders_node_pointer->matrix_,benders_node_pointer->column_lower_,benders_node_pointer->column_upper_,benders_node_pointer->objective_,benders_node_pointer->unbounded_right_hand_side_.get(),benders_node_pointer->unbounded_right_hand_side_.get());

        }
        else{
            if ( stage_pointer->stochastic_bounds_ ){
                // Update variable bounds
                std::vector<double> col_bounds;
                col_bounds.reserve(stage_pointer->num_columns_*2);
                for ( int i = 0; i < stage_pointer->num_columns_; i++ ){
                    col_bounds.push_back(benders_node_pointer->column_lower_[i]);
                    col_bounds.push_back(benders_node_pointer->column_upper_[i]);
                }
                osi_solver->setColSetBounds(&stage_pointer->column_indices_[0],&stage_pointer->column_indices_[stage_pointer->num_columns_],&col_bounds[0]);
            }
            if ( stage_pointer->stochastic_objective_ ){
                // Update objective
                osi_solver->setObjCoeffSet(&stage_pointer->column_indices_[0],&stage_pointer->column_indices_[stage_pointer->num_columns_],&benders_node_pointer->objective_[0]);
            }
            // Update Row Bounds (RHS) for the basic problem 
            std::vector<double> row_bounds;
            row_bounds.reserve(number_of_allocated_rows*2);
            for ( int i = 0; i < number_of_allocated_rows; i++ ){
                if ( !benders_node_pointer->unbounded_problem_ ){
                    row_bounds.push_back(benders_node_pointer->row_lower_[i]);
                    row_bounds.push_back(benders_node_pointer->row_upper_[i]);
                }
                else {
                    row_bounds.push_back(benders_node_pointer->unbounded_right_hand_side_[i]);
                    row_bounds.push_back(benders_node_pointer->unbounded_right_hand_side_[i]);
                }
            }
            osi_solver->setRowSetBounds(&stage_pointer->row_indices_[0],&stage_pointer->row_indices_[number_of_allocated_rows],&row_bounds[0]);

            // Remove all cuts from previous solutions ( includes "Theta" Constraints )
            int cuts_to_remove = osi_solver->getNumRows()-stage_pointer->num_rows_;
            if ( d_square_ && current_stage_ == 1 && !stage_data_[current_stage_]->solve_binary_subproblems_)
            {
                cuts_to_remove = osi_solver->getNumRows()-stage_pointer->num_rows_-stage_pointer->num_d_square_cuts_;
            }
            if ( cuts_to_remove ){
                std::vector<int> remove_indices;
                remove_indices.reserve(cuts_to_remove);
                for (int i = stage_pointer->num_rows_; i < stage_pointer->num_rows_+cuts_to_remove;i++)
                    remove_indices.push_back(i);
                osi_solver->deleteRows(cuts_to_remove,&remove_indices[0]);
            }
        }

        // We need to set the integer indices, if we have not done so already!
        if ( !reuse_solver && integer_problems_[current_stage_] ){
            std::vector<int> integer_indices = core_->getIntCols(current_stage_);
            osi_solver->setInteger(&integer_indices[0],integer_indices.size()); // Sets integer only, if we actually use osi_solver->branchAndBound() method!!
        }

    }

    ParallelNestedBenders::~ParallelNestedBenders()
    {
        // Solver 
        delete osi_solver_;
        std::pair< boost::thread::id,OsiSolverInterface*> id_solver_pair;
        BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
            delete id_solver_pair.second;
        }
        // Data structures (SmiTree deletes the remaining BendersNodeData elements
        BOOST_FOREACH( BendersStageData* stage_data, stage_data_ ){
            delete stage_data;
        }
        //BOOST_FOREACH // Delete Benders Note Data ?
    }

    void ParallelNestedBenders::GenerateOptimalityCut( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // We need the values for the H and the T matrix.
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        BendersNodeData* ancestor_node_pointer = benders_tree_node_pointer->getParent()->getDataPtr();
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        SmiNodeData* node_pointer = benders_node_pointer->smi_node_data_;

        // Compute probability used for this scenario 
        double prob = benders_node_pointer->probability_/ancestor_node_pointer->probability_;

        // We compute prob* \pi * T + \theta >= prob*\pi*h (+ prob*dual_value_variable_bounds)
        double* row_lower = nullptr; double * row_upper = nullptr;
        if ( node_pointer->getRowLowerLength() ){
            row_lower = new double[stage_pointer->num_rows_];
            memcpy(row_lower, core_->getDenseRowLower(current_stage_), stage_pointer->num_rows_*sizeof(double));
            for (int i = 0; i < node_pointer->getRowLowerLength(); i++) {
                row_lower[node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowLowerElements()[i];
            }
        }
        else 
            row_lower = const_cast<double*> (core_->getDenseRowLower(current_stage_));

        if ( node_pointer->getRowUpperLength() ){

            row_upper = new double[stage_pointer->num_rows_];
            memcpy(row_upper, core_->getDenseRowUpper(current_stage_), stage_pointer->num_rows_*sizeof(double));
            for (int i = 0; i < node_pointer->getRowUpperLength(); i++) {
                row_upper[node_pointer->getRowUpperIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowUpperElements()[i];
            }
        }
        else           
            row_upper = const_cast<double*> (core_->getDenseRowUpper(current_stage_));

        // Store results of the computation 
       std::vector<double> cut_coefficients(stage_data_[current_stage_-1]->num_columns_,0);
       std::vector<double>& dual_solution = benders_node_pointer->dual_solution_;
       assert( !dual_solution.empty() );

        ComputeCutCoefficients(benders_tree_node_pointer,dual_solution,&cut_coefficients);

        BOOST_FOREACH(double& value, cut_coefficients){
            value *= prob;
        }

        // Now we generate the RHS value of the cut
        // To calculate RHS, use constant RHS/LHS; if the 
        // corresponding dual variable has a negative value, use
        // RHS; if the corresponding dual variable has a positive
        // value, use LHS;
        double cut_rhs = 0;
        int for_bound = getNumRowsProblem();
        int row = 0;
        for ( ; row < for_bound; row++){
            if ( dual_solution[row] < 0 )
                cut_rhs += dual_solution[row]*row_upper[row];
            else if ( dual_solution[row] > 0)
                cut_rhs += dual_solution[row]*row_lower[row];
        }
        // Skip the values of the bounds_in_matrix, if we have them!
        if ( bounds_in_matrix_ ){ // TODO: Redo, if needed
            for (std::vector<OsiRowCut*>::iterator it = benders_node_pointer->variable_bounds_.begin(); it != benders_node_pointer->variable_bounds_.end(); ++it){
                // Depending on the value of the dual solution, we need the lower or the upper bound!
                if ( dual_solution[row] < 0){ // Dual value is negative => corresponds to 
                    cut_rhs += dual_solution[row]*(*it)->ub();
                }
                else if ( dual_solution[row] > 0) {
                    cut_rhs += dual_solution[row]*(*it)->lb();
                }
                row++;
                for_bound++;
            }
        }

        // Append values for optimality and feasibility cuts.. ( if we have any at this node )
        for_bound += benders_node_pointer->feasibility_cuts_.size();
        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->feasibility_cuts_.begin(); it != benders_node_pointer->feasibility_cuts_.end(); ++it){
            cut_rhs += (*it)->cut_->lb()*dual_solution[row];
            row++;
        }
        assert( row == for_bound);
        for_bound += benders_node_pointer->optimality_cuts_.size();
        int not_used = 0;
        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
            if ( (*it)->to_use_ ){
                cut_rhs += (*it)->cut_->lb()*dual_solution[row];
                row++;
            }
            else
                not_used++;
        }
        assert( row+not_used == for_bound);
        if ( benders_node_pointer->unbounded_ ){
            // We MUST add the unbounded row value to the RHS, otherwise we probably restrict theta values too much (lower bound gets much higher than allowed)!
            // We must assert that the row index points to the unbounded row!
            assert( row == benders_node_pointer->unbounded_row_);
            cut_rhs += -10000000000000*dual_solution[row];
        }

        if ( d_square_ ){
            assert( row == stage_pointer->num_rows_);
            cut_rhs += d_square_translation_term_;
            if ( !stage_data_[current_stage_]->solve_binary_subproblems_ )
                for ( int i = 0; i < stage_pointer->num_d_square_cuts_;i++)
                    cut_rhs += benders_node_pointer->d_square_cut_rhs_[i]*dual_solution[i+row];
        }


        // Append value for dual variable bounds
        cut_rhs += benders_node_pointer->dual_value_variable_bounds_;
        // reset dual_value_variable_bounds value
        benders_node_pointer->dual_value_variable_bounds_ = 0;

        // Multiply with probability
        cut_rhs *= prob;

        if ( CompareWithZero(cut_rhs))
            cut_rhs = 0.0;

        // Store computed values in BendersNodeData
        benders_node_pointer->current_cut_coefficients_ = std::move(cut_coefficients);
        benders_node_pointer->current_cut_rhs_ = cut_rhs;

        if ( oda_method_ ){ 
            // We store a multi-cut type optimality cut 
            std::vector<double> cut_elements;
            std::vector<int> cut_indices;
            // Make copying less frequent!
            cut_elements.reserve(stage_data_[current_stage_-1]->num_columns_+1);
            cut_indices.reserve(stage_data_[current_stage_-1]->num_columns_+1);
            int new_index = 0;
            for (int index = 0; index < stage_data_[current_stage_-1]->num_columns_; index++){
                if ( CompareWithZero(cut_coefficients[index]) ){
                    // Set cut element to zero, if near enough to zero, otherwise numerical difficulties arise by not adding it to the vector!
                }
                else {
                    cut_elements.push_back( cut_coefficients[index]);
                    cut_indices.push_back( index );
                    new_index++;
                }
            }
            // Add theta variable index!
            cut_elements.push_back(1.0);
            cut_indices.push_back(stage_data_[current_stage_-1]->num_columns_);
            new_index++;

            std::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
            cut_pointer->setLb(cut_rhs);
            cut_pointer->setUb(infinity_);
            cut_pointer->setRow(new_index,&cut_indices[0],&cut_elements[0],debug_test_duplicate_indices_);
            // Store OsiRowCut in a list
            benders_node_pointer->oda_optimality_cuts_.push_back(cut_pointer);
        }

        // Cleanup
        if ( node_pointer->getRowLowerLength() )
            delete [] row_lower; 
        if (node_pointer->getRowUpperLength())
            delete [] row_upper;



        // We also remove the dual values, because we do not need them anymore.
        if ( !store_primal_and_dual_ ){
            vector<double>().swap(benders_node_pointer->dual_solution_); // Clear and reallocate
        }

    }

    void ParallelNestedBenders::AggregateOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer ){
        assert(!forward_);
        if ( infeasible_problems_.any() && !generate_feasibility_and_optimality_cuts_){
            console_mutex_.lock();
            LOG(WARNING) << "Attempting to generate optimality cuts in the presence of infeasible problems in this stage.";
            console_mutex_.unlock();
            return;
        }
        if ( generate_feasibility_and_optimality_cuts_ && infeasible_problems_.any() ){
            // This is not yet implemented TOOD: we do not call this method, if we have infeasibly problems
            assert(false);
        }

        std::vector<double>& column_solution = benders_tree_node_pointer->getDataPtr()->col_solution_;
        if ( column_solution.empty() ){
            // We do not add any cuts, we have no prior column solution. This is an error.
            if ( warm_up_ ){
                int offset = 0;
                for (int i = 0; i < current_stage_-1;i++)
                    offset += stage_data_[i]->num_columns_;
                //std::copy(warm_up_solution_.get()+offset,warm_up_solution_.get()+offset+stage_data_[current_stage_-1]->num_columns_,column_solution.begin());
                column_solution.assign(warm_up_solution_.get()+offset,warm_up_solution_.get()+offset+stage_data_[current_stage_-1]->num_columns_);
            }
            else 
                assert(false);

        }
        if ( stage_data_[current_stage_]->solve_binary_subproblems_ ){
            // Return or better: call method that calls laporte-louveaux cut generation routine
            LOG(INFO) << "Not adding new benders style optimality cuts to problems at stage " << current_stage_-1 << " because we solved mip-subproblems.";
            return;
        }

        // We specified the number of thetas in the stage_pointer (for now)
        int num_cuts = stage_data_[current_stage_-1]->num_thetas_;
        int node_num_cuts = benders_tree_node_pointer->getDataPtr()->num_thetas_;
        if ( num_cuts != node_num_cuts ){
            if ( node_num_cuts > 0 ) // Initialized with 0, so in first iteration not reasonable
                num_cuts = node_num_cuts;
        }
        assert( num_cuts > 0 );
        // We need all the children of this node..
        int num_childs = benders_tree_node_pointer->numChildren();
        num_cuts = std::min(num_cuts,num_childs);
        // Write that back to stage_data? No, as number of childs can differ between nodes.
        // Num Subproblem per cuts
        int num_subproblems_per_cut = (int) std::ceil( ((double)num_childs)/num_cuts );
        // If num_cuts is zero, we have a node wise rule..
        if (num_cuts == 0){ // Node wise rule
            // Set num_subproblems_per_cut if we did not set
            num_subproblems_per_cut = std::min(num_childs,benders_tree_node_pointer->getDataPtr()->num_scen_pro_cut_);
            // Compute number of cuts we need for this number of subproblems per cut
            num_cuts = (int)(std::ceil(((double)num_childs)/num_subproblems_per_cut));
            assert(num_cuts >= 1);
        }

        // We have to adopt the number of cuts per subproblem.
        assert(num_subproblems_per_cut >= 1);
        // See if we want to somehow change our cut level aggregation.

        bool change_aggregation_level = benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_;
        if ( num_cuts != benders_tree_node_pointer->getDataPtr()->num_thetas_ ){
            // We change the level of aggregation
            change_aggregation_level = true;
            benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_ = true;
            createRoundRobinAggregation(benders_tree_node_pointer->getDataPtr(),num_childs,num_cuts,num_subproblems_per_cut);
            // We have to transform the old optimality cuts to the new number of theta's we would like to have
            // We already called a function that gives us a new number of whatever it is we need.

            // Transform all old optimality cuts to new ones.
            // Set new values
            benders_tree_node_pointer->getDataPtr()->num_thetas_ = num_cuts;
            benders_tree_node_pointer->getDataPtr()->active_aggregates_ = boost::dynamic_bitset<>(num_cuts); // Gets initialized with 0
            benders_tree_node_pointer->getDataPtr()->was_under_threshold_ = boost::dynamic_bitset<>(num_cuts); // Gets initialized with 0
        }

        int num_columns = stage_data_[current_stage_-1]->num_columns_;
        // Do some magic to find a good partition for the number of cuts we want to produce: TODO

        int added_cuts = 0;
        bool initialized_iterator = !benders_tree_node_pointer->getDataPtr()->optimality_cuts_.empty();


        bool added_iterator = false;
        for ( int i = 0; i < num_cuts; i++){
            // Start a new OsiRowCut
            double cut_lower_bound = 0.0;
            int num_theta_indices = 1;//benders_tree_node_pointer->numChildren(); // or 1. What needs to be changed accordingly is the number in AppendCut method
            std::vector<double> cut_elements(num_columns+num_theta_indices,0);
            for ( int j = num_columns; j < num_columns+num_theta_indices; j++)
                cut_elements[j] = 1;
            std::vector<int> cut_indices;
            cut_indices.reserve(num_columns+num_theta_indices); // Try something new/and shiny
            for (int j = 0; j < num_columns; j++)
                cut_indices.push_back(j);

            if ( num_theta_indices == 1)
                cut_indices.push_back(num_columns+i);
            else {
                for (int j = 0; j < num_theta_indices; j++)
                    cut_indices.push_back(num_columns+j);
            }

            // For all the indices, that belong to this cut.
            std::vector<int>& nodes_in_cut(benders_tree_node_pointer->getDataPtr()->nodes_in_cut_[i]);
            for ( int j = 0; j < nodes_in_cut.size(); j++){
                for (int k = 0; k < num_columns; k++)
                    cut_elements[k] += tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->current_cut_coefficients_[k];
                cut_lower_bound += tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->current_cut_rhs_;
            }
            // Check if cut is not redundant: Conditional Expected Value of Subproblems (theta) is greater than before. For implementation reasons, we only do this if we do not change aggregation level..

            // Theta = cut lower bound - cut_vector * column_solution
            // Compute cut_vector * column_solution
            double sum = 0.0;
            for ( int index = 0;  index < num_columns;  index++ ) { // We iterate over column_solution, but not over the theta index, which is always the last one.
                int column_index = cut_indices[index];
                sum += column_solution[column_index] * cut_elements[index];
            }
            // Add computed cut right hand side
            sum = cut_lower_bound-sum;
            // Check with previous stuff only, if we did not changed the aggregation level. Because if we changed we do not have the data yet to check for validity of new cut
            // If solution value of problem is greater than approximation we have, we need to add a new cut, otherwise we don't, but it does not harm to do so (except solution time..)


            // "Sparsify" the cut
            int new_index = 0;
            // Clear zero elements and create sparse vector for future reference
            for (int index = 0; index < num_columns+num_theta_indices; index++){
                if ( CompareWithZero(cut_elements[index]) ){
                    // Set cut element to zero, if near enough to zero, otherwise numerical difficulties arise
                    cut_elements[index] = 0;
                }
                else {
                    cut_elements[new_index] = cut_elements[index];
                    cut_indices[new_index] = cut_indices[index];
                    new_index++;
                }
            }
            assert( new_index <= num_columns+num_theta_indices);

            // Add cut
            std::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
            if ( CompareWithZero(cut_lower_bound))
                cut_lower_bound = 0.0;
            cut_pointer->setLb(cut_lower_bound);
            cut_pointer->setUb(infinity_);
            cut_pointer->setRow(new_index,&cut_indices[0],&cut_elements[0],debug_test_duplicate_indices_);
            std::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
            cut_stuff->aggregate_number_ = i;
            if ( change_aggregation_level || sum > (benders_tree_node_pointer->getDataPtr()->num_thetas_ ? benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i]-d_square_translation_term_ : 0) ){
                cut_stuff->to_use_ = true;
                added_cuts++;
            }
            else
                cut_stuff->to_use_ = false;

            AddOptimalityCutToContainer(benders_tree_node_pointer,cut_stuff);

            // Set aggregate to active: We have a cut for this aggregate
            benders_tree_node_pointer->getDataPtr()->active_aggregates_[i] = true;

            // Store information about this new cut, we added a new one to our set!
            //benders_tree_node_pointer->getDataPtr()->cut_information_.CreateCut(current_iteration_);
            /************************************************************************/
            /* End If-Condition here                                                */
            /************************************************************************/

        }// End for num_cuts

        add_cut_counter_mutex_.lock();
        added_optimality_cuts_[current_stage_-1] += added_cuts;
        add_cut_counter_mutex_.unlock();

    }

    void ParallelNestedBenders::AddOptimalityCutToContainer( SmiTreeNode<BendersNodeData*> * benders_tree_node_pointer, std::shared_ptr<CutStuff> cut_stuff){
        if ( cut_stuff->to_use_ || adaptive_ || store_cut_information_){ // We will add the cut in the case where it is usable (defined above) or where we might need it later one )
            benders_tree_node_pointer->getDataPtr()->optimality_cuts_.push_back(cut_stuff);
            // Add first added cut to last_iteration start pointer
            if ( seperate_first_stage_solver_ && current_stage_ == 1 && benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ == benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end())
                benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ = --benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end();
        }
    }

    void ParallelNestedBenders::GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    { // Called directly from solve subproblem.
        // We need the dual rays..
        assert(osi_solver->isProvenPrimalInfeasible()); // Otherwise we do not need to call this method
        bool use_cplex = false;
        std::unique_ptr<double[]> dual_solution_farkas = nullptr; // We need to delete this, if we use dual_farkas, else not
        double * dual_solution_primal_variable_bounds = nullptr;
        const double * dual_solution = nullptr;
        int number_rows = osi_solver->getNumRows(); // Also account for optimality and feasibility cuts
        int number_cols_of_basic_problem = stage_data_[current_stage_]->num_columns_; //Ignore next stage approximations

        double computed_dual_value_for_variable_bounds = 0;

        const double * dual_values_variable_bounds = nullptr;
        const double * variable_lower_bounds = nullptr;
        const double * variable_upper_bounds = nullptr;

        bool use_dual_farkas = false;
        if ( feas_cut_lp_method_ == PNB_INT_PRIMAL_SIMPLEX )
            use_dual_farkas = false;
        else
            use_dual_farkas = true;

        bool fischetti_cut_procedure = false;
        if ( fischetti_cut_procedure ){
            // Maybe we would like to change the problem: Fischetti Generation!
            // We must add a new row for this: q y = \le \theta !
            // But we have no theta yet..
            // How does this translate into a constraint I like?!
            std::vector<int> indices(number_rows);
            std::vector<double> elements(number_rows);
            double * row_lower_node = benders_tree_node_pointer->getDataPtr()->row_lower_;
            double * row_upper_node = benders_tree_node_pointer->getDataPtr()->row_upper_;
            for (std::size_t i = 0; i < number_rows;i++){
                indices[i] = i;
                if ( row_lower_node[i] == row_upper_node[i] ){ // This is an equality row, therefore the dual value is free. 
                    // The RHS is equal to the obj function value of the dual. The dual gets maximized. Therefore a RHS >= 0 implies that the value will be increased when it is possible.
                    if ( row_lower_node[i] >= 0 )
                        elements[i] = 0;
                    else 
                        elements[i] = 0;
                }
                if ( row_lower_node[i] <= -infinity_ ) // This is a less-than row, therefore the dual value is <= 0. Set w_i = -1
                    elements[i] = -1;
                else if (row_upper_node[i] >= infinity_ ) // This is a greater-than row, therefore the dual value is >= 0. Set w_i = 1.
                    elements[i] = 1;

            }
            CoinPackedVector cpv(number_rows,&indices[0],&elements[0]);
            osi_solver->addCol(cpv,-infinity_,infinity_,1);
        }
        // We need the currents problem row sense to set the correct values!

        // Now we try to solve the same stuff with dual farkas: Primal problem is infeasible, we use the dual simplex, so it should work fine.
        // See for example https://www-304.ibm.com/support/docview.wss?uid=swg21400058
        OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        if ( cpx_ptr ){
            use_cplex = true; 
            // Get ENV and LP ptr
            CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
            CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
            int cpx_stat = CPXgetstat(env_ptr,lp_ptr);

            dual_solution_farkas.reset(new double[number_rows]);

            if ( true || cpx_stat != CPX_STAT_INFEASIBLE || lp_method_ != feas_cut_lp_method_){// If this is not done everytime, problems can occur with fxm4_16 that say no dual solution present!
                // We lose the warm-start solution this way, so we should add it.
                std::vector<int> cstat(osi_solver->getNumCols());
                std::vector<int> rstat(osi_solver->getNumRows());
                std::vector<double> dnorm(osi_solver->getNumRows());
                bool copied_basednorms = false;
                if ( cpx_stat == CPX_STAT_INFEASIBLE ){ // This is usually the case!
                    // Store basednorms
                    cpx_stat = CPXgetbasednorms(env_ptr,lp_ptr,&cstat[0],&rstat[0],&dnorm[0]);
                    if ( cpx_stat == 0)
                        copied_basednorms = true;
                }
                CPXchgobjsen(env_ptr, lp_ptr, CPX_MAX);
                CPXchgobjsen(env_ptr, lp_ptr, CPX_MIN);
                CPXsetintparam( env_ptr, CPX_PARAM_PREIND, CPX_OFF );
                if ( copied_basednorms )
                    cpx_stat = CPXcopybasednorms(env_ptr,lp_ptr,&cstat[0],&rstat[0],&dnorm[0]);
                if ( stage_data_[current_stage_]->non_zero_bounds_ && !use_dual_farkas)
                    CPXprimopt(env_ptr,lp_ptr); // I need primopt to obtain the reduced costs of the phase 1 problem ( CPXgetdj ) and the dual ray via CPXgetpi
                else
                    CPXdualopt(env_ptr,lp_ptr); // Use CPXdualfarkas for dual ray, but nothing about the reduced costs yet..
                CPXsetintparam( env_ptr, CPX_PARAM_PREIND, CPX_ON );
                cpx_stat = CPXgetstat(env_ptr,lp_ptr);
                if ( cpx_stat == CPX_STAT_INFEASIBLE )
                    StoreWarmStart(osi_solver,benders_tree_node_pointer);
            }

            double proof = 0;
            if ( cpx_stat == CPX_STAT_INFEASIBLE ){
                if ( !use_dual_farkas){
                    cpx_stat = CPXgetpi(env_ptr,lp_ptr,dual_solution_farkas.get(),0,number_rows-1);// If we use the primal method, we get the dual ray via the dual variables
                    if ( cpx_stat != 0){
                        //Routine was unsuccesful, we need to compute the values by the normal method
                        LOG_IF(INFO,cpx_stat) << "CPXgetpi : " << cpx_stat;
                        use_cplex = false;
                    }
                }
                else {
                    cpx_stat = CPXdualfarkas(env_ptr,lp_ptr,dual_solution_farkas.get(),&proof);
                    LOG_IF(INFO,cpx_stat) << "CPXdualfarks : " << cpx_stat;
                    if ( cpx_stat != 0 ){// If CPXdualfarkas returns a value that is not zero, it was unsuccessful, so we need to compute the dual ray the normal way
                        use_cplex = false;
                    }
                }

                if ( use_cplex && stage_data_[current_stage_]->non_zero_bounds_ && use_dual_farkas){ // We need to compute the dual values for the primal variable bounds! Where do we get the values? Link blogpost by OR in an OB World: http://orinanobworld.blogspot.de/2011/07/farkas-certificates-in-cplex.html
                    // q^T = y^T*A 
                    // We can compute this with the column based matrices
                    variable_lower_bounds = benders_tree_node_pointer->getDataPtr()->column_lower_;
                    variable_upper_bounds = benders_tree_node_pointer->getDataPtr()->column_upper_;
                    const CoinPackedMatrix* matrix = nullptr;
                    matrix = osi_solver->getMatrixByCol(); // This matrix contains also feasibility/optimality cuts..

                    dual_solution_primal_variable_bounds = new double[number_cols_of_basic_problem];
                    const double* objective_coefficients = benders_tree_node_pointer->getDataPtr()->objective_;
                    for (int i = 0; i < number_cols_of_basic_problem; i++){
                        dual_solution_primal_variable_bounds[i] = 0;
                        if ( variable_lower_bounds[i] && variable_lower_bounds[i] > -infinity_ || variable_upper_bounds[i] && variable_upper_bounds[i] < infinity_ ){
                            // Get column of the matrix to compute q_j = y^T A_j
                            CoinShallowPackedVector vector = matrix->getVector(i);
                            double result = 0.0;
                            for ( int j = 0; j < vector.getNumElements(); j++){
                                result += (CompareWithZero(dual_solution_farkas[vector.getIndices()[j]]) ? 0: dual_solution_farkas[vector.getIndices()[j]] )* vector.getElements()[j];
                            }
                            // q_j + v_j - w_j = c_j
                            // Therefore: We need to compute the resulting value, which is c_j-q_j. But: Cplex does not care about the RHS itself?
                            result = -result;// If value is negative, we need the positive and vice versa.
                            if ( !CompareWithZero(result) )
                                dual_solution_primal_variable_bounds[i] = result;

                        }
                    }// End For
                }// End If non_zero_bounds
            }//End If infeasible
            else {
                // Print out the stat
                std::unique_lock<std::mutex> u_lock(console_mutex_);
                LOG(INFO) << "Solver returned with status code : " << cpx_stat << " instead infeasible, while solving the original problem again with primal or dual simplex during generation of feasibility cuts.";

                use_cplex = false;
                if ( cpx_stat == CPX_STAT_OPTIMAL ){
                    CPXwriteprob(env_ptr,lp_ptr,"optimal_infeasible_problem.lp","lp");
                    return;
                }

            }

            if ( use_cplex && stage_data_[current_stage_]->non_zero_bounds_ && !use_dual_farkas){ // If we use the primal simplex to obtain the multipliers, we take the reduced costs = dual logicals
                dual_solution_primal_variable_bounds = new double[number_cols_of_basic_problem];
                cpx_stat = CPXgetdj(env_ptr,lp_ptr,dual_solution_primal_variable_bounds,0,number_cols_of_basic_problem-1);
            }

            // Compute the value of the primal variable bounds * dual multipliers
            if ( use_cplex && stage_data_[current_stage_]->non_zero_bounds_ ) {
                variable_lower_bounds = osi_solver->getColLower();
                variable_upper_bounds = osi_solver->getColUpper();
                if ( stage_data_[current_stage_]->non_zero_bounds_)
                    for (int i = 0; i < number_cols_of_basic_problem; i++){
                        if ( variable_lower_bounds[i] && variable_lower_bounds[i] > -infinity_ && dual_solution_primal_variable_bounds[i] > 0 ){ // This means, we have a variable lower bound not zero and the bound is breached below
                            computed_dual_value_for_variable_bounds += dual_solution_primal_variable_bounds[i]*variable_lower_bounds[i];
                        }
                        else if ( variable_upper_bounds[i] && variable_upper_bounds[i] < infinity_ && dual_solution_primal_variable_bounds[i] < 0){ // Variable has non-zero upper bound and the current variable value is above
                            computed_dual_value_for_variable_bounds += dual_solution_primal_variable_bounds[i]*variable_upper_bounds[i];
                        }
                    }
                    delete [] dual_solution_primal_variable_bounds;
                    dual_solution_primal_variable_bounds = nullptr;
            }


        }
        bool cplex_licence_problem = true;
        OsiSolverInterface* dual_ray_problem = nullptr;
        if ( !use_cplex){
            dual_ray_problem = osi_solver;
            GenerateDualRayProblem(dual_ray_problem); // Append columns to the existing problems, to allow feasible solutions.
            if ( abs(dual_ray_problem->getObjValue()) <= zero_tolerance_){
                // cleanup and return
                return;
            }
            number_rows = dual_ray_problem->getNumRows(); // Should have the same number of rows?!
            dual_solution = dual_ray_problem->getRowPrice();
        }

        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        SmiNodeData* node_pointer = benders_tree_node_pointer->getDataPtr()->smi_node_data_;
        BendersStageData* stage_pointer = stage_data_[current_stage_];

        // Compute the cut coefficients for the matrix
        std::vector<double> cut_coefficients(stage_data_[current_stage_-1]->num_columns_,0);

        // Copy solution to enable zero bound check
        std::vector<double> checked_dual_solution; checked_dual_solution.reserve(number_rows);
        if ( use_cplex ){
            StoreValuesFromSolverInVector(checked_dual_solution,dual_solution_farkas.get(),number_rows);
        }
        else {
            StoreValuesFromSolverInVector(checked_dual_solution,dual_solution,number_rows);
        }

        ComputeCutCoefficients(benders_tree_node_pointer,checked_dual_solution,&cut_coefficients);

        // Compute fixed value for RHS

        // Compute the values: \lambda * lower_bound - \mu * upper_bound
        // Compute the resulting values right away, as we have the osi_solver_interface at hand.

        if ( !use_cplex ){ 
            dual_values_variable_bounds = dual_ray_problem->getReducedCost();
            variable_lower_bounds = osi_solver->getColLower();
            variable_upper_bounds = osi_solver->getColUpper();
            computed_dual_value_for_variable_bounds = 0;
            const double* column_solution = dual_ray_problem->getColSolution(); // We need to check if the column solution is at one of the bounds..

            for (int i = 0; i< stage_data_[current_stage_]->num_columns_; i++){
                // Look at variable value and their bounds
                if ( !CompareWithZero(column_solution[i]) && !CompareWithZero(dual_values_variable_bounds[i]) ){ // If variable is zero, it is either at one of its bounds ( then we do not want to worry ) or it is basic and we also do not have to worry..
                    std::cout << "var " << i << " has reduced cost of " << dual_values_variable_bounds[i] << "\n";
                    if ( CompareForEquality( column_solution[i], variable_lower_bounds[i],zero_tolerance_) )
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_lower_bounds[i];
                    else if ( CompareForEquality( column_solution[i], variable_upper_bounds[i],zero_tolerance_))
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_upper_bounds[i]; 
                }
            }
        }
        // Compute the cut coefficients for the RHS.
        // We compute prob* \pi * T + \theta >= prob*\pi*h (+ prob*dual_value_variable_bounds)
        double* row_lower = nullptr; double * row_upper = nullptr;
        if ( !benders_node_pointer->solve_ray_right_hand_side_ ){
            if ( node_pointer->getRowLowerLength() ){
                row_lower = new double[stage_pointer->num_rows_];
                memcpy(row_lower, core_->getDenseRowLower(current_stage_), stage_pointer->num_rows_*sizeof(double));
                for (int i = 0; i < node_pointer->getRowLowerLength(); i++) {
                    row_lower[node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowLowerElements()[i];
                }
            }
            else 
                row_lower = const_cast<double*> (core_->getDenseRowLower(current_stage_));

            if ( node_pointer->getRowUpperLength() ){
                row_upper = new double[stage_pointer->num_rows_];
                memcpy(row_upper, core_->getDenseRowUpper(current_stage_), stage_pointer->num_rows_*sizeof(double));
                for (int i = 0; i < node_pointer->getRowUpperLength(); i++) {
                    row_upper[node_pointer->getRowUpperIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowUpperElements()[i];
                }
            }
            else           
                row_upper = const_cast<double*> (core_->getDenseRowUpper(current_stage_));
        }
        else {
            // If we generate a feasibility cut for the ray problem, we need to take the extreme ray stored in this node!

            row_lower = benders_node_pointer->unbounded_right_hand_side_.get();
            row_upper = benders_node_pointer->unbounded_right_hand_side_.get();
        }


        // Now we generate the RHS value of the cut
        // To calculate RHS, use constant RHS/LHS; if the 
        // corresponding dual variable has a negative value, use
        // RHS; if the corresponding dual variable has a positive
        // value, use LHS;
        double cut_rhs = 0;
        int for_bound = stage_pointer->num_rows_;
        int row = 0;
        for ( ; row < for_bound; row++){
            if ( checked_dual_solution[row] < 0 )
                cut_rhs += checked_dual_solution[row]*row_upper[row];
            else if ( checked_dual_solution[row] > 0)
                cut_rhs += checked_dual_solution[row]*row_lower[row];
        }
        assert( row == for_bound);
        // Append values for bounds? bounds are moved in the matrix.. TODO: Test it
        if ( !benders_node_pointer->variable_bounds_.empty() ){
            for ( int i = 0; i < benders_node_pointer->variable_bounds_.size(); i++){
                if ( checked_dual_solution[row] < 0 )
                    cut_rhs += benders_node_pointer->variable_bounds_[i]->ub()*checked_dual_solution[row];
                else if ( checked_dual_solution[row] > 0)
                    cut_rhs += benders_node_pointer->variable_bounds_[i]->lb()*checked_dual_solution[row];
                row++;
            }
        }
        //// Append values for optimality and feasibility cuts.. ( if we have any at this node )
        // Optimality cuts should not count into the feasibility question
        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->feasibility_cuts_.begin(); it != benders_node_pointer->feasibility_cuts_.end(); ++it){
            assert(checked_dual_solution[row] >= 0);
            cut_rhs += (*it)->cut_->lb()*checked_dual_solution[row];
            row++;
        }

        // We do not need optimality cuts here!

        //for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
        //    if ( (*it)->to_use_ ) {
        //        LOG_IF(INFO,checked_dual_solution[row] < 0) << "Problem at cut with row " << row << " has dual value " << checked_dual_solution[row] << " writing problem.";
        //        //if ( checked_dual_solution[row] < 0) 
        //        //    osi_solver->writeLp("murks");
        //        assert( checked_dual_solution[row] >= 0 || CompareWithZero(checked_dual_solution[row]));
        //        cut_rhs += (*it)->cut_->lb()*checked_dual_solution[row];
        //        row++;
        //    }
        //}
        // Append value for dual variable bounds
        cut_rhs += computed_dual_value_for_variable_bounds;
        if ( CompareWithZero(cut_rhs))
            cut_rhs = 0.0;

        // Add cut to set of feasibility cuts of parent problem
        std::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
        cut_pointer->setLb(cut_rhs);
        cut_pointer->setUb(infinity_);
        cut_pointer->setRow(stage_data_[current_stage_-1]->num_columns_,&stage_data_[current_stage_-1]->column_indices_[0],&cut_coefficients[0],debug_test_duplicate_indices_);
        // Increase feasCutCounter, together with locking/unlocking a std::mutex, and adding the cut the the parent problem
        std::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
        infeasible_problem_mutex_.lock();
        // Check if this is the first cut we added
        num_added_feasibility_cuts_++;

        benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.push_back(cut_stuff);
        if ( num_added_feasibility_cuts_ == 1 ){
            benders_tree_node_pointer->getParent()->getDataPtr()->last_iteration_start_feasibility_ = --benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.end();
        }
        // If the first stage solver has some optimality cuts, we need to construct it anew, because we add feasibility cuts at the moment at the end of the problem instead of at first!
        //if ( !benders_tree_node_pointer->getParent()->getDataPtr()->optimality_cuts_.empty() && current_stage_ == 1){
        //    seperate_first_stage_solver_ = false;
        //    LOG(FATAL)<< "First stage solver is no longer a seperate instance, but build each time. This is due to added feasibility cuts.";
        //}
        //if ( store_cut_information_ || adaptive_ ){
        //    seperate_first_stage_solver_ = false;
        //    LOG(INFO) << "First stage solver is no longer a seperate instance, but build each time. This is due to feasibility cuts.";
        //}

        // Why do we want to not longer allow a seperate_first_stage_solver_ when we add feasibility cuts? 
        // It is the wrong order of cuts.
        // We assume that we add the feasibility cuts first and the optimality cuts second, so that we have blocks of cuts in the problem.
        // We can relax this assumption only, if we store an index for every cut.

        infeasible_problem_mutex_.unlock();

        // Delete dual ray problem
        if ( !cplex_licence_problem && !use_cplex)
            delete dual_ray_problem;
        else if ( !use_cplex ){
            // Reset the solver interface
            dual_ray_problem->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
        }

        if ( fischetti_cut_procedure){
            int number_of_cols = osi_solver->getNumCols()-1;
            osi_solver->deleteCols(1,&number_of_cols);
        }

        // Delete the computed lower and upper bounds
        if ( node_pointer->getRowLowerLength() && !benders_node_pointer->solve_ray_right_hand_side_)
            delete [] row_lower;
        if ( node_pointer->getRowUpperLength() && !benders_node_pointer->solve_ray_right_hand_side_)
            delete [] row_upper;

        delete [] dual_solution_primal_variable_bounds;
    }

    OsiSolverInterface* ParallelNestedBenders::GetSolverByThread(){
        // We assign the used solver here.
        if ( seperate_first_stage_solver_ && current_stage_ == 0 ){
            return osi_solver_; // We do not clear this solver. We ALWAYS reuse it.
        }
        std::map<boost::thread::id,OsiSolverInterface*>::iterator it = osi_solver_map_.find(boost::this_thread::get_id());
        if ( it != osi_solver_map_.end() )
            return (*it).second;
        else {
            std::unique_lock<std::mutex> lock(solver_map_mutex_); 
            OsiSolverInterface* osi_solver = nullptr;
            // Create new osi_solver object from sepearete first stage solver? if we have a mip
            if ( seperate_first_stage_solver_ && integer_problems_[0] ){ // If we have a seperate first stage solver AND a MIP, we need to get rid of the mip first before we clone!
                // If we call this method we are not in the first stage.
                osi_solver_->initialSolve(); // Get rid of the integers!
            }
            osi_solver = osi_solver_->clone(false); // The boolean does not alway do what it is supposed to do.: We can only clone it if it is a LP problem
            // Clean the solver object by loading an empty problem ( that works for cplex, reset crashes cplex)
            osi_solver->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
            // We want to emphasize numerical stability / for CPLEX
            SetSolverOptions(osi_solver);
            // Set primal algorithm
            osi_solver_map_[boost::this_thread::get_id()] = osi_solver;
            return osi_solver;
        }
        return nullptr;
    }

    void ParallelNestedBenders::SetSolverByThread(){
        // Used in the beginning to set a solver for every thread
        {
            std::unique_lock<std::mutex> lock(solver_map_mutex_); 
            std::map<boost::thread::id,OsiSolverInterface*>::iterator it = osi_solver_map_.find(boost::this_thread::get_id());
            if ( it == osi_solver_map_.end() )
            {
                OsiSolverInterface* osi_solver = nullptr;
                // Create new osi_solver object from sepearete first stage solver? if we have a mip
                if ( seperate_first_stage_solver_ && integer_problems_[0] ){ // If we have a seperate first stage solver AND a MIP, we need to get rid of the mip first before we clone!
                    // If we call this method we are not in the first stage.
                    osi_solver_->initialSolve(); // Get rid of the integers!
                }
                osi_solver = osi_solver_->clone(false); // The boolean does not alway do what it is supposed to do.: We can only clone it if it is a LP problem
                // Clean the solver object by loading an empty problem ( that works for cplex, reset crashes cplex)
                osi_solver->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
                // We want to emphasize numerical stability / for CPLEX
                SetSolverOptions(osi_solver);
                // Set primal algorithm
                osi_solver_map_[boost::this_thread::get_id()] = osi_solver;
            }
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));

    }

    void ParallelNestedBenders::ComputeCutCoefficients( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, const std::vector<double>& dual_solution, std::vector<double>* cut_coefficients_pointer )
    {
        assert( !dual_solution.empty());
        SmiNodeData* node_pointer = benders_tree_node_pointer->getDataPtr()->smi_node_data_;
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        assert(cut_coefficients_pointer->size() == stage_data_[current_stage_-1]->num_columns_);

        // Go on with the matrix
        RowOrderedSparseMatrixPointer deterministic_matrix = CreateRowOrderedSparseMatrixPointer( core_->getNode(current_stage_) );
        RowOrderedSparseMatrixPointer stochastic_matrix = CreateRowOrderedSparseMatrixPointer(node_pointer);

        int current_stage_columns = core_->getColStart(current_stage_);
        int column_start_previous_stage = core_->getColStart(current_stage_-1);
        int stochastic_column_index = 0;
        bool stochastic_element_in_row = false;
        // Compute a coefficient for every row
        for (int row = 0; row < stage_pointer->num_rows_; row++){ // For every row we have to calculate the correct value that has to be added to rlo or rup
            double value = 0;
            if ( node_pointer->hasMatrix() )// We only need stochastic T matrix here
                stochastic_element_in_row = (stochastic_matrix.start_[row+1]-stochastic_matrix.start_[row]) ? true : false;

            // We iterate over the row of the deterministic matrix
            for ( int column_index = deterministic_matrix.start_[row]; column_index < deterministic_matrix.start_[row+1];column_index++){
                // Handle stochastic columns only in the presence of stochastic matrix..
                while ( stochastic_element_in_row ){
                    if ( stochastic_column_index < stochastic_matrix.start_[row+1] && stochastic_matrix.indices_[stochastic_column_index] >= current_stage_columns ){
                        // We need to advance the index, if we are still within this row
                        if ( stochastic_column_index < stochastic_matrix.start_[row+1] )
                            stochastic_column_index++;
                        else if ( stochastic_column_index == stochastic_matrix.start_[row+1]){
                            stochastic_element_in_row = false;
                            break; // We reached the end of the row
                        }
                        else
                            break;
                    }
                    else
                        break; // We found a stochastic T matrix entry for this row
                }
                // If we added all the stochastic elements of this row, we have to stop.
                if ( stochastic_element_in_row && stochastic_column_index == stochastic_matrix.start_[row+1])
                    stochastic_element_in_row = false;

                if ( deterministic_matrix.indices_[column_index] < current_stage_columns){ // Only these entries are entries of the T-matrix!
                    if ( stochastic_element_in_row && deterministic_matrix.indices_[column_index] == stochastic_matrix.indices_[stochastic_column_index]){ // Add stochastic element
                        cut_coefficients_pointer->operator[](deterministic_matrix.indices_[column_index]-column_start_previous_stage) += stochastic_matrix.elements_[stochastic_column_index]*dual_solution[row];
                        stochastic_column_index++;
                    }
                    else    
                        cut_coefficients_pointer->operator[](deterministic_matrix.indices_[column_index]-column_start_previous_stage) += deterministic_matrix.elements_[column_index]*dual_solution[row];
                }
            }
            if ( stochastic_element_in_row )
                assert(stochastic_column_index == stochastic_matrix.start_[row+1]);
        }
        // Add cut coefficients for the d_square_case
        if ( d_square_ && stage_pointer->num_d_square_cuts_ > 0 && !stage_data_[current_stage_]->solve_binary_subproblems_){
            std::vector<double> x_results(stage_data_[current_stage_-1]->num_columns_,0.0);
            benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_->transposeTimes(&dual_solution[stage_pointer->num_rows_],&x_results[0]);
            for (int i = 0; i< stage_data_[current_stage_-1]->num_columns_;i++)
                cut_coefficients_pointer->operator[](i) += x_results[i];
        }
    }

    bool ParallelNestedBenders::CompareWithZero( double value ) 
    {
        return std::abs(value) <= zero_tolerance_; // True, if zero
    }

    void ParallelNestedBenders::AppendCutsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        BendersNodeData::cut_container::iterator feas_it = benders_node_pointer->feasibility_cuts_.begin();

        bool add_feas_cut_with_opt_cuts_present = false;
        if ( !benders_node_pointer->feasibility_cuts_.empty()){
            // Add the cuts
            if ( seperate_first_stage_solver_ && current_stage_ == 0 ){
                feas_it = benders_node_pointer->last_iteration_start_feasibility_;
                benders_node_pointer->last_iteration_start_feasibility_ = benders_node_pointer->feasibility_cuts_.end();
            }
			if ( feas_it != benders_node_pointer->feasibility_cuts_.end()){
				if ( level_decomp_ && current_stage_ < num_stages_-1 && integer_problems_[current_stage_] == false){
					OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
					if ( osi_cpx != nullptr && level_cstat_ != nullptr ){
						CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
						CPXLPptr lp_ptr = osi_cpx->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
						if ( level_length_ == osi_cpx->getNumRows() )
							CPXcopybasednorms(env_ptr,lp_ptr,level_cstat_.get(),level_rstat_.get(),level_dnorm_.get());
						else
							LOG(INFO) << "Trying to copy basednorms where problem has " << osi_cpx->getNumRows() << " rows and had " << level_length_;
					}
				}
				// TODO: This can be changed to addRows
				for (; feas_it != benders_node_pointer->feasibility_cuts_.end();++feas_it) {
					if ( seperate_first_stage_solver_ && current_stage_ == 0 && !benders_node_pointer->optimality_cuts_.empty()){
						add_feas_cut_with_opt_cuts_present = true;
						break;
					}
					assert( (*feas_it)->cut_->row().getNumElements() <= osi_solver->getNumCols());

					if ( !benders_node_pointer->warm_start_added_ && ( (*feas_it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*feas_it)->cut_creation_iteration_ == current_iteration_-1 && current_stage_ == 0)){
						// Add warm start
						SetWarmStart(osi_solver,benders_tree_node_pointer);
						benders_node_pointer->warm_start_added_ = true;
					}
					osi_solver->applyRowCuts(1,(*feas_it)->cut_.get());
				}
			}
        }

        if ( !benders_node_pointer->optimality_cuts_.empty()){
            // Add the cuts
            BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin();
            if ( seperate_first_stage_solver_ &&  current_stage_ == 0 ){
                if (benders_node_pointer->last_iteration_start_optimality_ != benders_node_pointer->optimality_cuts_.end()){
                    it = benders_node_pointer->last_iteration_start_optimality_;
                    //benders_node_pointer->last_iteration_start_optimality_ = benders_node_pointer->optimality_cuts_.end();
                }
                else
                    it = benders_node_pointer->optimality_cuts_.end();
            }
            benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_ = false;

            // Problems with warm start and optimality/feasibility cuts mixes..
            for ( ; it != benders_node_pointer->optimality_cuts_.end();++it) {
                if ( !benders_node_pointer->warm_start_added_ && ( (*it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*it)->cut_creation_iteration_ == current_iteration_-1 && current_stage_ == 0)){
                    // Add warm start
                    SetWarmStart(osi_solver,benders_tree_node_pointer);
                    benders_node_pointer->warm_start_added_ = true;
                    // When you got level decomposition, we use the same problem, so we might want to store/restore the dual norms!
                    // Works only with cplex
                    if ( level_decomp_ && current_stage_ < num_stages_-1 && integer_problems_[current_stage_] == false){
                        OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
                        if ( osi_cpx != nullptr && level_cstat_ != nullptr ){
                            CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
                            CPXLPptr lp_ptr = osi_cpx->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
							if ( level_length_ == osi_cpx->getNumRows() )
								CPXcopybasednorms(env_ptr,lp_ptr,level_cstat_.get(),level_rstat_.get(),level_dnorm_.get());
							else
								LOG(INFO) << "Trying to copy basednorms where problem has now " << osi_cpx->getNumRows() << " rows and had " << level_length_;
                        }

                    }
                }
                if ( (*it)->to_use_ ){
                    assert( (*it)->cut_->row().getNumElements() <= osi_solver->getNumCols());
                    osi_solver->applyRowCuts(1,(*it)->cut_.get());
                }
            }
        }

        if ( add_feas_cut_with_opt_cuts_present ){
            // Where are we now? We added a feas cut in last iteration (so no opt cut), but it is not yet added to the problem
            // What we want: add feas cut after other feas cuts and solve problem!
            // TODO: This can be changed to addRows
            int new_feas_cuts = 0;
            for (; feas_it != benders_node_pointer->feasibility_cuts_.end();++feas_it) {
                assert( (*feas_it)->cut_->row().getNumElements() <= osi_solver->getNumCols());
                if ( !benders_node_pointer->warm_start_added_ && ( (*feas_it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*feas_it)->cut_creation_iteration_ == current_iteration_-1 && current_stage_ == 0)){
                    // Add warm start
                    SetWarmStart(osi_solver,benders_tree_node_pointer);
                    benders_node_pointer->warm_start_added_ = true;
                }
                osi_solver->applyRowCuts(1,(*feas_it)->cut_.get());
                new_feas_cuts++;
            }
            // Now we have the problem with all cuts, that we can solve right now! (with applied warm start)
            SolveSolverInterface(osi_solver,false);
            // Store the warm start
            seperate_first_stage_solver_ = false;
			if ( osi_solver->isProvenOptimal() )
				StoreWarmStart(osi_solver,benders_tree_node_pointer);
            seperate_first_stage_solver_ = true;
            PrintSubproblem(osi_solver,0,"optfeas");

            // Delete the optimality cuts from the problem and add them again, so all feas cuts are clumped together!
            int num_cuts = 0;
            std::vector<int> optimality_cuts_to_delete;
            int optimality_cut_start = getNumRowsProblem()+benders_node_pointer->feasibility_cuts_.size()-new_feas_cuts;
            for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
                if ( (*it)->to_use_ ){
                    optimality_cuts_to_delete.push_back(optimality_cut_start+num_cuts);
                    num_cuts++;
                }
            }
            int number_of_rows_in_problem = osi_solver->getNumRows();
            osi_solver->deleteRows(num_cuts,&optimality_cuts_to_delete[0]);
            int number_of_rows_without_opt_cuts = osi_solver->getNumRows();
            // This should be equal to number of feasibility cuts, otherwise something is wrong! (When not using level decomp and stuff..)
            if (!( level_decomp_ || regularization_ )){
                LOG_IF(ERROR,number_of_rows_without_opt_cuts != benders_node_pointer->feasibility_cuts_.size()+getNumRowsProblem()) << "Problem with adding opt and feas cuts to problem!";

                //assert( number_of_rows_in_problem == benders_node_pointer->optimality_cuts_.size()+benders_node_pointer->feasibility_cuts_.size()+getNumRowsProblem());

            }
            // Add all optimality cuts to solver!
            for (BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
                if ( (*it)->to_use_ ){
                    assert( (*it)->cut_->row().getNumElements() <= osi_solver->getNumCols());
                    osi_solver->applyRowCuts(1,(*it)->cut_.get());
                }
            }

            CoinWarmStartBasis* warm_start_basis = dynamic_cast<CoinWarmStartBasis*>(benders_node_pointer->warm_start_.get());
            if ( warm_start_basis != nullptr){
                std::vector<CoinWarmStartBasis::Status> status_array;
                for (int i = 0; i < num_cuts;i++){
                    status_array.push_back(warm_start_basis->getArtifStatus(i+optimality_cut_start));
                }
                warm_start_basis->deleteRows(num_cuts,&optimality_cuts_to_delete[0]);
                warm_start_basis->resize(number_of_rows_without_opt_cuts+num_cuts,osi_solver->getNumCols());
                for (int i = 0; i < num_cuts;i++){
                    warm_start_basis->setArtifStatus(i+number_of_rows_without_opt_cuts,status_array[i]);
                }                
                osi_solver->setWarmStart(warm_start_basis);
                // Resolve the problem is done in the typical way, outside the scope of this method!
            }
        }

        if ( level_decomp_ && (level_manhattan_ || level_infinity_) && current_stage_ < num_stages_-1 ){
            if ( !seperate_first_stage_solver_ || current_stage_ > 0 ){
                //LOG(FATAL) << "Regularized Decomposition can not work without a seperable first stage solver instance.";
                regularized_initialized_rows_ = false;
            }
            if ( !regularized_initialized_rows_ ){
                regularized_initialized_rows_ = true;// This has to be added to the benders_node_pointer TODO
                int num_columns = stage_data_[current_stage_]->num_columns_;
                int num_theta = benders_tree_node_pointer->getDataPtr()->num_thetas_;
                if ( num_theta == 0)
                    num_theta = getNumThetasByStage(benders_tree_node_pointer);
                // Prepare the new rows that are meant to set w +x >= x' and w - x >= -x'
                int overall_columns = num_columns*2;
                std::vector<double> rowlb(overall_columns);
                std::vector<double> rowub(overall_columns);
                std::vector<int> rowstrt(overall_columns+1);
                std::vector<int> columns(overall_columns*2);
                std::vector<double> elements(overall_columns*2);
                rowstrt[0] = 0;
                for (int i = 0; i < num_columns; i++){ // We need two rows per column!
                    rowlb[2*i] = regularized_solution_[i]; rowub[2*i] = infinity_; // we have a >= x'
                    rowlb[2*i+1] = -regularized_solution_[i]; rowub[2*i+1] = infinity_; // we have a >= -x'
                    columns[4*i] = i; columns[4*i+1] = num_columns+num_theta+ (level_manhattan_ ? i:0); // x+w
                    elements[4*i] = 1; elements[4*i+1] = 1;
                    columns[4*i+2] = i; columns[4*i+3] = num_columns+num_theta+(level_manhattan_ ? i:0); // -x+w
                    elements[4*i+2] = -1; elements[4*i+3] = 1;
                    rowstrt[2*i+1] = rowstrt[2*i]+2; // We have two elements per row, so we have to add two for first row
                    rowstrt[2*i+2] = rowstrt[2*i+1]+2; // We have two elements per row, so we have to add two for second row
                }
                int start_rows = osi_solver->getNumRows();    
                regularized_start_row_ = start_rows;
                // We have to store the row numbers, so we can update these rows later on if the regularized solution changes
                osi_solver->addRows(overall_columns, &rowstrt[0], &columns[0], &elements[0], &rowlb[0], &rowub[0] );
            }
        }

        if ( ( regularization_ || level_decomp_ && level_quadratic_new_variables_) && current_stage_ < num_stages_-1){
            if ( !seperate_first_stage_solver_ || current_stage_ > 0){
                //LOG(FATAL) << "Regularized Decomposition can not work without a seperable first stage solver instance.";
                regularized_initialized_rows_ = false;
            }
            if ( !regularized_initialized_rows_ ){
                regularized_initialized_rows_ = true;
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr == nullptr)
                    LOG(FATAL)<< "OsiSolverInstance is no cplex instance.";
                // Get ENV and LP ptr
                CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);;
                int num_columns = stage_data_[current_stage_]->num_columns_;
                int num_theta = benders_tree_node_pointer->getDataPtr()->num_thetas_;
                if ( num_theta == 0)
                    num_theta = getNumThetasByStage(benders_tree_node_pointer);
                // Prepare the new rows that are meant to set w = x-a
                double * rowlb = new double[num_columns];
                double * rowub = new double[num_columns];
                int * rowstrt = new int[num_columns+1];
                int * columns = new int[2*num_columns];
                double * elements = new double[2*num_columns];
                rowstrt[0] = 0;
                for (int i = 0; i < num_columns; i++){
                    rowlb[i] = regularized_solution_[i]; rowub[i] = regularized_solution_[i]; // we have an equality constraint
                    columns[2*i] = i; columns[2*i+1] = num_columns+num_theta+i;
                    elements[2*i] = 1; elements[2*i+1] = -1;
                    rowstrt[i+1] = rowstrt[i]+2; // We have two elements per row, so we have to add two
                }
                int start_rows = osi_solver->getNumRows();    
                regularized_start_row_ = start_rows;
                // We have to store the row numbers, so we can update these rows later on if the regularized solution changes
                osi_solver->addRows(num_columns, rowstrt, columns, elements, rowlb, rowub );

                //// delete new-ed arrays for rows
                delete[] rowlb;
                delete[] rowub;
                delete[] rowstrt;
                delete[] columns;
                delete[] elements;
            }
        }

    }

    void ParallelNestedBenders::InitializeBendersNodeDataTree(int num_scen_pro_cut)
    {
        tree_ = SmiScenarioTree<BendersNodeData*>();

        // Do a quick check with probabilities of the model.
        double total_prob = 0;
        for (int i = 0; i < num_scenarios_; i++){
            total_prob += smi_->getScenarioProb(i);
        }
        LOG_IF(WARNING,!CompareForEquality(1,total_prob,ParallelNestedBenders::zero_tolerance_)) << "Your model probabilties do not sum up to one, but " << total_prob;
        if ( total_prob > 1+zero_tolerance_ || total_prob < 1-zero_tolerance_ ){
            // Scenario probability is wrong for the leafs. We normalize these and recompute all the probabilities back up the tree 
            for (int i = 0; i < num_scenarios_; i++){
                smi_->getSmiTree()->getLeaf(i)->getDataPtr()->setProb( smi_->getScenarioProb(i)/total_prob );
            }
            // Now we need to calculate the probabilities for all the nodes in the tree!
            SmiTreeNode<SmiScnNode*>* current_node = smi_->getSmiTree()->getRoot();
            std::vector<SmiTreeNode<SmiScnNode*>* > parent_node_vector;
            bool goSibling = false;
            while ( current_node != nullptr ){
                if ( current_node->hasChild() && !goSibling){
                    current_node = current_node->getChild();
                }
                else if ( !goSibling)
                { // We have no childs
                    current_node = current_node->getParent();
                    // Get all the child nodes!
                    std::vector<SmiTreeNode<SmiScnNode*> * >* childs = current_node->getChildren();
                    // Go one step up and get all the childs of this node. Set the proability of this node. Get the siblings.
                    // Sum up the probabilities
                    double sum = 0;
                    BOOST_FOREACH( SmiTreeNode<SmiScnNode*> * child_node, *childs )
                        sum += child_node->getDataPtr()->getProb();
                    current_node->getDataPtr()->setProb(sum);
                    goSibling = true;
                    delete childs;
                    if ( current_node->hasSibling() ){
                        current_node = current_node->getSibling();
                        goSibling = false;
                    }
                    else
                        current_node = current_node->getParent(); // We must update the probability and move then to the 
                }
                else if ( goSibling ){
                    // Go to the sibling node, after we updated our stuff
                    std::vector<SmiTreeNode<SmiScnNode*> * >* childs = current_node->getChildren();
                    // Go one step up and get all the childs of this node. Set the proability of this node. Get the siblings.
                    // Sum up the probabilities
                    double sum = 0;
                    BOOST_FOREACH( SmiTreeNode<SmiScnNode*> * child_node, *childs )
                        sum += child_node->getDataPtr()->getProb();
                    current_node->getDataPtr()->setProb(sum);

                    goSibling = false;
                    if ( current_node->hasSibling() )
                        current_node = current_node->getSibling();
                    else {
                        current_node = current_node->getParent();
                        goSibling = true;
                    }


                }
            }//while
            // We modified your problem
            total_prob = 0;
            for (int i = 0; i < num_scenarios_; i++){
                total_prob += smi_->getScenarioProb(i);
            }
            LOG_IF(ERROR,!CompareForEquality(1,total_prob,ParallelNestedBenders::zero_tolerance_)) << "We tried to fix your problem with the model probabilities, but did not succeed. The probabilities sum up to " << total_prob;
            LOG(INFO) << "We modified your problem by normalizing the leaf node probabilities and updating the scenario tree accordingly.";
        }// if

        std::vector<std::vector<BendersNodeData*> > nodeVec;
        nodeVec.reserve(num_scenarios_);

        // Create one BendersNodeData vector for every scenario
        for (int i = 0; i < num_scenarios_; i++) {
            nodeVec.emplace_back(std::vector<BendersNodeData*>());
        } 
        // Fill the node vectors with real nodes
        std::vector<SmiScnNode *>::iterator it = smi_->getSmiTree()->treeBegin();
        while (it != smi_->getSmiTree()->treeEnd()) {
            BendersNodeData *node = new BendersNodeData((*it)->getNode());
            node->scenario_index_ = (*it)->getScenarioIndex();
            node->probability_ = (*it)->getProb();
            if ( (*it)->getParent() != nullptr )
                node->conditional_probability_ = node->probability_/(*it)->getParent()->getProb();
            if ((*it)->getParent() != nullptr && (*it)->getParent()->getScenarioIndex() != (*it)->getScenarioIndex()) {
                node->branch_scenario_ = (*it)->getParent()->getScenarioIndex();
            }
            nodeVec[(*it)->getScenarioIndex()].push_back(node);
            it++;
        }
        // Fill the tree
        for (int i = 0; i < smi_->getNumScenarios(); i++) {
            BendersNodeData *node = nodeVec[i][0];
            tree_.addPathtoLeaf(node->branch_scenario_, node->smi_node_data_->getStage()-1, nodeVec[i]);
        }

        tree_nodes_by_stage_.clear();
        // Make a list of list of nodes, seperated by stage
        // the first list contains stage 0 nodes, the second stage 1 nodes, the third stage 2 nodes, ...
        tree_nodes_by_stage_.reserve(num_stages_);
        for (int i = 0; i < num_stages_; i++) {
            std::vector<SmiTreeNode<BendersNodeData*>*> vec;
            tree_nodes_by_stage_.push_back(vec);
        }
        tree_nodes_by_stage_[0].push_back(tree_.getRoot());
        // Set probability 1 to root node, to prevent numerical issues
        tree_nodes_by_stage_[0][0]->getDataPtr()->probability_ = 1;
        // Set multicut aggregation for the first stage also
        if ( num_scen_pro_cut)
            tree_nodes_by_stage_[0].back()->getDataPtr()->num_scen_pro_cut_ = num_scen_pro_cut;


        // Create tree nodes by stage vectors
        for (int i = 1; i < num_stages_; i++) {
            int number_of_nodes_in_this_stage = 0;
            int number_of_child_nodes_for_next_stage = 0;
            for (unsigned int j = 0; j < tree_nodes_by_stage_[i-1].size(); j++) {
                std::vector<SmiTreeNode<BendersNodeData*> *> *children = tree_nodes_by_stage_[i-1][j]->getChildren();
                for (int k = 0; k < children->size(); k++) {
                    tree_nodes_by_stage_[i].emplace_back((*children)[k]);
                    tree_nodes_by_stage_[i].back()->getDataPtr()->num_node_in_stage_ = number_of_nodes_in_this_stage++;
                    tree_nodes_by_stage_[i].back()->getDataPtr()->child_node_start_ = number_of_child_nodes_for_next_stage;
                    number_of_child_nodes_for_next_stage += tree_nodes_by_stage_[i].back()->numChildren();
                    // Set num_scen_pro_cut, if we want to use it..
                    if ( num_scen_pro_cut ){
                        tree_nodes_by_stage_[i].back()->getDataPtr()->num_scen_pro_cut_ = num_scen_pro_cut;
                    }
                }
                delete children;
            }
        }

        // Check for probability at every stage. Sanity check, that we can, but do not need to run.
        //for (int i = 0; i < num_stages_; i++){
        //    double sum = 0;
        //    
        //    for (int j = 0; j < tree_nodes_by_stage_[i].size(); j++){
        //        sum += tree_nodes_by_stage_[i][j]->getDataPtr()->probability_;
        //    }
        //    if ( sum > 1 + zero_tolerance_ || sum < 1-zero_tolerance_)
        //        LOG(ERROR) << "Probabilities in stage " << i << " do not sum to one, but " << sum;
        //}
    }

    void ParallelNestedBenders::DeleteSubproblem( OsiSolverInterface* osi_solver,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Delete all the stuff we created
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        SmiNodeData* node_pointer = benders_node_pointer->smi_node_data_;

        if ( benders_node_pointer->unbounded_ && benders_node_pointer->unbounded_row_ > 0 ){
            osi_solver->deleteRows(1,&benders_node_pointer->unbounded_row_);
            //SolveSolverInterface(osi_solver);
            benders_node_pointer->unbounded_row_ = 0;
            // Add this to all methods that resolve the problem in one way or another!
        }
        benders_node_pointer->unbounded_ = false;
        benders_node_pointer->unbounded_problem_ = false;

        delete benders_node_pointer->matrix_; benders_node_pointer->matrix_ = nullptr;
        if ( node_pointer->getColLowerLength())
            delete [] benders_node_pointer->column_lower_; 
        benders_node_pointer->column_lower_ = nullptr;

        if ( node_pointer->getColUpperLength())
            delete [] benders_node_pointer->column_upper_; 
        benders_node_pointer->column_upper_ = nullptr;

        if ( node_pointer->getObjectiveLength())
            delete [] benders_node_pointer->objective_; 
        benders_node_pointer->objective_ = nullptr;

        delete [] benders_node_pointer->row_upper_; benders_node_pointer->row_upper_ = nullptr;
        delete [] benders_node_pointer->row_lower_; benders_node_pointer->row_lower_ = nullptr;

        benders_node_pointer->last_iteration_start_optimality_ = benders_node_pointer->optimality_cuts_.end();
    }

    void ParallelNestedBenders::findAggregationPossibilities( BendersNodeData* param1, std::vector< std::vector<int> >& mapping, int num_childs, int num_cuts, int num_subproblems_per_cut )
    {
        assert(false);
        //throw std::exception("The method or operation is not implemented.");
    }

    void ParallelNestedBenders::IntraIterationCutAggregation( OsiSolverInterface* osi_solver,  SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Should we really use this method?
        if ( benders_tree_node_pointer->getDataPtr()->num_thetas_ <= 1){
            // IN the case of pure single cuts, we can still remove cuts from the problem!
            //return;
        }
        if ( benders_tree_node_pointer->getDataPtr()->dual_solution_.empty()){
            LOG(INFO) << "You tried to do cut consolidation without dual solution information.";
            return;
        }

        double start_time = CoinCpuTime();
        BendersNodeData* benders_node_pointer(benders_tree_node_pointer->getDataPtr());
        // What do we need? The dual values. They are stored and thus checked against zero tolerance
        assert( !benders_node_pointer->dual_solution_.empty());
        assert(store_cut_information_); // If we do not want to store cut information this method should not be called
        CutStuffComparison comparison_object; // Compares number,active value and non-active-generations



        bool active_aggregates_per_aggregate_information = false;
        // We want to add a new single cut after deletion of old cuts!

        if ( forward_ && !benders_node_pointer->changed_aggregation_level_){ // We only try to aggregate cuts, if aggregation level did not changed in this iteration!
            // We want to store the information about cuts!
            std::vector< std::list< std::pair<int,double> > > active_aggregates_per_aggregate(benders_node_pointer->num_thetas_); // Stores information about cut from which iterations are active for an aggregate and how active they are
            std::vector< std::pair<int,double> > cuts_from_iteration(current_iteration_); // Stores information about how many cuts are active from a given iteration and how active they are

            std::vector<int> cuts_in_iteration(current_iteration_,0);
            std::vector<int> consolidatable_cuts_in_iteration(current_iteration_,0);
            // Initialize data structures
            for (int i = 0; i< current_iteration_; i++){
                cuts_from_iteration[i].first = 0;
                cuts_from_iteration[i].second = 0;
            }
            // Threshold that defines when active_value is bigger than threshold, we do not aggregates
            CutStuff::active_threshold_ = zero_tolerance_*2.0;
            // Define a threshold above which we do not aggregate cuts for an iteration
            double intra_aggregate_threshold = cut_consolidation_partial_threshold_*benders_node_pointer->num_thetas_;

            // 0: aggregate only if NO cut is active for an iteration, benders_node_pointer->num_thetas_
            // If cut is consecutively non-active for more than strike_threshold iterations, we can aggregate it!
            CutStuff::strike_threshold_ = cut_consolidation_strike_threshold_; 
            bool add_cut = cut_consolidation_add_cut_;

            if ( benders_tree_node_pointer->getDataPtr()->num_thetas_ <= 1 ){
                add_cut = false;
            }

            if ( adaptive_ ){
                intra_aggregate_threshold = 0; // If we use adaptive cuts, we can only handle single cuts when we change aggregates (for now)
                CutStuff::strike_threshold_ = 0; // Otherwise we could end up having no active cut, but not inactive enough, so we do not aggregate all cuts together 
            }
            if ( regularization_ ){
                intra_aggregate_threshold = 0.5*benders_node_pointer->num_thetas_;
                CutStuff::strike_threshold_ = 1;
                add_cut = false;
            }
            if ( CompareWithZero(intra_aggregate_threshold))
                intra_aggregate_threshold = zero_tolerance_*2;

            int current_cut = getNumRowsProblem()+benders_node_pointer->feasibility_cuts_.size(); // TODO: Store size in variable
            if ( !benders_node_pointer->variable_bounds_.empty() )
                current_cut += benders_node_pointer->variable_bounds_.size();

            // TODO: If we have any more cuts expect of these, we need to be careful here! (i.e. level decomp or regularization )
            // Store the number of rows/columns

            /************************************************************************/
            /* Gather information about the cuts : active values                    */
            /************************************************************************/
            for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
                if (!(*it)->to_use_)
                    continue;
                // First cut corresponds to the first row after the rows of the original problem!
                // Find out if cut is active!
                int cut_creation = (*it)->cut_creation_iteration_;
                assert( cut_creation < current_iteration_); // Alls cuts are added before the current iteration (except for the FF-case, there we might need to change the iteration number?)
                int aggregate = (*it)->aggregate_number_;
                // Get active value via dual value
                double active = benders_node_pointer->dual_solution_[current_cut];
                // Put values in the corresponding vectors!
                if ( active >= CutStuff::active_threshold_ ){
                    if ( active_aggregates_per_aggregate_information && aggregate >= 0)
                        active_aggregates_per_aggregate[aggregate].push_back(std::pair<int,double>(cut_creation,active));
                    cuts_from_iteration[cut_creation].first++;
                    cuts_from_iteration[cut_creation].second += active;
                    (*it)->active_value = active;
                    (*it)->not_active_iterations_ = 0;
                }
                else {
                    (*it)->active_value = 0;
                    ++(*it)->not_active_iterations_;
                }
                cuts_in_iteration[cut_creation]++;
                if ( comparison_object(*it,cut_creation) ) 
                    consolidatable_cuts_in_iteration[cut_creation]++;
                (*it)->current_index_in_solver_ = current_cut; // Update current_index_in_solver_ with actual index used in osi_solver
                current_cut++;
            }
            int number_rows_solver = osi_solver->getNumRows();
            assert( current_cut == number_rows_solver || current_cut == number_rows_solver-1 || current_cut == number_rows_solver-stage_data_[current_stage_]->num_columns_); // First case should be default, second case can appear during solution (add one cut to allow a solution ), third case happens when we use regularized decomposition



            /************************************************************************/
            /* Goal: Aggregate all cuts of one iteration into one new single cut for this generation
            /* Reasoning: Fewer cuts reduce solution time, keeping a single cut prevents new generation of cuts
            /************************************************************************/
            if ( benders_node_pointer->num_thetas_ > 1 ){ // Only if we have more than one aggregate, this aggregation scheme makes sense

                bool hierarchical_consolidation = true;
                int hierarchy_level = 0;
                std::vector<int> number_of_cuts_per_hierarchy_level;
                number_of_cuts_per_hierarchy_level.push_back(0); // We need no cut for first iteration.

                int already_removed_cuts = 0;
                std::list<std::shared_ptr<CutStuff> > cuts_to_add;
                std::list<BendersNodeData::cut_container::iterator> to_remove_from_optimality_cuts;
                std::vector<int> to_remove_from_solver;
                int reduce_by_ratio = 100;
                int max_minimal_cuts = 1;

                // The active value for all cuts sums up to the number of aggregates. All cuts for each aggregate sum up to 1.
                for (int i = 1; i < current_iteration_; i++){ // Iterate over all iterations
                    assert( cuts_from_iteration[i].second >= 0);
                    //number_of_cuts_per_hierarchy_level.push_back(max(consolidatable_cuts_in_iteration[i]/reduce_by_ratio,max_minimal_cuts)); // So we use one-fourth of the aggregates from now on, no more single-cuts?
                    number_of_cuts_per_hierarchy_level.push_back(max_minimal_cuts); // So we use one-fourth of the aggregates from now on, no more single-cuts?
                    if ( cuts_from_iteration[i].second > intra_aggregate_threshold ) // We only proceed with the aggregation if the active level falls below the threshold
                        continue;
                    if ( cuts_in_iteration[i] <= max_minimal_cuts )// We do not need to consolidate single cuts ( when we have not consolidated cuts earlier )
                        continue;



                    //LOG_IF(INFO,cuts_from_iteration[i].second > 0) << "Iteration " << i << " is still active with " << cuts_from_iteration[i].second << " for only " << cuts_from_iteration[i].first << " cuts";
                    // Let's put all the cuts of one iteration in one single cut!
                    BendersNodeData::cut_container::iterator cut_stuff_it = std::find_if(benders_node_pointer->optimality_cuts_.begin(), benders_node_pointer->optimality_cuts_.end(),std::bind2nd(CutStuffComparison(),i));
                    if ( cut_stuff_it == benders_node_pointer->optimality_cuts_.end())
                        continue; // In this case we have an iteration where we did not added any optimality cuts
                    if ( (*cut_stuff_it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_ALL ){
                        // Rethink this // TODO
                        continue; // We jump over cuts we already intra-aggregated
                    }
                    assert( (*cut_stuff_it)->aggregate_number_ >= 0 || (*cut_stuff_it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL);
                    assert( (*cut_stuff_it)->active_value < CutStuff::active_threshold_);

                    std::vector<std::shared_ptr<CutStuff> > cut_stuffs; // Will have number_of_cuts_per_hierarchy_level many elements
                    std::vector<double*> new_elements_vector;
                    std::vector<int*> new_indices_vector;
                    std::vector<double> rhs_vector;
                    int col_size = stage_data_[current_stage_]->num_columns_+ benders_node_pointer->num_thetas_; 
                    BendersNodeData::cut_container::iterator cut_it = cut_stuff_it;
                    // We need to create more cut_stuff elements here!
                    int count_cuts = 0;
                    for ( count_cuts = 0; count_cuts < number_of_cuts_per_hierarchy_level[i]; count_cuts++){
                        cut_stuffs.push_back(std::shared_ptr<CutStuff>(*cut_it));
                        // We need to create a new vector
                        // This many variables are needed
                        double * new_elements = new double[col_size]; // Ownership is taken later by CoinPackedVector new_row
                        int * new_indices = new int[col_size];// Ownership is taken later by CoinPackedVector new_row
                        for (int k = 0; k < col_size; k++){
                            new_elements[k] = 0.0;
                            new_indices[k] = k;
                        }
                        new_elements_vector.push_back(new_elements);
                        new_indices_vector.push_back(new_indices);
                        rhs_vector.push_back(0.0);

                        //if ( count_cuts == number_of_cuts_per_hierarchy_level[i]-1){ // When we created all number_of_cuts_per_hierarchy_level many cuts, we can stop searching for another cut
                        //    count_cuts++;
                        //    break;
                        //}
                        ++cut_it;
                        cut_it = std::find_if(cut_it, benders_node_pointer->optimality_cuts_.end(),std::bind2nd(CutStuffComparison(),i));
                        if ( cut_it == benders_node_pointer->optimality_cuts_.end() ) // If we do not have enough optimality cuts that fit the criteria, do not consolidate further!
                            break;
                    }
                    if ( cut_it == benders_node_pointer->optimality_cuts_.end()){ // ||  count_cuts < number_of_cuts_per_hierarchy_level[i] ){ // We could not aggregate any cuts together, so we go to the next iteration!
                        for (int j = 0; j < count_cuts; j++){
                            delete [] new_elements_vector[j];
                            delete [] new_indices_vector[j];
                        }
                        continue; 
                    }
                    assert( count_cuts == number_of_cuts_per_hierarchy_level[i]);

                    //LOG(INFO) << "Iteration " << i << " with now " << number_of_cuts_per_hierarchy_level[i] << " and before consolidable " << consolidatable_cuts_in_iteration[i] << " from " << cuts_in_iteration[i];

                    /******
                    // Compute the new cut coefficients and right hand side values
                    // Remove the cuts from the solver (only if they are in there (tested with to_use_)
                    ******/
                    // Declarations of needed variables
                    BendersNodeData::cut_container::iterator end_it = benders_node_pointer->optimality_cuts_.end();
                    int number_cuts_in_this_iteration = 0;

                    // Iterate over cuts of this iteration
                    int cut_counter = 0;

                    for ( BendersNodeData::cut_container::iterator it = cut_stuff_it; it != benders_node_pointer->optimality_cuts_.end(); ++it){
                        // We must change the CoinPackedVector!
                        if ( (*it)->cut_creation_iteration_ == i && (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_ALL ){
                            assert(false);
                        }
                        if ( (*it)->cut_creation_iteration_ == i && (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL ){
                            //// That means
                            //end_it = it;
                            //break; // For now just quit!
                        }
                        //assert( (*it)->aggregate_number_ >= 0);
                        // Decide to which new cut we would like to add the current old cut
                        int cut_counter_modulo = cut_counter % number_of_cuts_per_hierarchy_level[i];
                        if ( cut_counter_modulo >= number_of_cuts_per_hierarchy_level[i] ){
                            cut_counter_modulo = number_of_cuts_per_hierarchy_level[i]-1;
                        }
                        if ( (*it)->cut_creation_iteration_ == i ){
                            if ( comparison_object(*it,i) ){ // See if this cut is aggregatable!
                                for (int j = 0; j < (*it)->cut_->row().getNumElements(); j++ ){
                                    new_elements_vector[cut_counter_modulo][(*it)->cut_->row().getIndices()[j]] += (*it)->cut_->row().getElements()[j];
                                }
                                rhs_vector[cut_counter_modulo] += (*it)->cut_->lb();
                                number_cuts_in_this_iteration++;
                                to_remove_from_optimality_cuts.push_back(it);
                                if ( (*it)->to_use_ ){ // Only remove a cut from the solver, if it is in the solver, otherways we have a problem
                                    to_remove_from_solver.push_back((*it)->current_index_in_solver_);
                                }
                                cut_counter++;
                            }

                        }
                        else if ( cut_counter == benders_node_pointer->num_thetas_)
                            break;
                        if ( cut_counter == consolidatable_cuts_in_iteration[i]) // Shortcut, because we can not find more than this number of cuts in the problem!
                            break;
                    }
                    assert( number_cuts_in_this_iteration > number_of_cuts_per_hierarchy_level[i]);

                    assert( to_remove_from_solver.empty() || to_remove_from_solver.back() < osi_solver->getNumRows());


                    for (int j = 0; j < number_of_cuts_per_hierarchy_level[i]; j++){
                        // Set the values to our old cut
                        CoinPackedVector new_row(col_size,col_size,new_indices_vector[j],new_elements_vector[j],debug_test_duplicate_indices_); // CoinPackedVector takes ownership of new_indices and new_elements
                        // Generate the new single cut!
                        OsiRowCut * new_single_cut = cut_stuffs[j]->cut_.get();
                        new_single_cut->setRow(new_row);
                        new_single_cut->setLb(rhs_vector[j]);
                        if ( consolidatable_cuts_in_iteration[i] == cuts_in_iteration[i] && number_of_cuts_per_hierarchy_level[i] == 1)
                            cut_stuffs[j]->aggregate_number_ = CutStuff::AGGREGATE_OVER_ALL;  // Indicate that this cut spans over all aggregates we have (until now, but this can be redone)
                        else {
                            cut_stuffs[j]->aggregate_number_ = CutStuff::AGGREGATE_OVER_PARTIAL;
                            // We do not want to create these cuts in the case of adaptive_ aggregation
                            LOG_IF(ERROR,adaptive_) << "We IntraIteration aggregated, but only " << number_cuts_in_this_iteration << " instead of " << benders_node_pointer->num_thetas_;
                        }

                        cut_stuffs[j]->active_value = 1.0;
                        cut_stuffs[j]->not_active_iterations_ = 0;
                        cut_stuffs[j]->to_use_ = true; // Because of this, we must at first sort out the cuts for the solver
                        cuts_to_add.push_back(cut_stuffs[j]);
                    }
                } //End For

                if ( !to_remove_from_solver.empty()){


                    // TODO: Implementation issue: we can also delete the cuts from the container with the classical list.erase function and begin-iterator to end_iterator that we also store. Only feasible if it holds that alls cuts we remove are also fulfill the CutStuffComparison operator(), i.e. should be aggregated!

                    // Remove old cuts from container
                    for ( std::list<BendersNodeData::cut_container::iterator>::iterator list_it = to_remove_from_optimality_cuts.begin();list_it != to_remove_from_optimality_cuts.end(); ++list_it )
                        benders_node_pointer->optimality_cuts_.erase(*list_it);
                    // Add aggregated cuts to container
                    if ( add_cut ){
                        // Add all newly created cuts to optimality cut set
                        benders_node_pointer->optimality_cuts_.insert(benders_node_pointer->optimality_cuts_.end(),cuts_to_add.begin(),cuts_to_add.end());
                    }

                    // Remove old cuts from solver
                    if ( !to_remove_from_solver.empty()){
                        osi_solver->deleteRows(to_remove_from_solver.size(),&to_remove_from_solver[0]);
                    }
                    // Add aggregated cuts to solver
                    if ( add_cut ){
                        for (BendersNodeData::cut_container::iterator it = cuts_to_add.begin(); it != cuts_to_add.end(); ++it)
                            osi_solver->applyRowCuts(1,(*it)->cut_.get());
                    }
                    // Apply some kind of warm start here? We did it someplace else in the code, where we looked at the warm-start information.. TODO
                    SolveSolverInterface(osi_solver); 
                    if ( osi_solver->isProvenOptimal() ){
                        // Store warmstart
                        StoreWarmStart(osi_solver,benders_tree_node_pointer);      
                    }
                    else{
                        console_mutex_.lock();
                        LOG(INFO)<< "Resolve after deletion of old cuts has not led to a proper solution.";
                        console_mutex_.unlock();
                    }

                }

            }// End If 

        }//End If Forward
        start_time = CoinCpuTime()-start_time;
        misc_mutex_.lock();
        intra_cut_aggregation_time_ += start_time;
        misc_mutex_.unlock();
    }

    void ParallelNestedBenders::AnalyseOptimalityCuts( OsiSolverInterface* osi_solver,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {

        if ( benders_tree_node_pointer->getDataPtr()->optimality_cuts_.empty()) // We can only do something meaninungful if we have cuts to examine
            return;
        if ( benders_tree_node_pointer->getDataPtr()->num_thetas_ <= 1)
            return;
        double time = CoinCpuTime();
        int threshold = 3; // cuts that are not consecutively active for more than threshold iterations
        double inactive_ratio = 0.7; // Aggregates that are inactive for more than inactive_ratio of all iterations (only feasible iterations!)
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        // We have the dual solution
        if ( benders_node_pointer->dual_solution_.empty()){
            LOG(WARNING) << "You tried to analyse optimality cuts without dual information.";
        }
        // Optimality cuts start after normal rows and feasibility cuts
        int optimality_cut_start = stage_data_[current_stage_]->num_rows_+benders_node_pointer->feasibility_cuts_.size(); // TODO: Performance issues may arise here
        if ( bounds_in_matrix_ )
            optimality_cut_start += benders_node_pointer->variable_bounds_.size();

        int number_of_aggregates = benders_node_pointer->num_thetas_;
        if ( number_of_aggregates <= 3)
            return;

        // Process the data to see if we can aggregate something!
        int inactive_threshold = 2; 
        bool aggregate_aggregates = false;

        /************************************************************************/
        /* Specify a new aggregation, ultimately stored in map_old_to_new and new_nodes_in_cut
        /************************************************************************/
        std::map<int,int> map_old_to_new;
        int new_thetas = 0; 
        int removed_thetas = 0;

        bool all_into_one = true;
        if ( number_of_aggregates <= 3 )
            all_into_one = true;
        std::list<int> aggregates_into_one;
        typedef std::map<int, std::list<int> > many_many_aggregate;
        many_many_aggregate aggregates_into_many;

        /////
        // Use a specific scheme to aggregate aggregates. Each scheme sets either aggregates_into_one or aggregates_into_many.
        /////
        if ( aggregation_scheme_ == PNB_INT_AGGREGATION_STRATEGY_BY_INACTIVE_ITERATION ){
            // Keep a candidate list.
            BendersNodeData::cut_container proposed_cuts;
            std::map<int,double> active_time; // Maps aggregate to overall duals for all cuts for the aggregate
            boost::dynamic_bitset<> to_aggregate(number_of_aggregates); // Proposes cuts which should be aggregated
            std::map<int,double> iteration_wise_impact; // Stores the dual values for the iterations
            std::map<int,int> latest_active_cut_for_aggregate; // Stores the highest iteration creation for an aggregate
            for (int i = 0; i < benders_node_pointer->num_thetas_; i++)
                latest_active_cut_for_aggregate[i] = 0;
            int minimal_active_creation_iteration = current_iteration_;
            int maximal_active_creation_iteration = 0;
            //// Retrieve the data: Iterate over all cuts!
            int current_cut_index = optimality_cut_start; // index of rows where optimality cuts starts
            for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it ){
                assert( current_cut_index <= osi_solver->getNumRows());
                if ( (*it)->aggregate_number_ < 0 ){
                    current_cut_index++;
                    continue;
                }
                if ( !(*it)->to_use_  )// If we do not use the cut, i.e. it is not in the solver, we must not increase the current_cut_index
                    continue;

                if ( CompareWithZero(benders_node_pointer->dual_solution_[current_cut_index]) ){
                    (*it)->not_active_iterations_++;
                    if ( latest_active_cut_for_aggregate[(*it)->aggregate_number_] < (*it)->cut_creation_iteration_)
                        latest_active_cut_for_aggregate[(*it)->aggregate_number_] = (*it)->cut_creation_iteration_; 
                }
                else{
                    active_time[(*it)->aggregate_number_] += benders_node_pointer->dual_solution_[current_cut_index];
                    (*it)->active_value += benders_node_pointer->dual_solution_[current_cut_index];
                    iteration_wise_impact[(*it)->cut_creation_iteration_] += benders_node_pointer->dual_solution_[current_cut_index];
                    //(*it)->not_active_iterations_ = 0;
                    if ( latest_active_cut_for_aggregate[(*it)->aggregate_number_] < (*it)->cut_creation_iteration_ ){
                        latest_active_cut_for_aggregate[(*it)->aggregate_number_] = (*it)->cut_creation_iteration_;
                    }
                    if ( (*it)->cut_creation_iteration_ < minimal_active_creation_iteration )
                        minimal_active_creation_iteration = (*it)->cut_creation_iteration_;
                    if ( (*it)->cut_creation_iteration_ > maximal_active_creation_iteration )
                        maximal_active_creation_iteration = (*it)->cut_creation_iteration_;

                }
                current_cut_index++;
            }
            // Aggregates that have cuts that where inactive long enough get aggregated into one
            // Check if we want to 
            if ( current_iteration_-minimal_active_creation_iteration > threshold ){ // Fixed threshold for now!
                for (int i = 0; i < number_of_aggregates; i++){
                    //if ( aggregates_into_one.size() <= 10 ){
                    if ( latest_active_cut_for_aggregate[i] == minimal_active_creation_iteration )
                        aggregates_into_one.push_back(i);
                    //}
                    else
                        break;// Else we can skip the rest!

                }
            }
        } 
        else if ( aggregation_scheme_ == PNB_INT_AGGREGATION_STRATEGY_BY_AGGREGATE_DISCREPANCY){
            // Aggregates that have a similar low discrepancy get aggregated together
            if ( benders_node_pointer->discrepancy_ == nullptr )
                return; // We have a non nullptr pointer

            double discrepancy_threshold = 0;

            // Min-based
            //discrepancy_threshold = benders_node_pointer->min_discrepancy_+(median-benders_node_pointer->min_discrepancy_)*0.5;
            // Median based
            double median = 0;
            // memcpy to find median, because we need the original discrepancy matching by aggregate number to perform the later comparison
            double * temp_values = new double[number_of_aggregates];
            memcpy(temp_values,benders_node_pointer->discrepancy_,sizeof(double)*number_of_aggregates);
            int median_index = number_of_aggregates/2;
            std::nth_element(&temp_values[0],&temp_values[median_index],&temp_values[number_of_aggregates]);
            median = temp_values[median_index];
            if ( number_of_aggregates % 2 == 0){// If we have an even number of aggregates, median is average of the two middle elements of the sequence. Middle Elements are Elements/2 and Elements/2-1. Example: 8/2 = 4, 8/2-1 = 3. 8 numbers are indexed from 0 to 7. Two middle elements are correctly 3 and 4.
                --median_index;
                std::nth_element(&temp_values[0],&temp_values[median_index],&temp_values[number_of_aggregates]);
                median = (median+temp_values[median_index])/ 2.0;
            }
            delete [] temp_values;

            // Set threshold
            discrepancy_threshold = median; // We use the median normally here TODO
            if ( CompareWithZero(discrepancy_threshold ))
                discrepancy_threshold = zero_tolerance_;

            // Other opportunities for a rule
            if ( all_into_one ){
                for (int i = 0; i < number_of_aggregates; i++){
                    if ( benders_node_pointer->discrepancy_[i] < discrepancy_threshold){
                        if ( benders_node_pointer->was_under_threshold_[i] == true ){ // FIXME: Set a parameter here! (After first aggregation?)
                            aggregates_into_one.push_back(i);
                            benders_node_pointer->was_under_threshold_[i] = false;
                        }
                        else
                            benders_node_pointer->was_under_threshold_[i] = true;
                    }
                }
                //LOG_IF(INFO,aggregates_into_one.size() <= 1) << "Our threshold was " << discrepancy_threshold;
            }
            else {
                std::vector< std::pair<int,double> > discrepancy_vector;
                discrepancy_vector.reserve(number_of_aggregates);
                for (int i = 0; i < number_of_aggregates; i++)
                    discrepancy_vector.push_back(std::make_pair(i,benders_node_pointer->discrepancy_[i]));

                // Try another approach: Aggregates_Into_Many.
                // Idea: Sort values (ascending or descending?) and pick always the two on the most opposite to aggregate into one.
                std::sort(discrepancy_vector.begin(),discrepancy_vector.end(),comparator_less_int_double_pair);
                int half_index = (int)number_of_aggregates/2; 
                for (int i = 0; i < half_index; i++){
                    many_many_aggregate::iterator it = aggregates_into_many.find(discrepancy_vector[i].first);
                    if ( it == aggregates_into_many.end()){
                        std::list<int> temp_list;
                        if (discrepancy_vector[number_of_aggregates-1-i].second - discrepancy_vector[i].second > median/2.0){ // Condition is set here, if we want to aggregate more than two cuts into one!.
                            temp_list.push_back(discrepancy_vector[i].first);
                            temp_list.push_back(discrepancy_vector[number_of_aggregates-1-i].first);
                            aggregates_into_many.insert(std::make_pair(discrepancy_vector[i].first,temp_list));
                        }
                        else{ // We need to add aggregates anyway, but in a way that we do not aggregate them!
                            temp_list.push_back(discrepancy_vector[i].first);
                            aggregates_into_many.insert(std::make_pair(discrepancy_vector[i].first,temp_list));
                            std::list<int> temp2_list;
                            temp2_list.push_back(discrepancy_vector[number_of_aggregates-1-i].first);
                            aggregates_into_many.insert(std::make_pair(discrepancy_vector[number_of_aggregates-1-i].first,temp2_list));

                        }
                    }
                }
                if ( number_of_aggregates % 2 == 1){// If we have an odd number_of_aggregates, we need to add the last entry by hand!
                    half_index;
                    std::list<int> temp_list;
                    temp_list.push_back(discrepancy_vector[half_index].first);
                    aggregates_into_many.insert(std::make_pair(discrepancy_vector[half_index].first,temp_list));
                }
            }
        }

        //////
        // Update the BendersNodeData to reflect the new aggregation we achieved by parsing aggregates_into_one or aggregates_into_many
        //////
        if ( aggregates_into_one.size() <= 1 && aggregates_into_many.empty()){
            // We can break here!
            //LOG(INFO) << "We did not aggregate any aggregate in this iteration. #Aggregates " << benders_node_pointer->num_thetas_;
        }
        else {
            aggregate_aggregates = true;

            int aggregate_index = 0;
            // Determine the mapping of old aggregates to new aggregates
            if ( all_into_one && aggregates_into_one.size() > 1){
                new_thetas = number_of_aggregates - (aggregates_into_one.size()-1); // Total new number of aggregates!
                removed_thetas = benders_node_pointer->num_thetas_-new_thetas;
                int first_agg = aggregates_into_one.front();
                for (int i = 0; i < number_of_aggregates; i++){
                    if ( !aggregates_into_one.empty() && i == aggregates_into_one.front() ){
                        map_old_to_new[i] = first_agg;
                        aggregates_into_one.pop_front();
                        if ( i == first_agg)
                            aggregate_index++;
                    }
                    else{
                        map_old_to_new[i] = aggregate_index;
                        aggregate_index++;
                    }
                }
            }
            else if ( !all_into_one && !aggregates_into_many.empty()) {
                //new_thetas = number_of_aggregates - aggregates_into_many.size(); // Total new number of aggregates! Only counts for entries with more than one other value!
                //if ( number_of_aggregates % 2 == 1) // In case of odd number, we have to reduce the number of new_thetas by one!
                //    new_thetas++;
                //removed_thetas = benders_node_pointer->num_thetas_-new_thetas;
                for ( many_many_aggregate::iterator it = aggregates_into_many.begin(); it != aggregates_into_many.end() ; ++it){
                    for ( std::list<int>::iterator list_it = it->second.begin(); list_it != it->second.end(); ++list_it){
                        map_old_to_new[*list_it] = aggregate_index;
                    }
                    aggregate_index++;
                }
                new_thetas = aggregate_index;
                removed_thetas = benders_node_pointer->num_thetas_ - new_thetas;
                if ( removed_thetas == 0)
                    aggregate_aggregates = false;
                assert( removed_thetas >= 0);
                // In the end every old aggregate should have a correct mapping
            }

            if ( aggregate_aggregates ){
                // Determine the mapping of scenarios to aggregates
                std::vector< std::vector<int> > new_nodes_in_cut(new_thetas);
                for (int i = 0; i < benders_node_pointer->nodes_in_cut_.size(); i++){
                    // Insert values of old nodes_in_cut into new_nodes_in_cut.
                    new_nodes_in_cut[map_old_to_new[i]].insert(new_nodes_in_cut[map_old_to_new[i]].begin(),benders_node_pointer->nodes_in_cut_[i].begin(), benders_node_pointer->nodes_in_cut_[i].end());
                }

                LOG(INFO) << " Aggregated cuts from " << benders_node_pointer->num_thetas_ << " to " << new_thetas;
                benders_node_pointer->num_thetas_ = new_thetas;
                benders_node_pointer->changed_aggregation_level_ = true;
                benders_node_pointer->active_aggregates_ = boost::dynamic_bitset<>(new_thetas);
                benders_node_pointer->active_aggregates_.set(); // Set all to 1
                // Keep old was_under_threshold values!
                boost::dynamic_bitset<> new_threshold_set(new_thetas);
                size_t index = benders_node_pointer->was_under_threshold_.find_first();
                while( index != boost::dynamic_bitset<>::npos){
                    // Look up new index
                    std::map<int,int>::iterator it = map_old_to_new.find(index);
                    if ( it != map_old_to_new.end())
                        new_threshold_set[it->second] = true;
                    else
                        assert(false);
                    index = benders_node_pointer->was_under_threshold_.find_next(index);
                }
                benders_node_pointer->was_under_threshold_ = new_threshold_set;
                benders_node_pointer->nodes_in_cut_ = new_nodes_in_cut;
                stage_data_[current_stage_]->changed_aggregation_level_ = true; // Set the stage-wise handler also!
            }
        }

        //////
        // Change old cuts to the new scheme ( i.e. new aggregates and the old aggregates )
        /////
        // Iterate once over the set of cuts and create new cuts from the old ones. 
        // Assumption: Cuts are stored in this container en bloc regarding their initial creation. So we can keep the old ones and add the new ones to it
        if ( aggregate_aggregates ){ // Only if we aggregated something, we need to aggregate the actual cuts!
            assert(removed_thetas>0);
            std::vector< std::shared_ptr<CutStuff> > pointer_to_cuts(new_thetas); 
            int creation_iteration = (*benders_node_pointer->optimality_cuts_.begin())->cut_creation_iteration_;
            int aggregates_for_creation_iteration = 0;
            for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ){
                assert( store_cut_information_ || (*it)->cut_creation_iteration_ >= creation_iteration );
                if ( (*it)->cut_creation_iteration_ != creation_iteration || (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL){
                    // Count how many cuts we processed during this creation_iteration. If it is less than new_thetas we have to handle PartialCuts
                    if ( aggregates_for_creation_iteration < new_thetas ){
                        // Find remaining partial cuts and integrate them into one single cut. We need to get hold of one cut instance to start with. We start with the pointer_to_cuts array
                        std::shared_ptr<CutStuff> new_single_cut;
                        for (int i = 0; i < new_thetas; i++){
                            if ( new_single_cut != nullptr){
                                CombineTwoCuts(new_single_cut,pointer_to_cuts[i]);
                                // Remove added cut from optimality cut set
                                benders_node_pointer->optimality_cuts_.remove(pointer_to_cuts[i]);
                            }
                            if ( pointer_to_cuts[i] != nullptr && new_single_cut == nullptr){
                                new_single_cut = pointer_to_cuts[i];
                            }

                        }
                        assert( new_single_cut != nullptr); // We have a cut to work with
                        BendersNodeData::cut_container::iterator temp_it = it;
                        while ( aggregates_for_creation_iteration < new_thetas ){
                            assert( temp_it != benders_node_pointer->optimality_cuts_.end() );
                            if ( (*temp_it)->cut_creation_iteration_ == creation_iteration){
                                CombineTwoCuts(new_single_cut,*temp_it);
                                // Add the correct number of aggregates TODO
                                aggregates_for_creation_iteration += GetNumberOfAggregatesFromCut(*temp_it);
                                // Remove the temp_it cut from the set of optimality cuts!
                                benders_node_pointer->optimality_cuts_.erase(temp_it++);
                            }
                            else
                                ++temp_it;
                        }

                    }
                    // TODO: Sparsify existing cuts!
                    // Delete the iterator_to_cuts vector
                    pointer_to_cuts.clear();
                    pointer_to_cuts.resize(new_thetas);
                    creation_iteration = (*it)->cut_creation_iteration_;
                    aggregates_for_creation_iteration = 0;
                }

                if ( (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_ALL ){
                    // We want to change the row that has the form of cut_coefficients*x + unit_vector*\theta-vector so that it is correct with the new theta vector
                    OsiRowCut * new_single_cut = (*it)->cut_.get();
                    // We need to create a new vector
                    int col_size = stage_data_[current_stage_]->num_columns_+ benders_node_pointer->num_thetas_; // This many variables are needed
                    double * new_elements = new double[col_size]; // Ownership is taken later by CoinPackedVector new_row
                    int * new_indices = new int[col_size];// Ownership is taken later by CoinPackedVector new_row
                    // Initialize part of the vector that stores cut_coefficients
                    for (int i = 0; i < stage_data_[current_stage_]->num_columns_; i++){
                        new_elements[i] = 0;
                        new_indices[i] = i;
                    }

                    // Add the elements of the current (sparse) vector without the theta part
                    int number_aggregates_for_cut = GetNumberOfAggregatesFromCut(*it);
                    for ( int i = 0; i < new_single_cut->row().getNumElements()-number_aggregates_for_cut; i++)
                        new_elements[ new_single_cut->row().getIndices()[i]] = new_single_cut->row().getElements()[i];

                    // Create part of the vector that stores the theta variables
                    for (int i = stage_data_[current_stage_]->num_columns_; i < col_size; i++){
                        new_elements[i] = 1;
                        new_indices[i] = i;
                    }

                    CoinPackedVector cpv(col_size,col_size,new_indices,new_elements,debug_test_duplicate_indices_); // Assigns elements to vector, arrays get nullptred
                    assert( new_elements == nullptr);
                    assert( new_indices == nullptr);
                    new_single_cut->setRow(cpv);
                    aggregates_for_creation_iteration += benders_node_pointer->num_thetas_;
                    ++it;
                    continue; // We go to the next cut (and other iteration
                }
                if ( (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL ){
                    assert(false);
                }
                assert((*it)->aggregate_number_ >= 0);

                int new_number = map_old_to_new[(*it)->aggregate_number_];

                if ( pointer_to_cuts[new_number] == nullptr ){
                    pointer_to_cuts[new_number] = *it;
                    if ( new_number != (*it)->aggregate_number_ ) {
                        // We need to change the index of the aggregate in the cut!
                        (*it)->cut_->mutableRow().getIndices()[(*it)->cut_->row().getNumElements()-1] = stage_data_[current_stage_]->num_columns_+new_number;
                        (*it)->aggregate_number_ = new_number;
                    }
                    if ( !store_cut_information_ )
                        ++aggregates_for_creation_iteration;
                    if ( store_cut_information_ )
                        aggregates_for_creation_iteration += GetNumberOfAggregatesFromCut(*it);
                    ++it;

                }
                else {
                    CombineTwoCuts( pointer_to_cuts[new_number],*it);
                    if ( !store_cut_information_ )
                        ++aggregates_for_creation_iteration;
                    else {
                        aggregates_for_creation_iteration += GetNumberOfAggregatesFromCut(*it);
                    }
                    benders_node_pointer->optimality_cuts_.erase(it++);
                }
            }//End For
            // Delete the columns, delete the cuts and insert all cuts again!
            std::vector<int> removed_columns(removed_thetas);
            for (int i = 0; i < removed_thetas; i++)
                removed_columns[i] = stage_data_[current_stage_]->num_columns_+new_thetas+i;
            osi_solver->deleteCols(removed_thetas,&removed_columns[0]);
            // Delete the cuts
            int num_rows = osi_solver->getNumRows();
            int num_feas_cuts = benders_node_pointer->feasibility_cuts_.size();
            if ( bounds_in_matrix_ )
                num_feas_cuts += benders_node_pointer->variable_bounds_.size();
            std::vector<int> removed_rows(num_rows-stage_data_[current_stage_]->num_rows_-num_feas_cuts);
            for (int i = 0; i < num_rows-stage_data_[current_stage_]->num_rows_-num_feas_cuts;i++)
                removed_rows[i] = stage_data_[current_stage_]->num_rows_+num_feas_cuts+i;
            osi_solver->deleteRows(removed_rows.size(),&removed_rows[0]);
            // Apply row cuts
            for (BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
                if ( (*it)->to_use_ )
                    osi_solver->applyRowCuts(1,(*it)->cut_.get());
            }
            SolveSolverInterface(osi_solver);

            if ( osi_solver->isProvenOptimal() ){
                StoreWarmStart(osi_solver,benders_tree_node_pointer);
                StoreValuesFromSolverInVector(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),osi_solver->getNumRows()); // Store duals
                StoreValuesFromSolverInVector(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,new_thetas); // Store new approximation
            }
            else{
                console_mutex_.lock();
                LOG(ERROR) << "Solver did not solve problem to optimality after aggregation.";
                // Get CPLEX error code
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    // Get ENV and LP ptr
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                    int stat = CPXgetstat(env_ptr,lp_ptr);
                    LOG(INFO) << "Cplex returned solution process with " << stat;
                    //CPXsetintparam(env_ptr,CPX_PARAM_SCAIND,1);
                    //CPXsetdblparam(env_ptr,CPX_PARAM_EPMRK,0.9);
                }
                console_mutex_.unlock();
            }

            no_aggregation_ = false; // We did some aggregation, so we can no longer claim we did not aggregated.

            // Store the data! That means: We work with the revised values. The presumably "better" RHS values before we aggregated are therefore lost.
            //StoreData(osi_solver,benders_tree_node_pointer);
        }// End If Aggregate



        time = CoinCpuTime()-time;
        misc_mutex_.lock();
        adaptive_cut_time_ += time;
        misc_mutex_.unlock();
        //LOG(INFO) << "Took " << endTime << " seconds to analyse cuts.!";

    }

    void ParallelNestedBenders::GenerateAggregatedCuts( SmiTreeNode<BendersNodeData*> * benders_tree_node_pointer, int aggregate_number){
        // Find the subnodes needed for this aggregate
        BendersNodeData * benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        if ( aggregate_number >= benders_node_pointer->oda_aggregates_.size() )
            return;

        benders_node_pointer->active_aggregates_;
        int num_columns = stage_data_[current_stage_-1]->num_columns_;

        std::vector<int>& oda_aggregates(benders_node_pointer->oda_aggregates_[aggregate_number]);
        int num_aggregates = oda_aggregates.size();
        std::vector<double> cut_elements(num_columns+num_aggregates,0.0);
        std::vector<int> cut_indices(num_columns+num_aggregates,0);
        for ( int i = 0; i < num_columns; i++)
            cut_indices[i] = i;
        // Set theta variable index and element
        for (int i = 0; i < num_aggregates; i++){
            cut_indices[num_columns+i] = num_columns+oda_aggregates[i];
            cut_elements[num_columns+i] = 1.0;
        }

        double cut_lower_bound = 0.0;
        // For all the indices, that belong to this cut.
        for ( int k = 0; k < num_aggregates; k++){
            std::vector<int>& nodes_in_cut(benders_tree_node_pointer->getDataPtr()->nodes_in_cut_[oda_aggregates[k]]);
            for ( int j = 0; j < nodes_in_cut.size(); j++){
                std::shared_ptr<OsiRowCut> osi_cut(tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->oda_optimality_cuts_[ oda_chosen_optimality_cut_by_scenario_[nodes_in_cut[j]]]);
                const double * elements = osi_cut->row().getElements();
                const int * indices = osi_cut->row().getIndices();
                for (int element_index = 0; element_index < osi_cut->row().getNumElements()-1; element_index++) // Ignore index of multi-cut theta variable 
                    cut_elements[indices[element_index]] += elements[element_index];
                cut_lower_bound += osi_cut->lb();
            }
        }
        // Add cut to Optimality Cut Set!
        std::shared_ptr<OsiRowCut> osi_row_cut(new OsiRowCut());
        if ( CompareWithZero(cut_lower_bound))
            cut_lower_bound = 0.0;
        osi_row_cut->setLb(cut_lower_bound);
        osi_row_cut->setUb(infinity_);
        osi_row_cut->setRow(num_columns+num_aggregates,&cut_indices[0],&cut_elements[0],debug_test_duplicate_indices_);

        std::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,osi_row_cut));

        misc_mutex_.lock();
        AddOptimalityCutToContainer(benders_tree_node_pointer,cut_stuff);
        added_optimality_cuts_[current_stage_] += 1;
        misc_mutex_.unlock();

    }

    void ParallelNestedBenders::CombineTwoCuts( std::shared_ptr<CutStuff>& first_cut, std::shared_ptr<CutStuff>& cut_to_add)
    {
        if ( first_cut == nullptr)
            assert(false);
        if ( cut_to_add == nullptr)
            return;
        OsiRowCut* first = first_cut->cut_.get();
        OsiRowCut* second = cut_to_add->cut_.get();
        if ( first == second )
            return;
        if ( first_cut->aggregate_number_ < 0)
            assert(false); // We do not know what do do

        // It may be necessary that we have a full blown vector here instead of a sparse one!
        CoinPackedVector& first_vector(first->mutableRow());
        const CoinPackedVector& second_vector(second->row());
        double* elements = first_vector.getElements(); // Elements
        int* indices = first_vector.getIndices(); // and indices
        int first_length = first_vector.getNumElements();

        const double* elements2 = second_vector.getElements();
        const int* indices2 = second_vector.getIndices();
        int length = second_vector.getNumElements();

        // Complicated version
        //if ( first_length == length){ // Not necessarily enough!
        //    for (int i = 0; i < length-1; i++){ // We do not add the theta indices! TODO: What if we already have other cuts?!
        //        if ( indices[i]!=indices2[i]){
        //            create_new = true;
        //            break;
        //            LOG(FATAL)  << "Indices do not match"; // Not a viable option for a release version, so we can do it as in the if-clause TODO
        //        }
        //        elements[i] += elements2[i];
        //    }


        int num_columns = stage_data_[current_stage_]->num_columns_+1;
        if ( first_length > num_columns){
            LOG(INFO) << "Why?";
            num_columns += first_length;
        }
        if ( first_length == num_columns ) {
            for (int i = 0; i < length-1; i++){ // We do not add the theta indices! TODO: What if we already have other cuts?!
                assert( indices2[i]<first_length);
                elements[indices2[i]] += elements2[i];
            }
        }
        else {
            // We need a new NON-sparse vector, so we can not just use memcpy
            elements = new double[num_columns];
            indices = new int[num_columns];
            // Initialize with 0
            CoinFillN(elements,num_columns,0.0);
            for (int i = 0; i < num_columns;i++)
                indices[i] = i;

            // Copy old elements (except the aggregate number)
            for (int i = 0; i < first_length-1; i++){
                if ( first_vector.getIndices()[i] >= num_columns ) 
                    assert(false);
                assert( first_vector.getIndices()[i] < num_columns);
                elements[first_vector.getIndices()[i]] = first_vector.getElements()[i];
            }
            // Copy old aggregate number
            elements[num_columns-1] = 1;
            indices[num_columns-1] = first_vector.getIndices()[first_length-1];
            // Add second vector ( except the aggregate number )
            for (int i = 0; i < length-1;i++){
                if ( second_vector.getIndices()[i] >= num_columns)
                    assert(false);
                assert( second_vector.getIndices()[i] < num_columns);
                elements[second_vector.getIndices()[i]] += elements2[i];
            }
            // Create NEW CoinPackedVector that takes ownership of supplied arrays
            CoinPackedVector cpv(num_columns, num_columns,indices, elements,debug_test_duplicate_indices_);
            first_cut->cut_->setRow(cpv);
        }

        first->setLb(first->lb()+second->lb());
    }



    void ParallelNestedBenders::SetWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        if ( !integer_problems_[current_stage_] ){
            if ( warm_start_ && !benders_tree_node_pointer->getDataPtr()->warm_start_added_ && ( current_stage_ > 0 || !seperate_first_stage_solver_ || level_decomp_)){ // We add a warm_start if configured and first stage solver is not seperate!
                if ( warm_start_scenarios_ ){
                    if ( benders_tree_node_pointer->getDataPtr()->warm_start_  ){ // If we have a warm start object, we tell the solver to use it
                        osi_solver->setWarmStart(benders_tree_node_pointer->getDataPtr()->warm_start_.get());
                    }
                    else{
                        std::unique_lock<std::mutex> temp_lock(stage_warm_start_mutex_);
                        osi_solver->setWarmStart(stage_data_[current_stage_]->warm_start_.get());
                    }
                }
                else if ( warm_start_stage_ ){ // If we have a warm start object, we tell the solver to use it. We need a std::mutex, because we do not have read-only access.
                    std::unique_lock<std::mutex> temp_lock(stage_warm_start_mutex_);
                    osi_solver->setWarmStart(stage_data_[current_stage_]->warm_start_.get());
                }
            }
        }//Integer problems. No Warm start for MIP problems, but we need to set an incumbent solution!
        else {
            // Set incumbent of last solution?
            if ( warm_start_ && !benders_tree_node_pointer->getDataPtr()->warm_start_added_ ){
                double* mip_solution = nullptr;
                std::unique_lock<std::mutex> temp_lock(stage_warm_start_mutex_);
                if ( warm_start_scenarios_ ){
                    mip_solution = benders_tree_node_pointer->getDataPtr()->mip_solution_.get();
                    if ( mip_solution == nullptr){
                        // Try to take the solution wise solution
                        if ( !stage_data_[current_stage_]->mip_solution_.empty() )
                            mip_solution = &stage_data_[current_stage_]->mip_solution_[0][0];

                    }
                }
                else if (  warm_start_stage_ ){
                    if ( !stage_data_[current_stage_]->mip_solution_.empty() )
                        mip_solution = &stage_data_[current_stage_]->mip_solution_[0][0];
                }
                if ( mip_solution == nullptr)
                    return;


                // Get solution of first stage node?
                OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> ( osi_solver);
                if ( osi_cpx != nullptr ){
                    CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
                    CPXLPptr lp_ptr = osi_cpx->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);

                    int num_mipstarts = 1; // We only add one mipstart
                    int begin_mipstarts = 0; // First mipstart starts at location 0
                    const int effortlevel_mipstarts = 0; // CPLEX decides

                    //CPXdelmipstarts(env_ptr,lp_ptr,0,CPXgetnummipstarts(env_ptr,lp_ptr)-1);
                    int err = CPXaddmipstarts(env_ptr,lp_ptr,num_mipstarts,stage_data_[current_stage_]->num_columns_,&begin_mipstarts,&stage_data_[current_stage_]->column_indices_[0],mip_solution,&effortlevel_mipstarts,nullptr);
                    for (int i = 1; i < stage_data_[current_stage_]->mip_solution_.size(); i++){
                        mip_solution = &stage_data_[current_stage_]->mip_solution_[i][0];
                        err = CPXaddmipstarts(env_ptr,lp_ptr,num_mipstarts,stage_data_[current_stage_]->num_columns_,&begin_mipstarts,&stage_data_[current_stage_]->column_indices_[0],mip_solution,&effortlevel_mipstarts,nullptr);
                        LOG_IF(INFO,err != 0) << "CPXcopymipstart returned with " << err;
                    }
                } // End OsiCpx
            } // End If WarmStartAdded
        } // End Else
    } // End Method

    void ParallelNestedBenders::StoreWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        if ( warm_start_ && osi_solver->optimalBasisIsAvailable() && ( current_stage_ > 0 || !seperate_first_stage_solver_ || level_decomp_)){ // Store WarmStart Information, otherwise we reuse the old one..
            if ( warm_start_scenarios_ ){
                if ( benders_tree_node_pointer->getDataPtr()->warm_start_ == nullptr && !stage_data_[current_stage_]->problem_was_solved_in_this_iteration_){
                    std::unique_lock<std::mutex> temp_lock(stage_warm_start_mutex_);
                    stage_data_[current_stage_]->warm_start_.reset(osi_solver->getWarmStart());
                    stage_data_[current_stage_]->problem_was_solved_in_this_iteration_ = true;
                }
                benders_tree_node_pointer->getDataPtr()->warm_start_.reset(osi_solver->getWarmStart());
            }
            else if ( warm_start_stage_ &&  !stage_data_[current_stage_]->problem_was_solved_in_this_iteration_ && ( current_stage_ > 0 || !seperate_first_stage_solver_ || level_decomp_)){
                // We only generate a warm start for the first scenario and use it then for all the problems. Needs std::mutex
                std::unique_lock<std::mutex> temp_lock(stage_warm_start_mutex_);
                stage_data_[current_stage_]->warm_start_.reset(osi_solver->getWarmStart());
                stage_data_[current_stage_]->problem_was_solved_in_this_iteration_ = true;

            }
        }
        else if ( warm_start_ && integer_problems_[current_stage_] && osi_solver->isProvenOptimal() ){
            // Update current mip solution start!

            const double* col_solution = osi_solver->getColSolution();
            if ( warm_start_scenarios_ ){
                if ( benders_tree_node_pointer->getDataPtr()->mip_solution_.get() == nullptr )
                    benders_tree_node_pointer->getDataPtr()->mip_solution_.reset(new double[stage_data_[current_stage_]->num_columns_]);
                std::copy(col_solution,col_solution+stage_data_[current_stage_]->num_columns_,benders_tree_node_pointer->getDataPtr()->mip_solution_.get());
            }

            if ( (warm_start_stage_ || (warm_start_scenarios_ ) )){//&& stage_data_[current_stage_]->mip_solution_.empty() ) ) && !stage_data_[current_stage_]->problem_was_solved_in_this_iteration_){
                std::unique_lock<std::mutex> temp_lock(stage_warm_start_mutex_);
                //if ( stage_data_[current_stage_]->mip_solution_.empty() ){
                stage_data_[current_stage_]->mip_solution_.push_back(std::vector<double>());
                stage_data_[current_stage_]->mip_solution_.back().resize(stage_data_[current_stage_]->num_columns_,0.0);                   
                //}
                std::copy(col_solution,col_solution+stage_data_[current_stage_]->num_columns_,stage_data_[current_stage_]->mip_solution_.back().begin());
                stage_data_[current_stage_]->problem_was_solved_in_this_iteration_ = true;
            }
        }
        benders_tree_node_pointer->getDataPtr()->warm_start_added_ = false;
        if ( current_stage_ == 0 && level_decomp_ && integer_problems_[current_stage_] == false){ // TODO: Store Warm Start via tree nodes! Set a boolean to indicate if we solve projection problem or benders type problem
            // Store warm start for first stage
            OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> ( osi_solver);
            if ( osi_cpx != nullptr ){
                CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
                CPXLPptr lp_ptr = osi_cpx->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                level_cstat_.reset(new int[osi_cpx->getNumCols()]);
                level_rstat_.reset(new int[osi_cpx->getNumRows()]);
                level_dnorm_.reset(new double[osi_cpx->getNumRows()]);

                int cpx_stat = CPXgetbasednorms(env_ptr,lp_ptr,level_cstat_.get(),level_rstat_.get(),level_dnorm_.get());
				level_length_ = CPXgetnumrows(env_ptr,lp_ptr);
				LOG_IF(INFO,cpx_stat != 0) << "CPXgetbasednorms returned with : " << cpx_stat;

            }
        }
    }

    int ParallelNestedBenders::GetNumberOfAggregatesFromCut( std::shared_ptr<CutStuff>& cut )
    {
        if ( cut->aggregate_number_ >= 0)
            return 1;
        if ( cut->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL || cut->aggregate_number_ == CutStuff::AGGREGATE_OVER_ALL){
            int num_aggregates = 0;
            // Count the number of aggregates. We need to know the number of columns for the current stage for that!
            for (int i = cut->cut_->row().getNumElements(); i > 0; i--){
                if ( cut->cut_->row().getIndices()[i-1] > stage_data_[current_stage_]->num_columns_)
                    num_aggregates++;
                else
                    break;
            }
            return num_aggregates;
        }
        assert(false);
        return 0;

    }

    void ParallelNestedBenders::StoreValuesFromSolverInArray( double*& destination_values,const double* source_values,int size )
    {
        delete [] destination_values;
        assert( source_values != nullptr );
        destination_values = new double[size];
        memcpy(destination_values,source_values,size*sizeof(double));
        for (int i = 0; i < size;i++){
            if ( CompareWithZero(destination_values[i]))
                destination_values[i] = 0;
        }
    }

    void ParallelNestedBenders::StoreValuesFromSolverInVector( std::vector<double>& destination_vector,const double* source_values,int size )
    {
        if ( destination_vector.size() < size)
            destination_vector.reserve(size);
        destination_vector.assign(source_values,source_values+size);
        //memcpy(&destination_vector[0],,size*sizeof(double));
        for ( double i : destination_vector){
            if ( CompareWithZero(i))
                i = 0;
        }
    }

    double ParallelNestedBenders::computeScenarioValue( int scenario )
    {
        double sum = 0.0;
        std::vector<BendersNodeData*> scenario_nodes = tree_.getScenario(scenario);
        for ( std::vector<BendersNodeData*>::iterator it = scenario_nodes.begin(); it != scenario_nodes.end(); ++it)
            sum += (*it)->solution_value_;

        return sum;

    }

    void ParallelNestedBenders::fixFirstStageVariables( const double* solution )
    {
        // Copy old bounds
        first_stage_original_col_lower_.reset(new double[smi_->getCore()->getNumCols(0)]);
        first_stage_original_col_upper_.reset(new double[smi_->getCore()->getNumCols(0)]);
        fixed_solution_.reset(new double[smi_->getCore()->getNumCols(0)]);
        memcpy(first_stage_original_col_lower_.get(),smi_->getCore()->getDenseColLower(0),sizeof(double)*smi_->getCore()->getNumCols(0));
        memcpy(first_stage_original_col_upper_.get(),smi_->getCore()->getDenseColUpper(0),sizeof(double)*smi_->getCore()->getNumCols(0));
        memcpy(fixed_solution_.get(),solution,sizeof(double)*smi_->getCore()->getNumCols(0));
        for (int i = 0; i< smi_->getCore()->getNumCols(0);i++){
            if ( CompareWithZero(fixed_solution_[i]) )
                fixed_solution_[i] = 0.0;
        }
        fixed_first_stage_ = true;

    }

    void ParallelNestedBenders::setWarmUpSolution( const double* solution )
    {
        start_solution_ = PNB_INT_USE_SUPPLIED_SOLUTION; 
        warm_up_ = true;
        // Get num cols for all stages except the last
        int num_cols = 0;
        for (int i = 0; i < num_stages_-1; i++)
            num_cols += smi_->getCore()->getNumCols(i);
        warm_up_solution_.reset(new double[num_cols]);
        memcpy(warm_up_solution_.get(),solution,num_cols*sizeof(double)); // We copy all, but the last stage!
    }

    void ParallelNestedBenders::SetSolverOptions( OsiSolverInterface* osi_solver )
    {
        // Silence solver 
        osi_solver->messageHandler()->setLogLevel(0); // Log nothing
        osi_solver->setHintParam(OsiDoPresolveInInitial,true); // FIXME: Set to true again after testing is done!
        if ( lp_method_ == PNB_INT_PRIMAL_SIMPLEX ){
            osi_solver->setHintParam(OsiDoDualInInitial,false);// Parameter : Use Primal or Dual Algorithm for Subproblem Solve.
            osi_solver->setHintParam(OsiDoDualInResolve,false);
        }
        else if ( lp_method_ == PNB_INT_DUAL_SIMPLEX){
            osi_solver->setHintParam(OsiDoDualInInitial,true);
            osi_solver->setHintParam(OsiDoDualInResolve,true);
        }
        else if ( lp_method_ == PNB_INT_BARRIER || lp_method_ == PNB_INT_BARRIER_WITH_CROSSOVER ){ // We ignore the hint, so we use the methos CPXlpopt instead of primopt or dualopt in initialSolve();
            // We must set the algorithm paramter for cplex!
            osi_solver->setHintParam(OsiDoDualInInitial,true,OsiHintIgnore);
            osi_solver->setHintParam(OsiDoDualInResolve,true,OsiHintIgnore);
        }
        else if ( lp_method_ == PNB_INT_CONCURRENT_MASTER_STANDARD ){
            osi_solver->setHintParam(OsiDoDualInInitial,true,OsiHintIgnore);
            osi_solver->setHintParam(OsiDoDualInResolve,true,OsiHintIgnore);
        }

        OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        if ( cpx_ptr ){
            // Get ENV and LP ptr
            CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
            // Numerical emphasis is needed in a decomposition methods
            CPXsetintparam(env_ptr,CPX_PARAM_NUMERICALEMPHASIS,1);
            // Maximal solution time
            CPXsetdblparam(env_ptr,CPX_PARAM_TILIM,maximal_solution_time_);
            // Use aggresive scaling
            CPXsetintparam(env_ptr,CPX_PARAM_SCAIND,1); 
            //CPXsetdblparam(env_ptr,CPX_PARAM_EPGAP,0.01);
            // Use a supplied warm start basis and crush it, to use the presolved model!
            CPXsetintparam(env_ptr,CPX_PARAM_ADVIND,1);
            // Use only one thread per cplex instance (except for the first sage solver)
            if ( osi_solver != osi_solver_){
                if ( lp_method_ == PNB_INT_BARRIER || lp_method_ == PNB_INT_BARRIER_WITH_CROSSOVER ){
                    CPXsetintparam(env_ptr,CPX_PARAM_LPMETHOD,CPX_ALG_BARRIER);
                    CPXsetintparam(env_ptr,CPX_PARAM_BARCROSSALG, lp_method_ == PNB_INT_BARRIER ? -1 : 0); 
                }
                CPXsetintparam(env_ptr,CPX_PARAM_THREADS,1); // Should be set to 1 usually! (
                //CPXsetdblparam(env_ptr,CPX_PARAM_EPGAP,0.5);
            }
            else {
                if ( lp_method_ == PNB_INT_CONCURRENT_MASTER_STANDARD)
                    CPXsetintparam(env_ptr,CPX_PARAM_LPMETHOD,CPX_ALG_CONCURRENT); // Use all algorithms concurrently (should work for the master problem)
            }
            // Set markowitz threshold high for better accuracy 
            CPXsetdblparam(env_ptr,CPX_PARAM_EPMRK,0.999);
            //CPXsetdblparam(env_ptr,CPX_PARAM_EPOPT,1e-07);
            //CPXsetintparam(env_ptr,CPX_PARAM_SCRIND,CPX_ON);

        }
        // We need gurobi options, just in case..
        //OsiGrbSolverInterface* grb_ptr = dynamic_cast<OsiGrbSolverInterface*>(osi_solver);
        //if ( grb_ptr){
        //    GRBmodel* env_ptr = grb_ptr->getLpPtr();
        //    GRBsetintattr(env_ptr,GRB_INT_PAR_THREADS,0);
        //}
    }

    void ParallelNestedBenders::BuildAndSolveRegularizedProblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // To compute ||x-a||^2, we introduce a new variable w that is set to w = x-a and we use ||w||^2 in the objective. 
        // Could also be done by computing x^2 -2xa+ a^2 (so only the x^2 entries are left)
        // Initialize quadratic matrix with entries on the diagonal axis only
        int num_columns = stage_data_[current_stage_]->num_columns_;
        int num_thetas = getNumThetasByStage(benders_tree_node_pointer);

        double approximation_solution = stage_lower_upper_bounds_[0].first;
        // Due to the algorithm it always hold that approximation_solution <= solution_qx



        // Update regularized solution value, when upper bound is available! Only needed once, in the second iteration!
        if ( current_iteration_ == 2 ){
            if ( regularized_solution_value_ >= infinity_ && upper_bound_ < infinity_ ){
                regularized_solution_value_ = upper_bound_; // Compute the regularized solution value!
            }
            // Else: Regularized_solution_value_ stays at >= infinity and we update the incumbent at the first possible chance!
        }

        // Compute Q(x^it)
        double solution_qx = 0;
        bool change_sigma = true;// If we had an infeasible solution earlier, we do not change sigma and do not update the incumbent!
        for (int i = 0; i < num_stages_; i++){
            solution_qx += stage_lower_upper_bounds_[i].second;
            if ( stage_lower_upper_bounds_[i].second <= -infinity_ ) 
                change_sigma = false;
        }

        //regularized_gamma_ = 0.5;
        assert( regularized_gamma_ > 0.5 && regularized_gamma_ < 1.0);

        double old_sigma = regularized_sigma_;
        // Test if we should update the regularized solution
        if ( approximation_solution > -infinity_  && change_sigma){
            // Otherwise we have not yet added theta variables
            if ( solution_qx < (1-regularized_gamma_)*regularized_solution_value_+ (regularized_gamma_)*approximation_solution ){
                // Update the solution!
                assert(!benders_tree_node_pointer->getDataPtr()->col_solution_.empty());
                memcpy(regularized_solution_.get(),&benders_tree_node_pointer->getDataPtr()->col_solution_[0],sizeof(double)*stage_data_[0]->num_columns_);
                // Update the solution value
                regularized_solution_value_ = solution_qx;
                regularized_update_incumbent_ = true;
                if ( 1.0/regularized_sigma_ > 0.125 )
                    regularized_sigma_ *= 2;
                LOG(INFO) << "Exact serious step";

            }
            else if ( solution_qx > regularized_gamma_*regularized_solution_value_+ (1-regularized_gamma_)*approximation_solution ){
                if ( 1.0/regularized_sigma_ < 2 )
                    regularized_sigma_ *= 0.5;
                LOG(INFO) << "nullptr Step";

            }
            else 
                LOG(INFO) << "Approximate Step";
        }

        if ( regularized_update_incumbent_ ){ // Problem is initialized, we may have to update matrix coefficients
            regularized_update_incumbent_ = false;
            // Update problem coefficients!
            // What about warm start?
            for (int index = 0; index < stage_data_[0]->num_columns_; index++)
                osi_solver->setRowBounds(regularized_start_row_+index,regularized_solution_[index],regularized_solution_[index]);
        }

        // Solve problem!
        SolveSolverInterface(osi_solver,false);

        if ( old_sigma != regularized_sigma_ ){
            // The Obj Coefficients changed, so we have to resolve the problem!
            benders_tree_node_pointer->getDataPtr()->warm_start_.reset(osi_solver->getWarmStart());
            // Update coefficients in the objective function!
            std::vector<double> sigmas(stage_data_[current_stage_]->num_columns_*2+num_thetas,0);
            CoinFillN(&sigmas[stage_data_[current_stage_]->num_columns_+num_thetas],stage_data_[current_stage_]->num_columns_,1.0/regularized_sigma_);

            OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
            if ( cpx_ptr == nullptr)
                LOG(FATAL)<< "OsiSolverInstance is no cplex instance.";
            // Get ENV and LP ptr
            CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
            CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);

            // Store warmstart (gets invalidated by new objective)
            if ( integer_problems_[current_stage_] == false )
                benders_tree_node_pointer->getDataPtr()->warm_start_.reset(osi_solver->getWarmStart());
            // Copy quadratic objective (regular objective function is unchanged)
            CPXcopyqpsep(env_ptr,lp_ptr,&sigmas[0]);

            // Set warmstart
            if ( integer_problems_[current_stage_] == false )
                osi_solver->setWarmStart(benders_tree_node_pointer->getDataPtr()->warm_start_.get());
            // Solve problem
            SolveSolverInterface(osi_solver,false);
            LOG(INFO) << "Coefficients: " << 1.0/regularized_sigma_;
        }
    }

    int ParallelNestedBenders::getNumRowsProblem()
    { // TODO: If we want to solve level method in a later stage, we need a modifier that tells us what we want to know!
        if ( (regularization_ || level_decomp_ && level_quadratic_new_variables_)&& current_stage_ == 0 && seperate_first_stage_solver_)
            return stage_data_[current_stage_]->num_rows_+stage_data_[current_stage_]->num_columns_;
        else if ( level_decomp_ && (level_manhattan_ || level_infinity_)&& current_stage_ == 0)
            return stage_data_[current_stage_]->num_rows_+2*stage_data_[current_stage_]->num_columns_;
        else
            return stage_data_[current_stage_]->num_rows_;
    }

    void ParallelNestedBenders::SolveSolverInterface( OsiSolverInterface* osi_solver, bool use_coin )
    {

        // MIP Solving only at first stage..
        if ( integer_problems_[current_stage_] ){

            if ( !use_coin ){
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                    CPXchgctype( env_ptr, lp_ptr, stage_data_[current_stage_]->num_columns_, &stage_data_[current_stage_]->column_indices_[0], &stage_data_[current_stage_]->coltype_array_[0] );
                    // Set MIP options
                    CPXsetintparam(env_ptr,CPX_PARAM_BARCROSSALG,-1);
                    CPXsetintparam(env_ptr,CPX_PARAM_STARTALG,CPX_ALG_BARRIER); // BARRIER
                    CPXsetintparam(env_ptr,CPX_PARAM_SUBALG,CPX_ALG_BARRIER); // BARRIER
                    CPXwriteprob(env_ptr,lp_ptr,"test2.sav","sav");
                    //CPXreadcopyprob(env_ptr,lp_ptr,"test2.lp","lp");
                    CPXmipopt(env_ptr,lp_ptr);
                    int code = CPXgetstat(env_ptr,lp_ptr);
                    if ( code == CPXMIP_INForUNBD){ // Solve without presolve to determine correct direction!
                        LOG(WARNING) << "We need to resolve the problem, because it is either unbounded or infeasible.";
                        CPXsetintparam(env_ptr,CPX_PARAM_PREIND,CPX_OFF);
                        // Discard presolve information by changing objective Sense
                        int objsen = CPXgetobjsen(env_ptr, lp_ptr);
                        CPXchgobjsen(env_ptr, lp_ptr, -objsen);
                        CPXchgobjsen(env_ptr, lp_ptr, objsen);
                        CPXmipopt(env_ptr,lp_ptr);
                        code = CPXgetstat(env_ptr,lp_ptr);
                        CPXsetintparam(env_ptr,CPX_PARAM_PREIND,CPX_ON);
                    }
                    console_mutex_.lock();
                    LOG_IF(WARNING,code != CPXMIP_OPTIMAL) << "MIQP solved with status " << code;
                    console_mutex_.unlock();
                }
            }
            else{
                osi_solver->branchAndBound(); // We might need a fixed first-stage solver for this ( preferable from the viewpoint of performance anyway.. ) 
            }
        }
        else{
            if ( !use_coin && current_stage_ == 0){
                // Solve quadratic problem
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                    CPXsetintparam(env_ptr,CPX_PARAM_QPMETHOD,CPX_ALG_DUAL);
                    CPXqpopt(env_ptr,lp_ptr);
                    int code = CPXgetstat(env_ptr,lp_ptr);
                    if ( code == CPX_STAT_INForUNBD){ // Solve without presolve to determine correct direction!
                        CPXsetintparam(env_ptr,CPX_PARAM_PREIND,CPX_OFF);
                        // Discard presolve information by changing objective Sense
                        int objsen = CPXgetobjsen(env_ptr, lp_ptr);
                        CPXchgobjsen(env_ptr, lp_ptr, -objsen);
                        CPXchgobjsen(env_ptr, lp_ptr, objsen);
                        CPXqpopt(env_ptr,lp_ptr);
                        code = CPXgetstat(env_ptr,lp_ptr);
                        CPXsetintparam(env_ptr,CPX_PARAM_PREIND,CPX_ON);

                    }
                    console_mutex_.lock();
                    LOG_IF(WARNING,code != CPX_STAT_OPTIMAL) << "Quadratic problem solved with status " << code;
                    console_mutex_.unlock();

                }
            }
			else {
    			osi_solver->initialSolve(); // We might need a fixed first-stage solver for this ( preferable from the viewpoint of performance anyway.. ) 
			}

        }
    }

    int ParallelNestedBenders::getNumThetasByStage(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer)
    {
        int num_theta = stage_data_[current_stage_]->num_thetas_;
        if ( num_theta > benders_tree_node_pointer->numChildren() )
            num_theta = benders_tree_node_pointer->numChildren();
        return num_theta;
    }

    void ParallelNestedBenders::AppendColsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Append theta variables first

        // Append other variables later

        if ( current_stage_ == num_stages_-1 )
            return; // No optimality and feasibility cuts on last stage
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        int num_theta = benders_node_pointer->num_thetas_;
        bool no_real_thetas = false;
        if ( num_theta == 0 ){
            num_theta = getNumThetasByStage(benders_tree_node_pointer);
            no_real_thetas = true;
        }
        if ( current_stage_ == 0 && seperate_first_stage_solver_ && current_iteration_ == 1){
            // Add the columns only once!
            int num_cols = num_theta;
            assert( num_cols > 0);
            std::vector<double> lower(num_cols,-infinity_);
            std::vector<double> upper(num_cols,infinity_);
            double term = 0.0;
            if (!benders_node_pointer->optimality_cuts_.empty() )
                term = 1.0;
            std::vector<double> objective(num_cols,term); 
            CoinPackedVectorBase** coin_vector = new CoinPackedVectorBase*[num_cols];
            for (int i = 0; i < num_cols; i++){
                coin_vector[i] = new CoinShallowPackedVector(0,nullptr,nullptr,false);
            }
            osi_solver->addCols(num_cols,coin_vector,&lower[0],&upper[0],&objective[0]);
            LOG_IF(FATAL,osi_solver->getNumCols() != stage_pointer->num_columns_+num_cols) << osi_solver->getNumCols();
            for (int i = 0; i < num_cols; i++){
                delete coin_vector[i];
            }
            delete [] coin_vector;
        }
        // Add the theta-stuff ( how many we have depends on current multi-cut aggregation level.. ) TODO: Reuse stuff
        if ( !benders_node_pointer->optimality_cuts_.empty() ){// We have some optimality cuts, so we need theta variables
            // Remove theta columns from solver, if we reuse it
            int cols_to_remove = osi_solver->getNumCols()-stage_pointer->num_columns_; // This indicates if we reuse a solver, apart from initial run.
            if ( (regularization_ ||level_decomp_ && level_quadratic_new_variables_ || level_decomp_ && level_manhattan_) && regularized_solution_value_ < infinity_ && current_stage_ == 0)
                cols_to_remove -= stage_pointer->num_columns_;


            // We only want to remove the theta columns, if they are different in this node from a previous one. This is necessary if we changed aggregation level in this node or if we changed aggregation level during this iteration for nodes at this stage, as we resolve them during the process!
            if ( cols_to_remove > 0 && (benders_node_pointer->changed_aggregation_level_ || stage_data_[current_stage_]->changed_aggregation_level_) && ( current_stage_ > 0 || !seperate_first_stage_solver_ ) ) { 
                std::vector<int> remove_indices;
                remove_indices.reserve(cols_to_remove);
                for (int i = stage_pointer->num_columns_; i < stage_pointer->num_columns_+cols_to_remove;i++)
                    remove_indices.push_back(i);
                int extra_cols = cols_to_remove-benders_node_pointer->num_thetas_;// Number of cols we actually remove
                for (int i = 0; i < extra_cols; i++)
                    osi_solver->setObjCoeff(stage_pointer->num_columns_+benders_node_pointer->num_thetas_+i,0);
            }
            if ( seperate_first_stage_solver_ && benders_node_pointer->changed_aggregation_level_ && current_stage_ == 0){
                // We need to activate the theta variables!
                for (int i = 0; i < benders_node_pointer->num_thetas_; i++)
                    osi_solver->setObjCoeff(stage_data_[current_stage_]->num_columns_+i,1);
            }
            if ( (current_stage_ > 0 && cols_to_remove == 0) || (current_stage_ == 0 && !seperate_first_stage_solver_)){ // We have no thetas yet, so we add them all!
                int num_cols = benders_node_pointer->num_thetas_;
                assert( num_cols > 0);
                std::vector<double> lower(num_cols,-infinity_);
                std::vector<double> upper(num_cols,infinity_);
                std::vector<double> objective(num_cols,1); 
                CoinPackedVectorBase** coin_vector = new CoinPackedVectorBase*[num_cols];
                for (int i = 0; i < num_cols; i++){
                    coin_vector[i] = new CoinShallowPackedVector(0,nullptr,nullptr,false);
                }
                osi_solver->addCols(num_cols,coin_vector,&lower[0],&upper[0],&objective[0]);
                assert(osi_solver->getNumCols() == stage_pointer->num_columns_+num_cols);
                for (int i = 0; i < num_cols; i++){
                    delete coin_vector[i];
                }
                delete [] coin_vector;
            }
        }
        else {

        }

        // If we have a regularized problem, we add the following cols
        // Get Cplex solver
        if ( (regularization_ || level_decomp_ && level_quadratic_new_variables_) && current_stage_ == 0){

            if ( !seperate_first_stage_solver_ ){
                //LOG(FATAL) << "Regularized Decomposition can not work without a seperable first stage solver instance.";
                regularized_initialized_cols_ = false;
            }
            if ( !regularized_initialized_cols_ ){
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr == nullptr)
                    LOG(FATAL) << "OsiSolverInstance was not a cplex instance.";
                // Get ENV and LP ptr
                CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                int num_columns = stage_data_[current_stage_]->num_columns_;
                regularized_initialized_cols_ = true;
                assert(num_theta>0);

                double * quaddels = new double[num_columns*2+num_theta]; // contains entries of the matrix 
                double term = 1.0/regularized_sigma_; // Cplex multiplies all coefficients with 1/2
                if ( level_decomp_ && level_quadratic_new_variables_ )
                    term = 2.0;
                CoinFillN(quaddels,num_columns+num_theta,0.0);
                CoinFillN(quaddels+num_columns+num_theta,num_columns,term);
                // We add the new columns to the problem
                // We add the new columns to the problem
                std::vector<double> collb(num_columns,-infinity_); 
                std::vector<double> colub(num_columns,infinity_);
                std::vector<double> colobj(num_columns,0.0);// Only needed in level_decomp problem, so we set them there!
                CoinPackedVectorBase** cpvb = new CoinPackedVectorBase*[num_columns];
                for (int i = 0; i < num_columns; i++){
                    cpvb[i] = new CoinShallowPackedVector(0,nullptr,nullptr,false);
                }

                osi_solver->addCols(num_columns,cpvb, &collb[0], &colub[0] , &colobj[0]);
                assert(osi_solver->getNumCols() == 2*num_columns+num_theta);

                // Add the quadratic objective to the objective function
                int error = 0;
                if (integer_problems_[current_stage_])
                    error = CPXchgprobtype(env_ptr,lp_ptr,CPXPROB_MIQP); 
                else
                    error = CPXchgprobtype(env_ptr,lp_ptr,CPXPROB_QP);
                LOG_IF(ERROR,error) << " Error code " << error << " appeared during call to CPXchgprobtype";
                error = CPXcopyqpsep(env_ptr,lp_ptr,quaddels);
                LOG_IF(ERROR,error) << " Error code " << error << " appeared during call to CPXcopyquad";
                CPXsetintparam(env_ptr,CPX_PARAM_QPMETHOD,CPX_ALG_DUAL); // We might want to change that according to the overall algorithm selection setting!

                for (int i = 0; i < num_columns; i++)
                    delete cpvb[i];
                delete[] cpvb;
                delete[] quaddels;
            }//End regularized_initialized
        }//End Regularization

        // We need to add num_columns_ many columns to the problem
        if ( level_decomp_ && (level_manhattan_ || level_infinity_) && current_stage_ == 0){

            if ( !seperate_first_stage_solver_ ){
                regularized_initialized_cols_ = false;
            }
            if ( !regularized_initialized_cols_ ){
                int num_columns = level_manhattan_ ? stage_data_[current_stage_]->num_columns_ : 1;
                regularized_initialized_cols_ = true;
                assert(num_theta>0);

                // We add the new columns to the problem
                std::vector<double> collb(num_columns,0.0); 
                std::vector<double> colub(num_columns,infinity_);
                std::vector<double> colobj(num_columns,0.0);// Only needed in level_decomp problem, so we set them there!
                CoinPackedVectorBase** cpvb = new CoinPackedVectorBase*[num_columns];
                for (int i = 0; i < num_columns; i++){
                    cpvb[i] = new CoinShallowPackedVector(0,nullptr,nullptr,false);
                }
                osi_solver->addCols(num_columns,cpvb, &collb[0], &colub[0], &colobj[0]);

                assert(osi_solver->getNumCols() == stage_data_[current_stage_]->num_columns_ + num_columns+num_theta);

                for (int i = 0; i < num_columns; i++)
                    delete cpvb[i];
                delete[] cpvb;
            }//End regularized_initialized
        }//End Regularization
    }

    void ParallelNestedBenders::ExpandThetaVariablesInOptimalityCut( OsiRowCut* cut,int number_of_columns, int number_of_variables, int old_number_of_thetas /*= 0*/ )
    {
        const CoinPackedVector& row = cut->row();
        int cols_in_row = row.getNumElements();
        int first_index_of_theta = cols_in_row-old_number_of_thetas;

        if ( old_number_of_thetas == 0 ){
            // We assume the caller knows what he is doing and tells us the number of theta variables!
            // We need to find the index, from which we need to copy
            // TODO:
            assert(false);
        }
        int new_cols_in_row = first_index_of_theta+number_of_variables;

        double * new_elements = new double[new_cols_in_row]; // Ownership is taken later by CoinPackedVector new_row
        int * new_indices = new int[new_cols_in_row];// Owernship is taken later by CoinPackedVector new_row
        // Memcpy old elements and indices
        memcpy(new_elements,row.getElements(),sizeof(double)*first_index_of_theta);
        memcpy(new_indices,row.getIndices(),sizeof(int)*first_index_of_theta);
        for (int j = 0; j < number_of_variables; j++){
            new_elements[j+first_index_of_theta] = 1;
            new_indices[j+first_index_of_theta] = number_of_columns+j;
        }
        CoinPackedVector new_row(new_cols_in_row,new_cols_in_row,new_indices,new_elements,debug_test_duplicate_indices_);
        cut->setRow(new_row);

    }

    void ParallelNestedBenders::BuildAndSolveLevelProblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        if ( current_stage_ == num_stages_-1) // No need for a level problem in the last stage!
            return;

        double lower_bound = num_stages_ > 2 ? benders_tree_node_pointer->getDataPtr()->lower_bound_ : lower_bound_;
        double upper_bound = num_stages_ > 2 ? benders_tree_node_pointer->getDataPtr()->upper_bound_ : upper_bound_;

        if ( lower_bound <= -infinity_ || upper_bound >= infinity_) // We do not call the projection problem, when we have not yet achieved finite upper and/or lower bounds?!
            return;



        assert( regularized_solution_.get() != nullptr ); // We need a regularization point 

        detailed_timer_.Start(1); // TODO: This is no longer true in the multi-stage case!

        // We assume that the first stage problem is present!
        OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        CPXENVptr env_ptr = nullptr;
        CPXLPptr lp_ptr = nullptr;
        int cpx_stat = 0;
        if ( cpx_ptr != nullptr){
            // Get Environment and LP Pointer
            env_ptr = cpx_ptr->getEnvironmentPtr();
            lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
        }

        // We need to set a quadratic objective, ||x-x'||_2^2, similar to the regularized decomposition, when we use level_euclidean_
        // We have the objective ||x-x'||_1, when we use level_manhattan_
        // We have the objective ||x-x'||_\infty, when we use level_infinity_

        // Move the current objective function into a row constraint (theta constraints included)
        int number_of_problem_cols = stage_data_[current_stage_]->num_columns_ + getNumThetasByStage(benders_tree_node_pointer);
        int solver_cols = osi_solver->getNumCols();
        assert( number_of_problem_cols <= solver_cols);
        const double* obj_coefs = osi_solver->getObjCoefficients();
        std::vector<int> obj_indices(solver_cols);
        for ( unsigned int i = 0; i < obj_indices.size(); i++)
            obj_indices[i] = i;

        std::unique_ptr<OsiRowCut> osi_cut(new OsiRowCut);
        osi_cut->setLb(-infinity_);
        double descent_target = (1-level_lambda_)*lower_bound+level_lambda_*upper_bound; // TODO: We need stage-wise upper and lower bounds here!
        osi_cut->setUb(descent_target);
        osi_cut->setRow(number_of_problem_cols,&obj_indices[0],obj_coefs,debug_test_duplicate_indices_);
        int number_of_variables = stage_data_[current_stage_]->num_columns_;

        // Store old objective coefs
        std::vector<double> original_obj_coefs(solver_cols,0.0);
        const double * osi_obj_coefs = osi_solver->getObjCoefficients();
        std::copy(osi_obj_coefs,osi_obj_coefs+solver_cols,original_obj_coefs.begin());

        // Quadratic Objective
        if ( cpx_ptr != nullptr){
            if ( level_euclidean_ && !level_quadratic_new_variables_){
                // Compute quadratic coefficient matrix!
                std::vector<double> quadratic_coefs(number_of_problem_cols,2.0); // Cplex alway calculates 1/2 ||x||^2, so we need to anticipate this
                for (int i = number_of_variables; i < number_of_problem_cols; i++) // Set theta variables to zero
                    quadratic_coefs[i] = 0.0;

                // Compute normal objective function elements! -2x_i'
                std::vector<double> linear_coefs(number_of_problem_cols,0.0);
                for (int i = 0; i < number_of_variables; i++){
                    linear_coefs[i] = -regularized_solution_[i]*2.0;
                }

                // Set new linear objective : invalidates old warm_start
                CPXchgobj(env_ptr,lp_ptr,number_of_problem_cols,&obj_indices[0],&linear_coefs[0]);

                // Set quadratic objective
                CPXcopyqpsep(env_ptr,lp_ptr,&quadratic_coefs[0]); // Also changes problem accordingly!
            }
            else if ( level_euclidean_ && level_quadratic_new_variables_){
                // We need to set the row_bounds for the constraints!
                for (int index = 0; index < stage_data_[0]->num_columns_; index++)
                    osi_solver->setRowBounds(regularized_start_row_+index,regularized_solution_[index],regularized_solution_[index]);

                // Set new linear objective ( all zero )
                std::vector<double> linear_coefs(number_of_problem_cols,0.0);
                CPXchgobj(env_ptr,lp_ptr,number_of_problem_cols,&obj_indices[0],&linear_coefs[0]);

                // Can also be generated once and then stored instead of regenerating each iteration!
                double * quaddels = new double[number_of_variables*2+getNumThetasByStage(benders_tree_node_pointer)]; // contains entries of the matrix 
                double term = 1.0/regularized_sigma_; // Cplex multiplies all coefficients with 1/2
                CoinFillN(quaddels,number_of_problem_cols,0.0);
                CoinFillN(quaddels+number_of_problem_cols,number_of_variables,2.0);
                // Set the quadratic objective
                CPXcopyqpsep(env_ptr,lp_ptr,quaddels);
                delete [] quaddels;
            }
        }

        if ( level_manhattan_ || level_infinity_ ){
            // Update the obj_coefficients (null out the others, keep only the added |w| variables)
            // Compute normal objective function elements! -2x_i'
            std::vector<double> linear_coefs(number_of_problem_cols+number_of_variables,0.0);
            assert( solver_cols-number_of_problem_cols > 0);
            std::fill_n(linear_coefs.begin()+number_of_problem_cols,solver_cols-number_of_problem_cols,1.0);

            // Set new linear objective
            osi_solver->setObjCoeffSet(&obj_indices[0],&obj_indices[0]+solver_cols,&linear_coefs[0]);
            //CPXchgobj(env_ptr,lp_ptr,number_of_problem_cols+number_of_variables,&obj_indices[0],&linear_coefs[0]);

            // We need to set the row_bounds for the constraints, i.e. set the equivalent of -w <= x-x' <= w, where x' = regularized_solution_
            for (int index = 0; index < stage_data_[current_stage_]->num_columns_; index++){
                osi_solver->setRowBounds(regularized_start_row_+2*index,regularized_solution_[index],infinity_);
                osi_solver->setRowBounds(regularized_start_row_+2*index+1,-regularized_solution_[index],infinity_);
            }
        }


        // We really want to set the warm_start
        assert(benders_tree_node_pointer->getDataPtr()->warm_start_added_ == false);

        // If feasibility cuts where added in the meantime, we should remove them also?!

        // Remove all the cuts from the last iteration, so we get back the problem to which we can apply our warmstart!
        if ( benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ != benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end()){
            // Get distance
            int num_cuts = std::distance(benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_,benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end());
            int num_rows = osi_solver->getNumRows();
            std::vector<int> cut_indices;
            cut_indices.reserve(num_cuts);
            for (int i = 0; i < num_cuts; i++){
                cut_indices.push_back(num_rows-num_cuts+i);
            }
            osi_solver->deleteRows(num_cuts,&cut_indices[0]);
        }

        // Apply level decomposition constraint.
        osi_solver->applyRowCuts(1,osi_cut.get());
        int level_row_index = osi_solver->getNumRows()-1;

        // Now problem is exactly as it was before the added optimality cuts! (except for the eventual present bounding optimality cut when problem was unbounded otherwise..)
        if ( (level_manhattan_ || level_infinity_) && integer_problems_[current_stage_] == false){
            if ( cpx_ptr != nullptr && benders_tree_node_pointer->getDataPtr()->level_warm_start_length_ > 0){
				int number_of_rows_temp = CPXgetnumrows(env_ptr,lp_ptr);
				if ( number_of_rows_temp == benders_tree_node_pointer->getDataPtr()->level_warm_start_length_)
					CPXcopybasednorms(env_ptr,lp_ptr,benders_tree_node_pointer->getDataPtr()->level_warm_start_cstat_.get(),benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_.get(),benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_.get());
				else 
					LOG(INFO) << "Trying to copy basednorms where problem has " << osi_solver->getNumRows() << " and had " << benders_tree_node_pointer->getDataPtr()->level_warm_start_length_;
				// Question is: Can we really apply this call?
            }
        }
        else {
            osi_solver->setWarmStart(benders_tree_node_pointer->getDataPtr()->level_warm_start_.get());
        }


        // Add the optimality cuts we deleted earlier
        for (BendersNodeData::cut_container::iterator it = benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_; it != benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end();++it)
            osi_solver->applyRowCuts(1,(*it)->cut_.get());

        // If problem was unbounded, we do not care about it in the projection problem case?!


        PrintSubproblem(osi_solver,benders_tree_node_pointer->scenario(),"level");

        detailed_timer_.Stop(1);
        solution_times_.Start(1);

        SolveSolverInterface(osi_solver,false);

        solution_times_.Stop(1);
        detailed_timer_.Start(1);

        // Get the warm start and store it for the next iteration! This means put the value for the level-cut at the end!
        benders_tree_node_pointer->getDataPtr()->level_warm_start_.reset(osi_solver->getWarmStart());
        if ( cpx_ptr != nullptr && (level_manhattan_ || level_infinity_) && integer_problems_[current_stage_] == false){
            benders_tree_node_pointer->getDataPtr()->level_warm_start_cstat_.reset(new int[osi_solver->getNumCols()]);
            benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_.reset(new int[osi_solver->getNumRows()]);
            benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_.reset(new double[osi_solver->getNumRows()]);
            cpx_stat = CPXgetbasednorms(env_ptr,lp_ptr,benders_tree_node_pointer->getDataPtr()->level_warm_start_cstat_.get(),benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_.get(),benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_.get());
			LOG_IF(INFO,cpx_stat != 0) << "CPXgetbasednorms returned with : " << cpx_stat;
            // Now we need to switch the entries of the new optimality cuts with the extra row 
            int num_current_rows = osi_solver->getNumRows();
			benders_tree_node_pointer->getDataPtr()->level_warm_start_length_ = num_current_rows;
            int temp_rstat = benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_[level_row_index];
            double temp_dnorm = benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_[level_row_index];
            for (int i = level_row_index; i < num_current_rows-1; i++){
                benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_[i] = benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_[i+1];
                benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_[i] = benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_[i+1];
            }
            benders_tree_node_pointer->getDataPtr()->level_warm_start_rstat_[num_current_rows-1] = temp_rstat;
            benders_tree_node_pointer->getDataPtr()->level_warm_start_dnorm_[num_current_rows-1] = temp_dnorm;
        }
        if ( level_euclidean_ && integer_problems_[current_stage_] == false){// We need to restructure the warm start in the same way
            // We need to modify the warm start!
            CoinWarmStartBasis* warm_start_basis = dynamic_cast<CoinWarmStartBasis*>(benders_tree_node_pointer->getDataPtr()->level_warm_start_.get());
            if ( warm_start_basis != nullptr){
                int num_current_rows = osi_solver->getNumRows();
                CoinWarmStartBasis::Status temp_stat = warm_start_basis->getArtifStatus(level_row_index);
                for (int i = level_row_index; i < num_current_rows-1; i++){
                    warm_start_basis->setArtifStatus(i,warm_start_basis->getArtifStatus(i+1));
                }
                warm_start_basis->setArtifStatus(num_current_rows-1,temp_stat);
            }
        }

        // Store column solution and theta solution
        StoreValuesFromSolverInVector(benders_tree_node_pointer->getDataPtr()->col_solution_,osi_solver->getColSolution(),number_of_variables);
        // Copy solution to the incumbent as well!
        std::copy(benders_tree_node_pointer->getDataPtr()->col_solution_.begin(),benders_tree_node_pointer->getDataPtr()->col_solution_.end(),regularized_solution_.get());
        assert(number_of_problem_cols-number_of_variables == benders_tree_node_pointer->getDataPtr()->num_thetas_);
        StoreValuesFromSolverInVector(benders_tree_node_pointer->getDataPtr()->next_stage_approximation_,osi_solver->getColSolution()+number_of_variables,benders_tree_node_pointer->getDataPtr()->num_thetas_);

        // Compute objective value! Needed for Upper Bound computation!
        double solution = 0.0;
        for (int i = 0; i < number_of_variables; i++)
            solution += benders_tree_node_pointer->getDataPtr()->col_solution_[i]*original_obj_coefs[i];

        benders_tree_node_pointer->getDataPtr()->solution_value_ = solution;
        // We can not obtain a global lower bound by the projection problem!

        if ( oda_method_ && current_stage_ == 0)
            oda_first_stage_ = solution;

        if ( oda_method_ && current_stage_ == 0){
            oda_phi_i_ = std::accumulate(benders_tree_node_pointer->getDataPtr()->next_stage_approximation_.begin(),benders_tree_node_pointer->getDataPtr()->next_stage_approximation_.end(),oda_first_stage_);
            std::cout << "Phi_i : " << oda_phi_i_ << "\n";
        }

        // Remove row constraint and quadratic objective function afterwards!
        osi_solver->deleteRows(1,&level_row_index);

        // Reset original objective function, when the problem should be reused!
        osi_solver->setObjCoeffSet(&obj_indices[0],&obj_indices[0]+solver_cols,&original_obj_coefs[0]);
        //CPXchgobj(env_ptr,lp_ptr,number_of_problem_cols,&obj_indices[0],&original_obj_coefs[0]);
        // Setting problem to MILP or LP removes quadratic objective function
        if ( cpx_ptr != nullptr){
            if (integer_problems_[current_stage_])
                CPXchgprobtype( env_ptr, lp_ptr, CPXPROB_MILP );
            else
                CPXchgprobtype( env_ptr, lp_ptr, CPXPROB_LP );
        }
        detailed_timer_.Stop(1);

    }

    void ParallelNestedBenders::AppendDSquareCutsToSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // If the problem 

    }

    void ParallelNestedBenders::BuildAndSolveCTripleProblem(  )
    {
        double start_time = CoinCpuTime();
        assert(d_square_);
        // TODO: Put problem description here for C^3-SLP
        //
        // 
        //
        //
        // If we want to call this method, we need already a disjunctive indice and some expected value stuff!

        bool conditional_expectation = true; // Otherwise full expectation is used
        bool restricted_subproblem = true; // Otherwise we build full subproblem!
        bool use_w_counter = false; // Otherwise, we use restricted subproblem procedure
        //assert( restricted_subproblem || use_w_counter );// together it is an xor..
        assert( !(restricted_subproblem && use_w_counter));
        // Create a new problem via matrix or cuts..
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        SmiNodeData* deterministic_node_pointer = core_->getNode(current_stage_); // Deterministic node (there is one per stage)
        int disjunctive_variable_index = stage_data_[current_stage_]->violated_index_;
        if ( disjunctive_variable_index < 0 )
            return; // We can skip the computation of the C^3 problem (no disjunction necessary)
        int num_cols = stage_pointer->num_columns_;
        int num_rows_w_matrix = stage_pointer->num_rows_;
        int num_scenarios = tree_nodes_by_stage_[current_stage_].size();
        int total_rows_w_k_matrix = num_rows_w_matrix+stage_pointer->num_d_square_cuts_;
        assert( disjunctive_variable_index >= 0 && disjunctive_variable_index < num_cols);

        // We need this matrix two times, and transposed. Transpose without reverseOrdering just swaps rowOrdered to colOrdered matrices!
        CoinPackedMatrix* w_matrix = deterministic_node_pointer->generateCoinPackedWMatrix();

        // TODO: We must take care what kind of PI-elements we actually add to this problem, to assure convergence, see paper of Higle/Sen for that.
        int removed_rows = 0;
        LOG(INFO) << "DI : " << disjunctive_variable_index;
        std::vector<int> active_lambda(stage_pointer->num_d_square_cuts_,0); // Can be a bitset!
        if ( stage_pointer->d_square_matrix_ != nullptr){
            // I think we should store the disjunctive variables 
            for (int i = 0; i < stage_pointer->num_d_square_cuts_;i++){
                if ( restricted_subproblem && !use_w_counter){
                    if ( stage_pointer->d_square_disjunctive_indices_[i] < disjunctive_variable_index ){ 
                        // Add this row to the w_matrix
                        w_matrix->appendRow(stage_pointer->d_square_matrix_->getVector(i));
                        active_lambda[i] = 1;
                    }
                    else { // Else add an empty row
                        int int_temp = 0;
                        double double_temp = 0.0;
                        w_matrix->appendRow(0,&int_temp,&double_temp);
                        removed_rows++;
                    }
                }
                if ( use_w_counter && !restricted_subproblem){
                    if ( i <= d_square_w_iteration_[disjunctive_variable_index] )
                        w_matrix->appendRow(stage_pointer->d_square_matrix_->getVector(i));
                    else {
                        // Do we want to append the empty rows anyway? Do so for now
                        int int_temp = 0;
                        double double_temp = 0.0;
                        w_matrix->appendRow(0,&int_temp,&double_temp);
                        removed_rows++;
                    }
                }
                if ( !use_w_counter && !restricted_subproblem ){
                    w_matrix->appendRow(stage_pointer->d_square_matrix_->getVector(i));
                    active_lambda[i] = 1;
                }
            }
            if ( use_w_counter ){
                for (int i = disjunctive_variable_index+1; i < num_cols; i++)
                    d_square_w_iteration_[i] = stage_pointer->num_d_square_cuts_+1;
            }
        }
        w_matrix->transpose(); // From now on col_ordered.

        //total_rows_w_k_matrix -= removed_rows;

        CoinPackedMatrix problem_matrix; // This is the matrix we put in the solver.
        problem_matrix.copyOf(*w_matrix); // Copy w_matrix!
        // Add the same cols at the right end of it.
        std::vector<int> empty_start_array(total_rows_w_k_matrix+1,0);        // Need an empty (so to say) start array
        problem_matrix.appendCols(total_rows_w_k_matrix,&empty_start_array[0],nullptr,nullptr);

        // Second copy of W_matrix
        CoinPackedMatrix w_matrix_two;
        w_matrix_two.setDimensions(w_matrix->getNumRows(),w_matrix->getNumCols()); // Set empty matrix to original w_matrix dimensions
        w_matrix_two.rightAppendPackedMatrix(*w_matrix); // Right append matrix (i.e. new cols) => empty first half, w-matrix in second half
        delete w_matrix; // Not longer needed.

        // Append W-Matrix for the second time at the bottom, thereby generating new rows.
        problem_matrix.bottomAppendPackedMatrix(w_matrix_two);

        // Append the col for the disjunctive element lambda_0_2!
        int col_index = disjunctive_variable_index;
        double col_element = -1.0;
        problem_matrix.appendCol(1,&col_index,&col_element); // Col_index = 2*total_rows_w_k_matrix

        // Append the col for the disjunctive element lambda_1_2!
        col_index = num_cols+disjunctive_variable_index;
        col_element = 1.0;
        problem_matrix.appendCol(1,&col_index,&col_element);// Col_index = 2*total_rows_w_k_matrix+1

        // Append \pi colums to matrix, as well as \pi_0 (but we add the ones later), in a row ordered fashion
        std::vector<int> indices;
        std::vector<int> starts;
        std::vector<double> elements;
        starts.push_back(0);
        for (int i = 0; i < num_cols; i++){
            //indices.push_back(2*total_rows_w_k_matrix+2+i); // Row_ordered, we need to add the column_index
            indices.push_back(i); // Col_ordered, we need to add the row_index
            indices.push_back(num_cols+i);
            elements.push_back(-1.0);
            elements.push_back(-1.0);
            starts.push_back(starts.back()+2);
        }
        for (int i = 0; i < num_scenarios;i++){
            starts.push_back(starts.back());
        }
        problem_matrix.appendCols(num_cols+num_scenarios,&starts[0],&indices[0],&elements[0]);
        problem_matrix.reverseOrdering(); // From column back to row ordering


        // Now we need to append the matrix for all scenarios and pi_0
        // Start with the indices, stays the same for all scenarios!
        std::vector<int> indices_vector_one(total_rows_w_k_matrix+1);//indices for lambda_0_1
        std::vector<int> indices_vector_two(total_rows_w_k_matrix+2);//indices for lambda_1_1
        for (int i = 0; i < total_rows_w_k_matrix; i++){
            indices_vector_one[i] = i;
            indices_vector_two[i] = total_rows_w_k_matrix+i;
        }
        indices_vector_one[total_rows_w_k_matrix] = 0;// Points to current scenario index
        indices_vector_two[total_rows_w_k_matrix] = 2*total_rows_w_k_matrix+1; // We index lambda_1_2
        indices_vector_two[total_rows_w_k_matrix+1] = 0; // points to current scenario index

        int starting_scenario_index = 2*total_rows_w_k_matrix+2+num_cols;// |lambda+pi|

        // Iterate over all the scenarios (cols for lambda_0_2)
        double * temp_array = new double[total_rows_w_k_matrix+2];
        for (int scenario_index = 0;  scenario_index < num_scenarios; scenario_index++){
            indices_vector_one[total_rows_w_k_matrix] = starting_scenario_index+scenario_index;
            memcpy(temp_array,tree_nodes_by_stage_[current_stage_][scenario_index]->getDataPtr()->solver_rhs_,total_rows_w_k_matrix*sizeof(double));
            temp_array[total_rows_w_k_matrix] = -1.0;// pi_0_omega
            for (int i = 0; i < active_lambda.size();i++){
                if ( active_lambda[i] < 1 )
                    temp_array[num_rows_w_matrix+i] = 0.0;
            }
            problem_matrix.appendRow(total_rows_w_k_matrix+1,&indices_vector_one[0],temp_array);
        }
        // Iterate over all the scenarios. (cols for lambda_1_2)
        for (int scenario_index = 0;  scenario_index < num_scenarios; scenario_index++){
            indices_vector_two[total_rows_w_k_matrix+1] = starting_scenario_index+scenario_index;
            memcpy(temp_array,tree_nodes_by_stage_[current_stage_][scenario_index]->getDataPtr()->solver_rhs_,total_rows_w_k_matrix*sizeof(double));
            temp_array[total_rows_w_k_matrix] = 1.0; // upper rounding of disjunctive variable value leads to 1, for binary problems!
            temp_array[total_rows_w_k_matrix+1] = -1.0;// pi_0_omega
            for (int i = 0; i < active_lambda.size();i++){
                if ( active_lambda[i] < 1 )
                    temp_array[num_rows_w_matrix+i] = 0.0;
            }
            problem_matrix.appendRow(total_rows_w_k_matrix+2,&indices_vector_two[0],temp_array);
        }
        delete [] temp_array;

        // Now we have the problem matrix worked out. We need the RHS
        assert(problem_matrix.getNumRows() == num_cols*2+num_scenarios*2);
        assert(problem_matrix.getNumCols() == total_rows_w_k_matrix*2+2+num_cols+num_scenarios);
        int total_cols = total_rows_w_k_matrix*2+2+num_cols+num_scenarios;
        int total_rows = num_cols*2+2+num_scenarios*2;

        int lambda_0_1_col_start = 0;
        int lambda_1_1_col_start = total_rows_w_k_matrix;
        int lambda_0_2_col_start = total_rows_w_k_matrix*2;
        int lambda_1_2_col_start = total_rows_w_k_matrix*2+1;
        int pi_col_start = lambda_1_2_col_start+1;
        int pi_0_col_start = pi_col_start+num_cols;
        int lambda_0_1_pi_row_start = 0;
        int lambda_1_1_pi_row_start = num_cols;
        int lambda_0_1_pi_0_row_start = lambda_1_1_pi_row_start+num_cols;
        int lambda_1_1_pi_0_row_start = lambda_0_1_pi_0_row_start+num_scenarios;

        // Variable Bounds
        std::vector<double> bounds_lower(total_cols);
        std::vector<double> bounds_upper(total_cols);
        for (int i = lambda_0_1_col_start; i < pi_col_start; i++){
            bounds_lower[i] = 0.0;
            bounds_upper[i] = infinity_;
        }
        for (int i = pi_col_start; i < total_cols;i++){
            bounds_lower[i] = -1.0;
            bounds_upper[i] = 1.0;
        }

        // Objective Function
        std::vector<double> obj_coef(total_cols,0.0);
        for (int i = pi_col_start; i < pi_0_col_start; i++)
            obj_coef[i] = 0;
        // Compute overall conditional probability!
        double sum_prob = 0.0;
        for (int i = 0; i < stage_pointer->non_integer_scenarios.size(); i++)
            sum_prob += tree_nodes_by_stage_[current_stage_][stage_pointer->non_integer_scenarios[i]]->getDataPtr()->probability_;
        assert(sum_prob <= 1+integer_tolerance_);

        // Iterate over non_integer_scenarios if we use conditional expectation, otherwise iterate over all scenarios!
        if ( conditional_expectation ){
            for (std::vector<int>::iterator it = stage_pointer->non_integer_scenarios.begin(); it != stage_pointer->non_integer_scenarios.end(); ++it){
                double conditional_prob = tree_nodes_by_stage_[current_stage_][*it]->getDataPtr()->probability_/sum_prob;
                std::vector<double>& col_solution = tree_nodes_by_stage_[current_stage_][*it]->getDataPtr()->col_solution_;
                assert( !col_solution.empty());
                for (int i = 0; i < num_cols; i++)
                    obj_coef[pi_col_start+i] -= conditional_prob*col_solution[i];
            }
        }
        else {
            for (int j = 0; j < tree_nodes_by_stage_[current_stage_].size(); j++){
                std::vector<double>& col_solution = tree_nodes_by_stage_[current_stage_][j]->getDataPtr()->col_solution_;
                assert( !col_solution.empty());
                for (int i = 0; i < num_cols; i++)
                    obj_coef[pi_col_start+i] -= tree_nodes_by_stage_[current_stage_][j]->getDataPtr()->probability_*col_solution[i];
            }
        }

        for (int i = pi_0_col_start; i < total_cols;i++)
            obj_coef[i] = tree_nodes_by_stage_[current_stage_][i-pi_0_col_start]->getDataPtr()->probability_;

        // RHS values
        std::vector<double> rhs_lower(total_rows);
        std::vector<double> rhs_upper(total_rows);
        for (int i = 0; i < 2*num_cols; i++){
            rhs_lower[i] = -infinity_;
            rhs_upper[i] = 0;
        }
        for(int i = 0; i < 2*num_scenarios;i++){
            rhs_lower[i+lambda_0_1_pi_0_row_start] = 0;
            rhs_upper[i+lambda_0_1_pi_0_row_start] = infinity_;
        }

        // Load problem in solver
        OsiSolverInterface * osi_solver = GetSolverByThread();
        osi_solver->loadProblem(problem_matrix,&bounds_lower[0],&bounds_upper[0],&obj_coef[0],&rhs_lower[0],&rhs_upper[0]);

        //std::stringstream string_stream;
        //string_stream << "ctriple-" << current_iteration_ << ".lp";

        // Solve
        //////// Cplex specific cloning
        //OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        //if ( cpx_ptr ){
        //    // Get ENV and LP ptr
        //    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
        //    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
        //    CPXwriteprob(env_ptr,lp_ptr,string_stream.str().c_str(),"lp");
        //    CPXsetintparam(env_ptr,CPX_PARAM_NUMERICALEMPHASIS,0);
        //    CPXchgobjsen(env_ptr,lp_ptr,CPX_MAX);
        //    CPXprimopt(env_ptr,lp_ptr);
        //    double objective_value = 0;
        //    CPXgetobjval(env_ptr,lp_ptr,&objective_value);
        //    std::vector<double> objective_solution(total_cols,0.0);
        //    CPXgetx(env_ptr,lp_ptr,&objective_solution[0],0,total_cols-1);
        //    LOG(INFO) << "Done.";
        //}

        osi_solver->setObjSense(-1); // Maximize
        //osi_solver->writeLp(string_stream.str().c_str());

        osi_solver->initialSolve(); // Solve problem!
        if ( !osi_solver->isProvenOptimal() ){
            LOG(ERROR) << "C^3-SLP solve was not optimal.";
        }
        LOG(INFO) << osi_solver->getObjValue();

        // Store important variables (\lambda,\pi,\pi_0)
        //Lambda
        const double * column_solution = osi_solver->getColSolution();
        StoreValuesFromSolverInArray(stage_pointer->d_square_lambda_0_1_,osi_solver->getColSolution()+lambda_0_1_col_start,total_rows_w_k_matrix);
        StoreValuesFromSolverInArray(stage_pointer->d_square_lambda_1_1_,osi_solver->getColSolution()+lambda_1_1_col_start,total_rows_w_k_matrix);
        stage_pointer->d_square_lambda_0_2_ = osi_solver->getColSolution()[lambda_0_2_col_start];
        stage_pointer->d_square_lambda_1_2_ = osi_solver->getColSolution()[lambda_1_2_col_start];
        // Pi
        StoreValuesFromSolverInArray(stage_pointer->d_square_pi_,osi_solver->getColSolution()+pi_col_start,num_cols);
        if ( stage_pointer->d_square_matrix_ == nullptr ){
            stage_pointer->d_square_matrix_ = new CoinPackedMatrix();
            stage_pointer->d_square_matrix_->reverseOrdering();
            stage_pointer->d_square_matrix_->setDimensions(0,stage_pointer->num_columns_);
        }
        std::vector<int> indices_vector;
        indices_vector.reserve(num_cols);
        for (int i = 0; i < num_cols; i++)
            indices_vector.push_back(i);

        stage_pointer->d_square_matrix_->appendRow(num_cols,&indices_vector[0],stage_pointer->d_square_pi_);



        start_time = CoinCpuTime()-start_time;
        d_square_c_triple_timer_ += start_time;

    }

    void ParallelNestedBenders::BuildAndSolveRHSProblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        double start_time = CoinCpuTime();
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();

        int num_cols_first_stage = stage_data_[0]->num_columns_;
        int num_rows_first_stage = stage_data_[0]->num_rows_;  
        // TODO: Describe RHS-problem!

        //

        // Compute \bar gamma_h (vector) and \nu_h (scalar)
        double bar_nu_0 = 0;
        double bar_nu_1 = 0;
        std::vector<double> bar_gamma_0(num_cols_first_stage,0.0);
        std::vector<double> bar_gamma_1(num_cols_first_stage,0.0);

        // Build RHS via row lower and row upper. 
        int num_rows = stage_data_[current_stage_]->num_rows_;
        double* row_lower = new double[num_rows];
        memcpy(row_lower, core_->getDenseRowLower(current_stage_), num_rows*sizeof(double));
        if ( stage_data_[current_stage_]->stochastic_rhs_ ){
            for (int i = 0; i < benders_node_pointer->smi_node_data_->getRowLowerLength(); i++) {
                row_lower[benders_node_pointer->smi_node_data_->getRowLowerIndices()[i] - core_->getRowStart(current_stage_)] = benders_node_pointer->smi_node_data_->getRowLowerElements()[i];
            }
        }

        double* row_upper = new double[num_rows];
        memcpy(row_upper, core_->getDenseRowUpper(current_stage_), num_rows*sizeof(double));
        if  ( stage_data_[current_stage_]->stochastic_rhs_){
            for (int i = 0; i < benders_node_pointer->smi_node_data_->getRowUpperLength(); i++) {
                row_upper[benders_node_pointer->smi_node_data_->getRowUpperIndices()[i] - core_->getRowStart(current_stage_)] = benders_node_pointer->smi_node_data_->getRowUpperElements()[i];
            }
        }

        for (int i = 0; i < stage_data_[current_stage_]->num_rows_; i++){
            double rhs_value = 0.0;
            if ( row_upper[i] < infinity_) // Question is: Does this work this way? TODO
                rhs_value = row_upper[i];
            if ( row_lower[i] > -infinity_ ){
                rhs_value = row_lower[i];
                assert(row_upper[i]>= infinity_ || row_upper[i] == rhs_value);
            }
            assert( !(row_upper[i] >= infinity_ && row_lower[i] <= -infinity_ ));
            bar_nu_0 += rhs_value*stage_data_[current_stage_]->d_square_lambda_0_1_[i];
            bar_nu_1 += rhs_value*stage_data_[current_stage_]->d_square_lambda_1_1_[i];
        }
        delete [] row_lower;
        delete [] row_upper;
        // \bar_nu
        if ( !benders_tree_node_pointer->getDataPtr()->d_square_cut_rhs_.empty() ){
            for (int i = stage_data_[current_stage_]->num_rows_; i < stage_data_[current_stage_]->num_rows_+stage_data_[current_stage_]->num_d_square_cuts_; i++){
                bar_nu_0 += benders_tree_node_pointer->getDataPtr()->d_square_cut_rhs_[i-stage_data_[current_stage_]->num_rows_]*stage_data_[current_stage_]->d_square_lambda_0_1_[i];
                bar_nu_1 += benders_tree_node_pointer->getDataPtr()->d_square_cut_rhs_[i-stage_data_[current_stage_]->num_rows_]*stage_data_[current_stage_]->d_square_lambda_1_1_[i];
            }
        }
        bar_nu_1 += stage_data_[current_stage_]->d_square_lambda_1_2_;

        // \bar_gamma
        CoinPackedMatrix * t_matrix = core_->getNode(current_stage_)->generateCoinPackedTMatrix();
        // Get Stochastic CoinPackedMatrix
        CoinPackedMatrix * stochastic_t_matrix = benders_tree_node_pointer->getDataPtr()->smi_node_data_->generateCoinPackedTMatrix();
        if ( stochastic_t_matrix->getNumRows() == 0 && stochastic_t_matrix->getNumCols() == 0 ){
            delete stochastic_t_matrix;
        }
        else {
            // Iterate over stochastic matrix and change entries in t-matrix
            assert(false); // Need implementation
        }
        // Compute lambda_h_1^T \cdot T ( or T^T \cdot lambda_h_1 )
        t_matrix->transpose();
        t_matrix->times(stage_data_[current_stage_]->d_square_lambda_0_1_,&bar_gamma_0[0]);
        t_matrix->times(stage_data_[current_stage_]->d_square_lambda_1_1_,&bar_gamma_1[0]);
        // Now we need to add the d_square cut matrix, if we have one!
        if ( benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_ != nullptr){
            std::vector<double> temp_gamma_0(num_cols_first_stage,0.0);
            std::vector<double> temp_gamma_1(num_cols_first_stage,0.0);
            benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_->transposeTimes(stage_data_[current_stage_]->d_square_lambda_0_1_+stage_data_[0]->num_columns_,&temp_gamma_0[0]);
            benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_->transposeTimes(stage_data_[current_stage_]->d_square_lambda_1_1_+stage_data_[0]->num_columns_,&temp_gamma_1[0]);
            // Add these values to bar_gamma_0 and bar_gamma_1
            for (int i = 0; i < bar_gamma_0.size(); i++){
                bar_gamma_0[i] += temp_gamma_0[i];
                bar_gamma_1[i] += temp_gamma_1[i];
            }
        }

        // Finished with bar_gamma and bar_nu generation
        // Order of cols
        // tau_0 = 0..num_rows-1
        // tau_1 = num_rows..2*num_rows-1
        // tau_0_0
        // tau_0_1
        // sigma 
        // delta
        // sigma_0



        CoinPackedMatrix problem_matrix;

        CoinPackedMatrix* first_stage_matrix = core_->getNode(0)->generateCoinPackedWMatrix();
        first_stage_matrix->transpose();
        problem_matrix.copyOf(*first_stage_matrix);

        CoinPackedMatrix a_matrix;
        a_matrix.setDimensions(num_cols_first_stage,num_rows_first_stage);
        a_matrix.rightAppendPackedMatrix(*first_stage_matrix);
        delete first_stage_matrix;

        //Append the extra cols to problem_matrix first
        // Add the same cols at the right end of it.
        std::vector<int> empty_start_array(num_rows_first_stage+1,0);        // Need an empty (so to say) start array
        problem_matrix.appendCols(num_rows_first_stage,&empty_start_array[0],nullptr,nullptr);

        // Append the transposed A matrix again!
        problem_matrix.bottomAppendPackedMatrix(a_matrix);
        // Now we should have the matrix with A^T too times added! (First variables are there tau_0, and tau_1
        // Add rows for sigma_j(omega) >= \tau_h A_j + \tau0h \bar gamma_h(omega)
        std::vector<int> indices;
        std::vector<double> elements;
        std::vector<int> starts;
        starts.push_back(0);
        for (int i = 0; i < num_cols_first_stage; i++){ // Add the sigma-variables
            indices.push_back(i); elements.push_back(-1.0); 
            indices.push_back(num_cols_first_stage+i); elements.push_back(-1.0);            
            starts.push_back(starts.back()+2);
        }

        // Add tau_00
        for (int j = 0; j < num_cols_first_stage;j++){
            indices.push_back(j); elements.push_back(bar_gamma_0[j]);
        }
        starts.push_back(starts.back()+num_cols_first_stage);

        // Add tau_01
        for (int j = num_cols_first_stage; j < 2*num_cols_first_stage;j++){
            indices.push_back(j); elements.push_back(bar_gamma_1[j-num_cols_first_stage]);
        }
        starts.push_back(starts.back()+num_cols_first_stage);
        problem_matrix.appendCols(num_cols_first_stage+2,&starts[0],&indices[0],&elements[0]);

        int sigma_index = num_rows_first_stage*2;
        int tau_0_0_index = problem_matrix.getNumCols()-2;
        int tau_0_1_index = problem_matrix.getNumCols()-1;


        // Add delta and sigma_0 variables
        empty_start_array.clear();
        empty_start_array.push_back(0); empty_start_array.push_back(0); empty_start_array.push_back(0);
        problem_matrix.appendCols(2,&empty_start_array[0],nullptr,nullptr);
        int delta_index = problem_matrix.getNumCols()-2;
        int sigma_0_index = problem_matrix.getNumCols()-1;

        // Problem matrix needs only new rows, no new cols, put in row_ordered mode
        problem_matrix.reverseOrdering();

        // Prepare the sigma rows! (in row_ordered fashion)
        indices.clear(); elements.clear(); starts.clear(); starts.push_back(0);
        // Tau_0 row
        const double* rhs = tree_nodes_by_stage_[0][0]->getDataPtr()->solver_rhs_;
        for (int i = 0; i < num_rows_first_stage; i++){
            indices.push_back(i); elements.push_back(rhs[i]);
        }
        indices.push_back(tau_0_0_index); elements.push_back(bar_nu_0);
        indices.push_back(delta_index); elements.push_back(-1.0);
        starts.push_back(starts.back()+num_rows_first_stage+2);
        // Tau_1 row
        for (int i = 0; i < num_rows_first_stage; i++){
            indices.push_back(i+num_rows_first_stage); elements.push_back(rhs[i]);
        }
        indices.push_back(tau_0_1_index); elements.push_back(bar_nu_1);
        indices.push_back(delta_index); elements.push_back(-1.0);
        starts.push_back(starts.back()+num_rows_first_stage+2);

        //// Add rows for \sigma_0 >= tau_0h: 2 rows
        indices.push_back(tau_0_0_index); elements.push_back(1.0);
        indices.push_back(sigma_0_index); elements.push_back(-1.0);
        starts.push_back(starts.back()+2);
        indices.push_back(tau_0_1_index); elements.push_back(1.0);
        indices.push_back(sigma_0_index); elements.push_back(-1.0);
        starts.push_back(starts.back()+2);
        //
        //// Add rows for \sum tau = 1
        indices.push_back(tau_0_0_index); elements.push_back(1.0);
        indices.push_back(tau_0_1_index); elements.push_back(1.0);
        starts.push_back(starts.back()+2);
        problem_matrix.appendRows(5,&starts[0],&indices[0],&elements[0]);

        int total_cols = 4+num_rows_first_stage*2+num_cols_first_stage;
        int total_rows = 5+num_cols_first_stage*2;
        assert( problem_matrix.getNumCols() == total_cols);
        assert( problem_matrix.getNumRows() == total_rows);
        // Now we need objective function values, bounds and rhs
        std::vector<double> bounds_lower(total_cols,-infinity_);
        std::vector<double> bounds_upper(total_cols,infinity_);
        for (int i = 0; i < 2*num_rows_first_stage; i++)
            bounds_lower[i] = 0;
        bounds_lower[tau_0_0_index] = 0;
        bounds_lower[tau_0_1_index] = 0;

        std::vector<double> obj_coefs(total_cols,0.0);
        obj_coefs[sigma_0_index] = -1.0;
        obj_coefs[delta_index] = 1.0;
        for (int i = sigma_index; i < sigma_index+num_cols_first_stage;i++)
            obj_coefs[i] = - tree_nodes_by_stage_[0][0]->getDataPtr()->col_solution_[i-sigma_index];

        std::vector<double> rhs_lower(total_rows,0.0);
        std::vector<double> rhs_upper(total_rows,0.0);

        for (int i = 0; i < 2*num_cols_first_stage; i++){// sigma rows
            rhs_lower[i] = -infinity_;
            rhs_upper[i] = 0;
        }
        // Other five rows:tau_0-delta, tau_1-delta, tau_0_1-sigma, tau_0_1-sigma, tau_0=1
        int row_index = 2*num_cols_first_stage;
        rhs_lower[row_index] = 0; rhs_upper[row_index] = infinity_; row_index++;
        rhs_lower[row_index] = 0; rhs_upper[row_index] = infinity_; row_index++;
        // tau-sigma
        rhs_lower[row_index] = -infinity_; rhs_upper[row_index] = 0; row_index++;
        rhs_lower[row_index] = -infinity_; rhs_upper[row_index] = 0; row_index++;
        // tau=1
        rhs_lower[row_index] = 1; rhs_upper[row_index] = 1; row_index++;
        assert(row_index==total_rows);

        // Load problem in solver
        OsiSolverInterface * osi_solver = GetSolverByThread();
        osi_solver->loadProblem(problem_matrix,&bounds_lower[0],&bounds_upper[0],&obj_coefs[0],&rhs_lower[0],&rhs_upper[0]);
        osi_solver->setObjSense(-1); // It is a maximization problem


        // Solve
        //console_mutex_.lock();
        //std::stringstream file_name;
        //file_name << "rhs-" << benders_tree_node_pointer->scenario() << ".lp";
        //osi_solver->writeLp(file_name.str().c_str());
        //console_mutex_.unlock();

        ////// Cplex specific cloning
        //OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        //if ( cpx_ptr ){
        //    // Get ENV and LP ptr
        //    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
        //    CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
        //    CPXwriteprob(env_ptr,lp_ptr,file_name.str().c_str(),"lp");
        //    //CPXchgobjsen(env_ptr,lp_ptr,CPX_MAX);
        //    CPXprimopt(env_ptr,lp_ptr);
        //    double objective_value = 0;
        //    CPXgetobjval(env_ptr,lp_ptr,&objective_value);
        //    std::vector<double> objective_solution(total_cols,0.0);
        //    CPXgetx(env_ptr,lp_ptr,&objective_solution[0],0,total_cols-1);
        //    LOG(INFO) << "Done.";
        //}

        osi_solver->initialSolve(); // Solve problem!
        if ( !osi_solver->isProvenOptimal() ){
            LOG(ERROR) << "RHS-SLP solve was not optimal.";
        }
        else {
            // Store relevant values for D^2 Cut Generation
            double * temp_solution = nullptr;
            StoreValuesFromSolverInArray(temp_solution,osi_solver->getColSolution(),osi_solver->getNumCols());
            const double * solution = temp_solution;
            double nu = solution[delta_index]/ solution[sigma_0_index];
            std::vector<int> gamma_indices(num_cols_first_stage);
            std::vector<double> gamma(num_cols_first_stage,0.0);
            for (int i = 0; i < num_cols_first_stage; i++){
                gamma[i] = solution[i+sigma_index]/solution[sigma_0_index];
                gamma_indices[i] = i;
            }
            // Generate cut coefficients!
            //double d_square_rhs = gamma;
            //for (int i = 0; i < num_cols_first_stage; i++){
            //    d_square_rhs -= gamma[i]*tree_nodes_by_stage_[0][0]->getDataPtr()->col_solution_[i];
            //}
            benders_tree_node_pointer->getDataPtr()->d_square_cut_rhs_.push_back(nu);
            if ( stage_data_[current_stage_]->num_d_square_cuts_ == 0 ){
                assert( benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_ == nullptr);
                benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_ = new CoinPackedMatrix();
                benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_->reverseOrdering(); // Now row-ordered
                benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_->setDimensions(0,num_cols_first_stage); // Now correct dimensions
            }
            benders_tree_node_pointer->getDataPtr()->d_square_cut_t_matrix_->appendRow(gamma.size(),&gamma_indices[0],&gamma[0]);
            assert(benders_tree_node_pointer->getDataPtr()->d_square_cut_rhs_.size()-1 == stage_data_[current_stage_]->num_d_square_cuts_);
            delete [] temp_solution;
        }

        // TODO: WarmStart-Handling
        start_time = CoinCpuTime()-start_time;
        misc_mutex_.lock();
        d_square_c_rhs_timer_ += start_time;
        misc_mutex_.unlock();
    }

    void ParallelNestedBenders::HandleInitialSolutionProcedures()
    {
        int num_cols_for_warm_up_solution = 0;
        for (int i = 0; i < num_stages_-1;i++)
            num_cols_for_warm_up_solution += stage_data_[i]->num_columns_;
        // Check if method is applicable!
        if ( warm_up_ && start_solution_ == PNB_INT_COMPUTE_EV_SOLUTION){
            for (int stage = 1; stage < num_stages_; stage++){
                if ( stage_data_[stage]->stochastic_matrix_w_ || stage_data_[stage]->stochastic_objective_ || stage_data_[stage]->stochastic_bounds_ )
                {
                    start_solution_ = PNB_INT_USE_EV_SOLUTION;
                    LOG(INFO) << "Problem has stochastic coefficents not only in RHS and T-matrix, so we can not use EV-problem cuts.";
                    break;
                }
            }
        }

        // Warm up period
        if ( warm_up_ && start_solution_ == PNB_INT_COMPUTE_EV_SOLUTION ){
            //// Get an initial solution via expected value problem. Works for problems with RHS and T-matrix stochasticity

            SmiScnModel* ev_smi = smi_->createEVSmiModel();
            //ev_smi->writeSmps("ev_problem",true);
            ParallelNestedBenders ev_pnb(ev_smi);
            // Use standard parameter stuff..
            Parameter param;
            param.init();
            param.parameter_.put(PNB_EPSILON_RELATIVE,0.1);
            param.parameter_.put(PNB_AGGREGATED_OPTIMALITY_CUTS,1); // Single cut (only have one scenario anyway..)
            param.parameter_.put(PNB_COMPUTE_PRIMAL_SOLUTIONS,num_stages_-2); // We need to apply the solution later, otherwise we have a problem!
            param.parameter_.put(PNB_USE_START_SOLUTION,PNB_INT_USE_SUPPLIED_SOLUTION);
            ev_pnb.parameter_ = param;
            // Solve EV first with deteq, then use this as a start solution for EV problem
            //ev_smi->setOsiSolverHandleWithoutCloning(osi_solver_);
            //ev_smi->loadOsiSolverData();
            //osi_solver_->initialSolve();
            //if ( osi_solver_->isProvenOptimal() ){
            //    // Set parallel benders solution
            //    ev_pnb.setWarmUpSolution(osi_solver_->getColSolution());
            //}
            ev_pnb.Init();
            ev_pnb.Solve();
            double * temp_ptr = new double[num_cols_for_warm_up_solution];
            for (int i = 0; i < num_stages_-1; i++)
                memcpy(temp_ptr+ (i>0?stage_data_[i-1]->num_columns_:0),&ev_pnb.tree_nodes_by_stage_[i][0]->getDataPtr()->incumbent_col_solution_[0],sizeof(double)*stage_data_[i]->num_columns_);
            warm_up_solution_.reset(temp_ptr); // Store column solution for warm up problem in scoped_ptr.
            // Copy cuts to this problem!
            ev_smi->releaseCore();
            ev_smi->releaseSolver();
            // Reenable osi_solver
            osi_solver_->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);

            // Copy the optimality cuts to this node
            for (int i = 0; i < num_stages_-1;i++){
                // copy cuts to all nodes // TODO for multi-stage
                int num_thetas = stage_data_[i]->num_thetas_;
                if ( tree_nodes_by_stage_[i][0]->numChildren() < num_thetas )
                    num_thetas =  tree_nodes_by_stage_[i][0]->numChildren();
                BendersNodeData::cut_container& cuts_to_add = ev_pnb.tree_nodes_by_stage_[i][0]->getDataPtr()->optimality_cuts_;
                // We need to add the cuts for all j stuff
                for (int j = 0; j < tree_nodes_by_stage_[i].size(); j++){
                    BendersNodeData::cut_container& insert_cuts_here = tree_nodes_by_stage_[i][j]->getDataPtr()->optimality_cuts_;
                    insert_cuts_here.insert(insert_cuts_here.end(),cuts_to_add.begin(),cuts_to_add.end());
                    tree_nodes_by_stage_[i][j]->getDataPtr()->last_iteration_start_optimality_ = insert_cuts_here.begin();
                    tree_nodes_by_stage_[i][j]->getDataPtr()->num_thetas_ = num_thetas;
                    createRoundRobinAggregation(tree_nodes_by_stage_[i][j]->getDataPtr(),tree_nodes_by_stage_[i][j]->numChildren(),tree_nodes_by_stage_[i][j]->getDataPtr()->num_thetas_,(int)std::ceil((double)tree_nodes_by_stage_[i][j]->numChildren()/tree_nodes_by_stage_[i][j]->getDataPtr()->num_thetas_));
                    // We have to transform the old optimality cuts to the new number of theta's we would like to have
                    // We already called a function that gives us a new number of whatever it is we need.

                    // Transform all old optimality cuts to new ones.
                    // Set new values

                    tree_nodes_by_stage_[i][j]->getDataPtr()->active_aggregates_ = boost::dynamic_bitset<>(tree_nodes_by_stage_[i][j]->getDataPtr()->num_thetas_); // Gets initialized with 0
                    tree_nodes_by_stage_[i][j]->getDataPtr()->was_under_threshold_ = boost::dynamic_bitset<>(tree_nodes_by_stage_[i][j]->getDataPtr()->num_thetas_); // Gets initialized with 0
                }
                for( BendersNodeData::cut_container::iterator it = cuts_to_add.begin(); it != cuts_to_add.end();++it){
                    ExpandThetaVariablesInOptimalityCut((*it)->cut_.get(),stage_data_[i]->num_columns_,num_thetas,1);
                    (*it)->aggregate_number_ = -1;
                    (*it)->not_active_iterations_ = 0;
                    (*it)->cut_creation_iteration_ = 0;
                }


            }
            delete ev_smi;
        }
        if ( warm_up_ && start_solution_ == PNB_INT_USE_SUPPLIED_SOLUTION && warm_up_solution_.get() == nullptr){
            LOG(WARNING) << "No startup solution supplied. Starting without initial solution.";
            warm_up_ = false; start_solution_ = 0;
        }
        if ( warm_up_ && start_solution_ == PNB_INT_USE_EV_SOLUTION ){
            SmiScnModel* ev_smi = smi_->createEVSmiModel();
            ev_smi->setOsiSolverHandleWithoutCloning(osi_solver_);
            ev_smi->loadStoredDataInSolver();
            SolveSolverInterface(osi_solver_);

            if ( osi_solver_->isProvenOptimal() ){
                double * temp_ptr = new double[num_cols_for_warm_up_solution];
                memcpy(temp_ptr,osi_solver_->getColSolution(),sizeof(double)*num_cols_for_warm_up_solution);
                warm_up_solution_.reset(temp_ptr); // Store column solution for warm up problem in scoped_ptr.
                LOG(INFO) << "Calculated EV Solution with value " << osi_solver_->getObjValue();
            }
            else {
                warm_up_ = false;
                start_solution_ = PNB_INT_NO_START_SOLUTION;
            }
            ev_smi->releaseSolver();
            ev_smi->releaseCore();
            osi_solver_->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr); // Reset osi_solver to get a clean first stage solver instance
            delete ev_smi;

        }
        if ( warm_up_ ){
            current_stage_ = num_stages_-1;
            for (int i = 0; i < num_cols_for_warm_up_solution;i++)
                if ( CompareWithZero(warm_up_solution_[i]))
                    warm_up_solution_[i] = 0;
            if ( regularization_ || level_decomp_ ){
                double * temp_array = new double[num_cols_for_warm_up_solution];
                memcpy(temp_array,warm_up_solution_.get(),sizeof(double)*num_cols_for_warm_up_solution);
                regularized_solution_.reset(temp_array);
            }
            if ( start_solution_ == PNB_INT_COMPUTE_EV_SOLUTION ){
                warm_up_ = false; // If this stays true, we set the first solution of the problem to the EV-solution
                current_stage_ = 0;
            }
        }
        else
            if ( regularization_ || level_decomp_)
                LOG(FATAL) << "No supplied start solution for regularization method. Please change your settings to allow an expected value solution for example.";

    }

    void ParallelNestedBenders::BuildAndSolveOnDemandAccuracyProblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer)
    {
        // What kind of problem do we want to build or solve here?
        //
        //  \tilde q_s (x) = max_{1 \le j \le current_iteration_} p_s u_{s,j} ( h_s - T_s x)
        //
        // That is equivalent to the reformulation, where we want to find the smallest z that is restricted from below via the optimality cuts..
        // = min z
        // z >= u_{s,j} ( h_s-T_s x )
        //
        // We need a free solver instance
        BendersNodeData * benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        bool use_lp = false;
        int first_stage_cols_with_theta = stage_data_[current_stage_-1]->num_columns_ + 1;
        std::vector<double>& first_stage_solution = benders_tree_node_pointer->getParent()->getDataPtr()->col_solution_;

        if ( use_lp ){
            // We need to build a new model consisting of num first stage variables (fixed to the current solution) plus num_theta aggregate variables
            OsiSolverInterface * osi_solver = GetSolverByThread();
            // If Solver Interface is non-empty we need to empty it first
            if ( osi_solver->getNumCols() > 0) // We could reuse it somehow? I do not know..
                osi_solver->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);


            // Create arrays for lb,ub (set to first stage solution), obj (only one 1, namely the aggregate variable aggregate_number), with no rows.
            std::vector<double> lower_bound(first_stage_cols_with_theta,0.0);
            std::vector<double> upper_bound(first_stage_cols_with_theta,0.0);
            std::vector<double> obj(first_stage_cols_with_theta,0.0);
            obj[first_stage_cols_with_theta-1] = 1.0; // We minimize by default..


            std::copy(first_stage_solution.begin(),first_stage_solution.end(),lower_bound.begin());
            std::copy(first_stage_solution.begin(),first_stage_solution.end(),upper_bound.begin());
            lower_bound[first_stage_cols_with_theta-1] = -infinity_;
            upper_bound[first_stage_cols_with_theta-1] = infinity_;

            // Append all the optimality cuts for this aggregate number
            osi_solver->loadProblem(first_stage_cols_with_theta,0,nullptr,nullptr,nullptr,&lower_bound[0],&upper_bound[0],&obj[0],nullptr,nullptr);
            // Iterate through opt-cut list!
            std::vector<std::shared_ptr<OsiRowCut> >::iterator it = benders_node_pointer->oda_optimality_cuts_.begin();
            while ( it != benders_node_pointer->oda_optimality_cuts_.end() ){
                osi_solver->applyRowCuts(1,(*it).get());
                ++it;
            }

            //console_mutex_.lock();
            //std::stringstream stringstream; 
            //stringstream << "oda_q_" << benders_tree_node_pointer->scenario();
            //osi_solver->writeLp(stringstream.str().c_str());
            //console_mutex_.unlock();

            // Solve problem
            osi_solver->initialSolve();

            // Problem has to be solved to optimality!
            assert(osi_solver->isProvenOptimal());

            double result = osi_solver->getObjValue();

            // Store solution value AND find out which set of dual variables is used to bound the aggregate variable. Try with dual variables!
            const double * dual_values = osi_solver->getRowPrice();
            int iteration_index = std::distance(dual_values,std::max_element(dual_values,dual_values+osi_solver->getNumRows()));

            oda_chosen_optimality_cut_by_scenario_[benders_tree_node_pointer->scenario()] = iteration_index;
            oda_current_tilde_q_s_[benders_tree_node_pointer->scenario()] = result;
            // Clean solver interface
            osi_solver->loadProblem(0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
        }
        else {
            double max_element = -infinity_;
            int max_index = -1;
            int count = 0;
            for ( std::vector<std::shared_ptr<OsiRowCut> >::iterator it = benders_node_pointer->oda_optimality_cuts_.begin(); it != benders_node_pointer->oda_optimality_cuts_.end(); ++it){
                double temp = EvaluateCutWithOneAggregate((*it).get(),&first_stage_solution[0]);
                if ( temp > max_element ){
                    max_element = temp;
                    max_index = count;
                }
                count++;
            }
            oda_chosen_optimality_cut_by_scenario_[benders_tree_node_pointer->scenario()] = max_index;
            oda_current_tilde_q_s_[benders_tree_node_pointer->scenario()] = max_element;
        }
    }

    void ParallelNestedBenders::PrintSubproblem( OsiSolverInterface* osi_solver, int scenario, const std::string& append )
    {
        if ( current_stage_ <= 0){ // Only write solution file if we are in the first stage..
            std::stringstream stream;
            stream << "stage" << current_stage_ << "_" << (forward_ ? "F_" : "B_") << current_iteration_ << "_" << scenario;
            if ( append != "")
                stream << "_" << append;
            OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
            if ( cpx_ptr ){
                CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
                std::unique_lock<std::mutex> lock(console_mutex_);
                std::string name = stream.str()+"lp";
                CPXwriteprob(env_ptr,lp_ptr,name.c_str(),"lp");
                name = stream.str()+"mps";
                //CPXwriteprob(env_ptr,lp_ptr,name.c_str(),"mps");
                //name = stream.str()+"sav";
                //CPXwriteprob(env_ptr,lp_ptr,name.c_str(),"sav");
                //console_mutex_.unlock();
            }
            //stream << "osi";
            //osi_solver->writeLp(stream.str().c_str());
        } // TODO: Debugging stuff

    }


    bool RelativeGapPresent(double upper_bound, double lower_bound, double infinity, double epsilon){
        if (upper_bound == infinity || lower_bound == -infinity)
            return true;
        if (std::abs(upper_bound) < 1 && std::abs(lower_bound) < 1) //in this case we do not want relative difference, instead we want absolute difference..
            return true;
        return std::abs(upper_bound-lower_bound)/(std::abs(lower_bound)+0.00000000001) >= epsilon; 
        // Cplex MIPGAP is |bestnode-bestinteger| / ( |bestinteger| + 0.0000000001 )
        //Gap as defined in POSTS test set (except 0.0..01 instead of 0.1)
    }

    RowOrderedSparseMatrixPointer CreateRowOrderedSparseMatrixPointer( SmiNodeData* smiNode )
    {
        if ( smiNode->hasMatrix() )
            return RowOrderedSparseMatrixPointer(smiNode->getElements(smiNode->getMatStart()),smiNode->getIndices(smiNode->getMatStart()),smiNode->getStarts(smiNode->getMatStart()));
        else
            return RowOrderedSparseMatrixPointer(nullptr,nullptr,nullptr);

    }

    bool CompareForEquality( double first, double second, double tolerance )
    {
        return std::abs(first-second)<= tolerance;
    }

    void GenerateDualRayProblem(OsiSolverInterface* osi_solver){
        // If we clone and do not have a solution, we get an error.
        const int numcols = osi_solver->getNumCols();
        const int numrows = osi_solver->getNumRows();
        int* index = new int[CoinMax(numcols,numrows)];
        int i;
        for ( i = CoinMax(numcols,numrows)-1; i >= 0; --i) {
            index[i] = i;
        }
        double* obj = new double[CoinMax(numcols,2*numrows)];
        CoinFillN(obj, numcols, 0.0);
        osi_solver->setObjCoeffSet(index, index+numcols, obj);

        double* clb = new double[2*numrows];
        double* cub = new double[2*numrows];

        const double plusone = 1.0;
        const double minusone = -1.0;
        const char* sense = osi_solver->getRowSense();

        const CoinPackedVectorBase** cols =
            new const CoinPackedVectorBase*[2*numrows];
        int newcols = 0;
        for (i = 0; i < numrows; ++i) {
            switch (sense[i]) {
      case 'L':
          cols[newcols++] =
              new CoinShallowPackedVector(1, &index[i], &minusone, false);
          break;
      case 'G':
          cols[newcols++] =
              new CoinShallowPackedVector(1, &index[i], &plusone, false);
          break;
      case 'R':
          cols[newcols++] =
              new CoinShallowPackedVector(1, &index[i], &minusone, false);
          cols[newcols++] =
              new CoinShallowPackedVector(1, &index[i], &plusone, false);
          break;
      case 'N':
          break;
      case 'E':	 
          cols[newcols++] =
              new CoinShallowPackedVector(1, &index[i], &minusone, false);
          cols[newcols++] =
              new CoinShallowPackedVector(1, &index[i], &plusone, false);
          break;
            }
        }

        CoinFillN(obj, newcols, 1.0);
        CoinFillN(clb, newcols, 0.0);
        CoinFillN(cub, newcols, osi_solver->getInfinity());

        osi_solver->addCols(newcols, cols, clb, cub, obj);
        delete[] index;
        for (int j = 0; j < newcols;j++)
            delete cols[j];
        delete[] cols;
        delete[] clb;
        delete[] cub;
        delete[] obj;
        osi_solver->setObjSense(1.0); // minimize
        osi_solver->initialSolve();
    }

    void ComputeDualSolutionForMipProblem( OsiSolverInterface* osi_solver, double* dual, double* reduced, double* col_solution)
    {
        // Cplex specific cloning
        OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        if ( cpx_ptr ){
            // Get ENV and LP ptr
            CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
            CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
            CPXchgprobtype(env_ptr,lp_ptr,CPXPROB_FIXEDMILP); 
            CPXlpopt(env_ptr,lp_ptr);
            // Store dual solution and reduced cost!
            CPXgetpi(env_ptr,lp_ptr,dual,0,CPXgetnumrows(env_ptr,lp_ptr)-1);
            CPXgetdj(env_ptr,lp_ptr,reduced,0,CPXgetnumcols(env_ptr,lp_ptr)-1);
            CPXchgprobtype(env_ptr,lp_ptr,CPXPROB_MILP); 
            //// Resolve MIP
            // CPXmipopt(env_ptr,lp_ptr);
            // CPXgetobjval(env_ptr,lp_ptr,&objective);
            // LOG(INFO) << "MIP: " << objective;
            // TODO: Ask for status codes
        }

    }

    double EvaluateCutWithOneAggregate( OsiRowCut* osi_cut, const double* col_solution )
    {
        assert(col_solution != nullptr);
        // Caller has to ensure that dimensions are correct!
        // Get RHS of cut
        // Get Row
        const CoinPackedVector& row(osi_cut->row());
        const double * elements = row.getElements();
        const int * indices = row.getIndices();
        int num_elements = row.getNumElements();
        double sum = 0.0;
        for (int i = 0; i < num_elements-1; i++){ // We do not need the aggregate index (which is by construction always the last one!
            sum += col_solution[indices[i]]*elements[i];
        }
        return osi_cut->lb()-sum;     
    }
} // end namespace parallel_benders
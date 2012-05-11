#include "ParallelNestedBenders.hpp"

#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <math.h>
#include <sstream>


#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/multi_array.hpp"
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include <glog/logging.h>
// Include all the other files

#include "CoinHelperFunctions.hpp"
#include "CoinPackedMatrix.hpp"
#include "CoinWarmStart.hpp"
#include "OsiRowCut.hpp"
#include "OsiSolverInterface.hpp"
#include "OsiCpxSolverInterface.hpp"
#include "SmiScnModel.hpp"
#include "SmiCoreCombineRule.hpp"
#include "CoinTime.hpp"

// Include files only necessary if we have cplex
#define CPLEX_PRESENT
#ifdef CPLEX_PRESENT
#include "ilcplex/cplex.h"
#include "ilcplex/cplexcheck.h"
#include "check.c"
#include "OsiCpxSolverInterface.hpp"
#endif


namespace parallel_benders {

    void ParallelNestedBenders::AdjustProblem()
    {
        num_stages_ = core_->getNumStages();
        num_scenarios_ = smi_->getNumScenarios(); // The leaf nodes do not work correctly!

        if ( bounds_in_matrix_ ){
            smi_->moveBoundsIntoMatrix();
            smi_->writeSmps("no_bounds",true,false);
            bounds_in_matrix_ = false;
        }
        // Check for integrality right away
        if ( core_->getIntegerLength() || core_->getBinaryLength()){ // Set boolean variable (For now)
            contains_integer_ = true;
            // Set stage wise integers
            // All binary are integer, but not all integer are binary
            const int* integer_indices = core_->getIntegerIndices();
            for (int i = 0; i < num_stages_;i++){
                if ( !core_->getIntCols(i).empty() )
                    integer_problems_[i] = true;
            } //TODO: No integer
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
                        if ( row_indices[k] < core_->getColStart(stage-1) ){ // This means, we have a non-staircase structure. Store all the indices in a std::map[]
                            non_staircase_row = true;
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
                    // Choose a mutex to use here TODO
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
        for ( int i = 0; i < tree_nodes_by_stage_[stage].size(); i++){
            // Look at current node
            BendersNodeData* benders_node_ptr = tree_nodes_by_stage_[stage][i]->getDataPtr();
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

    }

    ParallelNestedBenders::ParallelNestedBenders( SmiScnModel* smi_model_ptr ) :
    smi_(smi_model_ptr),core_(smi_->getCore()),osi_solver_(NULL),osi_solver_map_(),tree_(),tree_nodes_by_stage_(),stage_data_(),
        num_stages_(core_->getNumStages()),num_scenarios_(smi_->getNumScenarios()),infinity_(0),contains_integer_(false),integer_problems_(core_->getNumStages()),parameter_(),
        zero_tolerance_(0.0),integer_tolerance_(0.0),absolute_epsilon_(0.0),relative_epsilon_(0.0),
        current_iteration_(0),max_iteration_(10000),minor_iteration_(0),major_iteration_(0),feasible_iteration_(0),lower_bound_(-infinity_),upper_bound_(infinity_),temporary_upper_bound_(0),old_lower_bound_(-infinity_),
        current_stage_(0),non_staircase_problem_(false),forward_(true),infeasible_problem_(false),unbounded_problem_(false),infeasible_problems_(smi_->getNumScenarios()),added_optimality_cuts_(num_stages_,0),added_feasibility_cuts_(num_stages_,0),num_added_feasibility_cuts_(0),store_cut_information_(false),adaptive_(false),aggregation_scheme_(0),no_aggregation_(true),trust_region_(false),regularization_(false),regularized_solution_value_(infinity_),regularized_sigma_(1.0),regularized_gamma_(0.5),regularized_initialized_cols_(false),regularized_initialized_rows_(false),regularized_start_row_(0),start_solution_(0),timer_(num_stages_+1),wall_clock_solution_time_(1),detailed_timer_(4),maximal_solution_time_(0),
        num_cpu_(0),console_mutex_(),solver_map_mutex_(),stage_warm_start_mutex_(),upper_bound_mutex_(),infeasible_problem_mutex_(),osi_solver_mutex_(),add_cut_counter_mutex_(),stage_lower_upper_bounds_mutex_(),misc_mutex_(),thread_pool_(0),
        warm_start_(false),warm_start_scenarios_(false),warm_start_stage_(false),only_one_feasibility_cut_(true),generate_feasibility_and_optimality_cuts_(false),store_primal_and_dual_(false),
        sequencing_protocol_(0),second_iteration_stage_(1),strategy_epsilon_(0.0),repeat_second_iteration_(1),second_iteration_counter_(0),cascading_second_iteration_(false),current_cascading_stage_(1),
        stage_lower_upper_bounds_(),old_lower_upper_bounds_(),temporary_stage_lower_bound_(-infinity_),temporary_stage_upper_bound_(infinity_),compute_stage_bounds_(false),at_last_iteration_(false),cuts_added_last_round_(0),theta_approximations_everywhere_(false),enforce_direction_(false),enforce_stage_(0),
        warm_up_(false),warm_up_solution_(NULL),stage_aggregation_(false),begin_aggregation_stage_(0),end_aggregation_stage_(0),aggregate_from_start_(true),seperate_first_stage_solver_(true),seperate_stage_solver_(false),pretty_printing_(false),pretty_printing_verbose_(true),compute_primal_solutions_for_stage_(PNB_INT_SOLUTION_PRIMAL_NO),compute_dual_solutions_for_stage_(PNB_INT_SOLUTION_DUAL_NO),compute_objective_values_(false),first_stage_original_col_lower_(NULL),first_stage_original_col_upper_(NULL),fixed_solution_(NULL),fixed_first_stage_(false),non_staircase_coefficients_(),optimality_cut_counter_(0),intra_cut_aggregation_time_(0),adaptive_cut_time_(0)
    {

    }

    void ParallelNestedBenders::Init()
    {
        // Do not sync console output (makes no difference)
        //ios_base::sync_with_stdio(true);
        // Enable Buffering
        int buffer_size = 16384;
        setvbuf(stdout, NULL, _IOFBF, buffer_size); // Full Bufferung, IOLBF = Line Buffering, IONBF = No Buffering

        num_stages_ = core_->getNumStages();
        num_scenarios_ = smi_->getNumScenarios();
        integer_problems_ = boost::dynamic_bitset<>(num_stages_);
        infeasible_problems_ =  boost::dynamic_bitset<>(num_scenarios_);
        timer_.Start(num_stages_);
        // Initialize osiSolver
        if (smi_->getOsiSolverInterface() == NULL)
            osi_solver_ = new OsiCpxSolverInterface();
        else
            osi_solver_ = smi_->getOsiSolverInterface()->clone();
        // Load empty problem (i.e. reset the solver interface
        osi_solver_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
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
            compute_primal_solutions_for_stage_ = num_stages_;
        if ( compute_dual_solutions_for_stage_ == PNB_INT_SOLUTION_DUAL_ALL )
            compute_dual_solutions_for_stage_ = num_stages_;

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
        num_scen_pro_cut = param.get(PNB_SCENARIOS_PRO_CUT,0);

        // Adaptive aggregation
        adaptive_ = param.get(PNB_DYNAMIC_AGGREGATION,false);

        // Intra-Cut Aggregation
        store_cut_information_ = param.get(PNB_INTRA_CUT_AGGREGATION,false);

        // Warm Up Period
        start_solution_ = param.get(PNB_USE_START_SOLUTION,0);
        // Start_solution=0 Nothing
        if ( start_solution_ > PNB_INT_NO_START_SOLUTION)// At the moment we only have EV solution 
            warm_up_ = true;

        // Choice of algorithm
        int algorithm = param.get(PNB_BENDERS,1); // Use Benders as default
        if ( algorithm == PNB_INT_ALGORITHM_REGULARIZATION )
            regularization_ = true;

        // Trust Region
        trust_region_ = param.get(PNB_TRUST_REGION,false);

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
        second_iteration_stage_ = param.get(PNB_TREE_WALK_STRATEGY_STAGE,1);
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
        // Assign parameters for sequencing_protocol_ == PNB_INT_SECOND_ITERATION
        cascading_second_iteration_ = param.get(PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_CASCADE,true);
        repeat_second_iteration_ = param.get(PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_REPEAT,2);


        // 
        // Feasibility problems
        only_one_feasibility_cut_ = true;

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
                stage_data_.back()->warm_start_ = osi_solver_->getEmptyWarmStart();
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

        if ( compute_stage_bounds_ ){
            // Initialize all upper and lower bounds
            stage_lower_upper_bounds_.reserve(num_stages_);
            for (int i = 0; i < num_stages_;i++){
                stage_lower_upper_bounds_.push_back( std::make_pair(-infinity_,infinity_));
                old_lower_upper_bounds_.push_back( std::make_pair(-infinity_,infinity_));
            }
            stage_lower_upper_bounds_[num_stages_-1].first = 0; // We do not have another stage in the last one.
        }

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
        if ( trust_region_)
            store_primal_and_dual_ = true;

        build_problem_timer_ = 0;
        solve_problem_timer_ = 1;
        cut_generation_timer_ = 2;
        // Detailed_timer 

        // Set options for solver!
        SetSolverOptions(osi_solver_);
        // Unsilence first stage solver!
        //osi_solver_->messageHandler()->setLogLevel(1);

        // Initialize all benders node data objects with the correct cut aggregation policy. TODO
        timer_.Stop(num_stages_);
    }

    void ParallelNestedBenders::Solve()
    {
        if ( thread_pool_.size() == 0)
            return;
        int setup_timer = num_stages_;
        timer_.Start(setup_timer);
        AdjustProblem();
        // We can stop here, if we have integer variables on more than the second stage..
        if ( integer_problems_[0] ){ // In the first stage.
            LOG(INFO) << "Your problem contains first-stage integer variables.";
            //return;
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
        for (int i = 1; i < num_stages_;i++){
            LOG_IF(WARNING,integer_problems_[i]) << "Your problem contains integer variables at stage " << i+1 << ". We solve the LP relaxation for that stage.";
            integer_problems_[i] = false; // Ensure we do solve the LP relaxation!
        }


        // Stage Aggregation!
        if ( stage_aggregation_){
            //smi_->writeSmps("beforeStageAgg",true,false);
            smi_->aggregateStages(begin_aggregation_stage_,end_aggregation_stage_, aggregate_from_start_); // Aggregate stage 0 and 1
            smi_->writeSmps("stageAggreg",true,false);
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

        // Initialize solver instance for every thread?
        current_stage_ = 1;
        for ( int i = 0; i < num_cpu_*2; i++ ){
            // Schedule the tasks for this stage
            thread_pool_.schedule(boost::bind(&ParallelNestedBenders::GetSolverByThread,this));
        }
        thread_pool_.wait(0); // We wait until all threads are done
        current_stage_ = 0;

        // Check if we can really use the warm_up procedures
        //if ( warm_up_ ){
        //    for (int i = 1; i < num_stages_;i++)
        //        if ( stage_data_[i]->stochastic_objective_ || stage_data_[i]->stochastic_matrix_w_ ){
        //            if ( start_solution_ == PNB_INT_COMPUTE_EV_SOLUTION || start_solution_ == PNB_INT_USE_EV_SOLUTION ){
        //                start_solution_ = PNB_INT_NO_START_SOLUTION;
        //                warm_up_ = false;
        //                LOG(INFO) << "Can not use advanced start procedure due to problem structure.";
        //            }
        //        }
        //}


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
            param.parameter_.put(PNB_COMPUTE_PRIMAL_SOLUTIONS,0); // We need to apply the solution later, otherwise we have a problem!
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
            double * temp_ptr = new double[stage_data_[current_stage_]->num_columns_];
            memcpy(temp_ptr,ev_pnb.tree_nodes_by_stage_[0][0]->getDataPtr()->col_solution_,sizeof(double)*stage_data_[current_stage_]->num_columns_);
            warm_up_solution_.reset(temp_ptr); // Store column solution for warm up problem in scoped_ptr.
            // Copy cuts to this problem!
            ev_smi->releaseCore();
            ev_smi->releaseSolver();
            // Reenable osi_solver
            osi_solver_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

            // Copy the optimality cuts to this node
            for (int i = 0; i < num_stages_-1;i++){
                // copy cuts to all nodes // TODO for multi-stage
                int num_thetas = stage_data_[i]->num_thetas_;
                if ( tree_nodes_by_stage_[i][0]->numChildren() < num_thetas )
                    num_thetas =  tree_nodes_by_stage_[i][0]->numChildren();
                BendersNodeData::cut_container& cuts_to_add = ev_pnb.tree_nodes_by_stage_[i][0]->getDataPtr()->optimality_cuts_;
                BendersNodeData::cut_container& insert_cuts_here = tree_nodes_by_stage_[i][0]->getDataPtr()->optimality_cuts_;
                insert_cuts_here.insert(insert_cuts_here.end(),cuts_to_add.begin(),cuts_to_add.end());
                tree_nodes_by_stage_[i][0]->getDataPtr()->last_iteration_start_optimality_ = insert_cuts_here.end();
                tree_nodes_by_stage_[i][0]->getDataPtr()->num_thetas_ = num_thetas;
                createRoundRobinAggregation(tree_nodes_by_stage_[i][0]->getDataPtr(),tree_nodes_by_stage_[i][0]->numChildren(),tree_nodes_by_stage_[i][0]->getDataPtr()->num_thetas_,(int)std::ceil((double)tree_nodes_by_stage_[i][0]->numChildren()/tree_nodes_by_stage_[i][0]->getDataPtr()->num_thetas_));
                // We have to transform the old optimality cuts to the new number of theta's we would like to have
                // We already called a function that gives us a new number of whatever it is we need.

                // Transform all old optimality cuts to new ones.
                // Set new values

                tree_nodes_by_stage_[i][0]->getDataPtr()->active_aggregates_ = boost::dynamic_bitset<>(tree_nodes_by_stage_[i][0]->getDataPtr()->num_thetas_); // Gets initialized with 0
                tree_nodes_by_stage_[i][0]->getDataPtr()->was_under_threshold_ = boost::dynamic_bitset<>(tree_nodes_by_stage_[i][0]->getDataPtr()->num_thetas_); // Gets initialized with 0
                for( BendersNodeData::cut_container::iterator it = insert_cuts_here.begin(); it != insert_cuts_here.end();++it){
                    ExpandThetaVariablesInOptimalityCut((*it)->cut_.get(),stage_data_[i]->num_columns_,num_thetas,1);
                    (*it)->aggregate_number_ = -1;
                    (*it)->not_active_iterations_ = 0;
                    (*it)->cut_creation_iteration_ = 0;
                }
            }
            delete ev_smi;
        }
        if ( warm_up_ && start_solution_ == PNB_INT_USE_SUPPLIED_SOLUTION && warm_up_solution_.get() == NULL){
            LOG(WARNING) << "No startup solution supplied. Starting without initial solution.";
            warm_up_ = false; start_solution_ = 0;
        }
        if ( warm_up_ && start_solution_ == PNB_INT_USE_EV_SOLUTION ){
            SmiScnModel* ev_smi = smi_->createEVSmiModel();
            ev_smi->setOsiSolverHandleWithoutCloning(osi_solver_);
            ev_smi->loadOsiSolverData();
            SolveSolverInterface(osi_solver_);
            //TODO Remove write out stuff here!
            //ev_smi->writeSmps("ev_problem",true);
            //osi_solver_->writeLp("ev_problem");
            //osi_solver_->writeLp("ev_problem");
            if ( osi_solver_->isProvenOptimal() ){
                double * temp_ptr = new double[stage_data_[current_stage_]->num_columns_];
                memcpy(temp_ptr,osi_solver_->getColSolution(),sizeof(double)*stage_data_[current_stage_]->num_columns_);
                warm_up_solution_.reset(temp_ptr); // Store column solution for warm up problem in scoped_ptr.

                //std::pair<double,double*> ev_result = smi_->solveEV(osi_solver_);
                LOG(INFO) << "Calculated EV Solution with value " << osi_solver_->getObjValue();
            }
            else {
                warm_up_ = false;
                start_solution_ = PNB_INT_NO_START_SOLUTION;
            }
            ev_smi->releaseSolver();
            ev_smi->releaseCore();
            osi_solver_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL); // Reset osi_solver to get a clean first stage solver instance
            delete ev_smi;
           
        }
        if ( warm_up_ ){
            for (int i = 0; i < stage_data_[0]->num_columns_;i++)
                if ( CompareWithZero(warm_up_solution_[i]))
                    warm_up_solution_[i] = 0;
            if ( regularization_ ){
                double * temp_array = new double[stage_data_[0]->num_columns_];
                memcpy(temp_array,warm_up_solution_.get(),sizeof(double)*stage_data_[0]->num_columns_);
                regularized_solution_.reset(temp_array);
            }

            if ( start_solution_ == PNB_INT_COMPUTE_EV_SOLUTION )
                warm_up_ = false; // If this stays true, we set the first solution of the problem to the EV-solution
           
        }
        else
            if ( regularization_ )
                LOG(FATAL) << "No supplied start solution for regularization method. Please change your settings to allow an expected value solution for example.";


        if ( integer_problems_.any() ){
            //if ( warm_up_ ) {
            //    // No warm start!
            //    LOG(INFO) << "Disabling warm start procedure due to MIP";
            //    warm_up_ = false;
            //    start_solution_ = PNB_INT_NO_START_SOLUTION;
            //}
        }

        // Solve WS-problem
        bool ws_cut_ = false;
        double ws_value = 0;
        double ev_value = 0;
        if ( ws_cut_ ){
            ws_value = smi_->getWSValue(osi_solver_);
            //std::pair<double,double*> ev_result = smi_->solveEV(osi_solver_);

            osi_solver_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
            // Generate a WS-cut for first stage solution
            BendersNodeData * node = tree_nodes_by_stage_[0][0]->getDataPtr();
            // Get the first stage coefficients
            const double* coefs = core_->getDenseObjCoefficients(0); // Get Dense Obj Coefficients of first stage
            double * new_coefs = new double[stage_data_[0]->num_columns_+1];
            memcpy(new_coefs,coefs,sizeof(double)*stage_data_[0]->num_columns_);
            new_coefs[stage_data_[0]->num_columns_] = 1;
            //// Calculate results
            //double sum = 0;
            //for (int i = 0; i < stage_data_[0]->num_columns_; i++ ){
            //    sum += ev_result.second[i]*new_coefs[i];
            //}
            int * indices = new int[stage_data_[0]->num_columns_+1];
            memcpy(indices,&stage_data_[0]->column_indices_[0],sizeof(int)*stage_data_[0]->num_columns_);
            indices[stage_data_[0]->num_columns_] = stage_data_[0]->num_columns_;
            boost::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
            cut_pointer->setLb(ws_value);
            cut_pointer->setUb(infinity_);
            cut_pointer->setRow(stage_data_[0]->num_columns_+1,indices,new_coefs);
            boost::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
            node->optimality_cuts_.push_back(cut_stuff);
            // Set aggregate to active: We have a cut for this aggregate
            node->num_thetas_ = 1;
            node->active_aggregates_ = boost::dynamic_bitset<>(1);
            node->active_aggregates_[0] = true;
            node->was_under_threshold_ = boost::dynamic_bitset<>(1);
            node->was_under_threshold_[0] = false;
            //node->cut_information_.CreateCut(current_iteration_);
            // Care about the theta variable also..
            int num_childs = tree_nodes_by_stage_[0][0]->numChildren();
            int num_cuts = std::min(1,num_childs);
            // Write that back to stage_data? No, as number of childs can differ between nodes.
            // Num Subproblem per cuts
            int num_subproblems_per_cut = (int) std::ceil( ((double)num_childs)/num_cuts );
            createRoundRobinAggregation(node,num_childs,num_cuts,num_subproblems_per_cut);
        }






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
        if ( critical_stage > 0 )// If we have a critical stage that is not trivial, we say that we already have measured the time, i.e. do not set critical stage!
            time_not_measured = false;
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
                current_iteration_ < max_iteration_;
            else
            {
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
            if ( !not_stop_loop ){
                if (compute_primal_solutions_for_stage_ == PNB_INT_SOLUTION_PRIMAL_NO && compute_dual_solutions_for_stage_ == PNB_INT_SOLUTION_DUAL_NO && !compute_objective_values_)
                    break;
                // Do some things to ensure proper termination
                store_primal_and_dual_ = true;
                sequencing_protocol_ = PNB_INT_FASTPASS;
                if ( current_stage_ > compute_dual_solutions_for_stage_ && current_stage_ > compute_primal_solutions_for_stage_ && !compute_objective_values_)
                    break;
                if ( !forward_ && current_stage_ < num_stages_-1){
                    assert( current_stage_ == num_stages_-2);
                    break;
                }
            }
            wall_clock_solution_time_.Stop(0);


            // Check if we already went over the time limit
            if ( ((int) wall_clock_solution_time_.GetTime(0)) > maximal_solution_time_ ){
                LOG(ERROR) << "We stopped solving the problem after " << wall_clock_solution_time_.GetTime(0)/60 << " minutes.";
                return; // We stop solving the problem
            }
            wall_clock_solution_time_.Start(0);
            timer_.Start(current_stage_);
            if ( upper_bound_ < lower_bound_ ){
                if ( !regularization_)
                    LOG(INFO) << "Upper Bound is lower than lower Bound. This should not happen, unfortunately it has. Iteration: " << current_iteration_;
            }
            if ( old_lower_bound_ > (lower_bound_ + ParallelNestedBenders::zero_tolerance_) && old_lower_bound_ > -infinity_  && !infeasible_problem_){
                if ( !regularization_)
                    LOG(INFO) << "New Lower bound " << lower_bound_ << " is less than old lower bound " << old_lower_bound_;
            }
            if ( !infeasible_problem_ ){
                // Count if gap is similar
                if ( current_stage_ == 0 ){
                    old_gap = gap;
                    gap = abs(upper_bound_-lower_bound_)/abs(upper_bound_);
                    if ( regularization_ ){
                        double solution_qx = 0;
                        double ten_to_minus_ten = 1e-10;
                        for (int i = 0; i < num_stages_; i++)
                            solution_qx += stage_lower_upper_bounds_[i].second;
                        gap = std::abs(regularized_solution_value_-solution_qx)/( std::abs(regularized_solution_value_)+ten_to_minus_ten);
                    }
                    if (CompareForEquality(old_gap,gap,zero_tolerance_) )
                        gap_counter++;
                    else
                        gap_counter = 0;
                    if ( gap_counter > gap_counter_maximum ){
                        LOG(INFO) << "Algorithm stops as gap does not narrow anymore for " << gap_counter << " successive iterations.";
                        break;
                    }
                }
            }
            else
                gap_counter = 0;

            if ( current_stage_ == 0 ){ // Things to do if we are in the first stage
                current_iteration_++;
                minor_iteration_++;
            }
            if ( current_stage_ == num_stages_-1)// We only have a major iteration, if we are at the last stage!
                major_iteration_++;

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
                    if ( RelativeGapPresent(current_upper_bound,stage_lower_upper_bounds_[current_stage_].first,infinity_,strategy_epsilon_) ){// If true, values are not equal. 
                        forward_ = true; // We want to move forward again, with the newest cuts..
                        // Check for nearby values
                        if ( std::abs(old_lower_upper_bounds_[current_stage_].first-stage_lower_upper_bounds_[current_stage_].first) < absolute_epsilon_ 
                            && std::abs(old_lower_upper_bounds_[current_stage_].second-stage_lower_upper_bounds_[current_stage_].second) < absolute_epsilon_  ){
                                forward_ = false;
                        }
                    }
                    else {
                        // We have to check if old_lower_bound and so are the same. if yes, we must go forward to reach something new, don't we?
                    }
                }
            }

            // Change direction depending on bouncing strategy
            if ( sequencing_protocol_ == PNB_INT_SECOND_ITERATION && upper_bound_ < infinity_ && !infeasible_problem_){
                // Boolean to indicate we are in a second iteration. This can be manifested at the first stage.
                if ( in_second_iteration_ && forward_){
                    // Simple case: We reached second_iteration_stage
                    if ( current_stage_ == second_iteration_stage_ && current_stage_ > 0){// To prevent wrong specification of second_iteration_stage
                        forward_ = false;
                        in_second_iteration_ = false;
                        second_iteration_counter_++;
                        current_cascading_stage_ = 1; // Other values makes no sense, as we can at first cascade back when we reached stage 1
                        if ( second_iteration_counter_ < repeat_second_iteration_){
                            in_second_iteration_ = true;
                        }
                        else
                            second_iteration_counter_ = 0;
                    }
                    else if ( cascading_second_iteration_ ){
                        // We want cascading second iteration
                        if ( current_stage_ == current_cascading_stage_ ){
                            forward_ = false;
                            current_cascading_stage_++;
                        }
                    }
                }

                if ( current_stage_ == num_stages_-1 ){
                    assert( in_second_iteration_ == false); // Otherwise something is wrong. We do not need second_iteration in the last stage, makes no sense
                    in_second_iteration_ = true;
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

            /************************************************************************/
            /* Solve the subproblems                                                */
            /************************************************************************/
            int num_nodes = tree_nodes_by_stage_[current_stage_].size();
            for ( int i = 0; i < num_nodes; i++ ){
                // Schedule the tasks for this stage
                thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]));
            }
            thread_pool_.wait(0); // We wait until all threads are done

            if ( current_stage_ == num_stages_-1 && infeasible_problems_.none() ){// Update upper bound only, when we have no infeasible problems!
                theta_approximations_everywhere_ = true; // Only needed in case of FAST_BACKWARD and EPSILON_FAST_BACKWARD and FASTPASS_WITH_FASTBACK
                // This ensures that we get back to the first stage when we finally reached the last stage!
                enforce_stage_ = 0;
                enforce_direction_ = true;
                if ( !compute_stage_bounds_ && temporary_upper_bound_ < upper_bound_ )
                    upper_bound_ = temporary_upper_bound_;
            }

            // Update stage wise upper/lower bounds, only if we did not had infeasible problems.
            if ( !infeasible_problem_ && infeasible_problems_.none() && compute_stage_bounds_){
                old_lower_upper_bounds_[current_stage_].first = stage_lower_upper_bounds_[current_stage_].first;
                old_lower_upper_bounds_[current_stage_].second = stage_lower_upper_bounds_[current_stage_].second;
                // If we solved all problems without infeasibility, we update these values.
                stage_lower_upper_bounds_[current_stage_].first = temporary_stage_lower_bound_;
                stage_lower_upper_bounds_[current_stage_].second = temporary_stage_upper_bound_;
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

            // Update upper bound if we solved all problems in this iteration once
            // Lower bound is updated in the StoreData Method after solving first stage problem
            // This does only work for methods, that do not reiterate and use the stage_bounds.. Other methods update the upper_bound_ 
            // Has to be put here, because otherwise the stage_lower_upper_bounds array is not fully calculated!
            if ( current_stage_ == num_stages_ -1 && infeasible_problems_.none() ){
                if ( compute_stage_bounds_ ){
                    double sum_stage_solutions = 0;
                    for (int i = 0; i < num_stages_; i++)
                        sum_stage_solutions += stage_lower_upper_bounds_[i].second;
                    if ( upper_bound_ > sum_stage_solutions)
                        upper_bound_ = sum_stage_solutions;
                }
                if ( regularization_ ){
                    if ( regularized_solution_value_ >= infinity_ && upper_bound_ < infinity_ )
                        regularized_solution_value_ = upper_bound_;
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
                    std::cout << std::endl;
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
                        std::cout << "Current Incumbent solution " << regularized_solution_value_ << std::endl;
                        double current_solution = 0;
                        for (int i = 0; i < num_stages_; i++)
                            current_solution += stage_lower_upper_bounds_[i].second;
                        double regularized_tolerance = 1e-10;
                        std::cout << "Current Iteration solution " << current_solution << std::endl;
                        std::cout << "Relative Gap: " << abs(regularized_solution_value_-current_solution)/(abs(regularized_solution_value_)+regularized_tolerance ) << std::endl;
                        std::cout << "Lower Bound: " << lower_bound_ << std::endl;
                    }
                }
                else if ( current_stage_ == 0){
                    std::cout << "Lower Bound:  " << lower_bound_ << std::endl;
                    std::cout << "Upper Bound:  " << upper_bound_ << std::endl;
                    std::cout << "Relative Gap: " << abs(upper_bound_-lower_bound_)/abs(upper_bound_+1e-10)  << std::endl;

                }

            }

            // If we had unbounded problems, we have to stop the algorihtm.
            if ( unbounded_problem_ ){
                if ( infeasible_problems_.any() ){
                    unbounded_problem_ = false;
                }
                else {
                    LOG(ERROR) << "Your problem got unbounded in stage " << current_stage_+1 << ".";
                    break;
                }
            }

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
            }

            // If we have an infeasible subproblem, we already added the cuts to the corresponding parent problems.
            if ( infeasible_problems_.any() ){
                // Set boolean variable to indicate that during this backward run, we do not generate optimality cuts!.
                infeasible_problem_ = true;
                if ( current_stage_ == 0){
                    LOG(INFO) << "First stage problem is infeasible.";
                    // We have to stop the algorithm, our master problem is infeasible..
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
                id_solver_pair.second->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                // Workaround: Load empty problem. 
                assert(id_solver_pair.second->getNumCols() == 0);
            }


            if ( store_primal_and_dual_ && !not_stop_loop){ 
                // Depending on if we store primal and dual solution, we need to delete the memory allocated for its storage, to lower memory requirements! Only if we are not in the last run to compute solutions we want to return!.
                for (int i = 0; i < tree_nodes_by_stage_[current_stage_].size(); i++){
                    tree_nodes_by_stage_[current_stage_][i]->getDataPtr()->CleanUnusedMemory(forward_);
                }
            }
            stage_data_[current_stage_]->changed_aggregation_level_ = false; // Necessary for adaptive_ methods!

            timer_.Stop(current_stage_);
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
            // Sanity Check for direction!
            // Change next stage, depending on where we are heading (forward or backward)
            forward_ ? current_stage_++ : current_stage_--;
        }
        timer_.Stop(current_stage_);
        if ( pretty_printing_ )
            std::cout << std::endl;
        // Output the final values once again!
        std::cout << "Lower Bound:  " << lower_bound_ << std::endl;
        std::cout << "Upper Bound:  " << upper_bound_ << std::endl;
        std::cout << "Relative Gap: " << abs(upper_bound_-lower_bound_)/abs(upper_bound_+1e-10)  << std::endl;
        std::cout << "Major Iterations: " << major_iteration_ << std::endl;
        std::cout << "Overall Iterations: " << current_iteration_ << std::endl;
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
            upper_bound_ = -infinity_;
            LOG(INFO) << "Problem is infeasible.";
        }
        LOG_IF(INFO,store_cut_information_) << "Spent " << intra_cut_aggregation_time_ << " seconds to intra-aggregate cuts.";
        LOG_IF(INFO,adaptive_) << "Spent " << adaptive_cut_time_ << " seconds to aggregate aggregates.";
        LOG_IF(WARNING,current_iteration_ >= max_iteration_) << "Algorithm exceeded maximum number of iterations.";

    }


    void ParallelNestedBenders::HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Check if we already got infeasible subproblems only, if we generate either feas or opt cuts
        if ( !generate_feasibility_and_optimality_cuts_ && only_one_feasibility_cut_ )
            if ( infeasible_problems_.any() )
                return; 

        // Check if we got unbounded subproblems
        if ( unbounded_problem_ )
            return; // There is no need to actually solve other problems, as we have unbounded problems..

        OsiSolverInterface* osi_solver = GetSolverByThread();

        if ( !seperate_first_stage_solver_ || current_stage_ > 0 || (current_stage_ == 0 && current_iteration_ == 1))
            BuildSubproblem(osi_solver, benders_tree_node_pointer);
        AppendColsToSubproblem(osi_solver,benders_tree_node_pointer);
        AppendCutsToSubproblem(osi_solver,benders_tree_node_pointer);

        

        SolveSubproblem(osi_solver,benders_tree_node_pointer);
        //if ( regularization_ && current_stage_ == 0)
        //    BuildAndSolveRegularizedProblem(osi_solver,benders_tree_node_pointer);
        // If Level method
        // BuildAndSolveLevelProjectionProblem
        // If Regularized
        // BuildAndSolveRegularizedProblem
        
        // We only need to store data if we have a feasible solution for the subproblem.
        if ( !infeasible_problems_[benders_tree_node_pointer->scenario()]){
            StoreData(osi_solver,benders_tree_node_pointer);
            // Check for degeneracy and stuff, if we like it..
            if ( !forward_ || (store_primal_and_dual_ && current_stage_ > 0))
                GenerateOptimalityCut(benders_tree_node_pointer);
            if ( !benders_tree_node_pointer->getDataPtr()->unbounded_ && !integer_problems_[current_stage_]){ // Only if problem was not bounded by special row, we want to call these methods and only if we have true LPs, no MIPS
                if ( adaptive_ && (forward_ && current_stage_ < num_stages_-1 || current_stage_ == 0 ) ) // Analyse optimality cuts of this node!
                    AnalyseOptimalityCuts(osi_solver,benders_tree_node_pointer);
                if ( store_cut_information_ && (forward_ && current_stage_ < num_stages_-1 || current_stage_ == 0 ))
                    IntraIterationCutAggregation(osi_solver,benders_tree_node_pointer);
            }
        }
        DeleteSubproblem(osi_solver,benders_tree_node_pointer);
    }

        void ParallelNestedBenders::SolveSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer  ){
        // Warm Start Handling
        // Depending on Warm Start behaviour, use the corresponding entry
        // If WarmstartForScenario or WarmstartForStage do the following
        // 
        //if ( current_stage_ <= 0){ // Only write solution file if we are in the first stage..
        //    std::stringstream stream;
        //    stream << "stage" << current_stage_ << "_" << (forward_ ? "F_" : "B_") << current_iteration_ << "_" << benders_tree_node_pointer->scenario();
        //    console_mutex_.lock();
        //    osi_solver->writeLp(stream.str().c_str());
        //    console_mutex_.unlock();
        //    OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        //    if ( cpx_ptr ){
        //        CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
        //        CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
        //        stream << "_C";
        //        //CPXsetintparam(env_ptr,)
        //        CPXwriteprob(env_ptr,lp_ptr,stream.str().c_str(),"lp");
        //    }
        //} // TODO: Debugging stuff

        SetWarmStart(osi_solver,benders_tree_node_pointer);



        // Set the Trust Region
        if ( trust_region_ && current_stage_ < num_stages_-1)
            AppendTrustRegionToSubproblem(osi_solver,benders_tree_node_pointer);

        if ( regularization_ && current_stage_ == 0)
            BuildAndSolveRegularizedProblem(osi_solver,benders_tree_node_pointer);
        else
            SolveSolverInterface(osi_solver);

        // Result/Error Handling: Introduce a global variable, when feasibility problems occurs..
        // Check for errors
        if ( !osi_solver->isProvenOptimal() ){
            // We need to check for errors in this case
            if ( osi_solver->isAbandoned() ){
                //Problem
                console_mutex_.lock();
                LOG(FATAL) << "Solution process abandoned.";
                console_mutex_.unlock();
            }
            else if ( osi_solver->isProvenDualInfeasible() ){
                // Primal problem is unbounded. => dual infeasible
                // If we are in a problem with theta variables, we might have success in restricting the theta variables
                if ( current_stage_ < num_stages_-1){
                    // Put bound on theta variable via row, column bounds do not cut it ( do not know why )
                    int num_thetas = benders_tree_node_pointer->getDataPtr()->num_thetas_;
                    if ( num_thetas ){
                        boost::scoped_array<int> extra_indices(new int[num_thetas]);
                        boost::scoped_array<double> extra_elements(new double[num_thetas]);
                        for (int i = 0; i < num_thetas; i++){
                            extra_indices[i] = stage_data_[current_stage_]->num_columns_+i;
                            extra_elements[i] = 1;
                        }
                        // we bound all thetas with one extra row?
                        // We could also bound every theta itself ( or set variable bounds do not work right now.. ) 
                        osi_solver->addRow(num_thetas,extra_indices.get(),extra_elements.get(),-10000000000000, infinity_); // We assume that will work. I mean: Half infinity? 
                        // We need to remove this row again, when we finished everything with this problem!
                        benders_tree_node_pointer->getDataPtr()->unbounded_ = true;
                        // Solve the model again
                        SolveSolverInterface(osi_solver);
                    }
                }

                if ( osi_solver->isProvenPrimalInfeasible() ){
                    // Go below for feasibility cut
                }
                else if ( !osi_solver->isProvenOptimal()){
                    if ( osi_solver->isProvenDualInfeasible() ){
                        if ( seperate_first_stage_solver_ && current_stage_ == 0) // Strange behaviour may results if we use the seperate_first_stage_solver!
                            seperate_first_stage_solver_ = false;
                        console_mutex_.lock();
                        LOG(ERROR)<< "Problem for scenario " << benders_tree_node_pointer->scenario() << " in stage " << current_stage_ << " is primal unbounded.";
                        LOG(ERROR) << "This can point to an error in your model/data. This may also happen, when your original problem was meant as an maximization problem. Switching off seperate first stage solver.";
                        SolveSolverInterface(osi_solver);
                        if ( osi_solver->isProvenDualInfeasible() ){
                            // We can not work with that in a reasonable manner..
                            upper_bound_ = infinity_;
                            unbounded_problem_ = true; // If a problem is really unbounded, the whole problem is unbounded ( see paper VanSlyke & Wets )
                            osi_solver->writeLp("unboundedProblem");
                            osi_solver->writeMps("unboundedProblem");
                        }
                        console_mutex_.unlock();

                    }
                }
            }

            if ( osi_solver->isProvenPrimalInfeasible() ){
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
                    osi_solver->writeLp("master_infeasible");
                    LOG(ERROR) << "Your master problem is infeasible.";
                }

            }
            else if ( !osi_solver->isProvenOptimal() ){
                // Get cplex status
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    // Get ENV and LP ptr
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
                    int stat = CPXgetstat(env_ptr,lp_ptr);
                    LOG(INFO) << "Cplex returned solution process with " << stat;
                }

            }
            // TODO Other possible cases
            // Get the cplex stat..

        }// if not provenOptimal end
        else { // Proven Optimal: 
            StoreWarmStart(osi_solver,benders_tree_node_pointer);
        }
    }

    void ParallelNestedBenders::StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer ){
        // Store Column Solution
        // We need to copy the solution, as we erase the osiSolverInterface
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        // Delete old values // Memory leak, as we do not delete them at cleanup?
        delete [] benders_node_pointer->col_solution_;
        delete [] benders_node_pointer->dual_solution_;
        delete [] benders_node_pointer->reduced_cost_;
        delete [] benders_node_pointer->next_stage_approximation_;
        // Set to null
        benders_node_pointer->col_solution_ = NULL;
        benders_node_pointer->dual_solution_ = NULL;
        benders_node_pointer->reduced_cost_ = NULL;
        benders_node_pointer->next_stage_approximation_ = NULL;


        // Copy theta variables
        int num_theta = osi_solver->getNumCols()-stage_data_[current_stage_]->num_columns_;
        if ( trust_region_ )
            num_theta -= benders_node_pointer->num_trust_region_cols_; // We have to subtract the added trust_region variables!
        assert(num_theta >= 0);

        // Retrieve and store the theta variables
        if ( num_theta ){
            if ( regularization_ && current_stage_ == 0){
                // Where are the num_thetas?
                if ( num_theta == stage_data_[current_stage_]->num_columns_ )
                    num_theta = 0;
                if ( num_theta > stage_data_[current_stage_]->num_columns_)
                    num_theta -= stage_data_[current_stage_]->num_columns_;
                 StoreValuesFromSolverInArray(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,num_theta);
            }
            else 
                StoreValuesFromSolverInArray(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,num_theta);
        }

        // We need the column solution for the next stage problem!
        // We want to differentiate between column solution and theta variables 
        StoreValuesFromSolverInArray(benders_node_pointer->col_solution_,osi_solver->getColSolution(),stage_data_[current_stage_]->num_columns_);

        double solution_value = 0; // Compute the value of the c*x solution without all thetas!
        for (int i = 0; i < stage_data_[current_stage_]->num_columns_;i++){
            solution_value += benders_node_pointer->col_solution_[i]*osi_solver->getObjCoefficients()[i];
        }
        benders_node_pointer->solution_value_ = solution_value;

        if ( !forward_ || current_stage_ == 0 || compute_stage_bounds_ ){
            // Compute lower and upper bound estimates on the way back!
            double current_lower_bound = solution_value; 
            if ( compute_stage_bounds_ || current_stage_ == 0){ 
                for (int i = 0; i < num_theta; i++){ // Add all thetas to the c*x solution to get lower bound for node at this stage
                    current_lower_bound += benders_node_pointer->next_stage_approximation_[i];
                }
                current_lower_bound *= benders_node_pointer->probability_; 
                if ( benders_node_pointer->num_thetas_ == 0 && current_stage_ < num_stages_-1) // If no theta, set to -infinity, but if last stage we have no approximation, so leave it.
                    current_lower_bound = -infinity_;
            }
            if ( current_stage_ == 0){ // This works in the multithreaded cases, as we have only one thread for the first stage problem{
                // We are again at stage 0, test for new upper bound!
                old_lower_bound_ = lower_bound_; // Set old lower bound to last lower bound. If we added a feasibility cut lately, we have to take special care.
                if ( !warm_up_ )
                    lower_bound_ = current_lower_bound;
                if (num_theta == 0){// We have no thetas yet
                    lower_bound_ = -infinity_;
                    old_lower_bound_ = -infinity_;
                }
                temporary_upper_bound_ = 0;
            }
            upper_bound_mutex_.lock();
            if ( compute_stage_bounds_ ){
                temporary_stage_lower_bound_ += current_lower_bound;
                temporary_stage_upper_bound_ += solution_value*benders_node_pointer->probability_;
            }
            upper_bound_mutex_.unlock();
            // Delete column solution, if we do not need to store primal_and_dual solutions. 
            // We can delete the column solution only if we move backward and we do not need to store both!
        }
        if ( !compute_stage_bounds_ ){
            // If we go forward, we add to our upper bound!
            upper_bound_mutex_.lock();
            temporary_upper_bound_ += solution_value*benders_node_pointer->probability_; // Add current solution to overall temporary upper bound, but only on way back?
            upper_bound_mutex_.unlock();
        }



        // Store dual solutions
        if ( !forward_ || store_primal_and_dual_ ){
            // Store Dual Solution ( only if we move backward ) ( or we need it anyway )
            if ( integer_problems_[current_stage_] ){

            }
            else { // In case of non-integer problems
                StoreValuesFromSolverInArray(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),osi_solver->getNumRows());

                if ( current_stage_ > 0 ) {
                    // Compute the values: \lambda * lower_bound - \mu * upper_bound
                    // Compute the resulting values right away, as we have the osi_solver_interface at hand.
                    if ( stage_data_[current_stage_]->non_zero_bounds_ ){
                        // Store Reduced Cost ( needed for dual values of primal variable bounds )
                        StoreValuesFromSolverInArray(benders_node_pointer->reduced_cost_,osi_solver->getReducedCost(),osi_solver->getNumCols());
                        const double* dual_values_variable_bounds = benders_node_pointer->reduced_cost_;
                        const double* variable_lower_bounds = osi_solver->getColLower();
                        const double* variable_upper_bounds = osi_solver->getColUpper();
                        double computed_dual_value_for_variable_bounds = 0;
                        const double* column_solution = benders_node_pointer->col_solution_;
                        assert(column_solution!=NULL);
                        for (int i = 0; i< stage_data_[current_stage_]->num_columns_; i++){
                            // Look at variable value and their bounds
                            if ( !CompareWithZero(column_solution[i]) && !CompareWithZero(dual_values_variable_bounds[i]) ){ // If variable is zero, it is either at one of its bounds ( then we do not want to worry ) or it is basic and we also do not have to worry..

                                // We have to look at bounds and multiply with correct one. Reduced Costs = Dual logial variables
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
        }
        // If we go backward AND we computed all the dual values we need, we can delete the column solution.
        if ( !forward_ && !store_primal_and_dual_  ){
            delete [] benders_node_pointer->col_solution_;
            benders_node_pointer->col_solution_ = NULL;
        }

        // We got all the data stored, now we can remove TrustRegionConstraints!
        if ( trust_region_ ){
            // Just keep old trust region constraints!!
            //// Remove trust region constraints or rows from the problem!
            //if ( benders_node_pointer->num_trust_region_cols_ > 0)
            //    osi_solver->deleteCols(benders_node_pointer->num_trust_region_cols_,benders_node_pointer->trust_region_cols_.get());
            //if ( benders_node_pointer->num_trust_region_rows_ > 0)
            //    osi_solver->deleteRows(benders_node_pointer->num_trust_region_rows_,benders_node_pointer->trust_region_rows_.get());
            //if ( benders_node_pointer->num_trust_region_cols_  > 0 || benders_node_pointer->num_trust_region_rows_ > 0){
            //    // Store WarmStart!
            //    osi_solver->resolve();
            //    //osi_solver->writeLp("After_Trust_region");
            //    StoreWarmStart(osi_solver,benders_tree_node_pointer);
            //    benders_node_pointer->num_trust_region_cols_ = 0;
            //    benders_node_pointer->num_trust_region_rows_ = 0;
            //}
        }


        //if ( store_cut_information_ && !integer_problems_[current_stage_]){ // Store Cut information only for non-integer problems!
        //    // Store values about active cuts in own data structure
        //    for ( int i = stage_data_[current_stage_]->num_rows_; i < stage_data_[current_stage_]->num_rows_+benders_node_pointer->optimality_cuts_.size();i++ ){
        //        CutInformation& cut_information = benders_node_pointer->cut_information_.GetCut(i-stage_data_[current_stage_]->num_rows_);
        //        double cut_active_value = osi_solver->getRowPrice()[i];
        //        if ( cut_active_value ){ // Not Zero
        //            if ( forward_ )
        //                cut_information.iteration_forward_active_vector_[current_iteration_] = cut_active_value;
        //            else
        //                cut_information.iteration_backward_active_vector_[current_iteration_] = cut_active_value;
        //            cut_information.active_iteration_counter_++;
        //        }
        //        else
        //            cut_information.inactive_iteration_counter_++;
        //    }// End For
        //    // Print it out
        //    //  std::cout << "Node " << benders_node_pointer->scenario_index_  << std::endl;
        //    //if ( std::abs(osi_solver->getRowPrice()[i]) > zero_tolerance_)
        //    //    std::cout << "Cut " << i-stage_data_[current_stage_]->num_rows_ << " is active with " << osi_solver->getRowPrice()[i] << std::endl;
        //}// End if store_cut_information_
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

        // Do a quick matrix check, only if we do not reuse the solver..
        if ( !reuse_solver &&benders_node_pointer->matrix_->getMinorDim() == 0){
            benders_node_pointer->matrix_->reverseOrdering(); // We have an empty matrix, it is col ordered
            benders_node_pointer->matrix_->setDimensions(1,stage_pointer->num_columns_);
            // Insert at least empty entries for our columns
            for (int i = 0; i < stage_pointer->num_columns_; i++)
                benders_node_pointer->matrix_->modifyCoefficient(0,i,0,true);
        }

        // TODO: Insert appropriate boolean conditions to prevent unnecessary work for the 5 arrays to come
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
        benders_node_pointer->row_lower_ = new double[stage_pointer->num_rows_];
        memcpy(benders_node_pointer->row_lower_, core_->getDenseRowLower(current_stage_), stage_pointer->num_rows_*sizeof(double));
        if ( stage_pointer->stochastic_rhs_ ){
            for (int i = 0; i < node_pointer->getRowLowerLength(); i++) {
                assert(node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_) >= 0);
                assert(node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_) < stage_pointer->num_rows_);
                benders_node_pointer->row_lower_[node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowLowerElements()[i];
            }
        }

        benders_node_pointer->row_upper_ = new double[stage_pointer->num_rows_];
        memcpy(benders_node_pointer->row_upper_, core_->getDenseRowUpper(current_stage_), stage_pointer->num_rows_*sizeof(double));
        if  ( stage_pointer->stochastic_rhs_){
            for (int i = 0; i < node_pointer->getRowUpperLength(); i++) {
                benders_node_pointer->row_upper_[node_pointer->getRowUpperIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowUpperElements()[i];
            }
        }

        // Add solution from previous problem
        if ( current_stage_ > 0 ){
            // Get solution from predecessor in tree
            double* column_solution = benders_tree_node_pointer->getParent()->getDataPtr()->col_solution_; // Is of length num_columns of previous stage
            if ( column_solution != NULL){
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
                    if ( node_pointer->hasMatrix() )// We only need stochastic T matrix here
                        stochastic_element_in_row = stochastic_matrix.start_[row+1]-stochastic_matrix.start_[row] ? true : false;

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
                                stochastic_column_index++;
                            }

                            else {  // We assume Staircase-Problems. 
                                assert( deterministic_matrix.indices_[column_index]-previous_stage_columns < stage_data_[current_stage_-1]->num_columns_ && deterministic_matrix.indices_[column_index]-previous_stage_columns >= 0 );
                                value += deterministic_matrix.elements_[column_index]*column_solution[deterministic_matrix.indices_[column_index]-previous_stage_columns];
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
                }
            }
        }

        // Load the basic problem
        // We call this method if we just constructed the matrix for the first time
        if ( !reuse_solver ){
            osi_solver->loadProblem(*benders_node_pointer->matrix_,benders_node_pointer->column_lower_,benders_node_pointer->column_upper_,benders_node_pointer->objective_,benders_node_pointer->row_lower_,benders_node_pointer->row_upper_);
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
            row_bounds.reserve(stage_pointer->num_rows_*2);
            for ( int i = 0; i < stage_pointer->num_rows_; i++ ){
                row_bounds.push_back(benders_node_pointer->row_lower_[i]);
                row_bounds.push_back(benders_node_pointer->row_upper_[i]);
            }
            osi_solver->setRowSetBounds(&stage_pointer->row_indices_[0],&stage_pointer->row_indices_[stage_pointer->num_rows_],&row_bounds[0]);

            // Remove all cuts from previous solutions ( includes "Theta" Constraints )
            int cuts_to_remove = osi_solver->getNumRows()-stage_pointer->num_rows_;
            if ( cuts_to_remove ){
                std::vector<int> remove_indices;
                remove_indices.reserve(cuts_to_remove);
                for (int i = stage_pointer->num_rows_; i < stage_pointer->num_rows_+cuts_to_remove;i++)
                    remove_indices.push_back(i);
                osi_solver->deleteRows(cuts_to_remove,&remove_indices[0]);
            }
        }
        // We set the first stage warm_up solution, if we had one!
        if ( warm_up_  && current_stage_ == 0 ){
            for (int i = 0; i < stage_data_[0]->num_columns_; i++){
                osi_solver->setColBounds(i,warm_up_solution_[i],warm_up_solution_[i]);
            }
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
        double* row_lower = NULL; double * row_upper = NULL;
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
        const double* dual_solution = benders_node_pointer->dual_solution_;
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
        int for_bound = stage_pointer->num_rows_;
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


        // Append value for dual variable bounds
        cut_rhs += benders_node_pointer->dual_value_variable_bounds_;

        // Multiply with probability
        cut_rhs *= prob;

        // Store computed values in BendersNodeData
        benders_node_pointer->current_cut_coefficients_ = cut_coefficients;
        benders_node_pointer->current_cut_rhs_ = cut_rhs;

        // Cleanup
        if ( node_pointer->getRowLowerLength() )
            delete [] row_lower; 
        if (node_pointer->getRowUpperLength())
            delete [] row_upper;



        // We also remove the dual values, because we do not need them anymore.
        if ( !store_primal_and_dual_ ){
            delete [] benders_node_pointer->dual_solution_;
            benders_node_pointer->dual_solution_ = NULL;
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
            num_cuts = std::ceil(((double)num_childs)/num_subproblems_per_cut);
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
            //LOG(INFO) << "Changed aggregation Level :::::::::::::::::";
        }

        int num_columns = stage_data_[current_stage_-1]->num_columns_;
        // Do some magic to find a good partition for the number of cuts we want to produce: TODO

        int added_cuts = 0;
        bool initialized_iterator = !benders_tree_node_pointer->getDataPtr()->optimality_cuts_.empty();
        if ( initialized_iterator && seperate_first_stage_solver_ && current_stage_ == 1)
            benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ = --benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end();
        else if ( seperate_first_stage_solver_ && current_stage_ == 1){
            benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ = benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end();
        }

        // Analyse thetas 
        if ( benders_tree_node_pointer->getDataPtr()->next_stage_approximation_ && !forward_ && !benders_tree_node_pointer->getDataPtr()->optimality_cuts_.empty()){
            double minimum = infinity_;
            double maximum = -infinity_;
            double average = 0;
            if ( benders_tree_node_pointer->getDataPtr()->discrepancy_ != NULL)
                delete [] benders_tree_node_pointer->getDataPtr()->discrepancy_;
            benders_tree_node_pointer->getDataPtr()->discrepancy_ = new double[benders_tree_node_pointer->getDataPtr()->num_thetas_];
            for (int i = 0; i < num_cuts; i++){

                //LOG(INFO) << "Approximation of node " << benders_tree_node_pointer->scenario() << " for child nodes is : " << benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i];
                std::vector<int>& nodes_in_cut(benders_tree_node_pointer->getDataPtr()->nodes_in_cut_[i]);
                double real_value = 0;

                for ( int j = 0; j < nodes_in_cut.size(); j++){
                    real_value += (tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->probability_ / benders_tree_node_pointer->getDataPtr()->probability_) * tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->solution_value_;
                }
                benders_tree_node_pointer->getDataPtr()->discrepancy_[i] = real_value-benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i];
                if (CompareWithZero(benders_tree_node_pointer->getDataPtr()->discrepancy_[i]) )
                    benders_tree_node_pointer->getDataPtr()->discrepancy_[i] = 0;
                //LOG(INFO) << " Recourse Function value is : " << real_value;
                minimum = std::min<double>(minimum,real_value-benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i]);
                maximum = std::max<double>(maximum,real_value-benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i]);
                average += benders_tree_node_pointer->getDataPtr()->discrepancy_[i];

            }


            if ( CompareWithZero(minimum))
                minimum = 0;
            if ( CompareWithZero(maximum))
                maximum = 0;
            benders_tree_node_pointer->getDataPtr()->min_discrepancy_ = minimum;
            benders_tree_node_pointer->getDataPtr()->max_discrepancy_ = maximum;
            benders_tree_node_pointer->getDataPtr()->average_discrepancy_ = average/num_cuts;
            benders_tree_node_pointer->getDataPtr()->discrepancy_value_ = std::accumulate(&benders_tree_node_pointer->getDataPtr()->discrepancy_[0],&benders_tree_node_pointer->getDataPtr()->discrepancy_[num_cuts],0.0);
            if ( upper_bound_ != infinity_ && lower_bound_ != -infinity_)
                benders_tree_node_pointer->getDataPtr()->average_gap_per_aggregate_ = (upper_bound_-lower_bound_)/num_cuts;
            //LOG(INFO) << "Discrepancy is : " <<  benders_tree_node_pointer->getDataPtr()->discrepancy_value_;
            if ( benders_tree_node_pointer->getDataPtr()->discrepancy_lower_bound_ > benders_tree_node_pointer->getDataPtr()->discrepancy_value_ )
                benders_tree_node_pointer->getDataPtr()->discrepancy_lower_bound_ = benders_tree_node_pointer->getDataPtr()->discrepancy_value_;
            if ( benders_tree_node_pointer->getDataPtr()->average_lower_bound_ > benders_tree_node_pointer->getDataPtr()->average_discrepancy_ )
                benders_tree_node_pointer->getDataPtr()->average_lower_bound_ = benders_tree_node_pointer->getDataPtr()->average_discrepancy_;
            if ( upper_bound_ != infinity_ && lower_bound_ != -infinity_)
                if ( benders_tree_node_pointer->getDataPtr()->average_gap_per_aggregate_lower_bound_ > benders_tree_node_pointer->getDataPtr()->average_gap_per_aggregate_)
                    benders_tree_node_pointer->getDataPtr()->average_gap_per_aggregate_lower_bound_ = (upper_bound_-lower_bound_)/num_cuts;

        }
        else {
            benders_tree_node_pointer->getDataPtr()->min_discrepancy_ = 0;
            benders_tree_node_pointer->getDataPtr()->max_discrepancy_ = 0;
            benders_tree_node_pointer->getDataPtr()->average_discrepancy_ = 0;
            delete [] benders_tree_node_pointer->getDataPtr()->discrepancy_;
            benders_tree_node_pointer->getDataPtr()->discrepancy_ = NULL;
        }


        bool added_iterator = false;
        for ( int i = 0; i < num_cuts; i++){
            // Start a new OsiRowCut
            double cut_lower_bound = 0;
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
            const double * column_solution = benders_tree_node_pointer->getDataPtr()->col_solution_;
            double sum = 0.0;
            for ( int index = 0;  index < num_columns;  index++ ) { // We iterate over column_solution, but not over the theta index, which is always the last one.
                int column_index = cut_indices[index];
                sum += column_solution[column_index] * cut_elements[index];
            }
            // Add computed cut right hand side
            sum = cut_lower_bound-sum;
            // Check with previous stuff only, if we did not changed the aggregation level. Because if we changed we do not have the data yet to check for validity of new cut
            // If solution value of problem is greater than approximation we have, we need to add a new cut, otherwise we don't, but it does not harm to do so (except solution time..)

            /************************************************************************/
            /* If-Condition can be put here, if the cut should be added, instead of just set to to_use_ = false/true */
            /************************************************************************/

            //if ( current_iteration_ >= 44 )
            //    LOG(INFO) << "Break here!.";
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
            boost::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
            cut_pointer->setLb(cut_lower_bound);
            cut_pointer->setUb(infinity_);
            cut_pointer->setRow(new_index,&cut_indices[0],&cut_elements[0]);
            boost::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
            cut_stuff->aggregate_number_ = i;
            if ( change_aggregation_level || sum > (benders_tree_node_pointer->getDataPtr()->num_thetas_ ? benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i] : 0) ){
                cut_stuff->to_use_ = true;
                added_cuts++;
            }
            else
                cut_stuff->to_use_ = false;

            if ( cut_stuff->to_use_ || adaptive_ || store_cut_information_) // We will add the cut in the case where it is usable (defined above) or where we might need it later one )
                benders_tree_node_pointer->getDataPtr()->optimality_cuts_.push_back(cut_stuff);

            // Set aggregate to active: We have a cut for this aggregate
            benders_tree_node_pointer->getDataPtr()->active_aggregates_[i] = true;

            // Store information about this new cut, we added a new one to our set!
            //benders_tree_node_pointer->getDataPtr()->cut_information_.CreateCut(current_iteration_);
            /************************************************************************/
            /* End If-Condition here                                                */
            /************************************************************************/

        }// End for num_cuts

        // Add number of added cuts.
        //if ( added_cuts && seperate_first_stage_solver_ && current_stage_ == 1){
        //    if ( initialized_iterator ) 
        //        ++benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_; // Start at the correct one ( points to last of previous iteration before ++operation )
        //    else
        //        benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ = benders_tree_node_pointer->getDataPtr()->optimality_cuts_.begin();
        //}

        add_cut_counter_mutex_.lock();
        added_optimality_cuts_[current_stage_-1] += added_cuts;
        add_cut_counter_mutex_.unlock();

    }

    void ParallelNestedBenders::GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    { // Called directly from solve subproblem.
        // We need the dual rays..
        assert(osi_solver->isProvenPrimalInfeasible()); // Otherwise we do not need to call this method
        bool use_cplex = false;
        double * dual_solution_farkas = NULL; // We need to delete this, if we use dual_farkas, else not
        double * dual_solution_primal_variable_bounds = NULL;
        const double * dual_solution = NULL;
        int number_rows = osi_solver->getNumRows(); // Also account for optimality and feasibility cuts
        int number_coss = stage_data_[current_stage_]->num_columns_;//osi_solver->getNumCols(); Ignore next stage approximations

        const double* variable_lower_bounds = NULL;
        const double* variable_upper_bounds = NULL;
        const double* dual_values_variable_bounds = NULL; // We set this either in the dual_farkas section or via the traditional way

        double computed_dual_value_for_variable_bounds = 0;



#ifdef CPLEX_PRESENT
        use_cplex = true; //TODO: change this
        bool use_dual_farkas = true;
        if ( feas_cut_lp_method_ == PNB_INT_PRIMAL_SIMPLEX )
            use_dual_farkas = false;
        else
            use_dual_farkas = true;

        // Now we try to solve the same stuff with dual farkas: Primal problem is infeasible, we use the dual simplex, so it should work fine.
        // See for example https://www-304.ibm.com/support/docview.wss?uid=swg21400058
        OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
        if ( cpx_ptr ){
            // Get ENV and LP ptr
            CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
            CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
            int cpx_stat = 0;

            dual_solution_farkas = new double[number_rows];

            CPXchgobjsen(env_ptr, lp_ptr, CPX_MAX);
            CPXchgobjsen(env_ptr, lp_ptr, CPX_MIN);
            CPXsetintparam( env_ptr, CPX_PARAM_PREIND, CPX_OFF );
            if ( stage_data_[current_stage_]->non_zero_bounds_ && !use_dual_farkas)
                CPXprimopt(env_ptr,lp_ptr); // I need primopt to obtain the reduced costs of the phase 1 problem ( CPXgetdj ) and the dual ray via CPXgetpi
            else
                CPXdualopt(env_ptr,lp_ptr);// Use CPXdualfarkas for dual ray, but nothing about the reduced costs yet..

            cpx_stat = CPXgetstat(env_ptr,lp_ptr);
            double proof = 0;
            if ( cpx_stat == CPX_STAT_INFEASIBLE ){
                if ( stage_data_[current_stage_]->non_zero_bounds_ && !use_dual_farkas){
                    cpx_stat = CPXgetpi(env_ptr,lp_ptr,dual_solution_farkas,0,number_rows-1);// If we use the primal method, we get the dual ray via the dual variables
                    if ( cpx_stat != 0){
                        //Routine was unsuccesful, we need to compute the values by the normal method
                        LOG_IF(INFO,cpx_stat) << "CPXgetpi : " << cpx_stat;
                    }
                }
                else {
                    cpx_stat = CPXdualfarkas(env_ptr,lp_ptr,dual_solution_farkas,&proof);
                    LOG_IF(INFO,cpx_stat)<< "CPXdualfarks : " << cpx_stat;
                    if ( cpx_stat != 0 )// If CPXdualfarkas returns a value that is not zero, it was unsuccessful, so we need to compute the dual ray the normal way
                        use_cplex = false;
                    else if ( stage_data_[current_stage_]->non_zero_bounds_ ){ // We need to compute the dual values for the bounds! Where do we get the values? Link blogpost by OR in an OB World
                        // q^T = y^T*A 
                        // We can compute this with the column based matrices
                        variable_lower_bounds = benders_tree_node_pointer->getDataPtr()->column_lower_;
                        variable_upper_bounds = benders_tree_node_pointer->getDataPtr()->column_upper_;
                        const CoinPackedMatrix* matrix = NULL;
                        //if ( benders_tree_node_pointer->getDataPtr()->matrix_ == NULL ){ // Do we reuse a problem or not?
                        //    matrix = osi_solver->getMatrixByCol(); // This matrix is column ordered
                        //}
                        //else {
                        //    benders_tree_node_pointer->getDataPtr()->matrix_->transpose();
                        //    benders_tree_node_pointer->getDataPtr()->matrix_->reverseOrdering(); // Maybe it is faster to get the matrix from cplex..
                        //    matrix = benders_tree_node_pointer->getDataPtr()->matrix_;
                        //}
                        // TODO: test something
                        matrix = osi_solver->getMatrixByCol(); // This matrix contains also feasibility/optimality cuts..

                        dual_solution_primal_variable_bounds = new double[number_coss];
                        const double* objective_coefficients = benders_tree_node_pointer->getDataPtr()->objective_;
                        for (int i = 0; i < number_coss; i++){
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
                }// End Else
            }//End If infeasible
            else {
                // Print out the stat
                LOG(INFO) << "Solver returned with status code : " << cpx_stat << " while solving the original problem again with primal or dual simplex.";
                use_cplex = false;
            }

            if ( stage_data_[current_stage_]->non_zero_bounds_ && !use_dual_farkas){ // If we use the primal simplex to obtain the multipliers, we take the reduced costs = dual logicals
                dual_solution_primal_variable_bounds = new double[number_coss];
                cpx_stat = CPXgetdj(env_ptr,lp_ptr,dual_solution_primal_variable_bounds,0,number_coss-1);
            }

            // Compute the value of the primal variable bounds * dual multipliers
            if ( stage_data_[current_stage_]->non_zero_bounds_ ) {
                variable_lower_bounds = osi_solver->getColLower();
                variable_upper_bounds = osi_solver->getColUpper();
                if ( stage_data_[current_stage_]->non_zero_bounds_)
                    for (int i = 0; i < number_coss; i++){
                        if ( variable_lower_bounds[i] && variable_lower_bounds[i] > -infinity_ && dual_solution_primal_variable_bounds[i] > 0 ){ // This means, we have a variable lower bound not zero and the bound is breached below
                            computed_dual_value_for_variable_bounds += dual_solution_primal_variable_bounds[i]*variable_lower_bounds[i];
                        }
                        else if ( variable_upper_bounds[i] && variable_upper_bounds[i] < infinity_ && dual_solution_primal_variable_bounds[i] < 0){ // Variable has non-zero upper bound and the current variable value is above
                            computed_dual_value_for_variable_bounds += dual_solution_primal_variable_bounds[i]*variable_upper_bounds[i];
                        }
                    }
                    delete [] dual_solution_primal_variable_bounds;
                    dual_solution_primal_variable_bounds = NULL;
            }


        }
#endif // CPLEX_PRESENT
        bool cplex_licence_problem = true;
        OsiSolverInterface* dual_ray_problem = NULL;
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
        double* checked_dual_solution = new double[number_rows];
        if ( use_cplex ){
            memcpy(checked_dual_solution,dual_solution_farkas,sizeof(double)*number_rows);
            delete dual_solution_farkas;
        }
        else
            memcpy(checked_dual_solution,dual_solution,sizeof(double)*number_rows);

        for ( int i = 0; i < number_rows; i++)
            if ( CompareWithZero(checked_dual_solution[i]) )
                checked_dual_solution[i] = 0;

        ComputeCutCoefficients(benders_tree_node_pointer,checked_dual_solution,&cut_coefficients);

        // Compute fixed value for RHS

        // Compute the values: \lambda * lower_bound - \mu * upper_bound
        // Compute the resulting values right away, as we have the osi_solver_interface at hand.

        if ( !use_cplex ){ 
            dual_values_variable_bounds = const_cast<double*> (dual_ray_problem->getReducedCost());
            variable_lower_bounds = osi_solver->getColLower();
            variable_upper_bounds = osi_solver->getColUpper();
            computed_dual_value_for_variable_bounds = 0;
            const double* column_solution = dual_ray_problem->getColSolution(); // We need to check if the column solution is at one of the bounds..

            for (int i = 0; i< stage_data_[current_stage_]->num_columns_; i++){
                // Look at variable value and their bounds
                if ( !CompareWithZero(column_solution[i]) && !CompareWithZero(dual_values_variable_bounds[i]) ){ // If variable is zero, it is either at one of its bounds ( then we do not want to worry ) or it is basic and we also do not have to worry..
                    std::cout << "var " << i << " has reduced cost of " << dual_values_variable_bounds[i] << std::endl;
                    if ( CompareForEquality( column_solution[i], variable_lower_bounds[i],zero_tolerance_) )
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_lower_bounds[i];
                    else if ( CompareForEquality( column_solution[i], variable_upper_bounds[i],zero_tolerance_))
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_upper_bounds[i]; 
                }
            }
        }
        // Compute the cut coefficients for the RHS.
        // We compute prob* \pi * T + \theta >= prob*\pi*h (+ prob*dual_value_variable_bounds)
        double* row_lower = NULL; double * row_upper = NULL;
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

        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
            if ( (*it)->to_use_ ) {
                LOG_IF(INFO,checked_dual_solution[row] < 0) << "Problem at cut with row " << row << " has dual value " << checked_dual_solution[row] << " writing problem.";
                if ( checked_dual_solution[row] < 0) 
                    osi_solver->writeLp("murks");
                assert( checked_dual_solution[row] >= 0 || CompareWithZero(checked_dual_solution[row]));
                cut_rhs += (*it)->cut_->lb()*checked_dual_solution[row];
                row++;
            }
        }
        // Append value for dual variable bounds
        cut_rhs += computed_dual_value_for_variable_bounds;

        // Add cut to set of feasibility cuts of parent problem
        boost::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
        cut_pointer->setLb(cut_rhs);
        cut_pointer->setUb(infinity_);
        cut_pointer->setRow(stage_data_[current_stage_-1]->num_columns_,&stage_data_[current_stage_-1]->column_indices_[0],&cut_coefficients[0]);
        // Increase feasCutCounter, together with locking/unlocking a mutex, and adding the cut the the parent problem
        boost::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
        infeasible_problem_mutex_.lock();
        // Check if this is the first cut we added
        num_added_feasibility_cuts_++;

        benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.push_back(cut_stuff);
        if ( num_added_feasibility_cuts_ == 1 ){
            benders_tree_node_pointer->getParent()->getDataPtr()->last_iteration_start_feasibility_ = --benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.end();
        }
        if ( store_cut_information_ || adaptive_ ){
            seperate_first_stage_solver_ = false;
            LOG(INFO) << "First stage solver is no longer a seperate instance, but build each time. This is due to feasibility cuts.";
        }
        // Why do we want to not longer allow a seperate_first_stage_solver_ when we add feasibility cuts? Wrong order of cuts? That may well be the case..

        infeasible_problem_mutex_.unlock();

        // Delete dual ray problem
        if ( !cplex_licence_problem && !use_cplex)
            delete dual_ray_problem;
        else if ( !use_cplex ){
            // Reset the solver interface
            dual_ray_problem->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        }

        // Delete the computed lower and upper bounds
        if ( node_pointer->getRowLowerLength())
            delete row_lower;
        if ( node_pointer->getRowUpperLength())
            delete row_upper;

        delete [] checked_dual_solution;
        delete [] dual_solution_primal_variable_bounds;
    }

    OsiSolverInterface* ParallelNestedBenders::GetSolverByThread(){
        // We assign the used solver here.
        if ( seperate_first_stage_solver_ && current_stage_ == 0 ){
            return osi_solver_; // We do not clear this solver. We ALWAYS reuse it.
        }
        boost::unique_lock<boost::mutex> lock(solver_map_mutex_); // TODO: Remove the mutex and initialize the solver in the init method.
        OsiSolverInterface* osi_solver = osi_solver_map_[boost::this_thread::get_id()];
        if ( osi_solver == NULL ){
            // Create new osi_solver object from sepearete first stage solver? if we have a mip
            if ( seperate_first_stage_solver_ && integer_problems_[0] ){ // If we have a seperate first stage solver AND a MIP, we need to get rid of the mip first before we clone!
                // If we call this method we are not in the first stage.
                osi_solver_->initialSolve(); // Get rid of the integers!
            }
            osi_solver = osi_solver_->clone(false); // The boolean does not alway do what it is supposed to do.: We can only clone it if it is a LP problem
            // Clean the solver object by loading an empty problem ( that works for cplex, reset crashes cplex)
            osi_solver->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
            // We want to emphasize numerical stability / for CPLEX
            SetSolverOptions(osi_solver);
            // Set primal algorithm
            osi_solver_map_[boost::this_thread::get_id()] = osi_solver;

        }
        return osi_solver;
    }

    void ParallelNestedBenders::ComputeCutCoefficients( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, const double* dual_solution, std::vector<double>* cut_coefficients_pointer )
    {
        assert( dual_solution != NULL);
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
                stochastic_element_in_row = stochastic_matrix.start_[row+1]-stochastic_matrix.start_[row] ? true : false;

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

                if ( deterministic_matrix.indices_[column_index] < current_stage_columns){
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
    }

    bool ParallelNestedBenders::CompareWithZero( double value ) 
    {
        return std::abs(value) < zero_tolerance_; // True, if zero
    }

    void ParallelNestedBenders::AppendCutsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        if ( !benders_node_pointer->feasibility_cuts_.empty()){
            // Add the cuts
            BendersNodeData::cut_container::iterator it = benders_node_pointer->feasibility_cuts_.begin();
            if ( seperate_first_stage_solver_ && current_stage_ == 0 ){
                it = benders_node_pointer->last_iteration_start_feasibility_;
                benders_node_pointer->last_iteration_start_feasibility_ = benders_node_pointer->feasibility_cuts_.end();
            }
            // TODO: This can be changed to addRows
            for (; it != benders_node_pointer->feasibility_cuts_.end();++it) {
                assert( (*it)->cut_->row().getNumElements() <= osi_solver->getNumCols());
                osi_solver->applyRowCuts(1,(*it)->cut_.get());
            }
        }

        if ( !benders_node_pointer->optimality_cuts_.empty()){
            // Add the cuts
            BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin();
            if ( seperate_first_stage_solver_ &&  current_stage_ == 0 && benders_node_pointer->last_iteration_start_optimality_ != benders_node_pointer->optimality_cuts_.end()){
                it = benders_node_pointer->last_iteration_start_optimality_; // We already added this cut
                if ( benders_node_pointer->last_iteration_start_optimality_ != benders_node_pointer->optimality_cuts_.end()){
                    ++it;
                    benders_node_pointer->last_iteration_start_optimality_ = benders_node_pointer->optimality_cuts_.end();
                }
            }
            // If this was true, reset it now!
            benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_ = false;

            // Problems with warm start and optimality/feasibility cuts mixes..
            for ( ; it != benders_node_pointer->optimality_cuts_.end();++it) {
                if ( !benders_node_pointer->warm_start_added_ && ( (*it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*it)->cut_creation_iteration_ == current_iteration_-1 && current_stage_ == 0)){
                    // Add warm start
                    SetWarmStart(osi_solver,benders_tree_node_pointer);
                    benders_node_pointer->warm_start_added_ = true;
                }
                if ( (*it)->to_use_ ){
                    assert( (*it)->cut_->row().getNumElements() <= osi_solver->getNumCols());
                    osi_solver->applyRowCuts(1,(*it)->cut_.get());
                }

            }
        }

        if ( regularization_ && current_stage_ == 0){
            if ( !seperate_first_stage_solver_ ){
                //LOG(FATAL) << "Regularized Decomposition can not work without a seperable first stage solver instance.";
                regularized_initialized_rows_ = false;
            }
            if ( !regularized_initialized_rows_ ){
                regularized_initialized_rows_ = true;
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr == NULL)
                    LOG(FATAL)<< "OsiSolverInstance is no cplex instance.";
                // Get ENV and LP ptr
                CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
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

    void ParallelNestedBenders::TransformOptimalityCutAggregation( BendersNodeData* benders_node_pointer, const std::vector< std::vector<int> >& new_aggregation_scheme )
    {
        if ( benders_node_pointer->optimality_cuts_.empty() )
            return; // We are done

        // We have to transform the old row cuts with the new row cuts

        // Find out which thetas we want to aggregate and disaggregate
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
            while ( current_node != NULL ){
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
            std::vector<BendersNodeData*> vec;
            nodeVec.push_back(vec);
        } 
        // Fill the node vectors with real nodes
        std::vector<SmiScnNode *>::iterator it = smi_->getSmiTree()->treeBegin();
        while (it != smi_->getSmiTree()->treeEnd()) {
            BendersNodeData *node = new BendersNodeData((*it)->getNode());
            node->scenario_index_ = (*it)->getScenarioIndex();
            node->probability_ = (*it)->getProb();
            if ((*it)->getParent() != NULL && (*it)->getParent()->getScenarioIndex() != (*it)->getScenarioIndex()) {
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
        tree_nodes_by_stage_[0][0]->getDataPtr()->discrepancy_lower_bound_ = infinity_;
        tree_nodes_by_stage_[0][0]->getDataPtr()->average_lower_bound_ = infinity_;
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
                    tree_nodes_by_stage_[i].push_back((*children)[k]);
                    tree_nodes_by_stage_[i].back()->getDataPtr()->num_node_in_stage_ = number_of_nodes_in_this_stage++;
                    tree_nodes_by_stage_[i].back()->getDataPtr()->child_node_start_ = number_of_child_nodes_for_next_stage;
                    tree_nodes_by_stage_[i].back()->getDataPtr()->discrepancy_lower_bound_ = infinity_;
                    tree_nodes_by_stage_[i].back()->getDataPtr()->average_lower_bound_ = infinity_;
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
        if ( warm_up_ && current_stage_ == 0 ){
            if ( seperate_first_stage_solver_ ){ // We need to reset the bounds of the variables!
                // We need to store warm start before that
                CoinWarmStart* coin_warm_start = osi_solver->getWarmStart();
                for (int i = 0; i < stage_data_[0]->num_columns_; i++){
                    osi_solver->setColBounds(i,benders_node_pointer->column_lower_[i],benders_node_pointer->column_upper_[i]);
                }
                osi_solver->setWarmStart(coin_warm_start);
                //osi_solver->writeLp("after_warmstart");
                SolveSolverInterface(osi_solver);
                delete coin_warm_start;

            }
            /* osi_solver->setColLower(benders_node_pointer->column_lower_);
            osi_solver->setColUpper(benders_node_pointer->column_upper_);*/
            warm_up_ = false;
            warm_up_solution_.reset(); // Delete the content of the warm_up solution, not longer needed
            
        }
        if ( benders_node_pointer->unbounded_ ){
            // Remove last row
            int unbounded_row = osi_solver->getNumRows()-1;
            osi_solver->writeLp("before_unbounded");
            osi_solver->deleteRows(1,&unbounded_row);
            SolveSolverInterface(osi_solver);
            osi_solver->writeLp("after_unbounded");
            benders_node_pointer->unbounded_ = false;
            // Add this to all methods that resolve the problem in one way or another!
            if ( seperate_first_stage_solver_ && current_stage_ == 0){
                // Handle optimality cut and feasibility cut pointers!
            }
        }

        delete benders_node_pointer->matrix_; benders_node_pointer->matrix_ = NULL;
        if ( node_pointer->getColLowerLength())
            delete [] benders_node_pointer->column_lower_; 
        benders_node_pointer->column_lower_ = NULL;

        if ( node_pointer->getColUpperLength())
            delete [] benders_node_pointer->column_upper_; 
        benders_node_pointer->column_upper_ = NULL;

        if ( node_pointer->getObjectiveLength())
            delete [] benders_node_pointer->objective_; 
        benders_node_pointer->objective_ = NULL;

        delete [] benders_node_pointer->row_upper_; benders_node_pointer->row_upper_ = NULL;
        delete [] benders_node_pointer->row_lower_; benders_node_pointer->row_lower_ = NULL;
    }

    void ParallelNestedBenders::findAggregationPossibilities( BendersNodeData* param1, std::vector< std::vector<int> >& mapping, int num_childs, int num_cuts, int num_subproblems_per_cut )
    {
        throw std::exception("The method or operation is not implemented.");
    }

    void ParallelNestedBenders::IntraIterationCutAggregation( OsiSolverInterface* osi_solver,  SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Should we really use this method?
        if ( benders_tree_node_pointer->getDataPtr()->num_thetas_ <= 1){
            // IN the case of pure single cuts, we can still remove cuts from the problem!
            //return;
        }

        if ( benders_tree_node_pointer->getDataPtr()->dual_solution_ == NULL)
            return;

        double start_time = CoinCpuTime();
        BendersNodeData* benders_node_pointer(benders_tree_node_pointer->getDataPtr());
        // What do we need? The dual values. They are stored and thus checked against zero tolerance
        _ASSERT( benders_node_pointer->dual_solution_ != NULL);
        assert(store_cut_information_); // If we do not want to store cut information this method should not be called

        bool active_aggregates_per_aggregate_information = false;


        if ( forward_ && !benders_node_pointer->changed_aggregation_level_){ // We only try to aggregate cuts, if aggregation level did not changed in this iteration!
            // We want to store the information about cuts!
            std::vector< std::list< std::pair<int,double> > > active_aggregates_per_aggregate(benders_node_pointer->num_thetas_); // Stores information about cut from which iterations are active for an aggregate and how active they are
            std::vector< std::pair<int,double> > cuts_from_iteration(current_iteration_); // Stores information about how many cuts are active from a given iteration and how active they are

            // Initialize data structures
            for (int i = 0; i< current_iteration_; i++){
                cuts_from_iteration[i].first = 0;
                cuts_from_iteration[i].second = 0;
            }
            // Threshold that defines when active_value is bigger than threshold, we do not aggregates
            CutStuff::active_threshold_ = zero_tolerance_*2.0;
            // Define a threshold above which we do not aggregate cuts for an iteration
            double intra_aggregate_threshold = 0.05*benders_node_pointer->num_thetas_;

            // 0: aggregate only if NO cut is active for an iteration, benders_node_pointer->num_thetas_
            // If cut is consecutively non-active for more than strike_threshold iterations, we can aggregate it!
            CutStuff::strike_threshold_ = 2; 

            

            // We want to add a new single cut after deletion of old cuts!
            bool add_new_single_cut = true;
            if ( benders_tree_node_pointer->getDataPtr()->num_thetas_ <= 1 ){
                add_new_single_cut = false;
                CutStuff::strike_threshold_ = 5; 
            }

            if ( adaptive_ ){
                intra_aggregate_threshold = 0; // If we use adaptive cuts, we can only handle single cuts when we change aggregates (for now)
                CutStuff::strike_threshold_ = 0; // Otherwise we could end up having no active cut, but not inactive enough, so we do not aggregate all cuts together 
            }
            if ( regularization_ ){
                intra_aggregate_threshold = 0.5*benders_node_pointer->num_thetas_;
                CutStuff::strike_threshold_ = 1;
                add_new_single_cut = false;
            }

            int current_cut = getNumRowsProblem()+benders_node_pointer->feasibility_cuts_.size(); // TODO: Store size in variable
            if ( !benders_node_pointer->variable_bounds_.empty() )
                current_cut += benders_node_pointer->variable_bounds_.size();
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
                int aggregate = (*it)->aggregate_number_;
                // Get active value via dual value
                double active = benders_node_pointer->dual_solution_[current_cut];
                // Put values in the corresponding vectors!
                if ( active >= CutStuff::active_threshold_ ){
                    if ( active_aggregates_per_aggregate_information && aggregate >= 0)
                        active_aggregates_per_aggregate[aggregate].push_back(make_pair<int,double>(cut_creation,active));
                    cuts_from_iteration[cut_creation].first++;
                    cuts_from_iteration[cut_creation].second += active;
                    (*it)->active_value = active;
                    (*it)->not_active_iterations_ = 0;
                }
                else {
                    (*it)->active_value = 0;
                    ++(*it)->not_active_iterations_;
                }
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

                int already_removed_cuts = 0;
                std::list<boost::shared_ptr<CutStuff> > cuts_to_add;
                std::list<BendersNodeData::cut_container::iterator> to_remove_from_optimality_cuts;
                std::vector<int> to_remove_from_solver;
                // The active value for all cuts sums up to the number of aggregates. All cuts for each aggregate sum up to 1.
                for (int i = 1; i< current_iteration_; i++){ // Iterate over all iterations
                    assert( cuts_from_iteration[i].second >= 0);
                    if ( cuts_from_iteration[i].second > intra_aggregate_threshold ) // We only proceed with the aggregation if the active level falls below the threshold
                        continue;

                    //LOG_IF(INFO,cuts_from_iteration[i].second > 0) << "Iteration " << i << " is still active with " << cuts_from_iteration[i].second << " for only " << cuts_from_iteration[i].first << " cuts";
                    // Let's put all the cuts of one iteration in one single cut!
                    BendersNodeData::cut_container::iterator cut_stuff_it = std::find_if(benders_node_pointer->optimality_cuts_.begin(), benders_node_pointer->optimality_cuts_.end(),std::bind2nd(CutStuffComparison(),i));
                    if ( cut_stuff_it == benders_node_pointer->optimality_cuts_.end())
                        continue; // In this case we have an iteration where we did not added any optimality cuts
                    if ( (*cut_stuff_it)->aggregate_number_ < 0 ){
                        // Rethink this // TODO
                        continue; // We jump over cuts we already intra-aggregated
                    }
                    assert( (*cut_stuff_it)->aggregate_number_ >= 0);
                    assert( (*cut_stuff_it)->active_value < CutStuff::active_threshold_);

                    boost::shared_ptr<CutStuff> cut_stuff(*cut_stuff_it);
                    // Generate the new single cut!
                    OsiRowCut * new_single_cut = cut_stuff->cut_.get();
                    // We need to create a new vector
                    int col_size = stage_data_[current_stage_]->num_columns_+ benders_node_pointer->num_thetas_; // This many variables are needed
                    double * new_elements = new double[col_size]; // Ownership is taken later by CoinPackedVector new_row
                    int * new_indices = new int[col_size];// Owernship is taken later by CoinPackedVector new_row
                    for (int j = 0; j < col_size; j++){
                        new_elements[j] = 0;
                        new_indices[j] = j;
                    }
                    double rhs = 0;

                    /******
                    // Compute the new cut coefficients and right hand side values
                    // Remove the cuts from the solver (only if they are in there (tested with to_use_)
                    ******/
                    // Declarations of needed variables
                    BendersNodeData::cut_container::iterator end_it = benders_node_pointer->optimality_cuts_.end();
                    int number_cuts_in_this_iteration = 0;
                    CutStuffComparison comparison_object; // Compares number,active value and non-active-generations

                    // Iterate over cuts of this iteration
                    for ( BendersNodeData::cut_container::iterator it = cut_stuff_it; it != benders_node_pointer->optimality_cuts_.end(); ++it){
                        // We must change the CoinPackedVector!
                        if ( (*it)->cut_creation_iteration_ != i ){
                            end_it = it;
                            break;
                        }
                        if ( (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_ALL || (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL){
                            // That means
                            end_it = it;
                            break; // For now just quit!
                        }
                        assert( (*it)->aggregate_number_ >= 0);
                        if ( comparison_object(*it,i) ){
                            for (int j = 0; j < (*it)->cut_->row().getNumElements(); j++ ){
                                new_elements[(*it)->cut_->row().getIndices()[j]] += (*it)->cut_->row().getElements()[j];
                            }
                            rhs += (*it)->cut_->lb();
                            number_cuts_in_this_iteration++;
                            to_remove_from_optimality_cuts.push_back(it);
                            if ( (*it)->to_use_ ){ // Only remove a cut from the solver, if it is in the solver, otherways we have a problem
                                to_remove_from_solver.push_back((*it)->current_index_in_solver_-already_removed_cuts);
                            }
                        }
                    }
                    if ( number_cuts_in_this_iteration <= 1){ // We could not aggregate any cuts together, so we go to the next iteration!
                        delete [] new_elements;
                        delete [] new_indices;
                        continue; 
                    }
                    assert( to_remove_from_solver.empty() || to_remove_from_solver.back() < osi_solver->getNumRows());


                    // Set the values to our old cut
                    CoinPackedVector new_row(col_size,col_size,new_indices,new_elements); // CoinPackedVector takes ownership of new_indices and new_elements
                    new_single_cut->setRow(new_row);
                    new_single_cut->setLb(rhs);
                    if ( number_cuts_in_this_iteration == benders_node_pointer->num_thetas_ )
                        cut_stuff->aggregate_number_ = CutStuff::AGGREGATE_OVER_ALL;  // Indicate that this cut spans over all aggregates we have (until now, but this can be redone)
                    else {
                        cut_stuff->aggregate_number_ = CutStuff::AGGREGATE_OVER_PARTIAL;
                        // We do not want to create these cuts in the case of adaptive_ aggregation
                        LOG_IF(ERROR,adaptive_) << "We IntraIteration aggregated, but only " << number_cuts_in_this_iteration << " instead of " << benders_node_pointer->num_thetas_;
                    }

                    cut_stuff->active_value = 1.0;
                    cut_stuff->to_use_ = true; // Because of this, we must at first sort out the cuts for the solver
                    cuts_to_add.push_back(cut_stuff);
                } //End For

                if ( !to_remove_from_solver.empty()){
                    // TODO: Implementation issue: we can also delete the cuts from the container with the classical list.erase function and begin-iterator to end_iterator that we also store. Only feasible if it holds that alls cuts we remove are also fulfill the CutStuffComparison operator(), i.e. should be aggregated!
                    for ( std::list<BendersNodeData::cut_container::iterator>::iterator list_it = to_remove_from_optimality_cuts.begin();list_it != to_remove_from_optimality_cuts.end(); ++list_it )
                        benders_node_pointer->optimality_cuts_.erase(*list_it);


                    if ( add_new_single_cut ){
                        // Add all newly created cuts to optimality cut set
                        benders_node_pointer->optimality_cuts_.insert(benders_node_pointer->optimality_cuts_.end(),cuts_to_add.begin(),cuts_to_add.end());
                        //benders_node_pointer->optimality_cuts_.push_back(cut_stuff);
                    }

                    if ( !to_remove_from_solver.empty()){
                        already_removed_cuts += to_remove_from_solver.size();
                        osi_solver->deleteRows(to_remove_from_solver.size(),&to_remove_from_solver[0]);
                    }
                    if ( add_new_single_cut ){
                        for (BendersNodeData::cut_container::iterator it = cuts_to_add.begin(); it != cuts_to_add.end(); ++it)
                            osi_solver->applyRowCuts(1,(*it)->cut_.get());
                    }
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
                    // Just keep it up to date ( should do no harm, as we update it in AggregateOptimalityCuts anyway )
                    if ( seperate_first_stage_solver_ && current_stage_ == 0)
                        benders_node_pointer->last_iteration_start_optimality_ = --benders_node_pointer->optimality_cuts_.end();

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
        if ( benders_node_pointer->dual_solution_ == NULL){
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
            if ( benders_node_pointer->discrepancy_ == NULL || benders_node_pointer->min_discrepancy_ == 0 && benders_node_pointer->max_discrepancy_ == 0)
                return; // We have a non NULL pointer

            // Compute a threshold for discrepancies we consider low
            if ( benders_node_pointer->max_discrepancy_ == 0 && benders_node_pointer->min_discrepancy_ == 0)
                return; // We can not do anything..
            double absolute_difference = benders_node_pointer->max_discrepancy_-benders_node_pointer->min_discrepancy_;
            double relative_difference = benders_node_pointer->max_discrepancy_ / benders_node_pointer->min_discrepancy_;
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

            ////double average = std::accumulate(&benders_node_pointer->discrepancy_[0],&benders_node_pointer->discrepancy_[number_of_aggregates],0.0)/ number_of_aggregates;
            //LOG(INFO) << "Minimum : " << benders_node_pointer->min_discrepancy_;
            //LOG(INFO) << "Median  : " << median;
            //LOG(INFO) << "Average : " << benders_node_pointer->average_discrepancy_;
            //LOG(INFO) << "Maximum : " << benders_node_pointer->max_discrepancy_;
            //benders_node_pointer->average_discrepancies_.push_back(benders_node_pointer->average_discrepancy_);
            //benders_node_pointer->median_discrepancies_.push_back(median);
            //LOG(INFO) << "Gap per Aggregate : " << benders_node_pointer->average_gap_per_aggregate_;

            //// Compute average averages and average medians
            //double average_averages = std::accumulate(benders_node_pointer->average_discrepancies_.begin(),benders_node_pointer->average_discrepancies_.end(),0.0)/benders_node_pointer->average_discrepancies_.size();
            //double average_medians = std::accumulate(benders_node_pointer->median_discrepancies_.begin(),benders_node_pointer->median_discrepancies_.end(),0.0)/benders_node_pointer->median_discrepancies_.size();
            //LOG(INFO) << "Average Averages : " << average_averages;
            //LOG(INFO) << "Average Medians  : " << average_medians;

            //if ( average_averages < benders_node_pointer->average_discrepancy_){
            //    LOG(INFO) << "Not aggregating, because average discrepancy is already higher than on average.";
            //    return;
            //}
            //if ( average_medians < median){
            //    LOG(INFO) << "Not aggregating, because median discrepancy is already higher than on average.";
            //    return;
            //}
            //if ( benders_node_pointer->discrepancy_lower_bound_ < benders_node_pointer->discrepancy_value_ ){
            //    LOG(INFO) << "Not aggregating, because discrepancy was higher than the discrepancy lower bound!";
            //    return;
            //}
            //if ( benders_node_pointer->average_lower_bound_ < benders_node_pointer->average_discrepancy_ ){
            //    LOG(INFO) << "Not aggregating, because of average";
            //    return;
            //}
            //if ( benders_node_pointer->average_gap_per_aggregate_lower_bound_ < benders_node_pointer->average_gap_per_aggregate_){
            //    LOG(INFO) << "Not aggregating, because of average gap violation";
            //    return;
            //}


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
            std::vector< boost::shared_ptr<CutStuff> > pointer_to_cuts(new_thetas); 
            int creation_iteration = (*benders_node_pointer->optimality_cuts_.begin())->cut_creation_iteration_;
            int aggregates_for_creation_iteration = 0;
            for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ){
                assert( store_cut_information_ || (*it)->cut_creation_iteration_ >= creation_iteration );
                if ( (*it)->cut_creation_iteration_ != creation_iteration || (*it)->aggregate_number_ == CutStuff::AGGREGATE_OVER_PARTIAL){
                    // Count how many cuts we processed during this creation_iteration. If it is less than new_thetas we have to handle PartialCuts
                    if ( aggregates_for_creation_iteration < new_thetas ){
                        // Find remaining partial cuts and integrate them into one single cut. We need to get hold of one cut instance to start with. We start with the pointer_to_cuts array
                        boost::shared_ptr<CutStuff> new_single_cut;
                        for (int i = 0; i < new_thetas; i++){
                            if ( new_single_cut != NULL){
                                CombineTwoCuts(new_single_cut,pointer_to_cuts[i]);
                                // Remove added cut from optimality cut set
                                benders_node_pointer->optimality_cuts_.remove(pointer_to_cuts[i]);
                            }
                            if ( pointer_to_cuts[i] != NULL && new_single_cut == NULL){
                                new_single_cut = pointer_to_cuts[i];
                            }

                        }
                        assert( new_single_cut != NULL); // We have a cut to work with
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

                    CoinPackedVector cpv(col_size,col_size,new_indices,new_elements); // Assigns elements to vector, arrays get nulled
                    assert( new_elements == NULL);
                    assert( new_indices == NULL);
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

                if ( pointer_to_cuts[new_number] == NULL ){
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
            //osi_solver->writeLp("aggregated");
            if ( seperate_first_stage_solver_ && current_stage_ == 0)
                benders_node_pointer->last_iteration_start_optimality_ = --benders_node_pointer->optimality_cuts_.end();

            if ( osi_solver->isProvenOptimal() ){
                StoreWarmStart(osi_solver,benders_tree_node_pointer);
                StoreValuesFromSolverInArray(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),osi_solver->getNumRows()); // Store duals
                StoreValuesFromSolverInArray(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,new_thetas); // Store new approximation
            }
            else{
                console_mutex_.lock();
                LOG(ERROR) << "Solver did not solve problem to optimality after aggregation.";
                // Get CPLEX error code
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    // Get ENV and LP ptr
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
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

    void ParallelNestedBenders::CombineTwoCuts( boost::shared_ptr<CutStuff>& first_cut, boost::shared_ptr<CutStuff>& cut_to_add)
    {
        if ( first_cut == NULL)
            assert(false);
        if ( cut_to_add == NULL)
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
            CoinPackedVector cpv(num_columns, num_columns,indices, elements);
            first_cut->cut_->setRow(cpv);
        }

        first->setLb(first->lb()+second->lb());
    }

    void ParallelNestedBenders::PrimalDegeneracyHandling(OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer){
        BendersNodeData* benders_node_pointer(benders_tree_node_pointer->getDataPtr());
        bool check_primal_degeneracy = false;
        double * reduced_cost = NULL;
        if ( check_primal_degeneracy ){
            OsiCpxSolverInterface * osi_cpx = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
            CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
            CPXLPptr lp_ptr = osi_cpx->getLpPtr();
            // Check for primal degeneracy : Do we have basic variables at their bounds ?
            int maximal_iterations = 10;
            // Store values for double array
            int num_cols = osi_solver->getNumCols();
            int num_rows = osi_solver->getNumRows();
            std::vector<double> old_objective(num_cols);
            memcpy(&old_objective[0],osi_solver->getObjCoefficients(),sizeof(double)*num_cols);

            double * dual_solutions = new double[num_rows*(maximal_iterations+1)];
            memcpy(dual_solutions,benders_node_pointer->dual_solution_,sizeof(double)*num_rows);
            double * reduced_costs = new double[num_cols*(maximal_iterations+1)];
            memcpy(reduced_costs,osi_solver->getReducedCost(),sizeof(double)*num_cols);
            double * column_solutions = new double[num_cols*(maximal_iterations+1)];
            memcpy(column_solutions,osi_solver->getColSolution(),sizeof(double)*num_cols);

            // Look at basednorms
            //int* cstat = new int[num_cols];
            //int* rstat = new int[num_rows];
            //double* dnorms = new double[num_rows];
            //CPXgetbasednorms(env_ptr,lp_ptr,cstat,rstat,dnorms);
            // // NO norms available

            double * solution_array = new double[maximal_iterations*num_cols];
            int solution_times = 0;
            int multiple_solution_count = 0;
            std::vector<int> indices(osi_solver->getNumRows(),0);
            std::vector<OsiRowCut*> added_cuts;
            bool use_pivoting = true;
            bool use_multiple_solution_algorithm = false;
            assert( use_pivoting ^ use_multiple_solution_algorithm );
            for (int count = 0; count < maximal_iterations; count++){

                const double * colum_lower = osi_solver->getColLower();
                const double * colum_upper = osi_solver->getColUpper();
                const double * obj_coefs = osi_solver->getObjCoefficients();
                const double * column_solution1 = osi_solver->getColSolution();
                if ( use_pivoting ){
                    osi_solver->getBasics(&indices[0]);
                    //Find out if we have a primal degenerate optimal solution
                    std::vector<int> degenerate_variables;
                    double * slack = new double[osi_solver->getNumRows()];
                    //CPXgetslack(env_ptr, lp_ptr, slack, 0, CPXgetnumrows(env_ptr,lp_ptr)-1);
                    for (int i = 0; i < osi_solver->getNumRows(); i++){
                        if ( indices[i] >= 0 && indices[i] < num_cols ){ // We look at structual variables first
                            if ( column_solution1[indices[i]] <= colum_lower[indices[i]]+zero_tolerance_ || column_solution1[indices[i]] >= colum_upper[indices[i]]-zero_tolerance_  )// Column is at one of its bounds
                                if ( !(colum_lower[indices[i]] > -infinity_ && colum_upper[indices[i]] < infinity_ && obj_coefs[indices[i]]) )// Column is not two-side bounded and in the objective function, because if this is so, we can not push this variable in the basis without altering the solution value..
                                    degenerate_variables.push_back(indices[i]);
                        }
                    }
                    if ( degenerate_variables.size() ){
                        int cpx_stat = 0;
                        // Which one should I choose?
                        int entering_index = rand() % degenerate_variables.size();// Random
                        if ( degenerate_variables[entering_index] >= 0 && colum_lower[degenerate_variables[entering_index]] > -infinity_ && colum_upper[degenerate_variables[entering_index]] < infinity_ )
                            cpx_stat = CPXpivot(env_ptr,lp_ptr,CPX_NO_VARIABLE,degenerate_variables[entering_index],CPX_BASIC);
                        else
                            cpx_stat = CPXpivot(env_ptr,lp_ptr,CPX_NO_VARIABLE,degenerate_variables[entering_index],CPX_AT_LOWER);
                        if ( cpx_stat )
                            cpx_stat = CPXpivot(env_ptr,lp_ptr,CPX_NO_VARIABLE,degenerate_variables[entering_index],CPX_BASIC);
                        if ( cpx_stat )
                            cpx_stat = CPXpivot(env_ptr,lp_ptr,CPX_NO_VARIABLE,degenerate_variables[entering_index],CPX_AT_UPPER);
                        if ( cpx_stat == 0 ){
                            solution_times++;
                            memcpy(dual_solutions+num_rows*solution_times,osi_solver->getRowPrice(),sizeof(double)*osi_solver->getNumRows());
                            memcpy(reduced_costs+num_cols*solution_times,osi_solver->getReducedCost(),sizeof(double)*osi_solver->getNumCols());
                        }
                    }
                    else 
                        break;
                }

                if ( use_multiple_solution_algorithm ){
                    std::vector<double> obj_elements(num_cols,0);
                    std::vector<int> variables_at_upper_bounds;
                    std::vector<int> variables_at_lower_bounds; // We can get this info from cplex faster, i suppose
                    for (int i = 0; i < num_cols; i++){
                        if ( column_solution1[i] <= colum_lower[i]+zero_tolerance_ )
                            variables_at_lower_bounds.push_back(i);
                        if ( column_solution1[i] >= colum_upper[i]-zero_tolerance_ )
                            variables_at_upper_bounds.push_back(i);
                    }
                    for (int i = 0; i < variables_at_lower_bounds.size(); i++)
                        obj_elements[variables_at_lower_bounds[i]] = -1;
                    for (int i = 0; i < variables_at_upper_bounds.size(); i++)
                        obj_elements[variables_at_upper_bounds[i]] = 1;

                    if ( multiple_solution_count == 0){
                        std::vector<double> col_elements(num_cols);
                        std::vector<int> col_indices(num_cols);
                        const double * obj_elements_old = osi_solver->getObjCoefficients();
                        for (int i = 0; i < num_cols; i++){
                            col_indices[i] = i;
                            col_elements[i] = obj_elements_old[i];
                        }
                        double obj_val = osi_solver->getObjValue();
                        osi_solver->addRow(num_cols,&col_indices[0],&col_elements[0],obj_val,obj_val); // This invalidates the solution!
                    }
                    else
                        osi_solver->applyRowCuts(1,added_cuts.back());
                    multiple_solution_count++;
                    solution_times++;
                    osi_solver->setObjective(&obj_elements[0]);
                    //std::stringstream lp_name;
                    //lp_name << "fixinglp" << solution_times;
                    //osi_solver->writeLp(lp_name.str().c_str());
                    SolveSolverInterface(osi_solver);
                    if ( osi_solver->isProvenOptimal() ){
                        //if ( osi_solver->getObjValue() > 0){
                        const double * column_solution2 = osi_solver->getColSolution();
                        memcpy(column_solutions+num_cols*solution_times,osi_solver->getColSolution(),sizeof(double)*num_cols);
                        // Now i may have another solution ( alternate optimal solution maybe ) but how do i get the duals from here?!
                        // Set the upper/lower bounds correctly and solve with original objective function to get the duals
                        std::cout << "ObjVal " << osi_solver->getObjValue();


                        int overall_size = variables_at_upper_bounds.size()+variables_at_lower_bounds.size();
                        double*  col_elements = new double[overall_size];
                        int*  col_indices = new int[overall_size];
                        for (int i = 0; i < variables_at_lower_bounds.size(); i++){
                            col_indices[i] = variables_at_lower_bounds[i];
                            col_elements[i] = -1;
                        }
                        for (int i = 0; i < variables_at_upper_bounds.size(); i++){
                            col_indices[variables_at_lower_bounds.size()+i] = variables_at_upper_bounds[i];
                            col_elements[variables_at_lower_bounds.size()+i] = 1;
                        }
                        OsiRowCut * row_cut = new OsiRowCut(0.1*osi_solver->getObjValue(),infinity_,overall_size,overall_size,col_indices,col_elements);
                        added_cuts.push_back(row_cut);
                        //}
                    }  // Push back a cut for the current lp   added_cuts.back()->setLb(0.1*osi_solver->getObjValue());
                    else
                        assert(false);

                }



            }
            // Count+1 gives the number of solutions or maximal_iteration, whatever is less
            // Check if we relly solved the model some times
            if ( solution_times && use_multiple_solution_algorithm ){
                // Create a new solution ( convex combination ) and use this to get the dual values!
                std::vector<int> remove_rows(solution_times,0);
                for (int i = 0; i < solution_times; i++)
                    remove_rows[i] = osi_solver->getNumRows()-i-1;
                osi_solver->deleteRows(solution_times,&remove_rows[0]);
                osi_solver->setObjective(&old_objective[0]);
                //osi_solver->writeLp("afterall");
                osi_solver->setWarmStart(benders_node_pointer->warm_start_);
                // Set values for the column solution ( via variable bounds for example!)
                double * final_column_solution = new double[num_cols];
                for (int i = 0; i < num_cols; i++){
                    final_column_solution[i] = 0;
                    for (int j = 0; j < solution_times+1; j++)
                        final_column_solution[i] += 1.0/(solution_times+1) * column_solutions[i+j*num_cols];
                }
                double* lower = new double[num_cols];
                double* upper = new double[num_cols];
                memcpy(lower,osi_solver->getColLower(),sizeof(double)*num_cols);
                memcpy(upper,osi_solver->getColUpper(),sizeof(double)*num_cols);
                osi_solver->setColLower(final_column_solution);
                osi_solver->setColUpper(final_column_solution);
                SolveSolverInterface(osi_solver);
                const double * dual_again = osi_solver->getRowPrice();
                memcpy(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),sizeof(double)*num_rows);
                double obj_val = osi_solver->getObjValue();
                // Load empty LP to prevent from reusing this solver!
                osi_solver->setColLower(lower);
                osi_solver->setColUpper(upper);
                delete [] lower;
                delete [] upper;
            }
            if ( use_pivoting ){
                // Compute convex combination of solutions!
                for (int i = 0; i < num_rows; i++){
                    benders_node_pointer->dual_solution_[i] = 0;
                    for (int j = 0; j < solution_times+1; j++)
                        benders_node_pointer->dual_solution_[i] += 1.0/(solution_times+1) * dual_solutions[i+j*num_rows];
                }
                reduced_cost = new double[num_cols];
                for (int i = 0; i < num_cols; i++){
                    reduced_cost[i] = 0;
                    for (int j = 0; j < solution_times+1; j++)
                        reduced_cost[i] += 1.0/(solution_times+1) * reduced_costs[i+j*num_cols];
                }
                benders_node_pointer->reduced_cost_ = reduced_cost;
                delete [] dual_solutions;
                delete [] reduced_costs;
                delete [] solution_array;
                //std::cout << "Did "  << solution_times << " many pivots.\n";
            }




        }//End if primal_degeneracy..
    }

    void ParallelNestedBenders::SetWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        if ( warm_start_ && !benders_tree_node_pointer->getDataPtr()->warm_start_added_ && ( current_stage_ > 0 || !seperate_first_stage_solver_)){ // We add a warm_start if configured and first stage solver is not seperate!
            if ( warm_start_scenarios_ ){
                if ( benders_tree_node_pointer->getDataPtr()->warm_start_  ) // If we have a warm start object, we tell the solver to use it
                    osi_solver->setWarmStart(benders_tree_node_pointer->getDataPtr()->warm_start_);
                else{
                    stage_warm_start_mutex_.lock(); 
                    osi_solver->setWarmStart(stage_data_[current_stage_]->warm_start_);
                    stage_warm_start_mutex_.unlock();
                }
            }
            else if ( warm_start_stage_ ){ // If we have a warm start object, we tell the solver to use it. We need a mutex, because we do not have read-only access.
                stage_warm_start_mutex_.lock(); 
                osi_solver->setWarmStart(stage_data_[current_stage_]->warm_start_);
                stage_warm_start_mutex_.unlock();
            }
        }
    }

    void ParallelNestedBenders::StoreWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        if ( warm_start_ && osi_solver->optimalBasisIsAvailable() && ( current_stage_ > 0 || !seperate_first_stage_solver_)){ // Store WarmStart Information, otherwise we reuse the old one..
            if ( warm_start_scenarios_ ){
                if ( benders_tree_node_pointer->scenario() == 0 && benders_tree_node_pointer->getDataPtr()->warm_start_ == NULL ){
                    stage_warm_start_mutex_.lock();
                    delete stage_data_[current_stage_]->warm_start_;
                    stage_data_[current_stage_]->warm_start_ = osi_solver->getWarmStart();
                    stage_warm_start_mutex_.unlock();
                }
                delete benders_tree_node_pointer->getDataPtr()->warm_start_;
                benders_tree_node_pointer->getDataPtr()->warm_start_ = osi_solver->getWarmStart();
            }
            else if ( warm_start_stage_ && benders_tree_node_pointer->scenario() == 0 && ( current_stage_ > 0 || !seperate_first_stage_solver_)){
                // We only generate a warm start for the first scenario and use it then for all the problems. Needs mutex
                stage_warm_start_mutex_.lock();
                delete stage_data_[current_stage_]->warm_start_;
                stage_data_[current_stage_]->warm_start_ = osi_solver->getWarmStart();
                stage_warm_start_mutex_.unlock();
            }
            benders_tree_node_pointer->getDataPtr()->warm_start_added_ = false;
        }
    }

    int ParallelNestedBenders::GetNumberOfAggregatesFromCut( boost::shared_ptr<CutStuff>& cut )
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
        destination_values = new double[size];
        memcpy(destination_values,source_values,size*sizeof(double));
        for (int i = 0; i < size;i++){
            if ( CompareWithZero(destination_values[i]))
                destination_values[i] = 0;
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
        fixed_first_stage_ = true;

    }

    void ParallelNestedBenders::setWarmUpSolution( const double* solution )
    {
        start_solution_ = PNB_INT_USE_SUPPLIED_SOLUTION; 
        warm_up_ = true;
        warm_up_solution_.reset(new double[smi_->getCore()->getNumCols(0)]);
        memcpy(warm_up_solution_.get(),solution,smi_->getCore()->getNumCols(0)*sizeof(double));
    }

    void ParallelNestedBenders::AppendTrustRegionToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        if ( current_stage_ >= num_stages_-1 )
            return; // We do not need trust region at the last stage!

        bool infty_norm = true;
        bool manhattan_norm = false;
        // at most one must be true

        // Method gets called right before SolveSubproblem!
        int num_cols = stage_data_[current_stage_]->num_columns_;
        int num_rows = num_cols; // In case of infty_norm
        int solver_cols = osi_solver->getNumCols();
        int solver_rows = osi_solver->getNumRows();
        double* candidate_solution = benders_tree_node_pointer->getDataPtr()->col_solution_;
        if ( candidate_solution == NULL)
            return;
        // Our integer indices
        std::vector<int> binary_indices;
        double trust_region_radius = 10;

        bool binary_constraint = integer_problems_[current_stage_];
        if ( binary_constraint ){
            // Get Integer Colums
            binary_indices = core_->getBinaryCols(current_stage_);
            if ( binary_indices.empty())
                binary_constraint = false;
            else {
                // Deactivate other trust_region_radius stuff
                infty_norm = false;
                manhattan_norm = false;
                num_cols = 0;
                num_rows = 1; // Only one row
                trust_region_radius = 0.2*stage_data_[current_stage_]->num_columns_; 
                if ( trust_region_radius < 1)
                    trust_region_radius = 1.0;
            }
        }


        // Trust Region with L_\infty -norm or L_1 norm
        // In case of not infty_norm it is -infinity/infinity
        if ( num_cols > 0){
            std::vector<double> colum_lower_bounds(num_cols,-trust_region_radius);
            std::vector<double> colum_upper_bounds(num_cols,trust_region_radius);
            std::vector<double> column_objective(num_cols,0);
            // We need new rows: \mu + x = x' (that is: old column solution!)
            CoinPackedVectorBase** coin_vector = new CoinPackedVectorBase*[num_cols];
            // Put the row elements in the CoinVector!
            for (int i = 0; i < num_cols; i++){
                coin_vector[i] = new CoinShallowPackedVector(0,NULL,NULL,false);
            }
            osi_solver->addCols(num_cols,coin_vector,&colum_lower_bounds[0],&colum_upper_bounds[0],&column_objective[0]);
            // Store the cols we added into a int array!
            int* added_cols = new int[num_cols];
            for (int i = 0; i < num_cols; i++)
                added_cols[i] = solver_cols+i;
            benders_tree_node_pointer->getDataPtr()->trust_region_cols_.reset(added_cols);
            benders_tree_node_pointer->getDataPtr()->num_trust_region_cols_ = num_cols;

            for (int i = 0; i< num_cols; i++){
                delete coin_vector[i];
            }
            delete [] coin_vector;
        }

        if ( num_rows > 0 ){
            CoinPackedVectorBase** coin_vector_rows = new CoinPackedVectorBase*[num_rows];
            // Define the new rows: Equality rows (rlb = rub = candidate solution) with two/three entries \mu- + \mu+ + x = x' or \mu + x = x'
            std::vector<double> row_lower_bounds(num_rows,0);
            std::vector<double> row_upper_bounds(num_rows,0);

            if ( binary_constraint ){
                row_lower_bounds[0] = -infinity_;
                row_upper_bounds[0] = trust_region_radius;
                // Create the elements!
                int * inds = new int[binary_indices.size()];
                double * elems = new double[binary_indices.size()];
                int number_of_ones = 0;
                for (int i = 0; i < binary_indices.size(); i++){
                    if ( candidate_solution[binary_indices[i]] >= 1-integer_tolerance_ ){
                        inds[i] = binary_indices[i];
                        elems[i] = -1;
                        number_of_ones++;
                    }

                    else if ( candidate_solution[binary_indices[i]] <= 0 + integer_tolerance_){
                        inds[i] = binary_indices[i];
                        elems[i] = 1;
                    }
                    else{
                        LOG(INFO) << "Variable " << binary_indices[i] << " has value " << candidate_solution[binary_indices[i]];
                        assert(false);
                    }


                }
                coin_vector_rows[0] = new CoinPackedVector(stage_data_[current_stage_]->num_columns_,inds,elems,false);
                // The cut has the form sum (1-x) + sum (x) <= trust_region_radius. So we need to get all the ones to the RHS,i.e. row_upper_bound
                row_upper_bounds[0] -= number_of_ones;
            }
            else {
                memcpy(&row_lower_bounds[0],candidate_solution,sizeof(double)*num_cols);
                memcpy(&row_upper_bounds[0],candidate_solution,sizeof(double)*num_cols);
            }

            if ( infty_norm ){
                // If L1-Norm we need another row!
                int number_elements = 2;
                for (int i = 0; i < num_rows; i++){
                    int* inds = new int[number_elements];
                    double* elems = new double[number_elements];
                    inds[0] = i;
                    inds[1] = solver_cols+i;
                    elems[0] = 1;
                    elems[1] = 1;
                    coin_vector_rows[i] = new CoinShallowPackedVector(number_elements,inds,elems,false);
                }
            }
            // Add rows to solver
            osi_solver->addRows(num_rows,coin_vector_rows,&row_lower_bounds[0],&row_upper_bounds[0]);
            // Store the rows we added into a int array!
            int* added_rows = new int[num_rows];
            for (int i = 0; i < num_rows; i++)
                added_rows[i] = solver_rows+i;
            benders_tree_node_pointer->getDataPtr()->trust_region_rows_.reset(added_rows);
            benders_tree_node_pointer->getDataPtr()->num_trust_region_rows_ = num_rows;

            for (int i = 0; i< num_rows; i++){
                delete coin_vector_rows[i];
            }

            delete [] coin_vector_rows;
        }

        //upper_bound_mutex_.lock();
        //osi_solver->writeLp("TrustRegionTest");
        //upper_bound_mutex_.unlock();

        // Delete these vectors again!


    }

    void ParallelNestedBenders::SetSolverOptions( OsiSolverInterface* osi_solver )
    {
        // Silence solver 
        osi_solver->messageHandler()->setLogLevel(0);
        osi_solver->setHintParam(OsiDoPresolveInInitial,true);
        if ( lp_method_ == PNB_INT_PRIMAL_SIMPLEX )
            osi_solver->setHintParam(OsiDoDualInInitial,false);// Parameter : Use Primal or Dual Algorithm for Subproblem Solve.
        else 
            osi_solver->setHintParam(OsiDoDualInInitial,true);

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
            // Set markowitz threshold high for better accuracy 
            CPXsetdblparam(env_ptr,CPX_PARAM_EPMRK,0.9);
        }
    }

    void ParallelNestedBenders::BuildAndSolveRegularizedProblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // To compute ||x-a||^2, we introduce a new variable w that is set to w = x-a and we use ||w||^2 in the objective. 
        // Initialize quadratic matrix with entries on the diagonal axis only
        int num_columns = stage_data_[current_stage_]->num_columns_;
        int num_thetas = getNumThetasByStage(benders_tree_node_pointer);

        double approximation_solution = stage_lower_upper_bounds_[0].first;
        // Due to the algorithm it always hold that approximation_solution <= solution_qx

        // Compute Q(x^it)
        double solution_qx = 0;
        bool change_sigma = true;// If we had an infeasible solution earlier, we do not change sigma and do not update the incumbent!
        for (int i = 0; i < num_stages_; i++){
            solution_qx += stage_lower_upper_bounds_[i].second;
            if ( stage_lower_upper_bounds_[i].second <= -infinity_ )
                change_sigma = false;
        }

        bool updated_incumbent = false;
        regularized_gamma_ = 0.5;

        double old_sigma = regularized_sigma_;
        // Test if we should update the regularized solution
        if ( approximation_solution > -infinity_  && change_sigma){
            // Otherwise we have not yet added theta variables
            if ( solution_qx < (1-regularized_gamma_)*regularized_solution_value_+ (regularized_gamma_)*approximation_solution ){
                // Update the solution!
                assert( benders_tree_node_pointer->getDataPtr()->col_solution_);
                memcpy(regularized_solution_.get(),benders_tree_node_pointer->getDataPtr()->col_solution_,sizeof(double)*stage_data_[0]->num_columns_);
                // Update the solution value
                regularized_solution_value_ = solution_qx;
                updated_incumbent = true;
                regularized_sigma_ *= 2;
                LOG(INFO) << "Exact serious step";
                if ( 1.0/regularized_sigma_ < zero_tolerance_ )
                    regularized_sigma_ *= 0.5;
            }
            else if ( solution_qx > regularized_gamma_*regularized_solution_value_+ (1-regularized_gamma_)*approximation_solution ){
                regularized_sigma_ *= 0.5;
                LOG(INFO) << "Null Step";
                if ( 1.0/regularized_sigma_ > 1e8 )
                    regularized_sigma_ *= 2;
            }
            else 
                LOG(INFO) << "Approximate Step";
        }

        if ( updated_incumbent){ // Problem is initialized, we may have to update matrix coefficients
            // Update problem coefficients!
            // What about warm start?
            for (int index = 0; index < stage_data_[0]->num_columns_; index++)
                osi_solver->setRowBounds(regularized_start_row_+index,regularized_solution_[index],regularized_solution_[index]);
        }

        // Solve problem!
        SolveSolverInterface(osi_solver);

        if ( old_sigma != regularized_sigma_ ){
            benders_tree_node_pointer->getDataPtr()->warm_start_ = osi_solver->getWarmStart();
            // Update coefficients in the objective function!
            std::vector<double> sigmas(stage_data_[current_stage_]->num_columns_*2+num_thetas,0);
            CoinFillN(&sigmas[stage_data_[current_stage_]->num_columns_+num_thetas],stage_data_[current_stage_]->num_columns_,1.0/regularized_sigma_);

            OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
            if ( cpx_ptr == NULL)
                LOG(FATAL)<< "OsiSolverInstance is no cplex instance.";
            // Get ENV and LP ptr
            CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
            CPXLPptr lp_ptr = cpx_ptr->getLpPtr();

            // Store warmstart
            if ( integer_problems_[current_stage_] == false )
                benders_tree_node_pointer->getDataPtr()->warm_start_ = osi_solver->getWarmStart();
            CPXcopyqpsep(env_ptr,lp_ptr,&sigmas[0]);
            //int error_code2 = CPXcheckcopyqpsep(env_ptr,lp_ptr,&sigmas[0]);
            //LOG_IF(FATAL,error_code2!=0) << "Wrong quadratic coefficients, must not happen.";

            if ( integer_problems_[current_stage_] == false )
                osi_solver->setWarmStart(benders_tree_node_pointer->getDataPtr()->warm_start_);
            SolveSolverInterface(osi_solver);
            LOG(INFO) << "Coefficients: " << 1.0/regularized_sigma_;
        }
    }

    int ParallelNestedBenders::getNumRowsProblem()
    {
        if ( regularization_ && current_stage_ == 0 && seperate_first_stage_solver_)
            return stage_data_[current_stage_]->num_rows_+stage_data_[current_stage_]->num_columns_;
        else
            return stage_data_[current_stage_]->num_rows_;
    }

    void ParallelNestedBenders::SolveSolverInterface( OsiSolverInterface* osi_solver )
    {

        // MIP Solving only at first stage..
        if ( current_stage_ == 0 && integer_problems_[current_stage_] ){
            // We need to set the integer indices.
            std::vector<int> integer_indices = core_->getIntCols(current_stage_);
            osi_solver->setInteger(&integer_indices[0],integer_indices.size());
            if ( regularization_ ){
                // We have a QP, so branchAndBound method throws an error..
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
                    //CPXsetintparam(env_ptr,)
                    CPXmipopt(env_ptr,lp_ptr);
                    int code = CPXgetstat(env_ptr,lp_ptr);
                    LOG_IF(WARNING,code != 1) << "MIQP solved with status " << code;
                }
            }
            else
                osi_solver->branchAndBound(); // We might need a fixed first-stage solver for this ( preferable from the viewpoint of performance anyway.. ) 
        }
        else{
            if ( regularization_ && current_stage_ == 0){
                // Solve quadratic problem
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr ){
                    CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                    CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
                    CPXqpopt(env_ptr,lp_ptr);
                    int code = CPXgetstat(env_ptr,lp_ptr);
                    LOG_IF(WARNING,code != 1) << "Quadratic problem solved with status " << code;
                }
            }
            else 
                osi_solver->initialSolve();
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
                coin_vector[i] = new CoinShallowPackedVector(0,NULL,NULL,false);
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
            if ( regularization_ && regularized_solution_value_ < infinity_ && current_stage_ == 0)
                cols_to_remove -= stage_pointer->num_columns_;


            // We only want to remove the theta columns, if they are different in this node from a previous one. This is necessary if we changed aggregation level in this node or if we changed aggregation level during this iteration for nodes at this stage, as we resolve them during the process!
            if ( cols_to_remove && (benders_node_pointer->changed_aggregation_level_ || stage_data_[current_stage_]->changed_aggregation_level_) && ( current_stage_ > 0 || !seperate_first_stage_solver_ ) ) { 
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
                    coin_vector[i] = new CoinShallowPackedVector(0,NULL,NULL,false);
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
        if ( regularization_ && current_stage_ == 0){
            
            if ( !seperate_first_stage_solver_ ){
                //LOG(FATAL) << "Regularized Decomposition can not work without a seperable first stage solver instance.";
                regularized_initialized_cols_ = false;
            }
            if ( !regularized_initialized_cols_ ){
                OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
                if ( cpx_ptr == NULL)
                    LOG(FATAL) << "OsiSolverInstance was not a cplex instance.";
                // Get ENV and LP ptr
                CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                CPXLPptr lp_ptr = cpx_ptr->getLpPtr();
                int num_columns = stage_data_[current_stage_]->num_columns_;
                regularized_initialized_cols_ = true;
                assert(num_theta>0);

                double * quaddels = new double[num_columns*2+num_theta]; // contains entries of the matrix 
                double term = 1.0/regularized_sigma_; // Cplex multiplies all coefficients with 1/2
                CoinFillN(quaddels,num_columns+num_theta,0.0);
                CoinFillN(quaddels+num_columns+num_theta,num_columns,term);
                // We add the new columns to the problem
                double * collb = new double[num_columns];
                double * colub = new double[num_columns];
                double * colobj = new double[num_columns];
                for (int i = 0; i < num_columns; i++){
                    collb[i] = -infinity_;
                    colub[i] = infinity_;
                    colobj[i] = 0; 
                }
                CoinPackedVectorBase** cpvb = new CoinPackedVectorBase*[num_columns];
                for (int i = 0; i < num_columns; i++){
                    cpvb[i] = new CoinShallowPackedVector(0,NULL,NULL,false);
                }

                osi_solver->addCols(num_columns,cpvb, collb, colub , colobj);
                int number_cols = osi_solver->getNumCols();
                assert(number_cols == 2*num_columns+num_theta);

                // Add the quadratic objective to the objective function
                int error = 0;
                if (integer_problems_[current_stage_])
                    error = CPXchgprobtype(env_ptr,lp_ptr,CPXPROB_MIQP); 
                else
                    error = CPXchgprobtype(env_ptr,lp_ptr,CPXPROB_QP);
                LOG_IF(ERROR,error) << " Error code " << error << " appeared during call to CPXchgprobtype";
                error = CPXcopyqpsep(env_ptr,lp_ptr,quaddels);
                LOG_IF(ERROR,error) << " Error code " << error << " appeared during call to CPXcopyquad";
                //error = CPXcheckcopyqpsep(env_ptr,lp_ptr,quaddels);
                //LOG_IF(ERROR,error) << " Error code " << error << " appeared during call to CPXcopyquad";
                // Set dual simplex as solution method for QP problems
                CPXsetintparam(env_ptr,CPX_PARAM_QPMETHOD,CPX_ALG_DUAL);

                // Delete arrays for columns
                delete[] collb;
                delete[] colub;
                delete[] colobj;
                for (int i = 0; i < num_columns; i++)
                    delete cpvb[i];
                delete[] cpvb;
                delete[] quaddels;
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
        CoinPackedVector new_row;
        new_row.assignVector(new_cols_in_row,new_indices,new_elements);
        cut->setRow(new_row);

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
            return RowOrderedSparseMatrixPointer(NULL,NULL,NULL);

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
        delete[] cols;
        delete[] clb;
        delete[] cub;
        delete[] obj;
        osi_solver->setObjSense(1.0); // minimize
        osi_solver->initialSolve();
    }


} // end namespace parallel_benders
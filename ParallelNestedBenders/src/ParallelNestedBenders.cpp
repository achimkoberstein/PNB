#include "ParallelNestedBenders.hpp"

#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>

#include <boost/foreach.hpp>
#include "boost/multi_array.hpp"
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>

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

// Include files only necessary if we have cplex
#define CPLEX_PRESENT
#ifdef CPLEX_PRESENT
#include "cplex.h"
#include "OsiCpxSolverInterface.hpp"
#endif


namespace parallel_benders {

    void ParallelNestedBenders::AdjustProblem()
    {
        num_stages_ = core_->getNumStages();
        num_scenarios_ = smi_->getNumScenarios(); // The leaf nodes do not work correctly!
        // Check for integrality right away
        if ( core_->getIntegerLength() || core_->getBinaryLength()){ // Set boolean variable (For now)
            contains_integer_ = true;
            // Set stage wise integers
            // All binary are integer, but not all integer are binary
            const int* integer_indices = core_->getIntegerIndices();
            for (int i = 0; i < num_stages_;i++){
                if ( !core_->getIntCols(i).empty() )
                    integer_problems_[i] = true;
            }
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
        // What do we want to do?
        if ( non_staircase_problem_ ){ // We have non_staircase_rows! How do we handle these?
            // Iterate over all rows
            BOOST_FOREACH( non_staircase_row_map::value_type vec_ref,non_staircase_rows){
                for (int i = 0; i < vec_ref.second.size(); i++){
                    // Do something here!
                }
            }
            // We have to go through it from bottom to top.
            // If we have a non-staircase row in the last stage, we have to find the coefficient of the lowest stage.
            // Then we have to insert new rows with free variables ( and most likely stochastic coefficients.. ) in all stages lower than the last stage.
            // Because it would be necessary to change the whole SmiTree to handle Non-Staircase structure, we can add non-staircase handling behaviour inside our algorithm.
            // We have to manipulate the last stage in the following way: insert the new column, and insert the new row.
            // 

        }
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
        current_iteration_(0),minor_iteration_(0),max_iteration_(1000),lower_bound_(-infinity_),upper_bound_((int)infinity_),temporary_upper_bound_(0),old_lower_bound_(-infinity_),
        current_stage_(0),non_staircase_problem_(false),forward_(true),infeasible_problem_(false),unbounded_problem_(false),infeasible_problems_(smi_->getNumScenarios()),added_optimality_cuts_(num_stages_,0),added_feasibility_cuts_(num_stages_,0),num_added_feasibility_cuts_(0),timer_(num_stages_+1),wall_clock_solution_time_(1),detailed_timer_(4),maximal_solution_time_(0),
        num_cpu_(0),console_mutex_(),solver_map_mutex_(),stage_warm_start_mutex_(),thread_pool_(0),
        warm_start_(false),warm_start_scenarios_(false),warm_start_stage_(false),only_one_feasibility_cut_(false),generate_feasibility_and_optimality_cuts_(false),
		sequencing_protocol_(0),second_iteration_stage_(1),strategy_epsilon_(0.0),repeat_second_iteration_(1),second_iteration_counter_(0),cascading_second_iteration_(false),current_cascading_stage_(1),
        stage_lower_upper_bounds_(),old_lower_upper_bounds_(),temporary_stage_lower_bound_(-infinity_),temporary_stage_upper_bound_(infinity_),compute_stage_bounds_(false),cuts_added_last_round_(0),theta_approximations_everywhere_(false),enforce_direction_(false),
        warm_up_(false),warm_up_solution_(NULL),stage_aggregation_(false),begin_aggregation_stage_(0),end_aggregation_stage_(0),aggregate_from_start_(true),seperate_first_stage_solver_(true),seperate_stage_solver_(false),optimality_cut_counter_(0)
    {

    }

    void ParallelNestedBenders::Init()
    {
        timer_.Start(num_stages_);
        // Initialize osiSolver
        if (smi_->getOsiSolverInterface() == NULL)
            osi_solver_ = new OsiCpxSolverInterface();
        else
            osi_solver_ = smi_->getOsiSolverInterface()->clone();
		// Load empty problem (i.e. reset the solver interface
		osi_solver_->messageHandler()->setLogLevel(0);
		osi_solver_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);


          // Set infinity to solver infinity
        infinity_ = osi_solver_->getInfinity();
        lower_bound_ = -infinity_;
        upper_bound_ = infinity_;
        old_lower_bound_ = lower_bound_;

        int num_theta = 1; // Single cut as default
        int num_scen_pro_cut = 0; // Single cut as default.

        // Iterate over property map..
        boost::property_tree::ptree& param(parameter_.parameter_);

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
        strategy_epsilon_ = param.get(PNB_TREE_WALK_STRATEGY_EPSILON,0.0);
        if ( strategy_epsilon_ < relative_epsilon_ )
            strategy_epsilon_ = relative_epsilon_;

        switch (sequencing_protocol_){
            case PNB_INT_FAST_BACKWARD:
            case PNB_INT_FAST_FORWARD:
            case PNB_INT_EPSILON_FAST_BACKWARD:
            case PNB_INT_EPSILON_FAST_FORWARD:
                compute_stage_bounds_ = true;
                break;
            default:
                compute_stage_bounds_ = false;
        }
        // Assign parameters for sequencing_protocol_ == PNB_INT_SECOND_ITERATION
        cascading_second_iteration_ = param.get(PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_CASCADE,true);
        repeat_second_iteration_ = param.get(PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_REPEAT,2);

        // Warm Up Period
        int start_solution = param.get(PNB_USE_START_SOLUTION,0);
        if ( start_solution == 1)// At the moment we only have EV solution 
            warm_up_ = true;
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

       // Set basic settings, in case of cplex solver..
       OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
       if ( cpx_ptr ){
           // Get ENV and LP ptr
           CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
           CPXsetintparam(env_ptr,CPX_PARAM_NUMERICALEMPHASIS,1);
           CPXsetdblparam(env_ptr,CPX_PARAM_TILIM,maximal_solution_time_);
           //CPXsetintparam(env_ptr,CPX_PARAM_SCAIND,1);
           //CPXsetdblparam(env_ptr,CPX_PARAM_EPMRK,0.9);
       } 

       build_problem_timer_ = 0;
       solve_problem_timer_ = 1;
       cut_generation_timer_ = 2;
       // Detailed_timer 

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
        for (int i = 1; i < num_stages_;i++)
            LOG_IF(WARNING,integer_problems_[i]) << "Your problem contains integer variables at stage " << i+1 << ". We solve the LP relaxation for that stage.";
        

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

        if ( second_iteration_stage_ > num_stages_-2 && sequencing_protocol_ == PNB_INT_SECOND_ITERATION) // Fix second iteration stage by disabling it!
            sequencing_protocol_ = PNB_INT_FASTPASS;
        

        // Warm up period
        if ( warm_up_){
            // We should not use the osi_solver_ we use for the first stage?
            std::pair<double,double*> ev_result = smi_->solveEV(osi_solver_);
            osi_solver_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL); // Reset osi_solver to get a clean first stage solver instance
            warm_up_solution_.reset(ev_result.second); // Store column solution for warm up problem in scoped_ptr.
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
            //OsiRowCut* cut_pointer = new OsiRowCut();
            //cut_pointer->setLb(sum);
            //cut_pointer->setUb(infinity_);
            //cut_pointer->setRow(stage_data_[0]->num_columns_,indices,new_coefs);
            //node->feasibility_cuts_.push_back(cut_pointer);
            // Set aggregate to active: We have a cut for this aggregate
            node->num_thetas_ = 1;
            node->active_cuts_ = boost::dynamic_bitset<>(1);
            node->active_cuts_[0] = true;
            node->cut_information_.CreateCut(current_iteration_);
            // Care about the theta variable also..
            int num_childs = tree_nodes_by_stage_[0][0]->numChildren();
            int num_cuts = std::min(1,num_childs);
            // Write that back to stage_data? No, as number of childs can differ between nodes.
            // Num Subproblem per cuts
            int num_subproblems_per_cut = (int) std::ceil( ((double)num_childs)/num_cuts );
            createRoundRobinAggregation(node,num_childs,num_cuts,num_subproblems_per_cut);
        }

        timer_.Stop(setup_timer);
        wall_clock_solution_time_.Start(0);
        // Do initialization stuff necessary for solve process
        while ( lower_bound_ < upper_bound_ &&
            std::abs(upper_bound_-lower_bound_) > absolute_epsilon_ &&  
            ComputeRelativeGap(upper_bound_,lower_bound_,infinity_,relative_epsilon_) &&
            current_iteration_ < max_iteration_ 
            ) 
        {
            wall_clock_solution_time_.Stop(0);
            // Check if we already went over the time limit
            if ( ((int) wall_clock_solution_time_.GetTime(0)) > maximal_solution_time_ ){
                LOG(ERROR) << "We stopped solving the problem after " << wall_clock_solution_time_.GetTime(0)/60 << " minutes.";
                return; // We stop solving the problem
            }
            wall_clock_solution_time_.Start(0);
            timer_.Start(current_stage_);
            if ( upper_bound_ <= lower_bound_ ){
                LOG(INFO) << "Upper Bound is lower than lower Bound. This should not happen, unfortunately it has. Iteration: " << current_iteration_;
                //break;
            }
            if ( old_lower_bound_ > (lower_bound_ + ParallelNestedBenders::zero_tolerance_) && old_lower_bound_ > -infinity_  && !infeasible_problem_){// TODO: If we have feasCuts we might have to look at this again..
                LOG(INFO) << "New Lower bound " << lower_bound_ << " is less than old lower bound " << old_lower_bound_;
                //break;
            }

            minor_iteration_++;
            if ( current_stage_ == 0 ){ // Things to do if we are in the first stage
                current_iteration_++;
            }

            // Change direction depending on last or first stage. This is sequencing protocol 0.

            if ( current_stage_ == 0 ){
                forward_ = true;
                // If we had infeasible problems in the last backward run, we added all the feas cuts and can start fresh
                infeasible_problem_ = false;
            }
            else if ( current_stage_ == num_stages_ -1)
                forward_ = false;

            // Try the shuffle or FastForward Strategy
            if ( (sequencing_protocol_ == PNB_INT_FAST_FORWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_FORWARD) && !forward_ ){
                if ( current_stage_ < num_stages_-1){// We can update the upper bound, because we do not do that elsewhere if we use this strategy.
                    // Decide wether we want to go forward again 
                    // We want to test if the lower bound for this stage is close enough to the upper bound
                    double current_upper_bound = 0;
                    for (int i = current_stage_; i < num_stages_; i++)
                        current_upper_bound += stage_lower_upper_bounds_[i].second;
					if ( ComputeRelativeGap(current_upper_bound,stage_lower_upper_bounds_[current_stage_].first,infinity_,strategy_epsilon_) ){// If true, values are not equal. 
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

            // Try the cautious or FastBackward Strategy. Main logic lies behind the solution at the current stage.
            if ( theta_approximations_everywhere_ && (sequencing_protocol_ == PNB_INT_FAST_BACKWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_BACKWARD) ){
                if ( current_stage_ > 0 ){
                    forward_ = false;
               }
            }

            // Change direction depending on whatever we like. 
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

            std::string direction(forward_ ? "F " : "B ");
            std::cout << minor_iteration_ << "Iteration " << direction << current_iteration_ << "." << current_stage_ << std::endl;
            if ( current_stage_ == 0){
                std::cout << "Lower Bound:  " << lower_bound_ << std::endl;
                std::cout << "Upper Bound:  " << upper_bound_ << std::endl;
                std::cout << "Relative Gap: " << abs(upper_bound_-lower_bound_)/abs(upper_bound_)  << std::endl;
            }

            // Initialize lower/upper bounds for this stage.
            temporary_stage_lower_bound_ = 0;
            temporary_stage_upper_bound_ = 0;

            int num_nodes = tree_nodes_by_stage_[current_stage_].size();
            for ( int i = 0; i < num_nodes; i++ ){
                // Schedule the tasks for this stage
                thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]));
            }
            thread_pool_.wait(0); // We wait until all threads are done

            // Update stage wise upper/lower bounds, only if we did not had infeasible problems.
            if ( infeasible_problems_.none() && compute_stage_bounds_){
                old_lower_upper_bounds_[current_stage_].first = stage_lower_upper_bounds_[current_stage_].first;
                old_lower_upper_bounds_[current_stage_].second = stage_lower_upper_bounds_[current_stage_].second;
                // If we solved all problems without infeasibility, we update these values.
                stage_lower_upper_bounds_[current_stage_].first = temporary_stage_lower_bound_;
                stage_lower_upper_bounds_[current_stage_].second = temporary_stage_upper_bound_;
            }

            // Update upper bound if we solved all problems in this iteration once
            // Lower bound is updated in the StoreData Method after solving first stage problem
            // This does only work for methods, that do not reiterate and use the stage_bounds..
            if ( current_stage_ == num_stages_ -1 && infeasible_problems_.none() ){
                if ( compute_stage_bounds_ ){
                    double sum_stage_solutions = 0;
                    for (int i = 0; i < num_stages_; i++)
                        sum_stage_solutions += stage_lower_upper_bounds_[i].second;
                    if ( upper_bound_ > sum_stage_solutions)
                        upper_bound_ = sum_stage_solutions;
                }
                else if ( upper_bound_ > temporary_upper_bound_ )
                    upper_bound_ = temporary_upper_bound_;
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

            // If we use fast backward strategy, we have to decide where we go now
            if ( theta_approximations_everywhere_ && infeasible_problems_.none() && (sequencing_protocol_ == PNB_INT_FAST_BACKWARD || sequencing_protocol_ == PNB_INT_EPSILON_FAST_BACKWARD )){
                //cuts_added_last_round_ += added_optimality_cuts_[current_stage_];
                // Or we should test the discrepancy thesis ( either relative or absolute )
                // We always check the gap between this and the stage before.
                if ( current_stage_< num_stages_-1 && current_stage_ > 0 && stage_lower_upper_bounds_[current_stage_-1].second < infinity_
                    && stage_lower_upper_bounds_[current_stage_-1].first > -infinity_ && stage_lower_upper_bounds_[current_stage_].second < infinity_ ){
                        // What do we want to compute? Discrepancy it is.
                        double before_stage_theta = stage_lower_upper_bounds_[current_stage_-1].first-stage_lower_upper_bounds_[current_stage_-1].second;
                        double current_stage_x = stage_lower_upper_bounds_[current_stage_].second;
                        double current_stage_theta = stage_lower_upper_bounds_[current_stage_].first-stage_lower_upper_bounds_[current_stage_].second;
                        if ( current_stage_ == num_stages_-1 )
                            current_stage_theta = 0;
                        // Compute the gap of this stage with the stage before to decide where to go
                        if ( !ComputeRelativeGap(current_stage_x,before_stage_theta-current_stage_theta,infinity_,strategy_epsilon_) ||
                            std::abs(current_stage_x-before_stage_theta+current_stage_theta) <= absolute_epsilon_ ||
                            current_stage_x < before_stage_theta-current_stage_theta ){
                                // We fulfilled our epsilon distance, so we can move to the next stage
                                // Question: What happens, if we get so close, we do not add any cuts to the lower stage problem and go here forever?
                                // Same must in principle hold for the EPSILON_FORWARD algorithm. Not foreseen in Morton 1996.
                                // Solve this by storing current lower / upper bounds and compare if we come here again
                                forward_ = true; // skip this stage, i.e. move forward this stage
                                if ( std::abs(old_lower_upper_bounds_[current_stage_].first-stage_lower_upper_bounds_[current_stage_].first) < absolute_epsilon_ 
                                    && std::abs(old_lower_upper_bounds_[current_stage_].second-stage_lower_upper_bounds_[current_stage_].second) < absolute_epsilon_  ){
                                        // We have the same old_lower_upper_bound.first and second. we do not go forward, but apply the cuts we already have.
                                        forward_ = false;
                                        LOG(INFO) << "Your Epsilon chosen for your tree-walking strategy might be too big. We switch to fastpass, because we can not update your solution at stage " << current_stage_-1 << " anymore.";
                                        sequencing_protocol_ = PNB_INT_FASTPASS;
                                }
                        }//End If Gap checking
                }// End If Stage and Value Checking
            }//End If FastBackward/FastEpsilonBackward Procedure

            //if ( infeasible_problem_ && infeasible_problems_.none() ){
            //    // If one problem at later stage was infeasible and in this stage no problem is infeasible. We want to move forward again!
            //    // Only if we have not added optimality cuts! Otherwise we can deadlock ourselves, can we?
            //    forward_ = true;
            //    infeasible_problem_ = false;
            //}

            // Aggregate optimality cuts for nodes at the stage before the current stage
            if ( !forward_ && infeasible_problems_.none() ){ 
                // We need to aggregate optimality cuts for the parent node
                for ( int i = 0; i < tree_nodes_by_stage_[current_stage_-1].size(); i++)
                    // Schedule the tasks for this stage
                    thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AggregateOptimalityCuts,this,tree_nodes_by_stage_[current_stage_-1][i]));
                thread_pool_.wait(0); // We wait until all threads are done
                // If we added no cuts, we might want to notice that
            }

            // If we added no new cuts, we can move forward again? Or backward to test for stopping criterion?

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
            }




            // We change the stage, so we should clear all of the osiSolverInterfaces we used
            std::pair<boost::thread::id, OsiSolverInterface* > id_solver_pair;
            BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
                //id_solver_pair.second->reset(); // This method throws exception with Cplex
                id_solver_pair.second->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                // Workaround: Load empty problem. 
                assert(id_solver_pair.second->getNumCols() == 0);
            }
            timer_.Stop(current_stage_);
            // Change next stage, depending on where we are heading (forward or backward)
            forward_ ? current_stage_++ : current_stage_--;
        }
        timer_.Stop(current_stage_);
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
        if (!infeasible_problem_)
            LOG(INFO) << "Solution: " << upper_bound_;
        else{
            upper_bound_ = -infinity_;
            LOG(INFO) << "Problem is infeasible.";
        }

        // Do a Cut Analyzation
        // Iterate over the stages 
        //             // Create output!
        bool analyse_cuts_ = true;
        if ( analyse_cuts_ ){
            ofstream cut_analysis;
            cut_analysis.open("cut_analysis", std::ios_base::app);
            for (int i = 0; i < num_stages_-1; i++){ // Last stage has nothing of interest!
                // Stage Wise Portrait: Active Cuts in a given Iteration
                std::vector<int> active_cuts_in_iteration(current_iteration_,0);
                boost::multi_array<int,2> active_cuts_from_iteration(boost::extents[current_iteration_][current_iteration_]);
                //std::vector<int> active_cuts_from_iteration(current_iteration_,0);
                for (int j = 0; j < tree_nodes_by_stage_[i].size(); j++){
                    CutManager& cut_manager = tree_nodes_by_stage_[i][j]->getDataPtr()->cut_information_;
                    int num_cuts = cut_manager.NumCuts();
                    for (int current_cut = 0; current_cut < num_cuts; current_cut++){
                        CutInformation& cut_information(cut_manager.GetCut(current_cut));
                        for (int temp_iteration = cut_information.cut_creation_iteration_; temp_iteration < current_iteration_; temp_iteration++){
                            if ( cut_information.iteration_forward_active_vector_.find(temp_iteration) != cut_information.iteration_forward_active_vector_.end() ){
                                active_cuts_in_iteration[temp_iteration]++;
                                active_cuts_from_iteration[cut_information.cut_creation_iteration_+1][temp_iteration]++;
                            }
                        }
                    }
                }
                cut_analysis << "Stage " << i << ";";
                for (int array_dim1 = 2; array_dim1 < current_iteration_; array_dim1++){
                    cut_analysis << "It." << array_dim1 << ";";
                }
                cut_analysis << std::endl;
                for (int array_dim1 = 2; array_dim1 < current_iteration_; array_dim1++){
                    cut_analysis << "It. " << array_dim1 << ";";
                    for ( int array_dim2 = 2; array_dim2 < current_iteration_; array_dim2++){
                        cut_analysis << active_cuts_from_iteration[array_dim2][array_dim1] << ";";
                    }
                    cut_analysis << std::endl;
                }
            }
        }
        // Show the first stage solution! Only if feasible
        //if ( !infeasible_problem_ )
        //    for (int i = 0; i < stage_data_[0]->num_columns_; i++){
        //        std::cout << "x" << i << " : " << tree_nodes_by_stage_[0][0]->getDataPtr()->col_solution_[i] << std::endl;
        //    }

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
		AppendCutsToSubproblem(osi_solver,benders_tree_node_pointer);
        SolveSubproblem(osi_solver,benders_tree_node_pointer);
        // We only need to store data if we have a feasible solution for the subproblem.
        if ( !infeasible_problems_[benders_tree_node_pointer->scenario()]){
            StoreData(osi_solver,benders_tree_node_pointer);
            // Check for degeneracy and stuff, if we like it..
            // At this point we have indeed an optimal solution!
            if ( !forward_)
                GenerateOptimalityCut(osi_solver,benders_tree_node_pointer);
        }
        DeleteSubproblem(benders_tree_node_pointer);
    }

    void ParallelNestedBenders::SolveSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer  ){
        // Warm Start Handling
        // Depending on Warm Start behaviour, use the corresponding entry
        // If WarmstartForScenario or WarmstartForStage do the following
        // 
        //if ( current_stage_ <= 8){ // Only write solution file if we are in the first stage..
        //    std::stringstream stream;
        //    stream << "stage" << current_stage_ << "_" << current_iteration_ << "_" << benders_tree_node_pointer->scenario();
        //    console_mutex_.lock();
        //    osi_solver->writeLp(stream.str().c_str());
        //    console_mutex_.unlock();
        //} // TODO: Debugging stuff

        if ( warm_start_ ){
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
        // We set the first stage warm_up solution, if we had one!
        if ( warm_up_  && current_stage_ == 0){
            for (int i = 0; i < stage_data_[0]->num_columns_; i++){
                osi_solver->setColBounds(i,warm_up_solution_[i],warm_up_solution_[i]);
            }

        }
 
        osi_solver->setHintParam(OsiDoPresolveInInitial,true);
        if ( lp_method_ == PNB_INT_PRIMAL_SIMPLEX )
            osi_solver->setHintParam(OsiDoDualInInitial,false);// Parameter : Use Primal or Dual Algorithm for Subproblem Solve.
        else 
             osi_solver->setHintParam(OsiDoDualInInitial,true);

        // MIP Solving only at first stage..
        if ( current_stage_ == 0 && integer_problems_[current_stage_] ){
             // We need to set the integer indices.
             std::vector<int> integer_indices = core_->getIntCols(current_stage_);
             osi_solver->setInteger(&integer_indices[0],integer_indices.size());
             osi_solver->branchAndBound(); // We might need a fixed first-stage solver for this ( preferable from the viewpoint of performance anyway.. ) 

        }
        else{
            osi_solver->initialSolve();
        }


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
                // If we are in a problem with theta variables, we might have sucess in restricting the theta variables
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
                        osi_solver->addRow(num_thetas,extra_indices.get(),extra_elements.get(),-infinity_+10000000, infinity_); // We assume that will work. I mean: Half infinity? 
                        // Solve the model again
                        if ( current_stage_ == 0 && integer_problems_[current_stage_] )
                            osi_solver->branchAndBound();
                        else
                            osi_solver->resolve();
                    }
                }

                if ( osi_solver->isProvenPrimalInfeasible() ){
                    // Go below for feasibility cut
                }
                else if ( !osi_solver->isProvenOptimal()){
                    if ( osi_solver->isProvenDualInfeasible() ){
						console_mutex_.lock();
                        LOG(ERROR)<< "Problem for scenario " << benders_tree_node_pointer->scenario() << " in stage " << current_stage_ << " is primal unbounded.";
                        LOG(ERROR) << "This can point to an error in your model/data. This may also happen, when your original problem was meant as an maximization problem.";
                        osi_solver->writeLp("unboundedProblem.lp");
						console_mutex_.unlock();
                        // We can not work with that in a reasonable manner..
                        upper_bound_ = infinity_;
                        unbounded_problem_ = true; // If a problem is really unbounded, the whole problem is unbounded ( see paper VanSlyke & Wets )
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
                    //CPXsetintparam(env_ptr,CPX_PARAM_SCAIND,1);
                    //CPXsetdblparam(env_ptr,CPX_PARAM_EPMRK,0.9);
                }

            }
            
            // TODO Other possible cases
            // Get the cplex stat..

        }// if not provenOptimal end
        else { // Proven Optimal: 
            if ( warm_start_ && osi_solver->optimalBasisIsAvailable()) // Store WarmStart Information, otherwise we reuse the old one..
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
                else if ( warm_start_stage_ && benders_tree_node_pointer->scenario() == 0){
                    // We only generate a warm start for the first scenario and use it then for all the problems. Needs mutex
                    stage_warm_start_mutex_.lock();
                    delete stage_data_[current_stage_]->warm_start_;
                    stage_data_[current_stage_]->warm_start_ = osi_solver->getWarmStart();
                    stage_warm_start_mutex_.unlock();
                }

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
		assert(num_theta >= 0);

		// Retrieve and store the theta variables
        if ( num_theta ){
            benders_node_pointer->next_stage_approximation_ = new double[num_theta];
            memcpy(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,num_theta*sizeof(double) );
        }
        
        if ( forward_ || current_stage_ == num_stages_-1 || compute_stage_bounds_ ){ // We need the upper bound, therefore we need to store the column solutions
            // We want to differentiate between column solution and theta variables 
            benders_node_pointer->col_solution_ = new double[stage_data_[current_stage_]->num_columns_];
            memcpy(benders_node_pointer->col_solution_,osi_solver->getColSolution(),stage_data_[current_stage_]->num_columns_*sizeof(double));
            double solution_value = 0;
            for (int i = 0; i < stage_data_[current_stage_]->num_columns_;i++){
                if ( CompareWithZero(benders_node_pointer->col_solution_[i]))
                    benders_node_pointer->col_solution_[i] = 0;
                solution_value += benders_node_pointer->col_solution_[i]*osi_solver->getObjCoefficients()[i];
            }
			double current_lower_bound = solution_value;
            if ( compute_stage_bounds_ ){
                for (int i = 0; i < num_theta; i++){
                    current_lower_bound += benders_node_pointer->next_stage_approximation_[i];
                }
                current_lower_bound *= benders_node_pointer->probability_;
                if ( num_theta == 0)
                    current_lower_bound = -infinity_;
            }
            if ( current_stage_ == 0) // This work in the multithreaded cases, as we have only one thread for the first stage problem
                temporary_upper_bound_ = 0;
            upper_bound_mutex_.lock();
            temporary_upper_bound_ += solution_value*benders_node_pointer->probability_;
            if ( compute_stage_bounds_ ){
                // We have to see that we can use FAST_BACKWARD strategy. This can at first happen, when we have theta variables at stage 1.
                if (current_stage_ == 0 && current_lower_bound > -infinity_)
                    theta_approximations_everywhere_ = true;
                temporary_stage_lower_bound_ += current_lower_bound;
                temporary_stage_upper_bound_ += solution_value*benders_node_pointer->probability_;
            }
            upper_bound_mutex_.unlock();
			// Delete column solution, if we are in the last stage, otherwise we still need it.
			if ( current_stage_ == num_stages_-1 ){
				delete [] benders_node_pointer->col_solution_;
				benders_node_pointer->col_solution_ = NULL;
			}
        }


        // Set lower bound
        if ( current_stage_ == 0 ){
            old_lower_bound_ = lower_bound_; // Set old lower bound to last lower bound. If we added a feasibility cut lately, we have to take special care.
            lower_bound_ = temporary_upper_bound_;
            for ( int i = 0; i < num_theta;i++){
                lower_bound_ += benders_node_pointer->next_stage_approximation_[i];
            }
            if (num_theta == 0){// We have no thetas yet
                lower_bound_ = -infinity_;
                old_lower_bound_ = -infinity_;
            }
        }
        if ( !forward_ ){
            // Store Dual Solution ( only if we move backward )
            benders_node_pointer->dual_solution_ = new double[osi_solver->getNumRows()];
            memcpy(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),osi_solver->getNumRows()*sizeof(double));
            for (int i = 0; i < osi_solver->getNumRows();i++){
                if ( CompareWithZero(benders_node_pointer->dual_solution_[i]))
                    benders_node_pointer->dual_solution_[i] = 0;
            }
            // Put this in own method TODO:
            benders_node_pointer->reduced_cost_ = new double[osi_solver->getNumCols()];
            memcpy(benders_node_pointer->reduced_cost_,osi_solver->getReducedCost(),osi_solver->getNumCols()*sizeof(double));
            for (int i = 0; i < osi_solver->getNumCols();i++){
                if ( CompareWithZero(benders_node_pointer->reduced_cost_[i]))
                    benders_node_pointer->reduced_cost_[i] = 0;
            }
 

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
                        osi_solver->resolve();
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
                    osi_solver->resolve();
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



            // Compute the values: \lambda * lower_bound - \mu * upper_bound
            // Compute the resulting values right away, as we have the osi_solver_interface at hand.
            // TODO: If reduced cost already computed, use these instead of the other ones..
            const double* dual_values_variable_bounds = benders_node_pointer->reduced_cost_;
            const double* variable_lower_bounds = osi_solver->getColLower();
            const double* variable_upper_bounds = osi_solver->getColUpper();
            double computed_dual_value_for_variable_bounds = 0;
            const double* column_solution = osi_solver->getColSolution();
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
        bool store_cut_information_ = false;

        if ( forward_ && store_cut_information_){
            // We want to store the information about cuts!
            std::vector< std::list< std::pair<int,double> > > active_cuts_per_aggregate(benders_node_pointer->num_thetas_); // Stores information about cut from which iterations are active for an aggregate and how active they are
            std::vector< std::pair<int,double> > cuts_from_iteration(current_iteration_); // Stores information about how many cuts are active from a given iteration and how active they are

            std::list< BendersNodeData::cut_container::iterator > to_remove;
            // We need the dual values!
            benders_node_pointer->dual_solution_ = new double[osi_solver->getNumRows()];
            memcpy(benders_node_pointer->dual_solution_,osi_solver->getRowPrice(),osi_solver->getNumRows()*sizeof(double));
            for (int i = 0; i < osi_solver->getNumRows();i++){
                if ( CompareWithZero(benders_node_pointer->dual_solution_[i]))
                    benders_node_pointer->dual_solution_[i] = 0;
            }
            int current_row = stage_data_[current_stage_]->num_rows_;
            // Store the number of rows/columns
            for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
                // First cut corresponds to the first row after the rows of the original problem!
                // Find out if cut is active!
                int cut_creation = (*it)->cut_creation_iteration_;
                int aggregate = (*it)->aggregate_number_;
                // Get active value via dual value
                double active = benders_node_pointer->dual_solution_[current_row];
                // Put values in the corresponding vectors!
                if ( active ){
                    active_cuts_per_aggregate[aggregate].push_back(make_pair<int,double>(cut_creation,active));
                    cuts_from_iteration[cut_creation].first++;
                    cuts_from_iteration[cut_creation].second += active;
                    (*it)->active_value += active;
                }
                else 
                    (*it)->active_value -= 0.5;
                current_row++;
            }
            // Remove all cuts in the to_remove list
            //benders_node_pointer->optimality_cuts_.remove_if(is_active);

            if ( benders_node_pointer->num_thetas_ > 1 )
                for (int i = 1; i< current_iteration_; i++){
                    LOG(INFO) << "Active from iteration " << i << " are " << cuts_from_iteration[i].first << " cuts with a value of " << cuts_from_iteration[i].second;
                    if ( cuts_from_iteration[i].second < zero_tolerance_ ){
                        // Let's put all the cuts in one single cut!
                        OsiRowCut * new_single_cut = (*benders_node_pointer->optimality_cut_pointer_[i])->cut_.get();
                        CoinPackedVector& row(new_single_cut->mutableRow());
                        // We need to create a new vector
                        int col_size = stage_data_[current_stage_]->num_columns_+ benders_node_pointer->num_thetas_; // This many variables are needed
                        double * new_elements = new double[col_size];
                        int * new_indices = new int[col_size];
                        for (int j = 0; j < col_size; j++){
                            new_elements[j] = 0;
                            new_indices[j] = j;
                        }
                        double rhs = 0;
                        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cut_pointer_[i]; it != benders_node_pointer->optimality_cut_pointer_[i+1]; ++it){
                            // We must change the CoinPackedVector!
                            for (int j = 0; j < (*it)->cut_->row().getNumElements(); j++ ){
                                new_elements[(*it)->cut_->row().getIndices()[j]] += (*it)->cut_->row().getElements()[j];
                            }
                            rhs += (*it)->cut_->lb();
                        }
                        CoinPackedVector new_row(col_size,new_indices,new_elements);
                        new_single_cut->setRow(new_row);
                        new_single_cut->setLb(rhs);
                        (*benders_node_pointer->optimality_cut_pointer_[i])->active_value = 1.0;
                        BendersNodeData::cut_container::iterator start = benders_node_pointer->optimality_cut_pointer_[i];
                        ++start;
                        benders_node_pointer->optimality_cuts_.erase(start, benders_node_pointer->optimality_cut_pointer_[i+1]);
                        LOG(INFO) << "Building new cut for iteration " << i;
                        

                    }
                }

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
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        SmiNodeData* node_pointer = benders_node_pointer->smi_node_data_; // Stochastic node pointer
        SmiNodeData* deterministic_node_pointer = core_->getNode(current_stage_); // Deterministic node (there is one per stage)
        bool reuse_solver = true;
		// If we do not want to reuse the solver, we have to compute the matrix
		if ( node_pointer->hasMatrix() || osi_solver->getNumCols() == 0 || !reuse_solver ){ // We have to load the matrix / change the matrix => load the problem
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
        if ( stage_pointer->stochastic_rhs_ )
            for (int i = 0; i < node_pointer->getRowLowerLength(); i++) {
                benders_node_pointer->row_lower_[node_pointer->getRowLowerIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowLowerElements()[i];
            }

        benders_node_pointer->row_upper_ = new double[stage_pointer->num_rows_];
        memcpy(benders_node_pointer->row_upper_, core_->getDenseRowUpper(current_stage_), stage_pointer->num_rows_*sizeof(double));
        if  ( stage_pointer->stochastic_rhs_)
            for (int i = 0; i < node_pointer->getRowUpperLength(); i++) {
                benders_node_pointer->row_upper_[node_pointer->getRowUpperIndices()[i] - core_->getRowStart(current_stage_)] = node_pointer->getRowUpperElements()[i];
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
                            if ( stochastic_matrix.indices_[stochastic_column_index] >= current_stage_columns && stochastic_column_index < stochastic_matrix.start_[row+1] )
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

        // QuicknDirtyhack: REMOVE AGAIN
        std::vector<OsiRowCut*>& row_cuts(benders_node_pointer->variable_bounds_);
        if ( bounds_in_matrix_ ){
            if ( benders_node_pointer->variable_bounds_.empty() ){
                // Apply osi row_cuts
                boost::dynamic_bitset<> upper_column_indices(stage_data_[current_stage_]->num_columns_);
                boost::dynamic_bitset<> lower_column_indices(stage_data_[current_stage_]->num_columns_);
                for (int i = 0; i < stage_data_[current_stage_]->num_columns_; i++){
                    if ( benders_node_pointer->column_upper_[i] != 0 )
                        upper_column_indices.set(i);
                    if ( benders_node_pointer->column_lower_[i] != 0 )
                        lower_column_indices.set(i);
                }
                for (int i = upper_column_indices.find_first(); i< upper_column_indices.npos; i = upper_column_indices.find_next(i)){
                    // Create an osi row_cut
                    double * element1 = new double[1];
                    int * index1 = new int[1];
                    element1[0] = 1;
                    index1[0] = i;
                    row_cuts.push_back(new OsiRowCut(-infinity_,benders_node_pointer->column_upper_[i],1,1,index1,element1));
                    benders_node_pointer->column_upper_[i] = infinity_;
                }
                for (int i = lower_column_indices.find_first(); i< lower_column_indices.npos; i = lower_column_indices.find_next(i)){
                    // Create an osi row_cut
                    double * element1 = new double[1];
                    int * index1 = new int[1];
                    element1[0] = 1;
                    index1[0] = i;
                    row_cuts.push_back(new OsiRowCut(benders_node_pointer->column_lower_[i],infinity_,1,1,index1,element1));
                    benders_node_pointer->column_lower_[i] = -infinity_;
                }
            }
        }

        // Load the basic problem
        // We call this method if we just constructed the matrix for the first time
        if ( !reuse_solver ){
            osi_solver->loadProblem(*benders_node_pointer->matrix_,benders_node_pointer->column_lower_,benders_node_pointer->column_upper_,benders_node_pointer->objective_,benders_node_pointer->row_lower_,benders_node_pointer->row_upper_);
            if ( bounds_in_matrix_ )
                for (int i = 0; i < row_cuts.size(); i++)
                    osi_solver->applyRowCuts(1,row_cuts[i]); // TODO REMOVE AGAIN
        }
        else{
            if ( stage_pointer->stochastic_bounds_ ){
                // Update bounds
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
            if ( bounds_in_matrix_ )
                for (int i = 0; i < row_cuts.size(); i++)
                    osi_solver->applyRowCuts(1,row_cuts[i]); // TODO REMOVE AGAIN
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

    void ParallelNestedBenders::GenerateOptimalityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
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
                if ( dual_solution[row] < 0)
                    cut_rhs += dual_solution[row]*(*it)->ub();
                else if ( dual_solution[row] > 0)
                    cut_rhs += dual_solution[row]*(*it)->ub();
                row++;
                for_bound++;
            }
        }
        
        // Just print out the values of the cut-number 
        for_bound += benders_node_pointer->optimality_cuts_.size();
        // Append values for optimality and feasibility cuts.. ( if we have any at this node )
        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
            cut_rhs += (*it)->cut_->lb()*dual_solution[row];
            row++;
        }
        assert( row == for_bound);
        for_bound += benders_node_pointer->feasibility_cuts_.size();
        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->feasibility_cuts_.begin(); it != benders_node_pointer->feasibility_cuts_.end(); ++it){
            cut_rhs += (*it)->cut_->lb()*dual_solution[row];
            row++;
        }
        assert( row == for_bound);
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
		delete [] benders_node_pointer->dual_solution_;
		benders_node_pointer->dual_solution_ = NULL;

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

        bool change_aggregation_level = false;
        if ( num_cuts != benders_tree_node_pointer->getDataPtr()->num_thetas_ ){
            // We change the level of aggregation
            change_aggregation_level = true;
			createRoundRobinAggregation(benders_tree_node_pointer->getDataPtr(),num_childs,num_cuts,num_subproblems_per_cut);
            // We have to transform the old optimality cuts to the new number of theta's we would like to have
            // We already called a function that gives us a new number of whatever it is we need.
    
            // Transform all old optimality cuts to new ones.
            // Set new values
            benders_tree_node_pointer->getDataPtr()->num_thetas_ = num_cuts;
            benders_tree_node_pointer->getDataPtr()->active_cuts_ = boost::dynamic_bitset<>(num_cuts);
        }

        if ( num_cuts > 1 ) {
            // We can potentially aggregate some cuts
            // Use a potential policy here for the aggregation
            //std::vector< std::vector<int> > new_old_mapping;
            //findAggregationPossibilities(benders_tree_node_pointer->getDataPtr(),new_old_mapping,num_childs,num_cuts,num_subproblems_per_cut);
            //aggregateOldOptimalityCuts(benders_tree_node_pointer->getDataPtr(),new_old_mapping);

            // Look into all the cuts of 
        }

		// We have to set the start index again, but only if we used it ( marked by infinity )
		if ( current_stage_-1 == 0 ){
			if ( benders_tree_node_pointer->getDataPtr()->opt_cut_start_index_ == 0 || benders_tree_node_pointer->getDataPtr()->opt_cut_start_index_ == -1)
				benders_tree_node_pointer->getDataPtr()->opt_cut_start_index_ = benders_tree_node_pointer->getDataPtr()->optimality_cuts_.size();
		}

        int num_columns = stage_data_[current_stage_-1]->num_columns_;
        // Do some magic to find a good partition for the number of cuts we want to produce: TODO
        int added_cuts = 0;
        bool initialized_iterator = !benders_tree_node_pointer->getDataPtr()->optimality_cuts_.empty();
        if ( !benders_tree_node_pointer->getDataPtr()->optimality_cuts_.empty() )
            benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ = --benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end();

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
            if ( change_aggregation_level || sum > (benders_tree_node_pointer->getDataPtr()->num_thetas_ ? benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i] : 0) ){
                int new_index = 0;
                for (int index = 0; index < num_columns+num_theta_indices; index++){
                    if ( CompareWithZero(cut_elements[index]) ){
                        // Set cut element to zero, if near enough to zero, otherwise numerical difficulties arise
                        cut_elements[index] = 0;
                    }
                }
                new_index = num_columns+num_theta_indices;

                // Add cut
                boost::shared_ptr<OsiRowCut> cut_pointer(new OsiRowCut());
                cut_pointer->setLb(cut_lower_bound);
                cut_pointer->setUb(infinity_);
                cut_pointer->setRow(new_index,&cut_indices[0],&cut_elements[0]);
                boost::shared_ptr<CutStuff> cut_stuff(new CutStuff(current_iteration_,cut_pointer));
                cut_stuff->aggregate_number_ = i;
                benders_tree_node_pointer->getDataPtr()->optimality_cuts_.push_back(cut_stuff);
                if ( i == 0){
                    if ( benders_tree_node_pointer->getDataPtr()->optimality_cut_pointer_.empty())
                        benders_tree_node_pointer->getDataPtr()->optimality_cut_pointer_.resize(max_iteration_);
                    benders_tree_node_pointer->getDataPtr()->optimality_cut_pointer_[current_iteration_] = --benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end() ;
                }
                // Set aggregate to active: We have a cut for this aggregate
                benders_tree_node_pointer->getDataPtr()->active_cuts_[i] = true;
                // Add information about this cut to cut database!
                added_cuts++;

                // Store information about this new cut, we added a new one to our set!
                benders_tree_node_pointer->getDataPtr()->cut_information_.CreateCut(current_iteration_);
            }
        }// End for num_cuts
        // Add number of added cuts.
        if ( added_cuts ){
            if ( initialized_iterator ) 
                ++benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_; // Start at the correct one!
            else
                benders_tree_node_pointer->getDataPtr()->last_iteration_start_optimality_ = benders_tree_node_pointer->getDataPtr()->optimality_cuts_.begin();
        }

        add_cut_counter_mutex_.lock();
        added_optimality_cuts_[current_stage_-1] += added_cuts;
        add_cut_counter_mutex_.unlock();

        // Now we added all the cuts. Let's see if we can throw some of them away!
        // Num cuts is the number of aggregates we have here
        std::vector< std::list< BendersNodeData::cut_container::iterator > > cut_list(num_cuts);
        for ( BendersNodeData::cut_container::iterator it = benders_tree_node_pointer->getDataPtr()->optimality_cuts_.begin(); it != benders_tree_node_pointer->getDataPtr()->optimality_cuts_.end(); ++it){
            int index = (*it)->cut_->row().getIndices()[(*it)->cut_->row().getNumElements() - 1];

            if ( index > stage_data_[current_stage_-1]->num_columns_-1 ){ // Columns get indexed from 0 to num_columns-1, so if it is greater than that it is a theta var
                // This is the index we want
                if ( index > stage_data_[current_stage_-1]->num_columns_+num_cuts-1 )
                    LOG(INFO) << "This should not happen";
                cut_list[index-stage_data_[current_stage_-1]->num_columns_].push_back(it);
            }
            else
                LOG(INFO) << "This should not happen";
        }
        //if ( benders_tree_node_pointer->getDataPtr()->next_stage_approximation_ != NULL){
        //    // Prepare last column solution to work with!
        //    double* column_solution = new double[ stage_data_[current_stage_-1]->num_columns_+benders_tree_node_pointer->getDataPtr()->num_thetas_];
        //    memcpy(column_solution,benders_tree_node_pointer->getDataPtr()->col_solution_,stage_data_[current_stage_-1]->num_columns_*sizeof(double));
        //    memcpy(column_solution+stage_data_[current_stage_-1]->num_columns_,benders_tree_node_pointer->getDataPtr()->next_stage_approximation_,benders_tree_node_pointer->getDataPtr()->num_thetas_*sizeof(double));
        //    for (int i = 0; i < num_cuts; i++){
        //        // Go over all the aggregates and find the cuts that are "good"
        //        std::list< BendersNodeData::cut_container::iterator > delete_iterator;
        //        int number_of_cuts = 0;
        //        for ( std::list< BendersNodeData::cut_container::iterator >::iterator it = cut_list[i].begin(); it != cut_list[i].end(); ++it ){
        //            // Compute violation ( and distance
        //            // Column solution does not contain next_stage_approximation!!
        //            double violation = computeCutViolation( (*(*it))->cut_.get(), column_solution,infinity_);
        //            double euclidean_norm = computeEuclideanNorm((*(*it))->cut_.get());
        //            double distance = violation/euclidean_norm;
        //            number_of_cuts++;
        //        }
        //        // This is how many cuts we have now!
        //        // We want to throw away all cuts we do not like..
        //        // First cut = 
        //        BendersNodeData::cut_container::iterator first_cut = (*cut_list[i].begin());
        //        for ( std::list< BendersNodeData::cut_container::iterator >::iterator it = cut_list[i].begin(); it != cut_list[i].end(); ++it ){
        //            // Keep three cuts per scenario, the ones with the greatest distance!

        //        }

        //    }
        //    //if ( num_cuts > 1){ // We have not the single cut case, So we may need to find all the cuts that belong to a certain aggregate
        //    //    benders_tree_node_pointer->getDataPtr()->col_solution_; // This has to be present to check for violation?
        //    //    benders_tree_node_pointer->getDataPtr()->optimality_cuts_;
        //    //}
        //}
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
            
            //// Write the dual problem, to see what cplex solves
            //console_mutex_.lock();
            //double obj_shift = 0;
            //cpx_stat = CPXdualwrite(env_ptr,lp_ptr,"dual_test",&obj_shift);
            //console_mutex_.unlock();
            cpx_stat = CPXgetstat(env_ptr,lp_ptr);
            double proof = 0;
            if ( cpx_stat == CPX_STAT_INFEASIBLE ){
                if ( stage_data_[current_stage_]->non_zero_bounds_ && !use_dual_farkas){
                    cpx_stat = CPXgetpi(env_ptr,lp_ptr,dual_solution_farkas,0,number_rows-1);// If we use the primal method, we get the dual ray via the dual variables
                }
                else {
                    cpx_stat = CPXdualfarkas(env_ptr,lp_ptr,dual_solution_farkas,&proof);
                    LOG(INFO)<< "CPXdualfarks :" << cpx_stat;
                    if ( !(cpx_stat == CPX_STAT_OPTIMAL || cpx_stat == CPX_STAT_NUM_BEST) ) // If cpx_stat is not correct, we need to try the other stuff
                        use_cplex = false;
                    else if ( stage_data_[current_stage_]->non_zero_bounds_ ){ // We need to compute the values for the bounds! Where do we get the values? 
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
                                    result += dual_solution_farkas[vector.getIndices()[j]] * vector.getElements()[j];
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
                }
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
            if ( checked_dual_solution[i] >0 && CompareWithZero(checked_dual_solution[i]) )
                checked_dual_solution[i] = 0;

        ComputeCutCoefficients(benders_tree_node_pointer,checked_dual_solution,&cut_coefficients);

        // Compute fixed value for RHS
        
        // Compute the values: \lambda * lower_bound - \mu * upper_bound
        // Compute the resulting values right away, as we have the osi_solver_interface at hand.

        if ( !use_cplex ){ 
            dual_values_variable_bounds = const_cast<double*> (dual_ray_problem->getReducedCost());
            variable_lower_bounds = osi_solver->getColLower();
            variable_upper_bounds = osi_solver->getColUpper();
            double computed_dual_value_for_variable_bounds = 0;
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
        //// Append values for optimality and feasibility cuts.. ( if we have any at this node )
        // Optimality cuts should not count into the feasibility question
        //for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin(); it != benders_node_pointer->optimality_cuts_.end(); ++it){
        //    cut_rhs += (*it)->cut_->lb()*checked_dual_solution[row];
        //    row++;
        //}

        for_bound += benders_node_pointer->feasibility_cuts_.size();
        for ( BendersNodeData::cut_container::iterator it = benders_node_pointer->feasibility_cuts_.begin(); it != benders_node_pointer->feasibility_cuts_.end(); ++it){
            cut_rhs += (*it)->cut_->lb()*checked_dual_solution[row];
            row++;
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
			benders_tree_node_pointer->getParent()->getDataPtr()->feas_cut_start_index_ = benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.size();
            benders_tree_node_pointer->getParent()->getDataPtr()->last_iteration_start_feasibility_ = --benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.end();
        }
        
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
            // Silence solver object (Otherwise we get problems with the output (thread-related)
            osi_solver->messageHandler()->setLogLevel(0);
            // Clean the solver object by loading an empty problem ( that works for cplex, reset crashes cplex)
            osi_solver->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
            // We want to emphasize numerical stability / for CPLEX
            OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver);
            if ( cpx_ptr ){
                // Get ENV and LP ptr
                CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
                CPXsetintparam(env_ptr,CPX_PARAM_NUMERICALEMPHASIS,1);
                CPXsetdblparam(env_ptr,CPX_PARAM_TILIM,maximal_solution_time_);
                CPXsetintparam(env_ptr,CPX_PARAM_SCAIND,1);
                CPXsetdblparam(env_ptr,CPX_PARAM_EPMRK,0.9);
            }
            // Set primal algorithm
            osi_solver_map_[boost::this_thread::get_id()] = osi_solver;
        }
        return osi_solver;
    }

    void ParallelNestedBenders::ComputeCutCoefficients( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, const double* dual_solution, std::vector<double>* cut_coefficients_pointer )
    {
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
                    if ( stochastic_matrix.indices_[stochastic_column_index] >= current_stage_columns ){
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
        // We need to add the reduced costs for all the variables with bounds!
    }

    bool ParallelNestedBenders::CompareWithZero( double value ) 
    {
        return std::abs(value) < zero_tolerance_; // True, if zero
    }

	void ParallelNestedBenders::AppendCutsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
	{
		BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
		BendersStageData* stage_pointer = stage_data_[current_stage_];
		// Add the theta-stuff ( how many we have depends on current multi-cut aggregation level.. ) TODO: Reuse stuff
		if ( !benders_node_pointer->optimality_cuts_.empty() ){// We have some optimality cuts, so we need theta variables
			// Remove theta columns from solver, if we reuse it
			int cols_to_remove = osi_solver->getNumCols()-stage_pointer->num_columns_; // This indicates if we reuse a solver, apart from initial run.
			if ( cols_to_remove && benders_node_pointer->changed_aggregation_level_ ) { 
				std::vector<int> remove_indices;
				remove_indices.reserve(cols_to_remove);
				for (int i = stage_pointer->num_columns_; i < stage_pointer->num_columns_+cols_to_remove;i++)
					remove_indices.push_back(i);
				osi_solver->deleteCols(cols_to_remove,&remove_indices[0]);
				cols_to_remove = 0;

				// Remove all old cuts also.
				// Remove all cuts from previous solutions ( includes "Theta" Constraints )
				int cuts_to_remove = osi_solver->getNumRows()-stage_pointer->num_rows_;
				if ( cuts_to_remove ){
					std::vector<int> remove_indices;
					remove_indices.reserve(cuts_to_remove);
					for (int i = stage_pointer->num_rows_; i < stage_pointer->num_rows_+cuts_to_remove;i++)
						remove_indices.push_back(i);
					osi_solver->deleteRows(cuts_to_remove,&remove_indices[0]);
				}
				// Reset start indices for cuts
				benders_node_pointer->opt_cut_start_index_ = 0;
				benders_node_pointer->feas_cut_start_index_ = 0; // Also feas cuts, we deleted all old cuts..
            }
			if ( cols_to_remove == 0 ){ // We have no thetas yet, but optimality cuts. So we need thetas.
				// We set a theta value for every stage
				int num_cols = benders_node_pointer->num_thetas_;
				std::vector<double> lower(num_cols,-infinity_);
				std::vector<double> upper(num_cols,infinity_);
				std::vector<double> objective(num_cols,1); 
				CoinPackedVectorBase** coin_vector = new CoinPackedVectorBase*[num_cols];
				for (int i = 0; i < num_cols; i++){
					coin_vector[i] = new CoinShallowPackedVector(0,NULL,NULL,false);
				}
				// Set bounds of inactive cuts to 0
				int num_active_cuts = benders_node_pointer->active_cuts_.count();
                assert( num_active_cuts <= benders_node_pointer->num_thetas_ );
				if ( num_active_cuts < benders_node_pointer->num_thetas_ ){
					// We have less active cuts than num_thetas, so we have to fix some of these inactive variables to zero and remove them from the objective, as otherwise problem becomes unbounded
					for (int i = 0; i < benders_node_pointer->num_thetas_; i++){
						if ( benders_node_pointer->active_cuts_[i] == false){
							// Set variable fixed to zero
							lower[i] = 0; upper[i] = 0; objective[i] = 0;
						}
					}
				}
				osi_solver->addCols(num_cols,coin_vector,&lower[0],&upper[0],&objective[0]);
				assert(osi_solver->getNumCols() == stage_pointer->num_columns_+num_cols);
				for (int i = 0; i < num_cols; i++){
					delete coin_vector[i];
				}
				delete [] coin_vector;
			}
		}

		if ( !benders_node_pointer->optimality_cuts_.empty()){
			// Add the cuts
            BendersNodeData::cut_container::iterator it = benders_node_pointer->optimality_cuts_.begin();
			if ( seperate_first_stage_solver_ && !benders_node_pointer->changed_aggregation_level_ && current_stage_ == 0){
				it = benders_node_pointer->last_iteration_start_optimality_;
				benders_node_pointer->opt_cut_start_index_ = -1; // We used the start_index and signal this to the cut aggregation routine.
			}
			// TODO: This can be changed to addRows
			for ( ; it != benders_node_pointer->optimality_cuts_.end();++it) {
				osi_solver->applyRowCuts(1,(*it)->cut_.get());
			}
		}

		if ( !benders_node_pointer->feasibility_cuts_.empty()){
			// Add the cuts
			BendersNodeData::cut_container::iterator it = benders_node_pointer->feasibility_cuts_.begin();
			if ( seperate_first_stage_solver_ && current_stage_ == 0){
				it = benders_node_pointer->last_iteration_start_feasibility_;
				// If we apply feasibility cuts, we always go back and apply them right away. No need for setting the value to infinity, as in the optimality case.
			}
			// TODO: This can be changed to addRows
			for (; it != benders_node_pointer->feasibility_cuts_.end();++it) {
				osi_solver->applyRowCuts(1,(*it)->cut_.get());
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

    void ParallelNestedBenders::DeleteSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // We should delete this in an own method, after solving the problem, because we might need it?
        // Delete all the stuff we created
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        SmiNodeData* node_pointer = benders_node_pointer->smi_node_data_;
        if ( warm_up_ && current_stage_ == 0 && seperate_first_stage_solver_){
            // Restore bounds
            OsiSolverInterface* osi_solver = GetSolverByThread();
            for (int i = 0; i < stage_data_[0]->num_columns_; i++){
                osi_solver->setColBounds(i,benders_node_pointer->column_lower_[i],benders_node_pointer->column_upper_[i]);
            }
           /* osi_solver->setColLower(benders_node_pointer->column_lower_);
            osi_solver->setColUpper(benders_node_pointer->column_upper_);*/
            warm_up_ = false;
            warm_up_solution_.reset(); // Delete the content of the warm_up solution, not longer needed
            osi_solver->resolve(); // Prevents no solution exists error messages!
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

    void ParallelNestedBenders::aggregateOldOptimalityCuts( BendersNodeData* param1,  std::vector< std::vector<int> >& mapping )
    {
        // We need a place to store all the coefficients for the new cuts, that we want to add to the optimality cuts vector!

        // new number of cuts 
        int new_number_of_cuts_per_iteration = mapping.size();
        int total_cuts = param1->optimality_cuts_.size();

        int first_iteration_with_cuts = param1->cut_information_.GetCut(0).cut_creation_iteration_;
        for (int i = 0; i < total_cuts; i++){
            // We need all cuts of an iteration
        }

    }


    bool ComputeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon){
        if (upper_bound == infinity || lower_bound == -infinity)
            return true;
        if (upper_bound == 0) // We have no meaningful comparison
            return true;
        if (std::abs(upper_bound) < 1 && std::abs(lower_bound) < 1) //in this case we do not want relative difference, instead we want absolute difference..
            return true;
		return (std::abs(upper_bound-lower_bound)/std::abs(upper_bound)) >= epsilon; //See cplex mipgap
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
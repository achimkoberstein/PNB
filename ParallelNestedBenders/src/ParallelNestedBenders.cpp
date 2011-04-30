#include "ParallelNestedBenders.hpp"

#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>

#include <boost/foreach.hpp>
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
        // What do we want to do?
        // 1. Check for stochasticity of the model 
        for (int i = 0; i < num_stages_; i++)
            thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AdjustProblemInStage,this,i) );
        thread_pool_.wait(0);

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
        benders_stage_ptr->column_indices_.reserve(benders_stage_ptr->num_columns_);
        benders_stage_ptr->row_indices_.reserve(benders_stage_ptr->num_rows_);
        for ( int i = 0; i < benders_stage_ptr->num_columns_+1; i++ ){
            benders_stage_ptr->column_indices_.push_back(i);            
        }
        for ( int i = 0; i < benders_stage_ptr->num_rows_+1; i++ ){
            benders_stage_ptr->row_indices_.push_back(i);            
        }
        // End of CPLEX necessary adjustments

        // Look at core data to get values for the matrices..
        for ( int i = 0; i < tree_nodes_by_stage_[stage].size(); i++){
            // Look at current node
            BendersNodeData* benders_node_ptr = tree_nodes_by_stage_[stage][i]->getDataPtr();
            SmiNodeData* node_ptr = benders_node_ptr->smi_node_data_;
            if ( !node_ptr->isCoreNode()){ // Makes only sense for non-core nodes
                // Find stochastic T and W matrix
                if ( node_ptr->getNumMatrixElements() && !( benders_stage_ptr->stochastic_matrix_w_ && benders_stage_ptr->stochastic_matrix_t_) ){
                    // We split the matrix entries into W and T 
                    for ( int j = core_->getRowStart(stage); j < core_->getRowStart(stage)+benders_stage_ptr->num_rows_; j++){
                        //const double* row_elements = node_ptr->getRowElements(j);
                        const int* row_indices = node_ptr->getRowIndices(j);
                        int row_length = node_ptr->getRowLength(j);
                        for ( int k = 0; k < row_length; k++ ){
                            if ( row_indices[k] >= core_->getColStart(stage))
                                benders_stage_ptr->stochastic_matrix_w_ = true;
                            else
                                benders_stage_ptr->stochastic_matrix_t_ = true;
                        }
                        if ( benders_stage_ptr->stochastic_matrix_t_ && benders_stage_ptr->stochastic_matrix_w_ )
                            break; // We can skip the rest of the computation, if both booleans are true
                    }
                }
                if ( node_ptr->getCoreCombineRule() != SmiCoreCombineReplace::Instance() )
                    LOG(FATAL) << "The supplied SMPS format is not understood. You used something that is not REPLACE in your stoch file.";


                // Are there random bounds?
                if ( node_ptr->getLength(node_ptr->getCloStart()) || node_ptr->getLength(node_ptr->getCupStart()) )
                    benders_stage_ptr->stochastic_bounds_ = true;

                // Are there random objective function values?
                if ( node_ptr->getLength(node_ptr->getObjStart()) )
                    benders_stage_ptr->stochastic_objective_ = true;

                // Are there random right hand sides?
                if ( node_ptr->getLength(node_ptr->getRloStart()) || node_ptr->getLength(node_ptr->getRupStart()) )
                    benders_stage_ptr->stochastic_rhs_ = true;
            }
        }

    }

    ParallelNestedBenders::ParallelNestedBenders( SmiScnModel* smi_model_ptr ) : kT(1),kW(2),
        smi_(smi_model_ptr),core_(smi_->getCore()),osi_solver_(NULL),osi_solver_map_(),tree_(),tree_nodes_by_stage_(),stage_data_(),
        num_stages_(core_->getNumStages()),num_scenarios_(smi_->getNumScenarios()),infinity_(0),contains_integer_(false),integer_problems_(core_->getNumStages()),parameter_(),
        zero_tolerance_(0.0),integer_tolerance_(0.0),absolute_epsilon_(0.0),relative_epsilon_(0.0),
        current_iteration_(0),minor_iteration_(0),max_iteration_(1000),lower_bound_(-infinity_),upper_bound_((int)infinity_),temporary_upper_bound_(0),old_lower_bound_(-infinity_),
        current_stage_(0),forward_(true),infeasible_problem_(false),unbounded_problem_(false),infeasible_problems_(smi_->getNumScenarios()),added_optimality_cuts_(num_stages_,0),
        num_cpu_(0),console_mutex_(),solver_map_mutex_(),stage_warm_start_mutex_(),thread_pool_(0),
        warm_start_(false),warm_start_scenarios_(false),warm_start_stage_(false),only_one_feasibility_cut_(false),generate_feasibility_and_optimality_cuts_(false)
    {

    }

    void ParallelNestedBenders::Init()
    {
        // Initialize osiSolver
        if (smi_->getOsiSolverInterface() == NULL)
            osi_solver_ = new OsiCpxSolverInterface();
        else
            osi_solver_ = smi_->getOsiSolverInterface()->clone();

          // Set infinity to solver infinity
        infinity_ = osi_solver_->getInfinity();
        lower_bound_ = -infinity_;
        upper_bound_ = infinity_;
        old_lower_bound_ = lower_bound_;

        int num_theta = 1; // Single cut as default
        int num_scen_pro_cut = num_scenarios_; // Single cut as default.

        // Iterate over property map..
        boost::property_tree::ptree& param(parameter_.parameter_);

        // Tolerances
        zero_tolerance_ = param.get(PNB_TOLERANCE_ZERO,0.0);
        integer_tolerance_ = param.get(PNB_TOLERANCE_INTEGER,0.0);
        absolute_epsilon_ = param.get(PNB_EPSILON_ABSOLUTE,0.0);
        relative_epsilon_ = param.get(PNB_EPSILON_RELATIVE,0.0);
        // WarmStart
        int warm_start_version = param.get(PNB_WARM_START,0);
        if ( warm_start_version > 0 )
            warm_start_ = true;
        if ( warm_start_version == 1 )
            warm_start_scenarios_ = true;
        if ( warm_start_version == 2 )
            warm_start_stage_ = true;
        
        // Multicut Aggregation
        int value = param.get(PNB_AGGREGATED_OPTIMALITY_CUTS,1);
        if ( value <= 0 ){
            // Use something you like
            num_theta = 0;
            // Check if other values are OK
            num_scen_pro_cut = param.get(PNB_SCENARIOS_PRO_CUT,1); // If not specified, multicut
        }
        else
            num_theta = value;

		// Thread related stuff
		num_cpu_ = param.get(PNB_NUM_CPU,0);
		int available_cpus = boost::thread::hardware_concurrency();
		if ( num_cpu_ <= 0 )
			num_cpu_ = available_cpus;
		else
			num_cpu_ = std::min(num_cpu_,available_cpus);
		param.put(PNB_CPU_USED,num_cpu_);
		thread_pool_ = boost::threadpool::pool(num_cpu_); // Can we resize the threadpool?
		//thread_pool_ = boost::threadpool::pool(1); // Debugging reasonsTODO: Remove again

        // Do a quick check with probabilities of the model.
        double total_prob = 0;
        for (int i = 0; i < num_scenarios_; i++){
            total_prob += smi_->getScenarioProb(i);
        }
        LOG_IF(WARNING,!CompareForEquality(1,total_prob,ParallelNestedBenders::zero_tolerance_)) << "Your model probabilties do not sum up to one.";

        // create our benders node objects and build the tree
        tree_ = SmiScenarioTree<BendersNodeData*>();
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
        if ( num_theta == 0 ){
			tree_nodes_by_stage_[0].back()->getDataPtr()->num_scen_pro_cut_ = num_scen_pro_cut;
        }
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
				    // Set multicut aggregation
                    if ( num_theta == 0 ){
	                    tree_nodes_by_stage_[i].back()->getDataPtr()->num_scen_pro_cut_ = num_scen_pro_cut;
                    }
                }
                delete children;
            }
        }


		// Initialize all benders node data objects with the correct cut aggregation policy. TODO

    }

    void ParallelNestedBenders::Solve()
    {
        AdjustProblem();
        // We can stop here, if we have integer variables on more than the second stage..
        if ( integer_problems_[0] ){ // In the first stage.
            LOG(INFO) << "Your problem contains first-stage integer variables.";
            //return;
        }
        for (int i = 1; i < num_stages_;i++)
            LOG_IF(WARNING,integer_problems_[i]) << "Your problem contains integer variables at stage " << i+1 << ". We solve the LP relaxation for that stage.";
        
        int cut_counter = 0;
        // Do initialization stuff necessary for solve process
        while ( std::abs(upper_bound_-lower_bound_) > absolute_epsilon_ &&  
            ComputeRelativeGap(upper_bound_,lower_bound_,infinity_,relative_epsilon_) &&
            current_iteration_ < max_iteration_ 
            ) 
        {
            if ( upper_bound_ <= lower_bound_ ){
                LOG(INFO) << "Upper Bound is lower than lower Bound. This can not happen, unfortunately it has. Iteration: " << current_iteration_;
                //break;
            }
            if ( old_lower_bound_ > (lower_bound_ + ParallelNestedBenders::zero_tolerance_) && old_lower_bound_ > -infinity_  && !infeasible_problem_){// TODO: If we have feasCuts we might have to look at this again..
                LOG(INFO) << "New Lower bound " << lower_bound_ << " is less than old lower bound " << old_lower_bound_;
                //break;
            }

            minor_iteration_++;
            if ( current_stage_ == 0 ){ // Things to do if we are in the first stage
                current_iteration_++;
                // Cleanup Added cuts counter
                for (int i = 0; i < num_stages_; i++){
                    cut_counter += added_optimality_cuts_[i];
                    added_optimality_cuts_[i] = 0;
                }
            }

            // Change direction depending on last or first stage
            if ( current_stage_ == 0 ){
                forward_ = true;
                // If we had infeasible problems in the last backward run, we added all the feas cuts and can start fresh
                infeasible_problem_ = false;
            }
            if ( current_stage_ == num_stages_ -1)
                forward_ = false;

            //// Change direction depending on whatever we like
            if ( (current_iteration_ % 2 == 0) && lower_bound_ > -infinity_ && upper_bound_ < infinity_){
                // Try some fancy stuff: Solve the first stage several times
                if ( current_stage_ == 2)
                    forward_ = false;

            }

            std::string direction(forward_ ? "F " : "B ");
            std::cout << minor_iteration_ << "Iteration " << direction << current_iteration_ << "." << current_stage_ << std::endl;
            if ( current_stage_ == 0){
                std::cout << "Lower Bound:  " << lower_bound_ << std::endl;
                std::cout << "Upper Bound:  " << upper_bound_ << std::endl;
                std::cout << "Relative Gap: " << abs(upper_bound_-lower_bound_)/abs(upper_bound_)  << std::endl;
            }

            int num_nodes = tree_nodes_by_stage_[current_stage_].size();
            for ( int i = 0; i < num_nodes; i++ ){
                // Schedule the tasks for this stage
                thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]));
            }
            thread_pool_.wait(0); // We wait until all threads are done

            // Calculate number of feasibility cuts that were added
            

            // If we had unbounded problems, we have to stop here
            if ( unbounded_problem_ ){
                LOG(ERROR) << "Your problem got unbounded in stage " << current_stage_+1;
                break;
            }

            if ( !forward_ && !infeasible_problem_ && infeasible_problems_.none() ){ 
                // We need to aggregate optimality cuts for the parent node
                for ( int i = 0; i < tree_nodes_by_stage_[current_stage_-1].size(); i++)
                    // Schedule the tasks for this stage
                    thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AggregateOptimalityCuts,this,tree_nodes_by_stage_[current_stage_-1][i]));
                thread_pool_.wait(0); // We wait until all threads are done
                // If we added no cuts, we might want to notice that
                LOG_IF(INFO,added_optimality_cuts_[current_stage_]==0) << "We added no new optimality cuts in stage " << current_stage_;
            }

            // Update upper bound if we solved all problems in this iteration once
            // Lower bound is updated in the StoreData Method after solving first stage problem
            if ( current_stage_ == num_stages_ -1 && infeasible_problems_.none()){
                if ( upper_bound_ > temporary_upper_bound_ )
                    upper_bound_ = temporary_upper_bound_;
            }

            // If we have an infeasible subproblem, we already added the cuts to the corresponding parent problems.
            if ( infeasible_problems_.any() ){
                // Set boolean variable to indicate that during this backward run, we do not generate optimality cuts!.
                infeasible_problem_ = true;
                if ( current_stage_ == 0){
                    infeasible_problem_ = true;
                    LOG(INFO) << "First stage problem is infeasible.";
                    // We have to stop the algorithm, our master problem is infeasible..
                    break;
                }
                // Change direction
                forward_ = false;
                // Reset infeasible_problems_ set
                infeasible_problems_.reset();
            }

            // Change next stage, depending on where we are heading (forward or backward)
            forward_ ? current_stage_++ : current_stage_--;

            // We change the stage, so we should clear all of the osiSolverInterfaces
            std::pair<boost::thread::id, OsiSolverInterface* > id_solver_pair;
            BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
                //id_solver_pair.second->reset(); // This method does not really work for OsiCpx, I do not know why.
                id_solver_pair.second->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                // Workaround: Load empty problem. 
            }

        }
        LOG(INFO) << "Added " << cut_counter << " optimality cuts.";
        // We finished solving
        if (!infeasible_problem_)
            LOG(INFO) << "Solution: " << upper_bound_;
        else{
            upper_bound_ = -infinity_;
            LOG(INFO) << "Problem is infeasible.";
        }

    }


    void ParallelNestedBenders::HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Check if we already got infeasible subproblems only, if we generate either feas or opt cuts
        if ( !generate_feasibility_and_optimality_cuts_ && only_one_feasibility_cut_ )
            if ( infeasible_problems_.any() )
                return; 
        // Get thread ID
        //console_mutex_.lock();
        //LOG(INFO) << "Current thread has ID: " << boost::this_thread::get_id();
        //console_mutex_.unlock();
        OsiSolverInterface* osi_solver = GetSolverByThread();

        BuildSubproblem(osi_solver, benders_tree_node_pointer);
        SolveSubproblem(osi_solver,benders_tree_node_pointer);
        // We only need to store data if we have a feasible solution for the subproblem.
        if ( !infeasible_problems_[benders_tree_node_pointer->scenario()]){
            StoreData(osi_solver,benders_tree_node_pointer);
            if ( !forward_)
                GenerateOptimalityCut(benders_tree_node_pointer);
        }
    }

    void ParallelNestedBenders::SolveSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer  ){
	    // Warm Start Handling
        
        // Depending on Warm Start behaviour, use the corresponding entry
        // If WarmstartForScenario or WarmstartForStage do the following
        if ( warm_start_ ){
            if ( warm_start_scenarios_ ){
                if ( benders_tree_node_pointer->getDataPtr()->warm_start_  ) // If we have a warm start object, we tell the solver to use it
                    osi_solver->setWarmStart(benders_tree_node_pointer->getDataPtr()->warm_start_);
            }
            else if ( warm_start_stage_ ){ // If we have a warm start object, we tell the solver to use it. We need a mutex, because we do not have read-only access.
                stage_warm_start_mutex_.lock(); 
                osi_solver->setWarmStart(stage_data_[current_stage_]->warm_start_);
                stage_warm_start_mutex_.unlock();
            }
        }
        // MIP Solving only at first stage..
        if ( current_stage_ == 0 && integer_problems_[current_stage_] ){
             // We need to set the integer indices.
             std::vector<int> integer_indices = core_->getIntCols(current_stage_);
             osi_solver->setInteger(&integer_indices[0],integer_indices.size());
             osi_solver->branchAndBound(); // We might need a fixed first-stage solver for this ( preferable from the viewpoint of performance anyway.. ) 

        }
        else
            osi_solver->initialSolve();
        //if ( current_stage_ == 0){ // Only write solution file if we are in the first stage..
        //    std::stringstream stream;
        //    stream << "stage" << current_stage_ << "_" << current_iteration_ << "_" << benders_tree_node_pointer->scenario();
        //    console_mutex_.lock();
        //    osi_solver->writeLp(stream.str().c_str());
        //    console_mutex_.unlock();
        //}
        // Result/Error Handling: Introduce a global variable, when feasibility problems occurs..
        // Check for errors
        if ( !osi_solver->isProvenOptimal() ){
            // We need to check for errors in this case
            if ( osi_solver->isAbandoned() ){
                //Problem
                LOG(FATAL) << "Solution process abandoned.";
            }
            if ( osi_solver->isProvenDualInfeasible() ){
                // Primal problem is unbounded. => dual infeasible
                // If we are in a problem with theta variables, we might have sucess in restricting the theta variables
                if ( current_stage_ < num_stages_-1){
                    // Put bound on theta variable via row, column bounds do not cut it ( do not know why )
                    int num_thetas = benders_tree_node_pointer->getDataPtr()->num_thetas_;
                    boost::scoped_array<int> extra_indices(new int[num_thetas]);
                    boost::scoped_array<double> extra_elements(new double[num_thetas]);
                    for (int i = 0; i < num_thetas; i++){
                        extra_indices[i] = stage_data_[current_stage_]->num_columns_+i;
                        extra_elements[i] = 1;
                    }
                    // Throws error, do not know why
                    osi_solver->addRow(num_thetas,extra_indices.get(),extra_elements.get(),-infinity_ + 1000000, infinity_);
                    // Solve the model again
                    osi_solver->resolve();
                }
                if ( osi_solver->isProvenPrimalInfeasible() ){
                    // Go below for feasibility cut
                }
                else if ( !osi_solver->isProvenOptimal()){
                    if ( osi_solver->isProvenDualInfeasible() ){
                        LOG(ERROR)<< "Problem for scenario " << benders_tree_node_pointer->scenario() << " in stage " << current_stage_ << " is primal unbounded.";
                        upper_bound_ = infinity_;
                        unbounded_problem_ = true;
                    }
                    
                    // In case of CPLEX, we can return the error code..
                    OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
                    if ( cpx_solver ){
                        CPXENVptr env = cpx_solver->getEnvironmentPtr();
                        CPXLPptr lp = cpx_solver->getLpPtr();
                        // Check status code
                        int error = CPXgetstat(env,lp);
                        LOG(WARNING) << "Solver returned with status code." << error;
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
                    DLOG(INFO) << "Generate feasibility cut for scenario " << benders_tree_node_pointer->scenario() << " in stage " << current_stage_;
                    GenerateFeasibilityCut( osi_solver, benders_tree_node_pointer ); 
                }
                else
                    LOG(ERROR) << "Your master problem is infeasible.";
                
            }
            
            // TODO Other possible cases

        }// if not provenOptimal end
        else { // Proven Optimal: 
            if ( warm_start_ && osi_solver->optimalBasisIsAvailable()) // Store WarmStart Information, otherwise we reuse the old one..
                if ( warm_start_scenarios_ ){
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
        delete [] benders_node_pointer->next_stage_approximation_;
        // Set to null
        benders_node_pointer->col_solution_ = NULL;
        benders_node_pointer->dual_solution_ = NULL;
        benders_node_pointer->next_stage_approximation_ = NULL;
        
        if ( forward_ || current_stage_ == num_stages_-1 ){ // We need the upper bound
            // We want to differentiate between column solution and theta variables 
            benders_node_pointer->col_solution_ = new double[stage_data_[current_stage_]->num_columns_];
            memcpy(benders_node_pointer->col_solution_,osi_solver->getColSolution(),stage_data_[current_stage_]->num_columns_*sizeof(double));
            double solution_value = 0;
            for (int i = 0; i < stage_data_[current_stage_]->num_columns_;i++){
                if ( CompareWithZero(benders_node_pointer->col_solution_[i]))
                    benders_node_pointer->col_solution_[i] = 0;
                solution_value += benders_node_pointer->col_solution_[i]*osi_solver->getObjCoefficients()[i];
            }
            if ( current_stage_ == 0) // This work in the multithreaded cases, as we have only one thread for the first stage problem
                temporary_upper_bound_ = 0;
            upper_bound_mutex_.lock();
            temporary_upper_bound_ += solution_value*benders_node_pointer->probability_;
            upper_bound_mutex_.unlock();

            //console_mutex_.lock();
            //// Write to file
            //ofstream myfile;
            //std::stringstream filename;
            //filename << "iteration_" << current_iteration_;
            //myfile.open (filename.str().c_str(),fstream::out | fstream::app);
            //myfile << benders_tree_node_pointer->scenario() << " " << solution_value*benders_node_pointer->probability_ << std::endl;
            //myfile.close();
            //console_mutex_.unlock();

        }

        // Copy theta variables
        int num_theta = osi_solver->getNumCols()-stage_data_[current_stage_]->num_columns_;
        assert(num_theta >= 0);
        
        // Retrieve and store the theta variables
        benders_node_pointer->next_stage_approximation_ = new double[num_theta];
        memcpy(benders_node_pointer->next_stage_approximation_,osi_solver->getColSolution()+stage_data_[current_stage_]->num_columns_,num_theta*sizeof(double) );

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

            // Compute the values: \lambda * lower_bound - \mu * upper_bound
            // Compute the resulting values right away, as we have the osi_solver_interface at hand.
            const double* dual_values_variable_bounds = osi_solver->getReducedCost();
            const double* variable_lower_bounds = osi_solver->getColLower();
            const double* variable_upper_bounds = osi_solver->getColUpper();
            double computed_dual_value_for_variable_bounds = 0;
            const double* column_solution = osi_solver->getColSolution();
            for (int i = 0; i< stage_data_[current_stage_]->num_columns_; i++){
                // Look at variable value and their bounds
                if ( !CompareWithZero(column_solution[i]) && !CompareWithZero(dual_values_variable_bounds[i]) ){ // If variable is zero, it is either at one of its bounds ( then we do not want to worry ) or it is basic and we also do not have to worry..
                    
                    // We have to look at bounds TODO: ZEROTEST For reduced cost
                    if ( CompareForEquality( column_solution[i], variable_lower_bounds[i],zero_tolerance_) )
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_lower_bounds[i];
                    else if ( CompareForEquality( column_solution[i], variable_upper_bounds[i],zero_tolerance_))
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_upper_bounds[i]; 
                }
            }
            benders_node_pointer->dual_value_variable_bounds_ = computed_dual_value_for_variable_bounds;
        }
    }

    void ParallelNestedBenders::BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        BendersStageData* stage_pointer = stage_data_[current_stage_];
        SmiNodeData* node_pointer = benders_node_pointer->smi_node_data_; // Stochastic node pointer
        SmiNodeData* deterministic_node_pointer = core_->getNode(current_stage_); // Deterministic node (there is one per stage)
        bool reuse_solver = true;
        if ( node_pointer->hasMatrix() || osi_solver->getNumCols() == 0){ // We have to load the matrix / change the matrix => load the problem
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
                            else if (stochastic_column_index==stochastic_matrix.start_[row+1]) {
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
                        if (benders_node_pointer->row_lower_[row] == benders_node_pointer->row_upper_[row])
                                //LOG(INFO) << "You have used a ranged constraint.";
                        benders_node_pointer->row_lower_[row] -= value; // lb == row == ub, we can not handle lb <= row <= ub with one real inequality ( have to split up to two rows somewhere..
                        benders_node_pointer->row_upper_[row] -= value;
                    }
                }
            }
        }


        // Load the basic problem
        // We call this method if we just constructed the matrix for the first time
        if ( !reuse_solver )
            osi_solver->loadProblem(*benders_node_pointer->matrix_,benders_node_pointer->column_lower_,benders_node_pointer->column_upper_,benders_node_pointer->objective_,benders_node_pointer->row_lower_,benders_node_pointer->row_upper_);
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
        }

        // Add the theta-stuff ( how many we have depends on current multi-cut aggregation level.. ) TODO: Reuse stuff
		if ( !benders_node_pointer->optimality_cuts_.empty() ){//&& !reuse_solver){// We have some optimality cuts, so we need theta variables
            // Remove theta columns from solver, if we reuse it
			int cols_to_remove = osi_solver->getNumCols()-stage_pointer->num_columns_;
			if ( cols_to_remove ) {
				std::vector<int> remove_indices;
				remove_indices.reserve(cols_to_remove);
				for (int i = stage_pointer->num_columns_; i < stage_pointer->num_columns_+cols_to_remove;i++)
					remove_indices.push_back(i);
				osi_solver->deleteCols(cols_to_remove,&remove_indices[0]);
			}
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
            if ( num_active_cuts < benders_node_pointer->num_thetas_ ){
                // We have less active cuts than num_thetas, so we have to fix some of these inactive variables to zero and remove them from the objective.
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

        if ( !benders_node_pointer->optimality_cuts_.empty())
            // Add the cuts
            for (int i = 0; i<benders_node_pointer->optimality_cuts_.size();i++) {
                osi_solver->applyRowCuts(1,benders_node_pointer->optimality_cuts_[i]);
            }

        if ( !benders_node_pointer->feasibility_cuts_.empty())
            // Add the cuts
            for (int i = 0; i<benders_node_pointer->feasibility_cuts_.size();i++) {
                osi_solver->applyRowCuts(1,benders_node_pointer->feasibility_cuts_[i]);
            }

        // Delete all the stuff we created
        delete benders_node_pointer->matrix_; benders_node_pointer->matrix_ = NULL;
        if ( node_pointer->getColLowerLength())
            delete [] benders_node_pointer->column_lower_; benders_node_pointer->column_lower_ = NULL;
        if ( node_pointer->getColUpperLength())
            delete [] benders_node_pointer->column_upper_; benders_node_pointer->column_upper_ = NULL;

        if ( node_pointer->getObjectiveLength())
            delete [] benders_node_pointer->objective_; benders_node_pointer->objective_ = NULL;

        delete [] benders_node_pointer->row_upper_; benders_node_pointer->row_upper_ = NULL;
        delete [] benders_node_pointer->row_lower_; benders_node_pointer->row_lower_ = NULL;



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
        for_bound += benders_node_pointer->optimality_cuts_.size();
        // Append values for optimality and feasibility cuts.. ( if we have any at this node )
        for ( ; row < for_bound; row++ ){
            cut_rhs += benders_node_pointer->optimality_cuts_[row-stage_pointer->num_rows_]->lb()*dual_solution[row];
        }
        for_bound += benders_node_pointer->feasibility_cuts_.size();
        for ( ; row < for_bound; row++ ){
            cut_rhs += benders_node_pointer->feasibility_cuts_[row-stage_pointer->num_rows_-benders_node_pointer->optimality_cuts_.size()]->lb()*dual_solution[row];
        }
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
        


    }

    void ParallelNestedBenders::AggregateOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer ){
        assert(!forward_);
        if ( infeasible_problems_.any() ){
            LOG(WARNING) << "Attempting to generate optimality cuts in the presence of infeasible problems in this stage.";
            return;
        }
        //console_mutex_.lock();
        //LOG(INFO) << "Aggregating cuts";
        //console_mutex_.unlock();
        // We specified the number of thetas in the stage_pointer (for now)
        int num_cuts = stage_data_[current_stage_-1]->num_thetas_;
        // We need all the children of this node..
        int num_childs = benders_tree_node_pointer->numChildren();
        num_cuts = std::min(num_cuts,num_childs);
        // Write that back to stage_data? No, as number of childs can differ between nodes.
		// Num Subproblem per cuts
		int num_subproblems_per_cut = std::ceil( ((double)num_childs)/num_cuts );
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
        //assert(num_cuts <= num_childs);
        // Num cuts is num_thetas_ 

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
        
        int num_columns = stage_data_[current_stage_-1]->num_columns_;
        // Do some magic to finde a good partition for the number of cuts we want to produce: TODO
        int added_cuts = 0;
        for ( int i = 0; i < num_cuts; i++){
            // Start a new OsiRowCut
            double cut_lower_bound = 0;
            std::vector<double> cut_elements(num_columns+1,0);
            cut_elements[num_columns] = 1;
            std::vector<int> cut_indices;
            cut_indices.reserve(num_columns+1);
            for (int j = 0; j < num_columns; j++)
                cut_indices.push_back(j);
            cut_indices.push_back(num_columns+i);
            // For all the indices, that belong to this cut.
            std::vector<int>& nodes_in_cut(benders_tree_node_pointer->getDataPtr()->nodes_in_cut_[i]);
            for ( int j = 0; j < nodes_in_cut.size(); j++){
                for (int k = 0; k < num_columns; k++)
                        cut_elements[k] += tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->current_cut_coefficients_[k];
                cut_lower_bound += tree_nodes_by_stage_[current_stage_][nodes_in_cut[j]]->getDataPtr()->current_cut_rhs_;
                 
            }
            int new_index = 0;
            for (int index = 0; index < num_columns+1; index++){
                if ( CompareWithZero(cut_elements[index]) ){
                    // Set cut element to zero.
                    cut_elements[index] = 0;
                }
                else if ( index > new_index ){
                    cut_elements[new_index] = cut_elements[index];
                    cut_indices[new_index] = cut_indices[index];
                    new_index++;
                }
                else{
                    new_index++;
                }
            }
            // Check if cut is not redundant: Conditional Expected Value of Subproblems (theta) is greater than before. For implementation reasons, we only do this if we do not change aggregation level..

            // Theta = cut lower bound - cut_vector * column_solution
            // Compute cut_vector * column_solution
            const double * column_solution = benders_tree_node_pointer->getDataPtr()->col_solution_;
            double sum = 0.0;
            for ( int index = 0;  index < new_index-1;  index++ ) { // We iterate over column_solution, but not over the theta index, which is always the last one.
                int column_index = cut_indices[index];
                sum += column_solution[column_index] * cut_elements[index];
            }
            // Add computed cut right hand side
            sum = cut_lower_bound-sum+zero_tolerance_;
            // Check with previous stuff only, if we did nit changed the aggregation level.
            if ( change_aggregation_level || sum > (benders_tree_node_pointer->getDataPtr()->num_thetas_ ? benders_tree_node_pointer->getDataPtr()->next_stage_approximation_[i] : 0) ){
                // Add cut
                OsiRowCut* cut_pointer = new OsiRowCut();
                cut_pointer->setLb(cut_lower_bound);
                cut_pointer->setUb(infinity_);
                cut_pointer->setRow(new_index,&cut_indices[0],&cut_elements[0]);
                //LOG_IF(INFO,new_index<num_columns+1) << "We spared to store " << num_columns+1-new_index << " many indices, in percent " << 1-(double)new_index/(num_columns+1) << "% less storage";
                benders_tree_node_pointer->getDataPtr()->optimality_cuts_.push_back(cut_pointer);
                // Set cut to active
                benders_tree_node_pointer->getDataPtr()->active_cuts_[i] = true;
                added_cuts++;
            }
            else { // Cut is redundant
                //Clean up
                //LOG(INFO) << "Just for debugging reasons";

            }
            

        }// End for num_cuts
        // Add number of added cuts.
        add_cut_counter_mutex_.lock();
        added_optimality_cuts_[current_stage_] += added_cuts;
        add_cut_counter_mutex_.unlock();
    }

    void ParallelNestedBenders::GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    { // Called directly from solve subproblem.
        // We need the dual rays..
        assert(osi_solver->isProvenPrimalInfeasible()); // Otherwise we do not need to call this method
        bool cplex_licence_problem = true;
        OsiSolverInterface* dual_ray_problem = osi_solver;
        if ( !cplex_licence_problem ){
            // Get the dual ray
            osi_solver_mutex_.lock();
            dual_ray_problem = osi_solver->clone(true);
            osi_solver_mutex_.unlock(); 
        }
        GenerateDualRayProblem(dual_ray_problem); // Append columns to the existing problems, to allow feasible solutions.
        double objValue = dual_ray_problem->getObjValue();

        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        SmiNodeData* node_pointer = benders_tree_node_pointer->getDataPtr()->smi_node_data_;
        BendersStageData* stage_pointer = stage_data_[current_stage_];

        // Compute the cut coefficients for the matrix
        std::vector<double> cut_coefficients(stage_data_[current_stage_-1]->num_columns_,0);
        const double* dual_solution = dual_ray_problem->getRowPrice();
        // Copy solution to enable zero bound check
        double* checked_dual_solution = new double[dual_ray_problem->getNumRows()];
        memcpy(checked_dual_solution,dual_solution,sizeof(double)*dual_ray_problem->getNumRows());
        for ( int i = 0; i < dual_ray_problem->getNumRows(); i++)
            if ( checked_dual_solution[i] >0 && CompareWithZero(checked_dual_solution[i]) )
                checked_dual_solution[i] = 0;

        ComputeCutCoefficients(benders_tree_node_pointer,checked_dual_solution,&cut_coefficients);

        // Compute fixed value for RHS
        
        // Compute the values: \lambda * lower_bound - \mu * upper_bound
        // Compute the resulting values right away, as we have the osi_solver_interface at hand.
        const double* dual_values_variable_bounds = dual_ray_problem->getReducedCost();
        const double* variable_lower_bounds = osi_solver->getColLower();
        const double* variable_upper_bounds = osi_solver->getColUpper();
        double computed_dual_value_for_variable_bounds = 0;
        const double* column_solution = dual_ray_problem->getColSolution(); // We need to check that to: Write method for that..

        for (int i = 0; i< stage_data_[current_stage_]->num_columns_; i++){
            // Look at variable value and their bounds
            if ( !CompareWithZero(column_solution[i]) && !CompareWithZero(dual_values_variable_bounds[i]) ){ // If variable is zero, it is either at one of its bounds ( then we do not want to worry ) or it is basic and we also do not have to worry..

                // We have to look at bounds TODO: ZEROTEST For reduced cost
                if ( CompareForEquality( column_solution[i], variable_lower_bounds[i],zero_tolerance_) )
                    computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_lower_bounds[i];
                else if ( CompareForEquality( column_solution[i], variable_upper_bounds[i],zero_tolerance_))
                    computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_upper_bounds[i]; 
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
        // Append values for optimality and feasibility cuts.. ( if we have any at this node )
        //for ( ; row < for_bound; row++ ){
        //    cut_rhs += benders_node_pointer->optimality_cuts_[row-stage_pointer->num_rows_]->lb()*dual_solution[row];
        //}
        row += benders_node_pointer->optimality_cuts_.size();
        for_bound += benders_node_pointer->optimality_cuts_.size()+benders_node_pointer->feasibility_cuts_.size();
        for ( ; row < for_bound; row++ ){
            cut_rhs += benders_node_pointer->feasibility_cuts_[row-stage_pointer->num_rows_-benders_node_pointer->optimality_cuts_.size()]->lb()*checked_dual_solution[row];
        }
        // Append value for dual variable bounds
        cut_rhs += computed_dual_value_for_variable_bounds;

        // Add cut to set of feasibility cuts of parent problem
        OsiRowCut* cut_pointer = new OsiRowCut();
        cut_pointer->setLb(cut_rhs);
        cut_pointer->setUb(infinity_);
        cut_pointer->setRow(stage_data_[current_stage_-1]->num_columns_,&stage_data_[current_stage_-1]->column_indices_[0],&cut_coefficients[0]);
        benders_tree_node_pointer->getParent()->getDataPtr()->feasibility_cuts_.push_back(cut_pointer);

        // Increase feasCutCounter, together with locking/unlocking a mutex..


        // Deal with it. What about two-sided rows? TODO: Applies to optimality cuts as well

        // Delete dual ray problem
        if ( !cplex_licence_problem )
            delete dual_ray_problem;
        else {
            // Reset the solver interface
            dual_ray_problem->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        }

        // Delete the computed lower and upper bounds
        if ( node_pointer->getRowLowerLength())
            delete row_lower;
        if ( node_pointer->getRowUpperLength())
            delete row_upper;




    }

    OsiSolverInterface* ParallelNestedBenders::GetSolverByThread(){
        boost::unique_lock<boost::mutex> lock(solver_map_mutex_);
        OsiSolverInterface* osi_solver = osi_solver_map_[boost::this_thread::get_id()];
        if ( osi_solver == NULL ){
            // Create new osi_solver object
            osi_solver = osi_solver_->clone();
            // Silence solver object (Otherwise we get problems with the output (thread-related)
            osi_solver->messageHandler()->setLogLevel(0);
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
        for (int row = 0; row < stage_pointer->num_rows_; row++){ // For every row we have to calculate the correct value that has to added to rlo or rup
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
    }

    bool ParallelNestedBenders::CompareWithZero( double value ) 
    {
        return std::abs(value) < zero_tolerance_;
    }

    bool ComputeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon){
        if (upper_bound == infinity || lower_bound == -infinity)
            return true;
        if (upper_bound == 0) // We have no meaningful comparison
            return true;
        if (std::abs(upper_bound) < 1 && std::abs(lower_bound) < 1) //in this case we do not want relative difference, instead we want absolute difference..
            return true;
        return (abs(upper_bound-lower_bound)/abs(upper_bound)) >= epsilon; //See cplex mipgap
    }

    RowOrderedSparseMatrixPointer CreateRowOrderedSparseMatrixPointer( SmiNodeData* smiNode )
    {
        return RowOrderedSparseMatrixPointer(smiNode->getElements(smiNode->getMatStart()),smiNode->getIndices(smiNode->getMatStart()),smiNode->getStarts(smiNode->getMatStart()));
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
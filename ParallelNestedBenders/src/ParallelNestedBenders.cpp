#include "ParallelNestedBenders.hpp"

#include <iostream>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>
// Include all the other files
#include "OsiRowCut.hpp"
#include "OsiCpxSolverInterface.hpp"
#include "SmiScnModel.hpp"


namespace parallel_benders {
    // Define static variables
    double ParallelNestedBenders::zero_tolerance_ = std::pow(10.0,-6);

    void ParallelNestedBenders::AdjustProblem()
    {
        // Check for integrality right away
        if ( core_->getIntegerLength() || core_->getBinaryLength()){ // Set boolean variable (For now)
            contains_integer_ = true;
            return;
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

        for ( int i = 0; i < core_->getColStart(stage)+benders_stage_ptr->num_columns_; i++ ){
            // It is not necessary to have a strict staircase structure, so to the T-matrix belong all columns that are < then current stage
            if ( core_->getColStage(i) < stage ){
                benders_stage_ptr->t_indices_.push_back(i);
            }
            else{
                benders_stage_ptr->w_indices_.push_back(i);
            }
        }
        assert( benders_stage_ptr->t_indices_.size() + benders_stage_ptr->w_indices_.size() == core_->getColStart(stage)+benders_stage_ptr->num_columns_);

        // Look at core data to get values for the matrices..
        for ( int i = 0; i < tree_nodes_by_stage_[stage].size(); i++){
            // Look at current node
            BendersNodeData* benders_node_ptr = tree_nodes_by_stage_[stage][i]->getDataPtr();
            SmiNodeData* node_ptr = benders_node_ptr->smi_node_data_;
            if ( !node_ptr->isCoreNode()){ // Makes only sense for non-core nodes
                // Build stochastic T and W matrix
                if ( node_ptr->getNumMatrixElements() && !( benders_stage_ptr->stochastic_matrix_w_ && benders_stage_ptr->stochastic_matrix_t_) ){
                    // We split the matrix entries into W and T 
                    for ( int j = 0; j < benders_stage_ptr->num_rows_; j++){
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
        num_stages_(0),num_scenarios_(0),infinity_(0),contains_integer_(false),
        integer_tolerance_(0.0),absolute_epsilon_(0.0),relative_epsilon_(0.0),
        current_iteration_(0),minor_iteration_(0),max_iteration_(1000),lower_bound_(-infinity_),upper_bound_((int)infinity_),temporary_upper_bound_(0),old_lower_bound_(-infinity_),
        current_stage_(0),forward_(true),infeasible_problem_(false),
        num_cpu_(0),thread_pool_(0)
    {
        Init();

    }

    void ParallelNestedBenders::Init()
    {
        // Set number of stages and scenarios
        num_scenarios_ = smi_->getNumScenarios();
        num_stages_ = core_->getNumStages();

        // Do a quick check :
        double total_prob = 0;
        for (int i = 0; i < num_scenarios_; i++){
            total_prob += smi_->getScenarioProb(i);
        }
        LOG_IF(WARNING,compare_equality(1,total_prob,ParallelNestedBenders::zero_tolerance_)) << "Your model probabilties do not sum up to one.";

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

        // Magic numbers, but we set these values via options
        integer_tolerance_ = std::pow(10.0,-6);

        absolute_epsilon_ = std::pow(10.0,-6);
        relative_epsilon_ = std::pow(10.0,-6);

        // create our benders node objects and build the tree
        tree_ = SmiScenarioTree<BendersNodeData*>();
        std::vector<std::vector<BendersNodeData*>> nodeVec;
        nodeVec.reserve(num_scenarios_);

        // Create one BendersNodeData vector for every scenario
        for (int i = 0; i < num_scenarios_; i++) {
            std::vector<BendersNodeData*> vec;
            nodeVec.push_back(vec);
        } 

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
        for (int i = 1; i < num_stages_; i++) {
            for (unsigned int j = 0; j < tree_nodes_by_stage_[i-1].size(); j++) {
                std::vector<SmiTreeNode<BendersNodeData*> *> *children = tree_nodes_by_stage_[i-1][j]->getChildren();
                for (int k = 0; k < children->size(); k++) {
                    tree_nodes_by_stage_[i].push_back((*children)[k]);
                }
                delete children;
            }
        }
        // Create Benders Stage data
        stage_data_.reserve(num_stages_);
        for (int i = 0; i < num_stages_; i++)
            stage_data_.push_back(new BendersStageData);


        // Thread related stuff
        num_cpu_ = boost::thread::hardware_concurrency();
        thread_pool_ = boost::threadpool::pool(num_cpu_); // Can we resize the threadpool?

    }

    void ParallelNestedBenders::Solve()
    {
        AdjustProblem();
        // We can stop here, if we have integer variables on more than the second stage..
        if ( contains_integer_ ){
            LOG(WARNING) << "Your problem contains integer variables. We can not solve mixed-integer problems. Instead we solve the relaxed problem.";
            //return;
        }
        
        // Do initialization stuff necessary for solve process
        while ( std::abs(upper_bound_-lower_bound_) > absolute_epsilon_ &&  
            computeRelativeGap(upper_bound_,lower_bound_,infinity_,relative_epsilon_) &&
            current_iteration_ < max_iteration_ 
            ) 
        {
            if ( old_lower_bound_ >= lower_bound_ && old_lower_bound_ > -infinity_)// TODO: If we have feasCuts we might have to look at this again..
                break;

            minor_iteration_++;
            if ( current_stage_ == 0 ){
                current_iteration_++;
            }
            std::cout << "Iteration: " << current_iteration_ << "." << minor_iteration_ << std::endl;
            if ( current_stage_ == 0){
                std::cout << "Lower Bound:  " << lower_bound_ << std::endl;
                std::cout << "Upper Bound:  " << upper_bound_ << std::endl;
                std::cout << "Relative Gap: " << computeRelativeGap(upper_bound_,lower_bound_,infinity_,relative_epsilon_) << std::endl;
            }

            // Change direction 
            if ( current_stage_ == 0 )
                forward_ = true;
            if ( current_stage_ == num_stages_ -1)
                forward_ = false;


            int num_nodes = tree_nodes_by_stage_[current_stage_].size();
            for ( int i = 0; i < num_nodes; i++ ){
                // Schedule the tasks for this stage
                thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]));
            }
            thread_pool_.wait(0); // We wait until all threads are done

            // If we have an infeasible subproblem, we generate feasibility cuts and go backward then.
            if ( infeasible_problem_ ){
                // Generate feasibility cuts (and optimality cuts )
                
                // Change direction
                forward_ = false;
            }

            if ( !forward_ ){
                // We need to aggregate optimality cuts for the parent node
                for ( int i = 0; i < tree_nodes_by_stage_[current_stage_-1].size(); i++)
                    // Schedule the tasks for this stage
                    thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AggregateOptimalityCuts,this,tree_nodes_by_stage_[current_stage_-1][i]));
                thread_pool_.wait(0); // We wait until all threads are done
            }

            // Update upper bound if we solved all problems in this iteration once
            // Lower bound is updated in the StoreData Method after solving first stage problem
            if ( current_stage_ == num_stages_ -1){
                if ( upper_bound_ > temporary_upper_bound_ )
                    upper_bound_ = temporary_upper_bound_;

            }

            // Change direction, depending on where we are
            forward_ ? current_stage_++ : current_stage_--;

            // We change the stage, so we should clear all of the osiSolverInterfaces
            std::pair<boost::thread::id, OsiSolverInterface* > id_solver_pair;
            BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
                //id_solver_pair.second->reset(); // This method does not really work for OsiCpx, I do not know why.
                id_solver_pair.second->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                // Workaround: Load empty problem. 
            }

        }
        // We finished solving
        LOG(INFO) << "Solution: " << lower_bound_;

    }


    void ParallelNestedBenders::HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {

        // Get thread ID
        console_mutex_.lock();
        LOG(INFO) << "Current thread has ID: " << boost::this_thread::get_id();
        console_mutex_.unlock();
        OsiSolverInterface* osi_solver = GetSolverByThread();

        BuildSubproblem(osi_solver, benders_tree_node_pointer);
        SolveSubproblem(osi_solver,benders_tree_node_pointer);
        StoreData(osi_solver,benders_tree_node_pointer);
        if ( !forward_)
            GenerateOptimalityCut(benders_tree_node_pointer);
    }

    void ParallelNestedBenders::SolveSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer  ){
        // Warm Start Handling
        console_mutex_.lock();
        std::cout << "Solve Subproblem " << boost::this_thread::get_id() << std::endl;
        console_mutex_.unlock();
        osi_solver->initialSolve();
        std::stringstream stream;
        stream << "stage" << current_stage_ << "_" << current_iteration_ << "_" << benders_tree_node_pointer->scenario();
        console_mutex_.lock();
        osi_solver->writeLp(stream.str().c_str());
        console_mutex_.unlock();
        // Result/Error Handling: Introduce a global variable, when feasibility problems occurs..
        // Check for errors
        if ( !osi_solver->isProvenOptimal() ){
            // We need to check for errors in this case
            if ( osi_solver->isProvenPrimalInfeasible() ){
                // Dual problem is unbounded: Generate feasibility cut if we are not in the first stage
                LOG(INFO) << "Generate feasibility cut.";
                GenerateFeasibilityCut( osi_solver, benders_tree_node_pointer ); // One for every scenario?
            }
            if ( osi_solver->isProvenDualInfeasible() ){
                // Primal problem is unbounded. => dual infeasible
                // If we are in the master problem, we might have to restrict the theta variables
                LOG(INFO) << "Primal problem is unbounded";
            }
        }

        // What about dual ray stuff for feasibility??what about reduced costs then?
    }

    void ParallelNestedBenders::StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer ){
        console_mutex_.lock();
        std::cout << "Store data " << std::endl; //boost::this_thread::get_id() << std::endl;
        console_mutex_.unlock();
        // Store Column Solution
        // We need to copy the solution, as we erase the osiSolverInterface
        BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
        // Delete old values
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
                if ( compare_zero(benders_node_pointer->col_solution_[i]))
                    benders_node_pointer->col_solution_[i] = 0;
                solution_value += benders_node_pointer->col_solution_[i]*osi_solver->getObjCoefficients()[i];
            }
            if ( current_stage_ == 0) // This work in the multithreaded cases, as we have only one thread for the first stage problem
                temporary_upper_bound_ = 0;
            temporary_upper_bound_ += solution_value*benders_node_pointer->probability_;
        }

        // Copy theta variables
        int num_theta = osi_solver->getNumCols()-stage_data_[current_stage_]->num_columns_;
        assert(num_theta >= 0);
        
        // TODO: Retrieve and store the theta variables
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
                if ( compare_zero(benders_node_pointer->dual_solution_[i]))
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
                if ( !compare_zero(column_solution[i] ) ){
                    // We have to look at bounds TODO: ZEROTEST For reduced cost
                    if ( compare_equality( column_solution[i], variable_lower_bounds[i],zero_tolerance_) )
                        computed_dual_value_for_variable_bounds += dual_values_variable_bounds[i]*variable_lower_bounds[i];
                    else if ( compare_equality( column_solution[i], variable_upper_bounds[i],zero_tolerance_))
                        computed_dual_value_for_variable_bounds -= dual_values_variable_bounds[i]*variable_upper_bounds[i]; 
                }
            }
            benders_node_pointer->dual_value_variable_bounds_ = computed_dual_value_for_variable_bounds;
        }
    }

    void ParallelNestedBenders::BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    {
        // Do stuff we would like to do
        std::cout << "Build subproblem and load it in the OsiSolverObject.." << std::endl;
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
                RowOrderedSparseMatrixPointer stochastic_matrix(create_row_ordered_sparse_matrix_pointer( node_pointer ));
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
                RowOrderedSparseMatrixPointer stochastic_matrix(create_row_ordered_sparse_matrix_pointer( node_pointer ));
                RowOrderedSparseMatrixPointer deterministic_matrix(create_row_ordered_sparse_matrix_pointer( deterministic_node_pointer ));
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

                            else {   
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
                        assert(benders_node_pointer->row_lower_[row] == benders_node_pointer->row_upper_[row]);
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
                for ( int i = 0; i < stage_pointer->num_columns_; i++ ){
                    osi_solver->setColBounds(i,benders_node_pointer->column_lower_[i],benders_node_pointer->column_upper_[i]);
                }
            }
            if ( stage_pointer->stochastic_objective_ ){
                // Update objective
                for ( int i = 0; i < stage_pointer->num_columns_; i++ ){
                    osi_solver->setObjCoeff(i,benders_node_pointer->objective_[i]);
                }
            }
            // Update Row Bounds (RHS) for the basic problem
            for ( int i = 0; i < stage_pointer->num_rows_; i++ ){
                osi_solver->setRowBounds(i,benders_node_pointer->row_lower_[i],benders_node_pointer->row_upper_[i]);
            }
            // Remove feasibility and optimality cuts from previous solution process
            int cuts_to_remove = osi_solver->getNumRows()-stage_pointer->num_rows_;
            if ( cuts_to_remove ){
                std::vector<int> remove_indices;
                remove_indices.reserve(cuts_to_remove);
                for (int i = stage_pointer->num_rows_; i < stage_pointer->num_rows_+cuts_to_remove;i++)
                    remove_indices.push_back(i);
                osi_solver->deleteRows(cuts_to_remove,&remove_indices[0]);

            }

                
        }

        // Add the theta-stuff ( how many we have depends on current multi-cut aggregation level.. )
        if ( !benders_node_pointer->optimality_cuts_.empty() && !reuse_solver){// We have some optimality cuts, so we need theta variables
            // We set a theta value for every stage
            int num_cols = stage_pointer->num_thetas_;
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
        
        
        // Go on with the matrix
        RowOrderedSparseMatrixPointer deterministic_matrix = create_row_ordered_sparse_matrix_pointer( core_->getNode(current_stage_) );
        RowOrderedSparseMatrixPointer stochastic_matrix = create_row_ordered_sparse_matrix_pointer(node_pointer);

        // Get dual solution values 
        const double* dual_solution = benders_node_pointer->dual_solution_;
        // Store results of the computation 
        std::vector<double> cut_coefficients(stage_data_[current_stage_-1]->num_columns_,0);

        int current_stage_columns = core_->getColStart(current_stage_);
        int column_start_previous_stage = core_->getColStart(current_stage_-1);
        int stochastic_column_index = 0;
        bool stochastic_element_in_row = false;
        // Now we need the loop. Do go on.
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
                
                if ( deterministic_matrix.indices_[column_index] < current_stage_columns){
                    if ( stochastic_element_in_row && deterministic_matrix.indices_[column_index] == stochastic_matrix.indices_[stochastic_column_index]){ // Add stochastic element
                        cut_coefficients[deterministic_matrix.indices_[column_index]-column_start_previous_stage] += stochastic_matrix.elements_[stochastic_column_index]*dual_solution[row];
                        stochastic_column_index++;
                    }
                    else    
                        cut_coefficients[deterministic_matrix.indices_[column_index]-column_start_previous_stage] += deterministic_matrix.elements_[column_index]*dual_solution[row];
                }

            }
            if ( stochastic_element_in_row )
                assert(stochastic_column_index == stochastic_matrix.start_[row+1]);
        }

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
        console_mutex_.lock();
        LOG(INFO) << "Aggregating cuts";
        console_mutex_.unlock();
        // We specified the number of thetas in the stage_pointer (for now)
        int num_cuts = stage_data_[current_stage_-1]->num_thetas_;
        // We need all the children of this node..
        int num_childs = benders_tree_node_pointer->numChildren();
        num_cuts = std::min(num_cuts,num_childs); // We have to propose this back
        // TODO: Ignore "cuts" that were not created, due to multicut condition ( theta_k >= solution value of subproblem)
        int num_subproblems_per_cut = std::ceil( ((double)num_childs)/num_cuts );

        int num_columns = stage_data_[current_stage_-1]->num_columns_;

        // Do some magic to finde a good partition for the number of cuts we want to produce: TODO
        int overall_start_index = benders_tree_node_pointer->scenario();
        int overall_end_index = overall_start_index+num_childs; 
        for ( int i = 0; i < num_cuts; i++){
            // For now: just split subproblems after num_subproblems_per_cut
            // Start a new OsiRowCut
            double cut_lower_bound = 0;
            std::vector<double> cut_elements(num_columns+1,0);
            cut_elements[num_columns] = 1;
            std::vector<int> cut_indices;
            cut_indices.reserve(num_columns+1);
            for (int j = 0; j < num_columns; j++)
                cut_indices.push_back(j);
            cut_indices.push_back(num_columns+i);

            for ( int j = overall_start_index+i*num_subproblems_per_cut; j < overall_start_index+(i+1)*num_subproblems_per_cut; j++){
                if ( j < overall_end_index){
                    for (int k = 0; k < num_columns; k++)
                        cut_elements[k] += tree_nodes_by_stage_[current_stage_][j]->getDataPtr()->current_cut_coefficients_[k];
                    cut_lower_bound += tree_nodes_by_stage_[current_stage_][j]->getDataPtr()->current_cut_rhs_;
                    
                }
                
            }
            OsiRowCut* cut_pointer = new OsiRowCut();
            cut_pointer->setLb(cut_lower_bound);
            cut_pointer->setUb(infinity_);
            cut_pointer->setRow(num_columns+1,&cut_indices[0],&cut_elements[0]);
            benders_tree_node_pointer->getDataPtr()->optimality_cuts_.push_back(cut_pointer);
        }
    }

    void ParallelNestedBenders::GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer )
    { // Called directly from solve subproblem.

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


    bool computeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon){
        if (upper_bound == infinity || lower_bound == -infinity)
            return true;
        if (upper_bound == 0) // We have no meaningful comparison
            return true;
        if (std::abs(upper_bound) > 1 && std::abs(lower_bound) > 1) //in this case we do not want relative difference, instead we want absolute difference..
            return true;
        return (abs(upper_bound-lower_bound)/abs(upper_bound)) >= epsilon; //See cplex mipgap
    }

    RowOrderedSparseMatrixPointer create_row_ordered_sparse_matrix_pointer( SmiNodeData* smiNode )
    {
        return RowOrderedSparseMatrixPointer(smiNode->getElements(smiNode->getMatStart()),smiNode->getIndices(smiNode->getMatStart()),smiNode->getStarts(smiNode->getMatStart()));
    }

    bool compare_zero( double value )
    {
        return std::abs(value) <= ParallelNestedBenders::zero_tolerance_;
    }

    bool compare_equality( double first, double second, double tolerance )
    {
        return std::abs(first-second)<= tolerance;
    }




    double OsiStochasticParallelNestedBendersTwoStageSolverInterface::getObjValue()
    {
        return nested_benders_->upper_bound_;
    }

    void OsiStochasticParallelNestedBendersTwoStageSolverInterface::initialSolve()
    {
        nested_benders_->Solve();
    }

    OsiStochasticParallelNestedBendersTwoStageSolverInterface::OsiStochasticParallelNestedBendersTwoStageSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model )
        : OsiStochasticSolverInterface(osi_solver,smi_model),nested_benders_(NULL)
    {
        this->smi_model_->setOsiSolverHandle(this->osi_solver_);
        nested_benders_ = new ParallelNestedBenders(smi_model_);
    }

    OsiStochasticParallelNestedBendersTwoStageSolverInterface::~OsiStochasticParallelNestedBendersTwoStageSolverInterface()
    {
        delete nested_benders_;
    }
} // end namespace parallel_benders
#include "ParallelNestedBenders.hpp"

#include <iostream>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>
// Include all the other files
#include "SmiScnModel.hpp"


namespace parallel_benders {

void ParallelNestedBenders::AdjustProblem()
{
    // What do we want to do?
    // 1. Check for stochasticity of the model 
    // 2. Partition matrix entries in matrices T and W for every stage
    

    for (int i = 0; i < num_stages_; i++)
        AdjustProblemInStage(i);

}

void ParallelNestedBenders::AdjustProblemInStage( int stage )
{
    // Look at core data to get values for the matrices..
    for ( int i = 0; i < tree_nodes_by_stage_[stage].size(); i++){
        // Look at current node
        BendersNodeData* benders_node_ptr = tree_nodes_by_stage_[stage][i]->getDataPtr();
        SmiNodeData* node_ptr = benders_node_ptr->smi_node_data_;
        // Build T and W matrix

        // Are there random bounds?
        node_ptr->getCloStart();
        node_ptr->getCupStart();

        // Are there random objective function values?

        // Are there random right hand sides?
    }
    
}

ParallelNestedBenders::ParallelNestedBenders( SmiScnModel* smi_model_ptr ) :
smi_(smi_model_ptr),core_(smi_->getCore()),osi_solver_(NULL),osi_solver_map_(),tree_(),tree_nodes_by_stage_(),stage_data_(),
num_stages_(0),num_scenarios_(0),infinity_(COIN_INT_MAX),
zero_tolerance_(0.0),integer_tolerance_(0.0),absolute_epsilon_(0.0),relative_epsilon_(0.0),
current_iteration_(0),max_iteration_(1000),lower_bound_(-infinity_),upper_bound_((int)infinity_),
current_stage_(0),forward_(true),infeasible_problem_(false),
num_cpu_(0),thread_pool_(0)
{
    Init();

}

void ParallelNestedBenders::Init()
{
    // Set number of stages and scenarios
    num_scenarios_ = smi_->getNumScenarios();
    num_stages_ = smi_->getCore()->getNumStages();

    // Initialize osiSolver
    if (smi_->getOsiSolverInterface() == NULL)
        osi_solver_ = new OsiCpxSolverInterface();
    else
        osi_solver_ = smi_->getOsiSolverInterface()->clone();

    // Set infinity to solver infinity
    infinity_ = osi_solver_->getInfinity();

    // Magic numbers, but we set these values via options
    zero_tolerance_ = std::pow(10.0,-6);
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
        node->conditional_probability_ = (*it)->getProb();
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
    stage_data_.resize(num_stages_,new BendersStageData());


    // Thread related stuff
    num_cpu_ = boost::thread::hardware_concurrency();
    thread_pool_ = boost::threadpool::pool(num_cpu_); // Can we resize the threadpool?

}

void ParallelNestedBenders::Solve()
{
    AdjustProblem();
    // Do initialization stuff necessary for solve process
    while ( std::abs(upper_bound_-lower_bound_) > absolute_epsilon_ &&  
        computeRelativeGap(upper_bound_,lower_bound_,infinity_,relative_epsilon_) &&
        current_iteration_ < max_iteration_) 
    {
        int num_nodes = tree_nodes_by_stage_[current_stage_].size();
         for ( int i = 0; i < num_nodes; i++ ){
            // Schedule the tasks for this stage
            thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem,this,tree_nodes_by_stage_[current_stage_][i]->getDataPtr()));
            
        }
        thread_pool_.wait(0); // We wait until all threads are done
        if ( forward_ ){
            // If in last stage: store dual solution too, change direction
            std::cout << "forward" << std::endl;
        }
        else {
            // If in first stage: change direction
            // Compute cut's depending on aggregation level and dual values computed before
            std::cout << "backward" << std::endl;
        }
        // If we have an infeasible subproblem, we generate feasibility cuts and go backward then. If we are in stage 0, we are done.

        // Change direction, depending on where we are
        if ( forward_ && current_stage_ < num_stages_-1 ){
            current_stage_++;
        }
        else if ( current_stage_ == num_stages_-1 ){
            forward_ = false; current_stage_--;
        }
        else if ( !forward_ && current_stage_ == 0){
            forward_ = true; current_stage_++;
        }
        else if ( !forward_ )
            current_stage_--;
   


        // We change the stage, so we should clear all of the osiSolverInterfaces
        std::pair<boost::thread::id, OsiSolverInterface* > id_solver_pair;
        BOOST_FOREACH( id_solver_pair, osi_solver_map_ ){
            id_solver_pair.second->reset(); // This method does not really work for OsiCpx, I do not know why.
            // Workaround: Load empty problem. Dynamic cast to OsiCpxSolverInterface and get C pointer to cplex.
        }

    }

}


void ParallelNestedBenders::HandleSubproblem( BendersNodeData* benders_node_pointer ){

    // Get thread ID
    LOG(INFO) << "Current thread has ID: " << boost::this_thread::get_id();
    OsiSolverInterface* osi_solver = osi_solver_map_[boost::this_thread::get_id()];
    if ( osi_solver == NULL ){
        // Create new osi_solver object
        osi_solver = osi_solver_->clone();
        osi_solver_map_[boost::this_thread::get_id()] = osi_solver;
    }
    
    BuildSubproblem(osi_solver, benders_node_pointer);
    SolveSubproblem(osi_solver);
    StoreData(osi_solver,benders_node_pointer);
}

void ParallelNestedBenders::SolveSubproblem( OsiSolverInterface* osi_solver ){
    std::cout << "Solve given problem." << std::endl;
}

void ParallelNestedBenders::StoreData( OsiSolverInterface* osi_solver, BendersNodeData* benders_node_pointer ){
    std::cout << "Store data." << std::endl;
}

void ParallelNestedBenders::BuildSubproblem( OsiSolverInterface* osi_solver, BendersNodeData* benders_node_pointer )
{
    // Do stuff we would like to do
    std::cout << "Build subproblem and load it in the OsiSolverObject.." << std::endl;
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

bool computeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon){
    if (upper_bound == infinity || lower_bound == -infinity)
        return true;
    if (upper_bound == 0) // We have no meaningful comparison
        return true;
    if (std::abs(upper_bound) > 1 && std::abs(lower_bound) > 1) //in this case we do not want relative difference, instead we want absolute difference..
        return true;
    return (abs(upper_bound-lower_bound)/abs(upper_bound)) >= epsilon; //See cplex mipgap
}

} // end namespace parallel_benders
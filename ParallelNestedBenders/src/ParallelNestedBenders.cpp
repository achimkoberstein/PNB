#include "ParallelNestedBenders.hpp"

#include <boost/thread.hpp>
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

    
}

ParallelNestedBenders::ParallelNestedBenders( SmiScnModel* smi_model_ptr ) :
smi_(smi_model_ptr),core_(smi_->getCore()),osi_solver_(NULL),tree_(),tree_nodes_by_stage_(),
num_stages_(0),num_scenarios_(0),infinity_(COIN_INT_MAX),
zero_tolerance_(0.0),integer_tolerance_(0.0),absolute_epsilon_(0.0),relative_epsilon_(0.0),
current_iteration_(0),max_iteration_(1000),lower_bound_(-infinity_),upper_bound_((int)infinity_)
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

    // Thread related stuff
    num_cpu_ = boost::thread::hardware_concurrency();

}

void ParallelNestedBenders::Solve()
{
    // Do initialization stuff necessary for solve process
    while ( std::abs(upper_bound_-lower_bound_) > absolute_epsilon_ &&  
        computeRelativeGap(upper_bound_,lower_bound_,infinity_,relative_epsilon_) &&
        current_iteration_ < max_iteration_) 
    {
        // Decide what to do depending on several factors. We can implement different strategies
        if ( current_stage_ == 0 ) {// We are at the first stage
            if ( forward_ == false){ // We are on a way back, so we have to solve the problem with the attached cuts and
            // Solve the problem and update the bounds 
            }
            forward_ = true; // We want to move forward

        }
        // We are on our way back: Apply generated cuts and solve problems at this stage. Generate Cuts for the previous stage
        for ( int i = 0; i < tree_nodes_by_stage_[current_stage_].size(); i++ ){
            // Generate problem
            BuildSubproblem(tree_nodes_by_stage_[current_stage_][i]);
            // Solve problem
            SolveSubproblem(tree_nodes_by_stage_[current_stage_][i]);
            // Store primal solution
            StorePrimalSolution(tree_nodes_by_stage_[current_stage_][i]);
            // Store dual solution
            StoreDualSolution(tree_nodes_by_stage_[current_stage_][i]);
        }
        if ( forward_ ){
            // If in last stage: store dual solution too, change direction
        }
        else {
            // If in first stage: change direction
            // Compute cut's
        }
    }

}

ParallelNestedBenders::~ParallelNestedBenders()
{
    // What to delete?
    delete osi_solver_;
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
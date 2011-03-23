#include "ParallelNestedBenders.hpp"

// Include all the other files
#include "SmiScnModel.hpp"


namespace parallel_benders {

void ParallelNestedBenders::AdjustProblem()
{
    // What do we want to do?
    // We have to look at every stage & every scenario (TODO: Only necessary for random variable bounds, tbd)
    // We can parallelize it stagewise
    

    for (int i = 0; i < num_stages_; i++)
        AdjustProblemInStage(i);

}

void ParallelNestedBenders::AdjustProblemInStage( int stage )
{
    
}

ParallelNestedBenders::ParallelNestedBenders( SmiScnModel* smi_model_ptr ) :
smi_(smi_model_ptr),osi_solver_(NULL),tree_(),tree_nodes_by_stage_(),
num_stages_(0),num_scenarios_(0),infinity_(COIN_INT_MAX),
current_iteration_(0),lower_bound_(-infinity_),upper_bound_((int)infinity_)
{
    Init();

}

void ParallelNestedBenders::Init()
{
    // Set number of stages and scenarios
    num_scenarios_ = smi_->getNumScenarios();
    num_stages_ = smi_->getCore()->getNumStages();

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


    // make a list of list of nodes, seperated by stage
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
            for (unsigned int k = 0; k < children->size(); k++) {
                tree_nodes_by_stage_[i].push_back((*children)[k]);
            }
            delete children;
        }
    }

}

void ParallelNestedBenders::Solve()
{

}

ParallelNestedBenders::~ParallelNestedBenders()
{
    // What to delete?
}
} // end namespace parallel_benders
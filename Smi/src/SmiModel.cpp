#include "SmiModel.hpp"

SmiModel::SmiModel(void)
	{
	}

SmiModel::~SmiModel(void)
	{
	}

//Generates Tree Nodes with Data for given scenario
SmiScenarioIndex
addScenarioToTree(SmiScenarioTree<SmiScnNode *> * smiTree,SmiCoreData *core,
				CoinPackedMatrix *matrix,
				CoinPackedVector *v_dclo, CoinPackedVector *v_dcup,
				CoinPackedVector *v_dobj,
				CoinPackedVector *v_drlo, CoinPackedVector *v_drup,
				SmiStageIndex branch, SmiScenarioIndex anc, double prob,
				SmiCoreCombineRule *r)
{

	// this coding takes branch to be the node that the scenario branches *from*
	--branch;
	//temporary vector, that stores all nodes that get created on the way up to the leaf node
	vector<SmiScnNode *> node_vec;

	node_vec.reserve(core->getNumStages()-branch); //We only add numStage-branch many nodes. Nodes get added to ScenarioTree in the vector order

	// first scenario is the one that branches from root node and has no ancestor
	if ( anc==0 && branch == 0 )
	{
		// generate root node
		SmiNodeData *node = core->getNode(0);
		SmiScnNode *tnode = new SmiScnNode(node);
		tnode->setScenarioIndex(0);
		node_vec.push_back(tnode);
		//Store this information someplace else?
		//this->ncol_ = core->getNumCols(0);
		//this->nrow_ = core->getNumRows(0);
		//this->nels_ = node->getNumMatrixElements();

	}
	else
	{
		// TODO: throw error if branch too large
		assert(branch<core->getNumStages());
	}

	// TODO: what to do about duplicate matrix entries?
	// ...can't do the following because eliminates zero entries...
	// matrix->eliminateDuplicates(0.0);

	//Christian: This works as follows
	// First: All nodes for the scenario from the branching stage to the leaf get created
	// Second: Connect the nodes with the present tree via the branching node, generates scenario index
	// Third: Add correct probabilities and set correct scenario index

	int t;
	for (t=branch+1; t<core->getNumStages(); t++) //Christian: Generate SmiScnNodes until the leaf nodes are reached
	{
		// generate new data node for given stage
		SmiNodeData *node = new SmiNodeData(t,core,matrix,
			v_dclo,v_dcup,v_dobj,v_drlo,v_drup);
		node->setCoreCombineRule(r);
		// generate new tree node
		SmiScnNode *tnode = new SmiScnNode(node);
		node_vec.push_back(tnode);

		//this->ncol_ += core->getNumCols(t); //TODO: Why is this done for every stage? What if a new scenario branches from e.g. root node? 
		//this->nrow_ += core->getNumRows(t); //Reson seems to be intersection between Tree Generation and Preparations for Det. Eq. Generation in SmiScnModel.. 
		//this->nels_ += core->getNode(t)->getNumMatrixElements() + node->getNumMatrixElements(); //Maybe we have to split that..
	}

	//Christian: What is this method doing? Connects the newly created nodes above
	SmiScenarioIndex scen = smiTree->addPathtoLeaf(anc,branch,node_vec);

	// add probability to all scenario nodes in path: TODO: Outsource to owhn method
	SmiTreeNode<SmiScnNode *> *child = smiTree->getLeaf(scen);
	SmiTreeNode<SmiScnNode *> *parent = child->getParent();
	SmiTreeNode<SmiScnNode *> *root = smiTree->getRoot();

	while (child != root)
	{
		SmiScnNode *tnode = child->getDataPtr();
		tnode->addProb(prob);
		tnode->setParent(parent->getDataPtr());
		if (tnode->getScenarioIndex()==-1)
			tnode->setScenarioIndex(scen);
		child = parent;
		parent = child->getParent();
	}
	root->getDataPtr()->addProb(prob);

	//this->totalProb_+=prob;
	return scen;
}

void generateSubproblem(OsiSolverInterface *osi,SmiScenarioTree<SmiScnNode *> *smiTree,SmiStageIndex stageFrom, SmiStageIndex stageTo, SmiScenarioIndex scenFrom, SmiScenarioIndex scenTo);

void generateSubproblem(OsiSolverInterface *osi, SmiScenarioTree<SmiScnNode *> *smiTree, SmiScnNode* node);
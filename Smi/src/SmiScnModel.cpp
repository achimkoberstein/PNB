#include "SmiDiscreteDistribution.hpp"
#include "SmiBlockStructure.hpp"
#include "SmiScenarioTree.hpp"
#include "SmiScnModel.hpp"
#include "SmiSmpsIO.hpp"
#include "CoinPackedMatrix.hpp"
#include "OsiSolverInterface.hpp"
#include "CoinHelperFunctions.hpp"
#include "CoinError.hpp"
#include "CoinPackedVector.hpp"
#include <assert.h>
#include <algorithm>
#include <map>
#include <vector>

#include "CoinTime.hpp"

using namespace std;

int SmiScnNode::getCoreColIndex(int i)
{
    SmiCoreData *core = node_->getCore();
    return core->getColExternalIndex(i-coffset_+core->getColStart(node_->getStage()));
}
//int
//SmiScnNode::getSolverColIndex(int i)
//{
//	SmiCoreData *core = node_->getCore();
//	return core->getColExternalIndex(i-coffset_+core->getColStart(node_->getStage()));
//}

int SmiScnNode::getCoreRowIndex(int i){
    SmiCoreData *core = node_->getCore();
    return core->getRowExternalIndex(i-roffset_+core->getRowStart(node_->getStage()));
}
//int
//SmiScnNode::getSolverRowIndex(int i)
//{
//	SmiCoreData *core = node_->getCore();
//	return core->getRowInternalIndex(i-coffset_+core->getColStart(node_->getStage()));
//}

SmiScnModel::~SmiScnModel()
{
    // loop to deleteNodes
    //for_each(smiTree_.treeBegin(),smiTree_.treeEnd(),SmiScnModelDeleteNode(this));

    if (osiStoch_)
        delete osiStoch_;

    if (core_)
        delete core_;

    if (drlo_)
        delete [] drlo_;

    if (drup_)
        delete [] drup_;

    if (dclo_)
        delete [] dclo_;

    if (dcup_)
        delete [] dcup_;

    if (dobj_)
        delete [] dobj_;

    if (matrix_)
        delete matrix_;

    if (maxNelsPerScenInStage)
        delete [] maxNelsPerScenInStage;

    delete smpsio_;

    delete discrete_distribution_;
    delete block_structure_;
}

//Generates Tree Nodes with Data for given scenario
// branch: Stage where the new scenario differs from previous scenario
// anc: the ancestor of this scenario, this is the one it branches from
// prob: unconditional probability of this scenario (leaf node probability)
SmiScenarioIndex SmiScnModel::generateScenario(SmiCoreData *core,
                              CoinPackedMatrix *matrix,
                              CoinPackedVector *v_dclo, CoinPackedVector *v_dcup,
                              CoinPackedVector *v_dobj,
                              CoinPackedVector *v_drlo, CoinPackedVector *v_drup,
                              SmiStageIndex branch, SmiScenarioIndex anc, double prob,
                              SmiCoreCombineRule *r)
{

    // this coding takes branch to be the node that the scenario branches *from*
    --branch;

    vector<SmiScnNode *> node_vec;

    node_vec.reserve(core->getNumStages());

    // If we add the first scenario..
    if (this->getNumScenarios()==0)
    {
        // According to SMPS, for the first scenario, the branch SHOULD be at the first stage, this is branch=0 in Smi term. With the first --branch, we can have branch = -1.
        if ( branch == -1 )
            branch = 0;
        assert(anc == 0);
        assert(branch == 0);
        anc = 0;
        branch = 0;

        // generate root node
        //SmiNodeData *node = core->getNode(0);
        CoinPackedMatrix matrix;
        CoinPackedVector vector;
        SmiNodeData *node = new SmiNodeData(0,core,&matrix,&vector,&vector,&vector,&vector,&vector);
        SmiScnNode *tnode = new SmiScnNode(node);
        tnode->setScenarioIndex(0);
        node_vec.push_back(tnode);
        this->ncol_ = core->getNumCols(0);
        this->nrow_ = core->getNumRows(0);
        this->nels_ = core->getNode(0)->getNumMatrixElements();// In the det-eq the first stage nels are needed, so we can not start with 0 here

        // prepare array for counting max nels per stage and scenario
        if (this->maxNelsPerScenInStage)
            delete[] maxNelsPerScenInStage;
        this->maxNelsPerScenInStage = new int[core->getNumStages()];
        std::fill_n(maxNelsPerScenInStage, core->getNumStages(), 0); //Initialize before using it. 
        this->maxNelsPerScenInStage[0] = this->nels_;
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
        // Now we can transform to replace
        node->transformToReplace();
        // generate new tree node
        SmiScnNode *tnode = new SmiScnNode(node);
        node_vec.push_back(tnode);

        this->ncol_ += core->getNumCols(t); //TODO: Why is this done for every stage? What if a new scenario branches from e.g. root node? 
        this->nrow_ += core->getNumRows(t); //Reson seems to be intersection between Tree Generation and Preparations for Det. Eq. Generation in SmiScnModel.. 
        this->nels_ += core->getNode(t)->getNumMatrixElements() + node->getNumMatrixElements(); //Maybe we have to split that..
        this->maxNelsPerScenInStage[t] = max(this->maxNelsPerScenInStage[t], core->getNode(t)->getNumMatrixElements() + node->getNumMatrixElements());
    }

    //Christian: What is this method doing? Connects the newly created nodes above
    SmiScenarioIndex scen = smiTree_.addPathtoLeaf(anc,branch,node_vec);

    // add probability to all scenario nodes in path: TODO: Outsource to own method
    SmiTreeNode<SmiScnNode *> *child = smiTree_.getLeaf(scen);
    SmiTreeNode<SmiScnNode *> *parent = child->getParent();
    SmiTreeNode<SmiScnNode *> *root = smiTree_.getRoot();

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

    this->totalProb_+=prob;

    return scen;
}

SmiScenarioIndex SmiScnModel::generateScenario(SmiCoreData *core,
                              CoinPackedMatrix *matrix,
                              CoinPackedVector *v_dclo, CoinPackedVector *v_dcup,
                              CoinPackedVector *v_dobj,
                              CoinPackedVector *v_drlo, CoinPackedVector *v_drup,
                              vector<int> labels, double prob,
                              SmiCoreCombineRule *r)
{

    // this code assumes that full path data (incl root node data)
    // is passed in.

    vector<SmiScnNode *> node_vec;

    node_vec.reserve(core->getNumStages());


    // TODO: what to do about duplicate matrix entries?
    // ...can't do the following because eliminates zero entries...
    // matrix->eliminateDuplicates(0.0);

    int t;
    for (t=0; t<core->getNumStages(); t++)
    {
        // generate new data node
        SmiNodeData *node = new SmiNodeData(t,core,matrix,
            v_dclo,v_dcup,v_dobj,v_drlo,v_drup);

        node->setCoreCombineRule(r);
        // generate new tree node
        SmiScnNode *tnode = new SmiScnNode(node);
        node_vec.push_back(tnode);

        this->ncol_ += core->getNumCols(t);
        this->nrow_ += core->getNumRows(t);
        this->nels_ += core->getNode(t)->getNumMatrixElements() + node->getNumMatrixElements();
    }

    SmiTreeNode<SmiScnNode *> *node = smiTree_.find(labels);
    int scen = smiTree_.addPathtoLeaf(node->scenario(),node->depth(),node_vec);
    smiTree_.setChildLabels(node,labels);

    // add probability to all scenario nodes in path
    SmiTreeNode<SmiScnNode *> *child = smiTree_.getLeaf(scen);
    SmiTreeNode<SmiScnNode *> *parent = child->getParent();
    SmiTreeNode<SmiScnNode *> *root = smiTree_.getRoot();

    while (child != root)
    {
        SmiScnNode *tnode = child->getDataPtr();
        tnode->addProb(prob);
        tnode->setParent(parent->getDataPtr());
        child = parent;
        parent = child->getParent();
    }
    root->getDataPtr()->addProb(prob);

    this->totalProb_+=prob;

    return scen;

}

// Main public interfaces for getting WS, EV, and EEV values (no coefficients whatsoever).
double SmiScnModel::getWSValue(OsiSolverInterface *osiSolver, double objSense ){
    //Test if wsValues is empty, if this is the case, run solveWS
    std::vector< std::pair<double,double*> > tempVector = solveWS(osiSolver, objSense);
    assert( tempVector.size() == smiTree_.getNumScenarios()); 
    double result = 0;
    for (unsigned int i = 0; i < tempVector.size(); i++){
        result += tempVector[i].first*this->getScenarioProb(i);
        delete [] tempVector[i].second;
    }
    return result;
}

double SmiScnModel::getEVValue(OsiSolverInterface *osiSolver, double objSense, bool calculateAverageValues){
    std::pair<double, double*> results(solveEV(osiSolver,objSense,calculateAverageValues));
    delete[] results.second;
    return results.first;
}

double SmiScnModel::getEEVValue(OsiSolverInterface* osiSolver, double objSense, bool calculateAverageValues){
    return solveEEV(osiSolver,objSense,calculateAverageValues);
}

SmiScnModel* SmiScnModel::createEVSmiModel(){
    //Clean up previous stuff.
    delete[] dclo_;
    delete[] dcup_;
    delete[] dobj_;
    delete[] drlo_;
    delete[] drup_;
    delete matrix_;

    // initialize arrays
    this->dclo_ = new double[this->core_->getNumCols()];
    this->dcup_ = new double[this->core_->getNumCols()];
    this->dobj_ = new double[this->core_->getNumCols()];
    this->drlo_ = new double[this->core_->getNumRows()];
    this->drup_ = new double[this->core_->getNumRows()];

    int tempNels = 0; //= nels_ ? nels_: this->core_->getNumCols()*this->core_->getNumRows(); //nels_ got set by det.eq. generations. If this is not done, we have a problem.
    for (int i = 0; i < this->core_->getNumStages(); i++) //Store this value in Core..
        tempNels += this->maxNelsPerScenInStage[i];

    // We have an empty model (zero cols,rows,els)
    ncol_=0;
    nrow_=0;
    nels_=0;
    // initialize row-ordered matrix arrays
    this->dels_ = new double[tempNels+1]; //TODO: Remove the +1 as it should work without it.. something is wrong there during addition of the elements..
    this->indx_ = new int[tempNels+1];
    this->rstrt_ = new int[this->core_->getNumRows()+1];
    this->rstrt_[0] = 0;

    // Call addNode on all these nodes, this generates the problem with the deterministic data from the core nodes
    for (int t = 0; t < this->core_->getNumStages(); t++) {
        addCoreNode(this->core_->getNode(t));
    }

    // create the matrix
    matrix_ = new CoinPackedMatrix(false,0,0);
    int *len=NULL;
    // Assign values from current arrays (which get nulled thereafter)
    matrix_->assignMatrix(false,ncol_,nrow_,nels_,dels_,indx_,rstrt_,len);

    // If the core model does not contain the exepected values for the random variables,
    // we compute them. So we start with replacing the values with the ones from the first scenario (weighted by the probability),
    // and then add the weighted values from the other scenarios
    //
    this->getSmiTree()->getRoot()->getDataPtr()->setProb(1.0);


    for (int i = 0; i < this->getNumScenarios(); i++) {
        
        std::vector<SmiScnNode*> &nodes = this->getSmiTree()->getScenario(i);
        for (unsigned int j = 0; j < nodes.size(); j++) {
            if (nodes[j]->getScenarioIndex() == i) {
                addNodeEVPToCore(nodes[j]);
            }
        }
    }
    // We need to multiply all entries with the corresponding probabilities?
    // We need to generate new cdrlo[] and so on arrays!
    //core_->regenerateCoreArrays();
  
    SmiScnModel* smi = new SmiScnModel;
    smi->setCore(this->core_);
    CoinPackedMatrix matrix;
    int * indices_array_cols = new int[core_->getNumCols()];
    int * indices_array_rows = new int[core_->getNumRows()];
    for (int i = 0; i< core_->getNumCols();i++)
        indices_array_cols[i] = i;
    for (int i = 0; i< core_->getNumRows();i++)
        indices_array_rows[i] = i;
    CoinPackedVector clo(core_->getNumCols(),indices_array_cols,dclo_);
    CoinPackedVector cup(core_->getNumCols(),indices_array_cols,dcup_);
    CoinPackedVector obj(core_->getNumCols(),indices_array_cols,dobj_);
    CoinPackedVector rlo(core_->getNumRows(),indices_array_rows,drlo_);
    CoinPackedVector rup(core_->getNumRows(),indices_array_rows,drup_);
    delete [] indices_array_rows;
    delete [] indices_array_cols;
    // Generate one scenarios!
    smi->generateScenario(this->core_,matrix_,&clo,&cup,&obj,&rlo,&rup,0,0,1.0);
    // Copy deteq data
    smi->matrix_ = new CoinPackedMatrix(*matrix_);
    smi->dclo_ = new double[this->core_->getNumCols()];
    smi->dcup_ = new double[this->core_->getNumCols()];
    smi->dobj_ = new double[this->core_->getNumCols()];
    smi->drlo_ = new double[this->core_->getNumRows()];
    smi->drup_ = new double[this->core_->getNumRows()];
    // Memcpy the stuff
    memcpy(smi->dclo_,dclo_,sizeof(double)*this->core_->getNumCols());
    memcpy(smi->dcup_,dcup_,sizeof(double)*this->core_->getNumCols());
    memcpy(smi->dobj_,dobj_,sizeof(double)*this->core_->getNumCols());
    memcpy(smi->drlo_,drlo_,sizeof(double)*this->core_->getNumRows());
    memcpy(smi->drup_,drup_,sizeof(double)*this->core_->getNumRows());

    // Set integer values!
    smi->integerInd = new int[this->getIntegerLen()];
    memcpy(smi->integerInd,this->integerInd,sizeof(int)*this->integerLen);
    smi->binaryInd = new int[this->getBinaryLen()];
    memcpy(smi->binaryInd,this->binaryInd,sizeof(int)*this->binaryLen);
    smi->integerLen = this->integerLen;
    smi->binaryLen = this->binaryLen;
    smi->intIndices = this->intIndices;

    return smi;

}

std::pair<double,double*> SmiScnModel::solveEV(OsiSolverInterface *osiSolver, double objSense, bool calculateAverageValues)
{
    // We create a new osiSolver Handle at this point. 
    OsiSolverInterface* tempPtr = osiStoch_;
    osiStoch_ = osiSolver; // think about cloning or not..

    //Clean up previous stuff.
    delete[] dclo_;
    delete[] dcup_;
    delete[] dobj_;
    delete[] drlo_;
    delete[] drup_;
    delete matrix_;

    // initialize arrays
    this->dclo_ = new double[this->core_->getNumCols()];
    this->dcup_ = new double[this->core_->getNumCols()];
    this->dobj_ = new double[this->core_->getNumCols()];
    this->drlo_ = new double[this->core_->getNumRows()];
    this->drup_ = new double[this->core_->getNumRows()];

    int tempNels = 0; //= nels_ ? nels_: this->core_->getNumCols()*this->core_->getNumRows(); //nels_ got set by det.eq. generations. If this is not done, we have a problem.
    for (int i = 0; i < this->core_->getNumStages(); i++) //Store this value in Core..
        tempNels += this->maxNelsPerScenInStage[i];

    // We have an empty model (zero cols,rows,els)
    ncol_=0;
    nrow_=0;
    nels_=0;
    // initialize row-ordered matrix arrays
    this->dels_ = new double[tempNels]; //TODO: Remove the +1 as it should work without it.. something is wrong there during addition of the elements..
    this->indx_ = new int[tempNels];
    this->rstrt_ = new int[this->core_->getNumRows()+1];
    this->rstrt_[0] = 0;

    // Call addNode on all these nodes, this generates the problem with the deterministic data from the core nodes
    for (int t = 0; t < this->core_->getNumStages(); t++) {
        addCoreNode(this->core_->getNode(t));
    }
    
    // create the matrix
    matrix_ = new CoinPackedMatrix(false,0,0);
    int *len=NULL;
    // Assign values from current arrays (which get nulled thereafter)
    matrix_->assignMatrix(false,ncol_,nrow_,nels_,dels_,indx_,rstrt_,len);
    
    // If the core model does not contain the exepected values for the random variables,
    // we compute them. So we start with replacing the values with the ones from the first scenario (weighted by the probability),
    // and then add the weighted values from the other scenarios
    //
    if (calculateAverageValues) {
        for (int i = 0; i < this->getNumScenarios(); i++) {
            std::vector<SmiScnNode*> &nodes = this->getSmiTree()->getScenario(i);
            for (unsigned int j = 0; j < nodes.size(); j++) {
                if (nodes[j]->getScenarioIndex() == i) {
                    addNodeEVP(nodes[j]);
                }
            }
        }
    }
    SmiScnModel* smi = new SmiScnModel;
    smi->setCore(this->core_);
    CoinPackedVector clo(this->core_->getNumCols(),dclo_);
    CoinPackedVector cup(this->core_->getNumCols(),dcup_);
    CoinPackedVector obj(this->core_->getNumCols(),dobj_);
    CoinPackedVector rlo(this->core_->getNumRows(),drlo_);
    CoinPackedVector rup(this->core_->getNumRows(),drup_);
    smi->generateScenario(this->core_,matrix_,&clo,&cup,&obj,&rlo,&rup,0,0,1.0);

    // pass data to osiStoch
    osiStoch_->loadProblem(*matrix_,dclo_,dcup_,dobj_,drlo_,drup_); //This works only for same-dimensional subproblems.

    // load integer values in solver
    for (unsigned int i = 0; i < intIndices.size(); i++) {
        if (intIndices[i] < ncol_)
            osiStoch_->setInteger(intIndices[i]);
    }
    //Set objSense
    osiStoch_->setObjSense(objSense);
    // If we have integer indices, we need the branch-and-bound solver
    if (intIndices.size()){
        //osiStoch_->setInteger(&intIndices[0],intIndices.size());
        osiStoch_->branchAndBound();
    }
    else
        osiStoch_->initialSolve(); //Solve this problem.

    //osiStoch_->writeLp("ev_problem.lp");

    //Delete new-ed objects
    delete matrix_;
    delete [] this->dels_;
    delete [] this->indx_;
    delete [] this->rstrt_;
    matrix_ = 0; dels_ = 0; indx_ = 0; rstrt_ = 0;

    //Print out Matrix.. if in Debug Mode
    //TODO: Define proper Debug Mode..
    //TODO: Printout obj-functions
#ifdef NDEBUG
    for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMinorDim();mi++){
        std::cout << osiStoch_->getObjCoefficients()[mi] << "*x" << mi << " + ";
    }
    std::cout << std::endl;
    for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMajorDim(); mi++){
        printf("\n%g <= ", osiStoch_->getRowLower()[mi]);
        for (int mj = 0; mj < osiStoch_->getMatrixByRow()->getMinorDim(); mj++){
            printf("%g ",osiStoch_->getMatrixByRow()->getCoefficient(mi,mj));
        }
        printf("<= %g",osiStoch_->getRowUpper()[mi]);
    }
    printf("\n Matrix printed \n");
    for (int mi = 0; mi < osiStoch_->getNumCols(); mi++) {
        printf("\n%g <= %d <= %g",osiStoch_->getColLower()[mi],mi,osiStoch_->getColUpper()[mi]);
    }
#endif


    double * colSolution = new double[core_->getNumCols()];
    memcpy(colSolution, osiStoch_->getColSolution(), core_->getNumCols()* sizeof(double) );
    double objValue = osiStoch_->getObjValue();

	if ( !osiStoch_->isProvenOptimal()){
		// Not optimal
		// If infeasible
		if ( osiStoch_->isProvenPrimalInfeasible())
			objValue = osiStoch_->getInfinity();
		if ( osiStoch_->isProvenDualInfeasible()){
			objValue = -osiStoch_->getInfinity();
		}
	}
	// Reset osiStoch_ to old solver
	osiStoch_ = tempPtr;
    return make_pair(objValue,colSolution );
}

double SmiScnModel::solveEEV(OsiSolverInterface *osiSolver, double objSense, bool calculateAverageValues)
{
    OsiSolverInterface* tempPtr = osiStoch_;
    osiStoch_ = osiSolver->clone(false);
    std::pair<double, double* > evResult(solveEV(osiSolver, objSense, calculateAverageValues)); //We create a new osiSolver object in here (osiStoch_)
    

 	int numStageTMinus1Cols = core_->getColStart(core_->getNumStages()-1);
    double * colSolution = evResult.second;
	double prob = 0;

#pragma region EEVP
    double eev = 0; // the value of the EEVP

    //Clean up previous stuff.
    delete[] dclo_;
    delete[] dcup_;
    delete[] dobj_;
    delete[] drlo_;
    delete[] drup_;
    delete matrix_;

    // initialize arrays
    this->dclo_ = new double[this->core_->getNumCols()];
    this->dcup_ = new double[this->core_->getNumCols()];
    this->dobj_ = new double[this->core_->getNumCols()];
    this->drlo_ = new double[this->core_->getNumRows()];
    this->drup_ = new double[this->core_->getNumRows()];

    int tempNels = 0; //= nels_ ? nels_: this->core_->getNumCols()*this->core_->getNumRows(); //nels_ got set by det.eq. generations. If this is not done, we have a problem.
    for (int i = 0; i < this->core_->getNumStages(); i++)
        tempNels += this->maxNelsPerScenInStage[i];

    // loop over all scenarios and solve each of it individually TODO: This can get easily parallelized.
    for( int i = 0; i < this->smiTree_.getNumScenarios(); i++) {
        // We have an empty model (zero cols,rows,els)
        ncol_=0;
        nrow_=0;
        nels_=0;
        osiStoch_->reset();
        osiStoch_->messageHandler()->setLogLevel(0); // Silence solver
        // initialize row-ordered matrix arrays
        this->dels_ = new double[tempNels+1]; //TODO: Remove the +1 as it should work without it.. something is wrong there during addition of the elements..
        this->indx_ = new int[tempNels];
        this->rstrt_ = new int[this->core_->getNumRows()+1];
        this->rstrt_[0] = 0;
        // Get all nodes for this scenario
        std::vector<SmiScnNode*>& nodes = smiTree_.getScenario(i);
        // Call addNode on all these nodes
        for (unsigned int i = 0; i < nodes.size(); i++) {
            addNode(nodes[i],true);
        }

        matrix_ = new CoinPackedMatrix(false,0,0);
        int *len=NULL;
        // Assign values from current arrays (which get nulled thereafter)
        matrix_->assignMatrix(false,ncol_,nrow_,nels_,
            dels_,indx_,rstrt_,len);
        // pass data to osiStoch
        osiStoch_->loadProblem(*matrix_,dclo_,dcup_,dobj_,drlo_,drup_); //This works only for same-dimensional subproblems.

        // fix stage-1-columns to the values of the evp
		for (int col = 0; col < numStageTMinus1Cols; col++) {
			osiStoch_->setColBounds(col, colSolution[col], colSolution[col]);
		}

        //Set objSense
        osiStoch_->setObjSense(objSense);
        // If we have integer indices, we need the branch-and-bound solver
        if (intIndices.size()){
            osiStoch_->setInteger(&intIndices[0],intIndices.size());
            osiStoch_->branchAndBound();
        }
        else
            osiStoch_->initialSolve(); //Solve this problem.

        bool optimal = osiStoch_->isProvenOptimal();
        bool infeasible = osiStoch_->isProvenPrimalInfeasible();
        bool unbounded = osiStoch_->isProvenDualInfeasible();

        if ( infeasible ){
            eev = osiStoch_->getInfinity();
            // Finish with computations!
            i = this->smiTree_.getNumScenarios();
        }
        if ( unbounded ){
            eev = -osiStoch_->getInfinity();
            i = this->smiTree_.getNumScenarios();
        }

		if (optimal){
            eev += osiStoch_->getObjValue() * nodes[nodes.size()-1]->getProb();
			prob += nodes[nodes.size()-1]->getProb();
		}
		else {
			std::cout << "test";
		}
        // sum up the solution values (multiplied with probabilities)
        

        //Delete new-ed objects
        delete matrix_;
        delete [] this->dels_;
        delete [] this->indx_;
        delete [] this->rstrt_;
        matrix_ = 0; dels_ = 0; indx_ = 0; rstrt_ = 0;

        //Print out Matrix.. if in Debug Mode
        //TODO: Define proper Debug Mode..
        //TODO: Printout obj-functions
#ifdef NDEBUG
        for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMinorDim();mi++){
            std::cout << osiStoch_->getObjCoefficients()[mi] << "*x" << mi << " + ";
        }
        std::cout << std::endl;
        for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMajorDim(); mi++){
            printf("\n%g <= ", osiStoch_->getRowLower()[mi]);
            for (int mj = 0; mj < osiStoch_->getMatrixByRow()->getMinorDim(); mj++){
                printf("%g ",osiStoch_->getMatrixByRow()->getCoefficient(mi,mj));
            }
            printf("<= %g",osiStoch_->getRowUpper()[mi]);
        }
        printf("\n Matrix printed \n");
        for (int mi = 0; mi < osiStoch_->getNumCols(); mi++) {
            printf("\n%g <= %d <= %g",osiStoch_->getColLower()[mi],mi,osiStoch_->getColUpper()[mi]);

        }
        printf("\n");
#endif
    }
#pragma endregion EEVP;

    delete [] colSolution;
    delete osiStoch_;
    osiStoch_ = tempPtr;
    return eev;
}

std::vector< std::pair<double,double*> > SmiScnModel::solveWS(OsiSolverInterface *osiSolver, double objSense) 
{
    OsiSolverInterface* tempPtr = osiStoch_;
    osiStoch_ = osiSolver->clone(false); // We copy the existing solverHandle, so we have to delete it at the end?

    //Clean up previous stuff.
    delete[] dclo_;
    delete[] dcup_;
    delete[] dobj_;
    delete[] drlo_;
    delete[] drup_;
    delete matrix_;

    // initialize arrays
    this->dclo_ = new double[this->core_->getNumCols()];
    this->dcup_ = new double[this->core_->getNumCols()];
    this->dobj_ = new double[this->core_->getNumCols()];
    this->drlo_ = new double[this->core_->getNumRows()];
    this->drup_ = new double[this->core_->getNumRows()];

    int tempNels = 0; //= nels_ ? nels_: this->core_->getNumCols()*this->core_->getNumRows(); //nels_ got set by det.eq. generations. If this is not done, we have a problem.
    for (int i = 0; i < this->core_->getNumStages(); i++)
        tempNels += this->maxNelsPerScenInStage[i];

    std::vector<std::pair<double,double*> > solutionValues;
    solutionValues.reserve(this->smiTree_.getNumScenarios());


    CoinWarmStart* warm_start = osiStoch_->getEmptyWarmStart();
    // loop over all scenarios and solve each of it individually
    for( int i = 0; i < this->smiTree_.getNumScenarios(); i++) {
        // We have an empty model (zero cols,rows,els)
        ncol_=0;
        nrow_=0;
        nels_=0;
        //TODO: Create a new SolverObject by using the type of osiStoch. Need to load osiCoreData in the newly created osiStoch..
        osiStoch_->reset();
        // initialize row-ordered matrix arrays
        this->dels_ = new double[tempNels]; 
        this->indx_ = new int[tempNels];
        this->rstrt_ = new int[this->core_->getNumRows()+1];
        this->rstrt_[0] = 0;
        // Get all nodes for this scenario
        std::vector<SmiScnNode*>& nodes = smiTree_.getScenario(i);
        // Call addNode on all these nodes
        for (unsigned int i = 0; i < nodes.size(); i++) {
            addNode(nodes[i],true);
        }

        matrix_ = new CoinPackedMatrix(false,0,0);
        int *len=NULL;
        // Assign values from current arrays (which get nulled thereafter)
        matrix_->assignMatrix(false,ncol_,nrow_,nels_,
            dels_,indx_,rstrt_,len);
        // pass data to osiStoch
        osiStoch_->loadProblem(*matrix_,dclo_,dcup_,dobj_,drlo_,drup_); //This works only for same-dimensional subproblems.

        //Set objSense
        osiStoch_->setObjSense(objSense);
            // If we have integer indices, we need the branch-and-bound solver
        // Silence Cplex!
        osiStoch_->messageHandler()->setLogLevel(0);
		int integer_cutoff = -1;
		for (int i = 0; i < intIndices.size(); i++){
			if ( intIndices[i] >= core_->getColStart(1)){
				integer_cutoff = i;
				break;
			}
			else 
				integer_cutoff = intIndices.size();
			
		}
        if (intIndices.size() && integer_cutoff > 0){
			// We set Integers only for the first stage!
            osiStoch_->setInteger(&intIndices[0],integer_cutoff);
            osiStoch_->branchAndBound();
        }
        else{
            osiStoch_->setWarmStart(warm_start);
            //delete warm_start;
            osiStoch_->initialSolve(); //Solve this problem.
            
            warm_start = osiStoch_->getWarmStart();
        }
        double* solution_array = new double[osiStoch_->getNumCols()];
        memcpy(solution_array,osiStoch_->getColSolution(),sizeof(double)*osiStoch_->getNumCols());
        solutionValues.push_back(std::make_pair(osiStoch_->getObjValue(),solution_array));

        //Delete new-ed objects
        delete matrix_;
        delete [] this->dels_;
        delete [] this->indx_;
        delete [] this->rstrt_;
        matrix_ = 0; dels_ = 0; indx_ = 0; rstrt_ = 0;

        //Print out Matrix.. if in Debug Mode
        //TODO: Define proper Debug Mode..
        //TODO: Printout obj-functions
#ifdef NDEBUG
        for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMinorDim();mi++){
            std::cout << osiStoch_->getObjCoefficients()[mi] << "*x" << mi << " + ";
        }
        std::cout << std::endl;
        for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMajorDim(); mi++){
            printf("\n%g <= ", osiStoch_->getRowLower()[mi]);
            for (int mj = 0; mj < osiStoch_->getMatrixByRow()->getMinorDim(); mj++){
                printf("%g ",osiStoch_->getMatrixByRow()->getCoefficient(mi,mj));
            }
            printf("<= %g",osiStoch_->getRowUpper()[mi]);
        }
        printf("\n Matrix printed \n");
        for (int mi = 0; mi < osiStoch_->getNumCols(); mi++) {
            printf("\n%g <= %d <= %g",osiStoch_->getColLower()[mi],mi,osiStoch_->getColUpper()[mi]);
        }
        printf("\n");
#endif
    }
    delete osiStoch_;
    osiStoch_ = tempPtr;
    return solutionValues;
}

OsiSolverInterface*
SmiScnModel::loadStoredDataInSolver(){
    osiStoch_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    // pass data to osiStoch
    osiStoch_->loadProblem(*matrix_,dclo_,dcup_,dobj_,drlo_,drup_);

    // If we have integer indices, we add these to the solver.
    if (intIndices.size()){
        osiStoch_->setInteger(&intIndices[0],intIndices.size());
    }
    return osiStoch_;
}

//Christian: Generates Deterministic Equivalent (Big Matrix)
OsiSolverInterface *
SmiScnModel::loadOsiSolverData()
{
    // We want to reset our osiSolverInterface
    // If we have a cplex solver, we can work around that by loading an empty model..
    //OsiCpxSolverInterface* osiSolver = dynamic_cast<OsiCpxSolverInterface *>(osiStoch_);
    //if (NULL != osiSolver)
    //{ // If we really have an OsiCpxSolverInterface, we do the workaround
    //	//
    osiStoch_->loadProblem(0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    //}
    //else
    //    osiStoch_->reset();

    delete[] dclo_;
    delete[] dcup_;
    delete[] dobj_;
    delete[] drlo_;
    delete[] drup_;
    delete matrix_;

    // initialize arrays
    this->dclo_ = new double[this->ncol_];
    this->dcup_ = new double[this->ncol_];
    this->dobj_ = new double[this->ncol_];
    this->drlo_ = new double[this->nrow_];
    this->drup_ = new double[this->nrow_];

    // initialize row-ordered matrix arrays, needed for SmiScnModelAddNode!
    this->dels_ = new double[this->nels_+1]; //TODO: Remove the +1 as it should work without it.. something is wrong there during addition of the elements..
    this->indx_ = new int[this->nels_];
    this->rstrt_ = new int[this->nrow_+1];
    this->rstrt_[0] = 0;
    this->nels_max = nels_;

    ncol_=0;
    nrow_=0;
    nels_=0;

    // loop to addNodes
    for_each(smiTree_.treeBegin(),smiTree_.treeEnd(),SmiScnModelAddNode(this));

    //What happens if no ScenarioTree is present, but a core model is?
    matrix_ = new CoinPackedMatrix(false,0,0);
    int *len=NULL;
    matrix_->assignMatrix(false,ncol_,nrow_,nels_,
        dels_,indx_,rstrt_,len);

    // pass data to osiStoch
    osiStoch_->loadProblem(*matrix_,dclo_,dcup_,dobj_,drlo_,drup_);

    // If we have integer indices, we add these to the solver.
    if (intIndices.size()){
        osiStoch_->setInteger(&intIndices[0],intIndices.size());
    }


    //Print out Matrix.. if in Debug Mode
    //TODO: Define proper Debug Mode..
    //TODO: Printout obj-functions
#ifndef NDEBUG 
    //   for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMinorDim();mi++){
    //       std::cout << osiStoch_->getObjCoefficients()[mi] << "*x" << mi << " + ";
    //   }
    //   std::cout << std::endl;
    //for (int mi = 0; mi < osiStoch_->getMatrixByRow()->getMajorDim(); mi++){
    //	printf("\n%g <= ", osiStoch_->getRowLower()[mi]);
    //	for (int mj = 0; mj < osiStoch_->getMatrixByRow()->getMinorDim(); mj++){
    //		printf("%g ",osiStoch_->getMatrixByRow()->getCoefficient(mi,mj));
    //	}
    //	printf("<= %g",osiStoch_->getRowUpper()[mi]);
    //       //printf(" %g",this->core_->getRowStage(mi)); //We have to transform indices back to core model..
    //}
    //printf("\n Matrix printed \n");
#endif
    return osiStoch_;
}

//Christian: Generate Submodel Specified by Stage and Scenario Number
// This Model contains only constraints for the specified stage and scenario numbers..
//TODO: Not yet implemented
OsiSolverInterface *
SmiScnModel::loadOsiSolverDataForSubproblem(int stage, int scenStart)
{

    //TODO Declare new SolverInterface?
    osiStoch_->reset();

    delete[] dclo_;
    delete[] dcup_;
    delete[] dobj_;
    delete[] drlo_;
    delete[] drup_;
    delete matrix_;

    // initialize arrays with correct dimensions
    //this->core_->nColInStage_
    this->dclo_ = new double[this->ncol_];
    this->dcup_ = new double[this->ncol_];
    this->dobj_ = new double[this->ncol_];
    this->drlo_ = new double[this->nrow_];
    this->drup_ = new double[this->nrow_];

    // initialize row-ordered matrix arrays
    this->dels_ = new double[this->nels_];
    this->indx_ = new int[this->nels_];
    this->rstrt_ = new int[this->nrow_+1];
    this->rstrt_[0] = 0;
    this->nels_max = nels_;

    ncol_=0;
    nrow_=0;
    nels_=0;

    // loop to addNodes
    for_each(smiTree_.treeBegin(),smiTree_.treeEnd(),SmiScnModelAddNode(this));

    matrix_ = new CoinPackedMatrix(false,0,0);
    int *len=NULL;
    matrix_->assignMatrix(false,ncol_,nrow_,nels_,
        dels_,indx_,rstrt_,len);

    // pass data to osiStoch
    osiStoch_->loadProblem(CoinPackedMatrix(*matrix_),dclo_,dcup_,dobj_,drlo_,drup_);

    return osiStoch_;
    ////	SmiScnNode tnode;
    //	SmiNodeData *node = tnode->getNode();
    //
    //	// set offsets for current node
    //	tnode->setColOffset(ncol_);
    //	tnode->setRowOffset(nrow_);
    //
    //	if (tnode->isVirtualNode())
    //	{
    //		// update column count
    //		ncol_ += tnode->getNumCols();
    //		return;
    //	}
    //
    //	// OsiSolverInterface *osi = this->osiStoch_;
    //	SmiCoreData *core = node->getCore();
    //
    //	// get stage and associated core node
    //	int stg = node->getStage();
    //	SmiNodeData *cnode = core->getNode(stg);
    //
    //	// pretty sure this is an error? //Christian: At a first glance, looks like it..
    //	core->copyRowLower(drlo_+nrow_,stg);
    //	core->copyRowUpper(drup_+nrow_,stg);
    //	core->copyColLower(dclo_+ncol_,stg);
    //	core->copyColUpper(dcup_+ncol_,stg);
    //	core->copyObjective(dobj_+ncol_,stg);
    //
    //
    //	node->copyColLower(dclo_+ncol_);
    //	node->copyColUpper(dcup_+ncol_);
    //	node->copyObjective(dobj_+ncol_);
    //	node->copyRowLower(drlo_+nrow_);
    //	node->copyRowUpper(drup_+nrow_);
    //
    //	// multiply obj coeffs by node probability and normalize
    //	double prob = tnode->getProb()/this->totalProb_;
    //	tnode->setModelProb(prob);
    //
    //	for(int j=ncol_; j<ncol_+core->getNumCols(stg); ++j)
    //		dobj_[j] *= prob;
    //
    //
    //	vector<int> stochColStart(stg+1);
    //	SmiScnNode *pnode=tnode;
    //
    //	stochColStart[stg]=ncol_;
    //	for (int t=stg-1; t>0; t--)
    //	{
    //		pnode=pnode->getParent();
    //		stochColStart[t] = pnode->getColStart();
    //	}
    //
    //	// row counter
    //	int rowCount=nrow_;
    //
    //	// add rows to matrix
    //	for (int i=core->getRowStart(stg); i<core->getRowStart(stg+1) ; i++)
    //	{
    //		if (stg)
    //		{
    //			// build row explicitly into sparse arrays
    //			int rowStart=this->rstrt_[rowCount];
    //			int rowNumEls=0;
    //			if (node->getRowLength(i))
    //			{
    //				double *denseCoreRow = cnode->getDenseRow(i);
    //				rowNumEls=node->combineWithDenseCoreRow(denseCoreRow,node->getRowLength(i),node->getRowIndices(i),node->getRowElements(i),dels_+rowStart,indx_+rowStart);
    //			}
    //			else
    //			{
    //				const double *cels=cnode->getRowElements(i);
    //				const int *cind=cnode->getRowIndices(i);
    //				const int len=cnode->getRowLength(i);
    //				memcpy(dels_+rowStart,cels,sizeof(double)*len);
    //				memcpy(indx_+rowStart,cind,sizeof(int)*len);
    //				rowNumEls=len;
    //			}
    //
    //
    //			rowCount++;
    //			nels_+=rowNumEls;
    //			this->rstrt_[rowCount] = nels_;
    //
    //			// coefficients of new row
    //			int *indx = indx_+rowStart;
    //
    //			// stage starts
    //			int t=stg;
    //			int jlo=core->getColStart(stg);
    //
    //			// net offset to be added to indices
    //			int coff= stochColStart[stg]-jlo;
    //
    //			if(coff)
    //			{
    //				// TODO -- decide if better to sort COL indices in core
    //#if 1
    //
    //				// main loop iterates backwards through indices
    //				for (int j=rowNumEls-1; j>-1;--j)
    //				{
    //					// get new offset from parent node when crossing stage bndy
    //					while (indx[j]<jlo)
    //					{
    //						jlo = core->getColStart(--t);
    //						coff = stochColStart[t] - jlo;
    //					}
    //
    //					// add offset to index
    //					indx[j]+=coff;
    //				}
    //#else
    //				for (int j=0; j<rowNumEls; ++j)
    //				{
    //					t = core->getColStage(indx[j]);
    //					indx[j] += stochColStart[t] - core->getColStart(t);
    //				}
    //#endif
    //			}
    //		}
    //		else
    //		{
    //			// build row explicitly into sparse arrays
    //			const double *els = cnode->getRowElements(i);
    //			const int *ind = cnode->getRowIndices(i);
    //			const int len = cnode->getRowLength(i);
    //
    //			int rowStart=this->rstrt_[rowCount];
    //
    //			memcpy(dels_+rowStart,els,sizeof(double)*len);
    //			memcpy(indx_+rowStart,ind,sizeof(int)*len);
    //
    //			rowCount++;
    //			nels_+=len;
    //			this->rstrt_[rowCount] = nels_;
    //
    //		}
}


void
SmiScnModel::deleteNode(SmiScnNode *tnode)
{
    //cout << "Deleting node from scenario " << tnode->getScenarioIndex() << " Stage " <<  tnode->getNode()->getStage() << endl;
    delete tnode;
}

void
SmiScnModel::addNode(SmiScnNode *tnode,bool notDetEq /* = false */)
{

    SmiNodeData *node = tnode->getNode();

    // set offsets for current node
    tnode->setColOffset(ncol_);
    tnode->setRowOffset(nrow_);

    if (tnode->isVirtualNode())
    {
        // update column count
        ncol_ += tnode->getNumCols();
        return;
    }

    // OsiSolverInterface *osi = this->osiStoch_;
    SmiCoreData *core = node->getCore();

    // get stage and associated core node 
    int stg = node->getStage();
    SmiNodeData *cnode = core->getNode(stg);

    //TODO: Generate integer information from information given in core node (offset manipulation)
    // Get index list of integers for current stage
    // Iterate through list 
    //    add offset
    //    add to overall integer list
    //cout << "Stage " << stg << endl;
    vector<int> intCols = core->getIntCols(stg);
    for (unsigned int i = 0; i < intCols.size(); i++) {
        //cout << intCols[i] << " + " << ncol_ << endl; // Prints out stage information for given column
        this->addIntIndice(intCols[i] + ncol_);
    }

    // pretty sure this is an error? //Christian: At a first glance, looks like it..
    //Christian: Copy and Combine with Core-Arrays to create Arrays for current scenario and stage
    //core->copyRowLower(drlo_+nrow_,stg);
    //core->copyRowUpper(drup_+nrow_,stg);
    //core->copyColLower(dclo_+ncol_,stg);
    //core->copyColUpper(dcup_+ncol_,stg);
    //core->copyObjective(dobj_+ncol_,stg);
    //Copy Core Data and Replace Stochastic Data to obtain a copy for the det. eq.
    node->copyColLower(dclo_+ncol_);
    node->copyColUpper(dcup_+ncol_);
    node->copyObjective(dobj_+ncol_);
    node->copyRowLower(drlo_+nrow_);
    node->copyRowUpper(drup_+nrow_);

    // multiply obj coeffs by node probability and normalize
    double prob = !notDetEq ? tnode->getProb()/this->totalProb_ : 1; //Christian: TODO: totalProb_ = Summer aller absoluten W'keiten (am Ende müsste die bei 1 sein..?!)
    tnode->setModelProb(prob);

    for(int j=ncol_; j<ncol_+core->getNumCols(stg); ++j)
        dobj_[j] *= prob;

    //Christian: Get Column Start Values for every stage less than current stage. This depends on the ordering of nodes in the nodes array..
    vector<int> stochColStart(stg+1);
    SmiScnNode *pnode=tnode;

    stochColStart[stg]=ncol_;
    for (int t=stg-1; t>0; t--)
    {
        pnode=pnode->getParent();
        stochColStart[t] = pnode->getColStart();
    }

    // row counter : initialized with number of rows counted so far
    int rowCount=nrow_;

    // add rows to det. eq. matrix for current stage
    for (int i=core->getRowStart(stg); i<core->getRowStart(stg+1) ; i++)
    {

        // build row explicitly into sparse arrays
        int rowStart=this->rstrt_[rowCount];
        int rowNumEls=0;
        //Christian: In my opinion the if/else is the same for all rows in a given stage
        //Christian: If row has stochastic coefficients (that implies that we are not in stage one which means 0 here)
        if (stg && node->getRowLength(i))
        {	//Christian: If I understood it correctly, a dense row is inserted (a row with values for all collumns)
            double *denseCoreRow = cnode->getDenseRow(i); //Christian: Why is the dense core row needed?? I don't get it..
            //I think the DenseCoreRow is needed because it simplifies things a little bit on the cost of performance
            //TODO: Change methods to use CompressedRowStorage for the core row also.. (Performance)
            //Christian: Returned row is a row that contains only non-zero elements, so it is not dense anymore
            rowNumEls=node->combineWithDenseCoreRow(denseCoreRow,node->getRowLength(i),node->getRowIndices(i),node->getRowElements(i),dels_+rowStart,indx_+rowStart);
        }
        //Christian: If row does not have stochastic coefficients, copy values from core node for current stage (no combination needed..)
        else
        {
            const double *cels=cnode->getRowElements(i);
            const int *cind=cnode->getRowIndices(i);
            const int len=cnode->getRowLength(i);
            memcpy(dels_+rowStart,cels,sizeof(double)*len);
            memcpy(indx_+rowStart,cind,sizeof(int)*len);
            rowNumEls=len;
        }

        //preparation for the next row
        rowCount++;
        nels_+=rowNumEls;
        this->rstrt_[rowCount] = nels_;
        //done with preparations for the next row

        //Coefficients of the newly added row needs to get adjusted, as they differ from the core model
        //We introduce the new variables for the det. eq. via the indx. array
        //This can only be done if we are not in the first stage aka stage 0
        if (stg) {

            int *indx = indx_+rowStart;

            // We start with the current stage
            int t=stg;
            int jlo=core->getColStart(stg);

            // net offset to be added to indices for the current stage
            int coff= stochColStart[stg]-jlo;
            if(coff)
            {
                // TODO -- decide if better to sort COL indices in core
#if 1

                // main loop iterates backwards through indices
                for (int j=rowNumEls-1; j>-1;--j)
                {
                    // get new offset from parent node when crossing stage boundary
                    // This happens, if the index of the coefficient at hand is less than the index of the first coefficient belonging to the current stage.
                    // In this case the index at hand is actually from an variable of an lower stage. Repeat procedure, until index is at the correct stage.
                    while (indx[j]<jlo)
                    {
                        jlo = core->getColStart(--t);
                        coff = stochColStart[t] - jlo;
                    }

                    // add offset to index
                    indx[j]+=coff;
                }
#else
                for (int j=0; j<rowNumEls; ++j)
                {
                    t = core->getColStage(indx[j]);
                    indx[j] += stochColStart[t] - core->getColStart(t);
                }
#endif
            }
        }

    }
    // update row, col counts
    ncol_ += core->getNumCols(stg);
    nrow_ += core->getNumRows(stg);

    // sanity check
    //assert(! ( this->nels_ > this->nels_max ) );

}

void
SmiScnModel::addCoreNode(SmiNodeData *node)
{

    SmiCoreData *core = node->getCore();
    int stg = node->getStage();

    //Generate integer information from information given in core node (offset manipulation)
    vector<int> intCols = core->getIntCols(stg);
    for (unsigned int i = 0; i < intCols.size(); i++) {
        //cout << intCols[i] << " + " << ncol_ << endl; // Prints out stage information for given column
        this->addIntIndice(intCols[i] + ncol_);
    }


    core->copyRowLower(drlo_+nrow_,stg);
    core->copyRowUpper(drup_+nrow_,stg);
    core->copyColLower(dclo_+ncol_,stg);
    core->copyColUpper(dcup_+ncol_,stg);
    core->copyObjective(dobj_+ncol_,stg);


    // row counter : initialized with number of rows counted so far
    int rowCount=nrow_;

    // add rows to det. eq. matrix for current stage
    for (int i=core->getRowStart(stg); i<core->getRowStart(stg+1) ; i++)
    {

        // build row explicitly into sparse arrays
        int rowStart=this->rstrt_[rowCount];
        int rowNumEls=0;

        const double *cels=node->getRowElements(i);
        const int *cind=node->getRowIndices(i);
        const int len=node->getRowLength(i);
        memcpy(dels_+rowStart,cels,sizeof(double)*len);
        memcpy(indx_+rowStart,cind,sizeof(int)*len);
        rowNumEls=len;

        //preparation for the next row
        rowCount++;
        nels_+=rowNumEls;
        this->rstrt_[rowCount] = nels_;
        //done with preparations for the next row

    }
    // update row, col counts
    ncol_ += core->getNumCols(stg);
    nrow_ += core->getNumRows(stg);

    // sanity check
    //assert(! ( this->nels_ > this->nels_max ) );

}

void
SmiScnModel::addNodeEVP(SmiScnNode *tnode)
{
    SmiNodeData *snode = tnode->getNode();
    int stage = snode->getStage();

    // Decide whether to replace or to add the (weighted) stochastic data: when we are in the first stage,
    // and the SmiCoreCombineRule from the node is replace, then we replace the values. Otherwise, we
    // have to add the (weighted) values.
    SmiCoreCombineRule* combineRule;
    if (tnode->getScenarioIndex() == 0 && snode->getCoreCombineRule() == SmiCoreCombineReplace::Instance()) {
        combineRule = SmiCoreCombineReplace::Instance();
    } else {
        combineRule = SmiCoreCombineAdd::Instance();
    }

    // Do it for the 5 arrays column lower, column upper, objective, row lower & row upper
    if (snode->getColLowerLength()) {
        CoinPackedVector vclo = CoinPackedVector(snode->getColLowerLength(), snode->getColLowerIndices(), snode->getColLowerElements(), false);
        vclo *= tnode->getProb();
        combineRule->Process(dclo_, 0, vclo);
    }

    if (snode->getColUpperLength()) {
        CoinPackedVector vcup = CoinPackedVector(snode->getColUpperLength(), snode->getColUpperIndices(), snode->getColUpperElements(), false);
        vcup *= tnode->getProb();
        combineRule->Process(dcup_, 0, vcup);
    }

    if (snode->getObjectiveLength()) {
        CoinPackedVector vobj = CoinPackedVector(snode->getObjectiveLength(), snode->getObjectiveIndices(), snode->getObjectiveElements(), false);
        vobj *= tnode->getProb();
        combineRule->Process(dobj_, 0, vobj);
    }

    if (snode->getRowLowerLength()) {
        CoinPackedVector vrlo = CoinPackedVector(snode->getRowLowerLength(), snode->getRowLowerIndices(), snode->getRowLowerElements(), false);
        vrlo *= tnode->getProb();
        combineRule->Process(drlo_, 0, vrlo);
    }

    if (snode->getRowUpperLength()) {
        CoinPackedVector vrup = CoinPackedVector(snode->getRowUpperLength(), snode->getRowUpperIndices(), snode->getRowUpperElements(), false);
        vrup *= tnode->getProb();
        combineRule->Process(drup_, 0, vrup);
    }

    // And now for the matrix entries
    if (snode->getNumMatrixElements()) {
        for (int i = core_->getRowStart(stage); i < core_->getRowStart(stage+1); i++) {
            for (int j = 0; j < snode->getRowLength(i); j++) {
                if (combineRule == SmiCoreCombineReplace::Instance())
                    this->matrix_->modifyCoefficient(i, snode->getRowIndices(i)[j], snode->getRowElements(i)[j] * tnode->getProb(),true);
                else
                    this->matrix_->modifyCoefficient(i, snode->getRowIndices(i)[j], this->matrix_->getCoefficient(i, snode->getRowIndices(i)[j]) + snode->getRowElements(i)[j] * tnode->getProb(),true); // We must keep zero entries
            }
        }
    }
}

void
SmiScnModel::addNodeEVPToCore(SmiScnNode *tnode)
{
    SmiNodeData *snode = tnode->getNode();
    int stage = snode->getStage();
    int col_beg = core_->getColStart(stage);
    int row_beg = core_->getRowStart(stage);
    SmiNodeData *core_node = core_->getNode(stage);

    // Decide whether to replace or to add the (weighted) stochastic data: when we are in the first stage,
    // and the SmiCoreCombineRule from the node is replace, then we replace the values. Otherwise, we
    // have to add the (weighted) values.
    SmiCoreCombineRule* combineRule;
    if (tnode->getScenarioIndex() == 0 && snode->getCoreCombineRule() == SmiCoreCombineReplace::Instance()) {
        combineRule = SmiCoreCombineReplace::Instance();
    } else {
        combineRule = SmiCoreCombineAdd::Instance();
    }

    // Do it for the 5 arrays column lower, column upper, objective, row lower & row upper
    if (snode->getColLowerLength()) {
        CoinPackedVector vclo = CoinPackedVector(snode->getColLowerLength(), snode->getColLowerIndices(), snode->getColLowerElements(), false);
        CopyStochasticDataToDetEQModel(core_->getNumCols(stage),core_->getNode(stage)->getColLowerElements(),dclo_,col_beg,vclo,combineRule,tnode->getProb());
    }

    if (snode->getColUpperLength()) {
        CoinPackedVector vcup = CoinPackedVector(snode->getColUpperLength(), snode->getColUpperIndices(), snode->getColUpperElements(), false);
        CopyStochasticDataToDetEQModel(core_->getNumCols(stage),core_->getNode(stage)->getColUpperElements(),dcup_,col_beg,vcup,combineRule,tnode->getProb());
    }

    if (snode->getObjectiveLength()) {
        CoinPackedVector vobj = CoinPackedVector(snode->getObjectiveLength(), snode->getObjectiveIndices(), snode->getObjectiveElements(), false);
        CopyStochasticDataToDetEQModel(core_->getNumCols(stage),core_->getNode(stage)->getObjectiveElements(),dobj_,col_beg,vobj,combineRule,tnode->getProb());
    }

    if (snode->getRowLowerLength()) {
        CoinPackedVector vrlo = CoinPackedVector(snode->getRowLowerLength(), snode->getRowLowerIndices(), snode->getRowLowerElements(), false);
        CopyStochasticDataToDetEQModel(core_->getNumRows(stage),core_->getNode(stage)->getRowLowerElements(),drlo_,row_beg,vrlo,combineRule,tnode->getProb());
    }

    if (snode->getRowUpperLength()) {
        CoinPackedVector vrup = CoinPackedVector(snode->getRowUpperLength(), snode->getRowUpperIndices(), snode->getRowUpperElements(), false);
        CopyStochasticDataToDetEQModel(core_->getNumRows(stage),core_->getNode(stage)->getRowUpperElements(),drup_,row_beg,vrup,combineRule,tnode->getProb());
    }

    // And now for the matrix entries
    // To rule out all eventualities, we need to get the matrix from each scenario, and add it up to one single matrix!
    
    // Get core matrix and replace entries with standard values from core model!
    // Iterate over stochastic stuff to!
    for (int i = core_->getRowStart(stage); i < core_->getRowStart(stage+1); i++) {
        int node_j = 0;
        for (int j = 0; j < core_node->getRowLength(i); j++) {
            double matrix_coef = core_node->getRowElements(i)[j];
            if ( snode->hasMatrix() && core_node->getRowIndices(i)[j] == snode->getRowIndices(i)[node_j]){
                matrix_coef = snode->getRowElements(i)[node_j];
                node_j++;
            }
            if (combineRule == SmiCoreCombineReplace::Instance())
                matrix_->modifyCoefficient(i,core_node->getRowIndices(i)[j],core_node->getRowElements(i)[j]*tnode->getProb(),true);
            else
                matrix_->modifyCoefficient(i,core_node->getRowIndices(i)[j],matrix_->getCoefficient(i,core_node->getRowIndices(i)[j])+core_node->getRowElements(i)[j]*tnode->getProb(),true);
        }
    }
    // Replace entries of this node

    //// Multiply all elements with probability and replace the entries of the deterministic matrix!!!!!!!!
    //if (snode->getNumMatrixElements()) {
    //    for (int i = core_->getRowStart(stage); i < core_->getRowStart(stage+1); i++) {
    //        for (int j = 0; j < snode->getRowLength(i); j++) {
    //            if (combineRule == SmiCoreCombineReplace::Instance())
    //                matrix_->modifyCoefficient(i,snode->getRowIndices(i)[j],snode->getRowElements[i](j)*tnode->getProb(),true);
    //            else
    //                matrix_->modifyCoefficient(i,snode->getRowIndices(i)[j],matrix_->getCoefficient(i,core_node->getRowIndices(i)[j])+snode->getRowElements[i](j)*tnode->getProb()-,true);
    //        }
    //    }
    //}
}



OsiSolverInterface *
SmiScnModel::getOsiSolverInterface()
{
    return osiStoch_;
}


double *
SmiScnModel::getColSolution(int ns, int *length)
{
    //CoinPackedVector *soln=new CoinPackedVector();
    const double * osiSoln = this->getOsiSolverInterface()->getColSolution();
    int numcols=0;

    assert( ns < this->getNumScenarios() );

    // start with leaf node
    SmiScnNode *node = this->getLeafNode(ns);
    while (node != NULL){
        // accumulate number of collumns along scenario
        numcols+=node->getNumCols();

        // get parent of node
        node = node->getParent();
    }

    // malloc vector
    double *dsoln = (double *)calloc(numcols,sizeof(double));

    // start with leaf node
    node = this->getLeafNode(ns);
    while (node != NULL){
        // copy entries
        // getColStart returns the starting index of node in OSI model
        for(int j=node->getColStart(); j<node->getColStart()+node->getNumCols(); ++j){
            // getCoreRowIndex returns the corresponding Core index
            // in the original (user's) ordering
            dsoln[node->getCoreColIndex(j)] = osiSoln[j];
        }
        // get parent of node
        node = node->getParent();
    }
    *length=numcols;
    return dsoln;
}

double 
SmiScnModel::getColSolution(int ns, int stage, int colIndex)
{
    //CoinPackedVector *soln=new CoinPackedVector();
    const double * osiSoln = this->getOsiSolverInterface()->getColSolution();

    assert( ns < this->getNumScenarios() );
    assert( stage < this->core_->getNumStages() );

    SmiScnNode *node = this->getLeafNode(ns);
    int curStage = this->core_->getNumStages() - stage-1; // Number of stages
    while (node != NULL && curStage){ // Go to the correct node. If Stage is reduced to zero we are at the correct node.
        // get parent of node
        node = node->getParent();
        --curStage;
    }

    assert( colIndex < core_->getColStart(stage)+node->getNumCols() && colIndex >= core_->getColStart(stage));
    return osiSoln[node->getColStart()+colIndex-core_->getColStart(stage)];// Return element in matrix that is at nodeColStart + colIndex for stage. Where to get colIndex for stage?
}

double *
SmiScnModel::getRowSolution(int ns, int *length)
{
    //CoinPackedVector *soln=new CoinPackedVector();
    const double * osiSoln = this->getOsiSolverInterface()->getRowActivity();
    int numrows=0;

    assert( ns < this->getNumScenarios() );


    // start with leaf node
    SmiScnNode *node = this->getLeafNode(ns);
    while (node != NULL){
        // accumulate number of rows along scenario
        numrows+=node->getNumRows();

        // get parent of node
        node = node->getParent();
    }

    // malloc vector
    double *dsoln = (double *)calloc(numrows,sizeof(double));

    // start with leaf node
    node = this->getLeafNode(ns);
    while (node != NULL){
        // copy entries
        // getRowStart returns the starting index of node in OSI model
        for(int j=node->getRowStart(); j<node->getRowStart()+node->getNumRows(); ++j){
            // getCoreRowIndex returns the corresponding Core index
            // in the original (user's) ordering
            dsoln[node->getCoreRowIndex(j)] = osiSoln[j];
        }
        // get parent of node
        node = node->getParent();
    }
    *length=numrows;
    return dsoln;
}

double 
SmiScnModel::getRowSolution(int ns, int stage, int rowIndex)
{
    //CoinPackedVector *soln=new CoinPackedVector();
    const double * osiSoln = this->getOsiSolverInterface()->getRowActivity();
    assert( ns < this->getNumScenarios() );
    assert( stage < this->core_->getNumStages() );
    // start with leaf node
    SmiScnNode *node = this->getLeafNode(ns);
    int curStage = this->core_->getNumStages() - stage-1; // Number of stages
    while (node != NULL && curStage){ // Go to the correct node. If Stage is reduced to zero we are at the correct node.
        // get parent of node
        node = node->getParent();
        --curStage;
    }

    assert( rowIndex < core_->getRowStart(stage)+node->getNumRows() && rowIndex >= core_->getRowStart(stage));
    return osiSoln[node->getRowStart()+rowIndex-core_->getRowStart(stage)];// Return element in matrix that is at nodeColStart + colIndex for stage. Where to get colIndex for stage?
}


int
SmiScnModel::readSmps(const char *c, SmiCoreCombineRule *r)
{
    return readSmps(c,c,c,r);
}

int
SmiScnModel::readSmps(const char *coreFile, const char* timeFile, const char* stochFile,SmiCoreCombineRule *r)
{
    int i;
    SmiSmpsIO *smiSmpsIO=NULL;
    string coreFname(coreFile);
    string timeFname(timeFile);
    string stochFname(stochFile);

    const char* core_ext[] = {"cor","core"};
    for (i = sizeof(core_ext)/sizeof(const char*) - 1; i >= 0; --i) {
        string ext(core_ext[i]);
        string fullname=coreFname+"."+ext;
        if (fileCoinReadable(fullname))
            break;
    }
    if (i == -1)
    {
        cerr << "SmiScnModel::readSmps() - No file "<< coreFname <<" with extensions .core or .cor were found." << endl;
        return -1;
    }

    smiSmpsIO = new SmiSmpsIO();
    if ( sampling_ )
        smiSmpsIO->setSampling(sampling_);
    if (osiStoch_) 
        smiSmpsIO->setInfinity(osiStoch_->getInfinity()); // Set solver infinity..

    if (r != NULL)
        smiSmpsIO->setCoreCombineRule(r);

    if (smiSmpsIO->readMps(coreFname.c_str(),core_ext[i]) == -1)
    {
        delete smiSmpsIO;
        return -1;
    }

    // Store the column and row Names that belong to the given indices!!
    

    SmiCoreData *smiCore = NULL;
    const char* time_ext[] = {"tim", "time"};

    for (i = sizeof(time_ext)/sizeof(const char*) - 1; i >= 0; --i) {
        string ext(time_ext[i]);
        string fullname=timeFname+"."+ext;
        if (fileCoinReadable(fullname))
            break;
    }
    if (i == -1)
    {
        cerr << "SmiScnModel::readSmps() - No file "<< timeFname <<" with extensions .time or .tim were found." << endl;
        delete smiSmpsIO;
        return -1;
    }

    smiCore = smiSmpsIO->readTimeFile(this,timeFname.c_str(),time_ext[i]);
    if (!smiCore)
    {
        delete smiSmpsIO;
        return -1;
    }

    const char* stoch_ext[] = {"sto", "stoc","stoch"};
    for (i = sizeof(stoch_ext)/sizeof(const char*) - 1; i >= 0; --i) {
        string ext(stoch_ext[i]);
        string fullname=stochFname+"."+ext;
        if (fileCoinReadable(fullname))
            break;
    }
    if (i == -1)
    {
        cerr << "SmiScnModel::readSmps() - No file "<< stochFname <<" with extensions .stoch, .stoc, or .sto were found." << endl;
        delete smiSmpsIO;
        return -1;
    }
    if (smiSmpsIO->readStochFile(this,smiCore,stochFname.c_str(),stoch_ext[i]) != 0)
    {
        delete smiSmpsIO;
        return -1;
    }

    // Do not delte smiSmpsIO, keep it, but delete the information we do not need
    smiSmpsIO->releaseRedundantInformation();
    smiSmpsIO->releaseMatrixInformation();
    // Store smiSmpsIO in SmiScnModel!
    smpsio_ = smiSmpsIO;

    core_ = smiCore;
    return 0;
}

int SmiScnModel::writeSmps(const char *name, bool winFileExtensions, bool strictFormat) {
    if (this != NULL) {
        SmiSmpsIO * smpsIO = new SmiSmpsIO(this->getCore(), this->getSmiTree());
        if (osiStoch_)
            smpsIO->setSolverInfinity(osiStoch_->getInfinity());
        else if (core_)
            smpsIO->setSolverInfinity(core_->getInfinity() );
        smpsIO->writeSmps(name, winFileExtensions, strictFormat);
        delete smpsIO;
        return 0;
    } else {
        std::cerr << "SMPS files for " << name << " cant be written - no stochastic model.";
        return -1;
    }
}

void replaceFirstWithSecond(CoinPackedVector &dfirst, const CoinPackedVector &dsecond)
{
    double *delt1 = dfirst.getElements();
    const double *delt2 = dsecond.getElements();
    const int *indx2 = dsecond.getIndices();
    for(int j=0;j<dsecond.getNumElements();++j)
        delt1[dfirst.findIndex(indx2[j])] = delt2[j];
}

void SmiScnModel::processDiscreteDistributionIntoBlocks(SmiDiscreteDistribution *smiDD,SmiBlockStructure * smiBS){

    SmiCoreData *core=smiDD->getCore();

    int nindp = smiDD->getNumRV();
    int nstages = core->getNumStages();
    assert(nstages>0);

    // Iterate over all RV to get the number of blocks we need..
    int num_blocks = 1;
    std::map<int,std::vector<SmiDiscreteRV*> > stage_rv_map;
    for (int i = 1; i< nstages; i++)
        stage_rv_map[i] = std::vector<SmiDiscreteRV*> (0);
    for (int i = 0; i< nindp;i++){
        num_blocks *= smiDD->getDiscreteRV(i)->getNumEvents();
        assert( smiDD->getDiscreteRV(i)->getStage()>= 0 && smiDD->getDiscreteRV(i)->getStage()< nstages);
        stage_rv_map[smiDD->getDiscreteRV(i)->getStage()].push_back(smiDD->getDiscreteRV(i));

    }
    // We need to do that stagewise..
    for (int stage = 1; stage < nstages; stage++){
        // Fille the map
        // Create a new block here
        SmiBlock* smi_block = new SmiBlock(stage);

        // Fill it with zeroes
        std::vector<SmiDiscreteRV*>& rv_vector = stage_rv_map[stage]; 

        // Store the current events in a vector
        std::vector<int> current_events(rv_vector.size(),0);
        bool running = true;
        while (running){
            // Iterate over all the elements we have
            CoinPackedMatrix matrix;
            matrix.setDimensions(core->getNumRows(),core->getNumCols());
            CoinPackedVector obj;
            CoinPackedVector clo;
            CoinPackedVector cup;
            CoinPackedVector rlo;
            CoinPackedVector rup;

            int cur_variable = 0;
            double prob = 1;
            while (cur_variable < rv_vector.size()){
                // We need to get the current event
                int current_event = current_events[cur_variable];
                SmiDiscreteRV* rv = rv_vector[cur_variable];
                // Append the vector stuff to the existing ones.
                obj.append(rv->getEventObjective(current_event));
                clo.append(rv->getEventColLower(current_event));
                cup.append(rv->getEventColUpper(current_event));
                rlo.append(rv->getEventRowLower(current_event));
                rup.append(rv->getEventRowUpper(current_event));
                // Modify matrix elements
                assert(rv->getEventMatrix(current_event).getNumElements() == 0);
                const CoinPackedMatrix& matrix2(rv->getEventMatrix(current_event));
                if ( matrix2.getNumElements() ){// Only do matrix processing if we have matrix elements. 
                    int pos = 0; // position in the elements / indice arrays
                    const int * colStartIndices = matrix2.getVectorStarts(); // We work with matrices which have gaps, so we need the colStartIndices
                    for (int k = 0; k < matrix2.getNumRows(); k++) {
                        pos = colStartIndices[k];
                        for (int l = 0; l < matrix2.getVectorSize(k); l++) {
                            matrix.modifyCoefficient(k,matrix2.getIndices()[pos],matrix2.getElements()[pos],true);
                            pos++;
                        }
                    }
                    assert( pos == matrix2.getNumElements());
                }

                // Modify probability
                prob *= rv->getEventProb(current_event);
                cur_variable++;

            }
            // Create SmiBlockEvent with modified matrix and stuff
            smi_block->addBlockEvent(matrix,clo,cup,obj,rlo,rup,prob);

            // go forward with the events..
            for (int event_number = current_events.size()-1; event_number >= 0; event_number--){
                if ( event_number == 0 && current_events[event_number] == rv_vector[event_number]->getNumEvents()-1){
                    // We have to break outer while loop
                    running = false;

                }
                if ( current_events[event_number] < rv_vector[event_number]->getNumEvents()-1 ){
                    ++current_events[event_number];
                    break;
                }
                else
                    current_events[event_number] = 0;

            }
        }

        // Add smi block to SmiBlockDistribution..
        smiBS->addSmiBlock(smi_block);

    // Iterate over all 
    }


}

void
SmiScnModel::processDiscreteDistributionIntoScenarios(SmiDiscreteDistribution *smiDD, bool test)

{
    SmiCoreData *core=smiDD->getCore();

    int nindp = smiDD->getNumRV();
    int nstages = 1;

    if (test)
    {
        nstages = 3;
        assert(nindp==4);
    }
    else
    {
        nstages = core->getNumStages();
        assert(nindp > 0);
    }


    int ns=1;
    double dp=1.0;

    CoinPackedMatrix matrix ;
    CoinPackedVector cpv_dclo ;
    CoinPackedVector cpv_dcup ;
    CoinPackedVector cpv_dobj ;
    CoinPackedVector cpv_drlo ;
    CoinPackedVector cpv_drup ;

    cpv_dclo.setTestForDuplicateIndex(true);
    cpv_dcup.setTestForDuplicateIndex(true);
    cpv_dobj.setTestForDuplicateIndex(true);
    cpv_drlo.setTestForDuplicateIndex(true);
    cpv_drup.setTestForDuplicateIndex(true);

    // initialize data for first scenario
    vector<int> indx(nindp);
    vector<int> nsamp(nindp);
    vector<int> label(nstages);
    vector<int>::iterator iLabel;

    for (iLabel=label.begin(); iLabel<label.end(); ++iLabel)
        *iLabel=0;

    int jj;
    for (jj=0;jj<nindp;jj++) {

        SmiDiscreteRV *smiRV = smiDD->getDiscreteRV(jj);

        indx[jj] = 0;
        nsamp[jj] = smiRV->getNumEvents();

        assert( COIN_INT_MAX / ns > nsamp[jj] );
        ns *= nsamp[jj];

        dp *= smiRV->getEventProb(indx[jj]);

        if (test)
        {
            double p;
            p=0.5*nsamp[jj]*(nsamp[jj]+1);
            assert(smiRV->getEventProb(indx[jj])==(indx[jj]+1)/p);
        }


        cpv_dclo.append(smiRV->getEventColLower(indx[jj]));
        cpv_dcup.append(smiRV->getEventColUpper(indx[jj]));
        cpv_dobj.append(smiRV->getEventObjective(indx[jj]));
        cpv_drlo.append(smiRV->getEventRowLower(indx[jj]));
        cpv_drup.append(smiRV->getEventRowUpper(indx[jj]));

        //TODO test smiModel code
        CoinPackedMatrix m = smiRV->getEventMatrix(indx[jj]);
        if (m.getNumElements()) assert(!m.isColOrdered());

        if (matrix.getNumElements()) //Christian: What is this about?! This can not work, as matrix is not yet initialized..
        {
            for (int i=0; i<m.getNumRows(); ++i)
            {
                CoinPackedVector row=m.getVector(i);
                CoinPackedVector rrow=matrix.getVector(i);
                for (int j=m.getVectorFirst(i); j<m.getVectorLast(j); ++j)
                {
                    matrix.modifyCoefficient(i,j,row[j],true);
                }
            }
        }
        else
            matrix = m;

    }


    // first scenario
    int anc = 0;
    int branch = 1;
    int	is = 0;

    if (!test)
        is=this->generateScenario(core,&matrix,&cpv_dclo,&cpv_dcup,&cpv_dobj,
        &cpv_drlo,&cpv_drup,branch,anc,dp);
    else
    {
        assert(matrix.getNumElements()==4);
        assert(cpv_dclo.getNumElements()==4);
        for (int j=0;j<nindp;j++)
        {
            assert(cpv_dclo.getIndices()[j]==j);
            assert(cpv_drlo.getIndices()[j]==indx[j]);
            assert(matrix.getCoefficient(indx[j],j)==(double)(j*indx[j]));
        }
    }



    SmiTreeNode<SmiScnNode *> *root = this->smiTree_.getRoot();
    this->smiTree_.setChildLabels(root,label);

    /* sample space increment initialized to 1 */
    int *incr = (int *) malloc( nindp*sizeof(int) );
    for (jj=0;jj<nindp;jj++) incr[jj] = 1;

    /***** ...main loop to generate scenarios from discrete random variables
    For each scenario index ii:
    If the sample size nsamp[jj] divides the scenario index ii,
    reverse the increment direction incr[jj]
    and increase the random variable index jj by 1.
    Increment the jj'th random variable by incr[jj]
    and generate new sample data.
    ***** */
    // Unfortunately this does not work. Why? Because of the way the scenario generation method works..
    for (int iss=1;iss<ns;iss++) {
        int iii=iss; jj=0;
        while ( !(iii%nsamp[jj]) ) {
            iii /= nsamp[jj];
            incr[jj] = -incr[jj];
            jj++;
        }

        SmiDiscreteRV *smiRV = smiDD->getDiscreteRV(jj);

        dp /= smiRV->getEventProb(indx[jj]);
        indx[jj] += incr[jj];
        dp *= smiRV->getEventProb(indx[jj]);

        if (test)
        {
            double p;
            p=0.5*nsamp[jj]*(nsamp[jj]+1);
            assert(smiRV->getEventProb(indx[jj])==(indx[jj]+1)/p);
        }

        for (iLabel=label.begin(); iLabel<label.end(); ++iLabel)
            *iLabel=0;

        for(int jjj=0; jjj<nindp; jjj++)
        {
            SmiDiscreteRV *s = smiDD->getDiscreteRV(jjj);

            label[s->getStage()] *= s->getNumEvents();
            label[s->getStage()] += indx[jjj];
        }


        // set data
        //TODO -- should we declare NULL entries to have 0 entries?
        //this would eliminate these tests
        replaceFirstWithSecond(cpv_dclo,smiRV->getEventColLower(indx[jj]));
        replaceFirstWithSecond(cpv_dcup,smiRV->getEventColUpper(indx[jj]));
        replaceFirstWithSecond(cpv_dobj,smiRV->getEventObjective(indx[jj]));
        replaceFirstWithSecond(cpv_drlo,smiRV->getEventRowLower(indx[jj]));
        replaceFirstWithSecond(cpv_drup,smiRV->getEventRowUpper(indx[jj]));

        //TODO test this code
        CoinPackedMatrix m = smiRV->getEventMatrix(indx[jj]);
        if (m.getNumElements()) assert(!m.isColOrdered());
        if (matrix.getNumElements())
        {
            for (int i=0; i<m.getNumRows(); ++i)
            {
                CoinPackedVector row=m.getVector(i);
                CoinPackedVector rrow=matrix.getVector(i);
                for (int j=m.getVectorFirst(i); j<m.getVectorLast(j); ++j)
                {
                    matrix.modifyCoefficient(i,j,row[j],true);
                }
            }
        }
        else
            matrix = m;

        // find ancestor node
        SmiTreeNode<SmiScnNode *> *tnode = this->smiTree_.find(label);
        anc = tnode->scenario();
        branch = tnode->depth();
        if (!test)
        {
            is = this->generateScenario(core,&matrix,&cpv_dclo,&cpv_dcup,&cpv_dobj,&cpv_drlo,&cpv_drup,branch,anc,dp);
        }
        else
        {
            assert(matrix.getNumElements()==4);
            assert(cpv_dclo.getNumElements()==4);
            for (int j=0;j<nindp;j++)
            {
                assert(cpv_dclo.getIndices()[j]==j);
                assert(cpv_drlo.getIndices()[j]==indx[j]);
                assert(matrix.getCoefficient(j,indx[j])==(double)(j*indx[j]));
            }
        }

        this->smiTree_.setChildLabels(tnode,label);

    }

    free (incr);
}

double SmiScnModel::getObjectiveValue(SmiScenarioIndex ns)
{
    const double *dsoln = this->getOsiSolverInterface()->getColSolution();
    const double *dobj  = this->getOsiSolverInterface()->getObjCoefficients();

    /* calculate the scenario objective value */
    double scenSum = 0.0;

    // start with leaf node
    SmiScnNode *node = this->getLeafNode(ns);

    while (node != NULL)
    {
        double nodeSum = 0.0;
        double nodeProb = node->getModelProb();

        assert(nodeProb>0);

        // getColStart returns the starting index of node in OSI model
        for(int j=node->getColStart(); j<node->getColStart()+node->getNumCols(); ++j)
        {
            nodeSum += dobj[j]*dsoln[j];
        }
        nodeSum /= nodeProb;
        scenSum += nodeSum;

        // get parent of node
        node = node->getParent();
    }

    return scenSum;
}

int
SmiScnModel::addNodeToSubmodel(SmiScnNode * smiScnNode)
{
    // stage
    int stg=smiScnNode->getStage();

    // vector of nodes
    SmiScnNode ** vecNode = (SmiScnNode **)malloc((stg+1)*sizeof(SmiScnNode*));

    // vector of labels -- the node's scenario index
    int * label = (int *) malloc((stg+1)*sizeof(int));

    // temporaries
    SmiScnNode *s=smiScnNode;

    // place incoming label into vectors offset by stage location
    vecNode[stg] = smiScnNode;
    label[stg]   = smiScnNode->getScenarioIndex();

    // fill in vector up to root
    int t=stg;
    while (s = s->getParent() )
    {
        vecNode[--t] = s;
        label[t]   = s->getScenarioIndex();
    }

    //assert(t==-1);

    // Search the tree for the labels
    SmiTreeNode<SmiScnNode *> *tnode = this->smiTree_.find(label,stg+1);

    // Set starting stage for new vector of SmiScnNodes
    int start_stg=0;
    if (tnode)
        start_stg=tnode->getDataPtr()->getStage()+1;

    if (start_stg<stg+1)
    {
        vector<SmiScnNode *> nwVecNode;
        SmiScnNode *nwNode;
        for (t=start_stg; t<stg+1; t++)
        {
            nwNode = new SmiScnNode(vecNode[t]->getNode());
            nwNode->setProb(vecNode[t]->getProb());
            nwVecNode.push_back(nwNode);
            nwNode->setIncludeOff();
            this->ncol_ += nwNode->getNumCols();
            this->nrow_ += nwNode->getNumRows();
            if (t)
                this->nels_ += nwNode->getNode()->getNumMatrixElements() + nwNode->getNode()->getCore()->getNode(t)->getNumMatrixElements();
            else
                this->nels_ += smiScnNode->getNode()->getNumMatrixElements();


        }
        this->smiTree_.addNodesToTree(tnode,0,nwVecNode,0);

        // Last node in path is the copy of the incoming node -- set Include on.
        nwNode->setIncludeOn();
    }

    free(vecNode);
    free(label);

    return (stg+1 - start_stg);
}

 void SmiScnModel::processBlockStructureIntoScenarios(SmiBlockStructure* smiBS){
	SmiScenarioIndex is;
	SmiCoreData *core = smiBS->getCore();
	SmiCoreCombineRule *r = smiBS->getCombineWithCoreRule();
    // Test if block structure is empty!
    if ( smiBS->getBlock(0,1) == NULL)
        return;
    // We now generate scenarios out of the block events. If we have more than one block for a stage, we have to combine them first.
    // Combine blocks stagewise.
    for (int stage = 1; stage < core->getNumStages(); stage++){
        // Combine blocks (if necessary)
        while ( smiBS->getNumBlocks(stage) > 1 ){
            // Create a new block and declare all needed vectors and matrices
            SmiBlock* combBlock = new SmiBlock(stage);
            CoinPackedVector combdobj;
            CoinPackedVector combdclo;
            CoinPackedVector combdcup;
            CoinPackedVector combdrlo;
            CoinPackedVector combdrup;
            CoinPackedMatrix combmatrix;
            //combmatrix.setDimensions(core->getNumRows(),core->getNumCols()); Not Necessary, we get the correct blockMatrix from the Events

            SmiBlock* block1 = smiBS->getBlock(smiBS->getNumBlocks(stage)-1,stage);
            SmiBlock* block2 = smiBS->getBlock(smiBS->getNumBlocks(stage)-2,stage);
            double combprob;

            for(size_t i=0;i<block1->getNumEvents();i++){
                for(size_t j=0;j<block2->getNumEvents();j++){
                    combdobj = block1->getBlockEventObjective(i);
                    combdclo = block1->getBlockEventColLower(i);
                    combdcup = block1->getBlockEventColUpper(i);
                    combdrlo = block1->getBlockEventRowLower(i);
                    combdrup = block1->getBlockEventRowUpper(i);
                    combmatrix = block1->getBlockEventMatrix(i);
                    const CoinPackedVector& dobj2(block2->getBlockEventObjective(j));
                    const CoinPackedVector& dclo2(block2->getBlockEventColLower(j));
                    const CoinPackedVector& dcup2(block2->getBlockEventColUpper(j));
                    const CoinPackedVector& drlo2(block2->getBlockEventRowLower(j));
                    const CoinPackedVector& drup2(block2->getBlockEventRowUpper(j));
                    const CoinPackedMatrix& matrix2(block2->getBlockEventMatrix(j));
                    for(int k=0;k<dobj2.getNumElements();k++){
                        combdobj.insert(dobj2.getIndices()[k],dobj2.getElements()[k]);
                    }
                    for(int k=0;k<dclo2.getNumElements();k++){
                        combdclo.insert(dclo2.getIndices()[k],dclo2.getElements()[k]);
                    }
                    for(int k=0;k<dcup2.getNumElements();k++){
                        combdcup.insert(dcup2.getIndices()[k],dcup2.getElements()[k]);
                    }
                    for(int k=0;k<drlo2.getNumElements();k++){
                        combdrlo.insert(drlo2.getIndices()[k],drlo2.getElements()[k]);
                    }
                    for(int k=0;k<drup2.getNumElements();k++){
                        combdrup.insert(drup2.getIndices()[k],drup2.getElements()[k]);
                    }
                    int pos = 0; // position in the elements / indice arrays
                    const int * colStartIndices = matrix2.getVectorStarts(); // We work with matrices which have gaps, so we need the colStartIndices
                    for (int k = 0; k < matrix2.getNumRows(); k++) {
                        pos = colStartIndices[k];
                        for (int l = 0; l < matrix2.getVectorSize(k); l++) {
                            combmatrix.modifyCoefficient(k,matrix2.getIndices()[pos],matrix2.getElements()[pos],true);
                            pos++;
                        }
                    }
                    combprob = block1->getBlockEventProb(i)*block2->getBlockEventProb(j);
                    combBlock->addBlockEvent(combmatrix,combdclo,combdcup,combdobj,combdrlo,combdrup,combprob);
                }// End For block2 events
            }// End For block1 events
            // Delete the two last blocks and add the newly created block
            smiBS->deleteSmiBlock(stage);
            smiBS->deleteSmiBlock(stage);
            smiBS->addSmiBlock(combBlock);
        }// End While
    }
    // Now we know the number of block events per stage. We need to generate the scenarios. Important: Ancestor scenario.
    // We have to compute the ancestor scenario first
    std::vector<std::pair<int,int> > num_childs_per_stage;
    int num_stages = core->getNumStages();
    num_childs_per_stage.push_back(make_pair(1,1));
    for ( int i = 1; i < num_stages; i++){
        num_childs_per_stage.push_back( make_pair(smiBS->getBlock(0,i)->getNumEvents(),1));
    }
    // Set last one correct
    num_childs_per_stage[num_stages-1].second = num_childs_per_stage[num_stages-1].first;
    for ( int i = num_stages-2; i > 0; i--)
        num_childs_per_stage[i].second = num_childs_per_stage[i+1].second * num_childs_per_stage[i].first;

    


    // What we do to avoid huge memory increase: Iterate over all events in the blocks, add stuff up, and generate scenario once we reach the last stage. Then start again.
    // We have to count the current event in a std::vector
    std::vector<int> current_event_by_stage(num_stages,0);
    int current_stage = 1;
    int current_event = 0;
    int cur_scen = 0;
    double combprob = 1;

    // We store the values in these objects. Prepare these at the first time, afterwards replace only at will..
    CoinPackedVector combdobj = smiBS->getBlock(0,1)->getBlockEventObjective(current_event);
    CoinPackedVector combdclo = smiBS->getBlock(0,1)->getBlockEventColLower(current_event);
    CoinPackedVector combdcup = smiBS->getBlock(0,1)->getBlockEventColUpper(current_event);
    CoinPackedVector combdrlo = smiBS->getBlock(0,1)->getBlockEventRowLower(current_event);
    CoinPackedVector combdrup = smiBS->getBlock(0,1)->getBlockEventRowUpper(current_event);
    CoinPackedMatrix combmatrix = smiBS->getBlock(0,1)->getBlockEventMatrix(current_event); // Set matrix size
    combmatrix.setDimensions(core->getNumRows(),core->getNumCols());

    // Outer loop
    while ( current_event_by_stage[0] == 0){

        current_event = current_event_by_stage[current_stage];
        // Calculate probability fresh from the start up to the current_stage
        combprob = num_stages>2 ? 1 : smiBS->getBlock(0,1)->getBlockEventProb(current_event);
        for (int i = 1; i < current_stage;i++){
            combprob *= smiBS->getBlock(0,i)->getBlockEventProb(current_event_by_stage[i]);
        }
        // Prepare common vectors by replacing the given indices (TODO), up to the current_stage (and including)
        combdobj = smiBS->getBlock(0,1)->getBlockEventObjective(current_event);
        combdclo = smiBS->getBlock(0,1)->getBlockEventColLower(current_event);
        combdcup = smiBS->getBlock(0,1)->getBlockEventColUpper(current_event);
        combdrlo = smiBS->getBlock(0,1)->getBlockEventRowLower(current_event);
        combdrup = smiBS->getBlock(0,1)->getBlockEventRowUpper(current_event);
        combmatrix = smiBS->getBlock(0,1)->getBlockEventMatrix(current_event);
        for ( int i = 2; i < current_stage; i++){ // Do this for all the following indices (TODO)
            combdobj.append(smiBS->getBlock(0,i)->getBlockEventObjective(current_event));
            combdclo.append(smiBS->getBlock(0,i)->getBlockEventColLower(current_event));
            combdcup.append(smiBS->getBlock(0,i)->getBlockEventColUpper(current_event));
            combdrlo.append(smiBS->getBlock(0,i)->getBlockEventRowLower(current_event));
            combdrup.append(smiBS->getBlock(0,i)->getBlockEventRowUpper(current_event));
            const CoinPackedMatrix& matrix2(smiBS->getBlock(0,i)->getBlockEventMatrix(current_event));
            if ( matrix2.getNumElements() ){// Only do matrix processing if we have matrix elements. 
                int pos = 0; // position in the elements / indice arrays
                const int * colStartIndices = matrix2.getVectorStarts(); // We work with matrices which have gaps, so we need the colStartIndices
                for (int k = 0; k < matrix2.getNumRows(); k++) {
                    pos = colStartIndices[k];
                    for (int l = 0; l < matrix2.getVectorSize(k); l++) {
                        combmatrix.modifyCoefficient(k,matrix2.getIndices()[pos],matrix2.getElements()[pos],true);
                        pos++;
                    }
                }

            }
        }
    
        while( current_stage < core->getNumStages() && num_stages > 2 ){ // Inner loop: Count from the starting stage until the last stage. For a Two-Stage problem this is not necessary.

            SmiBlock* current_block = smiBS->getBlock(0,current_stage);
            // Get event from current stage
            current_event = current_event_by_stage[current_stage];

            const CoinPackedMatrix& matrix2(current_block->getBlockEventMatrix(current_event));
            if ( current_stage != 1) { // If we are in the first stage, we do not need to append something to something..
                combdobj.append(current_block->getBlockEventObjective(current_event));
                combdclo.append(current_block->getBlockEventColLower(current_event));
                combdcup.append(current_block->getBlockEventColUpper(current_event));
                combdrlo.append(current_block->getBlockEventRowLower(current_event));
                combdrup.append(current_block->getBlockEventRowUpper(current_event));

                if ( matrix2.getNumElements() ){// Only do matrix processing if we have matrix elements. 
                    // This matrix processing does not work if matrix has GAPs. Try to workaround that issue..
                    int pos = 0; // position in the elements / indice arrays
                    int number_of_modifications = 0;
                    const int * colStartIndices = matrix2.getVectorStarts(); // We work with matrices which have gaps, so we need the colStartIndices
                    for (int k = 0; k < matrix2.getNumRows(); k++) {
                        pos = colStartIndices[k];
                        for (int l = 0; l < matrix2.getVectorSize(k); l++) {
                            combmatrix.modifyCoefficient(k,matrix2.getIndices()[pos],matrix2.getElements()[pos],true);
                            pos++; number_of_modifications++;
                        }
                    }
                    assert( number_of_modifications == matrix2.getNumElements());
                }
            }
            combprob *= current_block->getBlockEventProb(current_event);
            current_stage++;
        }
        // We reached the last stage: Generate scenario

        int branch = num_stages-1;
        int anc = 0;
        for ( int j = num_stages-1; j>0; j--){
            if ( cur_scen % num_childs_per_stage[j].second == 0 ){
                branch--;
            }
            // What is our ancestor scenario? We set it according to the following formula
            if ( anc < num_childs_per_stage[j].second && cur_scen / num_childs_per_stage[j].second && (cur_scen % num_childs_per_stage[j].second) != 0 ) // If we are greater than number of childs and we are not modulo 0..
                anc = num_childs_per_stage[j].second*(cur_scen/num_childs_per_stage[j].second);
        }
        if ( branch == 0) // Special case we need for the first stage only
            branch++;
        is = this->generateScenario(core,&combmatrix,&combdclo,&combdcup,&combdobj,&combdrlo,&combdrup,branch,anc,combprob,r);
        // count scenario number one up
        assert(is == cur_scen);
        cur_scen++;
        // Set the setting for the next scenario generation iteration. We also have to reset the probabilities
        for ( int count = num_stages-1; count > -1; count--){
            if ( count == 0){
                ++current_event_by_stage[count];
            }
            else if ( current_event_by_stage[count] < smiBS->getBlock(0,count)->getNumEvents()-1 ){
                ++current_event_by_stage[count];
                if (num_stages>2)// Only in the multi stage case, otherwise we do not need this..
                    --current_stage;
                break; // Leave for loop
            }
            else{
                current_event_by_stage[count] = 0;
                current_stage--;
            }

        }
    }
}



        
	
 

 double SmiScnModel::getScenarioProb( SmiScenarioIndex ns )
 {
     return smiTree_.getLeaf(ns)->getDataPtr()->getProb();
 }

 void SmiScnModel::transformToStaircaseStructure( std::map<int,int>& coefficients )
 {
    core_->transformToStaircaseStructure(coefficients, smiTree_.getRoot());
    for (int i = 0; i < core_->getNumStages();i++){
        this->maxNelsPerScenInStage[i] = core_->getNode(i)->getNumMatrixElements()+
            core_->getNode(i)->getColLowerLength() +
            core_->getNode(i)->getColUpperLength() +
            core_->getNode(i)->getObjectiveLength() +
            core_->getNode(i)->getRowLowerLength() +
            core_->getNode(i)->getRowUpperLength();
    }

 }

 void SmiScnModel::moveBoundsIntoMatrix(){
     this->core_->moveBoundsIntoMatrix(smiTree_.wholeTree());
     // Update max numNels
     for (int i = 0; i < core_->getNumStages();i++){
         this->maxNelsPerScenInStage[i] = core_->getNode(i)->getNumMatrixElements()+
             core_->getNode(i)->getColLowerLength() +
             core_->getNode(i)->getColUpperLength() +
             core_->getNode(i)->getObjectiveLength() +
             core_->getNode(i)->getRowLowerLength() +
             core_->getNode(i)->getRowUpperLength();
     }
 }

 void SmiScnModel::aggregateStages( int firstStage, int secondStage, bool aggregateFromStart )
 {
     // Call the method 
     int lesser_stage = std::min(firstStage,secondStage);
     int upper_stage = std::max(firstStage,secondStage);
     // Let's see if we can actually aggregate the stages we specified?!
     if ( upper_stage > core_->getNumStages()-1 )
         return; // We do not aggreagtes anything, if asked the wrong way
     if ( lesser_stage < 0 )
         return;
     if ( lesser_stage-upper_stage == 0 )
         return; // We do nothing
     if ( upper_stage-lesser_stage > 1) { // We need to apply the method several times, so we need to know from which direction we want to start!
         int number_of_aggregations = upper_stage-lesser_stage;
         if ( aggregateFromStart ) // With each aggregation, our final number of stages collapses by one.
             for (int i = 0; i < number_of_aggregations; i++)
                 core_->aggregateStages(lesser_stage,lesser_stage+1, smiTree_);

         else
             for (int i = upper_stage; i > lesser_stage; i--)
                 core_->aggregateStages( i-1, i, smiTree_);

     }
     else
         core_->aggregateStages(lesser_stage,upper_stage,smiTree_);

     // We need to reset the number of ncol/nrow and elements for the det.eq.
     this->ncol_ = 0;
     this->nrow_ = 0;
     this->nels_ = 0;

     std::vector<SmiScnNode*>& tree( smiTree_.wholeTree());
     for (int i = 0; i < tree.size(); i++){
         this->ncol_ += core_->getNumCols(tree[i]->getStage());
         this->nrow_ += core_->getNumRows(tree[i]->getStage());
         this->nels_ += core_->getNode(tree[i]->getStage())->getNumMatrixElements();
     }
 }

 void SmiScnModel::sampleScenarioTree( std::vector<int> number_samples, int sampling_method, double* (*random_generator)(int ) )
 {
    // We have a discrete distribution
    // We need a random_number generator
    
     // We need to reset totalProb
     this->totalProb_ = 0.0;

     SmiDiscreteDistribution* smiDD = discrete_distribution_;
     assert(smiDD);
     SmiCoreData *core=smiDD->getCore();
     int nstages = core->getNumStages();
     assert(nstages>0);

     //SmiBlockStructure* smiBS = new SmiBlockStructure(core);
     // Delete old scenario tree. Should be done by copy constructor!
     smiTree_ = SmiScenarioTree<SmiScnNode *>();

     int number_of_independent_vars = smiDD->getNumRV(); 
     assert( number_of_independent_vars > 0 ); // Otherwise we can not really sample, can we?
     std::vector<int> number_of_random_vars_per_stage(nstages);
 

     // Iterate over all RV to get the number of blocks we need..
     std::map<int,std::vector<SmiDiscreteRV*> > stage_rv_map;
     for (int i = 1; i< nstages; i++)
         stage_rv_map[i] = std::vector<SmiDiscreteRV*> (0);
     for (int i = 0; i< number_of_independent_vars;i++){
         assert( smiDD->getDiscreteRV(i)->getStage()>= 0 && smiDD->getDiscreteRV(i)->getStage()< nstages);
         stage_rv_map[smiDD->getDiscreteRV(i)->getStage()].push_back(smiDD->getDiscreteRV(i));
     }
     // Sample the random numbers from our function
     
     std::map<int,std::vector< std::vector<double> > > random_numbers_per_stage;
     int number_of_samples = 1;
     for (int cur_stage = 1; cur_stage < nstages; cur_stage++){
         number_of_random_vars_per_stage[cur_stage] = stage_rv_map[cur_stage].size();
         std::vector<std::vector<double> > random_numbers(number_of_independent_vars);
         number_of_samples *= number_samples[cur_stage];
         for (int i = 0; i < number_of_random_vars_per_stage[cur_stage]; i++){
             random_numbers[i].resize(number_of_samples);
             for (int current_batch = 0; current_batch < number_of_samples/number_samples[cur_stage]; current_batch += number_samples[cur_stage]){
                 // random_numbers[i].push_back(0.0); // Allows the use of memcpy later on
                 // Get the random numbers via the function // TODO: Here we can use Latin Hypercube Sampling, Antithetic Sampling or Crude Monte Carlo Sampling.
                 // Caller of this function decides that!
                 double * generated_random_numbers = random_generator(number_samples[cur_stage]);
                 assert(generated_random_numbers != NULL); // Not NULL
                 memcpy(&random_numbers[i][current_batch],generated_random_numbers,sizeof(double)*number_samples[cur_stage]);
                 delete [] generated_random_numbers;
             }

         }
         random_numbers_per_stage[cur_stage] = random_numbers;
     }
        
     // Now we have a sample of [0,1] numbers for each random independent variable
     // Get the realization via inverse transformation!
     // Store the SmiDiscreteEV in the random_events_per_stage vector
     std::map<int,std::vector< std::vector<SmiDiscreteEvent*> > > random_events_per_stage;
     for (int cur_stage = 1; cur_stage < nstages; cur_stage++){
         std::vector<std::vector<double> > &random_numbers(random_numbers_per_stage[cur_stage]);
         std::vector< std::vector<SmiDiscreteEvent*> > random_events(number_of_random_vars_per_stage[cur_stage]);
            
         for (int i = 0; i < number_of_random_vars_per_stage[cur_stage]; i++){
             random_events[i].reserve(random_numbers[i].size());
             for (int j = 0; j < random_numbers[i].size(); j++){
                 random_events[i].push_back(stage_rv_map[cur_stage][i]->inverseTransformation(random_numbers[i][j]));
             }
         }
         // Now we have all the information for this stage
         random_events_per_stage[cur_stage] = random_events;
     }

     // Now we can start and build the blocks! 

     // Assume we have independent stages!
     // We need to do that stagewise. 
     // Build first block for second stage. Then build a third stage block. Then build a forth stage block. etc. Then build all last stage block. Backtrack one stage. Repeat until stage completed. Backtrack one stage. repeat until backtracked to first stage.     

     // Create a new block here
     int cur_stage = 1;
     int current_sample = 0;
     int branch = 1;
     int anc = 0;
     double prob = 0;
     int cur_scen = 0;
     // We need counter for every stage!
     std::vector<int> current_sample_per_stage(nstages,0);
     std::vector<int> current_sample_per_stage_modulo_size(nstages,0);

     SmiBlock* smi_block = new SmiBlock(cur_stage);
     while ( cur_stage >= 1 ){

         current_sample = current_sample_per_stage[cur_stage];
         CoinPackedMatrix matrix;
         matrix.setDimensions(core->getNumRows(),core->getNumCols());
         CoinPackedVector obj;
         CoinPackedVector clo;
         CoinPackedVector cup;
         CoinPackedVector rlo;
         CoinPackedVector rup;
         // Combine all random variables at this stage
         for (int current_var = 0; current_var < number_of_random_vars_per_stage[cur_stage]; current_var++){
             // Append the vector stuff to the existing ones.
             obj.append(random_events_per_stage[cur_stage][current_var][current_sample]->getObjective());
             clo.append(random_events_per_stage[cur_stage][current_var][current_sample]->getColLower());
             cup.append(random_events_per_stage[cur_stage][current_var][current_sample]->getColUpper());
             rlo.append(random_events_per_stage[cur_stage][current_var][current_sample]->getRowLower());
             rup.append(random_events_per_stage[cur_stage][current_var][current_sample]->getRowUpper());
             const CoinPackedMatrix& matrix2(random_events_per_stage[cur_stage][current_var][current_sample]->getMatrix());
             if ( matrix2.getNumElements() ){// Only do matrix processing if we have matrix elements. 
                 int pos = 0; // position in the elements / indice arrays
                 const int * colStartIndices = matrix2.getVectorStarts(); // We work with matrices which have gaps, so we need the colStartIndices
                 for (int k = 0; k < matrix2.getNumRows(); k++) {
                     pos = colStartIndices[k];
                     for (int l = 0; l < matrix2.getVectorSize(k); l++) {
                         matrix.modifyCoefficient(k,matrix2.getIndices()[pos],matrix2.getElements()[pos],true);
                         pos++;
                     }
                 }
                 assert( pos == matrix2.getNumElements());
             }
             
         }
         prob = 1.0 / number_samples[cur_stage];
         // Create SmiBlockEvent with modified matrix and stuff
         cur_scen = generateScenario(core,&matrix,&clo,&cup,&obj,&rlo,&rup,branch,anc,prob);
         current_sample_per_stage[cur_stage]++;
         current_sample_per_stage_modulo_size[cur_stage]++;

         // Go to next stage, if possible
         if ( cur_stage < nstages -1 ){
             cur_stage++;
             branch++;
             anc = cur_scen;
         } // If we added all samples for this stage, we can go back a stage, else we don't
         else if ( current_sample_per_stage_modulo_size[cur_stage] >= number_samples[cur_stage]  ){
             cur_stage--;
             current_sample_per_stage_modulo_size[cur_stage] = 0;
             branch--;
             if ( cur_stage == 1)
                 anc = 0;
         }
         else{
             // We stay at this stage!
         }
     }
}

void SmiScnModel::setDiscreteDistribution( SmiDiscreteDistribution* smiDD )
{
    delete discrete_distribution_;
    discrete_distribution_ = smiDD;
}
const SmiDiscreteDistribution* SmiScnModel::getDiscreteDistribution(  )
{
    return discrete_distribution_;

}

void SmiScnModel::setSampling( bool do_sample)
{
    sampling_ = do_sample;
}

void SmiScnModel::CopyStochasticDataToDetEQModel( int size,const double* source_array,double* destination_array,int offset,const CoinPackedVector& cpv,SmiCoreCombineRule * combineRule,double prob )
{
    // We copy Stochastic Data to DetEq Model. Therefore we need the core data, present in source_array, multiply it with the probability and copy the cpv elements (stochastic node elements), multiplied with the probability to it. Finally we add this to the destination source_array. 
    // For size we need to copy the stuff that is present in source_array!
    double* temp = new double[size];
    int* indices = new int[size];
    memcpy(temp,source_array,sizeof(double)*size);
    combineRule->Process(temp,offset,cpv);
    for (int i = 0; i < size; i++){
        if ( temp[i] > -this->getCore()->getInfinity() && temp[i] < this->getCore()->getInfinity())
            temp[i] *= prob;
        indices[i] = offset+i;
    }
    // Update the values with the cpv, this usually means replace (if add is used somewhere we need a good combineRule for it!
    for (int i = 0; i < cpv.getNumElements();i++){
        if ( cpv.getElements()[i] > -this->getCore()->getInfinity() && cpv.getElements()[i] < this->getCore()->getInfinity())
            temp[cpv.getIndices()[i]-offset] = cpv.getElements()[i]*prob;
        else
            assert(false); // This means we have a stochastic value of infinity. This is most likely not correct modeling. => Result: Expected value is not finite..
    }


    // Put temp into a CoinPackedVector
    CoinPackedVector trlo(size,indices,temp,false);
    combineRule->Process(destination_array, 0, trlo);
    delete [] temp;
    delete [] indices;
}

const char* SmiScnModel::getColumnName( int index )
{
    if ( smpsio_ )
        return smpsio_->columnName(index);
    return "";
}
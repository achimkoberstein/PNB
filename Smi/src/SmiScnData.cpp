#include "SmiScnData.hpp"
#include "SmiScnModel.hpp"
#include "CoinHelperFunctions.hpp"
#include "OsiSolverInterface.hpp"
#include "CoinPackedMatrix.hpp"
#include <iostream>
#include <vector>
#include <deque>

using namespace std;


int nrow_;
int ncol_;
int nz_; // We can count the total number of elements, but I do not if we need this at one point
SmiStageIndex nstag_; //Total number of stages in the problem, apart from the first stage. A two-stage problem has nstag_ = 1 and so on. If the problem is deterministic, nstag_ = 0.
int *nColInStage_; //Number of Columns in Stage
int *nRowInStage_; //Number of Rows in Stage
int *stageColPtr_; //Start Index of Columns in Stage, with respect to ncol_
int *stageRowPtr_; //Start Index of Rows in Stage, with respect to nrow_
int *colStage_; //To which stage belongs the column?
int *rowStage_; //To which stage belongs the row?
int *colEx2In_; //Not clear yet
int *rowEx2In_; //Not clear yet
int *colIn2Ex_; // Not clear yet
int *rowIn2Ex_; // Not clear yet
int *integerIndices_; //indices of integer variables
int integerLength_; //number of integer variables
int *binaryIndices_; //indices of binary variables
int binaryLength_; //number of binary variables
double **cdrlo_;
double **cdrup_;
double **cdobj_;
double **cdclo_;
double **cdcup_;
vector<SmiNodeData*> nodes_; //Nodes, that contain stage dependent constraints (with Bounds,Ranges,Objective,Matrix), so called CoreNodes 
vector<double *> pDenseRow_; //dense probability vector
vector< vector<int> > intColsStagewise; // For each stage separately, it contains the position of every integer column

SmiCoreData::SmiCoreData(OsiSolverInterface *osi,int nstag,int *cstag,int *rstag, int* integerIndices,int integerLength, int* binaryIndices, int binaryLength):
nrow_(0),ncol_(0),nz_(0),nstag_(nstag),nColInStage_(NULL),nRowInStage_(NULL),stageColPtr_(NULL),colStage_(NULL),rowStage_(NULL),colEx2In_(NULL),rowEx2In_(NULL),
colIn2Ex_(NULL),rowIn2Ex_(NULL),integerIndices_(NULL),integerLength_(0),binaryIndices_(NULL),binaryLength_(0),
cdrlo_(NULL),cdrup_(NULL),cdobj_(NULL),cdclo_(NULL),cdcup_(NULL),nodes_(),pDenseRow_(),intColsStagewise(nstag,std::vector<int>()),colNamesFree(NULL),colNamesStrict(NULL)
{	//Copies all values already stored in the Solver..
    int nrow = osi->getNumRows();
    int ncol = osi->getNumCols();
    CoinPackedVector *drlo = new CoinPackedVector(nrow,osi->getRowLower());
    CoinPackedVector *drup = new CoinPackedVector(nrow,osi->getRowUpper());
    CoinPackedVector *dclo = new CoinPackedVector(ncol,osi->getColLower());
    CoinPackedVector *dcup = new CoinPackedVector(ncol,osi->getColUpper());
    CoinPackedVector *dobj = new CoinPackedVector(ncol,osi->getObjCoefficients());

    CoinPackedMatrix *matrix = new CoinPackedMatrix(*osi->getMatrixByRow());
    matrix->eliminateDuplicates(0.0);

    gutsOfConstructor(nrow,ncol,nstag,cstag,rstag,matrix,dclo,dcup,dobj,drlo,drup,integerIndices,integerLength,binaryIndices,binaryLength);
    infinity_ = osi->getInfinity();

    delete matrix;
    delete drlo;
    delete drup;
    delete dclo;
    delete dcup;
    delete dobj;
}

SmiCoreData::SmiCoreData(CoinMpsIO *osi,int nstag,int *cstag,int *rstag,int* integerIndices,int integerLength, int* binaryIndices, int binaryLength):
nrow_(0),ncol_(0),nz_(0),nstag_(nstag),nColInStage_(NULL),nRowInStage_(NULL),stageColPtr_(NULL),colStage_(NULL),rowStage_(NULL),colEx2In_(NULL),rowEx2In_(NULL),
colIn2Ex_(NULL),rowIn2Ex_(NULL),integerIndices_(NULL),integerLength_(0),binaryIndices_(NULL),binaryLength_(0),
cdrlo_(NULL),cdrup_(NULL),cdobj_(NULL),cdclo_(NULL),cdcup_(NULL),nodes_(),pDenseRow_(),intColsStagewise(nstag,std::vector<int>()),colNamesFree(NULL),colNamesStrict(NULL)
{
    int nrow = osi->getNumRows();
    int ncol = osi->getNumCols();
    CoinPackedVector *drlo = new CoinPackedVector(nrow,osi->getRowLower());
    CoinPackedVector *drup = new CoinPackedVector(nrow,osi->getRowUpper());
    CoinPackedVector *dclo = new CoinPackedVector(ncol,osi->getColLower());
    CoinPackedVector *dcup = new CoinPackedVector(ncol,osi->getColUpper());
    CoinPackedVector *dobj = new CoinPackedVector(ncol,osi->getObjCoefficients());

    CoinPackedMatrix *matrix = new CoinPackedMatrix(*osi->getMatrixByRow());
    matrix->eliminateDuplicates(0.0);
    
    gutsOfConstructor(nrow,ncol,nstag,cstag,rstag,matrix,dclo,dcup,dobj,drlo,drup,integerIndices,integerLength,binaryIndices,binaryLength);
    infinity_ = osi->getInfinity();

    delete matrix;
    delete drlo;
    delete drup;
    delete dclo;
    delete dcup;
    delete dobj;
}

OsiSolverInterface* SmiCoreData::generateCoreProblem(OsiSolverInterface* osi){
    osi->reset();
    //Set Matrix

    //Set column, row, objective function

    return osi;
}

void SmiCoreData::gutsOfConstructor( int nrow,int ncol,int nstag, int *cstag,int *rstag, CoinPackedMatrix *matrix, CoinPackedVector *dclo, CoinPackedVector *dcup, CoinPackedVector *dobj, CoinPackedVector *drlo, CoinPackedVector *drup, int* integerIndices /*= 0*/,int integerLength /*= 0*/,int* binaryIndices /*= 0*/,int binaryLength /*= 0*/ )
{
	int i;
	nrow_ = nrow;
	ncol_ = ncol;
    if ( integerLength ){
        integerIndices_ = new int[integerLength];
        integerLength_ = integerLength;
        memcpy(integerIndices_,integerIndices,sizeof(int)*integerLength);
    }
    if ( binaryLength ) {
        binaryIndices_ = new int[binaryLength];
        binaryLength_ = binaryLength;
        memcpy(binaryIndices_,binaryIndices,sizeof(int)*binaryLength);
    }



	// store number stages
	nstag_ = nstag;

	nColInStage_ = new int[nstag_+1];
	nRowInStage_ = new int[nstag_+1];


	colStage_ = new int[ncol_];
	colEx2In_ = new int[ncol_];
	colIn2Ex_ = new int[ncol_];

	rowStage_ = new int[nrow_];
	rowEx2In_ = new int[nrow_];
	rowIn2Ex_ = new int[nrow_];

	// store stage maps
	CoinDisjointCopyN(cstag,ncol_,colStage_);
	CoinDisjointCopyN(rstag,nrow_,rowStage_);

	// zero stage counts
	for (i=0;i<nstag_+1;i++)
	{
		nColInStage_[i] = 0;
		nRowInStage_[i] = 0;
	}

	// array to point to start of new stage
	stageRowPtr_ = new int[nstag_+1];

	// count rows in each stage
	for (i=0;i<nrow_;i++)
		nRowInStage_[rowStage_[i]]++;

	// set stage pointers
	stageRowPtr_[0] = 0;
	for (i=0;i<nstag_;i++)
		stageRowPtr_[i+1] = stageRowPtr_[i] + nRowInStage_[i];

	// place index into next open position in its stage
	for (i=0;i<nrow_;i++)
	{
		rowEx2In_[i] = stageRowPtr_[rowStage_[i]]; //The result of RowEx2In for Row i is the stageRowPointer for the given stage (but because this pointer is increased, nothing happens..)
		rowIn2Ex_[rowEx2In_[i]] = i;
		stageRowPtr_[rowStage_[i]]++;
	}
	cout << endl;

	// reset stage pointers
	stageRowPtr_[0] = 0;
	for (i=0;i<nstag_;i++)
		stageRowPtr_[i+1] = stageRowPtr_[i] + nRowInStage_[i];


	// array to point to start of new stage
	stageColPtr_ = new int[nstag_+1];

	// count cols in each stage
	for (i=0;i<ncol_;i++)
		nColInStage_[colStage_[i]]++;

	// set stage pointers
	stageColPtr_[0] = 0;
	for (i=0;i<nstag_;i++)
		stageColPtr_[i+1] = stageColPtr_[i] + nColInStage_[i];

	// place index into next open position in its stage
	for (i=0;i<ncol_;i++)
	{
		colEx2In_[i] = stageColPtr_[colStage_[i]];
		colIn2Ex_[colEx2In_[i]] = i;
		stageColPtr_[colStage_[i]]++;
	}

    //for (i=0;i<ncol_;i++){
	//	printf("colEx2In = %d, colIn2Ex = %d for index %d\n",colEx2In_[i],colIn2Ex_[i],i);
	//}

	// reset stage pointers
	stageColPtr_[0] = 0;
	for (i=0;i<nstag_;i++)
		stageColPtr_[i+1] = stageColPtr_[i] + nColInStage_[i];

    // store list with values of integer columns for cur stage only
    int * nColInPrevStages = new int[nstag_];
    nColInPrevStages[0] = 0;
    for (i=1; i<nstag_; i++) {
        nColInPrevStages[i] = nColInPrevStages[i-1] + nColInStage_[i-1];
    }    
    for (i=0; i<integerLength_; i++) {
        int indice = integerIndices[i];
        int stage = colStage_[indice];
        intColsStagewise[stage].push_back(indice - nColInPrevStages[stage]);
    }
    delete nColInPrevStages;


	// reserve place for nodes holding stage dependent fixed information
	this->nodes_.reserve(nstag_);

	// TODO: specialize this interface for core nodes
	cdclo_ = new double*[nstag_];
	cdcup_ = new double*[nstag_];
	cdobj_ = new double*[nstag_];
	cdrlo_ = new double*[nstag_];
	cdrup_ = new double*[nstag_];
	//Christian: Create core nodes for every stage
	for (i=0;i<nstag_;i++)
	{
		SmiNodeData *node = new SmiNodeData(i,this,
			matrix,dclo,dcup,dobj,drlo,drup);
		node->setCoreNode();
        node->addPtr();
		nodes_.push_back(node);
		int nrow=this->getNumRows(i);
		int ncol=this->getNumCols(i);
		int irow=this->getRowStart(i);
		int icol=this->getColStart(i);
		//Christian: TODO Why is irow added to these vectors? And Why are these vectors created, if no one uses the getMethods?
		CoinPackedVector cpv_rlo(node->getRowLowerLength(),node->getRowLowerIndices(),node->getRowLowerElements());
		cdrlo_[i]=cpv_rlo.denseVector(nrow+irow)+irow;

		CoinPackedVector cpv_rup(node->getRowUpperLength(),node->getRowUpperIndices(),node->getRowUpperElements());
		cdrup_[i]=cpv_rup.denseVector(nrow+irow)+irow;

		CoinPackedVector cpv_clo(node->getColLowerLength(),node->getColLowerIndices(),node->getColLowerElements());
		cdclo_[i]=cpv_clo.denseVector(ncol+icol)+icol;

		CoinPackedVector cpv_cup(node->getColUpperLength(),node->getColUpperIndices(),node->getColUpperElements());
		cdcup_[i]=cpv_cup.denseVector(ncol+icol)+icol;

		CoinPackedVector cpv_obj(node->getObjectiveLength(),node->getObjectiveIndices(),node->getObjectiveElements());
		cdobj_[i]=cpv_obj.denseVector(ncol+icol)+icol;

		// sort indices in each row
		//Christian: TODO Why do we have to sort indices?
		for (int ii=irow; ii<irow+nrow; ++ii)
		{
			if (node->getRowLength(ii)>0)
			{
			CoinPackedVector cpv(node->getRowLength(ii),node->getRowIndices(ii),node->getRowElements(ii));
			cpv.sortIncrIndex();
			memcpy(node->getMutableRowElements(ii),cpv.getElements(),sizeof(double)*cpv.getNumElements());
			memcpy(node->getMutableRowIndices(ii),cpv.getIndices(),sizeof(int)*cpv.getNumElements());
			}

		}
	}

	// reserve space for dense row pointers
	pDenseRow_.resize(nrow_);
	for (int i=0; i<nrow_; ++i) pDenseRow_[i]=NULL;

}

void SmiCoreData::copyRowLower(double * d,SmiStageIndex t )
{
	CoinDisjointCopyN(cdrlo_[t],this->getNumRows(t),d);
}

void SmiCoreData::copyRowUpper(double * d,SmiStageIndex t)
{
	CoinDisjointCopyN(cdrup_[t],this->getNumRows(t),d);
}
void SmiCoreData::copyColLower(double * d,SmiStageIndex t)
{
	CoinDisjointCopyN(cdclo_[t],this->getNumCols(t),d);
}
void SmiCoreData::copyColUpper(double * d,SmiStageIndex t)
{
	CoinDisjointCopyN(cdcup_[t],this->getNumCols(t),d);
}
void SmiCoreData::copyObjective(double * d,SmiStageIndex t)
{
	CoinDisjointCopyN(cdobj_[t],this->getNumCols(t),d);
}

SmiCoreData::~SmiCoreData()
{
	for(int t=0; t<this->getNumStages(); ++t)
	{
		int irow=this->getRowStart(t);
		int icol=this->getColStart(t);
		cdrlo_[t]-=irow;
		cdrup_[t]-=irow;
		cdclo_[t]-=icol;
		cdcup_[t]-=icol;
		cdobj_[t]-=icol;
		delete [] cdrlo_[t];
		delete [] cdrup_[t];
		delete [] cdclo_[t];
		delete [] cdcup_[t];
		delete [] cdobj_[t];
	}
	// Now we can delete pointers
	delete [] nColInStage_;
	delete [] nRowInStage_;
	delete [] colStage_;
	delete [] rowStage_;
	delete [] colEx2In_;
	delete [] rowEx2In_;
	delete [] colIn2Ex_;
	delete [] rowIn2Ex_;
	delete [] stageColPtr_;
	delete [] stageRowPtr_;
	delete [] cdrlo_;
	delete [] cdrup_;
	delete [] cdclo_;
	delete [] cdcup_;
	delete [] cdobj_;
    delete [] integerIndices_;
    delete [] binaryIndices_;
    for (int ijk=0; ijk<nodes_.size(); ijk++){ // Delete all the nodes, except the first, which is root of the Scenario Tree
        if ( nodes_[ijk]->deletePtr()  == 0 )
            delete nodes_[ijk];
    }
}

void SmiCoreData::transformToStaircaseStructure(std::map<int,int>& non_staircase_coefficients, SmiTreeNode<SmiScnNode*>* root_node)
{
    // We want to transform the problem to staircase structure
    std::vector<int> new_columns(nstag_,0);
    int num_new_cols = 0;
    for ( std::map<int,int>::iterator it = non_staircase_coefficients.begin(); it != non_staircase_coefficients.end(); ++it){
        // Count the number of columns we want to add to Smi
        int column_stage = getColStage(it->first);
        int how_many_constraints = it->second-column_stage-1; // We only need to add new columns in the stage after the column stage and the stages before the last stage it is used.
        assert(how_many_constraints >= -1);
        for ( int i = column_stage+1; i < how_many_constraints+column_stage+1; i++){
            ++new_columns[i];
        }
    }
    for (int i = 0; i < new_columns.size(); i++)
        num_new_cols += new_columns[i];
    // Now we know how many columns we have to add in which stage, and how many constraints we need to add in which stage ( one for every new column in every stage ) and modify it.
    // Now we can store for every coefficient
    
    // Create new stageColPtr array
    int * new_stageColPtr =  new int[nstag_+1];
    new_stageColPtr[0] = 0;
    for (int i = 1; i < nstag_+1; i++){
        new_stageColPtr[i] = stageColPtr_[i]+new_stageColPtr[i-1]-stageColPtr_[i-1]+new_columns[i-1];
        // The new column start is the old column start + the number of columns we added in the stages before that and the number fo columns we added in the stage before
    }
    int overall_cols = num_new_cols + ncol_;
    // Create new stageRowPtr array
    int * new_stageRowPtr =  new int[nstag_+1];
    new_stageRowPtr[0] = 0;
    for (int i = 1; i < nstag_+1; i++){
        new_stageRowPtr[i] = stageRowPtr_[i]+new_stageRowPtr[i-1]-stageRowPtr_[i-1]+new_columns[i-1];
    }
    int overall_rows = num_new_cols + nrow_;

    // Create the vector of map's
    std::vector<std::map<int,int> > coefficient_transformation(nstag_);
    std::vector<int> count_columns(nstag_,0);
    for (std::map<int,int>::iterator it = non_staircase_coefficients.begin(); it != non_staircase_coefficients.end(); ++it){
        int column_stage = colStage_[it->first];
        int how_many_constraints = it->second-column_stage-1; //
        for (int i = column_stage+1; i < how_many_constraints+column_stage+1;i++){
            coefficient_transformation[i][it->first] = new_stageColPtr[i]+count_columns[i]++; // This also helps in creating the number of new constraints..
        }
    }
    // At the end the count_columns array should have the same size as new_columns array

    // Create new colStage array
    int * new_colStage = new int[ncol_+num_new_cols];
    memcpy(new_colStage,colStage_,stageColPtr_[1]*sizeof(int) );
    // Memcpy everything in the correct places
    for (int i = 1; i < nstag_; i++){
        // At first fill the stage array correctly
        int j = 0;
        for (j = 0; j < new_columns[i]; j++){
            new_colStage[new_stageColPtr[i]+j] = i; 
        }
        // Fill the ones afterwards correctly
        memcpy(new_colStage+new_stageColPtr[i]+j,colStage_+stageColPtr_[i],nColInStage_[i]*sizeof(int) );
    }
    // TODO: Think about it, and below. THis is not yet correct


    // Create new rowStage array
    int * new_rowStage = new int[nrow_+num_new_cols];
    memcpy(new_rowStage,rowStage_,stageRowPtr_[1]*sizeof(int) );
    // Memcpy everything in the correct places
    for (int i = 1; i < nstag_; i++){
        memcpy(new_rowStage+new_stageRowPtr[i],rowStage_+stageRowPtr_[i],nRowInStage_[i]*sizeof(int) );
        // At first fill the stage array correctly
        int j = 0;
        for (j = 0; j < new_columns[i]; j++){ // Works, because at the last stage, we have no new rows..
            new_rowStage[new_stageRowPtr[i+1]-new_columns[i]+j] = i; 
        }
    }

    // Create new nColInStage array
    int * new_nColInStage = new int[nstag_];
    for (int i = 0; i < nstag_;i++)
        new_nColInStage[i] = nColInStage_[i]+new_columns[i];

    int * new_nRowInStage = new int[nstag_];
    for (int i = 0; i < nstag_;i++)
        new_nRowInStage[i] = nRowInStage_[i]+new_columns[i];

    // Look at integer variables? Yes. Find and replace, if necessary, because we moved I do not think that this is necessary!
    // Write method that takes an integer array, a length and does the following
    for (int i = 0; i < integerLength_; i++){
        integerIndices_[i] = new_columns[i]+new_stageColPtr[colStage_[integerIndices_[i]]]+integerIndices_[i]-stageColPtr_[colStage_[integerIndices_[i]]];
    }
    for (int i = 0; i < binaryLength_; i++){
        binaryIndices_[i] = new_columns[i]+new_stageColPtr[colStage_[binaryIndices_[i]]]+binaryIndices_[i]-stageColPtr_[colStage_[binaryIndices_[i]]];
    }
    // We need to add and change the core nodes

    // Now we change the stuff in all of the core nodes at first!
    // Get all the nodes
    for (int i = 0; i< nstag_;i++){
        nodes_[i]->transformToStaircaseStructure(non_staircase_coefficients,coefficient_transformation,new_columns,new_stageColPtr,new_stageRowPtr); // TODO: input new_stageRowPtr here
    }
    // Now we traverse the tree and call the method on each node
    SmiTreeNode<SmiScnNode*> * current_node = root_node; // <- start with root node
    SmiTreeNode<SmiScnNode*> * child_node =NULL;
    SmiTreeNode<SmiScnNode*> * sibling_node = NULL;
    std::deque<SmiTreeNode<SmiScnNode*>* > child_queue;
    std::deque<SmiTreeNode<SmiScnNode*>* > sibling_queue;
    bool down = true;
    int debug_count = 0;
    while ( true ){
        if ( current_node->hasChild() && down ){
            current_node = current_node->getChild();
            continue; 
        }
        else
        {
            down = false;
        }
        // We only want to call this method once! Ensure this!
        if ( !down ){
            current_node->getDataPtr()->getNode()->transformToStaircaseStructure(non_staircase_coefficients,coefficient_transformation,new_columns,new_stageColPtr,new_stageRowPtr);
            debug_count++;
        }

        if ( current_node->hasSibling() ){
            current_node = current_node->getSibling();
            down = true;
            continue;
        }
        if ( current_node->hasParent() ){
            current_node = current_node->getParent();
            down = false;
        }
        else
            break;

    } // end while

    // Change SmiCoreData 
    delete [] nColInStage_; nColInStage_ = new_nColInStage;
    delete [] nRowInStage_; nRowInStage_ = new_nRowInStage;
    delete [] stageColPtr_; stageColPtr_ = new_stageColPtr;
    delete [] stageRowPtr_; stageRowPtr_ = new_stageRowPtr;
    delete [] colStage_; colStage_ = new_colStage;
    delete [] rowStage_; rowStage_ = new_rowStage;
    // TODO: colEx2In etc.

    ncol_ = ncol_+num_new_cols;
    nrow_ = nrow_+num_new_cols;

    // Before we create the new arrays for rlo,rup,clo,cup,obj, we need to delete the old ones
    for (int i = 0; i < nstag_; i++){
        delete [] cdrlo_[i];
        delete [] cdrup_[i];
        delete [] cdobj_[i];
        delete [] cdclo_[i];
        delete [] cdcup_[i];
    }

	// Set collower/upper/objective/rhs..
	for (int i = 0; i < nstag_; i++){
		SmiNodeData * node = nodes_[i];
		int nrow=this->getNumRows(i);
		int ncol=this->getNumCols(i);
		int irow=this->getRowStart(i);
		int icol=this->getColStart(i);
		//Christian: TODO Why is irow added to these vectors? And Why are these vectors created, if no one uses the getMethods?
		CoinPackedVector cpv_rlo(node->getRowLowerLength(),node->getRowLowerIndices(),node->getRowLowerElements());
		cdrlo_[i]=cpv_rlo.denseVector(nrow+irow)+irow;

		CoinPackedVector cpv_rup(node->getRowUpperLength(),node->getRowUpperIndices(),node->getRowUpperElements());
		cdrup_[i]=cpv_rup.denseVector(nrow+irow)+irow;

		CoinPackedVector cpv_clo(node->getColLowerLength(),node->getColLowerIndices(),node->getColLowerElements());
		cdclo_[i]=cpv_clo.denseVector(ncol+icol)+icol;

		CoinPackedVector cpv_cup(node->getColUpperLength(),node->getColUpperIndices(),node->getColUpperElements());
		cdcup_[i]=cpv_cup.denseVector(ncol+icol)+icol;

		CoinPackedVector cpv_obj(node->getObjectiveLength(),node->getObjectiveIndices(),node->getObjectiveElements());
		cdobj_[i]=cpv_obj.denseVector(ncol+icol)+icol;

		// sort indices in each row
		for (int ii=irow; ii<irow+nrow; ++ii)
		{
			if (node->getRowLength(ii)>0)
			{
				CoinPackedVector cpv(node->getRowLength(ii),node->getRowIndices(ii),node->getRowElements(ii));
				cpv.sortIncrIndex();
				memcpy(node->getMutableRowElements(ii),cpv.getElements(),sizeof(double)*cpv.getNumElements());
				memcpy(node->getMutableRowIndices(ii),cpv.getIndices(),sizeof(int)*cpv.getNumElements());
			}

		}
	}

    // TODO: What about col/row names? we might have to fix these too.. 
}

void SmiCoreData::aggregateStages(int firstStage, int secondStage, SmiScenarioTree<SmiScnNode*>& smiTree){
    // Here we make our adjustments!
    assert(secondStage-firstStage == 1); // We can only merge consecutive stages by now! ( iteratively applying the method should also work..
    // We only merge at the end or beginning from now
    assert( firstStage == 0 || secondStage == nstag_-1 );
    SmiTreeNode<SmiScnNode*>* current_node = NULL;
    int num_children = 0;
    
    // At first we handle the case, were we aggregate stages from the beginning :
    if ( firstStage == 0){
        assert( nstag_ >= 3); // Otherwise we can simply return without doing anything


        int number_cols = nColInStage_[secondStage];
        int number_rows = nRowInStage_[secondStage];

        current_node = smiTree.getRoot()->getChild();
        num_children = current_node->numChildren();

        // We handle the childs of this node step by step!
        // Get the first node we handle
        
        // We need a new array that holds all the new matrix, the clo, cup, obj, rlo, rup
        int nels = 0;
        int nstrt = 0;
        // How many elements do we need? 
        // The number of cols of firstStage + num_childen*cols of secondStage
        // Same for number of rows
        // Matrix entries is different, there we need to look at the actual sizes
        SmiScnNode* current_scn_node = current_node->getDataPtr();
        SmiNodeData* current_node_data = current_scn_node->getNode();
        nels += nodes_[firstStage]->nels_;
        // Count the numer of nels in child nodes
        SmiTreeNode<SmiScnNode*>* temp_node = current_node->getChild();
        int num_det_matrix_entries = nodes_[secondStage]->strt_[nodes_[secondStage]->clo_strt_];
        while ( temp_node->hasSibling() ){
            nels += num_det_matrix_entries;
            nels += number_cols*3; // Clo,Cup, Obj
            nels += number_rows*2; // Rlo,Rup
            temp_node = temp_node->getSibling();
        }
        nstrt = num_children*number_rows+5+1;

        double* elements = (double *)calloc(nels  ,sizeof(double));
        int* indices = (int *)   calloc(nels ,sizeof(int)   );
        int* start = (int *)   calloc(nstrt ,sizeof(int)   );
        // Create the deterministic equivalent problem!


         // We need to add the contents of all these nodes to the root node ( which is also a core node ), because the det.eq. is totally deterministic!
        memcpy(elements,nodes_[firstStage]->dels_,nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_]*sizeof(double) );
        memcpy(indices,nodes_[firstStage]->inds_,nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_]*sizeof(double) );
        memcpy(start,nodes_[firstStage]->strt_,nodes_[firstStage]->clo_strt_ *sizeof(double) );

        int overall_current_col_offset = 0;
        int overall_current_row_offset = 0;

        // For the current node, we build the deterministic equivalent
        while ( current_node->hasSibling() ){
            // We need to combine the core row with the stochastic row and get a new row from that! Do this by hand! The other stuff works relatively simple
            int num_stochastic_matrix_entries = current_node_data->strt_[current_node_data->clo_strt_];
            double* new_elements = new double[num_det_matrix_entries];
            int* new_indices = new int[num_det_matrix_entries];
            int* new_start = new int[number_rows];
            // Copy old elements, indices and start arrays, we replace the values step by step.
            memcpy(new_elements,nodes_[secondStage]->dels_,num_det_matrix_entries*sizeof(double));
            memcpy(new_indices,nodes_[secondStage]->inds_,num_det_matrix_entries*sizeof(int));
            memcpy(new_start,nodes_[secondStage]->strt_,number_rows*sizeof(int));
            int current_index = 0;
           
            for (int i = 0; i < number_rows; i++){ // Iterate over the rows of the deterministic node
                int number_elements_in_row = current_node_data->strt_[i+1] - current_node_data->strt_[i];
                if ( number_elements_in_row ){
                    int temp_current_index = new_start[i];
                    // Put these rows together
                    for (int temp_stochastic_index = current_node_data->strt_[i]; temp_stochastic_index < current_node_data->strt_[i+1]; temp_stochastic_index++ ){
                        temp_current_index = current_index;
                        // We need to work here to put these two rows together
                        while ( current_node_data->inds_[temp_stochastic_index] != new_indices[temp_current_index] || temp_current_index < new_start[i+1] ){
                            temp_current_index++;
                        }    
                        assert( temp_current_index < new_start[i+1]); // Otherwise wrong SMPS file
                        new_elements[temp_current_index] = current_node_data->dels_[temp_stochastic_index];
                    }
                }
                for (int j = new_start[i]; j < new_start[i+1]; j++)  // Rewrite the coefficient indices!
                    new_indices[j] += overall_current_col_offset;
               
            }
            // Create the transformation step for the child nodes where we have wrong coefficients in the stochastic nodes AND in the deterministic nodes too!

        }
    }
    // If we aggregate stages from the end
    if ( secondStage == nstag_-1){

    }
    // Merge all the nodes of the two stages together. We need a new number of columns and rows, because we have to build the det-eq. for these two stages

    // 

}

void
SmiNodeData::setCoreNode()
{
	isCoreNode_=true;
}

// constructor from LP data
// copies present values in given data structures for an SmiNodeData
// TODO: allow for special node data like integer variables not in core, etc
//Christian: Stores information corresponding to given stage for ranges, objective and bounds, but not for the matrix. This needs to be done in a better way, eventually.
//Excess memory is released at the end of the method.
SmiNodeData::SmiNodeData(SmiStageIndex stg, SmiCoreData *core,
				 const CoinPackedMatrix * const matrix,
				 CoinPackedVector *dclo,
				 CoinPackedVector *dcup,
				 CoinPackedVector *dobj,
				 CoinPackedVector *drlo,
				 CoinPackedVector *drup):
				 stg_(stg),
				 core_(core), isCoreNode_(false),
				 combineRule_(SmiCoreCombineReplace::Instance()),
				 numarrays_(5), // 5 arrays: dclo, dcup, dobj, drlo, drup
				 nrow_(core->getNumRows(stg_)),
				 ncol_(core->getNumCols(stg_)),
				 rowbeg_(core->getRowStart(stg_)),
				 colbeg_(core->getColStart(stg_)),
				 ptr_count(0)  // used for counted pointer
{
	// count an upper bound for number elements
	nels_ = 0;
	if (matrix)
		nels_ += matrix->getNumElements();
	if (dclo)
		nels_ += dclo->getNumElements();
	if (dcup)
		nels_ += dcup->getNumElements();
	if (dobj)
		nels_ += dobj->getNumElements();
	if (drlo)
		nels_ += drlo->getNumElements();
	if (drup)
		nels_ += drup->getNumElements();

	// assign memory
	this->assignMemory();

	// temp variables for copying
	int offset_dst=0;
	int offset_src=0;
	int len=0;
	int		*ind=NULL;
	double	*els=NULL;
	int i_start=0;

	// offset_dst always points to the next free spot in SmiNodeData storage
	offset_dst      = 0;

	// Matrix
	//Christian: Copy Matrix Elements. 
	//What is done here? CoinPackedMatrix needs to have the same size as the core Matrix, but only the current stage constraints are read in
	//TODO: Change this behaviour, so that only a small matrix is given to this method?!?!
	this->mat_strt_ = i_start;
	if (matrix && matrix->getNumElements() > 0)
	{
		has_matrix_=true;

		// we need a row-ordered matrix here,
		// so we might need a reversed-ordered copy of 'matrix'.
		CoinPackedMatrix * revOrdMatrix = NULL;
		if (matrix->isColOrdered())
		{
			revOrdMatrix = new CoinPackedMatrix();
			revOrdMatrix->reverseOrderedCopyOf(*matrix);
		}
		// This construction is needed to honour the constness of 'matrix'
		const CoinPackedMatrix * localMatrix
			= (matrix->isColOrdered() ? revOrdMatrix : matrix);

		const double *matrix_els = localMatrix->getElements();
		const int    *matrix_ind = localMatrix->getIndices();
		const int    *matrix_len = localMatrix->getVectorLengths();
		const int    *matrix_str = localMatrix->getVectorStarts();


		// check all rows in the stage of this node
		for (int i=0; i<this->nrow_; ++i)
		{
			// "External" index of a matrix row that belongs to the stage
			int isrc = core->getRowExternalIndex(this->rowbeg_+i);

			//copy the matrix row to destination
			if ((len = matrix_len[isrc]))
			{
				//matrix row start
				offset_src = matrix_str[isrc];

				//copy elements and indices
				memcpy(this->dels_+offset_dst,matrix_els+offset_src,len*sizeof(double));
				memcpy(this->inds_+offset_dst,matrix_ind+offset_src,len*sizeof(int));

				//update offset
				offset_dst += len;
			}
			//store row start for next row
			i_start++;
			this->strt_[i_start] = offset_dst;
		

		}
		//convert indices to "Internal"
		for (int j=0; j<offset_dst; j++)
			this->inds_[j] = core->getColInternalIndex(this->inds_[j]);

		// if we had to make a reversed-ordered copy then delete it now
		if (revOrdMatrix)
		{
			delete revOrdMatrix;
		}
	}
	else
		has_matrix_=false;

	// Column Lower Bound
	this->clo_strt_ = i_start;
	if (dclo)
	{
		ind = dclo->getIndices();
		els = dclo->getElements();
		for (int j=0; j<dclo->getNumElements(); j++)
		{
			int icol = ind[j];
			if ( core->getColStage(icol) == stg )
			{
				this->dels_[offset_dst] = els[j];
				this->inds_[offset_dst] = core->getColInternalIndex(icol);
				offset_dst++;
			}
		}
	}
	i_start++;
	this->strt_[i_start] = offset_dst;

	// Column Upper Bound
	this->cup_strt_ = i_start;
	if (dcup)
	{
		ind = dcup->getIndices();
		els = dcup->getElements();
		for (int j=0; j<dcup->getNumElements(); j++)
		{
			int icol = ind[j];
			if ( core->getColStage(icol) == stg )
			{
				this->dels_[offset_dst] = els[j];
				this->inds_[offset_dst] = core->getColInternalIndex(icol);
				offset_dst++;
			}
		}
	}
	i_start++;
	this->strt_[i_start] = offset_dst;

	// Objective
	this->obj_strt_ = i_start;
	if (dobj)
	{
		ind = dobj->getIndices();
		els = dobj->getElements();
		for (int j=0; j<dobj->getNumElements(); j++)
		{
			int icol = ind[j];
			if ( core->getColStage(icol) == stg )
			{
				this->dels_[offset_dst] = els[j];
				this->inds_[offset_dst] = core->getColInternalIndex(icol);
				offset_dst++;
			}
		}
	}
	i_start++;
	this->strt_[i_start] = offset_dst;

	// Row Lower Bound
	this->rlo_strt_ = i_start;
	if (drlo)
	{
		ind = drlo->getIndices();
		els = drlo->getElements();
		for (int j=0; j<drlo->getNumElements(); j++)
		{
			int irow = ind[j];
			if ( core->getRowStage(irow) == stg )
			{
				this->dels_[offset_dst] = els[j];
				this->inds_[offset_dst] = core->getRowInternalIndex(irow);
				offset_dst++;
			}
		}
	}
	i_start++;
	this->strt_[i_start] = offset_dst;

	// Row Upper Bound
	this->rup_strt_ = i_start;
	if (drup)
	{
		ind = drup->getIndices();
		els = drup->getElements();
		for (int j=0; j<drup->getNumElements(); j++)
		{
			int irow = ind[j];
			if ( core->getRowStage(irow) == stg )
			{
				this->dels_[offset_dst] = els[j];
				this->inds_[offset_dst] = core->getRowInternalIndex(irow);
				offset_dst++;
			}
		}
	}
	i_start++;
	this->strt_[i_start] = offset_dst;

	// return excess memory to the heap
    void *temp_ptr = realloc(this->dels_,offset_dst*sizeof(double));
    if (temp_ptr) 
        this->dels_ = (double*)temp_ptr;
    else
        this->dels_ = NULL;
    temp_ptr = realloc(this->inds_,offset_dst*sizeof(int));
    if (temp_ptr)
        this->inds_ = (int*)temp_ptr;
    else
        this->inds_ = NULL;
    this->nels_ = offset_dst; // Set nels_ to the correct number of overall elements in this node
}

int SmiNodeData::combineWithDenseCoreRow(double *dr,const int nels,const int *inds, const double *dels, double *dest_dels,int *dest_indx)
{
	return this->getCoreCombineRule()->Process(dr,this->getCore()->getNumCols(), nels,inds,dels,dest_dels,dest_indx);
}
int SmiNodeData::combineWithDenseCoreRow(double *dr,CoinPackedVector *cpv,double *dels,int *indx)
{
	return getCoreCombineRule()->Process(dr,this->getCore()->getNumCols(),cpv,dels,indx);
}

CoinPackedVector * SmiNodeData::combineWithCoreRow(CoinPackedVector *cr, CoinPackedVector *nr)
{
	CoinPackedVector *cpv = getCoreCombineRule()->Process(cr,nr);
	return cpv;
}

void SmiNodeData::combineWithCoreDoubleArray(double *d_out, const CoinPackedVector &cpv, int o)
{
	if (!isCoreNode_)
		getCoreCombineRule()->Process(d_out,o,cpv);
}
void SmiNodeData::combineWithCoreDoubleArray(double *d_out, const int len, const int * inds, const double *dels, int o)
{
	if (!isCoreNode_)
		getCoreCombineRule()->Process(d_out,o,len,inds,dels);
}

void SmiNodeData::copyRowLower(double * d)
{
	int t=getStage();
	getCore()->copyRowLower(d,t);
	combineWithCoreDoubleArray(d,getRowLowerLength(),getRowLowerIndices(),getRowLowerElements(),getCore()->getRowStart(t));
}

void SmiNodeData::copyRowUpper(double * d){
	int t=getStage();
	getCore()->copyRowUpper(d,t);
	combineWithCoreDoubleArray(d,getRowUpperLength(),getRowUpperIndices(),getRowUpperElements(),getCore()->getRowStart(t));
}

void SmiNodeData::copyColLower(double * d){
	int t=getStage();
	getCore()->copyColLower(d,t);
	combineWithCoreDoubleArray(d,getColLowerLength(),getColLowerIndices(),getColLowerElements(),getCore()->getColStart(t));
}

void SmiNodeData::copyColUpper(double * d){
	int t=getStage();
	getCore()->copyColUpper(d,t);
	combineWithCoreDoubleArray(d,getColUpperLength(),getColUpperIndices(),getColUpperElements(),getCore()->getColStart(t));
}

void SmiNodeData::copyObjective(double * d){
	int t=getStage();
	getCore()->copyObjective(d,t);
	combineWithCoreDoubleArray(d,getObjectiveLength(),getObjectiveIndices(),getObjectiveElements(),getCore()->getColStart(t));
}

SmiNodeData::~SmiNodeData()
{
    SmiRowMap::iterator iRowMap;

    SmiDenseRowMap::iterator idRowMap;
    for (idRowMap=dRowMap.begin(); idRowMap!=dRowMap.end(); ++idRowMap)
        delete[] idRowMap->second;

    deleteMemory();
}

double *
SmiNodeData::getDenseRow(int i) {
		if ( dRowMap[i] == NULL )
		{
			const int  len = this->getRowLength(i);
			const int *ind = this->getRowIndices(i);
			const double *els = this->getRowElements(i);

			int denseSize=this->getCore()->getNumCols();
			double * dv = new double[denseSize];
			CoinFillN(dv, denseSize, 0.0);
			for (int j = 0; j < len; ++j)
			    dv[ind[j]] = els[j];

			dRowMap[i] = dv;
		}
		return dRowMap[i];
}


void
SmiNodeData::assignMemory()
{
	this->nstrt_    = (this->nrow_+1) + this->numarrays_;

	this->dels_     = (double *)calloc(this->nels_  ,sizeof(double));
	this->inds_     = (int *)   calloc(this->nels_  ,sizeof(int)   );
	this->strt_     = (int *)   calloc(this->nstrt_ ,sizeof(int)   );
}

void
SmiNodeData::deleteMemory()
{
	if (this->dels_)
	{
		free(this->dels_);
		this->dels_=NULL;
	}
	if (this->inds_)
	{
		free(this->inds_);
		this->inds_=NULL;
	}
	if (this->strt_)
	{
		free(this->strt_);
		this->strt_=NULL;
	}
}

CoinPackedMatrix* SmiNodeData::generateCoinPackedMatrix() const
{
    double* rowElements = new double[getNumMatrixElements()];
    int* rowIndices = new int[getNumMatrixElements()];
    int* rowStrt = new int[nrow_+1];
    memcpy(rowElements,dels_+mat_strt_,getNumMatrixElements()*sizeof(double));
    memcpy(rowIndices,inds_+mat_strt_,getNumMatrixElements()*sizeof(int));
    memcpy(rowStrt,strt_+mat_strt_,(nrow_+1)*sizeof(int));
    CoinPackedMatrix* matrix =  new CoinPackedMatrix(false,rowStrt,rowIndices,rowElements,getNumMatrixElements());
    return matrix;
    // TODO: Use assignMatrix to prevent unnecessary copying.
}

CoinPackedMatrix* SmiNodeData::generateCoinPackedWMatrix() const
{
    std::vector<double> elements;
    std::vector<int> indices;
    std::vector<int> start;
    std::vector<int> length;
    start.push_back(0);

    // Now we have to go through all the elements and add the W entries
    int nels = getNumMatrixElements();
    int overallWEntries = 0;
    int currentWEntries = 0;
    int row = 0;
    for (row =getMatStart(); row < getMatEnd(); row++){ // Iterate over the rows
        for (int column = strt_[row]; column < strt_[row+1]; column++){ // Iterate over all the columns in this row
            if ( inds_[column] >= colbeg_ ){ // W Column as columns are sorted by stage, and colbeg_ is begin of columns for this stage
                elements.push_back( dels_[column] );
                indices.push_back( inds_[column]-colbeg_ ); // We count from 0 in this matrix, that means we use stage-wise indices instead of global indices
                overallWEntries++;
                currentWEntries++;
            }
        }
        start.push_back(overallWEntries); // We count empty rows comes hell or high water..
        length.push_back(currentWEntries);
        currentWEntries = 0;
    }
    // Count number of different column entries?
    assert(row == core_->getNumRows(stg_));
    if ( overallWEntries ) 
        return new CoinPackedMatrix(false, core_->getNumCols((stg_)),row,overallWEntries,&elements[0],&indices[0],&start[0],&length[0]);
    else
        return new CoinPackedMatrix();
    // TODO: Use assignMatrix to prevent unnecessary copying.
}

CoinPackedMatrix* SmiNodeData::generateCoinPackedTMatrix() const
{
    double* rowElements = new double[getNumMatrixElements()];
    int* rowIndices = new int[getNumMatrixElements()];
    int* rowStrt = new int[nrow_+1];
    memcpy(rowElements,dels_+mat_strt_,getNumMatrixElements()*sizeof(double));
    memcpy(rowIndices,inds_+mat_strt_,getNumMatrixElements()*sizeof(int));
    memcpy(rowStrt,strt_+mat_strt_,(nrow_+1)*sizeof(int));
    // Now we have to go through all the elements and delete stuff we do not want
    int nels = getNumMatrixElements();
    int countTEntries = 0;
    int overallTEntries = 0;
    int length = rowStrt[1]-rowStrt[0];
    for (int i = 0; i < nrow_+1; i++){
        for (int j = rowStrt[i]; j< rowStrt[i+1]-rowStrt[i]; j++){
            if ( rowIndices[j] == stg_ ){ // W Column
                rowIndices[overallTEntries] = rowIndices[j];
                rowElements[overallTEntries] = rowElements[j];
                countTEntries++;
                overallTEntries++;
            }
            rowStrt[i] = (i>0?rowStrt[i-1]+countTEntries:countTEntries);
            countTEntries = 0;
        }
    }
    CoinPackedMatrix* matrix =  new CoinPackedMatrix(false,rowStrt,rowIndices,rowElements,overallTEntries);
    delete rowElements;
    delete rowIndices;
    delete rowStrt;
    return matrix;
    // TODO: Use assignMatrix to prevent unnecessary copying.
}

void SmiNodeData::transformToReplace()
{   // If we already use replace, do not change it
    if ( combineRule_ == SmiCoreCombineReplace::Instance() )
        return;
    // If we are a core node, i.e. deterministic node for the stage, we have nothing to do
    if ( isCoreNode() ){
        combineRule_ = SmiCoreCombineReplace::Instance();
        return;
    }

    if ( combineRule_ != SmiCoreCombineAdd::Instance()){
        // We continue only if we have an add instance
        return;
    }

    // Now we need to transform the node.
    // Which stage do we have?
    SmiNodeData* deterministic_node = core_->getNode(stg_);
    // That means: replace the values in the add case with the value from core node plus the value stored now.
    // Find the things we need to replace..
    // Do we have a matrix?
    int num_rows_ = core_->getNumRows(stg_);
    assert( num_rows_ = nrow_ );
    if ( has_matrix_ ){
        int current_index = 0;
        // Get pointer to the deterministic matrix first and replace accordingly
        for (int i = rowbeg_; i < rowbeg_+num_rows_;i++){
            const int* row_indices = deterministic_node->getRowIndices(i);
            const double* row_elements =  deterministic_node->getRowElements(i);
            for (int j = 0; j < deterministic_node->getRowLength(i); j++){
                // Iterate over all elements in the row
                if ( row_indices[j] == inds_[current_index] ){
                    // Do Something
                    dels_[current_index] = row_elements[j]+dels_[current_index];
                    ++current_index;
                }
                // We need to iterate over all stochastic elements and add the corresponding values.
            }
            assert(strt_[i+1]==current_index);
        }
    }
    if ( getLength(clo_strt_) ){
        transformElements(clo_strt_, deterministic_node, deterministic_node->getCloStart());
    }
    if ( getLength(cup_strt_)){
        transformElements(cup_strt_, deterministic_node, deterministic_node->getCupStart());
    }
    if ( getLength(obj_strt_)){
        transformElements(obj_strt_, deterministic_node, deterministic_node->getObjStart());
    }
    if ( getLength(rlo_strt_)){
        transformElements(rlo_strt_, deterministic_node, deterministic_node->getRloStart());
    }
    if ( getLength(rup_strt_)){
        transformElements(rup_strt_, deterministic_node, deterministic_node->getRupStart());
    }

    // Set replace rule
    combineRule_ = SmiCoreCombineReplace::Instance();
}

void SmiNodeData::transformElements( int start, SmiNodeData* deterministic_node, int deterministic_start )
{
    int internal_index = strt_[start];
    const int stoch_length = getLength(start);
    const double* stoch_elements = getElements(start);
    const int* stoch_indices = getIndices(start);
    const int* stoch_strt = getStarts(start);
    const double* deterministic_elements = deterministic_node->getElements(deterministic_start);
    const int* deterministic_indices = deterministic_node->getIndices(deterministic_start);
    const int deterministic_length = deterministic_node->getLength(deterministic_start);

    int stoch_index = 0;
    for (int i = 0; i < deterministic_length; i++){ // Iterate over all elements we have.
        if ( deterministic_indices[i] == stoch_indices[stoch_index] ){
            // We add the stuff (assuming CoreCombineAddRule
            dels_[internal_index+stoch_index] = deterministic_elements[i]+stoch_elements[stoch_index];
            stoch_index++;
        }
    }
    assert(stoch_index == stoch_length); // Otherwise we forgot an element.
}

void SmiNodeData::transformToStaircaseStructure( std::map<int,int>& non_staircase_coefficients,std::vector<std::map<int,int> >& coefficient_transformation,std::vector<int>& new_columns, const int* new_stageColPtr, const int * new_stageRowPtr )
{
    // We know which coefficients are non_staircase coefficients via the first map
    // We know which of the old coefficients is turned into which new coefficient via the vector of maps
    // We also know how many new columns we have in each stage via the vector
    // We should also know the Beginnings of the new Column Stages and Row Stages for that matter ( these are the ones we have to extend..

    // Iterate through the whole array and change coefficients 

    // We need our stage : stg_
    if ( stg_ == 0)
        return; // We do nothing in the first stage
    int last_index = strt_[rlo_strt_];
    // At first we add the new colbeg to all the columns
    // We change nsc-indices to the ones we specified before
    for (int i = 0; i < last_index; i++){
        if ( inds_[i] < core_->getColStart(stg_-1) && non_staircase_coefficients.find(inds_[i]) != non_staircase_coefficients.end() ){ // A non-staircase coefficient
            // Find the coefficient in the stage_wise map for the stage after the coefficient, because this is the stage where the new constraint is added which takes the column in the next stage.
            std::map<int,int>::iterator it(coefficient_transformation[stg_-1].find(inds_[i]));
            // What happens here is maybe wrong!.
            if ( it != coefficient_transformation[stg_-1].end() )
                inds_[i] = it->second;
            else{
                // What now? We have a nsc-coefficient which should not be replaced?
                assert(false);
            }
        }
        else {
            // Just count the index up: new columns + new_stage_index_start+ relative index 
            inds_[i] = new_columns[core_->getColStage(inds_[i])]+new_stageColPtr[core_->getColStage(inds_[i])]+ inds_[i] - core_->getColStart(core_->getColStage(inds_[i]));
        }
        assert(inds_[i]<new_stageColPtr[stg_]+new_columns[stg_]+core_->getNumCols(stg_));
    }
    // now we have to change the row indices
    for (int i = last_index; i < strt_[nstrt_-1]; i++){
        inds_[i] = new_stageRowPtr[core_->getRowStage(inds_[i])] - core_->getRowStart(core_->getRowStage(inds_[i])) + inds_[i];
    }
    if ( !isCoreNode_ && stg_ > 0 && stg_ < core_->getNumStages()-1){
        // In case of non-core node we still need to add empty placeholders for the new rows! (only the strt_ array)
        strt_ = (int *) realloc(strt_,(nstrt_+new_columns[stg_])*sizeof(int));
        //
        memcpy(strt_+clo_strt_+new_columns[stg_],strt_+clo_strt_, (nstrt_-clo_strt_)*sizeof(int));
        for (int i = 0; i < new_columns[stg_]; i++){
            // Fill the strt_array..
            strt_[nrow_+i] = strt_[nrow_];
        }
    }


    // Now we have to add the new rows and cols ( only in core nodes )
    // We need new space for the rows and the rowlb and rowub and obj,clo and cup.
    // How many cols and rows do we add? coefficient_transformation[stg] many
    // 
    if ( isCoreNode_ && stg_ > 0 && stg_ < core_->getNumStages()-1 ){// Not in the first or last stage
        int new_size = strt_[nstrt_-1];
        if ( has_matrix_ ){
            new_size += 7*new_columns[stg_];
        }
        else{
            assert(false); // This makes nearly no sense if we have no matrix..
            new_size += 3*new_columns[stg_];
        }

        double *new_dels     = (double *)calloc(new_size  ,sizeof(double));
        int *new_inds     = (int *)   calloc(new_size  ,sizeof(int)   );
        int * new_strt     = (int *)   calloc(nstrt_+new_columns[stg_] ,sizeof(int)   );
        //double * new_dels = new double[new_size];
        //int * new_inds = new int[new_size];
        //int * new_strt_ = new int[nstrt_+new_columns[stg_]];
        // memcpy the matrix
        memcpy(new_dels,dels_,strt_[clo_strt_]*sizeof(double) );
        memcpy(new_inds,inds_,strt_[clo_strt_]*sizeof(int) );
        memcpy(new_strt,strt_,clo_strt_*sizeof(int) );
        // Create the new matrix entries
        int start_index = strt_[clo_strt_];
        int strt_index = clo_strt_;
        int count = 0;
        // We iterate over the coefficient_transformation_map to create the required constraints!
        for (std::map<int,int>::iterator it = coefficient_transformation[stg_].begin(); it != coefficient_transformation[stg_].end(); ++it){
            int first_index = -1;
            int second_index = -1;
            if ( core_->getColStage(it->first) == stg_-1 ){
                first_index = it->first;
                second_index = it->second;
            } // We need to create a new row that maps it->first to it->second
            else { // We are in a stage where we have not direct access to it->first, but we already need to use a new column we introduced. Therefore we need the it->second of it->first of the stage before.
                assert( core_->getColStage(it->first) < stg_-1);
                std::map<int,int>::iterator it2(coefficient_transformation[stg_-1].find(it->first));
                if ( it2 != coefficient_transformation[stg_-1].end() ){
                    first_index = it2->second;
                    second_index = it->second;
                }
                else
                    assert(false);
            }
            // The first index is the index of the OLD columns, but not the new ones. So we *must* add the number of new columns we added
            int offset = 0;
            for (int i = 0; i< core_->getColStage(it->first);i++){
                offset += new_columns[i];
            }
            first_index += offset;
            new_dels[start_index+2*count] = 1.0;
            new_dels[start_index+2*count+1] = -1.0;
            new_inds[start_index+2*count] = first_index;
            new_inds[start_index+2*count+1] = second_index;
            new_strt[strt_index+count] = start_index+2*count; 
            count++;

        }// end for
        start_index = start_index+2*count; // advance our start_index
        strt_index += count; //I need to know how long the last bit was
        new_strt[strt_index] = start_index;

        // Fill new columns
        count = 0;
        for (int i = 0; i < new_columns[stg_]; i++){
            new_dels[start_index+i] = -core_->getInfinity();// -infinity
            new_inds[start_index+i] = new_stageColPtr[stg_]+i;
            new_dels[start_index+i+ncol_+new_columns[stg_]] = core_->getInfinity();// -infinity
            new_inds[start_index+i+ncol_+new_columns[stg_]] = new_stageColPtr[stg_]+i;
            new_dels[start_index+i+2*ncol_+2*new_columns[stg_]] = 0;// -infinity
            new_inds[start_index+i+2*ncol_+2*new_columns[stg_]] = new_stageColPtr[stg_]+i;
        }
        start_index += new_columns[stg_];
        // memcpy clo
        memcpy(new_dels+start_index,dels_+strt_[clo_strt_],ncol_*sizeof(double));
        memcpy(new_inds+start_index,inds_+strt_[clo_strt_],ncol_*sizeof(int));
        start_index += ncol_;
        new_strt[++strt_index] = new_strt[strt_index-1]+new_columns[stg_]+ncol_; 
        // memcpy cup
        start_index += new_columns[stg_];
        memcpy(new_dels+start_index,dels_+strt_[cup_strt_],ncol_*sizeof(double));
        memcpy(new_inds+start_index,inds_+strt_[cup_strt_],ncol_*sizeof(int));
        start_index += ncol_;
        new_strt[++strt_index] = new_strt[strt_index-1]+new_columns[stg_]+ncol_; 
        // memcpy obj
        start_index += new_columns[stg_];
        memcpy(new_dels+start_index,dels_+strt_[obj_strt_],ncol_*sizeof(double));
        memcpy(new_inds+start_index,inds_+strt_[obj_strt_],ncol_*sizeof(int));
        start_index += ncol_;
        new_strt[++strt_index] = new_strt[strt_index-1]+new_columns[stg_]+ncol_; 

        // Insert Rlo and Rup
        start_index += nrow_;
        for (int i = 0; i < new_columns[stg_]; i++){
            new_dels[start_index+i] = 0;
            new_inds[start_index+i] = new_stageRowPtr[stg_+1]-new_columns[stg_]+i;
            new_dels[start_index+i+nrow_+new_columns[stg_]] = 0;
            new_inds[start_index+i+nrow_+new_columns[stg_]] = new_stageRowPtr[stg_+1]-new_columns[stg_]+i;
        }
        // memcpy rlo
        start_index -= nrow_; // go back to fill the gap of nrow_
        memcpy(new_dels+start_index,dels_+strt_[rlo_strt_],nrow_*sizeof(double));
        memcpy(new_inds+start_index,inds_+strt_[rlo_strt_],nrow_*sizeof(int));
        start_index += new_columns[stg_]+nrow_;       


        new_strt[++strt_index] = new_strt[strt_index-1]+new_columns[stg_]+nrow_; 
        memcpy(new_dels+start_index,dels_+strt_[rup_strt_],nrow_*sizeof(double));
        memcpy(new_inds+start_index,inds_+strt_[rup_strt_],nrow_*sizeof(int));
        start_index += new_columns[stg_];
        start_index += nrow_;
        new_strt[++strt_index] = new_strt[strt_index-1]+new_columns[stg_]+nrow_; 
        assert(strt_index+1 == nstrt_+new_columns[stg_]);
        assert(start_index == new_size);

        // We also should set nels_ but this was wrong anyway..

        deleteMemory();

        dels_ = new_dels;
        inds_ = new_inds;
        strt_ = new_strt;
    }// end if: is core node?

    // These values have to be set in any case
    int new_ncol = ncol_+new_columns[stg_];
    int new_nrow = nrow_+new_columns[stg_];
    // colbeg_ and rowbeg_ can be set by new_stageColPtr and new_stageRowPtr
    // clo_strt_ and the like has to set + new_columns[stg_], and nstrt_ also
    // Set the values!
    colbeg_ = new_stageColPtr[stg_];
    rowbeg_ = new_stageRowPtr[stg_];
    ncol_ = new_ncol;
    nrow_ = new_nrow;
    clo_strt_ += new_columns[stg_];
    cup_strt_ += new_columns[stg_];
    obj_strt_ += new_columns[stg_];
    rlo_strt_ += new_columns[stg_];
    rup_strt_ += new_columns[stg_];
    nstrt_ += new_columns[stg_];
}
#include "SmiScnData.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <deque>
#include <sstream>

#include "SmiScnModel.hpp"
#include "CoinHelperFunctions.hpp"
#include "OsiSolverInterface.hpp"
#include "CoinPackedMatrix.hpp"


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
    // TODO: Think about it, and below. This is not yet correct


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
    //for (int i = 0; i < nstag_; i++){
    //    delete [] cdrlo_[i];
    //    delete [] cdrup_[i];
    //    delete [] cdobj_[i];
    //    delete [] cdclo_[i];
    //    delete [] cdcup_[i];
    //} // Test why this does no longer work!

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

void SmiCoreData::increaseIndices( SmiNodeData* node_data, int col_amount, int row_amount )
{
    for (int i = 0; i < node_data->strt_[node_data->rlo_strt_]; i++){
        node_data->inds_[i] += col_amount;
    }
    for (int i = node_data->strt_[node_data->rlo_strt_]; i < node_data->strt_[node_data->nstrt_-1]; i++){
        node_data->inds_[i] += row_amount;
    }
    node_data->colbeg_ += col_amount;
    node_data->rowbeg_ += row_amount;
}


void SmiCoreData::aggregateStages(int firstStage, int secondStage, SmiScenarioTree<SmiScnNode*>& smiTree){
    // Here we make our adjustments!
    assert(secondStage-firstStage == 1); // We can only merge consecutive stages by now! ( iteratively applying the method should also work..
    bool final_stage = true; // If not true, we have to deal with nodes after the child nodes..
    if ( secondStage < nstag_-1)
        final_stage = false;
     SmiTreeNode<SmiScnNode*>* current_node = NULL;
    int number_children = 0;
    int number_scenarios = 0;

    int number_cols = nColInStage_[secondStage];
    int number_rows = nRowInStage_[secondStage];

    std::vector<SmiTreeNode<SmiScnNode*>*> first_stage_nodes;
    
    // Here is what we do:
    /*
    * 1: Initialize variables with information about the second stage, find the current node.
    * 2: Build the Det.Eq in the first stage node(s)
    * * a) Copy the element data 
    * * b) Adjust the probability for the objective function coefficients
    * 3: (If needed) Transform nodes in stage after second stage to work with the new Det. Eq. (i.e. change indices in core node, change indices in stochastic nodes)
    */
    // At first we handle the case, were we aggregate stages from the beginning :
    if ( firstStage == 0){
        if ( nstag_ < 2)
            return; // We can simply return without doing anything, because then we would have a deterministic equivalent!.
        current_node = smiTree.getRoot();
        first_stage_nodes.push_back(current_node);
        number_children = current_node->numChildren();
        number_scenarios = smiTree.getNumScenarios();

    }
    else {
        // Find all the nodes at the current stage
        // We need to go through all the child/siblings until we reach our current stage
        SmiTreeNode<SmiScnNode*>* temporary_node = smiTree.getRoot()->getChild();
        bool goDown = true;
        bool goSibling = false;
        while ( temporary_node != smiTree.getRoot() ){
            if ( goDown && temporary_node->depth() < firstStage){
                temporary_node = temporary_node->getChild();
            }
            else if ( goDown && temporary_node->depth() == firstStage  ){
                first_stage_nodes.push_back(temporary_node);
                goDown = false;
                if ( temporary_node->hasSibling() ){
                    temporary_node = temporary_node->getSibling();
                    goDown = true;
                }
                else {
                    temporary_node = temporary_node->getParent();
                }

            }
            else if ( !goDown ){
                if ( temporary_node->depth() == firstStage )
                    first_stage_nodes.push_back(temporary_node);
                if ( temporary_node->hasSibling() ){
                    temporary_node = temporary_node->getSibling();
                    goDown = true;
                }
                else {
                    goDown = false;
                    temporary_node = temporary_node->getParent();
                }
            }
        }
        current_node = first_stage_nodes.back();
        number_children = current_node->numChildren();
        number_scenarios = first_stage_nodes.size();
    }
    // We build the detEq in the following order:
    // - Iterate over firstStageNode from 0 to size()-1
    // - Iterate over the childs of firstStageNodes in a while loop with the getChild/getSibling Semantic, starting first with the child and then the siblings.
    // 
    // Example: firststage = 7 cols, secondstage = 5 cols, 3 children: columns 7-11 to the first child, 12-16 to the second and 17-21 to the third child.
    // Subsequent childs therefore need to adress the correct columns!
    // Because childs of the firstStageNodes childs become childs of the firstStageNodes in the process, we need to address the problem of correct reference to the correct column.

    // Find the maximal number of children at this stage!
    SmiTreeNode<SmiScnNode*> * temp_node = current_node;
    bool different_tree_size = false;
    for (int i = 0; i < first_stage_nodes.size(); i++){
        if ( number_children < first_stage_nodes[i]->numChildren() ){
            number_children = first_stage_nodes[i]->numChildren();
            different_tree_size = true; // This is not good..
            // TODO: We can also check for different probabilities here!
        }
    }
    
    assert(number_children>= current_node->numChildren());
   // BuildDetEqInNode(SmiTreeNode<SmiScnNode*> * tree_node, int firstStage, int secondStage);

    // We need a new array that holds all the new matrix, the clo, cup, obj, rlo, rup
    int nels = 0;
    int nstrt = 0;
    // How many elements do we need? 
    // The number of cols of firstStage + num_childen*cols of secondStage
    // Same for number of rows
    // Matrix entries is different, there we need to look at the actual sizes

    /*
     * Creating the Deterministic Equivalent Core Model
     */

    // Determine how many elements we need to store
    //SmiTreeNode<SmiScnNode*>* temp_node = current_node->getChild();
    int num_det_matrix_entries = nodes_[secondStage]->strt_[nodes_[secondStage]->clo_strt_];
    nels += num_det_matrix_entries;
    nels += number_cols*3; // Clo,Cup, Obj
    nels += number_rows*2; // Rlo,Rup
    nels *= number_children; // This times the number of children we have 
    // + first stage entries
    nels += nodes_[firstStage]->strt_[nodes_[firstStage]->nstrt_-1];
    // Change the nstrt_ value to the amount of added rows 
    nstrt = nodes_[firstStage]->clo_strt_+number_children*number_rows+5+1;

    // Allocate the memory for the elements, indices and start arrays
    double* elements = (double *)calloc(nels  ,sizeof(double));
    int* indices = (int *)   calloc(nels ,sizeof(int)   );
    int* start = (int *)   calloc(nstrt ,sizeof(int)   );
    // Create the deterministic equivalent problem!

    // We need to add the contents of all these nodes to the root node ( which is also a core node ), because the det.eq. is totally deterministic!
    // Copy the core node information from the first stage
    memcpy(elements,nodes_[firstStage]->dels_,nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_]*sizeof(double) );
    memcpy(indices,nodes_[firstStage]->inds_,nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_]*sizeof(int) );
    memcpy(start,nodes_[firstStage]->strt_,(nodes_[firstStage]->clo_strt_+1) *sizeof(int) ); // We need to account for the leading 0: one element more than number of rows..

    int overall_current_col_offset = nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_];
    int overall_current_strt_offset = nodes_[firstStage]->clo_strt_+1;

    assert(number_rows==nodes_[secondStage]->clo_strt_);

    int col_offset = 0;
    int col_offset_second_stage = number_children*number_cols; // This is for the stages afterwards

    for (int i = 0; i < number_children; i++){
        // memcpy the deterministic data of secondStage
        memcpy(elements+overall_current_col_offset,nodes_[secondStage]->dels_,nodes_[secondStage]->strt_[number_rows]*sizeof(double));
        // We need to copy the indices one_by_one
        for (int j = 0; j < nodes_[secondStage]->strt_[number_rows]; j++){ // Depending on which index we have: FirstStage or SecondStage
            if ( nodes_[secondStage]->inds_[j] < stageColPtr_[secondStage] ) // TODO: Bedingter Operator
                indices[overall_current_col_offset+j] = nodes_[secondStage]->inds_[j];
            else
                indices[overall_current_col_offset+j] = nodes_[secondStage]->inds_[j]+ col_offset;
        }
        for (int j = 0; j < number_rows; j++)
            start[overall_current_strt_offset+j] = nodes_[secondStage]->strt_[j+1]+overall_current_col_offset; // Take the start value + the values in the elements/indices arrays so far

        // Update the offsets for the array indexation
        overall_current_col_offset += nodes_[secondStage]->strt_[number_rows]; // which should be equal to nels_
        overall_current_strt_offset += number_rows;
        // Update the col offset for the deterministic equivalent
        col_offset += number_cols;

        // We need to increase indices for all the nodes after the thirdstage nodes
        if ( secondStage < nstag_-2 ){
            SmiTreeNode<SmiScnNode* > * child_node = first_stage_nodes[i]->getChild();
            // Get all the children of this node
             bool goDown = true;
             // This loop does not do what it is supposed to do! TODO: Fix it
            while ( child_node != first_stage_nodes[i] ){ // As long as there are nodes below the thirdstage
                // Go get the child. We increase the indices in every one of these nodes
                if ( child_node->hasChild() && goDown)
                    child_node = child_node->getChild();
                else if ( goDown){
                    // We have no child
                    // Do something with the node
                    if ( child_node->depth() > secondStage+1 )
                        increaseIndices(child_node->getDataPtr()->getNode(),number_children*number_cols,(number_children-1)*number_rows+number_cols);
                    goDown = false;
                    if ( child_node->hasSibling() ){
                        child_node = child_node->getSibling();
                        goDown = true;
                    }
                    else
                        child_node = child_node->getParent();

                }
                else // goDown = false
                {
                    // Do something with the node
                    if ( child_node->depth() > secondStage+1 )
                        increaseIndices(child_node->getDataPtr()->getNode(),number_children*number_cols,(number_children-1)*number_rows+number_cols);
                    if ( child_node->hasSibling() ){
                        child_node = child_node->getSibling();
                        goDown = true;
                    }
                    else
                        child_node = child_node->getParent();

                }
            }// End While for subtree!
        }// End If 
 
    }// End For : matrix indices

    if ( !final_stage){
        // Now we need to change the thirdstage nodes
        for (int i = 0; i < first_stage_nodes.size(); i++){
            SmiTreeNode<SmiScnNode*>* second_stage_node = first_stage_nodes[i]->getChild(); // we need this node and all the sibling nodes!
            // We need to do the following for all the siblings!
            while ( second_stage_node != NULL ){
                SmiTreeNode<SmiScnNode*>* third_stage_node = second_stage_node->getChild();
                int count = 0;
                while ( third_stage_node != NULL){
                    // Where are we? Column begin is the same for all nodes in thirdstage, due to core node!
                    // We do the following: Exchange indices of previous stage nodes with indices of newly added columns, that ensure we react to the correct detEq index!
                    // Increase all the indices of the thirdstage 
                    // Set a -1 in the new row section!
                    SmiNodeData* third_stage_node_data = third_stage_node->getDataPtr()->getNode();
                    // We need to extend the dels, inds and strt arrays by holding the new rows!
                    int nels = third_stage_node_data->nels_;
                    assert ( nels >= third_stage_node_data->strt_[third_stage_node_data->nstrt_-1]);
                    nels += number_cols; // The new entries for the stochastic rows, everything else stays the same!
                    int num_start = third_stage_node_data->nstrt_+number_cols; // Account for the new rows!
                    double* new_dels = (double *) calloc(nels, sizeof(double));
                    int * new_inds = (int*) calloc(nels,sizeof(int));
                    int* new_start = (int*) calloc(num_start,sizeof(int));
                    int new_start_index = third_stage_node_data->clo_strt_;
                    assert( new_dels );
                    assert( new_inds );
                    assert( new_start );
                    // Copy in the old data elements
                    memcpy(new_dels,third_stage_node_data->dels_,third_stage_node_data->strt_[new_start_index]*sizeof(double));
                    memcpy(new_dels+third_stage_node_data->strt_[new_start_index]+number_cols,third_stage_node_data->dels_+third_stage_node_data->strt_[new_start_index], (third_stage_node_data->strt_[third_stage_node_data->nstrt_-1]-third_stage_node_data->strt_[new_start_index]) * sizeof(double));
                    for (int j = 0; j < number_cols; j++)
                        new_dels[third_stage_node_data->strt_[new_start_index]+j] = -1;

                    // Copy in the old data start array
                    memcpy(new_start,third_stage_node_data->strt_,(third_stage_node_data->clo_strt_+1)*sizeof(int));

                    for (int j = 0; j < number_cols; j++)
                        new_start[new_start_index+1+j] = new_start[new_start_index+j]+1;
                    // Add the rest
                    for (int j = 0; j < 5; j++)
                        new_start[new_start_index+number_cols+j+1] = new_start[new_start_index+j+number_cols] + third_stage_node_data->strt_[new_start_index+j+1]-third_stage_node_data->strt_[new_start_index+j];


                    // Now we copy the indices!
                    for (int j = 0; j < third_stage_node_data->strt_[new_start_index]; j++){
                        new_inds[j] = third_stage_node_data->inds_[j] +  number_cols*number_children;
                    }
                    for (int j = 0; j < number_cols; j++){ // We add the new rows. We need to set the indices of the stage before
                        new_inds[third_stage_node_data->strt_[new_start_index]+j] = stageColPtr_[secondStage]+(number_cols)*count+j;
                    }
                    for ( int j = third_stage_node_data->strt_[new_start_index]; j < third_stage_node_data->strt_[third_stage_node_data->rlo_strt_]; j++){ // Assign the missing indices for clo, cup, obj function
                        new_inds[j+number_cols] = third_stage_node_data->inds_[j]+number_children*number_cols;
                        
                    }
                    for ( int j = third_stage_node_data->strt_[third_stage_node_data->rlo_strt_]; j < third_stage_node_data->strt_[third_stage_node_data->nstrt_-1]; j++){ // Assign the missing indices for rlo,rup
                        new_inds[j+number_cols] = third_stage_node_data->inds_[j]+(number_children-1)*number_rows;
                    }
                    assert( third_stage_node_data->colbeg_ == stageColPtr_[secondStage+1]);

                    // Assign the new indices..
                    third_stage_node_data->clo_strt_ += number_cols;
                    third_stage_node_data->cup_strt_ += number_cols;
                    third_stage_node_data->obj_strt_ += number_cols;
                    third_stage_node_data->rlo_strt_ += number_cols;
                    third_stage_node_data->rup_strt_ += number_cols;
                    third_stage_node_data->nstrt_ += number_cols; // Because is the number of new rows we added..
                    third_stage_node_data->nels_ = new_start[third_stage_node_data->nstrt_-1]; // The number of elements
                    third_stage_node_data->deleteMemory();
                    third_stage_node_data->dels_ = new_dels;
                    third_stage_node_data->inds_ = new_inds;
                    third_stage_node_data->strt_ = new_start;
                    third_stage_node_data->colbeg_ += (number_children-1)*number_cols;
                    third_stage_node_data->rowbeg_ += (number_children-1)*number_rows;
                    third_stage_node_data->nrow_ += number_cols;
                    third_stage_node_data->ncol_ += number_cols;
                    


                    third_stage_node = third_stage_node->getSibling();
                    count++;
                }// End While Thirdstagenode
                second_stage_node = second_stage_node->getSibling();
            } // End While SecondStageNode
        } // End For
    }//End If !finalStage

#pragma region fivearrays
    // Column Lower
    int new_clo_offset = overall_current_strt_offset;
    col_offset = 0;
    memcpy(elements+overall_current_col_offset,nodes_[firstStage]->dels_+nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_],nColInStage_[firstStage]*sizeof(double));
    memcpy(indices+overall_current_col_offset,nodes_[firstStage]->inds_+nodes_[firstStage]->strt_[nodes_[firstStage]->clo_strt_],nColInStage_[firstStage]*sizeof(int));
    overall_current_col_offset += nColInStage_[firstStage]; // Copy first stage clo
    for (int i = 0; i < number_children; i++ ){
        // We need to fill clo, cup, obj, rlo, rup
        memcpy(elements+overall_current_col_offset,nodes_[secondStage]->dels_+nodes_[secondStage]->strt_[nodes_[secondStage]->clo_strt_],number_cols*sizeof(double));
        for (int j = 0; j < number_cols; j++){
            indices[overall_current_col_offset+j] = stageColPtr_[secondStage]+col_offset+j;
        }
        overall_current_col_offset += number_cols;
        col_offset += number_cols;
    }
    start[overall_current_strt_offset] = start[overall_current_strt_offset-1]+number_cols*number_children+nColInStage_[firstStage]; 
    overall_current_strt_offset++;
    // Column Upper
    col_offset = 0;
    memcpy(elements+overall_current_col_offset,nodes_[firstStage]->dels_+nodes_[firstStage]->strt_[nodes_[firstStage]->cup_strt_],nColInStage_[firstStage]*sizeof(double));
    memcpy(indices+overall_current_col_offset,nodes_[firstStage]->inds_+nodes_[firstStage]->strt_[nodes_[firstStage]->cup_strt_],nColInStage_[firstStage]*sizeof(int));
    overall_current_col_offset += nColInStage_[firstStage]; // Copy first stage cup
    for (int i = 0; i < number_children; i++ ){
        // We need to fill clo, cup, obj, rlo, rup
        memcpy(elements+overall_current_col_offset,nodes_[secondStage]->dels_+nodes_[secondStage]->strt_[nodes_[secondStage]->cup_strt_],number_cols*sizeof(double));
        for (int j = 0; j < number_cols; j++){
            indices[overall_current_col_offset+j] = stageColPtr_[secondStage]+col_offset+j;
        }
        overall_current_col_offset += number_cols;
        col_offset += number_cols;
    }
    start[overall_current_strt_offset] = start[overall_current_strt_offset-1]+number_cols*number_children+nColInStage_[firstStage];
    overall_current_strt_offset++;
    // Objective Function
    col_offset = 0;
    memcpy(elements+overall_current_col_offset,nodes_[firstStage]->dels_+nodes_[firstStage]->strt_[nodes_[firstStage]->obj_strt_],nColInStage_[firstStage]*sizeof(double));
    memcpy(indices+overall_current_col_offset,nodes_[firstStage]->inds_+nodes_[firstStage]->strt_[nodes_[firstStage]->obj_strt_],nColInStage_[firstStage]*sizeof(int));
    overall_current_col_offset += nColInStage_[firstStage]; // Copy first stage obj
    SmiTreeNode<SmiScnNode*>* obj_node = current_node->getChild();

    for (int i = 0; i < number_children; i++ ){
        // We need to fill clo, cup, obj, rlo, rup
        memcpy(elements+overall_current_col_offset,nodes_[secondStage]->dels_+nodes_[secondStage]->strt_[nodes_[secondStage]->obj_strt_],number_cols*sizeof(double));
        int obj_offset = nodes_[secondStage]->strt_[nodes_[secondStage]->obj_strt_];
        for (int j = 0; j < number_cols; j++){ // We need to set the correct value from the scenario subproblems! But this does not help, if we aggregate from the last stage, because we might have different Det.Eq. in different nodes!. Especially if the node probabilities are not the same. So we need to adapt the values in the stochastic setting!. Therefore we set the value to 0 to be replaced by stochastic node!
            elements[overall_current_col_offset+j] = 0;//nodes_[secondStage]->dels_[obj_offset+j]; // Times the correct probability?! But this can change across tree's
        }
        for (int j = 0; j < number_cols; j++){
            indices[overall_current_col_offset+j] = stageColPtr_[secondStage]+col_offset+j;
        }
        overall_current_col_offset += number_cols;
        col_offset += number_cols;
    }
    start[overall_current_strt_offset] = start[overall_current_strt_offset-1]+number_cols*number_children+nColInStage_[firstStage]; 
    overall_current_strt_offset++;
    // Row Lower
    col_offset = 0;
    int row_offset = 0;
    memcpy(elements+overall_current_col_offset,nodes_[firstStage]->dels_+nodes_[firstStage]->strt_[nodes_[firstStage]->rlo_strt_],nRowInStage_[firstStage]*sizeof(double));
    memcpy(indices+overall_current_col_offset,nodes_[firstStage]->inds_+nodes_[firstStage]->strt_[nodes_[firstStage]->rlo_strt_],nRowInStage_[firstStage]*sizeof(int));
    overall_current_col_offset += nRowInStage_[firstStage]; // Copy first stage rlo
    for (int i = 0; i < number_children; i++ ){
        // We need to fill clo, cup, obj, rlo, rup
        memcpy(elements+overall_current_col_offset,nodes_[secondStage]->dels_+nodes_[secondStage]->strt_[nodes_[secondStage]->rlo_strt_],number_rows*sizeof(double));
        for (int j = 0; j < number_rows; j++){
            indices[overall_current_col_offset+j] = stageRowPtr_[secondStage]+row_offset+j;
        }
        overall_current_col_offset += number_rows;
        row_offset += number_rows;
    }
    start[overall_current_strt_offset] = start[overall_current_strt_offset-1]+number_rows*number_children+nRowInStage_[firstStage]; 
    overall_current_strt_offset++;
    // Row Upper
    row_offset = 0;
    memcpy(elements+overall_current_col_offset,nodes_[firstStage]->dels_+nodes_[firstStage]->strt_[nodes_[firstStage]->rup_strt_],nRowInStage_[firstStage]*sizeof(double));
    memcpy(indices+overall_current_col_offset,nodes_[firstStage]->inds_+nodes_[firstStage]->strt_[nodes_[firstStage]->rup_strt_],nRowInStage_[firstStage]*sizeof(int));
    overall_current_col_offset += nRowInStage_[firstStage]; // Copy first stage rup
    for (int i = 0; i < number_children; i++ ){
        // We need to fill clo, cup, obj, rlo, rup
        memcpy(elements+overall_current_col_offset,nodes_[secondStage]->dels_+nodes_[secondStage]->strt_[nodes_[secondStage]->rup_strt_],number_rows*sizeof(double));
        for (int j = 0; j < number_rows; j++){
            indices[overall_current_col_offset+j] = stageRowPtr_[secondStage]+row_offset+j;
        }
        overall_current_col_offset += number_rows;
        row_offset += number_rows;
    }
    start[overall_current_strt_offset] = start[overall_current_strt_offset-1]+number_rows*number_children+nRowInStage_[firstStage]; 
    overall_current_strt_offset++;
    assert( overall_current_col_offset == nels);
    assert( overall_current_strt_offset == nstrt);
    #pragma endregion fivearrays

   
    // Now the elements, indices and start array are filled.

     // Now comes the scenario tree: We iterate over all the firstStage nodes (siblings) and need to include the information of the direct child nodes in 
    // - second stage nodes 
    // - sibling first-stage nodes

    // Build the stochastic entries for the current node
    int scenario_index = number_children-1; // We start backwards with the scenario counting, courtesy to the tree creation routine!
    for (int cur_scen = number_scenarios-1; cur_scen >= 0;  cur_scen--){
        current_node = first_stage_nodes[cur_scen]; // This means, the node with scen 0 gets the first det-eq values
        // TODO: Put it in own method!

        SmiScnNode* current_scn_node = current_node->getDataPtr();
        SmiNodeData* current_node_data = current_scn_node->getNode();

#pragma region copyElements
        // Find out how many elements we need to store!
        temp_node = current_node->getChild();
        int number_elements = current_node->getDataPtr()->getNode()->nels_;
        while ( temp_node != NULL){
            // Count number of elements and store place for this..
            number_elements += temp_node->getDataPtr()->getNode()->nels_;
            temp_node = temp_node->getSibling();
        }
        double* new_elements = (double *)calloc(number_elements, sizeof(double));
        int* new_indices = (int *) calloc(number_elements, sizeof(int));
        int* new_start = (int *) calloc(nstrt, sizeof(int));
        // Copy the elements, indices and the start arrays for the old second stage nodes
        memcpy(new_elements,current_node_data->dels_,current_node_data->strt_[current_node_data->clo_strt_]*sizeof(double));
        memcpy(new_indices,current_node_data->inds_,current_node_data->strt_[current_node_data->clo_strt_]*sizeof(int));
        memcpy(new_start,current_node_data->strt_,(current_node_data->clo_strt_+1)*sizeof(int));
        if ( !current_node_data->has_matrix_ ){
            // We need to take some precautions, as the current stochastic node has no matrix, but the following node might. So we add the first rows to this node!
            for (int ijk = 0; ijk < nRowInStage_[firstStage]+1; ijk++ )
                new_start[ijk] = 0;
        }
        assert( current_node_data->clo_strt_ == nRowInStage_[firstStage] || current_node_data->clo_strt_ == 0);
        int new_index_offset = current_node_data->strt_[current_node_data->clo_strt_]; // Number of elements
        int new_start_offset = nRowInStage_[firstStage]+1;
        // Now we iterate through all the child nodes of this current_node and copy the matrix elements
        int det_eq_column_offset = 0;
        int det_eq_row_offset = 0;
        // Copy stochastic matrix elements into the new arrays
        temp_node = current_node->getChild();
        while ( temp_node != NULL){
            SmiNodeData* temp_data = temp_node->getDataPtr()->getNode();
            memcpy(new_elements+new_index_offset,temp_data->dels_,temp_data->strt_[temp_data->clo_strt_]*sizeof(double));
            // Indices can not be copied straight away, we have to change them
            for (int i = 0; i < temp_data->strt_[temp_data->getMatEnd()]; i++)
                new_indices[new_index_offset+i] = ( temp_data->inds_[i] < stageColPtr_[secondStage] ) ? temp_data->inds_[i] : temp_data->inds_[i]+det_eq_column_offset;
            for (int i = 0; i < number_rows;i++){
                if ( temp_data->has_matrix_ ) // If our child node has a matrix, we copy the row data, also we copy nothing
                    new_start[new_start_offset+i] = temp_data->strt_[i+1]+new_index_offset;
                else
                    new_start[new_start_offset+i] = new_start[new_start_offset+i-1];
            }
            temp_node = temp_node->getSibling();
            det_eq_column_offset += number_cols;
            det_eq_row_offset += number_rows;
            new_index_offset += temp_data->strt_[temp_data->getMatEnd()];
            new_start_offset += number_rows;
        }
        // We need to adress the effect of unbalanced trees here!
        if ( new_start_offset < nRowInStage_[firstStage]+number_rows*number_children+1 ){
            // Then we have a case where we have less childs than in the other cases!
            for (int current_row = new_start_offset; current_row < nRowInStage_[firstStage]+number_rows*number_children+1; current_row++ ){
                // Create the empty rows
                new_start[current_row] = new_start[current_row-1];
                new_start_offset++;
            }
        }
        int new_clo_strt = new_start_offset-1;


        // Copy stochastic clo arrays
        memcpy(new_elements+new_index_offset,current_node_data->dels_+current_node_data->strt_[current_node_data->clo_strt_],current_node_data->getColLowerLength()*sizeof(double));
        memcpy(new_indices+new_index_offset,current_node_data->inds_+current_node_data->strt_[current_node_data->clo_strt_],current_node_data->getColLowerLength()*sizeof(int));
        new_index_offset += current_node_data->getColLowerLength();
 
        det_eq_column_offset = 0;
        det_eq_row_offset = 0;
        temp_node = current_node->getChild();
        while ( temp_node != NULL){
            SmiNodeData* temp_data = temp_node->getDataPtr()->getNode();
            int index_start = temp_data->strt_[temp_data->clo_strt_];
            int index_length = temp_data->getColLowerLength();
            memcpy(new_elements+new_index_offset,temp_data->dels_+index_start,index_length*sizeof(double));
            // Indices can not be copied straight away, we have to change them
            for (int i = index_start; i < index_start+index_length; i++)
                new_indices[new_index_offset+i-index_start] = temp_data->inds_[i]+det_eq_column_offset;

            temp_node = temp_node->getSibling();
            det_eq_column_offset += number_cols;
            det_eq_row_offset += number_rows;
            new_index_offset += index_length;
        }
        new_start[new_start_offset++] = new_index_offset;
        int new_cup_strt = new_start_offset-1;

        // Copy stochastic cup arrays
        memcpy(new_elements+new_index_offset,current_node_data->dels_+current_node_data->strt_[current_node_data->cup_strt_],current_node_data->getColUpperLength()*sizeof(double));
        memcpy(new_indices+new_index_offset,current_node_data->inds_+current_node_data->strt_[current_node_data->cup_strt_],current_node_data->getColUpperLength()*sizeof(int));
        new_index_offset += current_node_data->getColUpperLength();

        det_eq_column_offset = 0;
        det_eq_row_offset = 0;
        temp_node = current_node->getChild();
        while ( temp_node != NULL){
            SmiNodeData* temp_data = temp_node->getDataPtr()->getNode();
            int index_start = temp_data->strt_[temp_data->cup_strt_];
            int index_length = temp_data->getColUpperLength();
            memcpy(new_elements+new_index_offset,temp_data->dels_+index_start,index_length*sizeof(double));
            // Indices can not be copied straight away, we have to change them
            for (int i = index_start; i < index_start+index_length; i++)
                new_indices[new_index_offset+i-index_start] = temp_data->inds_[i]+det_eq_column_offset;

            temp_node = temp_node->getSibling();
            det_eq_column_offset += number_cols;
            det_eq_row_offset += number_rows;
            new_index_offset += index_length;
        }
        new_start[new_start_offset++] = new_index_offset;
        int new_obj_strt = new_start_offset-1;

        // Copy stochastic obj arrays
        memcpy(new_elements+new_index_offset,current_node_data->dels_+current_node_data->strt_[current_node_data->obj_strt_],current_node_data->getObjectiveLength()*sizeof(double));
        memcpy(new_indices+new_index_offset,current_node_data->inds_+current_node_data->strt_[current_node_data->obj_strt_],current_node_data->getObjectiveLength()*sizeof(int));
        new_index_offset += current_node_data->getObjectiveLength();

        det_eq_column_offset = 0;
        det_eq_row_offset = 0;
        temp_node = current_node->getChild();
        // FInd the probabilities
        std::vector<double> scenario_probs;
        double sum = 0;
        while ( temp_node != NULL){
            scenario_probs.push_back(temp_node->getDataPtr()->getProb());
            sum += scenario_probs.back();
            temp_node = temp_node->getSibling();
        }
        temp_node = current_node->getChild();
        int node_counter = 0;
        // TODO: We NEED to insert stochastic objective function values for EVERY coefficient we have in the core node!
        while ( temp_node != NULL){
            SmiNodeData* temp_data = temp_node->getDataPtr()->getNode();
            // Get the objective length of the core node and stuff
            const double* elems = nodes_[secondStage]->getObjectiveElements();
            const int* inds = nodes_[secondStage]->getObjectiveIndices();
            int obj_length = nodes_[secondStage]->getObjectiveLength();
            // We might need to realloc more space!
            int index_start = temp_data->strt_[temp_data->obj_strt_];
            int index_length = temp_data->getObjectiveLength();
            int difference = obj_length-index_length;
            if ( difference > 0 ){ // Else should not happen anyway
                number_elements += difference;
                // We need to realloc elements and indices arrays a bit
                 new_elements = (double*) realloc (new_elements, number_elements * sizeof(double));
                 new_indices = (int*) realloc (new_indices, number_elements * sizeof(int));
            }
            else 
                assert(false);
            // We need to replace the values of the deterministic problem with the probability weighted ones, and use the stochastic values were needed!
            assert(obj_length==number_cols);
            memcpy(new_elements+new_index_offset,elems,obj_length*sizeof(double));
            int index_counter = 0;
            for (int i = 0; i < obj_length; i++){
                // We need to check if the current index we look at is the same as the stochastic one
                if ( inds[i] == temp_data->inds_[index_start+index_counter] ){
                    // Use stochastic element
                    new_elements[new_index_offset+i] = temp_data->dels_[index_start+index_counter]*( scenario_probs[node_counter]/sum );
                    index_counter++;
                }
                else {
                    // Use deterministic element
                    new_elements[new_index_offset+i] *= scenario_probs[node_counter] / sum; 
                }
                
            }
            assert(index_counter == index_length);
            // Indices can not be copied straight away, we have to change them
            for (int i = 0; i < obj_length; i++)
                new_indices[new_index_offset+i] = inds[i]+det_eq_column_offset;

            temp_node = temp_node->getSibling();
            det_eq_column_offset += number_cols;
            det_eq_row_offset += number_rows;
            new_index_offset += obj_length;
            node_counter++;
            
        }
        assert( node_counter == scenario_probs.size());
        new_start[new_start_offset++] = new_index_offset;
        int new_rlo_strt = new_start_offset-1;

        // Copy stochastic rlo arrays
        memcpy(new_elements+new_index_offset,current_node_data->dels_+current_node_data->strt_[current_node_data->rlo_strt_],current_node_data->getRowLowerLength()*sizeof(double));
        memcpy(new_indices+new_index_offset,current_node_data->inds_+current_node_data->strt_[current_node_data->rlo_strt_],current_node_data->getRowLowerLength()*sizeof(int));
        new_index_offset += current_node_data->getRowLowerLength();

        det_eq_column_offset = 0;
        det_eq_row_offset = 0;
        temp_node = current_node->getChild();
        while ( temp_node != NULL){
            SmiNodeData* temp_data = temp_node->getDataPtr()->getNode();
            int index_start = temp_data->strt_[temp_data->rlo_strt_];
            int index_length = temp_data->getRowLowerLength();
            memcpy(new_elements+new_index_offset,temp_data->dels_+index_start,index_length*sizeof(double));
            // Indices can not be copied straight away, we have to change them
            for (int i = index_start; i < index_start+index_length; i++)
                new_indices[new_index_offset+i-index_start] = temp_data->inds_[i]+det_eq_row_offset;

            temp_node = temp_node->getSibling();
            det_eq_column_offset += number_cols;
            det_eq_row_offset += number_rows;
            new_index_offset += index_length;
        }
        new_start[new_start_offset++] = new_index_offset;
        int new_rup_strt = new_start_offset-1;

        // Copy stochastic rup arrays
        memcpy(new_elements+new_index_offset,current_node_data->dels_+current_node_data->strt_[current_node_data->rup_strt_],current_node_data->getRowUpperLength()*sizeof(double));
        memcpy(new_indices+new_index_offset,current_node_data->inds_+current_node_data->strt_[current_node_data->rup_strt_],current_node_data->getRowUpperLength()*sizeof(int));
        new_index_offset += current_node_data->getRowUpperLength();

        det_eq_column_offset = 0;
        det_eq_row_offset = 0;
        temp_node = current_node->getChild();
        while ( temp_node != NULL){
            SmiNodeData* temp_data = temp_node->getDataPtr()->getNode();
            int index_start = temp_data->strt_[temp_data->rup_strt_];
            int index_length = temp_data->getRowUpperLength();
            memcpy(new_elements+new_index_offset,temp_data->dels_+index_start,index_length*sizeof(double));
            // Indices can not be copied straight away, we have to change them
            for (int i = index_start; i < index_start+index_length; i++)
                new_indices[new_index_offset+i-index_start] = temp_data->inds_[i]+det_eq_row_offset;

            temp_node = temp_node->getSibling();
            det_eq_column_offset += number_cols;
            det_eq_row_offset += number_rows;
            new_index_offset += index_length;
        }
        new_start[new_start_offset++] = new_index_offset;

        // Now we finished the preparation of the stochastic node. We now need to update the node with the new arrays and variables!
        current_node->getDataPtr()->getNode()->clo_strt_ = new_clo_strt;
        current_node->getDataPtr()->getNode()->cup_strt_ = new_cup_strt;
        current_node->getDataPtr()->getNode()->obj_strt_ = new_obj_strt;
        current_node->getDataPtr()->getNode()->rlo_strt_ = new_rlo_strt;
        current_node->getDataPtr()->getNode()->rup_strt_ = new_rup_strt;
        current_node->getDataPtr()->getNode()->ncol_ += number_cols*number_children;
        current_node->getDataPtr()->getNode()->nrow_ += number_rows*number_children;
        current_node->getDataPtr()->getNode()->nels_ = number_elements;
        current_node->getDataPtr()->getNode()->nstrt_ = nstrt;
        current_node->getDataPtr()->getNode()->deleteMemory();
        current_node->getDataPtr()->getNode()->dels_ = new_elements;
        current_node->getDataPtr()->getNode()->inds_ = new_indices;
        current_node->getDataPtr()->getNode()->strt_ = new_start;
        assert( new_rup_strt +2 == nstrt);

        // Delete the child and sibling nodes, recast pointers of their child nodes, if necessary!
        temp_node = current_node->getChild();
        SmiTreeNode<SmiScnNode*>* new_child_node = temp_node->getChild();
        SmiTreeNode<SmiScnNode*>* sibling_node = NULL;
        int num_childs = 0;
        while ( temp_node != NULL ){ // What do we do in this loop? We want to recast all the pointers
            // If we are here, we have third_stage nodes to process
            // SecondStage nodes are merge in the current node, so all the third_stage nodes need to be siblings
            if ( temp_node->hasChild() ){ // We have thirdstage_nodes
                SmiTreeNode<SmiScnNode*>* third_stage_node = temp_node->getChild();
                if ( sibling_node != NULL )
                    sibling_node->setSibling(third_stage_node);
                while ( third_stage_node != NULL){
                    // Reset parent pointer AND Sibling Pointer!!
                    num_childs++;
                    third_stage_node->setParent(current_node);
                    third_stage_node->getDataPtr()->setParent(current_node->getDataPtr());
                    // We have to adapt the stochastic third_stage_nodes to the new det-eq world

                    if ( !third_stage_node->hasSibling() )// If we have no sibling, we need to set the sibling node to the current_node, because we cross node boundaries in the next outer loop
                        sibling_node = third_stage_node;

                    third_stage_node = third_stage_node->getSibling();
                }
                // Set Child Pointer to Null
                temp_node->setChild(NULL);
            }
            // Get Sibling and delete the other node
            temp_node = temp_node->getSibling();
        }
        // Now we have to store the childs childs node
        
        delete current_node->getChild(); // This deletes the childs and siblings of this node.
        current_node->setChild(new_child_node); // we set the child pointer to the new_child_node, which is zero, if we had no thirdstage nodes
        int old_scenario = current_node->scenario();
        // I think we only need to restructure the scenario indices, If we have no thirdstage nodes
        if ( secondStage == nstag_-1 ){
            current_node->setScenario(number_scenarios-cur_scen-1);
            current_node->getDataPtr()->setScenarioIndex(number_scenarios-cur_scen-1);
            temp_node = current_node;
            // Go up the tree to replace scenario indices!
            while ( temp_node->hasParent() ){
                temp_node = temp_node->getParent();
                if ( temp_node->scenario() == old_scenario ){
                    temp_node->setScenario(number_scenarios-cur_scen-1);
                    temp_node->getDataPtr()->setScenarioIndex(temp_node->scenario());
                }
                else
                    break;
            }
            
        }
        current_node->setNumChildren(num_childs);
    } // End while


    // Change the node_data private variables
    nodes_[firstStage]->ncol_ += number_cols*number_children;
    nodes_[firstStage]->nrow_ += number_rows*number_children;
    nodes_[firstStage]->nels_ = nels;
    nodes_[firstStage]->clo_strt_ = new_clo_offset-1;
    nodes_[firstStage]->cup_strt_ = new_clo_offset;
    nodes_[firstStage]->obj_strt_ = new_clo_offset+1;
    nodes_[firstStage]->rlo_strt_ = new_clo_offset+2;
    nodes_[firstStage]->rup_strt_ = new_clo_offset+3;
    nodes_[firstStage]->nstrt_ = new_clo_offset+5;
    nodes_[firstStage]->deleteMemory();
    nodes_[firstStage]->dels_ = elements;
    nodes_[firstStage]->inds_ = indices;
    nodes_[firstStage]->strt_ = start;

#pragma endregion CopyElements

    if ( ! final_stage){
        // Prepare the core node of the stage after secondStage. thirdstagenode
#pragma region thirdstagecore
        // Add the new columns and rows..
        int nels = nodes_[secondStage+1]->nels_ + number_cols*(number_children+1)+number_cols*3+number_cols*2;
        int new_start_overall = nodes_[secondStage+1]->nstrt_+number_cols;
        int new_start_index = nodes_[secondStage+1]->clo_strt_;
        double * new_dels = (double *) calloc(nels, sizeof(double));
        int * new_inds = (int*) calloc(nels,sizeof(int));
        int * new_start = (int*) calloc(new_start_overall,sizeof(int));

        // Memcpy the matrix elements and start
        memcpy(new_dels,nodes_[secondStage+1]->dels_,nodes_[secondStage+1]->strt_[new_start_index]*sizeof(double));

        memcpy(new_start,nodes_[secondStage+1]->strt_,(new_start_index+1)*sizeof(int));

        for (int i = 0; i < nodes_[secondStage+1]->strt_[new_start_index];i++){ // We want to reference the previous secondStage indices, to our new indices, made for this row!
            new_inds[i] = nodes_[secondStage+1]->inds_[i] + number_children*number_cols;//( nodes_[secondStage+1]->inds_[i] < stageColPtr_[secondStage+1] ? add_rows_to_third_stage : add_rows_to_third_stage+number_cols );
        }
        int current_index = nodes_[secondStage+1]->strt_[new_start_index]; // This is an overall index!
        int start_of_third_stage_col_indices = stageColPtr_[secondStage+1]+(number_children-1)*number_cols;
        int start_of_third_stage_row_indices = stageRowPtr_[secondStage+1]+(number_children-1)*number_rows;
        for (int i = 0; i < number_cols; i++){
            // Set the new rows 
            new_inds[current_index] = start_of_third_stage_col_indices+i; 
            new_dels[current_index] = 1;
            current_index++;
            for (int j = 0; j < number_children; j++){
                new_dels[current_index] = 0;
                new_inds[current_index] = stageColPtr_[secondStage]+i+number_cols*j;
                current_index++;
            }
            new_start[new_start_index+1+i] = current_index;
        }
        // We need to set the columnlower/upper/obj/rlo/rupper !
        // Set the start array first! We added current_index many elements, so we have to increase the values by these amounts plus the added columns/rows
        // 
        int new_clo_strt = new_start_index+number_cols;
        int new_cup_strt = new_clo_strt+1;
        int new_obj_strt = new_cup_strt+1;
        int new_rlo_strt = new_obj_strt+1;
        int new_rup_strt = new_rlo_strt+1;
        int end_strt = new_rup_strt+1;
        new_start[new_cup_strt] = new_start[new_clo_strt]+number_cols+nColInStage_[secondStage+1];
        new_start[new_obj_strt] = new_start[new_cup_strt]+number_cols+nColInStage_[secondStage+1];
        new_start[new_rlo_strt] = new_start[new_obj_strt]+number_cols+nColInStage_[secondStage+1];
        new_start[new_rup_strt] = new_start[new_rlo_strt]+number_cols+nRowInStage_[secondStage+1];
        new_start[end_strt] = new_start[new_rup_strt]+number_cols+nRowInStage_[secondStage+1];
        assert(new_start[end_strt] == nels);
        
        // Set the new variables first!
        for (int i = 0; i < number_cols; i++){
            new_inds[new_start[new_clo_strt]+i] = start_of_third_stage_col_indices+i;
            new_inds[new_start[new_cup_strt]+i] = start_of_third_stage_col_indices+i;
            new_inds[new_start[new_obj_strt]+i] = start_of_third_stage_col_indices+i;
            new_inds[new_start[new_rlo_strt]+nRowInStage_[secondStage+1]+i] = start_of_third_stage_row_indices+nRowInStage_[secondStage+1]+i;
            new_inds[new_start[new_rup_strt]+nRowInStage_[secondStage+1]+i] = start_of_third_stage_row_indices+nRowInStage_[secondStage+1]+i;
            new_dels[new_start[new_clo_strt]+i] = -infinity_;
            new_dels[new_start[new_cup_strt]+i] = infinity_;
            new_dels[new_start[new_obj_strt]+i] = 0;
            new_dels[new_start[new_rlo_strt]+nRowInStage_[secondStage+1]+i] = 0;
            new_dels[new_start[new_rup_strt]+nRowInStage_[secondStage+1]+i] = 0;
        }
        // Memcpy the rest
        memcpy(new_dels+new_start[new_clo_strt]+number_cols,nodes_[secondStage+1]->dels_+nodes_[secondStage+1]->strt_[nodes_[secondStage]->clo_strt_],nColInStage_[secondStage+1]*sizeof(double));
        memcpy(new_dels+new_start[new_cup_strt]+number_cols,nodes_[secondStage+1]->dels_+nodes_[secondStage+1]->strt_[nodes_[secondStage]->cup_strt_],nColInStage_[secondStage+1]*sizeof(double));
        memcpy(new_dels+new_start[new_obj_strt]+number_cols,nodes_[secondStage+1]->dels_+nodes_[secondStage+1]->strt_[nodes_[secondStage]->obj_strt_],nColInStage_[secondStage+1]*sizeof(double));
        memcpy(new_dels+new_start[new_rlo_strt],nodes_[secondStage+1]->dels_+nodes_[secondStage+1]->strt_[nodes_[secondStage]->rlo_strt_],nRowInStage_[secondStage+1]*sizeof(double));
        memcpy(new_dels+new_start[new_rup_strt],nodes_[secondStage+1]->dels_+nodes_[secondStage+1]->strt_[nodes_[secondStage]->rup_strt_],nRowInStage_[secondStage+1]*sizeof(double));

        for (int i = 0; i< nColInStage_[secondStage+1]; i++){
            new_inds[new_start[new_clo_strt]+number_cols+i] = start_of_third_stage_col_indices+number_cols+i;
            new_inds[new_start[new_cup_strt]+number_cols+i] = start_of_third_stage_col_indices+number_cols+i;
            new_inds[new_start[new_obj_strt]+number_cols+i] = start_of_third_stage_col_indices+number_cols+i;
        }
        for (int i = 0; i< nRowInStage_[secondStage+1]; i++){
            new_inds[new_start[new_rlo_strt]+i] = start_of_third_stage_row_indices+i;
            new_inds[new_start[new_rup_strt]+i] = start_of_third_stage_row_indices+i;
 
        }
        // Prepare the thirdstage core node
        nodes_[secondStage+1]->ncol_ += number_cols;
        nodes_[secondStage+1]->nrow_ += number_cols;
        nodes_[secondStage+1]->nels_ = nels; //New Matrix entries, column bounds/obj and rowlower/rowuppper
        int matrix_offset = number_cols*number_children;
        nodes_[secondStage+1]->clo_strt_ = new_clo_strt; // The matrix entries
        nodes_[secondStage+1]->cup_strt_ = new_cup_strt;
        nodes_[secondStage+1]->obj_strt_ = new_obj_strt;
        nodes_[secondStage+1]->rlo_strt_ = new_rlo_strt;
        nodes_[secondStage+1]->rup_strt_ = new_rup_strt;
        nodes_[secondStage+1]->nstrt_ = end_strt+1;
        nodes_[secondStage+1]->deleteMemory();
        nodes_[secondStage+1]->dels_ = new_dels;
        nodes_[secondStage+1]->inds_ = new_inds;
        nodes_[secondStage+1]->strt_ = new_start;
        nodes_[secondStage+1]->colbeg_ = start_of_third_stage_col_indices;
        nodes_[secondStage+1]->rowbeg_ = start_of_third_stage_row_indices;

        for (int i = secondStage+2; i < nstag_; i++){
            increaseIndices(nodes_[i],(number_children-1)*number_cols+number_cols, (number_children-1)*number_rows+number_cols);
        }
#pragma endregion thirdstagecore
    }// end if !finalStage
    
    
    // Change the core_ (nStageColPtr,stageColPtr,nColInStage, etc..
    // We reduced the number of stages by one
    
    nstag_--;
    // We also need to set the column and row names, if we want to print it out.
    int new_ncol = 0;
    int new_nrow = 0;
    if ( !final_stage){
        new_ncol = ncol_+(number_children-1)*number_cols+number_cols;
        new_nrow = nrow_+(number_children-1)*number_rows+number_cols;
    }
    else {
        new_ncol = ncol_+(number_children-1)*number_cols;
        new_nrow = nrow_+(number_children-1)*number_rows;
    }

    nColInStage_[firstStage] = nodes_[firstStage]->ncol_;
    nRowInStage_[firstStage] = nodes_[firstStage]->nrow_;
    // We need to delete the firstStage+1 node in any case!
    delete nodes_[firstStage+1];
    for (int i = firstStage+1; i < nstag_; i++){
        nodes_[i] = nodes_[i+1]; // We need to shift the nodes one up
        // And we set the correct stage in the process
        nodes_[i]->stg_ = i;
        nColInStage_[i] = nodes_[i]->ncol_;
        nRowInStage_[i] = nodes_[i]->nrow_;
    }
    // Remove the last node to prevent issues during deletion
    nodes_.pop_back();


    nColInStage_[nstag_] = 0;
    nRowInStage_[nstag_] = 0;
    // Start with the stageColPtr / stageRowPtr
    for (int i = firstStage+1; i < nstag_+1; i++){
        stageColPtr_[i] = stageColPtr_[i-1]+nColInStage_[i-1];
        stageRowPtr_[i] = stageRowPtr_[i-1]+nRowInStage_[i-1];
    }
    // Because this is the number of columns we added to the stage before! ( we moved columns from stage second to stage first)
    // Update colStage/rowStage array, introduce new ones..

    if ( !rowNamesFree.empty() && !colNamesFree.empty()){
        std::vector< std::string > col_names;
        std::vector< std::string > row_names;
        col_names.reserve(new_ncol); row_names.reserve(new_nrow);
        //memcpy before ( and after TODO )
        std::vector<std::string>::iterator it = rowNamesFree.begin();
        std::advance(it,stageRowPtr_[secondStage]-(number_children*number_rows));
        std::copy(rowNamesFree.begin(),it,std::back_inserter(row_names));

        it = colNamesFree.begin();
        std::advance(it,stageColPtr_[secondStage]-(number_children*number_cols));
        std::copy(colNamesFree.begin(),it,std::back_inserter(col_names));

        // Copy the names of the aggregated secondStage indices and add a "det_" in front of it, to distinguish them
        int first_index_of_original_problem_row = stageRowPtr_[secondStage]-number_children*number_rows; // Start of indices of det-eq of second stage
        int first_index_of_original_problem_col = stageColPtr_[secondStage]-number_children*number_cols;
        for (int i = 0; i < number_children; i++){
            for (int j = 0; j < number_cols; j++){
                std::stringstream ss;
                ss << "det_" << i << "_"  << colNamesFree[first_index_of_original_problem_col+j];
                col_names.push_back(ss.str());
            }
            for (int j = 0; j < number_rows; j++){
                std::stringstream ss;
                ss << "det_" << i << "_"  << rowNamesFree[first_index_of_original_problem_row+j];
                row_names.push_back(ss.str());
            }
        }
        if ( !final_stage ) {
            // We need to append the other names..
            for (int i = stageColPtr_[secondStage+1]; i < stageColPtr_[nstag_] ; i++){
                std::stringstream ss;
                ss << "det_later_" << i;
                col_names.push_back(ss.str());
            }
            for (int i = stageRowPtr_[secondStage+1]; i < stageRowPtr_[nstag_] ; i++){
                std::stringstream ss;
                ss << "det_later_" << i;
                row_names.push_back(ss.str());
            }
        }
        // Append the new names for the later stages!
        // TODO::
        // We set the new names
        colNamesFree = col_names;
        rowNamesFree = row_names;

        // Insert all the other strings at later stages (TODO), if we want to aggregate from the beginning
    }
    // We clear the strict strings
    colNamesStrict.clear();
    rowNamesStrict.clear();

    int * new_col_stage = new int [new_ncol];
    int * new_row_stage = new int [new_nrow];
    memcpy(new_col_stage,colStage_,ncol_*sizeof(int));
    memcpy(new_row_stage,rowStage_,nrow_*sizeof(int));
    // Update the remaining ones
    for (int j = firstStage; j < nstag_; j++){
        for (int i = stageColPtr_[j]; i < stageColPtr_[j]+nColInStage_[j]; i++){
            new_col_stage[i] = j;
        }
        for (int i = stageRowPtr_[j]; i < stageRowPtr_[j]+nRowInStage_[j]; i++){
            new_row_stage[i] = j;
        }
    }
    delete [] rowStage_;
    delete [] colStage_;
    rowStage_ = new_row_stage;
    colStage_ = new_col_stage;
    ncol_ = new_ncol;
    nrow_ = new_nrow;

    // Update clo/cup/obj/rhs/rlo/rup arrays..
    for (int i = firstStage; i < nstag_; i++){
        //delete [] cdclo_[i];
        //delete [] cdcup_[i];
        //delete [] cdobj_[i];
        //delete [] cdrlo_[i];
        //delete [] cdrup_[i];
        SmiNodeData * node = nodes_[i];
        int nrow=this->getNumRows(i);
        int ncol=this->getNumCols(i);
        int irow=this->getRowStart(i);
        int icol=this->getColStart(i);
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

    smiTree.repopulateLeafNodeSet(); // Repopulate leaf set and reset tree depth

    // Iterate over node set to set the stg_ of the node_data to depth_ of the tree_node
    SmiTreeNode<SmiScnNode*>* another_node = smiTree.getRoot()->getChild();
    bool goDown = true;
    int current_depth = 1;
    while ( another_node != NULL ){ // Walk through the whole tree and set the scenario to the tree depth_
        assert( another_node->depth() == current_depth);
        if ( another_node->hasChild() && goDown ){
            another_node = another_node->getChild();
            current_depth++;
        }
        else if( goDown ){
            another_node->getDataPtr()->getNode()->stg_ = another_node->depth();
            // Assert some things!
            assert( another_node->getDataPtr()->getNode()->nrow_ == nodes_[another_node->depth()]->nrow_ );
            assert( another_node->getDataPtr()->getNode()->ncol_ == nodes_[another_node->depth()]->ncol_ );
            goDown = false;
            if ( another_node->hasSibling() ){
                goDown = true;
                another_node = another_node->getSibling();
            }
            else{
                another_node = another_node->getParent();
                current_depth--;
            }
            
        }
        else {// goDown is false {
            another_node->getDataPtr()->getNode()->stg_ = another_node->depth();
            // Assert some things!
            assert( another_node->getDataPtr()->getNode()->nrow_ == nodes_[another_node->depth()]->nrow_ );
            assert( another_node->getDataPtr()->getNode()->ncol_ == nodes_[another_node->depth()]->ncol_ );
            
            if ( another_node->hasSibling() ){
                goDown = true;
                another_node = another_node->getSibling();
            }
            else{
                another_node = another_node->getParent();
                current_depth--;
            }
        }
    }

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
    // Return excess memory to heap. This should only be the case if we have no matrix!
    if ( has_matrix_ ){
        assert( this->nstrt_ == i_start+1);
    }
    else {
        this->nstrt_ = i_start+1;
        temp_ptr = realloc(this->strt_,nstrt_*sizeof(int));
        this->strt_ = (int*) temp_ptr;
    }
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
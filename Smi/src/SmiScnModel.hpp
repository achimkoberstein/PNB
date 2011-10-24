// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef SmiScnModel_HPP
#define SmiScnModel_HPP

#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif


// Include files
#include "SmiDiscreteDistribution.hpp"
#include "SmiBlockStructure.hpp"
#include "SmiScenarioTree.hpp"
#include "SmiScnData.hpp"
#include "SmiCoreCombineRule.hpp"
#include "OsiSolverInterface.hpp"
#include "CoinPackedVector.hpp"
#include "SmiMessage.hpp"

// STL declarations
#include <map>
#include <vector>
using namespace std;

// forward declaration of SmiScnNode
class SmiScnNode;


//#############################################################################

/** SmiScnModel: COIN-SMI Scenario Model Class

Concrete class for generating scenario stochastic linear programs.

This class implements the Scenarios format of the Stochastic MPS
modeling system (TODO: web pointer).  Core data and Scenarios data
can be passed using COIN/OSI structures, or can be read from SMPS
formatted files.

Typical driver fragment looks like this
\code
SmiScnModel smi;
smi.readSmps("app0110R");
smi.setOsiSolverHandle(new OsiClpSolverInterface());
OsiSolverInterface *osiStoch = smi.loadOsiSolverData();
osiStoch->initialSolve();
\endcode

The setOsiSolverHandle method allows the user to pass in any OSI
compatible solver.

*/
class SmiScnModel
{
    friend void SmiScnModelDiscreteUnitTest();
    friend void DecompUnitTest();
public:

    /**Read SMPS files.

    There should be three files: {name}.[core, time, stoch].
    If you have different extension conventions, then you can
    hack the method yourself.
    The files can be compressed. The object that reads the files is
    derived from CoinMpsIO.

    The optional argument SmiCoreCombineRule allows user to pass in
    a class to override the default methods to combine core and stochastic data.

    */
    int readSmps(const char *name,
        SmiCoreCombineRule *r=NULL);
    int readSmps(const char *coreFile, const char* timeFile, const char* stochFile,SmiCoreCombineRule *r=NULL);

    /** Writes SMPS files.
    This method generates three files {name}.[core, time, stoch] or {name}.[cor, tim, sto] (see second parameter).
    @param name The name for the model and the written files
    @param winFileExtensions optional; false by default, so extensions will be [core, time, stoch]. With this parameter set to true, it will be [cor, tim, sto].
    @param strictFormat optional, true by default. Set to false if SMPS files should be written in free format.
    @return -1 in case of no existing SMI model, otherwise 0
    */
    int writeSmps(const char *name, bool winFileExtensions = false, bool strictFormat = true);

    /** Get a pointer to the core node.
    @return The pointer to the core node.
    */
    SmiCoreData * getCore() {return core_;}

    /**@name Direct methods.

    Direct methods require the user to create instances of
    Core data and Scenario data.
    Currently, the dimension of the core nodes determines the
    dimension of the scenario nodes, but this is something that
    could easily be changed.
    */

    /** Processes SmiBlockBlockStrctures into scenarios
    @param A pointer to an SmiBlockStructure
    */
	void processBlockStructureIntoScenarios(SmiBlockStructure *smiBS);

    /** Generate scenarios from discrete distribution
    @param s a pointer to an SmiDiscreteDistribution
    @param test optional, false by default; set to true if the generation should be tested
    */
    void processDiscreteDistributionIntoScenarios(SmiDiscreteDistribution *s, bool test=false);

    void processDiscreteDistributionIntoBlocks(SmiDiscreteDistribution *smiDD, SmiBlockStructure * smiBS);

    /** Set the model probabilty, the sum of the probabilities of all scenarios.
    @param p the total probability
    */
    void setModelProb(double p) {totalProb_=p; }

    /** Adds an SmiScnNode to the model object.
    @param smiScnNode a pointer to the SmiScnNode
    */
    int addNodeToSubmodel(SmiScnNode * smiScnNode);

    /** generate scenario with ancestor/branch node identification

    Core argument must be supplied.
    Data values combine with corresponding core values,
    if found, or creates them if not.

    Scenario nodes need to have same dimensions as core nodes.

    Data field arguments can be NULL, or empty.

    branch, anc, arguments must be supplied.  These
    identify the branching node according to the Stochastic MPS
    standard.

    prob is unconditional probability of scenario

    */
    SmiScenarioIndex generateScenario(SmiCoreData *core,
        CoinPackedMatrix *matrix,
        CoinPackedVector *dclo, CoinPackedVector *dcup,
        CoinPackedVector *dobj,
        CoinPackedVector *drlo, CoinPackedVector *drup,
        SmiStageIndex branch, SmiScenarioIndex anc, double prob,
        SmiCoreCombineRule *r = SmiCoreCombineReplace::Instance());

    /** generate scenario with labels information

    Core argument must be supplied.
    Data values combine with corresponding core values,
    if found, or creates them if not.

    Scenario nodes need to have same dimensions as core nodes.

    Data field arguments can be NULL, or empty.

    Labels are passed as vector<int> array.
    Adds new path using labels to find branching node.
    The depth (root to leaf) of new path is labels.size().

    */
    SmiScenarioIndex generateScenario(SmiCoreData *core,
        CoinPackedMatrix *matrix,
        CoinPackedVector *dclo, CoinPackedVector *dcup,
        CoinPackedVector *dobj,
        CoinPackedVector *drlo, CoinPackedVector *drup,
        vector<int>labels, double prob,
        SmiCoreCombineRule *r = SmiCoreCombineReplace::Instance());

    /**@name loadOsiSolverData

    Loads deterministic equivalent model into internal osi data structures
    and return handle.

    Note: this uses a callback class SmiCoreCombineRule to decide how to combine
    the core and stochastic data. The user can override the default
    methods when the scenario is generated (see SmiScnModel::generateScenario)
    or when SMPS files are processed (see SmiScnModel::readSmps).

    */
    OsiSolverInterface * loadOsiSolverData();
    OsiSolverInterface * loadOsiSolverDataForSubproblem(int stage, int scenStart);

    /** Solves the Wait-And-See-Problem, with objective sense of 1 (= minimization) for default.
    @param osiSolver a pointer to an OsiSolverInterface
    @param objSense optional, the objective sense (1 for minimization, -1 for optimization)
    @return a vector that containts for every scenario a pair of values, namely the objective value and the probability of the the scenario
    */
    std::vector< std::pair<double,double> > solveWS(OsiSolverInterface *osiSolver, double objSense = 1);
    
    /** Solves the Expected-Value-Problem, with objective sense of 1 (= minimization) for default.
    @param osiSolver a pointer to an OsiSolverInterface
    @param objSense optional, the objective sense (1 for minimization, -1 for optimization)
    @param calculateAverageValues optional, when true (default) the average values for the random variables are calculated; 
        otherwise, it is expected that the model already contains the average values
    @return a pair of the objective value and a pointer to the column solution
    */
    std::pair<double,double*> solveEV(OsiSolverInterface *osiSolver, double objSense = 1, bool calculateAverageValues = true);
    
    /** Solves the EEV-Problem wich determines the expected value of the solution that provides 
    the average scenario problem, with objective sense of 1 (= minimization) for default.
    @param osiSolver a pointer to an OsiSolverInterface
    @param objSense optional, the objective sense (1 for minimization, -1 for optimization)
    @param calculateAverageValues optional, when true (default) the average values for the random variables are calculated; 
        otherwise, it is expected that the model already contains the average values
    @return the objective value of the solved EEVP
    */
    double solveEEV(OsiSolverInterface *osiSolver, double objSense = 1, bool calculateAverageValues = true);

    /** Returns only the solution value of the Wait-And-See-Problem, with objective sense of 1 (= minimization) for default.
    @param osiSolver a pointer to an OsiSolverInterface
    @param objSense optional, the objective sense (1 for minimization, -1 for optimization)
    @return the solution value of the WS-Problem
    @see solveWS(OsiSolverInterface *osiSolver, double objSense = 1);
    */
    double getWSValue(OsiSolverInterface *osiSolver, double objSense = 1);

    /** Returns only the solution value of the Expected-Value-Problem, with objective sense of 1 (= minimization) for default.
    @param osiSolver a pointer to an OsiSolverInterface
    @param objSense optional, the objective sense (1 for minimization, -1 for optimization)
    @return the solution value of the EVP
    @see solveEV(OsiSolverInterface *osiSolver, double objSense = 1, bool calculateAverageValues = true);
    */
    double getEVValue(OsiSolverInterface* osiSolver, double objSense = 1, bool calculateAverageValues = true);
    
    /** Returns only the solution value of the EEVP, with objective sense of 1 (= minimization) for default.
    @param osiSolver a pointer to an OsiSolverInterface
    @param objSense optional, the objective sense (1 for minimization, -1 for optimization)
    @return the solution value of the EEVP
    @see solveEEV(OsiSolverInterface *osiSolver, double objSense = 1, bool calculateAverageValues = true);
    */
    double getEEVValue(OsiSolverInterface* osiSolver, double objSense = 1, bool calculateAverageValues = true);

    /** Let SmiScnModel own core data
    @param val a pointer to an SmiCoreData object
    */
    void setCore(SmiCoreData * val) { core_ = val; }

    // get scenario problem data
    SmiScenarioIndex getNumScenarios(){ return smiTree_.getNumScenarios();} /// returns the number of scenarios
    double getScenarioProb(SmiScenarioIndex ns); // returns the probabilty of a specific scenario
    SmiScnNode * getLeafNode(SmiScenarioIndex i){ return smiTree_.getLeaf(i)->getDataPtr(); } /// returns the leaf node of a specific scenario
    SmiScnNode * getRootNode(){ return smiTree_.getRoot()->getDataPtr(); } /// returns the root node of a specific scenario

    const int* getIntegerInd(){ return core_->getIntegerIndices();} /// returns the indices of integer variables in the core node
    int getIntegerLen(){return core_->getIntegerLength(); } /// return the number of integer variables in the core node
    const int* getBinaryInd(){return core_->getBinaryIndices();} /// returns the indices of binary variables in the core node
    int getBinaryLen(){return core_->getBinaryLength();} /// return the number of integer variables in the core node

    inline vector<int> getIntIndices() {return intIndices;} /// return the indices of integer variables in this smi model
    inline void addIntIndice(int indice) {intIndices.push_back(indice);} /// adds an indice to the list of intgeger variables in this smi model

    // getXXX by scenario
    double getObjectiveValue(SmiScenarioIndex ns);  /// returns the objective value of scenario ns
    double *getColSolution(SmiScenarioIndex ns, int *length); /// returns a pointer to the column solution of scenario ns. the parameter length is an out-parameter and specifies the number of columns
    double *getRowSolution(SmiScenarioIndex ns, int *length); /// returns a pointer to the row solution of scenario ns. the parameter length is an out-parameter and specifies the number of rows
    double getColSolution(SmiScenarioIndex ns, int stage, int colIndex); /// return the column solution solution of column colIndex in a specific scenario and stage 
    double getRowSolution(SmiScenarioIndex ns, int stage, int colIndex); /// return the row solution solution of column colIndex in a specific scenario and stage 

    // OsiSolverInterface
    /** Sets the OsiSolverInterface for SMI.
    @param osi an OsiSolverInterface
    */
    void setOsiSolverHandle(const OsiSolverInterface &osi)
    {
        delete osiStoch_;
        osiStoch_ = osi.clone(false);
    }
    
    /** Sets the OsiSolverInterface for SMI.
    @param osi an OsiSolverInterface
    */
    void setOsiSolverHandle(const OsiSolverInterface *osi)
    {
        delete osiStoch_;
        osiStoch_ = osi->clone(false); // THis method does not work so well for cplex solver, instead we should just copy an empty model into the solver interface
    }
    
    /** Returns the OsiSolverInterface of this SMI model.
    @return a pointer to the OsiSolverInterface
    */
    OsiSolverInterface * getOsiSolverInterface();

    inline void releaseSolver() {osiStoch_=NULL;} /// Releases the solver. Use it if you want to delete the SmiScnModel but keep the OsiSolverInterface
    inline void releaseCore() { core_=NULL; } /// Releases the core node. Use it if you want to delete the SmiScnModel but keep the SmiCoreData

    void transformToStaircaseStructure( std::map<int,int>& coefficients );


    // constructor: Lesson from Effective C++: Initialize values in the same order as declared in .hpp file.
    SmiScnModel():
    handler_(NULL),messages_(NULL),osiStoch_(NULL), nrow_(0), ncol_(0), nels_(0),nels_max(0),
        drlo_(NULL), drup_(NULL), dobj_(NULL), dclo_(NULL), dcup_(NULL), matrix_(NULL),
        dels_(NULL),indx_(NULL),rstrt_(NULL),minrow_(0),
        solve_synch_(false),totalProb_(0),core_(NULL),smiTree_(),integerInd(NULL),integerLen(0),binaryInd(NULL),binaryLen(0),intIndices(),maxNelsPerScenInStage(NULL)
    { }

    // destructor
    ~SmiScnModel();

    // TODO: i think the following methods should be private
    /** Adds data of the SmiScnNode node to the big determinstic equivalent matrix. If the parameter notDetEq is set to false (default)
    the coefficients of this node get multiplied with the the probability for this node.
    @param node a pointer the SmiScnNode
    @param notDetEq if the coefficients should weighted with the probability (false) or not (true)
    */
    void addNode(SmiScnNode *node, bool notDetEq = false);
    
    /** Deletes the SmiScnNode tnode.
    @param tnode the node that will be deleted
    */
    void deleteNode(SmiScnNode *tnode);
    
    /** Adds the data of the given SmiNodeData node, wich is expected to be are core node, to the problem. This mehtod is used for generating the EVP.
    @param node the core node wich data will be added
    */
    void addCoreNode(SmiNodeData *node);
    
    /** Adds the data of the given SmiNodeData node to the problem when building the EV-Problem; only used when the average values are recalculated.
    @param node the node that containts the data
    */
    void addNodeEVP(SmiScnNode *node);
    
    /** Returns a pointer to the full SMI tree.
    @return the SMI tree
    */
    inline SmiScenarioTree<SmiScnNode *> * getSmiTree() { return &smiTree_; }
    void aggregateStages( int firstStage, int secondStage, bool aggregateFromStart = true );

private:
    CoinMessageHandler *handler_;
    SmiMessage *messages_;

    // internal clone of user declared OSI
    OsiSolverInterface * osiStoch_;
    // model statistics useful for predefining size of structures
    int nrow_;
    int ncol_;
    int nels_;
    int nels_max;
    // data pointers used in AddNode and loadOsiSolverData aka generateDetEq
    double *drlo_;
    double *drup_;
    double *dobj_;
    double *dclo_;
    double *dcup_;
    CoinPackedMatrix *matrix_;
    double *dels_;
    int    *indx_;
    int    *rstrt_;
    // number of scenarios
    //	int scen_;
    // not sure if this is used
    int minrow_;
    // not sure if this is used
    bool solve_synch_;
    // total probability of added scenarios; used to normalize probability to one
    double totalProb_;
    //core model --- used for discrete distributions
    SmiCoreData * core_;
    // scenario tree pointer
    SmiScenarioTree<SmiScnNode *> smiTree_;

    int* integerInd;
    int integerLen;
    int* binaryInd;
    int binaryLen;

    vector<int> intIndices;
    int* maxNelsPerScenInStage;
};

class SmiScnNode
{
    friend class SmiScnModel;
    friend void DecompUnitTest();
public:
    /** For a given indice i of a column in this node, the method returns the index of this column in the core node wich corresponds the original (user's) ordering.
    @param i the index of the column in this node.
    @return the index of the column in the core node.
    */
    int getCoreColIndex(int i);
    
    /** For a given indice i of a row in this node, the method returns the index of this row in the core node wich corresponds the original (user's) ordering.
    @param i the index of the row in this node.
    @return the index of the row in the core node.
    */
    int getCoreRowIndex(int i);
    
    /** Sets the scenario index of this node to a specific value.
    @param i the scenario index
    */
    inline void setScenarioIndex(SmiScenarioIndex i){ scen_=i;}
    
    /** Returns the scenario index of this node.
    @return the scenario index.
    */
    inline SmiScenarioIndex getScenarioIndex() {return scen_;}
    
    /** Return the starting index for columns in the OSI model. 
    @return the starting index in the OSI model
    */
    inline int  getColStart() {return coffset_;}
    
    /** Return the starting index for rows in the OSI model. 
    @return the starting index in the OSI model
    */
    inline int  getRowStart() {return roffset_;}
    
    /** Returns the number of columns.
    @return the number of columns
    */
    inline int getNumCols(){ return node_->getCore()->getNumCols(node_->getStage());}
    
    /** Returns the number of rows.
    @return the number of rows
    */
    inline int getNumRows(){ return node_->getCore()->getNumRows(node_->getStage());}
    
    //TODO: why is this method commented out?
    //inline int getNumElements() { return node_->getCore()->getNumElements(node_>getStage());}
    
    /** Returns the normalized probability. 
    For leaf nodes, this equal to the probability of the whole scenario.
    If the total probability (the sum of all absolute probabilities) is euqal to one, this method returns the same value as getProb().
    @return the normalized probability.
    */
    inline double getModelProb(){return mdl_prob_;}
    
    /** Returns the stage index of this node.
    @return the stage index
    */
    inline SmiStageIndex getStage() { return node_->getStage(); }
    
    /** Returns the parent node.
    @return a pointer to the parent node.
    */
    inline SmiScnNode * getParent(){ return parent_;}
    
    /** Sets the probability for this node.
    @param p the probability
    */
    inline void setProb(double p){prob_=p;}
    
    /** Increases the probability for this node by the value of prob.
    @param prob the probability to be added.
    */
    inline double addProb(double prob){ return prob_+=prob;}
    
    /** Returns the probability of this node.
    @return the probability
    */
    inline double getProb() { return prob_; }
    
    /** Sets the parent of this node.
    @param g the parent node
    */
    inline void setParent(SmiScnNode * g) {parent_=g;}

    /** Returns the SmiNodeData object of this node.
    @return a pointer to the SmiNodeData
    @see SmiNodeData
    */
    inline SmiNodeData *getNode() {return node_;}

    //TODO: what are the following methods for?
    inline bool isVirtualNode() { return !include_; }
    // So can delete root node
    inline void zapNode() {node_=NULL;}

    /** Constructs a new SmiScnNode wich holds the given SmiNodeData object.
    @param node a pointer to the SmiNodeData object to be associated with the SmiScnNode
    */
    inline SmiScnNode(SmiNodeData *node) 
    {
        node_=node;
        node->addPtr();
        prob_=0;
        condProb_ = 0;
        parent_=NULL;
        scen_=-1;
        include_=true;
    }
    
    /** Destructor. Deletes the associated SmiNodeData object only if its not a core node.
    */
    ~SmiScnNode()
    {
        if (node_)
        {
            ////Only delete non-core nodes. Core nodes get deleted via SmiCoreData..
            //if (!node_->isCoreNode()){
            //    delete node_;
            //    node_= NULL;
            //}
            if ( node_->deletePtr() == 0 )
                delete node_;
       	}
	}

    

private:
    /** Sets the starting index for the rows with respect to the OSI model. 
    @return the starting index 
    */
    inline void setRowOffset(int r) {roffset_ = r;}
    
    /** Sets the starting index for the columns with respect to the OSI model. 
    @return the starting index 
    */
    inline void setColOffset(int c) {coffset_ = c;}

    //TODO: the following 2 methods are never used - do we need them?
    inline double getCondProb(){return condProb_;}
    inline void setCondProb(double p){condProb_=p;}
    
    /** Sets the normalized probability. For leaf nodes, this equal to the probability of the whole scenario.
    @param p the normalized probability.
    */
    inline void setModelProb(double p){mdl_prob_=p;}
private:

    //TODO: include? i have no idea what's this for
    inline void setIncludeOff() { include_=false; }
    inline void setIncludeOn()  { include_=true; }
    inline bool getInclude() { return include_; }



private:
    SmiNodeData *node_;
    SmiScnNode *parent_;
    double prob_;
    double mdl_prob_;
    double condProb_;
    int coffset_;
    int roffset_;
    SmiScenarioIndex scen_;
    bool include_;
};


/** This class is used for adding multiple SmiScnNode's in a loop to the SmiScnModel object.
*/
class SmiScnModelAddNode{
public:
    void operator() (SmiScnNode *node)
    {
        s_->addNode(node);
    }

    SmiScnModelAddNode(SmiScnModel *s) { s_ = s;}
private:
    SmiScnModel *s_;


};

/** This class is used for deleting multiple SmiScnNode's in a loop that are stored in the SmiScnModel object.
*/
class SmiScnModelDeleteNode{
public:
    void operator() (SmiScnNode *node)
    {
        s_->deleteNode(node);
    }

    SmiScnModelDeleteNode(SmiScnModel *s) { s_ = s;}
private:
    SmiScnModel *s_;


};

#endif //#define SmiScnModel_HPP

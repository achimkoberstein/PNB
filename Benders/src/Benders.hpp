#ifndef Benders_HPP
#define Benders_HPP

#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif

#include "OsiCpxSolverInterface.hpp"

using namespace std;

// Forward declaration
class OsiStochasticBendersTwoStageSolverInterface;
class SmiScnModel;

class Benders{
    friend class OsiStochasticBendersTwoStageSolverInterface;

private:

	// Solver interface for master problem
	OsiSolverInterface* osiMaster_;
	// Solver interface for subproblems
	OsiSolverInterface* osiSubproblem_;
	// Solver interface for determinstic equivalent
	OsiSolverInterface* osiDetEq_;

	// Infinity
	double infty_;
	// Value for infinity test
	double INFTYTEST;
	// Value for zero test
	double ZEROTEST;

	// Boolean to indicate single cut or multi cut
	bool single_;
	// Counter; if single cut is chosen, it is initialized by 1, if 
	// multi cut is chosen, it is initialized by number of scenarios
	int zSub_;
	// Lower Bound for the problem
	double LB_;
	// Upper Bound for the problem
	double UB_;
	// Number of iterations of benders decomposition
	int nIter_;

	// Master problem:
	// Number of rows - Master
	int mnrow_;
	// Number of variables - Master
	int mncol_;
	// Coefficients of objective function - Master
	double* mdobj_;
	// Coefficients of LHS - Master
	double* mdrlo_;
	// Coefficients of RHS - Master
	double* mdrup_;
	// Coefficients of lower bound on variables - Master
	double* mdclo_;
	// Coefficients of upper bound on variables - Master
	double* mdcup_;

	// Matrix - Master
	// Number of matrix elements
	int mnels_;	
	// Row length
	int * mlen_;
	// Coefficients of matrix
	double* mdels_;
	// Indices of matrix elements
	int* miels_;
	// Row start 
	int* mrstrt_;	

	// Solution vector; contains the best feasible solution of stage 1 
	// and stage 2 variables found during the method
	double* bestSol_;
    double* bestSolTemp_; // Contains current stage 1 solution only

	// Subproblems:
	// Number of rows - Subproblem
	int snrow_;
	// Number of variables - Subproblem
	int sncol_;
	// Coefficients of objective function - Subproblem
	double* sdobj_;
	// Coefficients of LHS - Subproblem
	double* sdrlo_;
	// Coefficients of RHS - Subproblem
	double* sdrup_;
	// Coefficients of lower bound on variables - Subproblem
	double* sdclo_;
	// Coefficients of lower bound on variables - Subproblem
	double* sdcup_;

	// Matrix - Subproblem
	// Number of stage 1 coefficients
	int snSt1_;			
	// Coefficients of stage 1 variables
	double *sSt1dels_;	
	// Indices of stage 1 matrix elements
	int *sSt1iels_;	
	// Row start array of stage 1 variables
	int *sSt1rstrt_;
	// Number of stage 1 coefficients per row
	int *snSt1len_;
	// Number of stage 2 coefficients in matrix
	int snSt2_;		
	// Coefficients of stage 2 variables
	double *sSt2dels_;
	// Indices of stage 2 matrix elements
	int *sSt2iels_;		
	// Row start array of stage 2 variables
	int *sSt2rstrt_;
	// Number of stage 2 coefficients per row
	int *snSt2len_;

	//Stochastic data:
	// Number of scenarios
	int nScen_;
	// Current upper bound for master problem
	double upperBound_;
	// Probability for each scenario
	double *scenProb_;
	// Objective function - stochastic
	// Length of objective function
	int mutlobj_;
	// Indices of objective function for all scenarios
	int *mutiobj_;
	// Coefficients of objective function for all scenarios
	double *mutdobj_;
	// Start array of objective function; shows on which position scenario k begins
	int *scenObjStart_;
	// LHS - stochastic
	// Length of LHS
	int mutlrlo_;
	// Indices of LHS for all scenarios
	int *mutirlo_;
	// Coefficients of LHS for all scenarios
	double *mutdrlo_;
	// Start array of LHS; shows on which position scenario k begins 
	int *scenRloStart_;
	// RHS - stochastic
	// Length of RHS
	int mutlrup_;
	// Indices of RHS for all scenarios
	int *mutirup_;
	// Coefficients of RHS for all scenarios
	double *mutdrup_;
	// Start array of RHS; shows on which position scenario k begins 
	int *scenRupStart_;
	// Upper bounds - stochastic
	// Length of upper bounds
	int mutlcup_;
	// Indices of upper bounds for all scenarios
	int *muticup_;
	// Coefficients of upper bounds for all scenarios
	double *mutdcup_;
	// Start array of upper bounds; shows on which position scenario k begins 
	int *scenCupStart_;
	// Lower bounds - stochastic
	// Length of lower bounds
	int mutlclo_;
	// Indices of lower bounds for all scenarios
	int *muticlo_;
	// Coefficients of lower bounds for all scenarios
	double *mutdclo_;
	// Start array of lower bounds; shows on which position scenario k begins 
	int *scenCloStart_;

	// Matrix - stochastic
	// Number of stage 1 coefficients in all scenarios
	int mutnSt1_;
	// Coefficients of stage 1
	double *mutSt1dels_;
	// Indices of stage 1
	int *mutSt1iels_;
	// Row start array for stage 1
	int *mutSt1rstrt_;
	// Scenario start array for stage 1
	int *scenMatSt1Start_;
	// Number of stage 1 coefficients per row
	int *mutnSt1len_;
	// Number of stage 2 coefficients in all scenarios
	int mutnSt2_;
	// Coefficients of stage 2
	double *mutSt2dels_;
	// Indices of stage 2
	int *mutSt2iels_;
	// Row start array for stage 2
	int *mutSt2rstrt_;
	// Scenario start array for stage 2
	int *scenMatSt2Start_;
	// Number of stage 2 coefficients per row
	int *mutnSt2len_;

	// Combined data
	// Number of stage 2 coefficients
	int combSt2nels_;
	// Coefficients of stage 2
	double* combSt2dels_; 
	// Indices of stage 2
	int*  combSt2iels_;
	// Row start array of stage 2
	int* combSt2rstrt_;
	// Number of stage 2 coefficients per row
	int* combSt2len_; 
	// Coefficients of objective function
	double* combdobj_;
	// Coefficients of RHS
	double* combdrup_;
	// Coefficients of LHS
	double* combdrlo_;
	// Coefficients of upper bounds
	double* combdcup_;
	// Coefficients of lower bounds
	double* combdclo_;

	// LHS of stage 2 is saved; is needed to calculate the cuts
	double* constDrlo_;
	// RHS of stage 2 is saved; is needed to calculate the cuts
	double* constDrup_;

	// Translation - if there is either a lower or an upper bound
	// on a stage 2 variable which is not equal to zero, it is
	// converted to zero, therefore a translation vector is
	// needed. If there are both lower and upper bound on a stage 
	// 2 variable, then the upper bound is added to the matrix as 
	// another constraint and the lower bound is converted to zero.
	// Coefficients of translation vector
	double* dTrans_;
	// Indices of translation vector
	int* iTrans_;
	// Number of stage 2 variables which have a bound not equal to zero 
	int lTrans_;
	// Number of new constraints which are added to combined matrix
	int rowAdd_;
	// Sum of all coefficients of the translation vector, are added
	// to the objective functions of master and subproblems
	double objTrans_;

	// Optimality cut
	// Coefficients of cut elements
	double* cutElements_;
	// Indices of cut elements
	int * cutIndex_;
	// Cut RHS
	double* cutRowUB_;

	// Integer variables
	// Number of stage 1 integer variables 
	int mnInt_;
	// Indices of stage 1 integer variables
	int *miInt_;
	// Number of stage 2 integer variables
	int snInt_;
	// Indices of stage 2 integer variables
	int *siInt_;

	// Binary variables
	// Number of stage 1 binary variables
	int mnBin_;
	// Indices of stage 1 binary variables
	int *miBin_;
	// Number of stage 2 binary variables
	int snBin_;
	// Indices of stage 2 binary variables
	int *siBin_;

	// Trust Region informations
	// Boolean to indicate if trust region method
	// is used
	bool trustRegion_;
	// Delta for Trust Region
	double tRdelta_;
	// Delta rate for Trust Region
	double tRrateD_;
	// Rate for lower bound for Trust Region
	double tRrateLB_;

	// Boolean to indicate if knapsack inequalities
	// are used
	bool knapsack_;

	// Boolean to indicate if warmstart informations
	// are used
	// Warmstart informations between iterations
	bool warmStart_;
	// Warmstart informations between subproblems
	bool warmStartFol_;
	// Booleans to indicate, if warmstart informations
	// are available
	bool warmStartEmptyMaster_;
	bool *warmStartEmptySub_;
	// Warm start informations for master problem
	CoinWarmStart *warmStartMaster_;
	// Warm start informations for each subproblems
	CoinWarmStart **warmStartSub_;
	// Warmstart informations for one subproblem which are
	// given to the following subproblem
	CoinWarmStart *warmStartSubFol_;
	
	// This part is only needed for determinisitc equivalent
	// Number of deterministic coefficients
	int snels_;
	// Deterministic coefficients
	double* sdels_;
	// Indices of deterministic coefficients
	int* siels_;
	// Start array for deterministic matrix
	int* srstrt_;
	// Row length for determinstic matrix
	int* slen_;

	// Number of stochastic coefficients
	int mutnels_;
	// Stochastic coefficients
	double* mutdels_;
	// Indices of stochastic coefficients
	int* mutiels_;
	// Start array for stochastic matrix
	int* mutrstrt_;
	// Row length of stochastic matrix
	int* mutlen_;
	// Scenario start array for stochastic matrix
	int* scenMatStart_;

	// Number of combined coefficients
	int combnels_;
	// Combined coefficients
	double* combdels_; 
	// Indices of combined coefficients
	int*  combiels_;
	// Row start array of combined matrix
	int* combrstrt_;
	// Row length of combined matrix
	int* comblen_; 

	// Objective value
	double objDetEq_;
    double subValue_;
    std::vector<CoinPackedVector> feasCutVector;
    double epsilonFeasCut_;
    double masterValue_;

    // applied optimality cuts
    int appliedCuts_;

    // Regularized decomposition //
    bool useRegularizedDecomposition_; // this goes together with the multi-cut variant

    // regularizing parameter sigma
    double sigma_;

    // Solution values of the regularizing term
    double* regSolution_; 

    // objective function value of regularized solution 
    double regSolutionValue_;
    double* regSubproblemSolutionValues_;

    // store rows in master problem used for reg. decomposition
    int startRows_;
    int endRows_;

    // indicate if we make a serious step
    bool seriousStep_;
    bool computeRegularizedSubproblems_;

    // indiciate if we should change sigma
    bool changeSigma_; // seriousStep && changeSigma => increase, !seriousStep && changeSigma => decrease
    bool checkCuts_;

    /// Fischetti-Paper like computation of feasible or optimality cuts..
    bool fischetti_;

    // to which digit we want to round ? If precision is 1, we round 6.7 to 7. If it is 2 we round 6.78 to 6.8
    int precision_; 

public:
	// Constructor
	Benders():
	  
	mnrow_(0), mncol_(0), mdobj_(NULL), mdrlo_(NULL), mdrup_(NULL), 
	mdclo_(NULL), mdcup_(NULL), mnels_(0), mlen_(NULL), mdels_(NULL), 
	miels_(NULL), mrstrt_(NULL), bestSol_(NULL),snrow_(0), sncol_(0), 
	sdobj_(NULL), sdrlo_(NULL), sdrup_(NULL), sdclo_(NULL), sdcup_(NULL),
	constDrup_(NULL), constDrlo_(NULL), snSt1_(0), snSt2_(0), sSt1dels_(NULL), 
	sSt1iels_(NULL), sSt1rstrt_(NULL), sSt2dels_(NULL), sSt2iels_(NULL), sSt2rstrt_(NULL), 
	snSt1len_(NULL), snSt2len_(NULL), nScen_(0), upperBound_(infty_), scenProb_(NULL), 
	mutlobj_(0), mutiobj_(NULL), mutdobj_(NULL), scenObjStart_(NULL), mutlrlo_(0), 
	mutirlo_(NULL), mutdrlo_(NULL), scenRloStart_(NULL), mutlrup_(0), mutirup_(NULL), 
	mutdrup_(NULL), scenRupStart_(NULL), mutnSt1_(0), mutSt1dels_(NULL), mutSt1iels_(NULL), 
	mutSt1rstrt_(NULL), scenMatSt1Start_(NULL), mutnSt1len_(NULL), mutnSt2_(0), mutSt2dels_(NULL), 
	mutSt2iels_(NULL), mutSt2rstrt_(NULL), scenMatSt2Start_(NULL), mutnSt2len_(NULL), combSt2dels_(NULL), 
	combSt2iels_(NULL), combSt2rstrt_(NULL), combSt2len_(NULL), combSt2nels_(0), combdobj_(NULL), 
	INFTYTEST(pow(10.0, 20)-1), ZEROTEST(pow(10.0, -6)), cutIndex_(NULL), cutElements_(NULL), cutRowUB_(NULL),
	zSub_(0), combdrup_(NULL), combdrlo_(NULL), combdcup_(NULL), combdclo_(NULL),
	mutlcup_(0), muticup_(NULL), mutdcup_(NULL), scenCupStart_(NULL), mutlclo_(0),
	muticlo_(NULL), mutdclo_(NULL), scenCloStart_(NULL), dTrans_(NULL), iTrans_(NULL),
	lTrans_(0), rowAdd_(0), objTrans_(0), mnInt_(0), miInt_(NULL),
	snInt_(0), siInt_(NULL), mnBin_(0), miBin_(NULL), snBin_(0), 
	siBin_(NULL), tRdelta_(0.0), tRrateD_(0.5), tRrateLB_(0.8),
    knapsack_(false),warmStart_(false),warmStartFol_(false),warmStartEmptyMaster_(false),
    warmStartEmptySub_(NULL),warmStartMaster_(NULL),warmStartSub_(NULL),warmStartSubFol_(NULL),
    snels_(0),sdels_(NULL),siels_(NULL),srstrt_(NULL),slen_(NULL),
    mutnels_(0),mutdels_(NULL),mutiels_(NULL),mutrstrt_(NULL),mutlen_(NULL),scenMatStart_(NULL),
	combnels_(0), combdels_(NULL), combiels_(NULL), combrstrt_(NULL), comblen_(NULL),objDetEq_(0),subValue_(0),epsilonFeasCut_(pow(10.0,-7))
	{
		// Wich solver is used
		OsiCpxSolverInterface *cpx = new OsiCpxSolverInterface();
		//OsiClpSolverInterface *clp = new OsiClpSolverInterface();
		// Solver interface for master problem
		//osiMaster_ = clp->clone(false);
		osiMaster_ = cpx->clone(false);
		// Solver interface for subproblems
		//osiSubproblem_ = clp->clone(false);
		osiSubproblem_ = cpx->clone(false);
		// Solver interface for detEq
		osiDetEq_ = cpx->clone(false);
		// Initialize infinity
		infty_ = osiMaster_->getInfinity();
		// Solve problem with benders (true) or not (false)
		benders_ = true;
		// Single cut (true) or multi cut (false)
		single_ = true;
		// Use warmstart (true) or not (false)
		warmStart_ = false;
		// Use Trust Region (true) or not (false)
		trustRegion_ = false;
		// Use Knapsack Inequalities to accelerate branch and
		// bound algorithm; is only used, if all stage
		// 1 variables are integers
		knapsack_ = false;
		// Delete solver interfaces		
		delete cpx;
		//delete clp;
	}

	Benders(const char * single, const char * warmstart, const char * knap, const char * trust, const char * bend, const char * reg):
	  
	mnrow_(0), mncol_(0), mdobj_(NULL), mdrlo_(NULL), mdrup_(NULL), 
	mdclo_(NULL), mdcup_(NULL), mnels_(0), mlen_(NULL), mdels_(NULL), 
	miels_(NULL), mrstrt_(NULL), bestSol_(NULL),bestSolTemp_(0),snrow_(0), sncol_(0), 
	sdobj_(NULL), sdrlo_(NULL), sdrup_(NULL), sdclo_(NULL), sdcup_(NULL),
	constDrup_(NULL), constDrlo_(NULL), snSt1_(0), snSt2_(0), sSt1dels_(NULL), 
	sSt1iels_(NULL), sSt1rstrt_(NULL), sSt2dels_(NULL), sSt2iels_(NULL), sSt2rstrt_(NULL), 
	snSt1len_(NULL), snSt2len_(NULL), nScen_(0), upperBound_(infty_), scenProb_(NULL), 
	mutlobj_(0), mutiobj_(NULL), mutdobj_(NULL), scenObjStart_(NULL), mutlrlo_(0), 
	mutirlo_(NULL), mutdrlo_(NULL), scenRloStart_(NULL), mutlrup_(0), mutirup_(NULL), 
	mutdrup_(NULL), scenRupStart_(NULL), mutnSt1_(0), mutSt1dels_(NULL), mutSt1iels_(NULL), 
	mutSt1rstrt_(NULL), scenMatSt1Start_(NULL), mutnSt1len_(NULL), mutnSt2_(0), mutSt2dels_(NULL), 
	mutSt2iels_(NULL), mutSt2rstrt_(NULL), scenMatSt2Start_(NULL), mutnSt2len_(NULL), combSt2dels_(NULL), 
	combSt2iels_(NULL), combSt2rstrt_(NULL), combSt2len_(NULL), combSt2nels_(0), combdobj_(NULL),
	INFTYTEST(pow(10.0, 20)-1), ZEROTEST(pow(10.0, -6)), cutIndex_(NULL), cutElements_(NULL), cutRowUB_(NULL),
	zSub_(0), combdrup_(NULL), combdrlo_(NULL), combdcup_(NULL), combdclo_(NULL),
	mutlcup_(0), muticup_(NULL), mutdcup_(NULL), scenCupStart_(NULL), mutlclo_(0),
	muticlo_(NULL), mutdclo_(NULL), scenCloStart_(NULL), dTrans_(NULL), iTrans_(NULL),
	lTrans_(0), rowAdd_(0), objTrans_(0), mnInt_(0), miInt_(NULL),
	snInt_(0), siInt_(NULL), mnBin_(0), miBin_(NULL), snBin_(0), 
	siBin_(NULL), tRdelta_(0.0), tRrateD_(0.1), tRrateLB_(0.5),
    knapsack_(false),warmStart_(false),warmStartFol_(false),warmStartEmptyMaster_(false),
    warmStartEmptySub_(NULL),warmStartMaster_(NULL),warmStartSub_(NULL),warmStartSubFol_(NULL),
    snels_(0),sdels_(NULL),siels_(NULL),srstrt_(NULL),slen_(NULL),
    mutnels_(0),mutdels_(NULL),mutiels_(NULL),mutrstrt_(NULL),mutlen_(NULL),scenMatStart_(NULL),
	combnels_(0), combdels_(NULL), combiels_(NULL), combrstrt_(NULL), comblen_(NULL),objDetEq_(0),subValue_(0),epsilonFeasCut_(pow(10.0,-7)),masterValue_(0),appliedCuts_(0),
    useRegularizedDecomposition_(false),sigma_(1),regSolution_(0),regSolutionValue_(0), regSubproblemSolutionValues_(0),startRows_(0),endRows_(0),seriousStep_(false),computeRegularizedSubproblems_(true),changeSigma_(false),checkCuts_(false),fischetti_(false),precision_( (int)pow(10.0,5) )
	{
		// Wich solver is used
		OsiCpxSolverInterface *cpx = new OsiCpxSolverInterface();
		//OsiClpSolverInterface *clp = new OsiClpSolverInterface();
		// Solver interface for master problem
		//osiMaster_ = clp->clone(false);
		osiMaster_ = cpx->clone(false);
		// Solver interface for subproblems
		//osiSubproblem_ = clp->clone(false);
		osiSubproblem_ = cpx->clone(false);
		// Solver interface for detEq
		osiDetEq_ = cpx->clone(false);
		// Initialize infinity
		infty_ = osiMaster_->getInfinity();
        // set regSolutionValue to -infinity
        regSolutionValue_ = infty_;
		// Benders (true) or detEq (false)
		if(!strcmp(bend,"+ben")){
			benders_ = true;
		}
		if(!strcmp(bend,"-ben")){
			benders_ = false;
		}
		// Single cut (true) or multi cut (false)
		if(!strcmp(single,"s")){
			single_ = true;
		}
		if(!strcmp(single,"m")){
			single_ = false;	
		}
		// Warmstart
		if(!strcmp(warmstart, "+wsI") || !strcmp(warmstart, "+wsi") ){
			warmStart_ = true;
			warmStartFol_ = false;
		}
		if(!strcmp(warmstart,"-ws")){
			warmStart_ = false;
			warmStartFol_ = false;
		}
		if(!strcmp(warmstart,"+wsS") || !strcmp(warmstart, "+wss")){
			warmStart_ = false;
			warmStartFol_ = true;
		}
		// Use Trust Region (true) or not (false)
		if(!strcmp(trust,"+tr")){
		trustRegion_ = true;
		}
		if(!strcmp(trust,"-tr")){
		trustRegion_ = false;
		}	
		// Use knapsack inequalities to accelerate branch and
		// bound algorithm; is only used, if all stage
		// 1 variables are integers
		if(!strcmp(knap,"+ki")){
			knapsack_ = true;
		}
		if(!strcmp(knap,"-ki")){
			knapsack_ = false;
		}
        if(!strcmp(reg,"+reg")){
			useRegularizedDecomposition_ = true;
		}
		if(!strcmp(knap,"-reg")){
			useRegularizedDecomposition_ = false;
		}
		// Delete solver interfaces		
		delete cpx;
		//delete clp;
	}
	// Boolean to indicate if problem should be solved
	// with Benders or detEq
	bool benders_;
	// Read data from SMPS files; Input: path of SMPS file;
	// Output: -1 reading data failed, 0 reading data was successfull
	int readSMPS(const char*);
	// Copy data for master problem; Input: SmiScnModel
	void copyMaster(SmiScnModel *);
	// Copy data for subproblems; Input: SmiScnModel
	void copySubmodel(SmiScnModel *);
	// Solve master problem; Input: iteration counter; Output: -1 solving
	// master problem failed, 0 solving master problem was successfull
	int solveMaster();
	// Build subproblems; Input: number of current scenario
	void buildSubproblem(int);
	// Convert bounds on stage 2 variables
	void convertSubproblem();
	// Update subproblem with current stage 1 solution; Input: number 
	// of current scenario, current stage 1 solution
	void addSt1ToSubproblem(int, double*);
	// Solve subproblem; Input: iteration counter, number of current 
	// scenario; Output: -1 solving subproblem failed, 0 solving 
	// subproblem was successfull
	int solveSubproblem(int, bool);
	// Benders decomposition; Input: tolerance epsilon, maximal number 
	// of iterations
	void BendersSolve(double, double, int);
	// Calculate optimality cuts; Input: number of cuts which were 
	// generated during the current iteration, number of current scenario
	void calcOptiCut( int, int);
	// Add optimality cut to master problem; Input: number of cuts which 
	// were generated during the current iteration
	void addOptiCut( int);
	// Calculate and add a feasibility cut; Input: number of 
	// current scenario
	void addFeasCut( int );
	// Method to accelerate if there are binary variables; Input:
	// current stage 1 solution, current lower bound, best lower
	// bound, number of optimality cuts, indicator for feasibility 
	// cut; Output: 1 apply Trust Region in the next iteration, //
	// 0 stop applying Trust Region
	int trustRegion(double*, double, int, bool);
	// Write solution file to solution path
	int writeSolutionFileBenders(const char * path, double time);
	int writeSolutionFileDetEq(const char * solpath, double time );
	// Solve problem with determinisitc equivalent
	int detEq();
	// Build problem for det Eq, combine alle scenarios;
	// Input: number of current scenario
	void buildProblem(int);

    // Regularized Decomposition methods
    void setInitialSolution(double*);
    double getObjValue() { return this->LB_; }

    
	// Destructor
	~Benders();

	private:
		bool computeRelativeGap(double,double,double);
        void appendColVectorToMatrix( std::vector<int>& fixVars, CoinPackedMatrix &matrix );
        double computeDeviation( CoinPackedVector& vec, double rhs );
        double computeDeviation( int*, double*, double rhs );
        void relaxCut( CoinPackedVector& vec, double* rhs);
        double computeCoefficientTest( CoinPackedVector& vec );
        double computeCoefficientTest( int , const double* );
        double roundDouble(const double&);

};


//Forward declaration.
class SmiScnModel;
class OsiSolverInterface;

class OsiStochasticSolverInterface  {

public:
    OsiStochasticSolverInterface( OsiSolverInterface* osiSolver, SmiScnModel* smiModel);

    ~OsiStochasticSolverInterface();

    //What methods are necessary?
    virtual void initialSolve() = 0;
    virtual double getObjValue() = 0;

    SmiScnModel* getSmiModel() { return smiModel; }

protected:
    OsiSolverInterface* osiSolver;
    SmiScnModel* smiModel;

private:

};

class OsiStochasticDetEqSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticDetEqSolverInterface(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiStochasticDetEqSolverInterface();

    void initialSolve();
    double getObjValue();
};

class OsiStochasticBendersTwoStageSolverInterface : public OsiStochasticSolverInterface {
public:

     OsiStochasticBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiStochasticBendersTwoStageSolverInterface();

    void initialSolve();

    double getObjValue();
	int getNumIterations();

	void writeSolutionFileBenders(const char * path, double time ){
		benders->writeSolutionFileBenders(path, time );
	}
	
	typedef enum {MULTI, WARMSTART, KNAPSACK, TRUSTREGION, BENDERS, REGULARIZED} BendersIntParam;

    typedef enum { ABSGAP, RELGAP } BendersDoubleParam;
	
	inline void setIntParam(const OsiStochasticBendersTwoStageSolverInterface::BendersIntParam &param, const int &val) { this->intParam[param] = val; }
	inline int getIntParam(const OsiStochasticBendersTwoStageSolverInterface::BendersIntParam &param) { return this->intParam[param]; }
    inline void setDoubleParam(const OsiStochasticBendersTwoStageSolverInterface::BendersDoubleParam &param, const double &val) { this->doubleParam[param] = val; }
	inline int getDoubleParam(const OsiStochasticBendersTwoStageSolverInterface::BendersDoubleParam &param) { return this->doubleParam[param]; }
	inline double getSolutionTime() { return this->solTime; }
	
private:
    Benders* benders;
    int intParam[6];
    double doubleParam[6];
    double solTime;
};

#endif
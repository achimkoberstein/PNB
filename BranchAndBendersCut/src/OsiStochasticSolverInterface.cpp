#include "OsiStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"

#include "OsiSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"


OsiStochasticSolverInterface::OsiStochasticSolverInterface( OsiSolverInterface* osiSolver,SmiScnModel* smiModel ): osi_solver_(osiSolver),smi_model_(smiModel), log_file_("Log.csv"), instance_specification_file_("InstanceSpecs.csv") {}

OsiStochasticSolverInterface::~OsiStochasticSolverInterface(){

}

OsiSolverInterface* OsiStochasticSolverInterface::getOsiSolverInterface()
{
    return osi_solver_;
}

void OsiStochasticSolverInterface::setInstanceSpecsFile(const string& filename)
{
	instance_specification_file_ = filename;
	// Write header line.
	std::stringstream s;
	s << "Testset;Problem;Instance;Stages;Scenarios;1st Int;1st Bin;1st Cont;1st Constraints;2nd Int;2nd Bin;2nd Cont;2nd Constraints;";
	s << "1st <= constr;1st >= constr;1st == constr;1st non standard ub;1st non standard lb;"; //bool
	s << "Stochastic Obj;Stochastic RHS;Stochastic T;Stochastic W;Stochastic Bounds"; //bool

	std::string header = s.str();

	struct stat info;
	int ret = -1;
	ret = stat(instance_specification_file_.c_str(), &info); //get the file attributes
	if (!(ret == 0)) { // stat() is not able to get the file attributes, not exists
		ofstream log_file;
		log_file.open(instance_specification_file_.c_str(), ios::app | ios::out);
		log_file << header << std::endl;
		log_file.close();
	}

	// We need to check if first line is still valid
	ifstream log_file(instance_specification_file_.c_str());
	std::string line;
	std::getline(log_file, line);
	log_file.close();
	if (line != header) {
		LOG(FATAL) << "The columns in instance specs file have changed. Please remove file " << instance_specification_file_ << " before proceeding!.";
	}
}

OsiStochasticDetEqSolverInterface::OsiStochasticDetEqSolverInterface( OsiSolverInterface* osiSolver,SmiScnModel* smiModel )
: OsiStochasticSolverInterface(osiSolver,smiModel)
{
	// Set default parameter values
	parallel_benders::Parameter param_ = parallel_benders::Parameter();
	param_.SetParameterValue(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_DETEQ);
	setParameters(param_);

	// Silence solver 
	osi_solver_->messageHandler()->setLogLevel(0); // Log nothing
	osi_solver_->setHintParam(OsiDoPresolveInInitial, true);
}

OsiStochasticDetEqSolverInterface::~OsiStochasticDetEqSolverInterface()
{

}

void OsiStochasticDetEqSolverInterface::initialSolve()
{
	smi_model_->setOsiSolverHandle(osi_solver_);
	osi_solver_ = smi_model_->loadOsiSolverData();
	osi_solver_->initialSolve();
}

double OsiStochasticDetEqSolverInterface::getObjValue() const
{
    return osi_solver_->getObjValue();
}

void OsiStochasticDetEqSolverInterface::printSolutionToFile(const char* filename) const
{
	if (!osi_solver_->isProvenOptimal()) {
		LOG(ERROR) << "Solution can not be printed, because algorithm did not end with optimality.";
	}

	// We want to print the solution we got by solving the model, to the file! Therefore we need a solved problem first!
	std::ofstream solution_file(filename, fstream::out | fstream::trunc); // Open the file, regardless if it exists or not!
	LOG_IF(ERROR, !solution_file.is_open()) << "Solution file " << filename << " could not be opened to write.";
	// At first, print a summary of the solution value
	solution_file << "Objective Value : " << osi_solver_->getObjValue() << "\n";
	
	solution_file.close();
}

void OsiStochasticDetEqSolverInterface::setParameters(const parallel_benders::Parameter& parameters)
{
	
	// Update the param_ values
	for (auto leaf : parameters.parameter_) {
		param_.parameter_.put_child(leaf.first, leaf.second);
	}

	LOG_IF(FATAL, param_.GetIntParamValue(parallel_benders::PNB_BENDERS)) << "To solve the det. equivalent the PNB_BENDERS parameter should be set to 0.";

	OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
	assert(cpx_ptr);
	CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();

	int lp_method(param_.GetIntParamValue(parallel_benders::PNB_LP_METHOD));
	if (lp_method == parallel_benders::PNB_INT_PRIMAL_SIMPLEX) {
		osi_solver_->setHintParam(OsiDoDualInInitial, false);// Parameter : Use Primal or Dual Algorithm for Subproblem Solve.
		osi_solver_->setHintParam(OsiDoDualInResolve, false);
	}
	else if (lp_method == parallel_benders::PNB_INT_DUAL_SIMPLEX) {
		osi_solver_->setHintParam(OsiDoDualInInitial, true);
		osi_solver_->setHintParam(OsiDoDualInResolve, true);
	}
	else if (lp_method == parallel_benders::PNB_INT_BARRIER || lp_method == parallel_benders::PNB_INT_BARRIER_WITH_CROSSOVER) { // We ignore the hint, so we use the methos CPXlpopt instead of primopt or dualopt in initialSolve();
		// We must set the algorithm parameter for cplex!
		osi_solver_->setHintParam(OsiDoDualInInitial, true, OsiHintIgnore);
		osi_solver_->setHintParam(OsiDoDualInResolve, true, OsiHintIgnore);
	}
	else if (lp_method == parallel_benders::PNB_INT_CONCURRENT_MASTER_STANDARD) {
			osi_solver_->setHintParam(OsiDoDualInInitial, true);
			osi_solver_->setHintParam(OsiDoDualInResolve, true);
	}

	// PNB_NUM_CPU
	CPXsetintparam(env_ptr, CPX_PARAM_THREADS, param_.GetIntParamValue(parallel_benders::PNB_NUM_CPU));

	// PNB_MAXIMAL_SOLUTION_TIME
	CPXsetdblparam(env_ptr, CPX_PARAM_TILIM, param_.GetDblParamValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME));
	
	assert(false && "setParameter for the OsiStochasticDetEqSolverInterface is not implemented. Some of the parameters should be passed to osi solver (Cplex)");

}

int OsiStochasticDetEqSolverInterface::getSolutionCode() const
{
	OsiCpxSolverInterface* cpx_ptr = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
	assert(cpx_ptr);
	// Get Cplex result code
	CPXENVptr env_ptr = cpx_ptr->getEnvironmentPtr();
	CPXLPptr lp_ptr = cpx_ptr->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
	int CPXresult = CPXgetstat(env_ptr, lp_ptr);

	if ( osi_solver_->isProvenOptimal() || CPXresult == CPX_STAT_OPTIMAL) {
		return parallel_benders::PNB_SOLUTION_OPTIMAL;
	}
	else if( osi_solver_->isAbandoned() ) { // can have multiple reasons
		return parallel_benders::PNB_SOLUTION_SUBSOLVER_PROBLEM;
	}
	else if ( osi_solver_->isProvenPrimalInfeasible() || osi_solver_->isProvenDualInfeasible() ) {
		return parallel_benders::PNB_SOLUTION_INFEASIBLE;
	}
	else if (osi_solver_->isDualObjectiveLimitReached() || osi_solver_->isProvenDualInfeasible()) {
		return parallel_benders::PNB_SOLUTION_INFEASIBLE;
	}
	else if ( osi_solver_->isIterationLimitReached() ) {
		return parallel_benders::PNB_SOLUTION_ITERATION_LIMIT;
	}
	else if ( CPXresult == CPX_STAT_UNBOUNDED || CPXresult == CPXMIP_UNBOUNDED) {
		return parallel_benders::PNB_SOLUTION_UNBOUNDED;
	}
	else if ( CPXresult == CPX_STAT_ABORT_TIME_LIM || CPXresult == CPXMIP_TIME_LIM_FEAS || CPXresult == CPXMIP_TIME_LIM_INFEAS) {
		return parallel_benders::PNB_SOLUTION_TIME_LIMIT;
	}
	else {
		return parallel_benders::PNB_SOLUTION_SUBSOLVER_PROBLEM;
	}

	//Available codes in Osi interface
	//virtual bool isDualObjectiveLimitReached() const;

	//Available codes in PNB solver
	//const int PNB_SOLUTION_SIMILARITY_LIMIT = 5;
	//const int PNB_SOLUTION_GAP_NONIMPROVING = 6;
	//const int PNB_SOLUTION_PARAMETER_PROBLEM = 8;
	//const int PNB_SOLUTION_UPPER_LESS_LOWER_BOUND = 9;
	//const int PNB_SOLUTION_CVAR_H_NEGATIVE = 10;
}

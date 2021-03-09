//#include "OsiStochasticSolverInterface.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"
#include "BranchAndBendersCut.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"

OsiStochasticBranchAndBendersCutSolverInterface::OsiStochasticBranchAndBendersCutSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model)
: OsiStochasticSolverInterface(osi_solver, smi_model), bbc_(nullptr), osi_(nullptr)
{
	if (smi_model == nullptr)
		assert(false);
	//this->smi_model_->releaseSolver(); // Forget about previous solver, we do not care about it!
	this->smi_model_->setOsiSolverHandle(this->osi_solver_);
	bbc_ = new branch_and_benders_cut::BranchAndBendersCut(smi_model_);
}

OsiStochasticBranchAndBendersCutSolverInterface::~OsiStochasticBranchAndBendersCutSolverInterface()
{
	delete bbc_;
}

void OsiStochasticBranchAndBendersCutSolverInterface::initialSolve()
{

}

double OsiStochasticBranchAndBendersCutSolverInterface::getObjValue() const
{
	return 1.0;
}

void OsiStochasticBranchAndBendersCutSolverInterface::printSolutionToFile(const char* filename) const
{

}

void OsiStochasticBranchAndBendersCutSolverInterface::setParameters(const parallel_benders::Parameter& parameters)
{
	bbc_->parameter_ = parameters;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getSolutionCode() const
{
	return 1;
}

//virtual void setLogFile(std::string filename) = 0;
void OsiStochasticBranchAndBendersCutSolverInterface::setInstanceSpecsFile(const std::string& filename)
{

}

void OsiStochasticBranchAndBendersCutSolverInterface::setTestsetProblemInstanceNames(std::string testset, std::string problem, std::string instance)
{

}

void OsiStochasticBranchAndBendersCutSolverInterface::setTestsetProblemInstanceNames(std::string path)
{

}

branch_and_benders_cut::BranchAndBendersCut* OsiStochasticBranchAndBendersCutSolverInterface::getBBC()
{
	assert(bbc_);
	return bbc_;
}

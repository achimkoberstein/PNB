//#include "OsiStochasticSolverInterface.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"
#include "BranchAndBendersCut.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"

OsiStochasticBranchAndBendersCutSolverInterface::OsiStochasticBranchAndBendersCutSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model)
: OsiStochasticParallelNestedBendersSolverInterface(osi_solver, smi_model), bbc_(nullptr), osi_(nullptr)
{
	bbc_ = new branch_and_benders_cut::BranchAndBendersCut(smi_model_);
}

OsiStochasticBranchAndBendersCutSolverInterface::~OsiStochasticBranchAndBendersCutSolverInterface()
{
	delete bbc_;
}

void OsiStochasticBranchAndBendersCutSolverInterface::initialSolve()
{
	assert(false);
}

double OsiStochasticBranchAndBendersCutSolverInterface::getObjValue() const
{
	assert(false);
	return 1.0;
}

void OsiStochasticBranchAndBendersCutSolverInterface::printSolutionToFile(const char* filename) const
{
	assert(false);
}

void OsiStochasticBranchAndBendersCutSolverInterface::setParameters(const parallel_benders::Parameter& parameters)
{
	bbc_->parameter_ = parameters;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getSolutionCode() const
{
	assert(false);
	return 1;
}

//virtual void setLogFile(std::string filename) = 0;
void OsiStochasticBranchAndBendersCutSolverInterface::setInstanceSpecsFile(const std::string& filename)
{
	assert(false);
}

void OsiStochasticBranchAndBendersCutSolverInterface::setTestsetProblemInstanceNames(std::string testset, std::string problem, std::string instance)
{
	assert(false);
}

void OsiStochasticBranchAndBendersCutSolverInterface::setTestsetProblemInstanceNames(std::string path)
{
	assert(false);
}

branch_and_benders_cut::BranchAndBendersCut* OsiStochasticBranchAndBendersCutSolverInterface::getBBC()
{
	assert(bbc_);
	return bbc_;
}

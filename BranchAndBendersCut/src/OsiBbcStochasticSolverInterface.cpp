//#include "OsiStochasticSolverInterface.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"
#include "BranchAndBendersCut.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"

OsiStochasticBranchAndBendersCutSolverInterface::OsiStochasticBranchAndBendersCutSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model)
	: OsiStochasticParallelNestedBendersSolverInterface(osi_solver, smi_model), bbc_(dynamic_cast<branch_and_benders_cut::BranchAndBendersCut*>(nested_benders_))
{
}

branch_and_benders_cut::BranchAndBendersCut* OsiStochasticBranchAndBendersCutSolverInterface::getBBC()
{
	return bbc_;
}

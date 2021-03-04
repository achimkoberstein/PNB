//#include "OsiStochasticSolverInterface.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"

#include "OsiSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"


OsiBbcStochasticSolverInterface::OsiBbcStochasticSolverInterface( OsiSolverInterface* osiSolver,SmiScnModel* smiModel )
: OsiStochasticDetEqSolverInterface(osiSolver,smiModel)
{
	
}

OsiStochasticDetEqSolverInterface::~OsiStochasticDetEqSolverInterface()
{

}

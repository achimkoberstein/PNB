#include "OsiStochasticSolverInterface.hpp"

OsiStochasticSolverInterface::OsiStochasticSolverInterface( OsiSolverInterface* osiSolver, SmiScnModel* smiModel ): osiSolver(osiSolver),smiModel(smiModel) {}

OsiStochasticSolverInterface::~OsiStochasticSolverInterface(){
    if (osiSolver != 0)
        delete osiSolver;
    if (smiModel != 0)
        delete smiModel;
}

void OsiStochasticSolverInterface::initialSolve()
{//Default implementation: Solve deterministic equivalent

}

OsiStochasticDetEqSolver::OsiStochasticDetEqSolver( OsiSolverInterface* osiSolver,SmiScnModel* smiModel )
: OsiStochasticSolverInterface(osiSolver,smiModel)
{

}

OsiStochasticDetEqSolver::~OsiStochasticDetEqSolver()
{

}

void OsiStochasticDetEqSolver::initialSolve()
{
    osiSolver = smiModel->loadOsiSolverData();
    osiSolver->initialSolve();
}
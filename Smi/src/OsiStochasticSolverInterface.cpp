#include "OsiSolverInterface.hpp"
#include "SmiScnModel.hpp"

#include "OsiStochasticSolverInterface.hpp"




OsiStochasticSolverInterface::OsiStochasticSolverInterface( OsiSolverInterface* osiSolver,SmiScnModel* smiModel ): osiSolver(osiSolver),smiModel(smiModel) {}

OsiStochasticSolverInterface::~OsiStochasticSolverInterface(){
    //if (smiModel != 0){
    //    delete smiModel;
    //    smiModel = 0;
    //}
}

void OsiStochasticSolverInterface::initialSolve()
{//Default implementation: Solve deterministic equivalent

}

OsiStochasticDetEqSolverInterface::OsiStochasticDetEqSolverInterface( OsiSolverInterface* osiSolver,SmiScnModel* smiModel )
: OsiStochasticSolverInterface(osiSolver,smiModel)
{

}

OsiStochasticDetEqSolverInterface::~OsiStochasticDetEqSolverInterface()
{

}

void OsiStochasticDetEqSolverInterface::initialSolve()
{
    //How to set correct solver in smiModel?
    smiModel->setOsiSolverHandle(osiSolver);
    OsiSolverInterface* solver = smiModel->loadOsiSolverData();
    solver->initialSolve();
}

double OsiStochasticDetEqSolverInterface::getObjValue()
{
    return osiSolver->getObjValue();
}

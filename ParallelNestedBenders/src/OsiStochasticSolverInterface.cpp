#include "OsiStochasticSolverInterface.hpp"

#include "OsiSolverInterface.hpp"
#include "SmiScnModel.hpp"


OsiStochasticSolverInterface::OsiStochasticSolverInterface( OsiSolverInterface* osiSolver,SmiScnModel* smiModel ): osi_solver_(osiSolver),smi_model_(smiModel) {}

OsiStochasticSolverInterface::~OsiStochasticSolverInterface(){

}

void OsiStochasticSolverInterface::initialSolve()
{
}

OsiSolverInterface* OsiStochasticSolverInterface::getOsiSolverInterface()
{
    return osi_solver_;
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
    smi_model_->setOsiSolverHandle(osi_solver_);
    OsiSolverInterface* solver = smi_model_->loadOsiSolverData();
    solver->initialSolve();
}

double OsiStochasticDetEqSolverInterface::getObjValue()
{
    return osi_solver_->getObjValue();
}
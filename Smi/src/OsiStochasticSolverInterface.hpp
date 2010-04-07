#ifndef OsiStochasticSolverInterface_HPP
#define OsiStochasticSolverInterface_HPP

#include "OsiSolverInterface.hpp"
#include "SmiScnModel.hpp"


//Inherit from OsiSolverInterface?! Or not?! 
class OsiStochasticSolverInterface : public OsiSolverInterface {

public:
    OsiStochasticSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel);

    ~OsiStochasticSolverInterface();

    //What methods are necessary?
    virtual void initialSolve() = 0;

    
protected:
    OsiSolverInterface* osiSolver;
    SmiScnModel* smiModel;

private:



};

class OsiStochasticDetEqSolver : public OsiStochasticSolverInterface {
public:

    OsiStochasticDetEqSolver(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiStochasticDetEqSolver();

    void initialSolve();
};

#endif //OsiStochasticSolverInterface_HPP
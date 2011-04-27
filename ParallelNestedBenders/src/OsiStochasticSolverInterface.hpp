#ifndef OsiStochasticSolverInterface_h__
#define OsiStochasticSolverInterface_h__



#include "OsiStochasticSolverInterface.hpp"

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

    SmiScnModel* getSmiModel() { return smi_model_; }

protected:
    OsiSolverInterface* osi_solver_;
    SmiScnModel* smi_model_;

private:

};

class OsiStochasticDetEqSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticDetEqSolverInterface(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiStochasticDetEqSolverInterface();

    void initialSolve();
    double getObjValue();
};

#endif // OsiStochasticSolverInterface_h__
#ifndef OsiStochasticSolverInterface_HPP
#define OsiStochasticSolverInterface_HPP


//To conform with rest of SMI
using namespace std;
//Forward declaration.
class SmiScnModel;
class OsiSolverInterface;
//Inherit from OsiSolverInterface?! Or not?! 
class OsiStochasticSolverInterface  {

public:
    OsiStochasticSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel);

    ~OsiStochasticSolverInterface();

    //What methods are necessary?
    virtual void initialSolve() = 0;

    OsiSolverInterface* getOsiSolver() { return osiSolver; }
    SmiScnModel* getSmiModel() { return smiModel; }

protected:
    OsiSolverInterface* osiSolver;
    SmiScnModel* smiModel;

private:



};

class OsiStochasticDetEqSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticDetEqSolverInterface(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiStochasticDetEqSolverInterface();

    void initialSolve();
};




#endif //OsiStochasticSolverInterface_HPP
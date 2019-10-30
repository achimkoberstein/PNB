#ifndef OsiStochasticSolverInterface_h__
#define OsiStochasticSolverInterface_h__

#include "OsiStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"

namespace parallel_benders {
	class ParallelNestedBenders;
	struct Parameter;
}

//Forward declaration.
class SmiScnModel;
class OsiSolverInterface;

class OsiStochasticSolverInterface  {

public:
    OsiStochasticSolverInterface( OsiSolverInterface* osiSolver, SmiScnModel* smiModel);

    ~OsiStochasticSolverInterface();

    //What methods are necessary?
    virtual void initialSolve() = 0;
    OsiSolverInterface* getOsiSolverInterface();
    virtual double getObjValue() const = 0;

	virtual void printSolutionToFile(const char* filename) const = 0;

	virtual void setParameters(const parallel_benders::Parameter& parameters) = 0;

    SmiScnModel* getSmiModel() { return smi_model_; }

	virtual int getSolutionCode() const = 0;

protected:
    OsiSolverInterface* osi_solver_;
    SmiScnModel* smi_model_;

private:

};

class OsiStochasticDetEqSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticDetEqSolverInterface(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiStochasticDetEqSolverInterface();

	parallel_benders::Parameter param_;

    void initialSolve();
    double getObjValue() const;

	void printSolutionToFile(const char* filename) const;
	
	void setParameters(const parallel_benders::Parameter& parameters);

	int getSolutionCode() const;
};

#endif // OsiStochasticSolverInterface_h__
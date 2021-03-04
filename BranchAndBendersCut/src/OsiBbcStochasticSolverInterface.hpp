#ifndef OsiBbcStochasticSolverInterface_h__
#define OsiBbcStochasticSolverInterface_h__

#include "OsiStochasticSolverInterface.hpp" // OsiStochasticDetEqSolverInterface
#include "BendersNodeData.hpp"

namespace parallel_benders {
    class ParallelNestedBenders;
    struct Parameter;
}

//Forward declaration.
//class SmiScnModel;
//class OsiSolverInterface;

class OsiBbcStochasticSolverInterface : public OsiStochasticDetEqSolverInterface {
public:

	OsiBbcStochasticSolverInterface(OsiSolverInterface* osiSolver,SmiScnModel* smiModel);
    ~OsiBbcStochasticSolverInterface();

    //parallel_benders::Parameter param_;

    //void initialSolve();
    //double getObjValue() const;

    //void printSolutionToFile(const char* filename) const;
    
    //void setParameters(const parallel_benders::Parameter& parameters);

    //int getSolutionCode() const;
};

#endif // OsiBbcStochasticSolverInterface_h__
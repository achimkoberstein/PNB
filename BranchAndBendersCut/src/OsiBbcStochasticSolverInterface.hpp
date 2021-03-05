#ifndef OsiBbcStochasticSolverInterface_h__
#define OsiBbcStochasticSolverInterface_h__

#include "OsiPnbStochasticSolverInterface.hpp" // OsiStochasticParallelNestedBendersSolverInterface
#include "BendersNodeData.hpp"

namespace branch_and_benders_cut {
    class BranchAndBendersCut;
    struct Parameter;
}

//Forward declaration.
//class SmiScnModel;
//class OsiSolverInterface;

class OsiStochasticBranchAndBendersCutSolverInterface : public OsiStochasticParallelNestedBendersSolverInterface {
public:
	OsiStochasticBranchAndBendersCutSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model);

	branch_and_benders_cut::BranchAndBendersCut* getBBC();

private:
	branch_and_benders_cut::BranchAndBendersCut* bbc_;
};

#endif // OsiBbcStochasticSolverInterface_h__
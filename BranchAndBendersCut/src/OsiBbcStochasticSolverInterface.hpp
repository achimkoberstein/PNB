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
	~OsiStochasticBranchAndBendersCutSolverInterface();

	double getIntegerGap() const override;

	void printSolutionToFile(const char* filename) const override;

	double getSolutionValueForScenario(int scenario) override;

	int getNumProcessedMipNodes() const;

	double getInitRootNodeObjectiveValue() const;

	branch_and_benders_cut::BranchAndBendersCut* getBBC() const;

	const double* getSolutionByNodeAndStage(int node_number, int stage) const override;

	void initialSolve() override;

private:
	OsiSolverInterface* osi_;
	int last_used_algorithm_;

	// Checks if initialSolve() was already called
	void assertTriedToSolve() const;
};

#endif // OsiBbcStochasticSolverInterface_h__
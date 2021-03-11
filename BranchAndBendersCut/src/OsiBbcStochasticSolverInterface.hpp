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

	void initialSolve();
	double getObjValue() const;

	void printSolutionToFile(const char* filename) const;

	void setParameters(const parallel_benders::Parameter& parameters);

	int getSolutionCode() const;

	//virtual void setLogFile(std::string filename) = 0;
	void setInstanceSpecsFile(const std::string& filename);
	void setTestsetProblemInstanceNames(std::string testset, std::string problem, std::string instance);
	void setTestsetProblemInstanceNames(std::string path);

	branch_and_benders_cut::BranchAndBendersCut* getBBC();

private:
	branch_and_benders_cut::BranchAndBendersCut* bbc_;
	OsiSolverInterface* osi_;
};

#endif // OsiBbcStochasticSolverInterface_h__
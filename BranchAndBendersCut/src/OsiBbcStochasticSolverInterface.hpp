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

	void initialSolve() override;
	double getObjValue() const override;

	int getNumContPhasePnbIterations() const override;

	double getUB() const override;

	double getLB() const override;

	double getRelativeGap() const override;

	double getIntegerGap() const override;

	const parallel_benders::Timer& getTimer() const override;

	const parallel_benders::Timer& getBuildTimer() const override;

	const parallel_benders::Timer& getComputingTimer() const override;

	const parallel_benders::Timer& getLiftAndProjectCutsTimer() const override;

	int getNumOptimalityCuts() const override;

	int getNumContinuousOptimalityCuts() const override;

	void printSolutionToFile(const char* filename) const override;

	void setParameters(const parallel_benders::Parameter& parameters) override;

	int getSolutionCode() const override;

	int getNumInfeasibilityIterations() const override;

	void fixFirstStageVariables(const double* solution) override;

	double getSolutionValueForScenario(int scenario) override;

	virtual void giveStartSolution(const double* solution, int num_cols) override;

	//virtual void setLogFile(std::string filename) = 0;
	void setInstanceSpecsFile(const std::string& filename) override;
	void setTestsetProblemInstanceNames(std::string testset, std::string problem, std::string instance) override;
	void setTestsetProblemInstanceNames(std::string path) override; // use path to the .sto file

	int getNumFeasibilityCuts() const override;
	int getNumEpigraphCuts() const override;
	int getNumStrengthenedEpigraphCuts() const override;
	int getNumPlainEpigraphCuts() const override;
	int getNumDomainCuts() const override;
	int getLpCgpCounter() const override;

	branch_and_benders_cut::BranchAndBendersCut* getBBC() const;

	const double* getSolutionByNodeAndStage(int node_number, int stage) const override;

private:
	branch_and_benders_cut::BranchAndBendersCut* bbc_;
	OsiSolverInterface* osi_;
	int last_used_algorithm_;
	void assertTriedToSolve() const;
};

#endif // OsiBbcStochasticSolverInterface_h__
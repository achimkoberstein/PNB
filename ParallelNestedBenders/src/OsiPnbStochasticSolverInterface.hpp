#ifndef OsiPnbStochasticSolverInterface_h__
#define OsiPnbStochasticSolverInterface_h__

#include "OsiStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"

namespace parallel_benders {
    class ParallelNestedBenders;
    struct Parameter;
}


class OsiStochasticParallelNestedBendersSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticParallelNestedBendersSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model);
    //OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model, BendersParameter& params);
    ~OsiStochasticParallelNestedBendersSolverInterface();

    void initialSolve();
    void printProblemCharacteristics();

    double getObjValue();
    int getNumOptimalityCuts();
    int getNumFeasibilityCuts();
    void setParameters(const parallel_benders::Parameter& parameters);
    int getNumIterations();
	const parallel_benders::Parameter& getParameters() const;
    const parallel_benders::Timer& getTimer() const;

    SmiScnModel* getSmiModel();
    OsiSolverInterface* getOsiSolverInterface();


private:
    parallel_benders::ParallelNestedBenders* nested_benders_;

};

class SampleAverageApproximationSolverInterface {
public:
    SampleAverageApproximationSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model);

    void initialSolve();
    void setParameters(const parallel_benders::Parameter& parameters);



private:
    OsiStochasticParallelNestedBendersSolverInterface osi_pnb;
};
#endif // OsiPnbStochasticSolverInterface_h__
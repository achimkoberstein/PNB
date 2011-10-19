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

    double getObjValue();
    int getNumOptimalityCuts();
    int getNumFeasibilityCuts();
    void setParameters(const parallel_benders::Parameter& parameters);
    int getNumIterations();
	const parallel_benders::Parameter& getParameters() const;
    const parallel_benders::Timer& getTimer() const;


private:
    parallel_benders::ParallelNestedBenders* nested_benders_;

};
#endif // OsiPnbStochasticSolverInterface_h__
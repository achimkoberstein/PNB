#ifndef OsiPnbStochasticSolverInterface_h__
#define OsiPnbStochasticSolverInterface_h__

#include "OsiStochasticSolverInterface.hpp"

namespace parallel_benders {
    class ParallelNestedBenders;
    class Parameter;
}


class OsiStochasticParallelNestedBendersTwoStageSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model);
    //OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model, BendersParameter& params);
    ~OsiStochasticParallelNestedBendersTwoStageSolverInterface();

    void initialSolve();

    double getObjValue();
    void setParameters(const parallel_benders::Parameter& parameters);
    int getNumIterations();
    //inline void setParams(BendersParameter &params) { this->params = params; }

    //inline void setIntParam(const BendersParameter::BendersIntParam &param, const int &val) { params.setIntParam(param, val); }
    //inline int getIntParam(const BendersParameter::BendersIntParam &param) { return params.getIntParam(param); }
    //inline void setDoubleParam(const BendersParameter::BendersDoubleParam &param, const double &val) { params.setDoubleParam(param, val); }
    //inline double getDoubleParam(const BendersParameter::BendersDoubleParam &param) { return params.getDoubleParam(param); }


private:
    parallel_benders::ParallelNestedBenders* nested_benders_;

};
#endif // OsiPnbStochasticSolverInterface_h__
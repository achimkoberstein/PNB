#ifndef ParallelNestedBenders_h__
#define ParallelNestedBenders_h__


#include <vector>

#include <CoinPackedVector.hpp>

#include "Benders.hpp" // Just for OsiStochasticSolverInterface, needs to be outsourced to own file



class OsiStochasticParallelNestedBendersTwoStageSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel);
    OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel, BendersParameter& params);
    ~OsiStochasticParallelNestedBendersTwoStageSolverInterface();

    void initialSolve();

    double getObjValue();

    inline void setParams(BendersParameter &params) { this->params = params; }

    inline void setIntParam(const BendersParameter::BendersIntParam &param, const int &val) { params.setIntParam(param, val); }
    inline int getIntParam(const BendersParameter::BendersIntParam &param) { return params.getIntParam(param); }
    inline void setDoubleParam(const BendersParameter::BendersDoubleParam &param, const double &val) { params.setDoubleParam(param, val); }
    inline double getDoubleParam(const BendersParameter::BendersDoubleParam &param) { return params.getDoubleParam(param); }


private:
    BendersParameter params;

};
#endif // ParallelNestedBenders_h__
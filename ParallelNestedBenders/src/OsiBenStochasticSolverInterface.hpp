#ifndef OsiBenStochasticSolverInterface_h__
#define OsiBenStochasticSolverInterface_h__

#include "OsiStochasticSolverInterface.hpp"

#include "Benders.hpp"

#include "OsiSolverInterface.hpp"
#include "SmiScnModel.hpp"

class OsiStochasticBendersTwoStageSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel);
    OsiStochasticBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel, BendersParameter& params);
    ~OsiStochasticBendersTwoStageSolverInterface();

    void initialSolve();

    double getObjValue();
    int getNumIterations();

    void writeSolutionFileBenders(const char * path, double time ){
        benders->writeSolutionFileBenders(path, time );
    }

    inline void setParams(BendersParameter &params) { this->params = params; }
    inline void setIntParam(const BendersParameter::BendersIntParam &param, const int &val) { params.setIntParam(param, val); }
    inline int getIntParam(const BendersParameter::BendersIntParam &param) { return params.getIntParam(param); }
    inline void setDoubleParam(const BendersParameter::BendersDoubleParam &param, const double &val) { params.setDoubleParam(param, val); }
    inline double getDoubleParam(const BendersParameter::BendersDoubleParam &param) { return params.getDoubleParam(param); }
    inline double getSolutionTime() { return this->solTime; }

    // time statistics: TODO keep updated or resort to TimeStatistics struct..
    inline Benders::TimeStatistics getTimeStatistics() { return benders->getTimeStatistics(); }
    inline double getTimeSolverMaster() { return benders->getTimeSolverMaster(); } // time the solver needed for the master problems
    inline double getTimeSolverSubproblems() { return benders->getTimeSolverSubproblems(); } // time the solver needed for the subproblems
    inline double getTimeBendersSolve() { return benders->getTimeBendersSolve(); } // time needed for the whole BendersSolve method
    inline double getTimeSubproblemLoop() { return benders->getTimeSubproblemLoop(); } // time needed for generating and solving all subproblems

    // Data statistics
    inline Benders::DataStatistics getDataStatistics() { return benders->getDataStatistics(); }

private:
    Benders* benders;
    BendersParameter params;
    double solTime;
    int numIter;
};
#endif // OsiBenStochasticSolverInterface_h__
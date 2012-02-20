#ifndef OsiPnbStochasticSolverInterface_h__
#define OsiPnbStochasticSolverInterface_h__


#include "PseudoRandomGenerator.hpp"
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
    double* getRandomVariates(int number_of_variates);


private:
    boost::shared_ptr<OsiStochasticParallelNestedBendersSolverInterface> osi_pnb_;
    parallel_benders::Parameter parameters_;
};
// Done this way to be able to call it from Smi without changing Smi to implement boost stuff
static Uniform01Generator uniform_generator_struct;

double get_random_0_1_variate();
double * get_random_01_variates(int number_of_variates);
double * get_latin_hypercube_random_01_variates(int number_of_variates);
double * get_antithetic_random_01_variates(int number_of_variates);
#endif // OsiPnbStochasticSolverInterface_h__
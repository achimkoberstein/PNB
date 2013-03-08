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
    void printProblemCharacteristics() const ;

    double getObjValue() const;
	
    const double * getFirstStageSolution();
    const double * getSolutionByNodeAndStage(int node_number, int stage) const;

    //const double * getFirstStageDuals();
    //const double * getDualsByNodeAndStage(int node_number, int stage) const;

	void printSolutionToFile(const char* filename);

    void fixFirstStageVariables(const double* solution);
    void giveStartSolution(const double* solution);

    double getSolutionValueForScenario(int scenario);

    int getNumOptimalityCuts() const;
    int getNumFeasibilityCuts() const;
    void setParameters(const parallel_benders::Parameter& parameters);
    int getNumIterations() const;
	const parallel_benders::Parameter& getParameters() const;
    const parallel_benders::Timer& getTimer() const;
    const parallel_benders::Timer& getBuildTimer() const;
    const parallel_benders::Timer& getComputingTimer() const;

    SmiScnModel* getSmiModel();
    int getNumInsubstantialIterations() const;
    int getNumInfeasibilityIterations() const;

    int getNumCriticalIteration() const ;
    int getMaximalLengthOfCriticalIterationSequence() const;

    int getSolutionCode() const;

private:
    parallel_benders::ParallelNestedBenders* nested_benders_;
    OsiSolverInterface * osi_;

};



class SampleAverageApproximationSolverInterface {
public:
    SampleAverageApproximationSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model);

    void initialSolve();

    void setSampleSize(const std::vector<int>& sample_size);
    void setSampleSizeUpperBound(const std::vector<int>& sample_size);

    void setNumberBatches(int batches);
    void setNumberBatchesUpperBound(int batches);

    void setConfidenceLevel(double one_minus_alpha);

    void setParameters(const parallel_benders::Parameter& parameters);
    double* getRandomVariates(int number_of_variates);


private:
    std::shared_ptr<OsiStochasticParallelNestedBendersSolverInterface> osi_pnb_;
    parallel_benders::Parameter parameters_;
    std::vector<int> samples_;
    std::vector<int> samples_upper_;
    int number_batches_;
    int number_batches_upper_;
    double alpha_;
};
// Done this way to be able to call it from Smi without changing Smi to implement boost stuff
static parallel_benders::Uniform01Generator uniform_generator_struct;

double get_random_0_1_variate();
double * get_random_01_variates(int number_of_variates);
double * get_latin_hypercube_random_01_variates(int number_of_variates);
double * get_antithetic_random_01_variates(int number_of_variates);

// call boost rng from std::random_shuffle
ptrdiff_t getRandomNumberFromRNG(ptrdiff_t maximal_number);


#endif // OsiPnbStochasticSolverInterface_h__
#ifndef _SAA_hpp_
#define _SAA_hpp_

#include <vector>

#include "OsiSolverInterface.hpp"
#include "OsiStochasticSolverInterface.hpp"

#include "flopc.hpp"


using namespace ::flopc;

class SAASolver {

    //TODO: We need some kind of fixing of variables to be able to compute the final step of the algorithm
    // Workaround: Introduce constraints that fix the variables to the already defined values..
    // Necessary for that: Find a way to return the first stage variables ONLY.
    // We need a listing of all the variables and see if they have a stage set.. this get's more complicated than i thought
    
public:
    SAASolver( MP_model& , OsiSolverInterface& );
    ~SAASolver();

    void setValues( int innerSampleSize, int outerSampleSize, int numberOfRuns );
    double getSolutionValue() const;
    double getSolutionValueForRun( int runNumber ) const;
    void solve( const MP_model::MP_direction& );

private:
    int innerSampleSize;
    int outerSampleSize;
    int numberOfRuns;

    bool initialized;

    //Confidence interval 

    std::vector<double> solutionValues;

    MP_model& model;
    OsiSolverInterface& solver;



};


#endif // _SAA_hpp_

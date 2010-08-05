#include "SAA.hpp"

//TODO: Initialize the other variables
SAASolver::SAASolver(MP_model& model, OsiSolverInterface& solver) : innerSampleSize(0),outerSampleSize(0),numberOfRuns(0),initialized(false),model(model),solver(solver)
{ }

void SAASolver::solve( const MP_model::MP_direction& direction) {
//
    if ( !initialized )
        return;

    // We already have set the discretization method, now we only need to set the sample size accordingly.
    model.setSampleSize(innerSampleSize);
    model.setSample(true); // We want to sample

    // Solve the problem numberOfRuns times and store the objective function value as well as the values for the first stage variables => how can we get these values?
    for (int i = 0; i < numberOfRuns; i++){
        model.attach(); // This step includes resampling..
        model.solve(direction);
        solutionValues[i] = model->getObjValue();
    }

    // Set the sample size to outerSampleSize

    // Solve problem with one of the computed solutions above to get the solution for the "real" problem..

    // Store the objective function value

    // Compute the confidence interval

}

SAASolver::~SAASolver()
{

}

void SAASolver::setValues( int innerSampleSize, int outerSampleSize, int numberOfRuns )
{
    this->innerSampleSize = innerSampleSize;
    this->outerSampleSize = outerSampleSize;
    this->numberOfRuns = numberOfRuns;
    initialized = true;

}
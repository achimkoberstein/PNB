#include "OsiPnbStochasticSolverInterface.hpp"

//Windows. Construct necessary as VisualC++ include stdlib.h before I can set _CRT_RAND_S
#ifdef _MSC_VER
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif 
#include <stdlib.h>
#else
#include <stdio.h>
#include <fcntl.h>
#endif
//Linux: Call /dev/urandom to generate a seed

#include <glog/logging.h>
#include "SmiScnModel.hpp"
#include "ParallelNestedBenders.hpp"
#include "BendersNodeData.hpp"

#define CPLEX_PRESENT
#ifdef CPLEX_PRESENT
#include "cplex.h"
#include "OsiCpxSolverInterface.hpp"
#endif




void OsiStochasticParallelNestedBendersSolverInterface::setParameters(const parallel_benders::Parameter& parameters){
    nested_benders_->parameter_ = parameters;
}


double OsiStochasticParallelNestedBendersSolverInterface::getObjValue()
{
    return nested_benders_->upper_bound_;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumIterations()
{
    return nested_benders_->current_iteration_;
}
void OsiStochasticParallelNestedBendersSolverInterface::initialSolve()
{
    if ( nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_BENDERS,1) == 0){
        parallel_benders::Timer solution_time(2);
        solution_time.Start(1);
        OsiSolverInterface* osi = nested_benders_->smi_->loadOsiSolverData();
        if ( nested_benders_->smi_->getNumScenarios() == 0){
            // We have no scenarios
            LOG(ERROR) << "No scenarios available.";
        }
        // Model statistics
        std::cout << "Variables: " << osi->getNumCols() << " Rows: " << osi->getNumRows() << " Elements: " << osi->getNumElements() << " Integer: " << osi->getNumIntegers() << std::endl;
        solution_time.Stop(1);
        solution_time.Start(0);
        //osi->writeMps("deterministicEquivalent","mps",1);
        // Use Barrier method if we use cplex.. ugly code results
#ifdef CPLEX_PRESENT
        OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi);
        if ( cpx_solver){
            CPXLPptr lpPtr = cpx_solver->getLpPtr(); // translates to getMutableLpPtr ( which does not change the problem )
            CPXENVptr envPtr = cpx_solver->getEnvironmentPtr();
			//CPXsetintparam(envPtr,CPX_PARAM_NUMERICALEMPHASIS,1);
   //         CPXsetintparam(envPtr,CPX_PARAM_SCAIND,1);
   //         CPXsetdblparam(envPtr,CPX_PARAM_EPMRK,0.9);
            CPXsetdblparam(envPtr,CPX_PARAM_TILIM,nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME,3600));
			// Wee need to differentiate between integer and not
			if ( nested_benders_->smi_->getCore()->getIntegerLength() ){
				osi->branchAndBound();
			}
			else {
                CPXdualopt(envPtr,lpPtr);
				//CPXbaropt(envPtr,lpPtr);
			}
            //CPXsetdblparam(envPtr,CPX_PARAM_EPOPT,0.00000001);
            // Check for errors
            int result = CPXgetstat(envPtr,lpPtr);
            if ( result )
                LOG(INFO) << "Cplex returned with status code " << result;
            double obj;
            CPXgetobjval(envPtr,lpPtr,&obj);
            //LOG(INFO) << "Solution: " << obj;
            // Get first stage solution
            // TODO: Move this to another place later..
            //int num_first_stage_cols = nested_benders_->smi_->getCore()->getNumCols(0);
            //const double* col_solution = osi->getColSolution();
            //for ( int i = 0; i < num_first_stage_cols; i++ ){
            //    std::cout << "x" << i << " : " << col_solution[i] << std::endl;
            //}

        }
        else{
#endif
            if ( nested_benders_->smi_->getCore()->getIntegerLength() ){
                //osi->setInteger(nested_benders_->smi_->getCore()->getIntegerIndices(),nested_benders_->smi_->getCore()->getIntegerLength());
                osi->branchAndBound();
            }
            else {
                osi->initialSolve();
            }

#ifdef CPLEX_PRESENT
        }
#endif
        solution_time.Stop(0);
        // Set solution value
        nested_benders_->upper_bound_ = osi->getObjValue();
        // We have the time, but can not set it accordingly. not good!
        LOG(INFO) << "Solution: " << nested_benders_->upper_bound_;
        LOG(INFO) << "Solution Time: " << solution_time.GetTime(0) << "s";

    }
    else {
        nested_benders_->Init();
        nested_benders_->Solve();
    }
}

OsiStochasticParallelNestedBendersSolverInterface::OsiStochasticParallelNestedBendersSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model )
: OsiStochasticSolverInterface(osi_solver,smi_model),nested_benders_(NULL)
{
    if ( smi_model == NULL )
        assert(false);
    this->smi_model_->setOsiSolverHandle(this->osi_solver_);
    nested_benders_ = new parallel_benders::ParallelNestedBenders(smi_model_);
}

OsiStochasticParallelNestedBendersSolverInterface::~OsiStochasticParallelNestedBendersSolverInterface()
{
    delete nested_benders_;
}

const parallel_benders::Parameter& OsiStochasticParallelNestedBendersSolverInterface::getParameters() const
{
	return nested_benders_->parameter_;
}

const parallel_benders::Timer& OsiStochasticParallelNestedBendersSolverInterface::getTimer() const
{
    return nested_benders_->timer_;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumOptimalityCuts()
{
    int cut_counter = 0;
    for (int i = 0; i < nested_benders_->num_stages_; i++){
        cut_counter += nested_benders_->added_optimality_cuts_[i];
    }
    return cut_counter;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumFeasibilityCuts()
{
    int cut_counter = 0;
    for (int i = 0; i < nested_benders_->num_stages_; i++){
        cut_counter += nested_benders_->added_feasibility_cuts_[i];
    }
    return cut_counter;
}

void OsiStochasticParallelNestedBendersSolverInterface::printProblemCharacteristics()
{
    if ( osi_solver_ ){
        double cols = osi_solver_->getNumCols();
        double rows = osi_solver_->getNumRows();
        double elems  = osi_solver_->getNumElements();
        LOG(INFO) << "Cols: " << cols << " Rows: " << rows << " Nonzeroes: " << elems;
    }
}

SmiScnModel* OsiStochasticParallelNestedBendersSolverInterface::getSmiModel()
{
    return nested_benders_->smi_;
}

OsiSolverInterface* OsiStochasticParallelNestedBendersSolverInterface::getOsiSolverInterface()
{
    return nested_benders_->osi_solver_;
}
SampleAverageApproximationSolverInterface::SampleAverageApproximationSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model ): osi_pnb(osi_solver,smi_model)
{

}

void SampleAverageApproximationSolverInterface::initialSolve()
{
    typedef boost::uniform_01<> distr_type;
    typedef boost::variate_generator<base_generator_type&, boost::uniform_01<double> > uniform_distr_gen;
    boost::variate_generator<uniform_distr_gen,distr_type > variateGen(uniform_distr_gen,distr_type);
    std::vector<int> samples;
    samples.push_back(0);
    samples.push_back(10);
    // We need a random generator function here!
    osi_pnb.getSmiModel()->sampleScenarioTree(samples,0,&variateGen );
    osi_pnb.getSmiModel()->writeSmps("saa_test");
    osi_pnb.initialSolve();
}

void SampleAverageApproximationSolverInterface::setParameters( const parallel_benders::Parameter& parameters )
{
    osi_pnb.setParameters(parameters);
}

Uniform01Generator::Uniform01Generator() : rng(),uniform(),seed(0) {
//#ifdef _MSC_VER
//    //Windows
//    errno_t err;
//    err = rand_s(&seed); //Write seed in seed variable
//    if (err != 0){
//        std::cerr << "Error occured during seed generation: " << err << std::endl;
//        // We need to abort here TODO
//    }
//#endif
//    //So we are not compiling using windows compiler, so we have /dev/urandom
//#ifndef _MSC_VER
//    //Copied from Good Practice RNG.pdf (google for it)
//    int fn;
//    fn = open("/dev/urandom", O_RDONLY);
//    if (fn == -1)
//        exit(-1); /* Failed! */
//    if (read(fn, &seed, 4) != 4)
//        exit(-1); /* Failed! */
//    close(fn);
//    //Without random numbers this does not make much sense
//#endif

    std::cout << "Seed is: " << seed << std::endl;
}
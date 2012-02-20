#include "OsiPnbStochasticSolverInterface.hpp"

#include "PseudoRandomGenerator.hpp"
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
SampleAverageApproximationSolverInterface::SampleAverageApproximationSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model ): osi_pnb_(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model))
{

}

void SampleAverageApproximationSolverInterface::initialSolve()
{
    unsigned int seed;

    // How it works: We need a random generator and a distribution we want to draw numbers from.
    // Set the seed 
    uniform_generator_struct.seed = get_random_number_from_system();
    // Create a random number generator instance
    uniform_generator_struct.rng = boost::shared_ptr<base_generator_type>(new base_generator_type(seed));
    // Create a random [0,1) variate generator;
    boost::uniform_01<> uniform_01_distribution;
    uniform_generator_struct.uniform = boost::shared_ptr<uniform_distr_gen>(new uniform_distr_gen(*uniform_generator_struct.rng.get(),uniform_01_distribution));

    // Some things we need to share later on
    OsiSolverInterface* osi_solver = osi_pnb_->getOsiSolverInterface();
    SmiScnModel* smi_model = osi_pnb_->getSmiModel();


    std::vector<int> samples;
    samples.push_back(0);
    samples.push_back(50);
    // We need a random generator function here!
    osi_pnb_->getSmiModel()->sampleScenarioTree(samples,0, get_random_01_variates );
    osi_pnb_->getSmiModel()->writeSmps("saa_monte_carlo_test");
    osi_pnb_->initialSolve();
    // get results

    osi_solver = osi_pnb_->getOsiSolverInterface();
    osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
    osi_pnb_->setParameters(parameters_);

    osi_pnb_->getSmiModel()->sampleScenarioTree(samples,0, get_antithetic_random_01_variates );
    osi_pnb_->getSmiModel()->writeSmps("saa_antithetic_test");
    osi_pnb_->initialSolve();
    
    osi_solver = osi_pnb_->getOsiSolverInterface();
    osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
    osi_pnb_->setParameters(parameters_);

    osi_pnb_->getSmiModel()->sampleScenarioTree(samples,0, get_latin_hypercube_random_01_variates );
    osi_pnb_->getSmiModel()->writeSmps("saa_latin_hypercube_test");
    osi_pnb_->initialSolve();
}

void SampleAverageApproximationSolverInterface::setParameters( const parallel_benders::Parameter& parameters )
{
    osi_pnb_->setParameters(parameters);
    parameters_ = parameters;
}


double get_random_0_1_variate(){
    // If seed is 0, random generator is not initialized!
    double random_variate = 0;
    if ( uniform_generator_struct.seed != 0 )
        random_variate = (*uniform_generator_struct.uniform)();

    return random_variate;
}

// Crude Monte Carlo Versions!
double * get_random_01_variates(int number_of_variates){
    assert( number_of_variates > 0);
    if ( uniform_generator_struct.seed == 0)
        return NULL;

    double* random_variates = new double[number_of_variates];
    for (int i = 0; i < number_of_variates; i++)
        random_variates[i] = (*uniform_generator_struct.uniform)();
    return random_variates;
}

// Antithetic Crude Monte Carlo Sampling 
double * get_antithetic_random_01_variates(int number_of_variates){
    assert( number_of_variates > 0);
    if ( uniform_generator_struct.seed == 0)
        return NULL;

    double* random_variates = new double[number_of_variates];
    for (int i = 0; i < number_of_variates; i++){
        if ( i % 2 == 0 )
            random_variates[i] = (*uniform_generator_struct.uniform)();
        else
            random_variates[i] = 1-random_variates[i-1];
    }
    return random_variates;
}

// Latin Hypercube Sampling
double * get_latin_hypercube_random_01_variates(int number_of_variates){
    assert( number_of_variates > 0);
    if ( uniform_generator_struct.seed == 0)
        return NULL;

    // Generate one variate uniformly at random in every interval
    double* random_variates = new double[number_of_variates];
    double division = 1.0/number_of_variates;

    int i = 0;
    for ( double lower_interval = 0.0; lower_interval < 1; lower_interval += division){
        // Sample one variable from [lower_interval,lower_interval+division)
        boost::uniform_real<> uniform_real_distribution(lower_interval,lower_interval+division);
        boost::variate_generator<base_generator_type&, boost::uniform_real<double> > variate_gen(*uniform_generator_struct.rng,uniform_real_distribution);
        random_variates[i] = variate_gen();
        assert( random_variates[i] >= lower_interval && random_variates[i] < lower_interval+division);
        i++;
    }
    // Shuffle random_variates randomly. TODO: Use random number generator uniform_int(0,number_of_variates-1) from boost
    std::random_shuffle(&random_variates[0],&random_variates[number_of_variates]);

    assert( i == number_of_variates);
    return random_variates;
}


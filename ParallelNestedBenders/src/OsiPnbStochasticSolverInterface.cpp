#include <algorithm>
#include <numeric>
#include <vector>

#include <glog/logging.h>
#include <boost/shared_array.hpp>
#include <boost/math/distributions/students_t.hpp>

#include "CoinHelperFunctions.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"

#include "PseudoRandomGenerator.hpp"
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

const double * OsiStochasticParallelNestedBendersSolverInterface::getSolutionByNodeAndStage( int node_number, int stage )
{
    assert( stage < nested_benders_->num_stages_);
    assert( node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
    return nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->col_solution_;
}

const double * OsiStochasticParallelNestedBendersSolverInterface::getFirstStageSolution()
{
    return getSolutionByNodeAndStage(0,0);
}

const double * OsiStochasticParallelNestedBendersSolverInterface::getFirstStageDuals()
{
    return getDualsByNodeAndStage(0,0);
}

const double * OsiStochasticParallelNestedBendersSolverInterface::getDualsByNodeAndStage( int node_number, int stage )
{
    assert( stage < nested_benders_->num_stages_);
    assert( node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
    return nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->dual_solution_;
}

double OsiStochasticParallelNestedBendersSolverInterface::getSolutionValueForScenario( int scenario )
{
    assert( scenario < nested_benders_->num_scenarios_ );
    return nested_benders_->computeScenarioValue(scenario);
}

void OsiStochasticParallelNestedBendersSolverInterface::fixFirstStageVariables( const double* solution )
{
    assert(solution != NULL);
    nested_benders_->fixFirstStageVariables(solution);
}

void OsiStochasticParallelNestedBendersSolverInterface::giveStartSolution( const double* solution )
{
    nested_benders_->setWarmUpSolution(solution);
}

SampleAverageApproximationSolverInterface::SampleAverageApproximationSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model ): osi_pnb_(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model)),parameters_(),samples_(),samples_upper_(),number_batches_(0),number_batches_upper_(0),alpha_(0.05)
{
    parameters_.init();
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
    int num_first_stage_cols = smi_model->getCore()->getNumCols(0);
    int num_stages = smi_model->getCore()->getNumStages();

    parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_DUAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_DUAL_NO);
    parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,0); // Store only first-stage primal solution

    // Define boolean variables to set the method we want to use
    // Set Sampling Method!

    // Set Sampling Method Upper Bound!
    //
    bool batch_means_upper_bound = true;

    // Stores a candidate solution
    boost::shared_array<double> candidate_array;

    bool naive_estimation = false;
    bool mak_morton_wood = false;
    bool linderoth_shapiro_wright = true;

    // Necessary variables for mak_morton_wood
    std::vector<double> gap_estimates;
    std::vector<int> candidate_samples;

    // Necessary variable for linderoth_shapiro_wright
    std::vector<double> upper_bound_estimates;
    


    if ( mak_morton_wood ){
        gap_estimates.resize(number_batches_,0.0);
        candidate_samples = samples_;
        // Use double the size for normal batches for a candidate solution!
        for (std::vector<int>::iterator it = candidate_samples.begin(); it != candidate_samples.end(); ++it)
            *it += *it;
        // Solve a candidate problem
        smi_model->sampleScenarioTree(candidate_samples,0, get_latin_hypercube_random_01_variates );
        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        osi_pnb_->setParameters(parameters_);
        osi_pnb_->initialSolve();
        // Copy first stage solution => candidate solution \hat x
        candidate_array.reset(new double[num_first_stage_cols]);
        memcpy(candidate_array.get(),osi_pnb_->getFirstStageSolution(),sizeof(double)*num_first_stage_cols);
    }

    if ( linderoth_shapiro_wright )
        upper_bound_estimates.resize(number_batches_,0.0);

    std::vector<double> objective_values(number_batches_,0);
    std::vector< boost::shared_array<double> > first_stage_solutions(number_batches_);
    // Compute lower bound approximation!
    for (int i = 0; i < number_batches_; i++){
        smi_model->sampleScenarioTree(samples_,0, get_latin_hypercube_random_01_variates );
        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
        if ( linderoth_shapiro_wright || naive_estimation )
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,0);
        else 
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        osi_pnb_->setParameters(parameters_);
        if ( mak_morton_wood )
            osi_pnb_->giveStartSolution(candidate_array.get());
        else if ( (linderoth_shapiro_wright || naive_estimation) && i > 0)
            osi_pnb_->giveStartSolution(first_stage_solutions[i-1].get());
        osi_pnb_->initialSolve();
        objective_values[i] = osi_pnb_->getObjValue();
        // Needed for linderoth and naive_estimation
        if ( linderoth_shapiro_wright || naive_estimation ) {
            first_stage_solutions[i].reset(new double[num_first_stage_cols]);
            memcpy(first_stage_solutions[i].get(),osi_pnb_->getFirstStageSolution(),sizeof(double)*num_first_stage_cols);
        }
        if ( mak_morton_wood ){
            // Evaluate candidate solution with the given sample set
            osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
            osi_pnb_->fixFirstStageVariables(candidate_array.get());
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
            osi_pnb_->setParameters(parameters_);
            osi_pnb_->initialSolve();
            gap_estimates[i] = osi_pnb_->getObjValue() - objective_values[i];
         }
        if ( linderoth_shapiro_wright ) {
            std::vector<double> upper_bound_estimates_for_this_solution(number_batches_upper_,0.0);
            for (int current_batch = 0; current_batch < number_batches_upper_; current_batch++){
                // Evaluate this very lower bound solution with a larger random number stream!
                // Evaluate candidate solution with the given sample set
                smi_model->sampleScenarioTree(samples_upper_,0, get_latin_hypercube_random_01_variates );
                osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
                osi_pnb_->fixFirstStageVariables(first_stage_solutions[i].get());
                parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
                osi_pnb_->setParameters(parameters_);
                osi_pnb_->initialSolve();
                upper_bound_estimates_for_this_solution[current_batch] = osi_pnb_->getObjValue();
            }
            upper_bound_estimates[i] = std::accumulate(upper_bound_estimates_for_this_solution.begin(),upper_bound_estimates_for_this_solution.end(),0.0)/number_batches_upper_;
        }
    }
    // Now we have number_batches many different objective values

    // Compute lower_bound_estimator
    double lower_bound_estimator = std::accumulate(objective_values.begin(),objective_values.end(),0.0)/number_batches_;
    // Compute Sample Variance Estimator ( is this also allowed for LHS?)
    double sample_variance_estimator = computeSampleVarianceEstimator(number_batches_,&objective_values[0],lower_bound_estimator);
    // Compute symmetric confidence interval half length
    double half_length = computeSymmetricConfidenceInterval(number_batches_,alpha_,sample_variance_estimator);
    // Output everything for lower bound
    LOG(INFO) << "Lower Bound : [ " << lower_bound_estimator-half_length << " <= " << lower_bound_estimator << " <= " << lower_bound_estimator+half_length;
    LOG(INFO) << "Lower Bound : " << lower_bound_estimator << " +/- " << half_length;

    if ( mak_morton_wood ){
        double gap_estimator = std::accumulate(gap_estimates.begin(),gap_estimates.end(),0.0)/number_batches_;
        double sample_gap_variance_estimator = computeSampleVarianceEstimator(number_batches_,&gap_estimates[0],gap_estimator);
        double gap_half_length = computeSymmetricConfidenceInterval(number_batches_,alpha_,sample_gap_variance_estimator);
        LOG(INFO) << " Gap Confidence Interval with " << 1-alpha_ << "% confidence : " << "[0," << gap_estimator+gap_half_length << "]";
    }
    
    // Get a candidate solution. Not necessary for Mak_Morton_Wood, because we already have one!
    if ( naive_estimation ) {
        candidate_array.reset(new double[num_first_stage_cols]);
        CoinFillN(candidate_array.get(),num_first_stage_cols,0.0);
        // Just use the convex combination of the already solved SAA problems
        for (int i = 0; i < number_batches_; i++){
            for (int j = 0; j < num_first_stage_cols; j++)
                candidate_array[j] += first_stage_solutions[i][j] / number_batches_;
        }
    }

    // Get the candidate solution from set of candidates that has lowest upper_bound_estimator!
    if ( linderoth_shapiro_wright ) {
        std::vector<double>::iterator min_element_it = std::min_element(upper_bound_estimates.begin(),upper_bound_estimates.end()); 
        assert( min_element_it != upper_bound_estimates.end());
        int min_element_index = min_element_it-upper_bound_estimates.begin();
        assert( min_element_index >= 0 && min_element_index < num_first_stage_cols);
        candidate_array.reset(new double[num_first_stage_cols]);
        memcpy(candidate_array.get(),first_stage_solutions[min_element_index].get(),sizeof(double)*num_first_stage_cols);
    }
    // Set alpha_ for confidence interval for gap or upper bound computation to a new value, if we use a candidate solution derived from the lower bound estimation!
    

    // "Naive" Upper Bound calculation
    if ( batch_means_upper_bound ) {
        std::vector<double> upper_bound_values(number_batches_upper_,0.0);

        for (int i = 0; i < number_batches_upper_; i++){
            LOG(INFO) << "Solving evaluation problem " << i;
            smi_model->sampleScenarioTree(samples_upper_,0, get_latin_hypercube_random_01_variates );
            osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
            osi_pnb_->setParameters(parameters_);

            // Fix first stage solution
            osi_pnb_->fixFirstStageVariables(candidate_array.get());
            osi_pnb_->initialSolve();
            upper_bound_values[i] = osi_pnb_->getObjValue();

            // Compute sample variance estimator for this problem ( we need the complete solution of the whole problem to be able to compute it )
            // Get the objective values for every scenario
            //std::vector<double> upper_bound_solution_values(samples_upper_[num_stages-1],0);
            //for (int i = 0; i < samples_upper_[num_stages-1];i++){
            //    upper_bound_solution_values[i] = osi_pnb_->getSolutionValueForScenario(i);
            //}
        }
        double upper_bound_estimator = std::accumulate(upper_bound_values.begin(),upper_bound_values.end(),0.0)/number_batches_upper_;        
        double upper_bound_sample_variance_estimator = computeSampleVarianceEstimator(number_batches_upper_,&upper_bound_values[0],upper_bound_estimator);

        // Compute confidence interval for upper bound
        double half_length_upper = computeSymmetricConfidenceInterval(number_batches_upper_,alpha_,upper_bound_sample_variance_estimator); // With Bonferroni this ensures an overall confidence interval of 1-alpha_ TODO: Change alpha_/2 to alpha_ if we use another candidate solution than that by lower bound..
        // Output everything for lower bound
        LOG(INFO) << "Upper Bound : [ " << upper_bound_estimator-half_length_upper << " <= " << upper_bound_estimator << " <= " << upper_bound_estimator+half_length_upper;
    }
    else{
        // We are doing it in a single run. Therefore Sample iid variates!
        smi_model->sampleScenarioTree(samples_upper_,0, get_random_01_variates );
        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_OBJECTIVE_VALUES,true);
        osi_pnb_->setParameters(parameters_);
        // Fix first stage solution
        osi_pnb_->fixFirstStageVariables(candidate_array.get());
        osi_pnb_->initialSolve();

        // Get the objective values for every scenario
        std::vector<double> upper_bound_solution_values(samples_upper_[num_stages-1],0);
        for (int i = 0; i < samples_upper_[num_stages-1];i++){
            upper_bound_solution_values[i] = osi_pnb_->getSolutionValueForScenario(i);
        }
        double upper_bound_estimator = osi_pnb_->getObjValue();      
        double upper_bound_sample_variance_estimator = computeSampleVarianceEstimator(samples_upper_[num_stages-1],&upper_bound_solution_values[0],upper_bound_estimator);

        // Compute confidence interval for upper bound
        double half_length_upper = computeSymmetricConfidenceInterval(samples_upper_[num_stages-1],alpha_/2,upper_bound_sample_variance_estimator); // With Bonferroni this ensures an overall confidence interval of 1-alpha_ TODO: Change alpha_/2 to alpha_ if we use another candidate solution than that by lower bound..
        // Output everything for lower bound
        LOG(INFO) << "Upper Bound : [ " << upper_bound_estimator-half_length_upper << " <= " << upper_bound_estimator << " <= " << upper_bound_estimator+half_length_upper;
    }


    LOG(INFO) << "Done.";
    
    //osi_pnb_->getSmiModel()->writeSmps("saa_latin_hypercube_test");

}

void SampleAverageApproximationSolverInterface::setParameters( const parallel_benders::Parameter& parameters )
{
    osi_pnb_->setParameters(parameters);
    parameters_ = parameters;
}

void SampleAverageApproximationSolverInterface::setSampleSizeUpperBound( const std::vector<int>& sample_size )
{
    assert(!sample_size.empty());
    samples_upper_ = sample_size;
}

void SampleAverageApproximationSolverInterface::setSampleSize( const std::vector<int>& sample_size )
{
    assert(!sample_size.empty());
    samples_ = sample_size;
}

void SampleAverageApproximationSolverInterface::setNumberBatches( int batches )
{
    number_batches_ = batches;
}

void SampleAverageApproximationSolverInterface::setNumberBatchesUpperBound( int batches )
{
    number_batches_upper_ = batches;
}

void SampleAverageApproximationSolverInterface::setConfidenceLevel( double one_minus_alpha )
{
    assert( one_minus_alpha > 0 && one_minus_alpha < 1);
    alpha_ = 1-one_minus_alpha;
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
        if ( i == number_of_variates )
            break;
        
    }
    // Shuffle random_variates randomly. TODO: Use random number generator uniform_int(0,number_of_variates-1) from boost
    std::random_shuffle(&random_variates[0],&random_variates[number_of_variates]);

    assert( i == number_of_variates);
    return random_variates;
}


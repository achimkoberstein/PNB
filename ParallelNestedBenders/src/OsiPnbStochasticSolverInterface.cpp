#include <algorithm>
#include <fstream>
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
#include "ilcplex/cplex.h"
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
    if ( nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ){
        parallel_benders::Timer solution_time(2);
        solution_time.Start(1);
        if ( osi_ != NULL ){
            delete osi_;
        }
        osi_ = nested_benders_->smi_->loadOsiSolverData();
        if ( nested_benders_->smi_->getNumScenarios() == 0){
            // We have no scenarios
            LOG(ERROR) << "No scenarios available.";
        }
        bool use_relaxation = nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_RELAXATION,false);
        // Model statistics
        std::cout << "Variables: " << osi_->getNumCols() << " Rows: " << osi_->getNumRows() << " Elements: " << osi_->getNumElements() << " Integer: " << osi_->getNumIntegers() << std::endl;
        solution_time.Stop(1);
        solution_time.Start(0);
        // Use Barrier method if we use cplex.. ugly code results
#ifdef CPLEX_PRESENT
        OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi_);
        if ( cpx_solver){
            CPXLPptr lpPtr = cpx_solver->getLpPtr(); // translates to getMutableLpPtr ( which does not change the problem )
            CPXENVptr envPtr = cpx_solver->getEnvironmentPtr();
			CPXsetintparam(envPtr,CPX_PARAM_NUMERICALEMPHASIS,1);
            CPXsetintparam(envPtr,CPX_PARAM_SCAIND,1);
            CPXsetdblparam(envPtr,CPX_PARAM_EPMRK,0.9);
            CPXsetdblparam(envPtr,CPX_PARAM_TILIM,nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME,3600));
            // We may want to set a start solution!

            //CPXsetdblparam(envPtr,CPX_PARAM_EPGAP,0.0000001);
            //CPXsetintparam(envPtr,CPX_PARAM_THREADS,1); //?
			// Wee need to differentiate between integer and not
			if ( nested_benders_->smi_->getCore()->getIntegerLength() && !use_relaxation){
				osi_->branchAndBound();
			}
			else {
                //CPXdualopt(envPtr,lpPtr);
                CPXsetintparam(envPtr,CPX_PARAM_BARCROSSALG,-1);
				CPXbaropt(envPtr,lpPtr);
			}
            // Check for errors
            int result = CPXgetstat(envPtr,lpPtr);
            if ( result )
                LOG(INFO) << "Cplex returned with status code " << result;
            double obj;
            CPXgetobjval(envPtr,lpPtr,&obj);
        }
        else{
#endif
            if ( nested_benders_->smi_->getCore()->getIntegerLength() && !use_relaxation){
                osi_->branchAndBound();
            }
            else {
                osi_->initialSolve();
            }

#ifdef CPLEX_PRESENT
        }
#endif
        solution_time.Stop(0);
        // Set solution value
        nested_benders_->upper_bound_ = osi_->getObjValue();
        // 

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
: OsiStochasticSolverInterface(osi_solver,smi_model),nested_benders_(NULL),osi_(NULL)
{
    if ( smi_model == NULL )
        assert(false);
    //this->smi_model_->releaseSolver(); // Forget about previous solver, we do not care about it!
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
    if ( getParameters().parameter_.get(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ){
        if ( node_number > 0 || stage > 0)
            assert(false); // Not yet done
        return osi_->getColSolution(); // TODO: We need to go to the correct numbers ( if we are not at the first node
    }
    else {
        assert( stage < nested_benders_->num_stages_);
        assert( node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
        return nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->col_solution_;
    }
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
    if ( getParameters().parameter_.get(parallel_benders::PNB_BENDERS,1) == parallel_benders::PNB_INT_ALGORITHM_DETEQ){
        return osi_->getRowPrice(); // TODO: We need to go to the correct numbers..
    }
    else {
        assert( stage < nested_benders_->num_stages_);
        assert( node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
        return nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->dual_solution_;
    }
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
    // Store a warm_up solution
    // If cplex, we may need to think about it?
    nested_benders_->setWarmUpSolution(solution);
}

SampleAverageApproximationSolverInterface::SampleAverageApproximationSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model ): osi_pnb_(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model)),parameters_(),samples_(),samples_upper_(),number_batches_(0),number_batches_upper_(0),alpha_(0.05)
{
    parameters_.init();
    // Initialize random generator
    unsigned int seed;

    // How it works: We need a random generator and a distribution we want to draw numbers from.
    // Set the seed 
    uniform_generator_struct.seed = get_random_number_from_system();
    // Create a random number generator instance
    uniform_generator_struct.rng = boost::shared_ptr<base_generator_type>(new base_generator_type(seed));
    // Create a random [0,1) variate generator;
    boost::uniform_01<> uniform_01_distribution;
    uniform_generator_struct.uniform = boost::shared_ptr<uniform_distr_gen>(new uniform_distr_gen(*uniform_generator_struct.rng.get(),uniform_01_distribution));
}

void SampleAverageApproximationSolverInterface::initialSolve()
{


    // Some things we need to share later on
    OsiSolverInterface* osi_solver = osi_pnb_->getOsiSolverInterface();
    SmiScnModel* smi_model = osi_pnb_->getSmiModel();
    int num_first_stage_cols = smi_model->getCore()->getNumCols(0);
    int num_stages = smi_model->getCore()->getNumStages();

    parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_DUAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_DUAL_NO);
    parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,0); // Store only first-stage primal solution

    bool dont_use_benders = parameters_.parameter_.get(parallel_benders::PNB_BENDERS,1) != parallel_benders::PNB_INT_ALGORITHM_BENDERS;
    int preferred_solution_algorithm = parameters_.parameter_.get(parallel_benders::PNB_BENDERS,1);

    double lower_bound_estimator = 0;
    double half_length_lower = 0;

    double upper_bound_estimator = 0;
    double half_length_upper = 0;

    // Define boolean variables to set the method we want to use
    // Set Sampling Method!

    // Set Sampling Method Upper Bound!
    bool batch_means_upper_bound = true;

    // Stores a candidate solution
    boost::shared_array<double> candidate_array;

    // Possibilities to estimate upper bound and/or gap
    bool naive_estimation = false;
    bool mak_morton_wood = false;
    bool linderoth_shapiro_wright = true;

    // Necessary variables for mak_morton_wood
    std::vector<double> gap_estimates;
    std::vector<int> candidate_samples;

    // Necessary variable for linderoth_shapiro_wright
    std::vector<double> upper_bound_estimates;
    

    // Solve a first problem to get a candidate solution for the upper bounding
    if ( mak_morton_wood ){
        gap_estimates.resize(number_batches_,0.0);
        candidate_samples = samples_;
        // Use double the size for normal batches for a candidate solution!
        for (std::vector<int>::iterator it = candidate_samples.begin(); it != candidate_samples.end(); ++it)
            *it += *it;
        // Solve a candidate problem
        smi_model->sampleScenarioTree(candidate_samples,0, get_latin_hypercube_random_01_variates ); // TODO: Is this allowed here?
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
        if ( linderoth_shapiro_wright || naive_estimation ) // In this case we need first stage solutions only
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,0);
        else // For MakMortonWood we need no solutions
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        osi_pnb_->setParameters(parameters_);
        // Use solution from previous problem to speed up calculations 
        if ( mak_morton_wood )
            osi_pnb_->giveStartSolution(candidate_array.get());
        else if ( (linderoth_shapiro_wright || naive_estimation) && i > 0)
            osi_pnb_->giveStartSolution(first_stage_solutions[i-1].get());
        osi_pnb_->initialSolve();
        // We assume that solving went well. If it does not, we have problem
        objective_values[i] = osi_pnb_->getObjValue();
        LOG(INFO) << "Solved lower bound problem " << i << " with objective value " << objective_values[i];
        // Needed for linderoth and naive_estimation
        if ( linderoth_shapiro_wright || naive_estimation ) {
            first_stage_solutions[i].reset(new double[num_first_stage_cols]);
            memcpy(first_stage_solutions[i].get(),osi_pnb_->getFirstStageSolution(),sizeof(double)*num_first_stage_cols);
        }

        // Now we compute values for upper bounding.
        parameters_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS); // Use Benders for evaluation of candidate solution!
        if ( mak_morton_wood ){
            LOG(INFO) << "Evaluating candidate solution with the current sampled scenario tree.";
            // Evaluate candidate solution with the given sample set
            osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
            osi_pnb_->setParameters(parameters_);
            osi_pnb_->fixFirstStageVariables(candidate_array.get());
            osi_pnb_->initialSolve();
            gap_estimates[i] = osi_pnb_->getObjValue() - objective_values[i];
         }
        if ( linderoth_shapiro_wright ) {
            
            std::vector<double> upper_bound_estimates_for_this_solution(number_batches_upper_,0.0);
            for (int current_batch = 0; current_batch < number_batches_upper_; current_batch++){
                LOG(INFO) << "Solving evaluation problem " << current_batch << " for current lower bound solution " << i;
                // Evaluate this very lower bound solution with a larger random number stream!
                smi_model->sampleScenarioTree(samples_upper_,0, get_latin_hypercube_random_01_variates );
                osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
                parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
                osi_pnb_->setParameters(parameters_);
                osi_pnb_->fixFirstStageVariables(first_stage_solutions[i].get());
                osi_pnb_->initialSolve();
                upper_bound_estimates_for_this_solution[current_batch] = osi_pnb_->getObjValue();
            }
            upper_bound_estimates[i] = std::accumulate(upper_bound_estimates_for_this_solution.begin(),upper_bound_estimates_for_this_solution.end(),0.0)/number_batches_upper_;
        }
        // Evaluation of solutions is done with Benders, but we have to switch back for the lower bounding problems!
        if ( dont_use_benders )
            parameters_.parameter_.put(parallel_benders::PNB_BENDERS,preferred_solution_algorithm);
    }
    // Now we have number_batches many different objective values

    // Compute lower_bound_estimator
    lower_bound_estimator = std::accumulate(objective_values.begin(),objective_values.end(),0.0)/number_batches_;
    // Compute Sample Variance Estimator ( is this also allowed for LHS?)
    double sample_variance_estimator = computeSampleVarianceEstimator(number_batches_,&objective_values[0],lower_bound_estimator);
    // Compute symmetric confidence interval half length
    half_length_lower = computeSymmetricConfidenceInterval(number_batches_,alpha_,sample_variance_estimator);
    // Output everything for lower bound
    LOG(INFO) << "Lower Bound : [ " << lower_bound_estimator-half_length_lower << " <= " << lower_bound_estimator << " <= " << lower_bound_estimator+half_length_lower;
    LOG(INFO) << "Lower Bound : " << lower_bound_estimator << " +/- " << half_length_lower;

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
        int min_element_index = std::distance(upper_bound_estimates.begin(),min_element_it); // Works because of vector
        assert( min_element_index >= 0 && min_element_index < number_batches_);
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
            parameters_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
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
        upper_bound_estimator = std::accumulate(upper_bound_values.begin(),upper_bound_values.end(),0.0)/number_batches_upper_;        
        double upper_bound_sample_variance_estimator = computeSampleVarianceEstimator(number_batches_upper_,&upper_bound_values[0],upper_bound_estimator);

        // Compute confidence interval for upper bound
        half_length_upper = computeSymmetricConfidenceInterval(number_batches_upper_,alpha_,upper_bound_sample_variance_estimator); // With Bonferroni this ensures an overall confidence interval of 1-alpha_ TODO: Change alpha_/2 to alpha_ if we use another candidate solution than that by lower bound..
        // Output everything for lower bound
        LOG(INFO) << "Upper Bound : [ " << upper_bound_estimator-half_length_upper << " <= " << upper_bound_estimator << " <= " << upper_bound_estimator+half_length_upper;
    }
    else{
        // We are doing it in a single run. Therefore Sample iid variates!
        smi_model->sampleScenarioTree(samples_upper_,0, get_random_01_variates );
        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_OBJECTIVE_VALUES,true);
        parameters_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
        osi_pnb_->setParameters(parameters_);
        // Fix first stage solution
        osi_pnb_->fixFirstStageVariables(candidate_array.get());
        osi_pnb_->initialSolve();

        // Get the objective values for every scenario
        std::vector<double> upper_bound_solution_values(samples_upper_[num_stages-1],0);
        for (int i = 0; i < samples_upper_[num_stages-1];i++){
            upper_bound_solution_values[i] = osi_pnb_->getSolutionValueForScenario(i);
        }
        upper_bound_estimator = osi_pnb_->getObjValue();      
        double upper_bound_sample_variance_estimator = computeSampleVarianceEstimator(samples_upper_[num_stages-1],&upper_bound_solution_values[0],upper_bound_estimator);

        // Compute confidence interval for upper bound
        half_length_upper = computeSymmetricConfidenceInterval(samples_upper_[num_stages-1],alpha_/2,upper_bound_sample_variance_estimator); // With Bonferroni this ensures an overall confidence interval of 1-alpha_ TODO: Change alpha_/2 to alpha_ if we use another candidate solution than that by lower bound..
        // Output everything for lower bound
        LOG(INFO) << "Upper Bound : [ " << upper_bound_estimator-half_length_upper << " <= " << upper_bound_estimator << " <= " << upper_bound_estimator+half_length_upper;
        LOG(INFO) << "Upper Bound : " << upper_bound_estimator << " +/- " << half_length_upper;
    }
    // Conclusion
    LOG(INFO) << "Lower Bound : " << lower_bound_estimator << " +/- " << half_length_lower;
    LOG(INFO) << "Upper Bound : " << upper_bound_estimator << " +/- " << half_length_upper;

    std::fstream logfile;
    logfile.open("saa-log", fstream::out | fstream::app);
    // Conclusion
    logfile << "N : ";
    for (int i = 1; i < num_stages; i++)
            logfile << samples_[i] << " ";
    logfile << std::endl;
    logfile << "M : " << number_batches_ << std::endl;
    logfile << "bar N : ";
    for (int i = 1; i < num_stages; i++)
        logfile << samples_upper_[i] << " ";
    logfile << std::endl;
    logfile << "T: " << number_batches_upper_ << std::endl;
    logfile << "Lower Bound : " << lower_bound_estimator << " +/- " << half_length_lower << std::endl;
    logfile << "Upper Bound : " << upper_bound_estimator << " +/- " << half_length_upper << std::endl;
    logfile << "Confidence Level : " << 1-alpha_ << "%" << std::endl;
    logfile.close();

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


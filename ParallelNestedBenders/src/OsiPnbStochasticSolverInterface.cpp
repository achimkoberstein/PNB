#include <algorithm>
#include <fstream>
#include <numeric>
#include <vector>
#include <iostream>
#define GOOGLE_GLOG_DLL_DECL // Has to be done to prevent linking issues!
#include <glog/logging.h>

#include <boost/shared_array.hpp>
#include <boost/math/distributions/students_t.hpp>

#include "CoinHelperFunctions.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"

#include "PseudoRandomGenerator.hpp"
#include "SmiScnModel.hpp"
#include "ParallelNestedBenders.hpp"
#include "BendersNodeData.hpp"

#include "CoinTime.hpp";

#define CPLEX_PRESENT
#ifdef CPLEX_PRESENT
#include "ilcplex/cplex.h"
#include "OsiCpxSolverInterface.hpp"
#endif




void OsiStochasticParallelNestedBendersSolverInterface::setParameters(const parallel_benders::Parameter& parameters){
    nested_benders_->parameter_ = parameters;
}

void OsiStochasticParallelNestedBendersSolverInterface::setInstanceSpecsFile(const std::string& filename) {
	OsiStochasticSolverInterface::setInstanceSpecsFile(filename);
	nested_benders_->instance_specification_file_ = filename;
}

void OsiStochasticParallelNestedBendersSolverInterface::setTestsetProblemInstanceNames(std::string testset, std::string problem, std::string instance) {
	
	// to lower case
	std::vector<std::reference_wrapper<std::string>> arguments = { testset, problem, instance };
	for (auto& s : arguments) {
		std::transform(s.get().begin(), s.get().end(), s.get().begin(),
			[](unsigned char c) { return std::tolower(c); } // correct
		);
	}

	nested_benders_->testset_name_ = testset;
	nested_benders_->problem_name_ = problem;
	nested_benders_->instance_name_ = instance;
}

std::vector<std::string> OsiStochasticParallelNestedBendersSolverInterface::getTestsetProblemInstanceNames() const {

	std::vector<std::string> result;
	result.push_back(nested_benders_->testset_name_);
	result.push_back(nested_benders_->problem_name_);
	result.push_back(nested_benders_->instance_name_);

	return result;
}

void OsiStochasticParallelNestedBendersSolverInterface::setTestsetProblemInstanceNames(std::string path) {
	// Split path to testset, problem, instances
	std::vector<std::string> splitted_path(3, "NA");
	int elem_counter(0);
	std::size_t current(0);
	std::size_t previous(path.size());
	current = path.rfind("/");
	while (current != std::string::npos && elem_counter < 3) {
		splitted_path[elem_counter] = path.substr(current + 1, previous - current);
		elem_counter++;
		previous = current - 1;
		current = path.rfind("/", previous);
	}
	if (splitted_path[0] == "NA") {
		splitted_path[0] = path;
	}
	
	setTestsetProblemInstanceNames(splitted_path[2], splitted_path[1], splitted_path[0]);
}

double OsiStochasticParallelNestedBendersSolverInterface::getObjValue() const
{
    if ( nested_benders_->cvar_constr_decomp_ )
        return nested_benders_->cvar_problem_lower_bound_; // Is equal to normal lower bound
    return nested_benders_->upper_bound_; // Depending on the solution status code, this might be appropriate!
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumIterations() const
{
    return nested_benders_->current_iteration_;
}
int OsiStochasticParallelNestedBendersSolverInterface::getNumInsubstantialIterations() const
{
    return nested_benders_->insubstantial_iteration_;
}
double OsiStochasticParallelNestedBendersSolverInterface::getUBForIteration(int iteration) const
{
	LOG_IF(FATAL, iteration > getNumIterations()) << "iteration arg in getUbForIteration() should be between 1 and number of iterations.";
    assert(iteration <= nested_benders_->iteration_u_and_l_bounds_.size() && "In some cases Benders stops earlier and does not record ub and lb for the last iteration. It should have been fixed.");
	
	return nested_benders_->iteration_u_and_l_bounds_[iteration - 1].first;
}
double OsiStochasticParallelNestedBendersSolverInterface::getLBForIteration(int iteration) const
{
	LOG_IF(FATAL, iteration > getNumIterations()) << "iteration arg in getLbForIteration() should be between 1 and number of iterations.";
    assert(iteration <= nested_benders_->iteration_u_and_l_bounds_.size() && "In some cases Benders stops earlier and does not record ub and lb for the last iteration. It should have been fixed.");

	return nested_benders_->iteration_u_and_l_bounds_[iteration - 1].second;
}
double OsiStochasticParallelNestedBendersSolverInterface::getUB() const
{
	return nested_benders_->upper_bound_;
}
double OsiStochasticParallelNestedBendersSolverInterface::getLB() const
{
	return nested_benders_->lower_bound_;
}

double OsiStochasticParallelNestedBendersSolverInterface::getRelativeGap() const
{
	return parallel_benders::ComputeRelativeGap(nested_benders_->upper_bound_, nested_benders_->lower_bound_, 1e-10);
}

void OsiStochasticParallelNestedBendersSolverInterface::initialSolve()
{
    if ( nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ || nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ_RISK ){

        bool deteq_risk = nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ_RISK;
        parallel_benders::Timer solution_time(2);
        solution_time.Start(1);
        if ( osi_ != nullptr ){
            delete osi_;
        }
        if ( deteq_risk ) {
            // Get Risk parameters!
            double cvar_beta = nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_CVAR_BETA,0.1);
            double cvar_rho = nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_CVAR_RHO,0.0);
            bool cvar_first_stage = nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_CVAR_WITH_FIRST_STAGE,false);
            nested_benders_->smi_->setCVaRConstraint(true,cvar_beta,cvar_rho,cvar_first_stage,false);
        }

		if (nested_benders_->parameter_.GetBoolParamValue(parallel_benders::PNB_BOUNDS_IN_MATRIX)) {
			nested_benders_->smi_->moveBoundsIntoMatrix();
		}

		LOG_IF(WARNING, nested_benders_->parameter_.GetIntParamValue(parallel_benders::PNB_USE_START_SOLUTION)) << "Start solution is not implemented for deteq.";

        osi_ = nested_benders_->smi_->loadOsiSolverData();
        if ( nested_benders_->smi_->getNumScenarios() == 0){
            // We have no scenarios
            LOG(ERROR) << "No scenarios available.";
        }
        int relaxation = nested_benders_->parameter_.GetIntParamValue(parallel_benders::PNB_RELAXATION);
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

			//CPXsetintparam(envPtr,CPX_PARAM_NUMERICALEMPHASIS,1);
            //CPXsetintparam(envPtr,CPX_PARAM_SCAIND,1);
			CPXsetintparam(envPtr,CPX_PARAM_THREADS,nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_NUM_CPU,0));
            //CPXsetdblparam(envPtr,CPX_PARAM_EPMRK,0.999);
            CPXsetdblparam(envPtr,CPX_PARAM_TILIM,nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME,3600));
            //CPXsetintparam(envPtr,CPX_PARAM_PREIND,0);
            // We may want to set a start solution!

			//PNB_TOLERANCE_ZERO
            CPXsetdblparam(envPtr,CPX_PARAM_EPRHS,nested_benders_->parameter_.parameter_.get(parallel_benders::PNB_TOLERANCE_ZERO,1e-06)); // Feasibility tolerance
            //CPXsetintparam(envPtr,CPX_PARAM_THREADS,1); //?

			//PNB_TOLERANCE_INTEGER
			CPXsetdblparam(envPtr, CPX_PARAM_EPINT, nested_benders_->parameter_.GetDblParamValue(parallel_benders::PNB_TOLERANCE_INTEGER));
			
			//PNB_EPSILON_ABSOLUTE
			double absolute_epsilon = nested_benders_->parameter_.GetDblParamValue(parallel_benders::PNB_EPSILON_ABSOLUTE);
			CPXsetdblparam(envPtr, CPX_PARAM_EPOPT, absolute_epsilon); //Optimality tolerance
			CPXsetdblparam(envPtr, CPX_PARAM_BAREPCOMP, absolute_epsilon); //Convergence tolerance for LP and QP problems
			CPXsetdblparam(envPtr, CPX_PARAM_EPAGAP, absolute_epsilon); //Absolute mipgap tolerance
			CPXsetdblparam(envPtr, CPX_PARAM_NETEPOPT, absolute_epsilon); //Optimality tolerance for network optimization

			//PNB_EPSILON_RELATIVE
			CPXsetdblparam(envPtr, CPX_PARAM_EPGAP, nested_benders_->parameter_.GetDblParamValue(parallel_benders::PNB_EPSILON_RELATIVE)); //Relative MIP gap tolerance

			// We need to differentiate between integer and not
			int num_ints = nested_benders_->smi_->getIntegerLen();
			if (num_ints && (!relaxation || relaxation > nested_benders_->core_->getNumStages() )){ // no relaxation
				osi_->branchAndBound();
			}
			else if ( num_ints && relaxation > 1) { // relax integers for stages starting from PNB_RELAXATION stage			
				int relaxation_int_start = 0;
				for (int stage_index = 0; stage_index < relaxation - 1; stage_index++) {
					relaxation_int_start += nested_benders_->core_->getIntCols(stage_index).size();
				}
				int num_ints_relax = num_ints - relaxation_int_start;
				if ( num_ints_relax ) {
					const std::vector<int>& int_indices = nested_benders_->smi_->getIntIndices();
					osi_->setContinuous(&int_indices[relaxation_int_start], int_indices.size() - relaxation_int_start);
					LOG(INFO) << "We solve the LP relaxation for stages starting from stage " << relaxation << ".";
				}
				osi_->branchAndBound();
			}
			else { // not integer (nor binary) or we relax all stages
				int lp_method = nested_benders_->parameter_.GetIntParamValue(parallel_benders::PNB_LP_METHOD);
				if (lp_method == parallel_benders::PNB_INT_BARRIER){
					osi_solver_->setHintParam(OsiDoDualInInitial, true, OsiHintIgnore); // barrier and barrier w crossover
					osi_solver_->setHintParam(OsiDoDualInResolve, true, OsiHintIgnore);
					CPXsetintparam(envPtr,CPX_PARAM_BARCROSSALG,2); // Do not use crossover
					CPXbaropt(envPtr,lpPtr);
				}
				else if (lp_method == parallel_benders::PNB_INT_DUAL_SIMPLEX){
					CPXdualopt(envPtr,lpPtr);
				}
				else if (lp_method == parallel_benders::PNB_INT_PRIMAL_SIMPLEX){
					CPXprimopt(envPtr,lpPtr);
				}
				else if (lp_method == parallel_benders::PNB_INT_BARRIER_WITH_CROSSOVER){
					osi_solver_->setHintParam(OsiDoDualInInitial, true, OsiHintIgnore);
					osi_solver_->setHintParam(OsiDoDualInResolve, true, OsiHintIgnore);
					CPXsetintparam(envPtr,CPX_PARAM_BARCROSSALG,0); // Choose to use crossover
					CPXbaropt(envPtr,lpPtr);
				}
				else if (lp_method == parallel_benders::PNB_INT_CONCURRENT_MASTER_STANDARD) {
					CPXsetintparam(envPtr, CPX_PARAM_LPMETHOD, CPX_ALG_CONCURRENT);
					CPXdualopt(envPtr, lpPtr);
				}
			}

			if (nested_benders_->parameter_.GetBoolParamValue(parallel_benders::PNB_PRINT_SUBPROBLEMS)) {
				CPXwriteprob(envPtr, lpPtr, "modeloutput.lp", "lp");
			}

            // Check for errors
			int CPXresult = CPXgetstat(envPtr, lpPtr);
			if (osi_solver_->isProvenOptimal()) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_OPTIMAL;
				nested_benders_->lower_bound_ = osi_->getObjValue();
			}
			// sometimes isProvenOptimal() does not work correct and returns false when it should be true. Check it "manually"
			else if (CPXresult == CPX_STAT_OPTIMAL || CPXresult == CPX_STAT_OPTIMAL_INFEAS || CPXresult == CPXMIP_OPTIMAL || CPXresult == CPXMIP_OPTIMAL_TOL) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_OPTIMAL;
				nested_benders_->lower_bound_ = osi_->getObjValue();
			}
			else if (osi_solver_->isAbandoned()) { // can have multiple reasons
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_SUBSOLVER_PROBLEM;
				nested_benders_->lower_bound_ = -INFINITY;
			}
			else if (osi_solver_->isProvenPrimalInfeasible() || osi_solver_->isProvenDualInfeasible()) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_INFEASIBLE;
				nested_benders_->lower_bound_ = -INFINITY;
			}
			else if (osi_solver_->isDualObjectiveLimitReached() || osi_solver_->isProvenDualInfeasible()) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_INFEASIBLE;
				nested_benders_->lower_bound_ = -INFINITY;
			}
			else if (osi_solver_->isIterationLimitReached()) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_ITERATION_LIMIT;
				nested_benders_->lower_bound_ = -INFINITY;
			}
			else if (CPXresult == CPX_STAT_UNBOUNDED || CPXresult == CPXMIP_UNBOUNDED) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_UNBOUNDED;
				nested_benders_->lower_bound_ = -INFINITY;
			}
			else if (CPXresult == CPX_STAT_ABORT_TIME_LIM || CPXresult == CPXMIP_TIME_LIM_FEAS || CPXresult == CPXMIP_TIME_LIM_INFEAS) {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_TIME_LIMIT;
				nested_benders_->lower_bound_ = -INFINITY;
			}
			else {
				nested_benders_->solution_code_ = parallel_benders::PNB_SOLUTION_SUBSOLVER_PROBLEM;
			}
			LOG_IF(INFO, nested_benders_->solution_code_ != parallel_benders::PNB_SOLUTION_OPTIMAL) << "Cplex returned with status code " << CPXresult;

            double obj;
            CPXgetobjval(envPtr,lpPtr,&obj);
        }
        else{
#endif
            if ( nested_benders_->smi_->getIntegerLen() && relaxation != 0){
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
        

        // We have the time, but can not set it accordingly. not good!
        LOG(INFO) << "Solution: " << nested_benders_->upper_bound_;
        LOG(INFO) << "Solution Time: " << solution_time.GetTime(0) << "s";

    }
    else {
		if (nested_benders_->parameter_.GetBoolParamValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS)) {
			setInstanceSpecsFile(instance_specification_file_);
		}
        nested_benders_->Init();
        nested_benders_->Solve();

        // If we quit (e.g., time limit) during the iteration in which we would generate L&P cuts, LB and UB were not added to the iteration_u_and_l_bounds_ vector
        if (nested_benders_->generate_lp_cuts_) {
            nested_benders_->iteration_u_and_l_bounds_.push_back(make_pair(nested_benders_->upper_bound_, nested_benders_->lower_bound_));
        }
    }
}

OsiStochasticParallelNestedBendersSolverInterface::OsiStochasticParallelNestedBendersSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model )
: OsiStochasticSolverInterface(osi_solver,smi_model),nested_benders_(nullptr),osi_(nullptr)
{
    if ( smi_model == nullptr )
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
    assert(nested_benders_);
	return nested_benders_->parameter_;
}

const parallel_benders::Timer& OsiStochasticParallelNestedBendersSolverInterface::getTimer() const
{
    assert(nested_benders_);
    return nested_benders_->timer_;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumOptimalityCuts() const 
{
    assert(nested_benders_);
    int cut_counter = 0;
    if ( !nested_benders_->stage_data_.empty())
        for (int i = 0; i < nested_benders_->num_stages_; i++){
            cut_counter += nested_benders_->stage_data_[i]->added_optimality_cuts_;
        }
    return cut_counter;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumFeasibilityCuts() const 
{
    assert(nested_benders_);
    int cut_counter = 0;
    if ( !nested_benders_->stage_data_.empty())
    for (int i = 0; i < nested_benders_->num_stages_; i++){
        cut_counter += nested_benders_->stage_data_[i]->added_feasibility_cuts_;
    }
    return cut_counter;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumEpigraphCuts() const
{
    assert(nested_benders_);
    int cut_counter = 0;
    if (!nested_benders_->stage_data_.empty())
        for (int i = 0; i < nested_benders_->num_stages_; i++) {
            cut_counter += nested_benders_->stage_data_[i]->added_epigraph_cuts_;
        }
    return cut_counter;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumDomainCuts() const
{
    assert(nested_benders_);
    int cut_counter = 0;
    if (!nested_benders_->stage_data_.empty())
        for (int i = 0; i < nested_benders_->num_stages_; i++) {
            cut_counter += nested_benders_->stage_data_[i]->added_domain_cuts_;
        }
    return cut_counter;
}

void OsiStochasticParallelNestedBendersSolverInterface::printProblemCharacteristics() const
{
    assert(nested_benders_);
    if ( osi_solver_ ){
        double cols = osi_solver_->getNumCols();
        double rows = osi_solver_->getNumRows();
        double elems  = osi_solver_->getNumElements();
        LOG(INFO) << "Cols: " << cols << " Rows: " << rows << " Nonzeroes: " << elems;
    }
}

SmiScnModel* OsiStochasticParallelNestedBendersSolverInterface::getSmiModel()
{
    assert(nested_benders_);
    return nested_benders_->smi_;
}

parallel_benders::ParallelNestedBenders* OsiStochasticParallelNestedBendersSolverInterface::getPNB()
{
    assert(nested_benders_);
    return nested_benders_;
}

const double * OsiStochasticParallelNestedBendersSolverInterface::getSolutionByNodeAndStage( int node_number, int stage ) const
{
    assert(nested_benders_);
    if ( getParameters().parameter_.get(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ){
        if ( node_number > 0 || stage > 0)
            assert(false); // Not yet done
        return osi_->getColSolution(); 
    }
    else {
        assert( stage < nested_benders_->num_stages_);
        assert( node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
        if ( nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->incumbent_col_solution_.empty() )
            return nullptr;
        else
            return &nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->incumbent_col_solution_[0];
    }
}

const double *  OsiStochasticParallelNestedBendersSolverInterface::getFirstStageSolution()
{
    return getSolutionByNodeAndStage(0,0);
}

//const double * OsiStochasticParallelNestedBendersSolverInterface::getFirstStageDuals()
//{
//    return getDualsByNodeAndStage(0,0);
//}
//
//const double * OsiStochasticParallelNestedBendersSolverInterface::getDualsByNodeAndStage( int node_number, int stage ) const
//{
//    if ( getParameters().parameter_.get(parallel_benders::PNB_BENDERS,1) == parallel_benders::PNB_INT_ALGORITHM_DETEQ){
//        return osi_->getRowPrice(); // TODO: We need to go to the correct numbers..
//    }
//    else {
//        assert( stage < nested_benders_->num_stages_);
//        assert( node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
//        return nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->dual_solution_;
//    }
//}

double OsiStochasticParallelNestedBendersSolverInterface::getSolutionValueForScenario( int scenario )
{
    assert( scenario < nested_benders_->num_scenarios_ );
    return nested_benders_->computeScenarioValue(scenario);
}

void OsiStochasticParallelNestedBendersSolverInterface::fixFirstStageVariables( const double* solution )
{
    assert(solution != nullptr);
    nested_benders_->fixFirstStageVariables(solution);
}

void OsiStochasticParallelNestedBendersSolverInterface::giveStartSolution( const double* solution, int num_cols )
{
    assert(nested_benders_);
    // Store a warm_up solution
    // If cplex, we may need to think about it?
    nested_benders_->setWarmUpSolution(solution,num_cols);
}

void OsiStochasticParallelNestedBendersSolverInterface::printSolutionToFile( const char* filename ) const
{
	if ( nested_benders_->compute_primal_solutions_for_stage_ < 0 )
		return;
    if ( nested_benders_->solution_code_ != 0){
        LOG(ERROR) << "Solution can not be printed, because algorithm did not ended with optimality.";
        return;
    }
	// We want to print the solution we got by solving the model, to the file! Therefore we need a solved problem first!
	std::ofstream solution_file(filename, fstream::out | fstream::trunc  ); // Open the file, regardless if it exists or not!
	LOG_IF(ERROR,!solution_file.is_open()) << "Solution file " << filename << " could not be opened to write.";
	// Get the values for which we actually want to store the solution from the param file
	assert( nested_benders_->compute_primal_solutions_for_stage_ >= 0);
	assert( nested_benders_->compute_primal_solutions_for_stage_ <= nested_benders_->num_stages_);
	// At first, print a summary of the solution value
	solution_file << "Objective Value : " << nested_benders_->upper_bound_ << "\n";
	for (int i = 0; i < nested_benders_->num_stages_; i++){
		// Sum over all nodes at this stage!
		double obj_value = 0.0;
		for (int scen = 0; scen < nested_benders_->tree_nodes_by_stage_[i].size();scen++){
			obj_value += nested_benders_->tree_nodes_by_stage_[i][scen]->getDataPtr()->incumbent_solution_value_;
		}
		solution_file << "Objective Value Stage " << i << " : " << obj_value << "\n";
	}
	solution_file << "Column Values sorted by Stage and Scenario" << "\n";
	for ( int stage = 0; stage < nested_benders_->compute_primal_solutions_for_stage_; stage++){
		for (int scen = 0; scen < nested_benders_->tree_nodes_by_stage_[stage].size();scen++){
			solution_file << "Objective Value Stage-Scenario=" <<  stage << "-" << scen << " : " <<  nested_benders_->tree_nodes_by_stage_[stage][scen]->getDataPtr()->incumbent_solution_value_ << "\n";
			const double*  primal_solution = getSolutionByNodeAndStage(scen,stage);
			assert( primal_solution != nullptr);
			// Write out objective value for stage and scenario wise (probability discounted)
			// Get the correct name for the variable in question via smi!
			int column_offset = smi_model_->getCore()->getColStart(stage);
			for ( int current_column = 0; current_column < nested_benders_->stage_data_[stage]->num_columns_; current_column++){
                if ( abs(primal_solution[current_column]) >= nested_benders_->zero_tolerance_ )
				solution_file << "Stage-Scenario=" << stage << "-" << scen << " : " << smi_model_->getColumnName(current_column+column_offset) << " : " << primal_solution[current_column] << "\n";
			}
			
		}
	}
	solution_file.close();

}

int OsiStochasticParallelNestedBendersSolverInterface::getSolutionCode() const
{
    assert(nested_benders_);
    return nested_benders_->solution_code_;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumInfeasibilityIterations() const 
{
    assert(nested_benders_);
    return getNumIterations()-nested_benders_->feasible_iteration_; // Feasible Iterations
}

const parallel_benders::Timer& OsiStochasticParallelNestedBendersSolverInterface::getBuildTimer() const
{
    assert(nested_benders_);
    return nested_benders_->detailed_timer_;
}

const parallel_benders::Timer& OsiStochasticParallelNestedBendersSolverInterface::getComputingTimer() const
{
    assert(nested_benders_);
    return nested_benders_->solution_times_;
}

int OsiStochasticParallelNestedBendersSolverInterface::getNumCriticalIteration() const
{
    assert(nested_benders_);
    return nested_benders_->uncritical_iteration_counter_;
}

int OsiStochasticParallelNestedBendersSolverInterface::getMaximalLengthOfCriticalIterationSequence() const
{
    assert(nested_benders_);
    return nested_benders_->maximal_number_of_uncritical_iterations_;
}
SampleAverageApproximationSolverInterface::SampleAverageApproximationSolverInterface( OsiSolverInterface* osi_solver, SmiScnModel* smi_model ): osi_pnb_(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model)),parameters_(),samples_(),samples_upper_(),number_batches_(0),number_batches_upper_(0),alpha_(0.05)
{
    parameters_.init();
}

void SampleAverageApproximationSolverInterface::initialSolve()
{

	parallel_benders::Parameter initial_parameter = parameters_;

	double time_lower_bounding = 0;
	double time_upper_bounding = 0;
	double time_sampling = 0;
	double current_time = 0;
    // Some things we need to share later on
    OsiSolverInterface* osi_solver = osi_pnb_->getOsiSolverInterface();
    SmiScnModel* smi_model = osi_pnb_->getSmiModel();
    int num_first_stage_cols = smi_model->getCore()->getNumCols(0);
    int num_stages = smi_model->getCore()->getNumStages();

	// We set the solutions that we want to get!
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
		current_time = CoinCpuTime();
        smi_model->sampleScenarioTree(candidate_samples,0, get_latin_hypercube_random_01_variates ); // TODO: Is this allowed here?
		time_sampling += CoinCpuTime()-current_time;

        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
		parameters_ = initial_parameter;
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        osi_pnb_->setParameters(parameters_);
		
		current_time = CoinCpuTime();
        osi_pnb_->initialSolve();
		time_lower_bounding += CoinCpuTime()-current_time;

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
		current_time = CoinCpuTime();
        smi_model->sampleScenarioTree(samples_,0, get_latin_hypercube_random_01_variates );
		time_sampling += CoinCpuTime()-current_time;

        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
		parameters_ = initial_parameter;
        if ( linderoth_shapiro_wright || naive_estimation ) // In this case we need first stage solutions only
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,1);
        else // For MakMortonWood we need no solutions
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
   
        // Use solution from previous problem to speed up calculations 
		if ( i == 0 ) {
			parameters_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_EV_SOLUTION);
		}
		else {
			parameters_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_SUPPLIED_SOLUTION);
			if ( mak_morton_wood )
				osi_pnb_->giveStartSolution(candidate_array.get());
			else if ( (linderoth_shapiro_wright || naive_estimation) && i > 0)
				osi_pnb_->giveStartSolution(first_stage_solutions[i-1].get());
		}
		osi_pnb_->setParameters(parameters_);

		current_time = CoinCpuTime();
        osi_pnb_->initialSolve();
		time_lower_bounding += CoinCpuTime()-current_time;

        // We assume that solving went well. If it does not, we have problem
        objective_values[i] = osi_pnb_->getObjValue();
        LOG(INFO) << "Solved lower bound problem " << i << " with objective value " << objective_values[i];
        // Needed for linderoth and naive_estimation
        if ( linderoth_shapiro_wright || naive_estimation ) {
            first_stage_solutions[i].reset(new double[num_first_stage_cols]);
            memcpy(first_stage_solutions[i].get(),osi_pnb_->getFirstStageSolution(),sizeof(double)*num_first_stage_cols);
        }

        // Now we compute values for upper bounding. Only evaluating current solution!
		parameters_ = initial_parameter;
        parameters_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS); // Use Benders for evaluation of candidate solution!
        if ( mak_morton_wood ){
            LOG(INFO) << "Evaluating candidate solution with the current sampled scenario tree.";
            // Evaluate candidate solution with the given sample set
            osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
			parameters_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_NO_START_SOLUTION);
            osi_pnb_->setParameters(parameters_);
            osi_pnb_->fixFirstStageVariables(candidate_array.get());

			current_time = CoinCpuTime();
            osi_pnb_->initialSolve();
			time_upper_bounding += CoinCpuTime()-current_time;

            gap_estimates[i] = osi_pnb_->getObjValue() - objective_values[i];
         }
        if ( linderoth_shapiro_wright ) {
			std::vector<double> upper_bound_estimates_for_this_solution(number_batches_upper_,0.0);
            for (int current_batch = 0; current_batch < number_batches_upper_; current_batch++){
                LOG(INFO) << "Solving evaluation problem " << current_batch << " for current lower bound solution " << i;
                // Evaluate this very lower bound solution with a larger random number stream!

				current_time = CoinCpuTime();
                smi_model->sampleScenarioTree(samples_upper_,0, get_latin_hypercube_random_01_variates );
				time_sampling += CoinCpuTime()-current_time;

                osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
                parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
				parameters_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_NO_START_SOLUTION);
                osi_pnb_->setParameters(parameters_);
                osi_pnb_->fixFirstStageVariables(first_stage_solutions[i].get());

				current_time = CoinCpuTime();
                osi_pnb_->initialSolve();
				time_upper_bounding += CoinCpuTime()-current_time;

                upper_bound_estimates_for_this_solution[current_batch] = osi_pnb_->getObjValue();
            }
            upper_bound_estimates[i] = std::accumulate(upper_bound_estimates_for_this_solution.begin(),upper_bound_estimates_for_this_solution.end(),0.0)/number_batches_upper_;
        }

    }
    // Now we have number_batches many different objective values

    // Compute lower_bound_estimator
    lower_bound_estimator = std::accumulate(objective_values.begin(),objective_values.end(),0.0)/number_batches_;
    // Compute Sample Variance Estimator ( is this also allowed for LHS?)
    double sample_variance_estimator = parallel_benders::computeSampleVarianceEstimator(number_batches_,&objective_values[0],lower_bound_estimator);
    // Compute symmetric confidence interval half length
    half_length_lower = parallel_benders::computeSymmetricConfidenceInterval(number_batches_,alpha_,sample_variance_estimator);
    // Output everything for lower bound
    LOG(INFO) << "Lower Bound : [ " << lower_bound_estimator-half_length_lower << " <= " << lower_bound_estimator << " <= " << lower_bound_estimator+half_length_lower;
    LOG(INFO) << "Lower Bound : " << lower_bound_estimator << " +/- " << half_length_lower;

    if ( mak_morton_wood ){
        double gap_estimator = std::accumulate(gap_estimates.begin(),gap_estimates.end(),0.0)/number_batches_;
        double sample_gap_variance_estimator = parallel_benders::computeSampleVarianceEstimator(number_batches_,&gap_estimates[0],gap_estimator);
        double gap_half_length = parallel_benders::computeSymmetricConfidenceInterval(number_batches_,alpha_,sample_gap_variance_estimator);
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
			current_time = CoinCpuTime();
            smi_model->sampleScenarioTree(samples_upper_,0, get_latin_hypercube_random_01_variates );
			time_sampling += CoinCpuTime()-current_time;

            osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
            parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
            parameters_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
            osi_pnb_->setParameters(parameters_);

            // Fix first stage solution
            osi_pnb_->fixFirstStageVariables(candidate_array.get());

			current_time = CoinCpuTime();
            osi_pnb_->initialSolve();
			time_upper_bounding += CoinCpuTime()-current_time;

            upper_bound_values[i] = osi_pnb_->getObjValue();

            // Compute sample variance estimator for this problem ( we need the complete solution of the whole problem to be able to compute it )
            // Get the objective values for every scenario
            //std::vector<double> upper_bound_solution_values(samples_upper_[num_stages-1],0);
            //for (int i = 0; i < samples_upper_[num_stages-1];i++){
            //    upper_bound_solution_values[i] = osi_pnb_->getSolutionValueForScenario(i);
            //}
        }
        upper_bound_estimator = std::accumulate(upper_bound_values.begin(),upper_bound_values.end(),0.0)/number_batches_upper_;        
        double upper_bound_sample_variance_estimator = parallel_benders::computeSampleVarianceEstimator(number_batches_upper_,&upper_bound_values[0],upper_bound_estimator);

        // Compute confidence interval for upper bound
        half_length_upper = parallel_benders::computeSymmetricConfidenceInterval(number_batches_upper_,alpha_,upper_bound_sample_variance_estimator); // With Bonferroni this ensures an overall confidence interval of 1-alpha_ TODO: Change alpha_/2 to alpha_ if we use another candidate solution than that by lower bound..
        // Output everything for lower bound
        LOG(INFO) << "Upper Bound : [ " << upper_bound_estimator-half_length_upper << " <= " << upper_bound_estimator << " <= " << upper_bound_estimator+half_length_upper;
    }
    else{
        // We are doing it in a single run. Therefore Sample iid variates!
		current_time = CoinCpuTime();
        smi_model->sampleScenarioTree(samples_upper_,0, get_random_01_variates );
		time_sampling += CoinCpuTime()-current_time;

        osi_pnb_.reset(new OsiStochasticParallelNestedBendersSolverInterface(osi_solver,smi_model));
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS,parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
        parameters_.parameter_.put(parallel_benders::PNB_COMPUTE_OBJECTIVE_VALUES,true);
        parameters_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
        osi_pnb_->setParameters(parameters_);
        // Fix first stage solution
        osi_pnb_->fixFirstStageVariables(candidate_array.get());
		current_time = CoinCpuTime();
        osi_pnb_->initialSolve();
		time_upper_bounding += CoinCpuTime()-current_time;

        // Get the objective values for every scenario
        std::vector<double> upper_bound_solution_values(samples_upper_[num_stages-1],0);
        for (int i = 0; i < samples_upper_[num_stages-1];i++){
            upper_bound_solution_values[i] = osi_pnb_->getSolutionValueForScenario(i);
        }
        upper_bound_estimator = osi_pnb_->getObjValue();      
        double upper_bound_sample_variance_estimator = parallel_benders::computeSampleVarianceEstimator(samples_upper_[num_stages-1],&upper_bound_solution_values[0],upper_bound_estimator);

        // Compute confidence interval for upper bound
        half_length_upper = parallel_benders::computeSymmetricConfidenceInterval(samples_upper_[num_stages-1],alpha_/2,upper_bound_sample_variance_estimator); // With Bonferroni this ensures an overall confidence interval of 1-alpha_ TODO: Change alpha_/2 to alpha_ if we use another candidate solution than that by lower bound..
        // Output everything for lower bound
        LOG(INFO) << "Upper Bound : [ " << upper_bound_estimator-half_length_upper << " <= " << upper_bound_estimator << " <= " << upper_bound_estimator+half_length_upper;
        LOG(INFO) << "Upper Bound : " << upper_bound_estimator << " +/- " << half_length_upper;
    }
    // Conclusion
    LOG(INFO) << "Lower Bound : " << lower_bound_estimator << " +/- " << half_length_lower;
    LOG(INFO) << "Upper Bound : " << upper_bound_estimator << " +/- " << half_length_upper;

    std::fstream logfile;
    logfile.open("saa-log.txt", fstream::out | fstream::app);
	LOG(INFO) << theGenerator().seed;
    logfile << "Seed: " << theGenerator().seed << "\n";
    // Conclusion
    logfile << "N : ";
    for (int i = 1; i < num_stages; i++)
            logfile << samples_[i] << " ";
    logfile << "\n";
    logfile << "M : " << number_batches_ << "\n";
    logfile << "bar N : ";
    for (int i = 1; i < num_stages; i++)
        logfile << samples_upper_[i] << " ";
    logfile << "\n";
    logfile << "T: " << number_batches_upper_ << "\n";
    logfile << "Lower Bound : " << setprecision(10) << setw(10) << lower_bound_estimator << " +/- " << half_length_lower << "\n";
    logfile << "Upper Bound : " << setprecision(10) << setw(10) << upper_bound_estimator << " +/- " << half_length_upper << "\n";
    logfile << "Confidence Level : " << 1-alpha_ << "%" << "\n";
	logfile << "Time Lower Bounding :" << time_lower_bounding << "\n";
	logfile << "Time Upper Bounding :" << time_upper_bounding << "\n";
	logfile << "Time Sampling : " << time_sampling << "\n";
    logfile.close();

    LOG(INFO) << "Done.";
    
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
    if ( theGenerator().seed != 0 )
        random_variate = (*theGenerator().uniform)();

    return random_variate;
}

// Crude Monte Carlo Versions!
double * get_random_01_variates(int number_of_variates){
    assert( number_of_variates > 0);
    if ( theGenerator().seed == 0)
        return nullptr;

    double* random_variates = new double[number_of_variates];
    for (int i = 0; i < number_of_variates; i++)
        random_variates[i] = (*theGenerator().uniform)();
    return random_variates;
}

// Antithetic Crude Monte Carlo Sampling 
double * get_antithetic_random_01_variates(int number_of_variates){
    assert( number_of_variates > 0);
    if ( theGenerator().seed == 0)
        return nullptr;

    double* random_variates = new double[number_of_variates];
    for (int i = 0; i < number_of_variates; i++){
        if ( i % 2 == 0 )
            random_variates[i] = (*theGenerator().uniform)();
        else
            random_variates[i] = 1-random_variates[i-1];
    }
    return random_variates;
}

// Latin Hypercube Sampling
double * get_latin_hypercube_random_01_variates(int number_of_variates){
    assert( number_of_variates > 0);
    if ( theGenerator().seed == 0)
        return nullptr;

    // Generate one variate uniformly at random in every interval
    double* random_variates = new double[number_of_variates];
    double division = 1.0/number_of_variates;

    int i = 0;
    for ( double lower_interval = 0.0; lower_interval < 1; lower_interval += division){
        // Sample one variable from [lower_interval,lower_interval+division)
        boost::uniform_real<> uniform_real_distribution(lower_interval,lower_interval+division);
        boost::variate_generator<parallel_benders::base_generator_type*, boost::uniform_real<double> > variate_gen(theGenerator().rng.get(),uniform_real_distribution);
        random_variates[i] = variate_gen();
        assert( random_variates[i] >= lower_interval && random_variates[i] < lower_interval+division);
        i++;
        if ( i == number_of_variates )
            break;
        
    }
    // Shuffle random_variates randomly. 
    // Invoke Functor to sample from the RNG we use for our other random numbers too.
    //ptrdiff_t (*p_myrandom)(ptrdiff_t) = getRandomNumberFromRNG;
    boost::uniform_int<> uniform_int_distribution;
    boost::variate_generator<parallel_benders::base_generator_type&, boost::uniform_int<> > variate_gen(*(theGenerator().rng.get()),uniform_int_distribution);
    std::random_shuffle(&random_variates[0],&random_variates[number_of_variates],variate_gen);

    assert( i == number_of_variates);
    return random_variates;
}

ptrdiff_t getRandomNumberFromRNG( ptrdiff_t maximal_number )
{
    // Sample one variable from [0,maximal_number)
    boost::uniform_int<> uniform_int_distribution(0,maximal_number);
    boost::variate_generator<parallel_benders::base_generator_type&, boost::uniform_int<> > variate_gen(*(theGenerator().rng.get()),uniform_int_distribution);
    return variate_gen();
}

#include "ParallelNestedBenders.hpp"

#include <assert.h>
#include <array>
#include <stdlib.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <utility> // std::pair

#include <mutex>

#include <math.h>
#include <sstream>


#include <boost/lexical_cast.hpp>
#include <boost/multi_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/math/special_functions/round.hpp>

#define GOOGLE_GLOG_DLL_DECL // Has to be done to prevent linking issues!
#include <glog/logging.h>
// Include all the other files

#include "CoinHelperFunctions.hpp"
#include "CoinPackedMatrix.hpp"
#include "CoinWarmStart.hpp"
#include "CoinWarmStartBasis.hpp"
#include "OsiRowCut.hpp"
#include "OsiSolverInterface.hpp"
#include "OsiCpxSolverInterface.hpp"
#include "SmiScnModel.hpp"
#include "SmiCoreCombineRule.hpp"
#include "CoinTime.hpp"
#include "PseudoRandomGenerator.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"
#include "ilcplex/cpxconst.h"
//#include "ilcplex/cplexcheck.h"
//#include "check.c"
#include "OsiCpxSolverInterface.hpp"
#include "BranchAndBendersCut.hpp"


namespace branch_and_benders_cut {

	int CPXPUBLIC benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle)
	{
		// see bendersatsp2.c in CPLEX examples
	
		BranchAndBendersCut* global_data = static_cast<BranchAndBendersCut*>(cbhandle);		
		
		int status = 0;
		int thread_id;
		status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &thread_id);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetinfoint: status = %d\n", status);
			return status;
		}

		BranchAndBendersCut* thread_data = global_data->getCopyForThread(thread_id);

		if (contextid == CPX_CALLBACKCONTEXT_THREAD_UP && !thread_data) {
			global_data->createCopyForThread(thread_id);
		}
		else if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE) 
		{	
			thread_data->current_stage_ = 0;
			thread_data->forward_ = true;

			// Check if solution is bounded
			int integer_feasible;
			status = CPXcallbackcandidateispoint(context, &integer_feasible);
			if (status) {
				fprintf(stderr, "Error in CPXXcallbackcandidateispoint: status = %d\n", status);
				return status;
			}
			if (!integer_feasible) {
				/* The model is bounded, so this can only happend 
				if theta cuts were not added to the model formulation.
				They were probably added as lazy cuts.*/ // TODO: is it also true for user cuts
				fprintf(stderr, "Unbounded solution\n");
				status = CPXERR_UNSUPPORTED_OPERATION;
				return status;
			}

			//Store Data
			status = thread_data->StoreNodeData(context, contextid);
			if (status) {
				fprintf(stderr, "Error in bbc_->StoreNodeData(): status = %d\n", status);
				return status;
			}

			// Generate optimality cuts and add them as lazy cuts
			int num_theta_cuts = thread_data->stage_data_[0]->added_optimality_cuts_ + thread_data->stage_data_[0]->added_epigraph_cuts_;
			thread_data->ConstructOptimalityCuts(); // and calculate Benders ub, lb
			int added_theta_cuts = thread_data->stage_data_[0]->added_optimality_cuts_ + thread_data->stage_data_[0]->added_epigraph_cuts_ - num_theta_cuts;
			// if violated cuts are found: add them and reject the integer candidate solution
			// if no violated cuts found (or ub = lb), no cuts are appended and candidate is excepted
			thread_data->AppendCutsToNode(context, thread_data->tree_nodes_by_stage_[thread_data->current_stage_ - 1][0]);
			thread_data->DeleteSubproblem(nullptr, thread_data->tree_nodes_by_stage_[thread_data->current_stage_ - 1][0]);

			// Root node is never solved in parallel - its data is "global"
			// If root is integer right away,
			// context CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS might never be called.
			// We need to set UB here
			// TODO: distinguish between root is integer vs root is integer + Benders optimal
			int node_id;
			status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &node_id);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetinfoint: status = %d\n", status);
				return status;
			}
			if (node_id == 0 && !added_theta_cuts)
			{
				assert(thread_id == 0);
				global_data->benders_optimal_incumbents_.push_back(thread_data->benders_optimal_incumbents_.back());
			}
			// TODO: what is with LB - should we store it in CPX_CALLBACKCONTEXT_RELAXATION. It seems to be wrong when relaxation solution == MIP
			// try to get it using branching context https://www.ibm.com/docs/en/icos/20.1.0?topic=context-cpx-callbackcontext-branching in addition to relaxation context

			for (auto node : thread_data->tree_nodes_by_stage_[thread_data->current_stage_]) {
				node->getDataPtr()->CleanUnusedMemory(thread_data->forward_);
			}
		}
		/*else if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
		// Do not forget to set this context for callback
		{
			thread_data->current_stage_ = 0;
			thread_data->forward_ = true;
			//Store Data
			status = thread_data->StoreNodeData(context, contextid);
			if (status) {
				fprintf(stderr, "Error in bbc_->StoreNodeData(): status = %d\n", status);
				return status;
			}

			// Generate L&P cuts
			assert(lp_round_ == 0);
			assert(current_stage_ == 0); // TODO: which stage do we need here?
			// TODO: temp
			lp_rounds_limit_ = 1;
			for (lp_round_ = 1; lp_round_ <= lp_rounds_limit_; lp_round_++) {
				osi_solver_->initialSolve();
				if (osi_solver_->isProvenOptimal()) {
					std::cout << "Optimal" << std::endl;
				}
				else if (osi_solver_->isProvenDualInfeasible()) {
					std::cout << "Unbounded" << std::endl;
				}
				generate_lp_cuts_ = true;
				StoreData(osi_solver_, tree_nodes_by_stage_[0][0]);

				if (!generate_lp_cuts_) {
					// solution was integer
					return;
				}
				ConstructLiftAndProjectCuts();
				current_stage_ = 0;
				assert(false && "Should it be parallel_benders::PNB_CUT_APPEND_TYPE_USER? or add to model formulation");
				AppendCutsToSubproblem(osi_solver_, tree_nodes_by_stage_[0][0], parallel_benders::PNB_CUT_APPEND_TYPE_LAZY);
			}
		}*/
		else if (contextid == CPX_CALLBACKCONTEXT_LOCAL_PROGRESS) {
			double thread_lower_bound;
			status = CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_BND, &thread_lower_bound);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetinfodbl: status = %d\n", status);
				return status;
			}
			// TOOD: collect them somewhere for local_data, is not it here?
			thread_data->best_bounds_.push_back(thread_lower_bound);
		}
		else if (contextid == CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS) {
			// TODO: is it so important for us. Maybe do not gather this data or gather it in THREAD_DOWN
			//this context locks all threads, be fast!
			//results of CPXcallbackgetinfo___ are global here
			//TODO: probably we do not need to return the status here because CPLEX will not consider it due to the contextid

			int ub_available;
			status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_FEASIBLE, &ub_available);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetinfodbl: status = %d\n", status);
				return status;
			}

			if (ub_available) {
				double global_upper_bound;
				status = CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &global_upper_bound);
				if (status) {
					fprintf(stderr, "Error in CPXcallbackgetinfodbl: status = %d\n", status);
					return status;
				}
				global_data->benders_optimal_incumbents_.push_back(global_upper_bound);
			}			

			double global_lower_bound;
			status = CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_BND, &global_lower_bound);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetinfodbl: status = %d\n", status);
				return status;
			}
			global_data->best_bounds_.push_back(global_lower_bound);
		}
		/*else if (contextid == CPX_CALLBACKCONTEXT_THREAD_DOWN) {
			// TODO: are other threads locked?
			// copy cuts and whatever else to the global_data

			// We delete thread_data in destructor because the thread with the same id
			// can be up and down multiple times. Choose time efficiency for memory efficiency.
		}*/

		return 0;
	} /* END benders_callback */
	
	BranchAndBendersCut::BranchAndBendersCut(SmiScnModel* smi_model_ptr)
		: parallel_benders::ParallelNestedBenders(smi_model_ptr),benders_optimal_incumbents_(),best_bounds_(),thread_copy_map_(),thread_copy_map_mutex_()
	{
	}

	BranchAndBendersCut::~BranchAndBendersCut()
	{
		for (auto id_copy_pair : thread_copy_map_) {
			delete id_copy_pair.second;
			id_copy_pair.second = nullptr;
		}
	}

	void BranchAndBendersCut::Init()
	{
		ParallelNestedBenders::Init();
		benders_optimal_incumbents_.push_back(infinity_);
		best_bounds_.push_back(-infinity_);
	}

	void BranchAndBendersCut::Solve()
	{
		smi_->writeSmps("modeloutput");

		if (solution_code_ == parallel_benders::PNB_SOLUTION_PARAMETER_PROBLEM) {
			LOG_IF(ERROR, log_level_ <= google::ERROR) << "Cannot solve because of the problem in parameters.";
			return;
		}
		if (thread_pool_.size() == 0) {
			solution_code_ = parallel_benders::PNB_SOLUTION_PARAMETER_PROBLEM;
			solved_counter_++;
			return;
		}

		OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
		LOG_IF(FATAL, !cpx_solver) << "osi_solver_ should be OsiCpxSolverInterface for BranchAndBendersCut";

		parallel_benders::BendersNodeData* mp_node_pointer = tree_nodes_by_stage_[0][0]->getDataPtr();
		

		// Construct continuous optimality, L&P cuts or
		// one round of Benders cuts (to avoid unboundness)
		assert(current_stage_ == 0);
		BuildSubproblem(osi_solver_, tree_nodes_by_stage_[0][0]);
		AppendColsToSubproblem(osi_solver_, tree_nodes_by_stage_[0][0]);
		ConstructRootNodeCuts();

		// Second Phase: Branch-And-Benders-Cut
		SetUpMPSolver(cpx_solver);
		PrintSubproblem(cpx_solver, 0, "MP_before");
		SolveSolverInterface(cpx_solver);
		current_stage_ = 0;
		PrintSubproblem(cpx_solver, 0, "MP_after");
		
		if (cpx_solver->isProvenOptimal()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_OPTIMAL;
			assert(parallel_benders::CompareForEquality(benders_optimal_incumbents_.back(), cpx_solver->getObjValue(), 0.001));
			// TODO: not sure why, but sometimes LB is -infinity
			if (!parallel_benders::CompareForEquality(best_bounds_.back(), benders_optimal_incumbents_.back(), relative_epsilon_) &&
				benders_optimal_incumbents_.back() > best_bounds_.back())
			{
				best_bounds_.push_back(benders_optimal_incumbents_.back());
			}			
		}
		else if (cpx_solver->isProvenPrimalInfeasible()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_INFEASIBLE;
		}
		else if (cpx_solver->isProvenDualInfeasible()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_UNBOUNDED;
		}
		else if (cpx_solver->isPrimalObjectiveLimitReached()) {
			LOG(FATAL) << "Unknown solution code";
		}
		else if (cpx_solver->isDualObjectiveLimitReached()) {
			LOG(FATAL) << "Unknown solution code";
		}
		else if (cpx_solver->isIterationLimitReached()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_ITERATION_LIMIT;
		}
		else if (cpx_solver->isTimeLimitReached()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_TIME_LIMIT;
		}
		else {
			LOG(FATAL) << "Unknown solution code";
		}

		if (pretty_printing_)
			std::cout << "\n";
		// Output the final values once again!
		if (log_level_ <= google::INFO) {
			LOG(INFO) << "Algorithm finished with status code " << solution_code_ << ", final values below.";
			LOG(INFO) << "Lower Bound:  " << best_bounds_.back() << "\n" << "Upper Bound:  " << benders_optimal_incumbents_.back() << "\n" << "Relative Gap: " << parallel_benders::ComputeRelativeGap(benders_optimal_incumbents_.back(), best_bounds_.back());
			//LOG(INFO) << "Major Iterations: " << major_iteration_ << "\n" << "Overall Iterations: " << current_iteration_;
			int cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += stage_data_[i]->added_optimality_cuts_;
			}
			// TODO: we need to calculate here cuts for continuous phase
			// TODO: we need to calculate here cuts for all bbc copies
			LOG(INFO) << "Added " << cut_counter << " optimality cuts.";
			cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += stage_data_[i]->added_feasibility_cuts_;
			}
			LOG(INFO) << "Added " << cut_counter << " feasibility cuts.";
			if (lp_rounds_limit_) { // if we use L&P cuts
				cut_counter = 0;
				for (int i = 0; i < num_stages_; i++) {
					cut_counter += stage_data_[i]->added_epigraph_cuts_;
				}
				LOG(INFO) << "Added " << cut_counter << " L&P epigraph cuts.";
				cut_counter = 0;
				for (int i = 0; i < num_stages_; i++) {
					cut_counter += stage_data_[i]->added_domain_cuts_;
				}
				LOG(INFO) << "Added " << cut_counter << " L&P domain cuts.";
			}
		}
	}

	int BranchAndBendersCut::StoreNodeData(CPXCALLBACKCONTEXTptr context, CPXLONG contextid)
	{
		assert(current_stage_ == 0);
		int status = 0;

		parallel_benders::BendersNodeData* mp_node_pointer = tree_nodes_by_stage_[0][0]->getDataPtr();
		int num_x_cols = mp_node_pointer->cols_;
		int num_theta = mp_node_pointer->num_thetas_;
		if (!num_theta) {
			num_theta = getNumThetasByStage(tree_nodes_by_stage_[0][0]);
		}
		assert(mp_node_pointer->num_thetas_ == getNumThetasByStage(tree_nodes_by_stage_[0][0]));

		std::vector<double> solution(num_x_cols + num_theta);

		if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE) {
			status = CPXcallbackgetcandidatepoint(context, solution.data(), 0, num_x_cols + num_theta - 1, NULL);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetcandidate: status = %d\n", status);
				return status;
			}
		}
		else if (contextid == CPX_CALLBACKCONTEXT_RELAXATION) {
			status = CPXcallbackgetrelaxationpoint(context, solution.data(), 0, num_x_cols + num_theta - 1, NULL);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetrelaxationpoint: status = %d\n", status);
				return status;
			}
		}

		mp_node_pointer->col_solution_.resize(num_x_cols);
		mp_node_pointer->col_solution_.assign(solution.begin(), solution.begin()+ num_x_cols);
		mp_node_pointer->next_stage_approximation_.resize(num_theta);
		mp_node_pointer->next_stage_approximation_.assign(solution.begin() + num_x_cols, solution.end());

		double solution_value = 0;
		if (mp_node_pointer->smi_node_data_->getObjectiveLength()) {
			mp_node_pointer->objective_ = new double[stage_data_[0]->num_columns_];
			memcpy(mp_node_pointer->objective_, core_->getDenseObjCoefficients(0), stage_data_[0]->num_columns_ * sizeof(double));
			if (stage_data_[0]->stochastic_objective_)
				for (int i = 0; i < mp_node_pointer->smi_node_data_->getObjectiveLength(); i++) {
					mp_node_pointer->objective_[mp_node_pointer->smi_node_data_->getObjectiveIndices()[i] - core_->getColStart(0)] = mp_node_pointer->smi_node_data_->getObjectiveElements()[i];
				}
		}
		else
			mp_node_pointer->objective_ = const_cast<double*> (core_->getDenseObjCoefficients(0));
		const double* obj_coefs = mp_node_pointer->objective_;

		int i = 0;
		for (; i < mp_node_pointer->cols_; i++) {
			solution_value += mp_node_pointer->col_solution_[i] * obj_coefs[i];
		}
		mp_node_pointer->solution_value_ = solution_value; // q(x): solution w/o thetas

		// Add all thetas to the c*x solution to get lower bound for node at this stage
		for (double& theta_value : mp_node_pointer->next_stage_approximation_) { 
			solution_value += theta_value;
			i++;
		}
		mp_node_pointer->lower_bound_ = solution_value; //  q(x) + theta: solution with thetas
		
		stage_old_lower_upper_bounds_[0].first = stage_lower_upper_bounds_[0].first;
		stage_old_lower_upper_bounds_[0].second = stage_lower_upper_bounds_[0].second;
		stage_lower_upper_bounds_[0].first = mp_node_pointer->lower_bound_; //  q(x) + theta: solution with thetas
		stage_lower_upper_bounds_[0].second = mp_node_pointer->solution_value_; // q(x): solution w/o thetas
		old_lower_bound_ = lower_bound_;
		lower_bound_ = mp_node_pointer->lower_bound_;	
		// UB will be calculated in ConstructOptimalityCuts()
		// after 2nd-stage subproblems are evaluated

		return status;
	}

	void BranchAndBendersCut::ConstructOptimalityCuts()
	{
		current_stage_ = 1;
		forward_ = false;
		int num_nodes = tree_nodes_by_stage_[current_stage_].size();
		
		for (int i = 0; i < num_nodes; i++) {
			thread_pool_.schedule(boost::bind(&BranchAndBendersCut::HandleSubproblem, this, tree_nodes_by_stage_[current_stage_][i]));
			//HandleSubproblem(tree_nodes_by_stage_[current_stage_][i]);
		}
		thread_pool_.wait(0); // We wait until all threads are done

		LOG_IF(FATAL, num_added_feasibility_cuts_) << "Feasibility cuts not implemented (not tested)";

		// Calculate UB
		// Could be simpler but let us keep it similar to pnb
		stage_old_lower_upper_bounds_[1].first = stage_lower_upper_bounds_[1].first;
		stage_old_lower_upper_bounds_[1].second = stage_lower_upper_bounds_[1].second;
		stage_lower_upper_bounds_[1].first = 0.0;
		stage_lower_upper_bounds_[1].second  = 0.0;
		for (auto node : tree_nodes_by_stage_[1]) {
			// q(x) * prob
			stage_lower_upper_bounds_[1].first += node->getDataPtr()->lower_bound_ * node->getDataPtr()->probability_;;
			stage_lower_upper_bounds_[1].second += node->getDataPtr()->solution_value_ * node->getDataPtr()->probability_;
		}
		assert(stage_lower_upper_bounds_[1].first == stage_lower_upper_bounds_[1].second && "UB and LB for the 2nd stage are identical");
		upper_bound_ = tree_nodes_by_stage_[0][0]->getDataPtr()->solution_value_ + stage_lower_upper_bounds_[1].second;
		// LB was updated in StoreNodeData()
		
		// Compare UB and LB for Benders optimality
		bool benders_not_optimal = parallel_benders::RelativeGapLargerEpsilon(upper_bound_, lower_bound_, infinity_, relative_epsilon_) &&
			lower_bound_ < upper_bound_ &&
			upper_bound_ - lower_bound_ >= absolute_epsilon_;
		if (!benders_not_optimal) {
			// Do not aggregate cuts
			// as a result, no new cuts constructed
			// and the current integer B&B candidate will be accepted
			benders_optimal_incumbents_.push_back(upper_bound_);
			return; 
		}

		// We need to aggregate optimality cuts for the parent node
		AggregateOptimalityCuts(tree_nodes_by_stage_[current_stage_ - 1][0]);		
	}

	void BranchAndBendersCut::ConstructRootNodeCuts()
	{
		assert(current_stage_ == 0);  // TODO: which stage do we need here?

		// *****************
		// Generate L&P cuts
		// *****************
		if (lp_rounds_limit_) {
			// Prepare parameters
			parallel_benders::Parameter lift_and_project_param = parallel_benders::Parameter(parameter_);
			lift_and_project_param.SetParameterValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS, false);
			lift_and_project_param.SetParameterValue(parallel_benders::PNB_PRINT_SUBPROBLEMS, false);

			// Create pnb for L&P cuts
			ParallelNestedBenders* lift_and_project_pnb = new ParallelNestedBenders(smi_);
			lift_and_project_pnb->setParameters(lift_and_project_param);
			lift_and_project_pnb->setTestsetProblemInstanceNames(testset_name_, problem_name_, instance_name_ + "_ROOT_LP_CUTS");
			lift_and_project_pnb->getMutableConsoleMutex() = console_mutex_ptr_; // we share the console as the resource
			lift_and_project_pnb->Init();

#ifdef NDEBUG
			lift_and_project_pnb->log_level_ = google::WARNING; // no INFO output for cont phase in release
#endif
			// Find solution
			// We stop the process after the total number of L&P rounds
			// reaches lp_rounds_limit_ (PNB_SOLUTION_GLOBAL_LP_ROUNDS_LIMIT status)
			lift_and_project_pnb->Solve();

			//parallel_benders::PNB_SOLUTION_OPTIMAL, "PNB_SOLUTION_OPTIMAL" - this is optimal for our bbc too. See HandleContinuousPhase
			// TODO: implement status PNB_SOLUTION_GLOBAL_LP_ROUNDS_LIMIT, we should only solve LP MP to get to it here
			//any other status is an error			

			// Move cuts to bbc:
			// - optimality (incl. PNB CGP cuts)
			// - feasibility
			// - epigraph
			// - domain
			// - continuous optimality cuts if use_first_phase_
			std::vector<parallel_benders::BendersNodeData::cut_container*> cut_containers;
			cut_containers.push_back(&lift_and_project_pnb->getTreeNode(0,0)->getDataPtr()->optimality_cuts_);					
			cut_containers.push_back(&lift_and_project_pnb->getTreeNode(0,0)->getDataPtr()->feasibility_cuts_);
			assert(lift_and_project_pnb->getTreeNode(0, 0)->getDataPtr()->feasibility_cuts_.empty() && "Not tested");
			cut_containers.push_back(&lift_and_project_pnb->getTreeNode(0,0)->getDataPtr()->epigraph_cuts_);
			assert(!lift_and_project_pnb->getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.empty() && "Why we have no epigraph L&P cuts?");
			cut_containers.push_back(&lift_and_project_pnb->getTreeNode(0,0)->getDataPtr()->domain_cuts_);
			assert(lift_and_project_pnb->getTreeNode(0, 0)->getDataPtr()->domain_cuts_.empty() && "Not tested");
			for (auto& cut_container : cut_containers) {
				for (auto& cut_stuff : *cut_container) {
					cut_stuff->cut_creation_iteration_ = 0;
					AddCutToContainer(tree_nodes_by_stage_[0][0], cut_stuff);
				}
			}
			
			// Copy warm start of the 2nd stage
			if (warm_start_) {
				CoinWarmStart* ws;
				if (lift_and_project_pnb->getStageData(1)->warm_start_) {
					ws = (lift_and_project_pnb->getStageData(1)->warm_start_.get())->clone();
					stage_data_[1]->warm_start_.reset(ws); // deep copy of warm start
				}
				for (int scenario = 0; scenario < tree_nodes_by_stage_[0].size(); scenario++) {
					if (lift_and_project_pnb->getTreeNode(1, scenario)->getDataPtr()->warm_start_) {
						ws = (lift_and_project_pnb->getTreeNode(1, scenario)->getDataPtr()->warm_start_.get())->clone(); // deep copy
						tree_nodes_by_stage_[1][scenario]->getDataPtr()->warm_start_.reset(ws);
					}
				}
			}
		}
		// *****************
		// Generate continuous optimality cuts
		// *****************
		// If we use L&P rounds,
		// cont. optimality cuts will be constructed in lift_and_project_pnb above
		else if (use_first_phase_)
		{
			// TODO: should we add Cont cuts as user cuts?
			current_iteration_ = 1;
			HandleContinuousPhase();
		}

		// If no theta cuts are present, generate them.
		// Without theta cuts the problem is unbounded as theta is unbounded.
		if (getTreeNode(0,0)->getDataPtr()->optimality_cuts_.empty() && getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.empty()) {
			osi_solver_->initialSolve();
			assert(osi_solver_->isProvenOptimal());
			StoreData(osi_solver_, getMutableTreeNode(0, 0));
			ConstructOptimalityCuts();
		}

		LOG_IF(FATAL, !getTreeNode(0,0)->getDataPtr()->feasibility_cuts_.empty()) << "Feasibility cuts not implemented (not tested)";
		LOG_IF(FATAL, !getTreeNode(0,0)->getDataPtr()->domain_cuts_.empty()) << "Domain cuts not implemented (not tested)";

		// Add all cuts from above
		current_stage_ = 0;
		// call this method to activate thetas,
		// i.e., set their coef to 1
		AppendColsToSubproblem(osi_solver_, getMutableTreeNode(0, 0));
		// add cuts from containers to the formulation of the MP
		// TODO: add them as user cuts? Lazy cuts lead to unboundness for root node
		AppendCutsToSubproblem(osi_solver_, tree_nodes_by_stage_[0][0]);
		DeleteSubproblem(nullptr, tree_nodes_by_stage_[0][0]);
	}

	void BranchAndBendersCut::ConstructLiftAndProjectCuts()
	{
		assert(false && "not tested");
		current_stage_ = 0;
		forward_ = false;
		int num_nodes = tree_nodes_by_stage_[current_stage_].size();

		int num_cuts = ceil(fractional_variables_.size() * (cuts_for_fract_vars_ / 100.0)); //at least cut for one fract variable
		assert(num_cuts > 0 && num_cuts <= fractional_variables_.size());
		stage_data_[current_stage_]->lp_cgp_counter_ += num_cuts; // every cut requires CGP to solve

		double theta_sum = std::accumulate(tree_nodes_by_stage_[0][0]->getDataPtr()->next_stage_approximation_.begin(), tree_nodes_by_stage_[0][0]->getDataPtr()->next_stage_approximation_.end(), 0.0);
		LOG_IF(INFO, log_level_ <= google::INFO) << current_iteration_ << " Iteration - L&P Switch " << lp_switch_counter_ << "/" << lp_switches_limit_ << " - L&P round " << lp_round_ << "/" << lp_rounds_limit_ << ": " << fractional_variables_.size() << " fractional variables, " << num_cuts << " L&P cuts to generate.";

		assert(thread_pool_.active() == 1 && "We generate L&P cuts on the first stage and should always have only 1 active thread.");
		// Set round specifics to all dual cgp
		lp_cuts_timer_.Start(2);
		for (auto id_solver_pair : osi_solver_map_) {
			assert(!(lp_round_ == 1 && id_solver_pair.second->getNumCols() > 0 && "In the first round all osi should be empty"));
			if (id_solver_pair.second->getNumCols() > 0) { // if solver is not empty
				if (num_cpu_ > 1) {
					thread_pool_.schedule(boost::bind(&BranchAndBendersCut::SetDualCGPRoundSpecifics, this, boost::ref(*id_solver_pair.second), tree_nodes_by_stage_[0][0]));
				}
				else {
					SetDualCGPRoundSpecifics(*id_solver_pair.second, tree_nodes_by_stage_[0][0]);
				}
			}
		}
		thread_pool_.wait(1); // We wait until all threads except the current one (for MP) are finished
		lp_cuts_timer_.Stop(2);

		// Set round specifics to all existing cgp pnb
		lp_cuts_timer_.Start(1);
		for (auto id_pnb_pair : cgp_pnb_map_) {
			if (num_cpu_ > 1) {
				thread_pool_.schedule(boost::bind(&BranchAndBendersCut::SetLnPCGPRoundSpecifics, this, boost::ref(*id_pnb_pair.second), tree_nodes_by_stage_[0][0]));
			}
			else {
				SetLnPCGPRoundSpecifics(*id_pnb_pair.second, tree_nodes_by_stage_[0][0]);
			}
		}
		thread_pool_.wait(1); // We wait until all threads except the current one (for MP) are finished
		lp_cuts_timer_.Stop(1);

		std::map<double, std::pair<int, double>>::reverse_iterator rit = fractional_variables_.rbegin(); // reverse iterator since map is ordered ascending and we start from the most fractional values
		for (int i = 0; i < num_cuts; i++) {
			if (num_cpu_ > 1 && num_cuts > 1 && lp_parallel_mode_ >= parallel_benders::PNB_PARAL_FRACTS) {
				thread_pool_.schedule(boost::bind(&BranchAndBendersCut::GenerateLiftAndProjectCut, this, i, rit->second));
			}
			else { // use the current thread
				GenerateLiftAndProjectCut(i, rit->second);
			}
			++rit; // goes backwards
		}
		thread_pool_.wait(1); // We wait until all threads except the current one (for MP) are finished

		tree_nodes_by_stage_[0][0]->getDataPtr()->last_in_lp_cgp_problem_start_feasibility_ = tree_nodes_by_stage_[0][0]->getDataPtr()->feasibility_cuts_.end();
		tree_nodes_by_stage_[0][0]->getDataPtr()->last_in_lp_cgp_problem_start_domain_ = tree_nodes_by_stage_[0][0]->getDataPtr()->domain_cuts_.end();
		tree_nodes_by_stage_[0][0]->getDataPtr()->last_in_lp_cgp_start_optimality_ = tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_.end();
		tree_nodes_by_stage_[0][0]->getDataPtr()->last_in_lp_cgp_start_epigraph_ = tree_nodes_by_stage_[0][0]->getDataPtr()->epigraph_cuts_.end();

		//Move L&P cuts to the main loop
		if (!tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_epigraph_cuts_.empty() || !tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_domain_cuts_.empty()) {
			stage_data_[current_stage_]->added_epigraph_cuts_ += tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_epigraph_cuts_.size();
			stage_data_[current_stage_]->added_str_epigraph_cuts_ += tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_str_epigraph_cuts_counter_;
			stage_data_[current_stage_]->added_plain_epigraph_cuts_ += tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_plain_epigraph_cuts_counter_;
			assert(stage_data_[current_stage_]->added_epigraph_cuts_ == (stage_data_[current_stage_]->added_str_epigraph_cuts_ + stage_data_[current_stage_]->added_plain_epigraph_cuts_));
			stage_data_[current_stage_]->added_domain_cuts_ += tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_domain_cuts_.size();

			bool change_aggregation_level = tree_nodes_by_stage_[0][0]->getDataPtr()->changed_aggregation_level_;
			assert(!change_aggregation_level && "Not tested when aggreg level changes before L&P round. Probably it is the first iteration.");
			assert(!adaptive_ && "W/o adaptive in the statement below we assume change 0 -> 1 and not in another direction");
			if (!tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_epigraph_cuts_.empty() && tree_nodes_by_stage_[0][0]->getDataPtr()->num_thetas_ == 0) { // we need to change the aggreation level here
				assert(false && "Not tested when aggreg level changed during L&P round.");
				change_aggregation_level = true;
				tree_nodes_by_stage_[0][0]->getDataPtr()->changed_aggregation_level_ = true;
				// Set new values
				assert(false && "Do we need to uncomment the values below");
				//benders_node_pointer->num_thetas_ = num_theta;
				assert(!(lp_main_loop_iteration_round_ && adaptive_) && "Never tested. Activate_aggregates should probably have size num_theta*2 etc.");
				//benders_node_pointer->active_aggregates_ = boost::dynamic_bitset<>(num_theta); // Gets initialized with 0
				//benders_node_pointer->was_under_threshold_ = boost::dynamic_bitset<>(num_theta); // Gets initialized with 0
			}
			for (auto& cut_container : { &tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_epigraph_cuts_, &tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_domain_cuts_ }) {
				for (auto& cut_stuff : *cut_container) {
					assert(!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_) &&
						!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->epigraph_cuts_) &&
						!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->feasibility_cuts_) &&
						!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->domain_cuts_) && "It cannot be a duplicate since we checked before.");
					cut_stuff->cut_creation_iteration_ = current_iteration_;
					AddCutToContainer(tree_nodes_by_stage_[0][0], cut_stuff);
				}
			}

			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_epigraph_cuts_.clear();
			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_str_epigraph_cuts_counter_ = 0;
			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_plain_epigraph_cuts_counter_ = 0;
			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_domain_cuts_.clear();

			LogSpecialCase(current_iteration_ == 1, "Wanted and built L&P cuts in first iteration");
		}
		else { // if no L&P cuts build - we can switch
			LOG_IF(WARNING, log_level_ <= google::WARNING) << "No L&P cuts build in this round. Switching to minor iteration." << std::endl;
			if (lp_round_ == 1) {
				lp_switch_counter_--; // we do no want to count as switch with one cut-less lp_round. Substract here but increase later
			}
			lp_round_ = lp_rounds_limit_;
			// we will switch osi to MIP in Solve() when switch to minor iteration
			// if not switched to MIP, might lead to issues in warm start: osi will be switched to MIP only in SolveSubproblem() but before that we will try to Set MIP Warm Start.
		}

		DeleteSubproblem(nullptr, tree_nodes_by_stage_[0][0]);

		//Move CGP cuts to the main loop
		if (!tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_.empty() || !tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.empty()) {
			assert(tree_nodes_by_stage_[0][0]->getDataPtr()->last_iteration_start_optimality_ == tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_.end() && "Should be like that after DeleteSubproblem()");
			assert(tree_nodes_by_stage_[0][0]->getDataPtr()->last_iteration_start_feasibility_ == tree_nodes_by_stage_[0][0]->getDataPtr()->feasibility_cuts_.end());
			stage_data_[current_stage_]->added_feasibility_cuts_ += tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.size(); // we always add feas cuts

			int added_opt_cuts = 0;
			bool evaluated_violation(false);
			for (auto& cut_container : { &tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_, &tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_ }) {
				for (auto& cut_stuff : *cut_container) {
					if ((lp_use_cgp_pnb_cuts_ == parallel_benders::PNB_CGP_CUTS_EFFICIENT || lp_use_cgp_pnb_cuts_ == parallel_benders::PNB_CGP_CUTS_EFFICIENT_AND_CONSOLIDATE) && cut_stuff->type_ == parallel_benders::PNB_CUT_TYPE_OPTIMALITY) {  // feas cuts are always efficient
						double dist_violation = cut_stuff->DistanceViolation(tree_nodes_by_stage_[0][0]->getDataPtr()->col_solution_, tree_nodes_by_stage_[0][0]->getDataPtr()->next_stage_approximation_);
						if (dist_violation <= 0.0001) { // in Bodur 0.00001 is the violation for Benders (lazy) cuts. For user (L&P) cuts it is 0.0001 in root node and 0.001 for other nodes 
							continue;
						}
					}
					added_opt_cuts += cut_stuff->type_ == parallel_benders::PNB_CUT_TYPE_OPTIMALITY ? 1 : 0;
					assert(!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_) &&
						!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->epigraph_cuts_) &&
						!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->feasibility_cuts_) &&
						!CheckForSameCut(*cut_stuff, tree_nodes_by_stage_[0][0]->getDataPtr()->domain_cuts_) && "It cannot be a duplicate since we checked before.");
					cut_stuff->cut_creation_iteration_ = current_iteration_;
					AddCutToContainer(tree_nodes_by_stage_[0][0], cut_stuff);
				}
			}

			stage_data_[current_stage_]->added_optimality_cuts_ += added_opt_cuts;
			LOG_IF(INFO, log_level_ <= google::INFO && evaluated_violation) << "Added " << added_opt_cuts << "/" << tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_.size() << " CGP optimality cuts and " << tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.size() << " CGP feasibility cuts (considering DistanceViolation).";
			LOG_IF(INFO, log_level_ <= google::INFO && !evaluated_violation) << "Added " << added_opt_cuts << " CGP optimality cuts and " << tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.size() << " CGP feasibility cuts.";

			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_.clear();
			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.clear();
		}

		LOG_IF(FATAL, num_added_feasibility_cuts_) << "Feasibility cuts not implemented (not tested)";

		// TODO: clear cgp_pnb_map_
	}

	int BranchAndBendersCut::AddLazyCutToNode(CPXCALLBACKCONTEXTptr context, const parallel_benders::CutStuff& cut_stuff)
	{
		//based on OsiCpxSolverInterface::addLazyRowCut

		const OsiRowCut& rowCut = cut_stuff.getCut();

		int status = 0;
		double rhs = 0.0;
		double rng = 0.0;
		char sns;
		double lb = rowCut.lb();
		double ub = rowCut.ub();
		if (lb <= -infinity_ && ub >= infinity_)   // free constraint
		{
			rhs = -infinity_;
			rng = 2 * infinity_;  // CPLEX doesn't support free constraints
			sns = 'R';           // -> implement them as ranged rows with infinite bounds
			LOG(FATAL) << "CPLEX does not accept ranged rows as lazy constraints";
		}
		else if (lb <= -infinity_)  // <= constraint
		{
			rhs = ub;
			sns = 'L';
		}
		else if (ub >= infinity_)  // >= constraint
		{
			rhs = lb;
			sns = 'G';
		}
		else if (ub == lb)  // = constraint
		{
			rhs = ub;
			sns = 'E';
		}
		else  // range constraint
		{
			rhs = lb;
			rng = ub - lb;
			sns = 'R';
			LOG(FATAL) << "CPLEX does not accept ranged rows as lazy constraints";
		}
		int rmatbeg = 0;

		status = CPXcallbackrejectcandidate(context, 1, rowCut.row().getNumElements(), 
			&rhs, &sns, &rmatbeg, 
			const_cast<int*>(rowCut.row().getIndices()),
			const_cast<double*>(rowCut.row().getElements()));
		
		LOG_IF(WARNING, status) << "Error in CPXcallbackrejectcandidate: status = " << status;

		return status;
	}

	int BranchAndBendersCut::AppendCutsToNode(CPXCALLBACKCONTEXTptr context, SmiTreeNode<parallel_benders::BendersNodeData*>* benders_tree_node_pointer)
	{
		//based on AppendCutsToSubproblem

		int status = 0;

		parallel_benders::BendersNodeData* benders_node_pointer = benders_tree_node_pointer->getDataPtr();
		parallel_benders::BendersNodeData::cut_container::const_iterator feas_it = benders_node_pointer->feasibility_cuts_.begin();
		parallel_benders::BendersNodeData::cut_container::const_iterator domain_it = benders_node_pointer->domain_cuts_.begin();

		if (!benders_node_pointer->feasibility_cuts_.empty()) {
			LOG(FATAL) << "Never tested";
			// Add the cuts
			//if (seperate_first_stage_solver_ && current_stage_ == 0) {
			//	feas_it = benders_node_pointer->last_iteration_start_feasibility_;
			//	benders_node_pointer->last_iteration_start_feasibility_ = benders_node_pointer->feasibility_cuts_.end();
			//}
			//if (feas_it != benders_node_pointer->feasibility_cuts_.end()) {
			//	for (; feas_it != benders_node_pointer->feasibility_cuts_.end(); ++feas_it) {
			//		assert((*feas_it)->getCut().row().getNumElements() <= osi_solver->getNumCols());

			//		if (!benders_node_pointer->warm_start_added_ && ((*feas_it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*feas_it)->cut_creation_iteration_ == current_iteration_ - 1 && current_stage_ == 0)) {
			//			// Add warm start
			//			SetWarmStart(osi_solver, benders_tree_node_pointer);
			//			benders_node_pointer->warm_start_added_ = true;
			//			if (level_decomp_ && current_stage_ == 0 && integer_problems_[current_stage_] == false) {
			//				OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
			//				if (osi_cpx != nullptr && level_cstat_ != nullptr) {
			//					CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
			//					CPXLPptr lp_ptr = osi_cpx->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
			//					if (level_length_ == osi_cpx->getNumRows())
			//						CPXcopybasednorms(env_ptr, lp_ptr, level_cstat_.get(), level_rstat_.get(), level_dnorm_.get());
			//					else
			//						LOG_IF(WARNING, log_level_ <= google::WARNING) << "Trying to copy basednorms where problem has " << osi_cpx->getNumRows() << " rows and had " << level_length_;
			//				}
			//			}
			//		}
			//		assert((*feas_it)->getCut().row().getNumElements() <= osi_solver->getNumCols());
			//		ApplyCut(osi_solver, **feas_it, append_type);
			//	}
			//}
		}

		if (!benders_node_pointer->domain_cuts_.empty()) {
			LOG(FATAL) << "Never tested";
			// Add the cuts
			//if (seperate_first_stage_solver_ && current_stage_ == 0) {
			//	domain_it = benders_node_pointer->last_round_start_domain_;
			//	benders_node_pointer->last_round_start_domain_ = benders_node_pointer->domain_cuts_.end();
			//}
			//if (domain_it != benders_node_pointer->domain_cuts_.end()) {
			//	for (; domain_it != benders_node_pointer->domain_cuts_.end(); ++domain_it) {
			//		assert((*domain_it)->getCut().row().getNumElements() <= osi_solver->getNumCols());

			//		if (!benders_node_pointer->warm_start_added_ && ((*domain_it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*domain_it)->cut_creation_iteration_ == current_iteration_ - 1 && current_stage_ == 0)) {
			//			// Add warm start
			//			SetWarmStart(osi_solver, benders_tree_node_pointer);
			//			benders_node_pointer->warm_start_added_ = true;
			//			if (level_decomp_ && current_stage_ == 0 && integer_problems_[current_stage_] == false) {
			//				OsiCpxSolverInterface* osi_cpx = dynamic_cast<OsiCpxSolverInterface*> (osi_solver);
			//				if (osi_cpx != nullptr && level_cstat_ != nullptr) {
			//					CPXENVptr env_ptr = osi_cpx->getEnvironmentPtr();
			//					CPXLPptr lp_ptr = osi_cpx->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
			//					if (level_length_ == osi_cpx->getNumRows())
			//						CPXcopybasednorms(env_ptr, lp_ptr, level_cstat_.get(), level_rstat_.get(), level_dnorm_.get());
			//					else
			//						LOG_IF(WARNING, log_level_ <= google::WARNING) << "Trying to copy basednorms where problem has " << osi_cpx->getNumRows() << " rows and had " << level_length_;
			//				}
			//			}
			//		}
			//		ApplyCut(osi_solver, **domain_it, append_type);
			//	}
			//}
		}

		if (!benders_node_pointer->optimality_cuts_.empty()) {
			// Add the cuts
			parallel_benders::BendersNodeData::cut_container::const_iterator it = benders_node_pointer->optimality_cuts_.begin();
			if (benders_node_pointer->last_iteration_start_optimality_ != benders_node_pointer->optimality_cuts_.end()) {
				it = benders_node_pointer->last_iteration_start_optimality_;
			}
			else
				it = benders_node_pointer->optimality_cuts_.end();

			benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_ = false;

			for (; it != benders_node_pointer->optimality_cuts_.end(); ++it) {
				if ((*it)->to_use_) {
					status = BranchAndBendersCut::AddLazyCutToNode(context, **it);
					if (status) {
						LOG(WARNING) << "Error in AddLazyCutToNode: status = " << status;
						return status;
					}
				}
			}
		}

		if (!benders_node_pointer->epigraph_cuts_.empty()) {
			LOG(FATAL) << "Never tested";
			// Add the cuts
			//BendersNodeData::cut_container::const_iterator it = benders_node_pointer->epigraph_cuts_.begin();
			//if (seperate_first_stage_solver_ && current_stage_ == 0) {
			//	if (benders_node_pointer->last_round_start_epigraph_ != benders_node_pointer->epigraph_cuts_.end()) {
			//		it = benders_node_pointer->last_round_start_epigraph_;
			//		if (current_stage_ > 0) // In non-first-stage nodes we must read all optimality cuts!
			//			//it = benders_node_pointer->optimality_cuts_.end();
			//			assert(false);
			//	}
			//	else
			//		it = benders_node_pointer->epigraph_cuts_.end();
			//}
			//LOG_IF(FATAL, current_iteration_ == 1 && generate_lp_cuts_ && !cont_phase_it_limit_ && !use_first_phase_) << "Is it a problem?";  //changed_aggregation_level_ should be changed if we build lp cut in 1st iteration. Right?
			//benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_ = false;

			//for (; it != benders_node_pointer->epigraph_cuts_.end(); ++it) {
			//	if (!benders_node_pointer->warm_start_added_ && ((*it)->cut_creation_iteration_ == current_iteration_ && current_stage_ > 0 || (*it)->cut_creation_iteration_ == current_iteration_ - 1 && current_stage_ == 0)) {
			//		// Add warm start
			//		SetWarmStart(osi_solver, benders_tree_node_pointer);
			//		benders_node_pointer->warm_start_added_ = true;
			//	}

			//	if ((*it)->to_use_) {
			//		assert((*it)->getCut().row().getNumElements() <= osi_solver->getNumCols());
			//		ApplyCut(osi_solver, **it, append_type);
			//	}
			//}
			//benders_node_pointer->last_round_start_epigraph_ = benders_node_pointer->epigraph_cuts_.end();
		}

		return 0;
	}

	void BranchAndBendersCut::SetUpMPSolver(OsiCpxSolverInterface* cpx_solver)
	{
		assert(current_stage_ == 0);
		LOG_IF(FATAL, getTreeNode(0,0)->getDataPtr()->optimality_cuts_.empty() && getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.empty()) << "When problem has no theta cuts, it is unbounded when B&B started as theta is not bounded.";
		
		// Set the CPLEX callback
		BranchAndBendersCut* user_data = this;
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXLPptr lp_ptr = cpx_solver->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
		// TODO: do we need all of them
		int status = CPXcallbacksetfunc(env_ptr, lp_ptr, 
			CPX_CALLBACKCONTEXT_CANDIDATE // - found integer solution
			// | CPX_CALLBACKCONTEXT_RELAXATION // - found infeasible (some int vars have non-int solution)
			| CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS // - update global UB (LB - ?). Locks other threads
			| CPX_CALLBACKCONTEXT_LOCAL_PROGRESS // - update thread UB (LB - ?)
			| CPX_CALLBACKCONTEXT_THREAD_UP // - activate thread, is different to thread creation and might happen several times for one thread
			// | CPX_CALLBACKCONTEXT_THREAD_DOWN  // - deactivate thread, is different to thread creation and might happen several times for one thread
			, benders_callback, user_data);
		if (status) {
			fprintf(stderr, "Error in CPXcallbacksetfunc, status = %d.\n", status);
		}

		// Set threads number for B&B
		// for CPLEX in Benders it was always mutlithread
		// value 0 in CPLEX has the same meaning as in our implementation -- use all CPUs
		// TODO: parameter to set num_cpu for B&B specifically (explicitely or parallel_mode)
		CPXsetintparam(env_ptr, CPX_PARAM_THREADS, num_cpu_);
	}

	BranchAndBendersCut* BranchAndBendersCut::getCopyForThread(int thread_id)
	{
		// TODO: delete the pointers of thread_copy_map_mutex_ destruction.
		// Just in case because we delete them anyways when thread goes down.

		{
			std::lock_guard<std::mutex> unique_lock(thread_copy_map_mutex_);
			std::map<int, BranchAndBendersCut*>::iterator it = thread_copy_map_.find(thread_id);
			
			if (it != thread_copy_map_.end())
				return (*it).second;
			else {
				return nullptr;
			}		
		}		
	}

	void BranchAndBendersCut::createCopyForThread(int thread_id)
	{
		BranchAndBendersCut* thread_copy = nullptr;
		int num_theta = 0;
		std::vector< std::vector<int> > nodes_in_cut;
		{
			std::lock_guard<std::mutex> unique_lock(thread_copy_map_mutex_);

			// Initialize new bbc from current data
			thread_copy = new BranchAndBendersCut(smi_);
			thread_copy_map_.emplace(thread_id, thread_copy);
			thread_copy->console_mutex_ptr_ = console_mutex_ptr_; // we share the console as the resource
			thread_copy->parameter_ = parallel_benders::Parameter(parameter_);
			thread_copy->testset_name_ = testset_name_;
			thread_copy->problem_name_ = problem_name_;
			thread_copy->instance_name_ = instance_name_ + "_thread_" + std::to_string(thread_id);
			
			// The data below was set when we built theta cuts.
			// We could copy the nodes but do not need so much data.
			num_theta = tree_nodes_by_stage_[0][0]->getDataPtr()->num_thetas_;
			nodes_in_cut = tree_nodes_by_stage_[0][0]->getDataPtr()->nodes_in_cut_;
		}
		thread_copy->Init();
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->num_thetas_ = num_theta;
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->nodes_in_cut_ = std::move(nodes_in_cut);
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->active_aggregates_ = boost::dynamic_bitset<>(num_theta); // Gets initialized with 0
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->was_under_threshold_ = boost::dynamic_bitset<>(num_theta); // Gets initialized with 0
					
		// Copy warm start
		if (warm_start_) {
			for (int stage = 1; stage < num_stages_; stage++) { // start from 2nd stage - warm start of main loop MP is not applicable to MP in PNB CGP 
				CoinWarmStart* ws;
				std::lock_guard<std::mutex> temp_lock(stage_warm_start_mutex_);
				if (stage_data_[stage]->warm_start_) {
					ws = (stage_data_[stage]->warm_start_.get())->clone();
					thread_copy->stage_data_[stage]->warm_start_.reset(ws); // deep copy of warm start
				}
				for (int scenario = 0; scenario < tree_nodes_by_stage_[stage].size(); scenario++) {
					if (tree_nodes_by_stage_[stage][scenario]->getDataPtr()->warm_start_) {
						ws = (tree_nodes_by_stage_[stage][scenario]->getDataPtr()->warm_start_.get())->clone(); // deep copy
						thread_copy->tree_nodes_by_stage_[stage][scenario]->getDataPtr()->warm_start_.reset(ws);
					}
				}
			}
		}

		// TODO: probably we need to copy cuts from the main bbc to avoid duplicate cuts. Move them to the main bbc in thread down and delete from here
	}

} // end namespace branch_and_benders_cut
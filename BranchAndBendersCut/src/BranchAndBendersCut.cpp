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

		int status = 0;
		int thread_id;
		BranchAndBendersCut* thread_data = static_cast<BranchAndBendersCut*>(cbhandle);

		thread_data->current_stage_ = 0;
		thread_data->forward_ = true;

		int num_x_cols = thread_data->stage_data_[0]->num_columns_;
		int integer_feasible;
		
		status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &thread_id);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetinfoint: status = %d\n", status);
			return status;
		}

		if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE) 
		{		
			// Check if solution is bounded
			status = CPXcallbackcandidateispoint(context, &integer_feasible);
			if (status) {
				fprintf(stderr, "Error in CPXXcallbackcandidateispoint: status = %d\n", status);
				return status;
			}
			if (!integer_feasible) {
				/* The model is bounded, so this cannot happen. */
				fprintf(stderr, "Unbounded solution");
				status = CPXERR_UNSUPPORTED_OPERATION;
				return 0; //TODO: some of the nodes are sometimes unbounded? How can it be? What should we do with them?
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

			for (auto id_solver_pair : thread_data->osi_solver_map_) {
				//id_solver_pair.second->reset(); // This method throws exception with Cplex
				id_solver_pair.second->loadProblem(0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				// Workaround: Load empty problem. 
				assert(id_solver_pair.second->getNumCols() == 0);
			}

			for (auto node : thread_data->tree_nodes_by_stage_[thread_data->current_stage_]) {
				node->getDataPtr()->CleanUnusedMemory(thread_data->forward_);
			}
		}
		else if (contextid == CPX_CALLBACKCONTEXT_RELAXATION)
		{
			//Store Data
			status = thread_data->StoreNodeData(context, contextid);
			if (status) {
				fprintf(stderr, "Error in bbc_->StoreNodeData(): status = %d\n", status);
				return status;
			}
		}
		else if (contextid == CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS) {
			//this context locks all threads, be fast!
			//results of CPXcallbackgetinfo___ are global here

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
				assert(parallel_benders::CompareForEquality(global_upper_bound, thread_data->benders_optimal_incumbents_.back(), thread_data->zero_tolerance_));
			}			

			double global_lower_bound;
			status = CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_BND, &global_lower_bound);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetinfodbl: status = %d\n", status);
				return status;
			}
			thread_data->best_bounds_.push_back(global_lower_bound);
		}

		return 0;
	} /* END benders_callback */
	
	BranchAndBendersCut::BranchAndBendersCut(SmiScnModel* smi_model_ptr)
		: parallel_benders::ParallelNestedBenders(smi_model_ptr), benders_optimal_incumbents_(), best_bounds_()
	{
	}

	BranchAndBendersCut::~BranchAndBendersCut()
	{
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
		
		//TODO: temp parameters
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXsetintparam(env_ptr, CPX_PARAM_THREADS, 1);

		// First Phase: construct Linear optimality cuts
		current_iteration_ = 1;
		HandleContinuousPhase();

		// TODO: probably here we will start L&P rounds for root node

		// Second Phase: Branch-And-Benders-Cut
		SetUpMPSolver(cpx_solver);
		PrintSubproblem(cpx_solver, 0, "MP_before");
		LOG_IF(FATAL, mp_node_pointer->optimality_cuts_.empty() && mp_node_pointer->epigraph_cuts_.empty()) << "Cannot start B&B when problem has not cuts for theta";
		SolveSolverInterface(cpx_solver);
		current_stage_ = 0;
		PrintSubproblem(cpx_solver, 0, "MP_after");
		
		if (cpx_solver->isProvenOptimal()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_OPTIMAL;
			assert(num_cpu_ == 1 && "For multithread the assertion below will probably fail");
			assert(parallel_benders::CompareForEquality(benders_optimal_incumbents_.back(), cpx_solver->getObjValue(), zero_tolerance_));
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
			LOG(INFO) << "Lower Bound:  " << (regularization_ ? regularized_solution_value_ : lower_bound_) << "\n" << "Upper Bound:  " << upper_bound_ << "\n" << "Relative Gap: " << parallel_benders::ComputeRelativeGap(upper_bound_, lower_bound_);
			LOG(INFO) << "Major Iterations: " << major_iteration_ << "\n" << "Overall Iterations: " << current_iteration_;
			if (insubstantial_iteration_ > 0)
				LOG(INFO) << "Insubstantial Iterations: " << insubstantial_iteration_;
			int cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += stage_data_[i]->added_optimality_cuts_;
			}
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
		assert(num_theta == 1); //TODO: correctly identify the number of theta, where do we set mp_node_pointer->num_thetas_

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
			//thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem, this, tree_nodes_by_stage_[current_stage_][i]));
			HandleSubproblem(tree_nodes_by_stage_[current_stage_][i]);

			LOG_IF(FATAL, num_added_feasibility_cuts_) << "Feasibility cuts not implemented (not tested)";
			stage_data_[current_stage_ - 1]->added_feasibility_cuts_ += num_added_feasibility_cuts_;
			num_added_feasibility_cuts_ = 0;
		}
		//thread_pool_.wait(0); // We wait until all threads are done

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
			lower_bound_ < upper_bound_;
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
		cpx_solver->messageHandler()->setLogLevel(4); // Log nothing
		BuildSubproblem(cpx_solver, tree_nodes_by_stage_[0][0]);
		AppendColsToSubproblem(cpx_solver, tree_nodes_by_stage_[0][0]);

		// Generate optimality cuts
		// because without theta cuts the problem will be unbounded
		if (tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_.empty() && tree_nodes_by_stage_[0][0]->getDataPtr()->epigraph_cuts_.empty()) {
			cpx_solver->initialSolve();
			StoreData(cpx_solver, tree_nodes_by_stage_[0][0]);
			ConstructOptimalityCuts();
			for (auto id_solver_pair : osi_solver_map_) {
				id_solver_pair.second->loadProblem(0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
			}
			current_stage_ = 0;
			// call this method to activate thetas,
			// i.e., set their coef to 1
			AppendColsToSubproblem(cpx_solver, tree_nodes_by_stage_[0][0]);
		}

		LOG_IF(FATAL, !tree_nodes_by_stage_[0][0]->getDataPtr()->feasibility_cuts_.empty()) << "Feasibility cuts not implemented (not tested)";

		// add Continuous and/or L&P cuts
		AppendCutsToSubproblem(cpx_solver, tree_nodes_by_stage_[0][0], parallel_benders::PNB_CUT_APPEND_TYPE_LAZY); 
		DeleteSubproblem(nullptr, tree_nodes_by_stage_[0][0]);

		// Set the CPLEX callback
		BranchAndBendersCut* user_data = this;
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXLPptr lp_ptr = cpx_solver->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
		int status = CPXcallbacksetfunc(env_ptr, lp_ptr, CPX_CALLBACKCONTEXT_CANDIDATE
			//| CPX_CALLBACKCONTEXT_RELAXATION
			| CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS
			//| CPX_CALLBACKCONTEXT_LOCAL_PROGRESS | CPX_CALLBACKCONTEXT_THREAD_DOWN | CPX_CALLBACKCONTEXT_THREAD_UP
			, benders_callback, user_data);
		if (status) {
			fprintf(stderr, "Error in CPXcallbacksetfunc, status = %d.\n", status);
		}
	}

} // end namespace branch_and_benders_cut
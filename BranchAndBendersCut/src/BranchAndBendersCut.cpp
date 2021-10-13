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
		/*This is the implementation with new generic CPLEX callbacks. 
		It is not possible to get the fixed node vars which are necessary for L&P cuts lifting.
		We use legacy callbacks therefore.
		*/
		
		// see bendersatsp2.c in CPLEX examples
	
		BranchAndBendersCut* global_data = static_cast<BranchAndBendersCut*>(cbhandle);		
		
		int status = 0;
		int thread_id;
		status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &thread_id);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetinfoint: status = %d\n", status);
			return status;
		}
		
		std::shared_ptr<ThreadBranchAndBendersCut> thread_data = global_data->getCopyForThread(thread_id);
		if (thread_data != nullptr) {
			assert(contextid != CPX_CALLBACKCONTEXT_THREAD_UP);
			thread_data->setCurrentContextData(context, contextid);
		}		

		/* Below is the explicit list of contextid_s.
		We need it for tracking what algorithm does.
		Enable contexts in SetUpMPSolver() */
		if (contextid == CPX_CALLBACKCONTEXT_THREAD_UP && !thread_data) {
			assert(thread_data == nullptr);
			global_data->createCopyForThread(thread_id);
		}
		else if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE) {
			global_data->get_mutable_timer().Start(1);

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
				if theta cuts were not added to the model formulation
				or as user cuts.*/
				fprintf(stderr, "Unbounded solution\n");
				status = CPXERR_UNSUPPORTED_OPERATION;
				return status;
			}

			// empty CPLEX (needed if we already built L&P cuts and used solvers)
			// TODO: Was if (thread_data->getCurrentNodeId() != -1)
			thread_data->current_lp_round_ = 0;
			for (auto& id_solver_pair : thread_data->osi_solver_map_) {
				id_solver_pair.second->loadProblem(0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
			}

			//Store Data
			status = thread_data->StoreNodeData();
			if (status) {
				fprintf(stderr, "Error in bbc_->StoreNodeData(): status = %d\n", status);
				return status;
			}

			// Generate optimality cuts and add them as lazy cuts
			int old_num_cuts = thread_data->stage_data_[0]->added_optimality_cuts_;
			
			thread_data->ConstructOptimalityCuts(); // and calculate Benders ub, lb
			int added_cuts = thread_data->stage_data_[0]->added_optimality_cuts_ - old_num_cuts;
			// if violated cuts are found: add them and reject the integer candidate solution
			// if no violated cuts found (or ub = lb), no cuts are appended and candidate is excepted
			thread_data->AppendCutsToNode(thread_data->tree_nodes_by_stage_[thread_data->current_stage_ - 1][0]);
			thread_data->DeleteSubproblem(nullptr, thread_data->tree_nodes_by_stage_[thread_data->current_stage_ - 1][0]);

			// Root node is never solved in parallel - its data is "global"
			// If root is integer right away,
			// context CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS might never be called.
			// We need to set UB here
			// TODO: distinguish between root is integer vs root is integer + Benders optimal
			if (thread_data->CanQueryNodeInfo() && thread_data->getCurrentNodeDepth() == 0 && !added_cuts)
			{
				global_data->benders_optimal_incumbents_.push_back(thread_data->benders_optimal_incumbents_.back());
			}
			// TODO: what is with LB - should we store it in CPX_CALLBACKCONTEXT_RELAXATION. It seems to be wrong when relaxation solution == MIP
			// try to get it using branching context https://www.ibm.com/docs/en/icos/20.1.0?topic=context-cpx-callbackcontext-branching in addition to relaxation context

			for (auto node : thread_data->tree_nodes_by_stage_[thread_data->current_stage_]) {
				node->getDataPtr()->CleanUnusedMemory(thread_data->forward_);
			}
			// also can be set to -1 in StoreNodeData()
			// TODO: Implement the line below. Do we need that.
			//thread_data->setCurrentNodeId(-1);

			global_data->get_mutable_timer().Stop(1);
		}
		else if (contextid == CPX_CALLBACKCONTEXT_RELAXATION) {
			// TODO: exclude this context completely if we do not use L&P cuts
			global_data->get_mutable_lp_cuts_timer().Start(3);
			
			thread_data->current_stage_ = 0;
			thread_data->forward_ = true;			

			// TODO: If we come from contextid == CPX_CALLBACKCONTEXT_CANDIDATE. How to check it?
			// We can check current context id. Was: if (thread_data->getCurrentNodeId() == -1)
			for (auto& id_solver_pair : thread_data->osi_solver_map_) {
				id_solver_pair.second->loadProblem(0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
			}
			
			//Store Data
			status = thread_data->StoreNodeData();
			if (status) {
				fprintf(stderr, "Error in bbc_->StoreNodeData(): status = %d\n", status);
				global_data->get_mutable_lp_cuts_timer().Stop(3);
				return status;
			}

			// We are not in the root node or we are in the root node after presolve restart
			if (thread_data->getCurrentNodeDepth() != 0 || thread_data->getCurrentNodeId() != 0) {
				// TODO: can it be that withing one thread we "jump" between open nodes
				// for which we construct L&P cuts.
				assert(thread_data->getCurrentNodeId() != 0);
				assert(thread_data->current_lp_round_ == 0);
				thread_data->generate_lp_cuts_ = false;
				global_data->get_mutable_lp_cuts_timer().Stop(3);
				return status;
			}

			// TODO: get rid of lp_cuts_constructed_ member
			if (thread_data->lp_cuts_constructed_) {
				thread_data->lp_cuts_constructed_ = false;
				thread_data->generate_lp_cuts_ = false;
				global_data->get_mutable_lp_cuts_timer().Stop(3);
				return 0;
			}

			// Quit if reached the L&P rounds limit
			if (thread_data->current_lp_round_ == thread_data->lp_rounds_limit_) {
				thread_data->lp_cuts_constructed_ = true;
				thread_data->current_lp_round_ = 0;
				thread_data->generate_lp_cuts_ = false;
				global_data->get_mutable_lp_cuts_timer().Stop(3);
				return 0;
			}
			thread_data->current_lp_round_++;

			// Generate L&P cuts			
			assert(thread_data->current_stage_ == 0);
			thread_data->ConstructLiftAndProjectCuts();
			status = thread_data->AppendCutsToNode(thread_data->tree_nodes_by_stage_[0][0]);
			if (status) {
				fprintf(stderr, "Error in bbc_->AppendCutsToNode(): status = %d\n", status);
				global_data->get_mutable_lp_cuts_timer().Stop(3);
				return status;
			}

			// TODO: empty CPLEX, do we need it?
			// Was: if (thread_data->getCurrentNodeId() != -1)
			for (auto& id_solver_pair : thread_data->osi_solver_map_) {
				id_solver_pair.second->loadProblem(0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
			}
			global_data->get_mutable_lp_cuts_timer().Stop(3);
		}
		else if (contextid == CPX_CALLBACKCONTEXT_BRANCHING){
			// Callback for this context is not called in release mode.
			
			thread_data->current_stage_ = 0;
			status = thread_data->StoreNodeData();
			if (status) {
				fprintf(stderr, "Error in bbc_->StoreNodeData(): status = %d\n", status);
				return status;
			}

			DLOG(INFO) << "Branching context for node " << thread_data->getCurrentNodeId();
		}
		else if (contextid == CPX_CALLBACKCONTEXT_LOCAL_PROGRESS) {
			//results of CPXcallbackgetinfo___ are local here
			// Callback for this context is not called in release mode.

		}
		else if (contextid == CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS) {
			// this context locks all threads, be fast!
			// results of CPXcallbackgetinfo___ are global here

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
		}
		else if (contextid == CPX_CALLBACKCONTEXT_THREAD_DOWN) {
			bool sync_cuts = false;

			// TODO: we need some function to sync time
			// Sync timers
			assert(thread_data->lp_cuts_timer_.GetTime(0) == 0 && "Solution timer of MP relaxation for L&P cuts is not applicable to BBC.");
			global_data->get_mutable_lp_cuts_timer().AddTime(1, thread_data->get_lp_cuts_timer().GetTime(1));
			global_data->get_mutable_lp_cuts_timer().AddTime(2, thread_data->get_lp_cuts_timer().GetTime(2));
			assert(thread_data->get_lp_cuts_timer().GetTime(3) == 0 && "timer 3 - L&P generation wall clock time is timed in global thread");
			assert(thread_data->get_solution_times().GetTime(0) == 0);
			assert(thread_data->get_solution_times().GetTime(1) == 0);
			assert(thread_data->get_solution_times().GetTime(2) == 0);
			global_data->get_mutable_solution_times().AddTime(3, thread_data->get_solution_times().GetTime(3));
			assert(thread_data->get_solution_times().GetTime(4) == 0 && "timer 4 - Cont phase is not run in threads");
			assert(thread_data->get_timer().GetTime(0) == 0 && "timer of 1st stage should be 0");
			assert(thread_data->get_timer().GetTime(1) == 0 && "timer of 2nd is timed in global thread");
			assert(thread_data->get_detailed_timer().GetTime(0) == 0);
			assert(thread_data->get_detailed_timer().GetTime(1) == 0);
			assert(thread_data->get_detailed_timer().GetTime(2) == 0);
			global_data->get_mutable_detailed_timer().AddTime(3, thread_data->get_detailed_timer().GetTime(3));
			global_data->get_mutable_detailed_timer().AddTime(4, thread_data->get_detailed_timer().GetTime(4));

						
			// TODO: otherwise just update their quantity
			if (sync_cuts) {
				// Add Benders and L&P cuts to the global data
				// TODO: do it in global progress (so that we share the cuts earlier)
				std::lock_guard<std::shared_mutex> exclusive_lock(global_data->thread_data_mutex_);
				LOG(INFO) << "Started syncing cuts in thread down context for thread " << thread_data->getCurrentThreadId();
				global_data->stage_data_[0]->lp_cgp_counter_ += thread_data->stage_data_[0]->lp_cgp_counter_;

				std::vector<parallel_benders::BendersNodeData::cut_container*> cut_containers;
				cut_containers.push_back(&thread_data->getTreeNode(0, 0)->getDataPtr()->optimality_cuts_);
				cut_containers.push_back(&thread_data->getTreeNode(0, 0)->getDataPtr()->feasibility_cuts_);
				cut_containers.push_back(&thread_data->getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_);
				cut_containers.push_back(&thread_data->getTreeNode(0, 0)->getDataPtr()->domain_cuts_);
				for (auto& cut_container : cut_containers) {
					for (auto& cut_stuff : *cut_container) {
						if (cut_stuff->cut_creation_thread_ == thread_data->getCurrentThreadId() && // -1 root node before B&B
							!(global_data->CheckForSameCut(*cut_stuff, global_data->getTreeNode(0, 0)->getDataPtr()->optimality_cuts_) ||
								global_data->CheckForSameCut(*cut_stuff, global_data->getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_) ||
								global_data->CheckForSameCut(*cut_stuff, global_data->getTreeNode(0, 0)->getDataPtr()->feasibility_cuts_) ||
								global_data->CheckForSameCut(*cut_stuff, global_data->getTreeNode(0, 0)->getDataPtr()->domain_cuts_)))
						{
							global_data->AddCutToContainer(global_data->getMutableTreeNode(0, 0), cut_stuff);

							if (cut_stuff->type_ == parallel_benders::PNB_CUT_TYPE_OPTIMALITY)
							{
								global_data->stage_data_[0]->added_optimality_cuts_++;
								//added_cgp_optimality_cuts_; // we cannot distinguish opt. cuts from cgp. opt. cuts
							}
							else if (cut_stuff->type_ == parallel_benders::PNB_CUT_TYPE_FEASIBILITY)
							{
								global_data->stage_data_[0]->added_feasibility_cuts_++;
								//added_cgp_feasibility_cuts_; // we cannot distinguish feas. cuts from cgp. feas. cuts
							}
							else if (cut_stuff->type_ == parallel_benders::PNB_CUT_TYPE_EPIGRAPH)
							{
								global_data->stage_data_[0]->added_epigraph_cuts_++;
								//added_str_epigraph_cuts_;  // we cannot distinguish strong and plain epigraph cuts
								//added_plain_epigraph_cuts_;
							}
							else if (cut_stuff->type_ == parallel_benders::PNB_CUT_TYPE_DOMAIN)
							{
								global_data->stage_data_[0]->added_domain_cuts_++;
								//added_domain_cuts_;
							}
							else
							{
								LOG(FATAL) << "Unknown cut type. PNB_CUT_TYPE_CVAR?";
							}
						}
					}
				}
				LOG(INFO) << "Finished syncing cuts." << thread_data->getCurrentThreadId();
			}

			{
				std::lock_guard<std::mutex> lock(global_data->thread_copy_map_mutex_);
				// sometimes bounds are not synced with the global data in CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS
				double thread_optimal_incumbend = thread_data->benders_optimal_incumbents_.back();
				if (thread_optimal_incumbend < global_data->benders_optimal_incumbents_.back()) {
					assert(*std::min_element(thread_data->benders_optimal_incumbents_.begin(), thread_data->benders_optimal_incumbents_.end()) == thread_optimal_incumbend);
					global_data->benders_optimal_incumbents_.push_back(thread_optimal_incumbend);
				}
				// We delete thread_data because the thread with the same id
				// can be up and down multiple times. // TODO: how often it happens, maybe keep the solver and restart it?
				global_data->thread_copy_map_.erase(thread_id);
			}					
		}

		return 0;
	} /* END benders_callback */
	
	BranchAndBendersCut::BranchAndBendersCut(SmiScnModel* smi_model_ptr)
		: parallel_benders::ParallelNestedBenders(smi_model_ptr),benders_optimal_incumbents_(1, infinity_),thread_copy_map_(),thread_copy_map_mutex_(),thread_data_mutex_()
	{
	}

	BranchAndBendersCut::~BranchAndBendersCut()
	{
	}

	void BranchAndBendersCut::Solve()
	{
		smi_->writeSmps("modeloutput");

		for (int stage = 0; stage < num_stages_; stage++)
		{
			// We cannot distinguish between normal and CGP cuts in B&B
			// set the counter value to -1 so that we know
			// that its value is not valid.
			stage_data_[stage]->added_cgp_optimality_cuts_ = -1;
			stage_data_[stage]->added_cgp_feasibility_cuts_ = -1;
		}

		if (solution_code_ == parallel_benders::PNB_SOLUTION_PARAMETER_PROBLEM) {
			LOG_IF(ERROR, log_level_ <= google::ERROR) << "Cannot solve because of the problem in parameters.";
			return;
		}
		if (thread_pool_.size() == 0) {
			solution_code_ = parallel_benders::PNB_SOLUTION_PARAMETER_PROBLEM;
			solved_counter_++;
			return;
		}

		LOG_IF(FATAL, !dynamic_cast<OsiCpxSolverInterface*>(osi_solver_)) << "osi_solver_ should be OsiCpxSolverInterface for BranchAndBendersCut";

		parallel_benders::BendersNodeData* mp_node_pointer = tree_nodes_by_stage_[0][0]->getDataPtr();
		
		// Construct continuous optimality, L&P cuts or
		// one round of Benders cuts (to avoid unboundness)
		wall_clock_solution_time_.Start(0);
		assert(current_stage_ == 0);
		BuildSubproblem(osi_solver_, tree_nodes_by_stage_[0][0]);
		AppendColsToSubproblem(osi_solver_, tree_nodes_by_stage_[0][0]);
		ConstructRootNodeCuts();

		// Second Phase: Branch-And-Benders-Cut
		SetUpMPSolver();
		PrintSubproblem(osi_solver_, 0, "MP_before");
		SolveSolverInterface(osi_solver_);
		current_stage_ = 0;
		PrintSubproblem(osi_solver_, 0, "MP_after");
		wall_clock_solution_time_.Stop(0);
		
		if (osi_solver_->isProvenOptimal()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_OPTIMAL;
			//TODO: the line below still generates error, even though the solution is correct
			//LOG_IF(FATAL, !parallel_benders::CompareForEquality(benders_optimal_incumbents_.back(), osi_solver_->getObjValue(), 0.001)) << "UB != solution";
		}
		else if (osi_solver_->isProvenPrimalInfeasible()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_INFEASIBLE;
		}
		else if (osi_solver_->isProvenDualInfeasible()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_UNBOUNDED;
		}
		else if (osi_solver_->isPrimalObjectiveLimitReached()) {
			LOG(FATAL) << "Unknown solution code";
		}
		else if (osi_solver_->isDualObjectiveLimitReached()) {
			LOG(FATAL) << "Unknown solution code";
		}
		else if (osi_solver_->isIterationLimitReached()) {
			solution_code_ = parallel_benders::PNB_SOLUTION_ITERATION_LIMIT;
		}
		else if (osi_solver_->isTimeLimitReached()) {
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
			LOG(INFO) << "Lower Bound:  " << getLB() << "\n" << "Upper Bound:  " << benders_optimal_incumbents_.back() << "\n" << "Relative Gap: " << parallel_benders::ComputeRelativeGap(benders_optimal_incumbents_.back(), getLB());
			int cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += stage_data_[i]->added_optimality_cuts_;
			}
			// TODO: we need to calculate here cuts for all bbc thread copies
			LOG(INFO) << "Added " << cut_counter << " optimality cuts (incl. cuts from L&P CGP).";
			LOG(INFO) << "Added " << getNumContinuousOptimalityCuts() << " continuous optimality cuts.";
			cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += stage_data_[i]->added_feasibility_cuts_;
			}
			LOG(INFO) << "Added " << cut_counter << " feasibility cuts (incl. cuts from L&P CGP).";
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
				/*
				We cannot distinguish these cuts from non CGP ones.
				cut_counter = 0;
				for (int i = 0; i < num_stages_; i++) {
					cut_counter += stage_data_[i]->added_cgp_optimality_cuts_;
				}	
				LOG(INFO) << "Added " << cut_counter << " optimality cuts from L&P CGP.";
				cut_counter = 0;
				for (int i = 0; i < num_stages_; i++) {
					cut_counter += stage_data_[i]->added_cgp_feasibility_cuts_;
				}
				LOG(INFO) << "Added " << cut_counter << " feasibility cuts from L&P CGP.";
				*/
			}
		}
		assert(get_lp_cuts_timer().GetTime(0) == 0 && "Solution timer of MP relaxation for L&P cuts is not applicable to BBC.");
		// these timers should be somewhere else
		assert(get_solution_times().GetTime(0) == 0 && "Solution timer of MP is not applicable to BBC.");
		assert(get_solution_times().GetTime(1) == 0 && "Timer for projection is not applicable because we never use that technique.");
		assert(get_solution_times().GetTime(2) == 0 && "Timer for ODA is not applicable because we never use ODA.");
		assert(get_detailed_timer().GetTime(0) == 0 && "Timer for build MP is not applicable.");
		assert(get_detailed_timer().GetTime(1) == 0 && "Timer for prejections is not applicable because we never use that technique.");
		assert(get_detailed_timer().GetTime(2) == 0 && "Time for solve ODA is not even used in PNB.");
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
			// We could record it in CPX_CALLBACKCONTEXT_LOCAL_PROGRESS
			// but sometimes that context is not called
			// and UB is not recorded.
			benders_optimal_incumbents_.push_back(upper_bound_);
			return; 
		}

		// We need to aggregate optimality cuts for the parent node
		AggregateOptimalityCuts(tree_nodes_by_stage_[current_stage_ - 1][0]);

		for (auto node : tree_nodes_by_stage_[1]) {
			detailed_timer_.AddTime(3, node->getDataPtr()->build_time_);
			detailed_timer_.AddTime(4, node->getDataPtr()->cut_generation_time_);
			solution_times_.AddTime(3, node->getDataPtr()->solution_time_);
		}
	}

	void BranchAndBendersCut::ConstructRootNodeCuts()
	{
		// TODO: maybe add cuts through the pool as user / lazy cuts https://www.ibm.com/docs/en/icos/12.8.0.0?topic=uclcp-what-are-pools-user-cuts-lazy-constraints
		assert(current_stage_ == 0);

		// *****************
		// Preprocess with L&P cuts
		// *****************
		if (lp_rounds_limit_) {
			// TODO: implement parameter to start it. We can choose to construct cuts here or in B&B for node with depth 0
			// Prepare parameters
			parallel_benders::Parameter lift_and_project_param = parallel_benders::Parameter(parameter_);
			lift_and_project_param.SetParameterValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS, false);
			lift_and_project_param.SetParameterValue(parallel_benders::PNB_PRINT_SUBPROBLEMS, false);
			lift_and_project_param.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, lp_rounds_limit_); // will anyways be < global_lp_rounds_limit_
			lift_and_project_param.SetParameterValue(parallel_benders::PNB_LP_SWITCHES_LIMIT, lp_rounds_limit_); // if cont solution is found, we construct opt cuts and continue

			// Create pnb for L&P cuts
			ParallelNestedBenders* lift_and_project_pnb = new ParallelNestedBenders(smi_);
			lift_and_project_pnb->setParameters(lift_and_project_param);
			lift_and_project_pnb->setTestsetProblemInstanceNames(testset_name_, problem_name_, instance_name_ + "_ROOT_LP_CUTS");
			lift_and_project_pnb->getMutableConsoleMutex() = console_mutex_ptr_; // we share the console as the resource
			lift_and_project_pnb->Init();
			lift_and_project_pnb->setGlobalLPRoundsLimit(lp_rounds_limit_);

#ifdef NDEBUG
			lift_and_project_pnb->setLogLevel(google::WARNING); // TODO: it will crash, no INFO output for cont phase in release
#endif
			// Find solution
			// We stop the process after the total number of L&P rounds
			// reaches lp_rounds_limit_ (PNB_SOLUTION_GLOBAL_LP_ROUNDS_LIMIT status)
			lift_and_project_pnb->Solve();
			
			// TODO: also here should be time limit
			//LOG_IF(FATAL, lift_and_project_pnb->getSolutionCode() != parallel_benders::PNB_SOLUTION_GLOBAL_LP_ROUNDS_LIMIT &&
			//	lift_and_project_pnb->getSolutionCode() != parallel_benders::PNB_SOLUTION_OPTIMAL);
			LOG_IF(WARNING, lift_and_project_pnb->getSolutionCode() == parallel_benders::PNB_SOLUTION_OPTIMAL) 
				<< "Optimal solution found during the L&P rounds before the B&B procedure.";

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
				for (int scenario = 0; scenario < tree_nodes_by_stage_[1].size(); scenario++) {
					if (lift_and_project_pnb->getTreeNode(1, scenario)->getDataPtr()->warm_start_) {
						ws = (lift_and_project_pnb->getTreeNode(1, scenario)->getDataPtr()->warm_start_.get())->clone(); // deep copy
						tree_nodes_by_stage_[1][scenario]->getDataPtr()->warm_start_.reset(ws);
					}
				}
			}
			if (!lift_and_project_pnb->getTreeNode(0, 0)->getDataPtr()->optimality_cuts_.empty() || !lift_and_project_pnb->getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.empty()) {
				getTreeNode(0, 0)->getDataPtr()->num_thetas_ = stage_data_[0]->num_thetas_;
				getTreeNode(0, 0)->getDataPtr()->changed_aggregation_level_ = true;
				createRoundRobinAggregation(tree_nodes_by_stage_[0][0]->getDataPtr(), tree_nodes_by_stage_[0][0]->numChildren(), getTreeNode(0, 0)->getDataPtr()->num_thetas_, (int)std::ceil((double)tree_nodes_by_stage_[0][0]->numChildren() / getTreeNode(0, 0)->getDataPtr()->num_thetas_), num_oda_aggregates_);
				getTreeNode(0, 0)->getDataPtr()->active_aggregates_ = boost::dynamic_bitset<>((getTreeNode(0, 0)->getDataPtr()->num_thetas_)); // Gets initialized with 0
				getTreeNode(0, 0)->getDataPtr()->active_aggregates_.set();
				getTreeNode(0, 0)->getDataPtr()->was_under_threshold_ = boost::dynamic_bitset<>(getTreeNode(0, 0)->getDataPtr()->num_thetas_); // Gets initialized with 0
			}

			// Calculate number cuts
			// - optimality (w/o PNB CGP cuts)
			int cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += lift_and_project_pnb->getStageData(i)->added_optimality_cuts_;
				cut_counter += lift_and_project_pnb->getStageData(i)->added_cgp_optimality_cuts_; // we cannot distinguish them in B&B
			}
			stage_data_[current_stage_]->added_optimality_cuts_ += cut_counter;

			// - feasibility
			cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += lift_and_project_pnb->getStageData(i)->added_feasibility_cuts_;
				cut_counter += lift_and_project_pnb->getStageData(i)->added_cgp_feasibility_cuts_; // we cannot distinguish them in B&B
			}
			stage_data_[current_stage_]->added_feasibility_cuts_ += cut_counter;

			// - epigraph
			stage_data_[current_stage_]->added_epigraph_cuts_ += getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.size();
			
			// - domain
			cut_counter = 0;
			for (int i = 0; i < num_stages_; i++) {
				cut_counter += lift_and_project_pnb->getStageData(i)->added_domain_cuts_;
			}
			stage_data_[current_stage_]->added_domain_cuts_ += cut_counter;

			// - continuous optimality cuts if use_first_phase_
			cont_phase_pnb_ = lift_and_project_pnb->getContPhasePNB();

			// Timers
			// TODO: use loop here
			lp_cuts_timer_.AddTime(1, lift_and_project_pnb->get_lp_cuts_timer().GetTime(1));
			lp_cuts_timer_.AddTime(2, lift_and_project_pnb->get_lp_cuts_timer().GetTime(2));
			lp_cuts_timer_.AddTime(3, lift_and_project_pnb->get_lp_cuts_timer().GetTime(3));
			lp_cuts_timer_.AddTime(4, lift_and_project_pnb->get_lp_cuts_timer().GetTime(4));
			lp_cuts_timer_.AddTime(5, lift_and_project_pnb->get_lp_cuts_timer().GetTime(5));

			solution_times_.AddTime(3, lift_and_project_pnb->get_solution_times().GetTime(3));
			solution_times_.AddTime(4, lift_and_project_pnb->get_solution_times().GetTime(4));

			detailed_timer_.AddTime(3, lift_and_project_pnb->get_detailed_timer().GetTime(3));
			detailed_timer_.AddTime(4, lift_and_project_pnb->get_detailed_timer().GetTime(4));

			delete lift_and_project_pnb;			
		}
		// *****************
		// Generate continuous optimality cuts
		// *****************
		// If we use L&P rounds,
		// cont. optimality cuts will be constructed in lift_and_project_pnb above
		else if (use_first_phase_)
		{
			current_iteration_ = 1;
			solution_times_.Start(4);
			HandleContinuousPhase();
			solution_times_.Stop(4);
		}

		// If no theta cuts are present, generate them.
		// Without theta cuts the problem is unbounded as theta is unbounded.
		if (getTreeNode(0,0)->getDataPtr()->optimality_cuts_.empty() && getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.empty()) {
			solution_times_.Start(4);
			osi_solver_->initialSolve();
			assert(osi_solver_->isProvenOptimal());
			StoreData(osi_solver_, getMutableTreeNode(0, 0));
			ConstructOptimalityCuts();

			// Calculate these cuts as continuous cuts
			// For that we creat the cont phase PNB,
			cont_phase_pnb_ = std::make_shared<ParallelNestedBenders>(smi_);
			// update the cut counter in it
			cont_phase_pnb_->getMutableStageData(0)->added_optimality_cuts_.store(stage_data_[0]->added_optimality_cuts_.load());
			// and reset the number of optimality cuts
			stage_data_[0]->added_optimality_cuts_ = 0;
			solution_times_.Stop(4);
		}

		LOG_IF(FATAL, !getTreeNode(0,0)->getDataPtr()->feasibility_cuts_.empty()) << "Feasibility cuts not implemented (not tested)";
		LOG_IF(FATAL, !getTreeNode(0,0)->getDataPtr()->domain_cuts_.empty()) << "Domain cuts not implemented (not tested)";

		// Add all cuts from above
		current_stage_ = 0;
		// call this method to activate thetas,
		// i.e., set their coef to 1
		AppendColsToSubproblem(osi_solver_, getMutableTreeNode(0, 0));
		// add cuts from containers to the formulation of the MP
		AppendCutsToSubproblem(osi_solver_, tree_nodes_by_stage_[0][0]);
		DeleteSubproblem(nullptr, tree_nodes_by_stage_[0][0]);

#ifndef NDEBUG
		/*We assume that all cuts constructed for the root node
		have cut_creation_thread -1.
		As for now in PNB getCurrentThreadId() always returns -1,
		and it should be true.*/
		// TODO: check the statement above here
#endif
	}

	void BranchAndBendersCut::SetUpMPSolver()
	{
		assert(current_stage_ == 0);
		OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
		cpx_solver->messageHandler()->setLogLevel(4); // Log everything // TODO: change it
		LOG_IF(FATAL, getTreeNode(0,0)->getDataPtr()->optimality_cuts_.empty() && getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.empty()) << "When problem has no theta cuts, it is unbounded when B&B started as theta is not bounded.";
		
		int status;

		// Set the CPLEX callback
		BranchAndBendersCut* user_data = this;
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXLPptr lp_ptr = cpx_solver->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
		// TODO: do we need all of them
		status = CPXcallbacksetfunc(env_ptr, lp_ptr, 
			CPX_CALLBACKCONTEXT_CANDIDATE // - found integer solution
			| CPX_CALLBACKCONTEXT_RELAXATION // - found infeasible (some int vars have non-int solution)
			| CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS // - update global UB. Locks other threads
			| CPX_CALLBACKCONTEXT_THREAD_UP // - activate thread, is different to thread creation and might happen several times for one thread
			| CPX_CALLBACKCONTEXT_THREAD_DOWN  // - deactivate thread, is different to thread creation and might happen several times for one thread
#ifndef NDEBUG
			| CPX_CALLBACKCONTEXT_LOCAL_PROGRESS
			| CPX_CALLBACKCONTEXT_BRANCHING
#endif
			, benders_callback, user_data);
		if (status) {
			fprintf(stderr, "Error in CPXcallbacksetfunc, status = %d.\n", status);
		}

		// Set threads number for B&B
		// for CPLEX in Benders it was always mutlithread
		// value 0 in CPLEX has the same meaning as in our implementation -- use all CPUs
		// TODO: parameter to set num_cpu for B&B specifically (explicitely or parallel_mode)
		CPXsetintparam(env_ptr, CPX_PARAM_THREADS, num_cpu_);
		//TODO: temp
		// Display the detailed solution log
#ifndef NDEBUG
		CPXsetintparam(env_ptr, CPX_PARAM_MIPINTERVAL, 1); // detailed log
		CPXsetintparam(env_ptr, CPX_PARAM_MIPDISPLAY, 5); // display all the information from the log
#endif
		
		// Dynamic Search
		//CPXsetintparam(env_ptr, CPXPARAM_MIP_Strategy_Search, CPX_MIPSEARCH_TRADITIONAL);  // no dynamic search (however, it is compatible with generic callbacks)
		//CPXsetintparam(env_ptr, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);  // for older CPLEX version
		
		// Preprocessing and Presolve
		//CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_Relax, CPX_OFF);
		//CPXsetintparam(env_ptr, CPX_PARAM_RELAXPREIND, CPX_OFF); // for older CPLEX version

		//CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_RepeatPresolve, CPX_OFF);
		//CPXsetintparam(env_ptr, CPX_PARAM_REPEATPRESOLVE, CPX_OFF); // for older CPLEX version

		//CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_NumPass, CPX_OFF);
		//CPXsetintparam(env_ptr, CPX_PARAM_PREPASS, CPX_OFF); // for older CPLEX version

		CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_Reduce, CPX_PREREDUCE_NOPRIMALORDUAL); // it is turned off automatically for cuts
		CPXsetintparam(env_ptr, CPX_PARAM_REDUCE, CPX_PREREDUCE_NOPRIMALORDUAL); // for older CPLEX version

		CPXsetintparam(env_ptr, CPX_PARAM_PREREFORM, CPX_OFF); // it is turned off automatically for cuts
		
		//CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_Aggregator, CPX_OFF);
		//CPXsetintparam(env_ptr, CPX_PARAM_AGGIND, CPX_OFF);  // for older CPLEX version

		//CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_Presolve, CPX_OFF);  // no presolve
		//CPXsetintparam(env_ptr, CPX_PARAM_PREIND, CPX_OFF); // for older CPLEX version
		//CPXsetintparam(env_ptr, CPXPARAM_Preprocessing_Symmetry, CPX_OFF); // no symmetry breaking		
		//CPXsetintparam(env_ptr, CPX_PARAM_SYMMETRY, CPX_OFF); // for older CPLEX version
		
		// Advanced start		
		//CPXsetintparam(env_ptr, CPXPARAM_Advance, CPX_OFF); // no advance start
		//CPXsetintparam(env_ptr, CPX_PARAM_ADVIND, CPX_OFF); // for older CPLEX version
		
		// Cuts
		CPXsetintparam(env_ptr, CPXPARAM_MIP_Limits_CutsFactor, 1.0); // no cuts
		CPXsetdblparam(env_ptr, CPX_PARAM_CUTSFACTOR, 1.0); // for older CPLEX version
		
		CPXsetdblparam(env_ptr, CPXPARAM_MIP_Cuts_Nodecuts, -1); // do not generate cuts for root node, impacts user cuts in pools but not callback 
		// I am not sure what is the difference between the one above and below
		CPXsetdblparam(env_ptr, CPXPARAM_MIP_Limits_CutPasses, -1); // turn off cut rounds for root node, also influences 
		CPXsetdblparam(env_ptr, CPX_PARAM_CUTPASS, -1); // for older CPLEX version

		// Heuristics
		CPXsetintparam(env_ptr, CPXPARAM_MIP_Strategy_FPHeur, -1);// turn off feasibility pump heuristic
		CPXsetintparam(env_ptr, CPX_PARAM_FPHEUR, -1); // for older CPLEX version
		CPXsetintparam(env_ptr, CPXPARAM_MIP_Strategy_HeuristicEffort, 0); // NB: set non-default value for either this or the next param but not for two simultaniously 
		//CPXsetintparam(env_ptr, CPXPARAM_MIP_Strategy_HeuristicFreq, -1); // turn off periodic node heuristic
		//CPXsetintparam(env_ptr, CPX_PARAM_HEURFREQ, -1); // for older CPLEX version
		CPXsetintparam(env_ptr, CPXPARAM_MIP_Strategy_RINSHeur, -1); // turn off RINS heuristic
		CPXsetintparam(env_ptr, CPX_PARAM_RINSHEUR, -1); // for older CPLEX version	
	}

	std::shared_ptr<ThreadBranchAndBendersCut> BranchAndBendersCut::getCopyForThread(int thread_id)
	{
		// TODO: delete the pointers of thread_copy_map_mutex_ destruction.
		// Just in case because we delete them anyways when thread goes down.

		{
			std::lock_guard<std::mutex> unique_lock(thread_copy_map_mutex_);
			std::map<int, std::shared_ptr<ThreadBranchAndBendersCut>>::iterator it = thread_copy_map_.find(thread_id);
			
			if (it != thread_copy_map_.end())
				return (*it).second;
			else {
				return std::shared_ptr<ThreadBranchAndBendersCut>(nullptr);
			}		
		}
		return std::shared_ptr<ThreadBranchAndBendersCut>(nullptr);
	}

	void BranchAndBendersCut::createCopyForThread(int thread_id)
	{
		// TODO: why in callback it prints "CPLEX Error  1013: Bad parameter number to CPLEX parameter routine."
		ThreadBranchAndBendersCut* thread_copy = nullptr;
		int num_theta = 0;
		std::vector< std::vector<int> > nodes_in_cut;
		{
			std::lock_guard<std::mutex> unique_lock(thread_copy_map_mutex_);

			// Initialize new bbc from current data
			thread_copy = new ThreadBranchAndBendersCut(smi_);
			thread_copy_map_.emplace(thread_id, std::shared_ptr<ThreadBranchAndBendersCut>(thread_copy));
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
		for (int stage = 0; stage < num_stages_; stage++)
		{
			// In thread data we can
			// distinguish between normal and CGP cuts in B&B
			stage_data_[stage]->added_cgp_optimality_cuts_ = 0;
			stage_data_[stage]->added_cgp_feasibility_cuts_ = 0;
		}
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->num_thetas_ = num_theta;
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->nodes_in_cut_ = std::move(nodes_in_cut);
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->active_aggregates_ = boost::dynamic_bitset<>(num_theta); // Gets initialized with 0
		thread_copy->tree_nodes_by_stage_[0][0]->getDataPtr()->was_under_threshold_ = boost::dynamic_bitset<>(num_theta); // Gets initialized with 0
		
		{
			std::shared_lock<std::shared_mutex> shared_lock(thread_data_mutex_);
			// Copy warm start
			if (warm_start_) {
				for (int stage = 1; stage < num_stages_; stage++) { // start from 2nd stage - we never solve MP of ThreadBBC 
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

			// Add new Benders and L&P cuts from the main loop
			// Here we deep copy the cuts since their attribs might be changed during the PNB Solve()        
			assert(num_stages_ == 2 && "We copy cuts only from the first stage");
			parallel_benders::BendersNodeData::cut_container::const_iterator it = getTreeNode(0, 0)->getDataPtr()->feasibility_cuts_.begin();
			for (; it != getTreeNode(0, 0)->getDataPtr()->feasibility_cuts_.end(); ++it) {
				std::shared_ptr<parallel_benders::CutStuff> cut_pointer = std::make_shared<parallel_benders::CutStuff>(*it->get());
				cut_pointer->cut_creation_iteration_ = 0;
				cut_pointer->column_set_ = parallel_benders::PNB_COL_SET_NONE; // it will be added to both - prime and double-prime - sides of MP CGP
				AddCutToContainer(thread_copy->getMutableTreeNode(0, 0), cut_pointer);
			}

			it = getTreeNode(0, 0)->getDataPtr()->domain_cuts_.begin();
			for (; it != getTreeNode(0, 0)->getDataPtr()->domain_cuts_.end(); ++it) {
				std::shared_ptr<parallel_benders::CutStuff> cut_pointer = std::make_shared<parallel_benders::CutStuff>(*it->get());
				cut_pointer->cut_creation_iteration_ = 0;
				assert(cut_pointer->column_set_ == parallel_benders::PNB_COL_SET_NONE);
				AddCutToContainer(thread_copy->getMutableTreeNode(0, 0), cut_pointer);
			}

			it = getTreeNode(0, 0)->getDataPtr()->optimality_cuts_.begin();
			for (; it != getTreeNode(0, 0)->getDataPtr()->optimality_cuts_.end(); ++it) {
				std::shared_ptr<parallel_benders::CutStuff> cut_pointer = std::make_shared<parallel_benders::CutStuff>(*it->get());
				cut_pointer->cut_creation_iteration_ = 0;
				cut_pointer->column_set_ = parallel_benders::PNB_COL_SET_NONE;
				AddCutToContainer(thread_copy->getMutableTreeNode(0, 0), cut_pointer);
			}

			it = getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.begin();
			for (; it != getTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.end(); ++it) {
				std::shared_ptr<parallel_benders::CutStuff> cut_pointer = std::make_shared<parallel_benders::CutStuff>(*it->get());
				cut_pointer->cut_creation_iteration_ = 0;
				assert(cut_pointer->column_set_ == parallel_benders::PNB_COL_SET_NONE);
				AddCutToContainer(thread_copy->getMutableTreeNode(0, 0), cut_pointer);
			}
		}

		// reset all cut pointers
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_iteration_start_optimality_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->optimality_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_in_lp_cgp_start_optimality_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->optimality_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_round_start_epigraph_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_in_lp_cgp_start_epigraph_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->epigraph_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_round_start_domain_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->domain_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_in_lp_cgp_problem_start_domain_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->domain_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_iteration_start_feasibility_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->feasibility_cuts_.end();
		thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->last_in_lp_cgp_problem_start_feasibility_ = thread_copy->getMutableTreeNode(0, 0)->getDataPtr()->feasibility_cuts_.end();
	}

	double BranchAndBendersCut::getLB()
	{
		int status;
		double lb;
		OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_);
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXLPptr lp_ptr = cpx_solver->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
		status = CPXgetbestobjval(env_ptr, lp_ptr, &lb);
		if (status) {
			fprintf(stderr, "Error in CPXgetbestobjval, status = %d.\n", status);
			LOG(FATAL) << "Did you query LB inside callback?";
		}
		return lb;
	}

	ThreadBranchAndBendersCut::ThreadBranchAndBendersCut(SmiScnModel* smi_model_ptr)
		: BranchAndBendersCut(smi_model_ptr),lp_cuts_constructed_(false),current_context_(nullptr),current_contextid_(-1)
	{
	}

	ThreadBranchAndBendersCut::~ThreadBranchAndBendersCut()
	{
	}

	int ThreadBranchAndBendersCut::getCurrentThreadId() const
	{
		int status = 0;
		int thread_id;
		status = CPXcallbackgetinfoint(current_context_, CPXCALLBACKINFO_THREADID, &thread_id);
		LOG_IF(FATAL, status) << "Error in CPXcallbackgetinfoint: status " << status;
		return thread_id;
	}

	int ThreadBranchAndBendersCut::StoreNodeData()
	{
		assert(current_stage_ == 0);
		assert(current_contextid_ == CPX_CALLBACKCONTEXT_CANDIDATE ||
			   current_contextid_ == CPX_CALLBACKCONTEXT_RELAXATION ||
			   current_contextid_ == CPX_CALLBACKCONTEXT_BRANCHING);
		int status = 0;

		// TODO: do we need it
		/*
		No limitations when it can be called.
		*/
		int num_processed_nodes;
		status = CPXcallbackgetinfoint(current_context_, CPXCALLBACKINFO_NODECOUNT, &num_processed_nodes);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetinfoint: status = %d\n", status);
			return status;
		}

		parallel_benders::BendersNodeData* mp_node_pointer = tree_nodes_by_stage_[0][0]->getDataPtr();
		int num_x_cols = mp_node_pointer->cols_;
		int num_theta = mp_node_pointer->num_thetas_;
		if (!num_theta) {
			num_theta = getNumThetasByStage(tree_nodes_by_stage_[0][0]);
		}
		assert(mp_node_pointer->num_thetas_ == getNumThetasByStage(tree_nodes_by_stage_[0][0]));

		std::vector<double> solution(num_x_cols + num_theta);
		if (current_contextid_ == CPX_CALLBACKCONTEXT_CANDIDATE) {
			assert(!generate_lp_cuts_);
			status = CPXcallbackgetcandidatepoint(current_context_, solution.data(), 0, num_x_cols + num_theta - 1, NULL);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetcandidate: status = %d\n", status);
				return status;
			}
		}
		else if (current_contextid_ == CPX_CALLBACKCONTEXT_RELAXATION) {
			status = CPXcallbackgetrelaxationpoint(current_context_, solution.data(), 0, num_x_cols + num_theta - 1, NULL);
			if (status) {
				fprintf(stderr, "Error in CPXcallbackgetrelaxationpoint: status = %d\n", status);
				return status;
			}
		}

		// Get the current node solution
		if (current_contextid_ == CPX_CALLBACKCONTEXT_CANDIDATE || current_contextid_ == CPX_CALLBACKCONTEXT_RELAXATION)
		{
			mp_node_pointer->col_solution_.resize(num_x_cols);
			mp_node_pointer->col_solution_.assign(solution.begin(), solution.begin() + num_x_cols);
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
		}		

		if (current_contextid_ == CPX_CALLBACKCONTEXT_RELAXATION) {
			assert(current_stage_ == 0);
			//Check for integrality
			for (int index : core_->getBinaryCols(current_stage_)) {
				double value = mp_node_pointer->col_solution_[index];
				double fractionality = abs(round(value) - value); // distance from 0 or 1
				if (!parallel_benders::CompareForEquality(fractionality, 0.0, integer_tolerance_)) {
					assert(fractionality > 0 && fractionality <= 0.5);
					//std::map cannot store records with duplicated keys
					while (fractional_variables_.find(fractionality) != fractional_variables_.end()) {
						fractionality += zero_tolerance_ * 2;
					}
					DLOG_IF(FATAL, fractional_variables_.find(fractionality) != fractional_variables_.end());
					fractional_variables_.emplace(fractionality, make_pair(index, value));
				}
			}

			assert(!fractional_variables_.empty() && "There must be fractional vars because we would not end up in CPX_CALLBACKCONTEXT_RELAXATION");
			generate_lp_cuts_ = true;
		}
		else if (current_contextid_ == CPX_CALLBACKCONTEXT_CANDIDATE) {
			stage_old_lower_upper_bounds_[0].first = stage_lower_upper_bounds_[0].first;
			stage_old_lower_upper_bounds_[0].second = stage_lower_upper_bounds_[0].second;
			stage_lower_upper_bounds_[0].first = mp_node_pointer->lower_bound_; //  q(x) + theta: solution with thetas
			stage_lower_upper_bounds_[0].second = mp_node_pointer->solution_value_; // q(x): solution w/o thetas
			old_lower_bound_ = lower_bound_;
			lower_bound_ = mp_node_pointer->lower_bound_;
			// UB will be calculated in ConstructOptimalityCuts()
			// after 2nd-stage subproblems are evaluated
		}	

		return status;
	}

	int ThreadBranchAndBendersCut::getCurrentNodeDepth()
	{
		LOG_IF(FATAL, !CanQueryNodeInfo()) << "Cannot getCurrentNodeDepth() because cannot query node data.";
		
		int status;
		int node_depth;
		status = CPXcallbackgetinfoint(current_context_, CPXCALLBACKINFO_NODEDEPTH, &node_depth);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetinfolong: status = %d\n", status);
			return status;
		}

		return node_depth;		
	}

	int ThreadBranchAndBendersCut::getCurrentNodeId()
	{
		LOG_IF(FATAL, !CanQueryNodeInfo()) << "Cannot getCurrentNodeId() because cannot query node data.";
		
		int status;
		int node_id;
		status = CPXcallbackgetinfoint(current_context_, CPXCALLBACKINFO_NODEUID, &node_id);
		if (status) {
			LOG(FATAL) << "Error in CPXcallbackgetinfolong: status = " << status;
		}

		return node_id;
	}

	void ThreadBranchAndBendersCut::setCurrentContextData(CPXCALLBACKCONTEXTptr context, CPXLONG contextid)
	{
		assert(this != nullptr && "Construct ThreadBBC");
		current_context_ = context;
		current_contextid_ = contextid;
	}

	void ThreadBranchAndBendersCut::ConstructLiftAndProjectCuts()
	{
		current_stage_ = 0;
		assert(generate_lp_cuts_);
		forward_ = false;
		int num_nodes = tree_nodes_by_stage_[current_stage_].size();

		int num_cuts = ceil(fractional_variables_.size() * (cuts_for_fract_vars_ / 100.0)); //at least cut for one fract variable
		assert(num_cuts > 0 && num_cuts <= fractional_variables_.size());
		stage_data_[current_stage_]->lp_cgp_counter_ += num_cuts; // every cut requires CGP to solve

		double theta_sum = std::accumulate(tree_nodes_by_stage_[0][0]->getDataPtr()->next_stage_approximation_.begin(), tree_nodes_by_stage_[0][0]->getDataPtr()->next_stage_approximation_.end(), 0.0);
		LOG_IF(INFO, log_level_ <= google::INFO) << current_iteration_ << " Iteration - L&P Switch " << lp_switch_counter_ << "/" << lp_switches_limit_ << " - L&P round " << current_lp_round_ << "/" << lp_rounds_limit_ << ": " << fractional_variables_.size() << " fractional variables, " << num_cuts << " L&P cuts to generate.";

		assert(thread_pool_.active() == 0 && "We generate L&P cuts on the first stage and should always have only 1 active thread.");
		// Set round specifics to all dual cgp
		lp_cuts_timer_.Start(2);
		for (auto id_solver_pair : osi_solver_map_) {
			assert(!(current_lp_round_ == 1 && id_solver_pair.second->getNumCols() > 0 && "In the first round all osi should be empty"));
			if (id_solver_pair.second->getNumCols() > 0) { // if solver is not empty
				thread_pool_.schedule(boost::bind(&ThreadBranchAndBendersCut::SetDualCGPRoundSpecifics, this, boost::ref(*id_solver_pair.second), tree_nodes_by_stage_[0][0]));
			}
		}
		thread_pool_.wait(0);
		lp_cuts_timer_.Stop(2);

		// Set round specifics to all existing cgp pnb
		lp_cuts_timer_.Start(1);
		for (auto id_pnb_pair : cgp_pnb_map_) {
			thread_pool_.schedule(boost::bind(&ThreadBranchAndBendersCut::SetLnPCGPRoundSpecifics, this, boost::ref(*id_pnb_pair.second), tree_nodes_by_stage_[0][0]));
		}
		thread_pool_.wait(0); // We wait until all threads except the current one (for MP) are finished
		lp_cuts_timer_.Stop(1);

		// Generate L&P cuts for all or some fract. variables
		std::map<double, std::pair<int, double>>::reverse_iterator rit = fractional_variables_.rbegin(); // reverse iterator since map is ordered ascending and we start from the most fractional values
		for (int i = 0; i < num_cuts; i++) {
			if (num_cpu_ > 1 && num_cuts > 1 && lp_parallel_mode_ >= parallel_benders::PNB_PARAL_FRACTS) {
				thread_pool_.schedule(boost::bind(&ThreadBranchAndBendersCut::GenerateLiftAndProjectCut, this, i, rit->second));
			}
			else { // use the current thread
				GenerateLiftAndProjectCut(i, rit->second);
			}
			++rit; // goes backwards
		}
		thread_pool_.wait(0);

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
			LOG_IF(WARNING, log_level_ <= google::WARNING) << "No L&P cuts build in this round. The node will be branched." << std::endl;
			if (current_lp_round_ == 1) {
				lp_switch_counter_--; // we do no want to count as switch with one cut-less lp_round. Substract here but increase later
			}
			current_lp_round_ = 0;
			lp_cuts_constructed_ = true;
		}

		// TODO: we do not want to reset last_iteration_start_optimality_
		// because of CGP cuts. Furthermore, we do not build MP here,
		// so probably do not need to delete anything.
		// DeleteSubproblem(nullptr, tree_nodes_by_stage_[0][0]);

		//Move CGP cuts to the main loop
		if (!tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_.empty() || !tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.empty()) {
			// TODO: the line below will probably fail because we do not execute delete now
			//assert(tree_nodes_by_stage_[0][0]->getDataPtr()->last_iteration_start_optimality_ == tree_nodes_by_stage_[0][0]->getDataPtr()->optimality_cuts_.end() && "Should be like that after DeleteSubproblem()");
			//assert(tree_nodes_by_stage_[0][0]->getDataPtr()->last_iteration_start_feasibility_ == tree_nodes_by_stage_[0][0]->getDataPtr()->feasibility_cuts_.end());
			stage_data_[current_stage_]->added_cgp_feasibility_cuts_ += tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.size(); // we always add feas cuts

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

			stage_data_[current_stage_]->added_cgp_optimality_cuts_ += added_opt_cuts;
			LOG_IF(INFO, log_level_ <= google::INFO && evaluated_violation) << "Added " << added_opt_cuts << "/" << tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_.size() << " CGP optimality cuts and " << tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.size() << " CGP feasibility cuts (considering DistanceViolation).";
			LOG_IF(INFO, log_level_ <= google::INFO && !evaluated_violation) << "Added " << added_opt_cuts << " CGP optimality cuts and " << tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.size() << " CGP feasibility cuts.";

			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_optimality_cuts_.clear();
			tree_nodes_by_stage_[0][0]->getDataPtr()->lp_round_feasibility_cuts_.clear();
		}

		LOG_IF(FATAL, num_added_feasibility_cuts_) << "Feasibility cuts not implemented (not tested)";

		// TODO: clear cgp_pnb_map_ or clear it when we find candidate (to spare space???)

		generate_lp_cuts_ = false;
	}

	int ThreadBranchAndBendersCut::AddLazyCutToNode(const parallel_benders::CutStuff& cut_stuff)
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

		status = CPXcallbackrejectcandidate(current_context_, 1, rowCut.row().getNumElements(), 
			&rhs, &sns, &rmatbeg, 
			const_cast<int*>(rowCut.row().getIndices()),
			const_cast<double*>(rowCut.row().getElements()));
		// We can also add these (Benders) cuts as local cuts
		// i.e., use them only with subtree,
		// with CPXcallbackrejectcandidatelocal()
		
		LOG_IF(WARNING, status) << "Error in CPXcallbackrejectcandidate: status = " << status;

		return status;
	}

	int ThreadBranchAndBendersCut::AddUserCutToNode(const parallel_benders::CutStuff& cut_stuff)
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

		int purgeable = CPX_USECUT_FORCE; // TODO: think about this https://www.ibm.com/docs/en/icos/12.8.0.0?topic=c-cpxxcutcallbackadd-cpxcutcallbackadd
		int is_local = 1;
		status = CPXcallbackaddusercuts(current_context_, 1, rowCut.row().getNumElements(),
			&rhs, &sns, &rmatbeg,
			const_cast<int*>(rowCut.row().getIndices()),
			const_cast<double*>(rowCut.row().getElements()),
			&purgeable, // TODO: try other purgable values CPX_USECUT_PURGE, CPX_USECUT_FILTER
			&is_local); // TODO: which value should it be: 1 - locally valid, 0 - globally valid

		LOG_IF(WARNING, status) << "Error in CPXcallbackaddusercuts: status = " << status;

		return status;
	}

	int ThreadBranchAndBendersCut::AppendCutsToNode(SmiTreeNode<parallel_benders::BendersNodeData*>* benders_tree_node_pointer)
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

		// We add optimality cuts only in contextid == CPX_CALLBACKCONTEXT_CANDIDATE
		// CGP optimality cuts are not added when they are generated (CPX_CALLBACKCONTEXT_RELAXATION)
		// but later in CPX_CALLBACKCONTEXT_CANDIDATE
		if (!benders_node_pointer->optimality_cuts_.empty() && current_contextid_ == CPX_CALLBACKCONTEXT_CANDIDATE) {
			// TODO: does it works for CGP cuts
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
					status = AddLazyCutToNode(**it);
					if (status) {
						LOG(WARNING) << "Error in AddCutToNode: status = " << status;
						return status;
					}
				}
			}
		}

		if (!benders_node_pointer->epigraph_cuts_.empty() && current_contextid_ == CPX_CALLBACKCONTEXT_RELAXATION) {
			// Add the cuts
			parallel_benders::BendersNodeData::cut_container::const_iterator it = benders_node_pointer->epigraph_cuts_.begin();
			if (benders_node_pointer->last_round_start_epigraph_ != benders_node_pointer->epigraph_cuts_.end()) {
				it = benders_node_pointer->last_round_start_epigraph_;
				if (current_stage_ > 0) // In non-first-stage nodes we must read all optimality cuts!
					//it = benders_node_pointer->optimality_cuts_.end();
					assert(false);
			}
			else
				it = benders_node_pointer->epigraph_cuts_.end();

			LOG_IF(FATAL, current_iteration_ == 1 && generate_lp_cuts_ && !cont_phase_it_limit_ && !use_first_phase_) << "Is it a problem?";  //changed_aggregation_level_ should be changed if we build lp cut in 1st iteration. Right?
			benders_tree_node_pointer->getDataPtr()->changed_aggregation_level_ = false;

			for (; it != benders_node_pointer->epigraph_cuts_.end(); ++it) {
				if ((*it)->to_use_) {
					status = AddUserCutToNode(**it);
					if (status) {
						LOG(WARNING) << "Error in AddUserCutToNode: status = " << status;
						return status;
					}
				}
			}
			benders_node_pointer->last_round_start_epigraph_ = benders_node_pointer->epigraph_cuts_.end();
		}

		return 0;
	}

} // end namespace branch_and_benders_cut

bool branch_and_benders_cut::ThreadBranchAndBendersCut::CanQueryNodeInfo()
{
	/*
		Can query node data - CPXCALLBACKINFO_NODEDEPTH and CPXCALLBACKINFO_NODEUID when the context is:
		CPX_CALLBACKCONTEXT_RELAXATION,
		CPX_CALLBACKCONTEXT_BRANCHING,
		CPX_CALLBACKCONTEXT_CANDIDATE but only if CPXCALLBACKINFO_CANDIDATE_SOURCE is CPX_LAZYCONSTRAINTCALLBACK_NODE.
	*/

	if (current_contextid_ == CPX_CALLBACKCONTEXT_CANDIDATE) {
		assert(!generate_lp_cuts_);
		
		//Can be only called in CPX_CALLBACKCONTEXT_CANDIDATE
		int candidate_source;
		int status;
		status = CPXcallbackgetinfoint(current_context_, CPXCALLBACKINFO_CANDIDATE_SOURCE, &candidate_source);
		if (status) {
			LOG(FATAL) << "Error in CPXcallbackgetinfolong for CPXCALLBACKINFO_CANDIDATE_SOURCE: status = " << status;
		}
		assert((candidate_source == CPX_LAZYCONSTRAINTCALLBACK_NODE
			|| candidate_source == CPX_LAZYCONSTRAINTCALLBACK_HEUR)
			&& "Why would we have another source?");
		/*
		- CPX_LAZYCONSTRAINTCALLBACK_MIPSTART, if the solution comes from a MIP start.
		- CPX_LAZYCONSTRAINTCALLBACK_HEUR, if the solution comes from a heuristic.
		- CPX_LAZYCONSTRAINTCALLBACK_USER, if the solution is provided by the user(via CPXXcallbackpostheursoln and CPXcallbackpostheursoln).
		- CPX_LAZYCONSTRAINTCALLBACK_NODE, if the solution comes from an integer feasible node.
		*/

		if (candidate_source == CPX_LAZYCONSTRAINTCALLBACK_HEUR) {
			// TODO: why we end up here sometimes (at least for SNDP_TestCase_16)
			LogSpecialCase(true, "Did not we disable heuristics? Will L&P CGP work correct for this case.", -1.0);
			return false;
		}
		
		if (candidate_source == CPX_LAZYCONSTRAINTCALLBACK_NODE) {
			return true;
		}
	}
	else if (current_contextid_ == CPX_CALLBACKCONTEXT_RELAXATION || current_contextid_ == CPX_CALLBACKCONTEXT_BRANCHING) {
		return true;
	}

	return false;
}

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


// Gurobi stuff
/*#include "OsiGrbSolverInterface.hpp"
extern "C" {
#include "gurobi_c.h"
}*/


namespace branch_and_benders_cut {

	int CPXPUBLIC benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle)
	{
		int status = 0;
		/*int me;*/
		BranchAndBendersCut* globaldata = static_cast<BranchAndBendersCut*>(cbhandle);
		//USERLOCALDATA* localdata = NULL;

		globaldata->lp_main_loop_iteration_round_ = make_unique<std::pair<int, int>>(1,1); //TODO: temp
		globaldata->current_stage_ = 0;
		globaldata->forward_ = true;

		/* Data structures to add the Benders' cut */

		//int k, nzcnt, cur_x_col, cur_v_col, cur_u_col, num_x_cols, num_nodes;
		int num_x_cols = globaldata->stage_data_[0]->num_columns_;
		//int beg = 0;
		//int worker_lp_sol_stat, purgeable, local, point;
		int integer_feasible;
		//double rhs;
		//double eps_ray = 1e-03;
		//char sense;

		////num_x_cols = globaldata->num_x_cols;
		////num_nodes = globaldata->num_nodes;

		//status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &me);
		//if (status) {
		//	fprintf(stderr, "Error in CPXcallbackgetinfoint: status = %d\n", status);
		//	return status;
		//}

		assert(contextid == CPX_CALLBACKCONTEXT_CANDIDATE);
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
			return 0; //TODO: what do we do when MP is unbounded?
		}

		//Store Data
		parallel_benders::BendersNodeData* benders_node_pointer = globaldata->tree_nodes_by_stage_[globaldata->current_stage_][0]->getDataPtr();
		int num_theta = 1; //TODO: correctly identify the number of theta
		
		benders_node_pointer->col_solution_.resize(num_x_cols);
		status = CPXcallbackgetcandidatepoint(context, benders_node_pointer->col_solution_.data(), 0, num_x_cols - num_theta, NULL);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetcandidate: status = %d\n", status);
			return status;
		}

		benders_node_pointer->next_stage_approximation_.resize(num_theta);
		status = CPXcallbackgetcandidatepoint(context, benders_node_pointer->next_stage_approximation_.data(), num_x_cols, num_x_cols + num_theta - 1, NULL);
		if (status) {
			fprintf(stderr, "Error in CPXcallbackgetcandidate: status = %d\n", status);
			return status;
		}

		std::cout << "Hello, Hello, Hello!" << std::endl;

		globaldata->current_stage_ = 1;
		globaldata->forward_ = false;
		int num_nodes = globaldata->tree_nodes_by_stage_[globaldata->current_stage_].size();
		// Do some kind of shuffling!
		//std::vector<int> test_vector(num_nodes);
		//std::iota(test_vector.begin(),test_vector.end(),0);
		//std::random_shuffle(test_vector.begin(),test_vector.end());
		//for ( int i : test_vector ){
		for (int i = 0; i < num_nodes; i++) {
			//thread_pool_.schedule(boost::bind(&ParallelNestedBenders::HandleSubproblem, this, tree_nodes_by_stage_[current_stage_][i]));
			globaldata->HandleSubproblem(globaldata->tree_nodes_by_stage_[globaldata->current_stage_][i]);
		}
		//thread_pool_.wait(0); // We wait until all threads are done

		//handle bounds here

		//if (infeasible_problems_.none() && (!infeasible_problem_)) { // TODO: if generate_feas_and_opt_cuts we need to change this
		// We need to aggregate optimality cuts for the parent node
		for (int i = 0; i < globaldata->tree_nodes_by_stage_[globaldata->current_stage_ - 1].size(); i++) {
			//thread_pool_.schedule(boost::bind(&ParallelNestedBenders::AggregateOptimalityCuts, this, tree_nodes_by_stage_[current_stage_ - 1][i]));
			//globaldata->AggregateOptimalityCuts(globaldata->tree_nodes_by_stage_[globaldata->current_stage_-1][i]);
		}

		for (auto id_solver_pair : globaldata->osi_solver_map_) {
			//id_solver_pair.second->reset(); // This method throws exception with Cplex
			id_solver_pair.second->loadProblem(0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
			// Workaround: Load empty problem. 
			assert(id_solver_pair.second->getNumCols() == 0);
		}

		for (auto node : globaldata->tree_nodes_by_stage_[globaldata->current_stage_]) {
			node->getDataPtr()->CleanUnusedMemory(globaldata->forward_);
		}

		//return status;
		globaldata->lp_main_loop_iteration_round_.reset(); //TODO: temp
		return 0;
	} /* END benders_callback */
	
	BranchAndBendersCut::BranchAndBendersCut(SmiScnModel* smi_model_ptr)
		: parallel_benders::ParallelNestedBenders(smi_model_ptr)
	{
	}

	BranchAndBendersCut::~BranchAndBendersCut()
	{
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

		int setup_timer = num_stages_;
		timer_.Start(setup_timer);

		bool made_one_turn = false;

		OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_); //TODO: from the very beginning it should be cpx solver

		//TODO: temp parameters
		print_subproblems_ = true;
		cont_phase_it_limit_ = 100;
		//TODO: for testing set one thread for CPLEX: for a means to force deterministic parallel optimization, see the parallel mode switch (ParallelMode, CPX_PRAM_PARALLELMODE) in the CPLEX Parameters Reference Manual.
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXsetintparam(env_ptr, CPX_PARAM_THREADS, 1); // Should be set to 1 usually!

		HandleContinuousPhase();

		timer_.Stop(setup_timer);
		wall_clock_solution_time_.Start(0);
		// Print initial solution line
		LOG_IF(INFO, log_level_ <= google::INFO) << left << std::setw(7) << "It." << std::setw(16) << "Lower Bound: " << std::setw(16) << "Upper Bound: " << std::setw(16) << "Relative Gap: ";
		// Do initialization stuff necessary for solve process
		bool not_stop_loop = false;

		// ------------------ Here comes the solution process itself

		// Solve MP
		SetUpMPSolver(cpx_solver); //TODO: add callbacks
		PrintSubproblem(cpx_solver, 0, "MP");
		SolveSolverInterface(cpx_solver);
		assert(current_stage_ == 1 && "Benders callback changes the stage to second-stage ()");
		current_stage_ = 0;
		StoreData(cpx_solver, tree_nodes_by_stage_[current_stage_][0]);
		parallel_benders::BendersNodeData* mp_node_pointer = tree_nodes_by_stage_[current_stage_][0]->getDataPtr();
		mp_node_pointer->col_solution_;
		mp_node_pointer->next_stage_approximation_;
		mp_node_pointer->solution_value_;
		mp_node_pointer->lower_bound_;


		// ------------------ Here ends solution process
		

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

	void BranchAndBendersCut::SetUpMPSolver(OsiCpxSolverInterface* cpx_solver)
	{
		cpx_solver->messageHandler()->setLogLevel(4); // Log nothing
		BuildSubproblem(cpx_solver, tree_nodes_by_stage_[current_stage_][0]);
		AppendColsToSubproblem(cpx_solver, tree_nodes_by_stage_[current_stage_][0]);
		//TODO: The CPLEX parameter that controls primaland dual reduction type(CPXPARAM_Preprocessing_Reduce) should be set to CPX_PREREDUCE_NOPRIMALORDUAL(0) or to CPX_PREREDUCE_PRIMALONLY(1) in order to turn off dual reductions.
		AppendCutsToSubproblem(cpx_solver, tree_nodes_by_stage_[current_stage_][0], parallel_benders::PNB_CUT_APPEND_TYPE_LAZY); //TODO: these are probably lazy cuts added via call back
		
		BranchAndBendersCut* user_data = this;
		CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
		CPXLPptr lp_ptr = cpx_solver->getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ALL);
		int status = CPXcallbacksetfunc(env_ptr, lp_ptr, CPX_CALLBACKCONTEXT_CANDIDATE, benders_callback, user_data);
		if (status) {
			fprintf(stderr, "Error in CPXcallbacksetfunc, status = %d.\n", status);
		}
	}

} // end namespace parallel_benders
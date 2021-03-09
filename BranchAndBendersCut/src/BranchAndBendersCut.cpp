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

		//TODO: temp parameters
		print_subproblems_ = true;
		cont_phase_it_limit_ = 100;

		HandleContinuousPhase();

		timer_.Stop(setup_timer);
		wall_clock_solution_time_.Start(0);
		// Print initial solution line
		LOG_IF(INFO, log_level_ <= google::INFO) << left << std::setw(7) << "It." << std::setw(16) << "Lower Bound: " << std::setw(16) << "Upper Bound: " << std::setw(16) << "Relative Gap: ";
		// Do initialization stuff necessary for solve process
		bool not_stop_loop = false;

		// ------------------ Here comes the solution process itself

		// Solve MP
		OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(osi_solver_); //TODO: from the very beginning it should be cpx solver
		SetUpMPSolver(cpx_solver); //TODO: add callbacks
		PrintSubproblem(cpx_solver, 0, "MP");
		SolveSolverInterface(cpx_solver);
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
	}

} // end namespace parallel_benders
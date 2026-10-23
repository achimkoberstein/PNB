//#include "OsiStochasticSolverInterface.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"
#include "BranchAndBendersCut.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"

OsiStochasticBranchAndBendersCutSolverInterface::OsiStochasticBranchAndBendersCutSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model)
: OsiStochasticParallelNestedBendersSolverInterface(osi_solver, smi_model), osi_(nullptr), last_used_algorithm_(-1)
{
	delete nested_benders_;
	nested_benders_ = new branch_and_benders_cut::BranchAndBendersCut(smi_model_);
}

OsiStochasticBranchAndBendersCutSolverInterface::~OsiStochasticBranchAndBendersCutSolverInterface()
{
}

void OsiStochasticBranchAndBendersCutSolverInterface::printSolutionToFile(const char* filename) const
{
	assertTriedToSolve();
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		LOG(WARNING) << "printSolutionToFile() not implemented for BBC";
	}
	else {
		OsiStochasticParallelNestedBendersSolverInterface::printSolutionToFile(filename);
	}
}

double OsiStochasticBranchAndBendersCutSolverInterface::getIntegerGap() const
{
	assertTriedToSolve();
	int value = 0.0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		parallel_benders::ParallelNestedBenders* cont_phase_pnb = getBBC()->cont_phase_pnb_.get();

		assert(getBBC()->cont_phase_it_limit_ == 0 && "We cannot take the solution of cont_phase_pnb_ in this case");
		if (getBBC()->integer_problems_.none()) {
			return 0.0;
		}
		else if (true) {
			LOG(WARNING) << "getIntegerGap() not implemented for BBC";
			return -1.0;
		}
		//else if (cont_phase_pnb && cont_phase_pnb->solution_code_ == parallel_benders::PNB_SOLUTION_OPTIMAL && bbc_->solved_counter_ > 0 && bbc_->solution_code_ == parallel_benders::PNB_SOLUTION_OPTIMAL) {
		//	assert(cont_phase_pnb->solved_counter_ == 1); // should be solved at least but not more than once
		//	if we used cont phase, we could take UB from the first iteration, since it will be the relaxation solution
		//		return parallel_benders::ComputeRelativeGap(bbc_->upper_bound_, cont_phase_pnb->upper_bound_, 1e-10);
		//}
		else {
			return -1.0;
		}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getIntegerGap();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumProcessedMipNodes() const
{
	assert(nested_benders_);
	return getBBC()->num_processed_nodes_;
}

double OsiStochasticBranchAndBendersCutSolverInterface::getInitRootNodeObjectiveValue() const
{
	assert(nested_benders_);
	return getBBC()->root_node_obj_value_;
}


branch_and_benders_cut::BranchAndBendersCut* OsiStochasticBranchAndBendersCutSolverInterface::getBBC() const
{
	return dynamic_cast<branch_and_benders_cut::BranchAndBendersCut*>(nested_benders_);
}

const double* OsiStochasticBranchAndBendersCutSolverInterface::getSolutionByNodeAndStage(int node_number, int stage) const
{
	assert(false && "Not tested for BBC.");
	return 0;
}

void OsiStochasticBranchAndBendersCutSolverInterface::initialSolve()
{
	int algorithm = getBBC()->parameter_.GetIntParamValue(parallel_benders::PNB_BENDERS);
	if (algorithm != parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		throw std::invalid_argument("PNB_BENDERS parameter is invalid. It can have only PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT value.");
	}
	if (getBBC()->parameter_.GetBoolParamValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS)) {
		setInstanceSpecsFile(instance_specification_file_);
	}
	nested_benders_->Init();
	nested_benders_->Solve();
	last_used_algorithm_ = algorithm;
}

void OsiStochasticBranchAndBendersCutSolverInterface::assertTriedToSolve() const
{
	if (last_used_algorithm_ == -1) {
		LOG(FATAL) << "Never called initialSolve() for this interface.";
	}
}

double OsiStochasticBranchAndBendersCutSolverInterface::getSolutionValueForScenario(int scenario)
{
	assert(false);
	//assert(scenario < nested_benders_->num_scenarios_);
	//return nested_benders_->computeScenarioValue(scenario);
	return 0.0;
}


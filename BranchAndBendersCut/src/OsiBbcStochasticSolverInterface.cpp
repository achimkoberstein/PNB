//#include "OsiStochasticSolverInterface.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"
#include "BranchAndBendersCut.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "SmiScnModel.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"

OsiStochasticBranchAndBendersCutSolverInterface::OsiStochasticBranchAndBendersCutSolverInterface(OsiSolverInterface* osi_solver, SmiScnModel* smi_model)
: OsiStochasticParallelNestedBendersSolverInterface(osi_solver, smi_model), bbc_(nullptr), osi_(nullptr), last_used_algorithm_(-1)
{
	bbc_ = new branch_and_benders_cut::BranchAndBendersCut(smi_model_);
}

OsiStochasticBranchAndBendersCutSolverInterface::~OsiStochasticBranchAndBendersCutSolverInterface()
{
	delete bbc_;
}

void OsiStochasticBranchAndBendersCutSolverInterface::initialSolve()
{
	last_used_algorithm_ = getParameters().GetIntParamValue(parallel_benders::PNB_BENDERS);
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		if (getParameters().GetBoolParamValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS)) {
			setInstanceSpecsFile(instance_specification_file_);
		}
		bbc_->Init();
		bbc_->Solve();
	}
	else {
		OsiStochasticParallelNestedBendersSolverInterface::initialSolve();
	}
}

double OsiStochasticBranchAndBendersCutSolverInterface::getObjValue() const
{
	assertTriedToSolve();
	double value = 0.0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		value = bbc_->upper_bound_; // TODO: updated UB in Solve()
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getObjValue();
	}

	return value;
}

void OsiStochasticBranchAndBendersCutSolverInterface::printSolutionToFile(const char* filename) const
{
	assertTriedToSolve();
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(false && "Not implemented.");
	}
	else {
		OsiStochasticParallelNestedBendersSolverInterface::printSolutionToFile(filename);
	}
}

void OsiStochasticBranchAndBendersCutSolverInterface::setParameters(const parallel_benders::Parameter& parameters)
{
	OsiStochasticParallelNestedBendersSolverInterface::setParameters(parameters);
	bbc_->parameter_ = parameters;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getSolutionCode() const
{
	assertTriedToSolve();
	double value = 0.0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		value = bbc_->solution_code_; // TODO: updated solution code in Solve()
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getSolutionCode();
	}

	return value;
}

void OsiStochasticBranchAndBendersCutSolverInterface::setInstanceSpecsFile(const std::string& filename)
{
	OsiStochasticParallelNestedBendersSolverInterface::setInstanceSpecsFile(filename);
	assert(bbc_);
	bbc_->instance_specification_file_ = filename;
}

void OsiStochasticBranchAndBendersCutSolverInterface::setTestsetProblemInstanceNames(std::string testset, std::string problem, std::string instance)
{
	OsiStochasticParallelNestedBendersSolverInterface::setTestsetProblemInstanceNames(testset, problem, instance);
	
	assert(bbc_);
	bbc_->testset_name_ = getTestsetProblemInstanceNames()[0];
	bbc_->problem_name_ = getTestsetProblemInstanceNames()[1];
	bbc_->instance_name_ = getTestsetProblemInstanceNames()[2];
}

void OsiStochasticBranchAndBendersCutSolverInterface::setTestsetProblemInstanceNames(std::string path) {
	OsiStochasticParallelNestedBendersSolverInterface::setTestsetProblemInstanceNames(path);
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumContPhasePnbIterations() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		value = bbc_->getNumContPhasePnbIterations();
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumContPhasePnbIterations();
	}

	return value;
}

double OsiStochasticBranchAndBendersCutSolverInterface::getUB() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		value = bbc_->upper_bound_;
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getUB();
	}

	return value;
}
double OsiStochasticBranchAndBendersCutSolverInterface::getLB() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		value = bbc_->lower_bound_;
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getLB();
	}

	return value;
}

double OsiStochasticBranchAndBendersCutSolverInterface::getIntegerGap() const
{
	assertTriedToSolve();
	int value = 0.0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		parallel_benders::ParallelNestedBenders* cont_phase_pnb = bbc_->cont_phase_pnb_;

		assert(bbc_->cont_phase_it_limit_ == 0 && "We cannot take the solution of cont_phase_pnb_ in this case");
		if (bbc_->integer_problems_.none()) {
			return 0.0;
		}
		else if (true) {
			assert(false);
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

const parallel_benders::Timer& OsiStochasticBranchAndBendersCutSolverInterface::getTimer() const
{
	assertTriedToSolve();
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		return bbc_->timer_;
	}
	else {
		return OsiStochasticParallelNestedBendersSolverInterface::getTimer();
	}
}

const parallel_benders::Timer& OsiStochasticBranchAndBendersCutSolverInterface::getBuildTimer() const
{
	assertTriedToSolve();
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		return bbc_->detailed_timer_;
	}
	else {
		return OsiStochasticParallelNestedBendersSolverInterface::getBuildTimer();
	}
}

const parallel_benders::Timer& OsiStochasticBranchAndBendersCutSolverInterface::getComputingTimer() const
{
	assertTriedToSolve();
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		return bbc_->solution_times_;
	}
	else {
		return OsiStochasticParallelNestedBendersSolverInterface::getComputingTimer();
	}
}

const parallel_benders::Timer& OsiStochasticBranchAndBendersCutSolverInterface::getLiftAndProjectCutsTimer() const
{
	assertTriedToSolve();
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		return bbc_->lp_cuts_timer_;
	}
	else {
		return OsiStochasticParallelNestedBendersSolverInterface::getLiftAndProjectCutsTimer();
	}
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumOptimalityCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->added_optimality_cuts_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumOptimalityCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumContinuousOptimalityCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		value = bbc_->getNumContinuousOptimalityCuts();
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumContinuousOptimalityCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumFeasibilityCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->added_feasibility_cuts_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumFeasibilityCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumEpigraphCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->added_epigraph_cuts_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumEpigraphCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumStrengthenedEpigraphCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->added_str_epigraph_cuts_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumStrengthenedEpigraphCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumPlainEpigraphCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->added_plain_epigraph_cuts_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumPlainEpigraphCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumDomainCuts() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->added_domain_cuts_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getNumDomainCuts();
	}

	return value;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getLpCgpCounter() const
{
	assertTriedToSolve();
	int value = 0;
	if (last_used_algorithm_ == parallel_benders::PNB_INT_ALGORITHM_BRANCH_AND_BENDERS_CUT) {
		assert(bbc_);
		if (!bbc_->stage_data_.empty())
			for (int i = 0; i < bbc_->num_stages_; i++) {
				value += bbc_->stage_data_[i]->lp_cgp_counter_;
			}
	}
	else {
		value = OsiStochasticParallelNestedBendersSolverInterface::getLpCgpCounter();
	}

	return value;
}


branch_and_benders_cut::BranchAndBendersCut* OsiStochasticBranchAndBendersCutSolverInterface::getBBC() const
{
	assert(bbc_);
	return bbc_;
}

const double* OsiStochasticBranchAndBendersCutSolverInterface::getSolutionByNodeAndStage(int node_number, int stage) const
{
	assert(false);
	//assert(nested_benders_);
	//if (getParameters().parameter_.get(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS) == parallel_benders::PNB_INT_ALGORITHM_DETEQ) {
	//	if (node_number > 0 || stage > 0)
	//		assert(false); // Not yet done
	//	return osi_->getColSolution();
	//}
	//else {
	//	assert(stage < nested_benders_->num_stages_);
	//	assert(node_number < nested_benders_->tree_nodes_by_stage_[stage].size());
	//	if (nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->incumbent_col_solution_.empty())
	//		return nullptr;
	//	else
	//		return &nested_benders_->tree_nodes_by_stage_[stage][node_number]->getDataPtr()->incumbent_col_solution_[0];
	//}
	return 0;
}

void OsiStochasticBranchAndBendersCutSolverInterface::assertTriedToSolve() const
{
	if (last_used_algorithm_ == -1) {
		LOG(FATAL) << "Never called initialSolve() for this interface.";
	}
}

void OsiStochasticBranchAndBendersCutSolverInterface::fixFirstStageVariables(const double* solution)
{
	assert(false);
	nested_benders_->fixFirstStageVariables(solution);
}

double OsiStochasticBranchAndBendersCutSolverInterface::getSolutionValueForScenario(int scenario)
{
	assert(false);
	//assert(scenario < nested_benders_->num_scenarios_);
	//return nested_benders_->computeScenarioValue(scenario);
	return 0.0;
}

void OsiStochasticBranchAndBendersCutSolverInterface::giveStartSolution(const double* solution, int num_cols)
{
	assert(false);
	assert(nested_benders_);
	// Store a warm_up solution
	// If cplex, we may need to think about it?
	nested_benders_->setWarmUpSolution(solution, num_cols);
}


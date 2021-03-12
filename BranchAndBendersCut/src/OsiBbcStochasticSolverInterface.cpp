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
	assert(false && "Never tested");
	//TODO: default algorithm should be bbc
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
	assert(false && "Never tested");
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
	assert(false);
	//if (nested_benders_->compute_primal_solutions_for_stage_ < 0)
	//	return;
	//if (nested_benders_->solution_code_ != 0) {
	//	LOG(ERROR) << "Solution can not be printed, because algorithm did not ended with optimality.";
	//	return;
	//}
	//// We want to print the solution we got by solving the model, to the file! Therefore we need a solved problem first!
	//std::ofstream solution_file(filename, fstream::out | fstream::trunc); // Open the file, regardless if it exists or not!
	//LOG_IF(ERROR, !solution_file.is_open()) << "Solution file " << filename << " could not be opened to write.";
	//// Get the values for which we actually want to store the solution from the param file
	//assert(nested_benders_->compute_primal_solutions_for_stage_ >= 0);
	//assert(nested_benders_->compute_primal_solutions_for_stage_ <= nested_benders_->num_stages_);
	//// At first, print a summary of the solution value
	//solution_file << "Objective Value : " << nested_benders_->upper_bound_ << "\n";
	//for (int i = 0; i < nested_benders_->num_stages_; i++) {
	//	// Sum over all nodes at this stage!
	//	double obj_value = 0.0;
	//	for (int scen = 0; scen < nested_benders_->tree_nodes_by_stage_[i].size(); scen++) {
	//		obj_value += nested_benders_->tree_nodes_by_stage_[i][scen]->getDataPtr()->incumbent_solution_value_;
	//	}
	//	solution_file << "Objective Value Stage " << i << " : " << obj_value << "\n";
	//}
	//solution_file << "Column Values sorted by Stage and Scenario" << "\n";
	//for (int stage = 0; stage < nested_benders_->compute_primal_solutions_for_stage_; stage++) {
	//	for (int scen = 0; scen < nested_benders_->tree_nodes_by_stage_[stage].size(); scen++) {
	//		solution_file << "Objective Value Stage-Scenario=" << stage << "-" << scen << " : " << nested_benders_->tree_nodes_by_stage_[stage][scen]->getDataPtr()->incumbent_solution_value_ << "\n";
	//		const double* primal_solution = getSolutionByNodeAndStage(scen, stage);
	//		assert(primal_solution != nullptr);
	//		// Write out objective value for stage and scenario wise (probability discounted)
	//		// Get the correct name for the variable in question via smi!
	//		int column_offset = smi_model_->getCore()->getColStart(stage);
	//		for (int current_column = 0; current_column < nested_benders_->stage_data_[stage]->num_columns_; current_column++) {
	//			if (abs(primal_solution[current_column]) >= nested_benders_->zero_tolerance_)
	//				solution_file << "Stage-Scenario=" << stage << "-" << scen << " : " << smi_model_->getColumnName(current_column + column_offset) << " : " << primal_solution[current_column] << "\n";
	//		}

	//	}
	//}
	//solution_file.close();
}

void OsiStochasticBranchAndBendersCutSolverInterface::setParameters(const parallel_benders::Parameter& parameters)
{
	OsiStochasticParallelNestedBendersSolverInterface::setParameters(parameters);
	bbc_->parameter_ = parameters;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getSolutionCode() const
{
	assert(false && "Never tested");
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
	assert(false && "Never tested");
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
	assert(false);
	//return nested_benders_->upper_bound_;
	return 0.0;
}
double OsiStochasticBranchAndBendersCutSolverInterface::getLB() const
{
	assert(false);
	//return nested_benders_->lower_bound_;
	return 0.0;
}

double OsiStochasticBranchAndBendersCutSolverInterface::getRelativeGap() const
{
	assert(false);
	//return parallel_benders::ComputeRelativeGap(nested_benders_->upper_bound_, nested_benders_->lower_bound_, 1e-10);
	return 0.0;
}

double OsiStochasticBranchAndBendersCutSolverInterface::getIntegerGap() const
{
	assert(false);
	//parallel_benders::ParallelNestedBenders* cont_phase_pnb = nested_benders_->cont_phase_pnb_;

	//assert(nested_benders_->cont_phase_it_limit_ == 0 && "We cannot take the solution of cont_phase_pnb_ in this case");
	//if (nested_benders_->integer_problems_.none()) {
	//	return 0.0;
	//}
	//else if (cont_phase_pnb && cont_phase_pnb->solution_code_ == parallel_benders::PNB_SOLUTION_OPTIMAL && nested_benders_->solved_counter_ > 0 && nested_benders_->solution_code_ == parallel_benders::PNB_SOLUTION_OPTIMAL) {
	//	assert(cont_phase_pnb->solved_counter_ == 1); // should be solved at least but not more than once
		// if we used cont phase, we could take UB from the first iteration, since it will be the relaxation solution
	//	return parallel_benders::ComputeRelativeGap(nested_benders_->upper_bound_, cont_phase_pnb->upper_bound_, 1e-10);
	//}
	//else {
	//	return -1.0;
	//}
	return 0.0;
}

const parallel_benders::Timer& OsiStochasticBranchAndBendersCutSolverInterface::getTimer() const
{
	assert(false && "Never tested");
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
	assert(false && "Never tested");
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
	assert(false && "Never tested");
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
	assert(false && "Never tested");
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
	assert(false);
	assert(nested_benders_);
	int cut_counter = 0;
	//if (!nested_benders_->stage_data_.empty())
	//	for (int i = 0; i < nested_benders_->num_stages_; i++) {
	//		cut_counter += nested_benders_->stage_data_[i]->added_optimality_cuts_;
	//	}
	return cut_counter;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumContinuousOptimalityCuts() const
{
	assert(false);
	return 0;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumFeasibilityCuts() const
{
	assert(false);
	return 0;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumEpigraphCuts() const
{
	assert(false);
	return 0;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumStrengthenedEpigraphCuts() const
{
	assert(false);
	return 0;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumPlainEpigraphCuts() const
{
	assert(false);
	return 0;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getNumDomainCuts() const
{
	assert(false);
	return 0;
}

int OsiStochasticBranchAndBendersCutSolverInterface::getLpCgpCounter() const
{
	assert(false);
	return 0;
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

int OsiStochasticBranchAndBendersCutSolverInterface::getNumInfeasibilityIterations() const
{
	assert(false);
	assert(nested_benders_);
	//return getNumIterations() - nested_benders_->feasible_iteration_; // Feasible Iterations
	return 0;
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


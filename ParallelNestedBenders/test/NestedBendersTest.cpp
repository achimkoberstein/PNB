#define BOOST_TR1_TUPLE_HPP_INCLUDED //This prevents compilation issues.
//#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <limits>
#include <cmath>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <array>

#define GOOGLE_GLOG_DLL_DECL // Has to be done to correct linking issues
#include <glog/logging.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/thread.hpp>

#include "OsiCpxSolverInterface.hpp"
//#include "OsiGrbSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"
#include "CoinTime.hpp"
#include "SmiScnModel.hpp"
#include "flopc.hpp"

#include "BendersNodeData.hpp"
#include "ParallelNestedBenders.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
//#include "OsiBenStochasticSolverInterface.hpp"

//#include "fortsp.h"
#include "ilcplex/cplex.h"
#include "ilcplex/cpxconst.h"

#include "OsiRowCut.hpp"

class TimerTest : public ::testing::Test {
public:
	typedef std::chrono::high_resolution_clock wall_clock;

	std::atomic<int> wall_thread_timer; // milliseconds
	parallel_benders::Timer pnb_thread_timer;

	void infiniteSumApprox(std::mutex& cout_mutex, int repetition, int log_every_it) {
		// Start timers
		auto begin_wall = wall_clock::now();
		double begin_pnb = pnb_thread_timer.Now();
		
		double sum = 0;
		double add = 1;

		int iterations = 1000000;
		for (int i = 0; i < iterations; i++) {
			sum += add;
			add /= 2.0;
			
			if (i % log_every_it == 0) {
				//std::lock_guard<std::mutex> unique_lock(cout_mutex);
				std::cout.setstate(std::ios_base::failbit);
				DLOG(INFO) << "Thread: " << boost::this_thread::get_id() << " Repetition: " << repetition << " Iteration: " << i << std::endl;
				//std::cout << "Thread: " << boost::this_thread::get_id() << " Repetition: " << repetition << " Iteration: " << i << std::endl;
			}
		}
		//this_thread::sleep_for(chrono::seconds(5)); // for testing CPU time
		int duration = std::chrono::duration_cast<std::chrono::milliseconds>(wall_clock::now() - begin_wall).count();
		wall_thread_timer += duration;
		pnb_thread_timer.AddTime(0, pnb_thread_timer.Now() - begin_pnb);
	}

	void sumApproxTimes(int repetitions, int num_cpu, int log_every_it, bool logging) {
		setLogging(logging);
		boost::threadpool::pool thread_pool = boost::threadpool::pool(num_cpu);
		std::mutex cout_mutex;
		for (int i = 0; i < repetitions; i++) {
			if (num_cpu > 1) { // schedule
				thread_pool.schedule(boost::bind(&TimerTest::infiniteSumApprox, this, boost::ref(cout_mutex), i, log_every_it));
			}
			else { // use the current thread
				infiniteSumApprox(cout_mutex, i, log_every_it);
			}
		}
		thread_pool.wait(0);

		setLogging(true);
		std::cout << "Finished sumApproxTimes with parameters Reps: " << repetitions << " Num CPU: " << num_cpu << " Log every It: " << log_every_it << std::endl;
	}

	void setLogging(bool logging) {
		// copy of void ParallelNestedBenders::SetLogLevel(int logging_level)

		if (!logging) { // block console output if logging leve > INFO
			std::cout.clear();
			std::cout.setstate(std::ios_base::failbit);
		}
		else {
			std::cout.clear(); // else unblock it
		}
	}
protected:
	TimerTest() : 
		pnb_thread_timer(10)
	{
		wall_thread_timer = 0;
	}
};

class UnitTestCutStuff : public ::testing::Test {
public:
	parallel_benders::CutStuff* CutStuffFromCutInfo(const std::vector<double>& cut_info) const {
		//Example of cut_info
		//					  col,	coef,		lb,	theta,	it,type,col set
		//cut_infos.push_back({ 4,	1,1,1,1,	10,	1,0,0,	4,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });

		//Position of info in cut_info
		const int col_ind(0); // number of columns incl. ones with coef 0
		int num_cols = cut_info[col_ind];
		int num_theta = cut_info.size() - num_cols - 5;

		const int coef_start_ind(col_ind + 1);
		const int lb_ind(coef_start_ind + num_cols);
		const int theta_start_ind(lb_ind + 1);
		const int it_ind(theta_start_ind + num_theta);
		const int type_ind(it_ind + 1);
		const int col_set_ind(type_ind + 1);

		CoinPackedVector row = CoinPackedVector();

		for (int i = coef_start_ind; i < coef_start_ind + num_cols; i++) {
			if (!parallel_benders::CompareForEquality(cut_info[i], 0.0, 0.00001)) {
				row.insert(i - 1, cut_info[i]);
			}
		}
		boost::dynamic_bitset<> aggregates(num_theta);
		for (int i = theta_start_ind; i < theta_start_ind + num_theta; i++) {
			if (cut_info[i] == 1.0) {
				row.insert(num_cols + i - theta_start_ind, 1.0);
			}
		}

		OsiRowCut* cut = new OsiRowCut();
		cut->setLb(cut_info[lb_ind]);
		cut->setUb(INFINITY);
		cut->setRow(row);
		std::unique_ptr<OsiRowCut> cut_pointer(cut);

		int type = cut_info[type_ind];
		int num_theta_for_cut = num_theta;
		if (type == parallel_benders::PNB_CUT_TYPE_FEASIBILITY || type == parallel_benders::PNB_CUT_TYPE_DOMAIN) {
			num_theta_for_cut = 0;
		}
		parallel_benders::CutStuff* cut_stuff = new parallel_benders::CutStuff(cut_info[it_ind], -2, cut_pointer, type, num_cols, num_theta_for_cut);
		cut_stuff->column_set_ = cut_info[col_set_ind];

		return cut_stuff;
	}	

protected:
	UnitTestCutStuff()
	{
	}
};


class NestedBendersTest : public ::testing::Test { // this is the Test set, e.g.,  TEST(NestedBendersTest, test_case). Second argument is the test (also called test case) 
public:

	parallel_benders::Parameter benders_parameter_; // Used to store parameters
	std::vector<parallel_benders::Parameter> parameter_sets_; // Used to store different combinations (Parameter instances) of parameters
	std::string instance_specification_file_; // for characteristics of the instance such as number vars, constraints, stages etc.
	std::string testbed_aggregated_log_file_string_; // average of repetitions
	std::string testbed_log_file_string_; // for every repetition
	std::string parameter_name_string_;
	static const int max_stages_ = 11; // This can be set differently
	bool use_multi_cut_;

	bool compute_statistics_;
	bool generate_initial_solution_;

	double solveSMPS(const char* name) {
		return solveSMPS(name, name, name);
	}
	double solveSMPS(const char* core_time_name, const char* stoch_name) {
		return solveSMPS(core_time_name, core_time_name, stoch_name);
	}

	void solveForProtocolsWithStartSolutions(std::vector<double>& solutions, const char* core_name, const char* time_name, const char* stoch_name) {
		int algorithm = benders_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS);
		// We can not use the no start solution option, when we have an algorithm that needs a start solution!
		if (generate_initial_solution_ && !(algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_ODA || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA || algorithm == parallel_benders::PNB_INT_ALGORITHM_REGULARIZATION || algorithm == parallel_benders::PNB_INT_ALGORITHM_REGULARIZATION_ODA || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA)) {
			benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION, parallel_benders::PNB_INT_NO_START_SOLUTION);
			solveForProtocols(solutions, num_stages_, core_name, time_name, stoch_name);
		}

		benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION, parallel_benders::PNB_INT_USE_EV_SOLUTION);
		benders_parameter_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, num_stages_ - 2); // Compute the first-stage solution! We need it for the subsequent solution run.
		solveForProtocols(solutions, num_stages_, core_name, time_name, stoch_name);
		if (generate_initial_solution_) {
			// Copy optimal solution value (of first scenario) to current_start_solution array
			current_start_solution_.reset(new double[num_single_scenario_cols_]);
			CoinFillN(current_start_solution_.get(), num_single_scenario_cols_, 0.0);
			std::copy(first_stage_solution_.get(), first_stage_solution_.get() + num_single_scenario_cols_, current_start_solution_.get());


			benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION, parallel_benders::PNB_INT_USE_SUPPLIED_SOLUTION);
			benders_parameter_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO);
			// Use the optimal solution!
			solveForProtocols(solutions, num_stages_, core_name, time_name, stoch_name);

			if (special_case_solution_length_ == num_single_scenario_cols_) {
				// Worst-case solution
				std::copy(worst_case_solution_.get(), worst_case_solution_.get() + num_single_scenario_cols_, current_start_solution_.get());
				solveForProtocols(solutions, num_stages_, core_name, time_name, stoch_name);

				// Best case
				std::copy(best_case_solution_.get(), best_case_solution_.get() + num_single_scenario_cols_, current_start_solution_.get());
				solveForProtocols(solutions, num_stages_, core_name, time_name, stoch_name);
			}
		}
	}

	void solveForProtocolsWithLambda(std::vector<double>& solutions, const char* core_name, const char* time_name, const char* stoch_name) {
		// Vary Lambda from 0.9 to 0.1
		//benders_parameter_.parameter_.put(parallel_benders::PNB_LEVEL_LAMBDA,0.9);
		//solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
		//benders_parameter_.parameter_.put(parallel_benders::PNB_LEVEL_LAMBDA,0.7);
		//solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
		benders_parameter_.parameter_.put(parallel_benders::PNB_LEVEL_LAMBDA, 0.5);
		solveForProtocolsWithStartSolutions(solutions, core_name, time_name, stoch_name);
		//benders_parameter_.parameter_.put(parallel_benders::PNB_LEVEL_LAMBDA,0.3);
		//solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
		//benders_parameter_.parameter_.put(parallel_benders::PNB_LEVEL_LAMBDA,0.1);
		//solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);

	}

	void solveForProtocolsWithKappa(std::vector<double>& solutions, const char* core_name, const char* time_name, const char* stoch_name) {
		int algorithm = benders_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, 1);
		bool use_lambda = false;
		if (algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA || algorithm == parallel_benders::PNB_INT_ALGORITHM_REGULARIZATION_ODA || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_ODA) {
			use_lambda = true;
		}
		else {
			benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_KAPPA, 0.5);
		}
		// Vary Kappa, call Lambda
		//benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_KAPPA,0.9);
		//use_lambda ? solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name) : solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
		//benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_KAPPA,0.7);
		//use_lambda ? solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name) : solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
		benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_KAPPA, 0.5);
		use_lambda ? solveForProtocolsWithLambda(solutions, core_name, time_name, stoch_name) : solveForProtocolsWithStartSolutions(solutions, core_name, time_name, stoch_name);
		//benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_KAPPA,0.3);
		//use_lambda ? solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name) : solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
		//benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_KAPPA,0.1);
		//use_lambda ? solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name) : solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);

	}

	void solveForProtocols(std::vector<double>& solutions, int num_stages, const char* core_name, const char* time_name, const char* stoch_name) {
		int algorithm = benders_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_BENDERS);
		bool level = (algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_ODA ||
			algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA ||
			algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN || algorithm == parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA ||
			algorithm == parallel_benders::PNB_INT_ALGORITHM_REGULARIZATION || algorithm == parallel_benders::PNB_INT_ALGORITHM_REGULARIZATION_ODA);
		if (num_stages > 2 && !level) {
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 1e-6);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_FAST_BACKWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 0.064);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_EPSILON_FAST_BACKWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 0.1);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_EPSILON_FAST_BACKWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 0.01);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_EPSILON_FAST_BACKWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 1e-6);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_FAST_FORWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 0.064);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_EPSILON_FAST_FORWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 0.1);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_EPSILON_FAST_FORWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 0.01);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_EPSILON_FAST_FORWARD);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON, 1e-6);
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_FASTPASS);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));

			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_FASTPASS_WITH_FASTBACK);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));
		}
		else {
			benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_FASTPASS);
			solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));
		}
		benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY, parallel_benders::PNB_INT_FASTPASS_WITH_FASTBACK);
	}

	bool checkConvergence(const char* core_name, const char* time_name, const char* stoch_name, const std::vector<std::pair<double, double>>& ub_lb_list) {

		solveProblemWithParameters(core_name, time_name, stoch_name, NULL, false);
		bool result = true;

		if (ub_lb_list.size() != ub_for_iteration_.size() || ub_lb_list.size() != lb_for_iteration_.size()) {
			LOG(INFO) << "UB LB list length is " << ub_lb_list.size() << " != lenght of the list from PNB " << ub_for_iteration_.size();
			return false;
		}
		for (int i = 0; i < ub_lb_list.size(); i++) {
			// UB
			if (!parallel_benders::CompareForEquality(ub_lb_list[i].first, ub_for_iteration_[i], 0.1)) {
				result = false;
				LOG(INFO) << "Iteration " << i + 1 << " Target UB: " << ub_lb_list[i].first << " != UB: " << ub_for_iteration_[i];
			}

			// LB
			if (!parallel_benders::CompareForEquality(ub_lb_list[i].second, lb_for_iteration_[i], 0.1)) {
				result = false;
				LOG(INFO) << "Iteration " << i + 1 << " Target LB: " << ub_lb_list[i].second << " != LB: " << lb_for_iteration_[i];
			}
		}
		return result;
	}

	double checkConvergence(const char* name, const std::vector<std::pair<double, double>>& ub_lb_list) {
		return checkConvergence(name, name, name, ub_lb_list);
	}

	double solveSMPS(const char* core_name, const char* time_name, const char* stoch_name) {
		// Start with single cut and Benders
		std::vector<double> solutions;
		std::string folder("../../../../Data/Benders/");
#if _M_X64 || __x86_64__ // Works only on windows i suppose
		folder = "../../../../Data/Benders/";
#endif
		//SmiScnModel smi;
		std::string core_name_string(core_name);
		std::string time_name_string(time_name);
		std::string stoch_name_string(stoch_name);
		std::string total_core_string = folder + core_name_string;
		std::string total_time_string = folder + time_name_string;
		std::string total_stoch_string = folder + stoch_name_string;

		num_stages_ = 2;
		int num_scenarios(-1);
		int num_childs_first_stage = 1;

		parallel_benders::Parameter param(benders_parameter_); // Copy parameter!
		updateParameterSets();

		int read_in_smps_file = 1;
		int num_aggregates_by_magic = 0;
		double* min_scenario_pointer = NULL;
		double* max_scenario_pointer = NULL;
		compute_statistics_ = false;
		generate_initial_solution_ = false;
		bool generate_cvar_constraints = false;
		double cvar_objective = 0.0;
		double cvar_beta = benders_parameter_.parameter_.get(parallel_benders::PNB_CVAR_BETA, 0.66);
		double cvar_rho = benders_parameter_.parameter_.get(parallel_benders::PNB_CVAR_RHO, -80000);
		bool cvar_first_stage = benders_parameter_.parameter_.get(parallel_benders::PNB_CVAR_WITH_FIRST_STAGE, false);
		if (read_in_smps_file) {
			boost::scoped_ptr<SmiScnModel> smi_model(new SmiScnModel());
			int smi_read_in = smi_model->readSmps(total_core_string.c_str(), total_time_string.c_str(), total_stoch_string.c_str());
			assert(smi_read_in == 0);
			num_stages_ = smi_model->getCore()->getNumStages();
			num_scenarios = smi_model->getNumScenarios();
			num_childs_first_stage = smi_model->getSmiTree()->getRoot()->numChildren();

			num_aggregates_by_magic = 0;
			num_aggregates_by_magic = smi_model->getSmiTree()->getRoot()->numChildren() / 10;
			assert(num_aggregates_by_magic >= 0);

			num_single_scenario_cols_ = 0;
			for (int i = 0; i < num_stages_ - 1; i++)
				num_single_scenario_cols_ += smi_model->getCore()->getNumCols(i);

			special_case_solution_length_ = num_single_scenario_cols_;

			if (compute_statistics_ || generate_initial_solution_) {
				//Open File!
				int det_cols = smi_model->getCore()->getNumCols(0) + num_childs_first_stage * smi_model->getCore()->getNumCols(1);
				int det_rows = smi_model->getCore()->getNumRows(0) + num_childs_first_stage * smi_model->getCore()->getNumRows(1);;
				int det_nonzeroes = smi_model->getCore()->getNode(0)->getNumMatrixElements() + num_childs_first_stage * smi_model->getCore()->getNode(1)->getNumMatrixElements();

				if (smi_model->getCore()->getNumStages() > 2)
				{
					// Compute det_cols and det_rows by aggregating over the scenario tree?!
					det_cols = 0; det_rows = 0; det_nonzeroes = 0;
					std::vector<SmiScnNode*> tree_nodes = smi_model->getSmiTree()->wholeTree();
					BOOST_FOREACH(SmiScnNode * node, tree_nodes) {
						int node_stage = node->getStage();
						det_cols += smi_model->getCore()->getNumCols(node_stage);
						det_rows += smi_model->getCore()->getNumRows(node_stage);;
						det_nonzeroes += smi_model->getCore()->getNode(node_stage)->getNumMatrixElements();
					}

				}

				OsiCpxSolverInterface osi_cpx;
				double ws = 0;
				double eev = 0;
				double hn = 0;
				// Compute Wait-And-See
				std::vector<std::pair<double, double*> > ws_results = smi_model->solveWS(&osi_cpx);
				std::pair<int, double> minimum = std::make_pair(-1, 1e20);
				std::pair<int, double> maximum = std::make_pair(-1, -1e20);
				for (int i = 0; i < ws_results.size(); i++) {
					ws += ws_results[i].first * smi_model->getScenarioProb(i);
					// Look for best and worst scenario!
					if (ws_results[i].first > maximum.second) {
						maximum.first = i;
						maximum.second = ws_results[i].first;
						max_scenario_pointer = ws_results[i].second;
					}
					if (ws_results[i].first < minimum.second) {
						minimum.first = i;
						minimum.second = ws_results[i].first;
						min_scenario_pointer = ws_results[i].second;
					}
				}
				best_case_solution_.reset(new double[num_single_scenario_cols_]);
				worst_case_solution_.reset(new double[num_single_scenario_cols_]);
				std::copy(max_scenario_pointer, max_scenario_pointer + num_single_scenario_cols_, worst_case_solution_.get());
				std::copy(min_scenario_pointer, min_scenario_pointer + num_single_scenario_cols_, best_case_solution_.get());
				for (int i = 0; i < ws_results.size(); i++) {
					delete[] ws_results[i].second;
					ws_results[i].second = NULL;
				}
				smi_model->releaseSolver();
				if (compute_statistics_) {
					boost::shared_array<double> ev_solution(NULL);
					double eev2 = 0;
					{
						std::unique_ptr<SmiScnModel> smi_model_ev(new SmiScnModel());
						smi_read_in = smi_model_ev->readSmps(total_core_string.c_str(), total_time_string.c_str(), total_stoch_string.c_str());
						std::pair<double, double*> results = smi_model_ev->solveEV(&osi_cpx);
						ev_solution.reset(results.second);
						OsiStochasticParallelNestedBendersSolverInterface osi_pnb(&osi_cpx, smi_model.get());
						osi_pnb.fixFirstStageVariables(ev_solution.get());
						osi_pnb.initialSolve();
						osi_pnb.printProblemCharacteristics();
						eev2 = osi_pnb.getObjValue();
					}

					//// Now we need the HN-solution
					{
						OsiStochasticParallelNestedBendersSolverInterface osi_pnb(&osi_cpx, smi_model.get());
						parallel_benders::Parameter param;
						param.init();
						param.parameter_.put(parallel_benders::PNB_BENDERS, parallel_benders::PNB_INT_ALGORITHM_LEVEL);
						osi_pnb.setParameters(param);
						osi_pnb.initialSolve();
						hn = osi_pnb.getObjValue();
					}

					// Compute DETEQ column, row and nonzeroes sizes..
					std::cout << "Problem Name; #Scen; Cols; Rows; Cols 2nd; Rows 2nd; Cols DET; Rows DET; NZ DET; Wait-and-See; Here-and-Now; EEV; EVPI; VSS";
					std::ofstream file_out;
					file_out.open("sizes.csv", ios::app);
					file_out << stoch_name << ";" << num_childs_first_stage << ";" << smi_model->getCore()->getNumCols(0) << ";" << smi_model->getCore()->getNumRows(0) << ";" << smi_model->getCore()->getNumCols(1)
						<< ";" << smi_model->getCore()->getNumRows(1) << ";" << det_cols << ";" << det_rows << ";" << det_nonzeroes << ";" <<
						ws << ";" << hn << ";" << eev2 << ";" << hn - ws << ";" << eev2 - hn << "\n";
					file_out.close();
					LOG(INFO) << "printed to file";
				}
			}

		}

		double cvar_lower = -1e20;
		double cvar_upper = 1e20;
		if (generate_cvar_constraints) {
			// Get CVaR from optimal solution!
			{
				//boost::scoped_ptr<SmiScnModel> smi_cvar_model(new SmiScnModel());
				//int smi_read_in = smi_cvar_model->readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
				//assert( smi_read_in == 0);
				//OsiCpxSolverInterface osi_cpx;
				//std::vector<double> eev_scenario_evaluation = smi_cvar_model->solveEEVByScenario(&osi_cpx);
				//// Create vector with probability/value pairs
				//std::vector<std::pair<double,double> > scenario_value_probability_vector(smi_cvar_model->getNumScenarios());
				//for (int i = 0; i < scenario_value_probability_vector.size(); i++){
				//    scenario_value_probability_vector[i] = make_pair(eev_scenario_evaluation[i],smi_cvar_model->getScenarioProb(i));
				//}

				boost::scoped_ptr<SmiScnModel> smi_cvar_model(new SmiScnModel());
				int smi_read_in = smi_cvar_model->readSmps(total_core_string.c_str(), total_time_string.c_str(), total_stoch_string.c_str());
				assert(smi_read_in == 0);
				OsiCpxSolverInterface osi_cpx;
				smi_cvar_model->setOsiSolverHandle(&osi_cpx);
				OsiSolverInterface* osi_solver = smi_cvar_model->loadOsiSolverData();
				smi_cvar_model->releaseSolver();
				//osi_solver->writeLp("hn_model");
				osi_solver->initialSolve();
				if (osi_solver->isProvenOptimal()) {
					const double* first_stage_solution = osi_solver->getColSolution();
					std::vector<double> scenario_evaluation = smi_cvar_model->evaluateWithFirstStageSolutionByScenario(&osi_cpx, first_stage_solution);
					// Create vector with probability/value pairs
					std::vector<std::pair<double, double> > scenario_value_probability_vector(smi_cvar_model->getNumScenarios());
					for (int i = 0; i < scenario_value_probability_vector.size(); i++) {
						scenario_value_probability_vector[i] = make_pair(scenario_evaluation[i], smi_cvar_model->getScenarioProb(i));
					}

					// Sort vector according to first element!
					std::sort(scenario_value_probability_vector.begin(), scenario_value_probability_vector.end(),
						[](const std::pair<double, double>& a, const std::pair<double, double>& b) -> bool
						{
							return a.first > b.first;
						});
					// Compute CVaR
					double summed_prob = cvar_beta;
					double cvar = 0.0;
					int index_i = 0;
					while (summed_prob > 0) {
						if (scenario_value_probability_vector[index_i].second > summed_prob) {
							cvar += scenario_value_probability_vector[index_i].first * summed_prob;
						}
						else {
							cvar += scenario_value_probability_vector[index_i].first * scenario_value_probability_vector[index_i].second;
						}
						summed_prob -= scenario_value_probability_vector[index_i].second;
					}
					cvar /= cvar_beta;
					if (cvar_first_stage) { // If we have first-stage cvar, we need to add the first stage cost to it.
						cvar += scenario_evaluation[scenario_evaluation.size() - 1];
					}

					// Set cvar_rho by 0.8*cvar for eev solution
					cvar_upper = cvar;
				}
				else
					LOG(FATAL) << "HN problem can not be solved with det eq solver.";

			}
			// Get CVaR from minimizing CVaR
			{
				// CVaR deteq model for comparison
				boost::scoped_ptr<SmiScnModel> smi_cvar_model(new SmiScnModel());
				int smi_read_in = smi_cvar_model->readSmps(total_core_string.c_str(), total_time_string.c_str(), total_stoch_string.c_str());
				assert(smi_read_in == 0);
				smi_cvar_model->setCVaRConstraint(true, cvar_beta, cvar_rho, cvar_first_stage, true);
				OsiCpxSolverInterface osi_cpx;
				// Get Environment and LP Solver 

				smi_cvar_model->setOsiSolverHandle(&osi_cpx);
				OsiSolverInterface* deteq_solver = smi_cvar_model->loadOsiSolverData();

				// Cast to OsiCpxSolverInterface
				OsiCpxSolverInterface* cpx_solver = dynamic_cast<OsiCpxSolverInterface*>(deteq_solver);
				if (cpx_solver != nullptr) {// Disable aggregator, it consumes too much memory!
					CPXENVptr env_ptr = cpx_solver->getEnvironmentPtr();
					CPXsetintparam(env_ptr, CPX_PARAM_AGGIND, 0);
				}
				// Parse solver interface!
				//deteq_solver->writeLp("cvar_model");
				deteq_solver->initialSolve();
				// Out of Memory error
				if (deteq_solver->isProvenOptimal()) {
					cvar_lower = deteq_solver->getObjValue();
				}
				else
					LOG(FATAL) << "CVaR minimization problem could not be solved to optimality.";
				smi_cvar_model->releaseSolver();
			}
			if (cvar_upper - cvar_lower > -1e-08) {
				double linear_combination_factor = 0.5;
				double cvar_rho = cvar_lower + linear_combination_factor * (cvar_upper - cvar_lower);
				// Set cvar_rho so that it can be used later in the algorithm!
				benders_parameter_.parameter_.put(parallel_benders::PNB_CVAR_RHO, cvar_rho);
			}
			else
				LOG(FATAL) << " We were not able to determine a correct CVaR.";



		}

		std::vector<int> aggregates;
		aggregates.push_back(1);
		//aggregates.push_back(2);
		// Just to ease result processing
  //      if ( num_childs_first_stage <= 5 )
  //          aggregates.push_back(1);
		//aggregates.push_back(5);
		//aggregates.push_back(10);
		//aggregates.push_back(20);
		//////aggregates.push_back(30);
		//////aggregates.push_back(40);
		//aggregates.push_back(50);
		//aggregates.push_back(100);
		//if ( use_multi_cut_ )
			//aggregates.push_back(0);



		 //If we have not yet added num_aggregates_by_magic, we add it now
		//if ( std::find(aggregates.begin(),aggregates.end(),num_aggregates_by_magic) == aggregates.end() ) {
  //          num_aggregates_by_magic = std::max(num_aggregates_by_magic,1);
		//	if ( num_aggregates_by_magic > 200 ) // Add an upper cap on the number of aggregates!
  //          num_aggregates_by_magic = 200;
		//	aggregates.push_back(num_aggregates_by_magic);
		//}

		for (auto const& parameter_set : parameter_sets_) {
			benders_parameter_ = parameter_set;
			int aggregates = benders_parameter_.parameter_.get(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 1);
			// we skip if number of aggregates > number of scenarios
			LOG_IF(INFO, num_scenarios < aggregates && num_stages_ == 2) << "Solving of " << stoch_name << " with PNB_AGGREGATED_OPTIMALITY_CUTS " << aggregates << " was skipped. Num 2nd stage scenarios < aggregates. Maybe you did not activate read_in_smps_file to set num_scenarios.";
			if (!(num_childs_first_stage < aggregates && num_stages_ == 2)) {
				write_xml("current_param.xml", benders_parameter_.parameter_);
				solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));
				LOG(INFO) << "";
			}
		}


		// Compare solution values



  //      bool test_cut_consolidation = false;
  //      bool test_level_euclidean = false;
  //      bool test_level_infinity = false;
  //      bool test_level_manhattan = false;
  //      bool test_regularized_decomposition = false;
  //      bool test_normal = true;
  //      bool test_oda = false;
		//bool test_only_oda = false;
  //      bool test_start_solutions = generate_initial_solution_;
  //      bool test_feas_cuts = false;
  //      bool test_dem = false;

  //      for (int cpu = 0; cpu < 0; cpu++){
  //              //// Solve DetEq as comparison

		//		// Copy benders_parameter
  //              //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //              for (int i = 0; i < aggregates.size(); i++){
  //                  if ( aggregates[i] >= num_childs_first_stage && aggregates[i] > 1){
  //                      LOG(INFO) << "Skipping test, because " << aggregates[i] << " is bigger than childs of first stage";
  //                      continue;
  //                  }
  //                  benders_parameter_ = copied_param_method;

  //                  benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
  //                  benders_parameter_.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,aggregates[i]);
		//			benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,cpu);
  //                  parallel_benders::Parameter copied_param_for_loop(benders_parameter_);
		//			
		//			if ( test_dem ){
  //                      benders_parameter_ = copied_param_for_loop;
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_DETEQ);
  //                      // We can choose different LP methods, but stick with the dual simplex for now!
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_LP_METHOD,parallel_benders::PNB_INT_DUAL_SIMPLEX);
  //                      solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_LP_METHOD,parallel_benders::PNB_INT_PRIMAL_SIMPLEX);
  //                      solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_LP_METHOD,parallel_benders::PNB_INT_BARRIER);
  //                      solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //                  }

  //                  if ( test_start_solutions && test_normal){
  //                      benders_parameter_ = copied_param_for_loop;
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
  //                      solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
  //                  }       

  //                  if ( test_feas_cuts ){ 
  //                      benders_parameter_ = copied_param_for_loop;
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_HANDLE_FEAS_CUTS,parallel_benders::PNB_INT_FEAS_CUT_ALL);
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_EV_SOLUTION);
  //                      solveForProtocols(solutions,num_stages_,core_name,time_name,stoch_name);
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_HANDLE_FEAS_CUTS,parallel_benders::PNB_INT_FEAS_CUT_WITH_OPT);
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_EV_SOLUTION);
  //                      solveForProtocols(solutions,num_stages_,core_name,time_name,stoch_name);
  //                  }

  //                 // We only need aggregation/consolidation for
  //                  if ( test_cut_consolidation ){
  //                      benders_parameter_ = copied_param_for_loop;

  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION,true);
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_CUT_COMPUTATION,true);
  //                      // We need differing parameters here!
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD,5);
  //                      benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_PARTIAL_THRESHOLD,0.01);// When less than 1% of the cuts of an iteration are active, we consolidate!

  //                      for ( int strike_threshold = 1; strike_threshold < 6; strike_threshold++){
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD,strike_threshold);
  //                          solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);

  //                          ////// Solve problem without added cut!
  //                          //benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_CUT_COMPUTATION,false);
  //                          //solveForProtocolsWithStartSolutions(solutions,core_name,time_name,stoch_name);
  //                      }
  //                  }
		//			if ( !test_only_oda ){
		//				if ( test_normal && !test_start_solutions){
		//					benders_parameter_ = copied_param_for_loop;
		//					benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
		//					solveForProtocols(solutions,num_stages_,core_name,time_name,stoch_name);
		//				}

		//				if ( test_level_euclidean ){
  //                          benders_parameter_ = copied_param_for_loop;
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL);
  //                          solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name);
  //                          
		//				}
		//				if ( test_level_manhattan ){
		//					benders_parameter_ = copied_param_for_loop;
		//					benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN);
		//					solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name);
		//				}
		//				if ( test_level_infinity ){
		//					benders_parameter_ = copied_param_for_loop;
		//					benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY);
		//					solveForProtocolsWithLambda(solutions,core_name,time_name,stoch_name);
		//				}
		//			}

  //                  if ( test_oda ){
		//				if ( test_normal ){
		//					benders_parameter_ = copied_param_for_loop;
		//					benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS_ODA);
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,0); // Choose cuts as in number of aggregates
  //                          solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name);
  //                          //benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,1);
  //                          //solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name); // Only use a single oda cut
		//				}

  //                      if ( test_level_euclidean ){
  //                          benders_parameter_ = copied_param_for_loop;
  //                              benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL_ODA);
  //                              benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,0); // Choose cuts as in number of aggregates
  //                              solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name);
  //                              // 
  //                              //benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,1);
  //                              //solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name);
  //                      }
  //                      if ( test_level_manhattan ){
  //                          benders_parameter_ = copied_param_for_loop;
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA);
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,0); // Choose cuts as in number of aggregates
  //                          solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name);
  //                          //benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,1);
  //                          //solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name);
  //                      }
  //                      if ( test_level_infinity ){
  //                          benders_parameter_ = copied_param_for_loop;
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA);
  //                          benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,0); // Choose cuts as in number of aggregates
  //                          solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name);
  //                          //benders_parameter_.parameter_.put(parallel_benders::PNB_ODA_AGGREGATES,1);
  //                          //solveForProtocolsWithKappa(solutions,core_name,time_name,stoch_name); // Only use a single oda cut
  //                      }
  //                  }
		//		}//end for: aggregates
  //      }//end for:cpu



		// Reset benders parameter setting!
		benders_parameter_ = param;
		double average = 0;
		for (int i = 0; i < solutions.size(); i++) {
			LOG(INFO) << "Solution " << i << ": " << solutions[i];
			average += solutions[i];
		}
		if (!solutions.empty()) {
			average = average / solutions.size();
		}
		return average;

	}
	// Solve the problems from the filename.csv one by one using solveSMPS()
	// every value in the file is the path to file: cor_tim_sto OR cor_tim;sto OR cor;tim;sto
	// // - is a comment and should start the line
	// case non sensitive
	// paths in format folder/subfolder/sto_file_name

	void solveSMPSFromList(const char* filename) {
		std::string line;
		std::ifstream list;
		std::string folder("../../../../Data/Benders/");
		list.open(filename);

		LOG_IF(FATAL, !list.is_open()) << "Could not open " << filename;

		// Prepare the list of cor,tim,sto files
		std::vector<std::string> cor_files;
		std::vector<std::string> tim_files;
		std::vector<std::string> sto_files;
		while (getline(list, line)) { // cor;tim;sto OR cor_tim;sto OR cor_tim_sto
			// Split path to testset, problem, instances
			std::vector<std::string> splitted_line;
			if (line.substr(0, 2) != "//") { // because double slash is a comment
				std::size_t current(0);
				std::size_t previous(0);
				current = line.find(";");
				for (int i = 0; i < 3; i++) {
					if (current == std::string::npos) {
						splitted_line.push_back(line.substr(previous, line.length() - previous));
						break;
					}
					splitted_line.push_back(line.substr(previous, current - previous));
					previous = current + 1;
					current = line.find(";", previous);
				}
			}

			//transform to lower
			for (auto& s : splitted_line) {
				std::transform(s.begin(), s.end(), s.begin(),
					[](unsigned char c) { return std::tolower(c); }
				);
			}
			std::string cor;
			std::string tim;
			std::string sto;
			if (splitted_line.size() == 0) { // the line was a comment
				assert(line.substr(0, 2) == "//");
				continue;
			}
			else if (splitted_line.size() == 1) {
				cor = splitted_line[0];
				tim = splitted_line[0];
				sto = splitted_line[0];
			}
			else if (splitted_line.size() == 2) {
				cor = splitted_line[0];
				tim = splitted_line[0];
				sto = splitted_line[1];
			}
			else if (splitted_line.size() == 3) {
				cor = splitted_line[0];
				tim = splitted_line[1];
				sto = splitted_line[2];
			}

			//check if files exist
			bool files_exist = (FileExists((folder + cor + ".cor").c_str()) || FileExists((folder + cor + ".core").c_str()))
				&& (FileExists((folder + tim + ".tim").c_str()) || FileExists((folder + tim + ".time").c_str()))
				&& (FileExists((folder + sto + ".sto").c_str()) || FileExists((folder + sto + ".stoch").c_str()));
			LOG_IF(FATAL, !files_exist) << "One of .cor, .tim, .sto files for " << cor << " was not found.";

			cor_files.push_back(cor);
			tim_files.push_back(tim);
			sto_files.push_back(sto);
		}

		// solve all instances
		for (int i = 0; i < cor_files.size(); i++) {
			solveSMPS(cor_files[i].c_str(), tim_files[i].c_str(), sto_files[i].c_str());
		}
	}

	bool FileExists(const char* filename)
	{
		struct stat info;
		int ret = -1;

		//get the file attributes
		ret = stat(filename, &info);
		if (ret == 0)
		{
			//stat() is able to get the file attributes,
			//so the file obviously exists
			return true;
		}
		else
		{
			//stat() is not able to get the file attributes,
			//so the file obviously does not exist or
			//more capabilities is required
			return false;
		}
	}

protected:
	NestedBendersTest() { // this is the fixture that is called for every test, e.g., TEST(NestedBendersTest, test)
		benders_parameter_.init();
		write_xml("../../../../Data/Benders/default_param.xml", benders_parameter_.parameter_);
		repetitions_ = 1;
		use_multi_cut_ = true;
		generate_initial_solution_ = false;
		compute_statistics_ = false;

		parameter_strings_.push_back("{}"); // default parameters - single-cut
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ\", \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 1 }"); // single cut
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0 }"); // multi cut
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0 }"); // hybrid 
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 1, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_SCENARIO_WISE\" }"); // warm-start scenario-wise
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 1, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_STAGE_WISE\" }"); // warm-start stage-wise
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : 1 }"); // warm-start
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : 2 }"); // warm-start
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_USE_START_SOLUTION\" : 2, \"PNB_NUM_CPU\" : 0 }"); // multi cut, PNB_INT_USE_EV_SOLUTION start solution
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 1 }"); // multi cut single core
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_CUT_CONSOLIDATION\" : true, \"PNB_WARM_START\" : 1 }");
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL\", \"PNB_USE_START_SOLUTION\" : 2 }"); // all level methods require the start solution
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_MANHATTAN\", \"PNB_USE_START_SOLUTION\" : 2 }");
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_INFINITY\", \"PNB_USE_START_SOLUTION\" : 2 }");
		//parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA\" }"); // crashes for some instances
		//parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_ODA\" }"); // ODA crashes and not working for multi stage
		//parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_REGULARIZATION_ODA\" }"); // crashes and not working for some problems
		//parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA\" }"); // crashes and not working for some problems
		//parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA\" }"); // crashes and not working for some problems
	}
	virtual ~NestedBendersTest() {}
	virtual void SetUp() { // this is also the fixture that is called for every test, e.g., TEST(NestedBendersTest, test)
		instance_specification_file_ = "../../../../Data/Benders/InstanceSpecs.csv";
		testbed_log_file_string_ = "../../../../Data/Benders/Results.csv";
		testbed_aggregated_log_file_string_ = "../../../../Data/Benders/Results-Aggregated.csv";

		// Write title line.
		std::stringstream s;
		// Header line of File should be this one
		// We need to write the header line for this file
		s << "Testset;Problem;Instance;Overall Wall Clock Time (s);Iterations;Solution value;UB;LB;Relative Gap;Return Code;Cont Phase Iterations;Infeasibility Iterations;Substantial Iterations;Insubstantial Iterations;Solver Wall Clock Time (s);Critical Iterations;Max Length between Crit. It.;";
		s << "Created Optimality Cuts;Created Continuous Optimality Cuts;Created Feasibility Cuts;Created Epigraph Cuts;Created Strengthened Epigraph Cuts;Created Plain Epigraph Cuts;Created Domain Cuts;Solved L&P CGPs;Integer Gap;";
		// //TODO: Iterate over property tree
		BOOST_FOREACH(boost::property_tree::ptree::value_type & v, benders_parameter_.parameter_)
		{
			std::string name = v.first;
			s << name << ";";
		}
		// Detailed Time for several stages. 
		s << "Time Setup;";
		for (int i = 0; i < max_stages_; i++) {
			s << "Time Stage " << i << ";";
		}
		// Detailled Time for several tasks
		s << "Time Master;Time Projection;Time ODA;Time Sub;Build Master;Build Projection;Build ODA;Build Sub;Generate Cut;Time Cont Phase;";
		// L&P Cuts related timers
		s << "Solve MP Relaxation;Build and Solve PNB CGP;Build Dual CGP;Generate L&P Cut;";
		s << "Solve Dual CGP;L&P Cuts Timer 5;L&P Cuts Timer 6;L&P Cuts Timer 7;L&P Cuts Timer 8;L&P Cuts Timer 9;";
		// Date/Timestamp
		s << "Date;Time;";
		s << "Min_CPU;Min_Solver;Max_CPU;Max_Solver";
		parameter_name_string_ = s.str();

		if (!FileExists(testbed_log_file_string_.c_str())) {
			ofstream log_file;
			log_file.open(testbed_log_file_string_.c_str(), ios::app | ios::out);
			log_file << parameter_name_string_ << std::endl;
			log_file.close();

		}
		if (!FileExists(testbed_aggregated_log_file_string_.c_str())) {
			ofstream log_file;
			log_file.open(testbed_aggregated_log_file_string_.c_str(), ios::app | ios::out);
			log_file << parameter_name_string_ << std::endl;
			log_file.close();
		}

		// We need to check if first line is still valid
		ifstream log_file(testbed_log_file_string_.c_str());
		std::string line;
		std::getline(log_file, line);
		log_file.close();
		if (line != parameter_name_string_) {
			LOG(FATAL) << "Your parameter line changed in your output log file. Please remove old log file " << testbed_log_file_string_ << " before proceeding!.";
		}

		log_file.open(testbed_aggregated_log_file_string_.c_str());
		std::getline(log_file, line);
		log_file.close();
		if (line != parameter_name_string_) {
			LOG(FATAL) << "Your parameter line changed in your output log file. Please remove old log file " << testbed_aggregated_log_file_string_ << " before proceeding!.";
		}

	}
	virtual void TearDown() {}

	double solveProblemWithParameters(const char* core_name, const char* time_name, const char* stoch_name, double* supplied_solution = NULL, bool stats_to_file = true) {
		// We set the correct folder here: All files are in Benders/instances
		std::string folder("../../../../Data/Benders/");
#if _M_X64 || __x86_64__ // Works only on windows i suppose
		folder = "../../../../Data/Benders/";
#endif
		std::string core_name_string(core_name);
		std::string time_name_string(time_name);
		std::string stoch_name_string(stoch_name);
		std::string total_core_string = folder + core_name_string;
		std::string total_time_string = folder + time_name_string;
		std::string total_stoch_string = folder + stoch_name_string;
		// Store some data in aggregated-form (for repetitions_ > 1)

		// Store results we would like to keep
		std::map<std::string, double> values_map;

		std::string cpu_time_string("cpu");
		std::string solver_time_string("solver");
		std::string iterations_string("iterations");
		std::string insubstantial_iterations_string("insusbstantial_iterations");
		std::string substantial_iterations_string("substantial_iterations");
		std::string infeasibility_iterations_string("infeasibility_iterations");
		std::string objective_value_string("objective_value");
		std::string ub_string("ub");
		std::string lb_string("lb");
		std::string relative_gap_string("relative_gap");
		std::string optimality_cuts_string("optimality_cuts");
		std::string cont_optimality_cuts_string("cont_optimality_cuts");
		std::string feasibility_cuts_string("feasibility_cuts");
		std::string epigraph_cuts_string("epigraph_cuts");
		std::string epigraph_str_cuts_string("epigraph_strengthened_cuts");
		std::string epigraph_plain_cuts_string("epigraph_plain_cuts");
		std::string domain_cuts_string("domain_cuts");
		std::string lp_cgp_problems_string("lp_cgp_problems");
		std::string integer_gap_string("integer_gap");
		std::string return_code_string("return_code");
		std::string cont_phase_iterations_string("cont_iterations");
		std::string critical_iteration_sequence_length_string("critical_iteration_sequence");
		std::string critical_iterations_string("critical_iterations");
		std::string time_stage_0_string("stage0");
		std::string time_stage_1_string("stage1");
		std::string time_stage_2_string("stage2");
		std::string time_stage_3_string("stage3");
		std::string time_stage_4_string("stage4");
		std::string time_stage_5_string("stage5");
		std::string time_stage_6_string("stage6");
		std::string time_stage_7_string("stage7");
		std::string time_stage_8_string("stage8");
		std::string time_stage_9_string("stage9");
		std::string time_initial_string("time_initial");
		std::string computing_time_0_string("computing0");
		std::string computing_time_1_string("computing1");
		std::string computing_time_2_string("computing2");
		std::string computing_time_3_string("computing3");
		std::string computing_time_4_string("computing4");
		std::string building_time_0_string("building0");
		std::string building_time_1_string("building1");
		std::string building_time_2_string("building2");
		std::string building_time_3_string("building3");
		std::string building_time_4_string("building4");
		std::string solve_master_relaxation_string("lp_timer0");
		std::string build_and_solve_pnb_cgp_string("lp_timer1");
		std::string build_dual_cgp_string("lp_timer2");
		std::string lp_generation_wall_clock_string("lp_timer3");
		std::string solve_dual_cgp_string("lp_timer4");
		std::string lp_timer_5("lp_timer5");
		std::string lp_timer_6("lp_timer6");
		std::string lp_timer_7("lp_timer7");
		std::string lp_timer_8("lp_timer8");
		std::string lp_timer_9("lp_timer9");


		std::stringstream aggregated_s;
		int num_stages = 0;
		double min_solver = DBL_MAX;
		double min_cpu = DBL_MAX;
		double max_solver = 0.0;
		double max_cpu = 0.0;

		stringstream testset_problem_instance;
		testset_problem_instance.str("");

		const int orig_repetitions = repetitions_;
		for (int loop_counter = 0; loop_counter < repetitions_; loop_counter++) {
			parallel_benders::Timer overall_and_solver_timer(2);
			// This can be done in a loop.
			overall_and_solver_timer.Start(0);
			SmiScnModel smi;
			OsiCpxSolverInterface osi_cpx;
			smi.setOsiSolverHandle(&osi_cpx);
			int read_in = smi.readSmps(total_core_string.c_str(), total_time_string.c_str(), total_stoch_string.c_str());
			if (read_in != 0) {
				LOG(ERROR) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
				return -osi_cpx.getInfinity();
			}
			num_stages = smi.getCore()->getNumStages();
			// Load ParallelNestedBendersSolver
			overall_and_solver_timer.Start(1);
			OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, &smi);
			nested_benders.setTestsetProblemInstanceNames(total_stoch_string);
			if (benders_parameter_.GetBoolParamValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS)) {
				nested_benders.setInstanceSpecsFile(instance_specification_file_);
			}
			nested_benders.setParameters(benders_parameter_);
			if (benders_parameter_.parameter_.get(parallel_benders::PNB_USE_START_SOLUTION, 0) == parallel_benders::PNB_INT_USE_SUPPLIED_SOLUTION) {
				assert(current_start_solution_.get() != nullptr);
				nested_benders.giveStartSolution(current_start_solution_.get(), num_single_scenario_cols_);
			}
			nested_benders.initialSolve();
			overall_and_solver_timer.Stop(0);
			overall_and_solver_timer.Stop(1);
			double solver_time = overall_and_solver_timer.GetTime(1); // Solver only time
			double cpuTime = overall_and_solver_timer.GetTime(0); // Overall WallClockTime;
			nested_benders.printSolutionToFile("primal_solution.txt");
			min_cpu = min(cpuTime, min_cpu);
			min_solver = min(solver_time, min_solver);
			max_cpu = max(max_cpu, cpuTime);
			max_solver = max(max_solver, solver_time);
			LOG(INFO) << "Solution Time: " << cpuTime;

			if (benders_parameter_.parameter_.get(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, 0) != parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO
				&& !(benders_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, 0) == parallel_benders::PNB_INT_ALGORITHM_DETEQ || benders_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, 0) == parallel_benders::PNB_INT_ALGORITHM_DETEQ_RISK)) {
				// Get num_cols when specified correctly
				// This works for non-appended problems!
				int last_stage = benders_parameter_.parameter_.get(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, 0);
				int number_of_cols = smi.getCore()->getNumCols(0);
				if (last_stage == parallel_benders::PNB_INT_SOLUTION_PRIMAL_ALL)
					last_stage = num_stages - 2;
				for (int i = 1; i <= last_stage; i++)
					number_of_cols += smi.getCore()->getNumCols(i);
				num_single_scenario_cols_ = number_of_cols;
				first_stage_solution_.reset(new double[num_single_scenario_cols_]);
				int offset_cols = 0;
				CoinFillN(first_stage_solution_.get(), number_of_cols, 0.0);
				for (int i = 0; i <= last_stage; i++) {
					const double* first_stage = nested_benders.getSolutionByNodeAndStage(0, i);
					if (first_stage != nullptr)
						std::copy(first_stage + offset_cols, first_stage + offset_cols + smi.getCore()->getNumCols(i), first_stage_solution_.get() + offset_cols);
					offset_cols += smi.getCore()->getNumCols(i);
				}
			}
			int created_opt_cuts = nested_benders.getNumOptimalityCuts();
			int created_cont_opt_cuts = nested_benders.getNumContinuousOptimalityCuts();
			int created_feas_cuts = nested_benders.getNumFeasibilityCuts();
			int created_epigraph_cuts = nested_benders.getNumEpigraphCuts();
			int created_str_epigraph_cuts = nested_benders.getNumStrengthenedEpigraphCuts();
			int created_plain_epigraph_cuts = nested_benders.getNumPlainEpigraphCuts();
			int created_domain_cuts = nested_benders.getNumDomainCuts();
			int solved_lp_cgp_problems = nested_benders.getLpCgpCounter();

			// We get the parameters from the solver back to evaluate it.
			benders_parameter_ = nested_benders.getParameters();

			// File name Should have the following form based on Parameters: Single/Multi/Aggregate-NumberCuts/Aggregate-Percent-Number/Aggregate-Fixed-Number
			std::stringstream s;
			s.str("");
			testset_problem_instance.str("");

			// Write solution line
			//Testset;Problem;Instance
			testset_problem_instance << nested_benders.getTestsetProblemInstanceNames()[0] << ";" << nested_benders.getTestsetProblemInstanceNames()[1] << ";" << nested_benders.getTestsetProblemInstanceNames()[2];
			s << testset_problem_instance.str() << ";" << cpuTime << ";" << nested_benders.getNumIterations() << ";" << nested_benders.getObjValue() << ";" << nested_benders.getUB() << ";" << nested_benders.getLB() << ";";
			s << nested_benders.getRelativeGap() << ";" << nested_benders.getSolutionCode() << ";" << nested_benders.getNumInfeasibilityIterations() << ";";
			s << nested_benders.getNumIterations() - nested_benders.getNumInsubstantialIterations() << ";" << nested_benders.getNumInsubstantialIterations() << ";";
			s << solver_time << ";" << nested_benders.getNumCriticalIteration() << ";" << nested_benders.getMaximalLengthOfCriticalIterationSequence() << ";";
			// Write num cuts
			s << created_opt_cuts << ";" << created_feas_cuts << ";" << created_epigraph_cuts << ";" << created_str_epigraph_cuts << ";" << created_plain_epigraph_cuts << ";" << created_domain_cuts << ";" << solved_lp_cgp_problems;
			// Store all of the above values in value_map!
			values_map[cpu_time_string] += cpuTime;
			values_map[solver_time_string] += solver_time;
			values_map[iterations_string] += nested_benders.getNumIterations();
			values_map[insubstantial_iterations_string] += nested_benders.getNumInsubstantialIterations();
			values_map[substantial_iterations_string] += nested_benders.getNumIterations() - nested_benders.getNumInsubstantialIterations();
			values_map[infeasibility_iterations_string] += nested_benders.getNumInfeasibilityIterations();
			values_map[objective_value_string] += nested_benders.getObjValue();
			values_map[ub_string] += nested_benders.getUB();
			values_map[lb_string] += nested_benders.getLB();
			values_map[relative_gap_string] += nested_benders.getRelativeGap();
			values_map[optimality_cuts_string] += created_opt_cuts;
			values_map[cont_optimality_cuts_string] += created_cont_opt_cuts;
			values_map[feasibility_cuts_string] += created_feas_cuts;
			values_map[epigraph_cuts_string] += created_epigraph_cuts;
			values_map[epigraph_str_cuts_string] += created_str_epigraph_cuts;
			values_map[epigraph_plain_cuts_string] += created_plain_epigraph_cuts;
			values_map[domain_cuts_string] += created_domain_cuts;
			values_map[lp_cgp_problems_string] += solved_lp_cgp_problems;
			values_map[integer_gap_string] += nested_benders.getIntegerGap();
			values_map[cont_phase_iterations_string] += nested_benders.getNumContPhasePnbIterations();
			values_map[return_code_string] += nested_benders.getSolutionCode();
			values_map[critical_iterations_string] += nested_benders.getNumCriticalIteration();
			values_map[critical_iteration_sequence_length_string] += nested_benders.getMaximalLengthOfCriticalIterationSequence();

			ub_for_iteration_.clear();
			lb_for_iteration_.clear();
			for (int i = 0; i < nested_benders.getNumIterations(); i++) {
				ub_for_iteration_.push_back(nested_benders.getUBForIteration(i + 1));
				lb_for_iteration_.push_back(nested_benders.getLBForIteration(i + 1));
			}

			// Write parameter values
			// Tree is ordered after insertion. This should be the same as the first iteration, because we insert EVERY value in the init() method of the parameter class.
			BOOST_FOREACH(boost::property_tree::ptree::value_type & v, benders_parameter_.parameter_)
			{
				std::string name = v.first;
				std::string value = benders_parameter_.parameter_.get<std::string>(name);
				s << value << ";";
			}
			// Write Detailed Time Statistics
			const parallel_benders::Timer& timer(nested_benders.getTimer());
			s << timer.GetTime(timer.GetTimeStages() - 1) << ";"; // num_stages is time for setup and stuff, because we set the timer to num_stages+1 many stages
			values_map[time_initial_string] += timer.GetTime(timer.GetTimeStages() - 1);

			if (num_stages >= max_stages_)
				LOG(ERROR) << "Only the first " << num_stages << " stages get reported with wall clock time.";
			for (int i = 0; i < num_stages; i++) {
				s << timer.GetTime(i) << ";";
				values_map["stage" + boost::lexical_cast<std::string>(i)] += timer.GetTime(i);
			}
			for (int i = 0; i < max_stages_ - num_stages; i++)// Ensure that the date/time stamp is where it is supposed to be!
				s << 0 << ";";

			const parallel_benders::Timer& solution_timer(nested_benders.getComputingTimer());
			const parallel_benders::Timer& build_timer(nested_benders.getBuildTimer());
			for (int i = 0; i < solution_timer.GetTimeStages(); i++) {
				s << solution_timer.GetTime(i) << ";";
				values_map["computing" + boost::lexical_cast<std::string>(i)] += solution_timer.GetTime(i);
			}
			for (int i = 0; i < build_timer.GetTimeStages(); i++) {
				s << build_timer.GetTime(i) << ";";
				values_map["building" + boost::lexical_cast<std::string>(i)] += build_timer.GetTime(i);
			}

			// L&P Cuts Times
			const parallel_benders::Timer& lp_cuts_timer(nested_benders.getLiftAndProjectCutsTimer());
			for (int i = 0; i < lp_cuts_timer.GetTimeStages(); i++) {
				s << lp_cuts_timer.GetTime(i) << ";";
				values_map["lp_timer" + boost::lexical_cast<std::string>(i)] += lp_cuts_timer.GetTime(i);
			}

			time_t timeStamp = time(NULL);
			tm* time = localtime(&timeStamp);
			char buffer[256];
			strftime(buffer, 256, "%m/%d/%y; %H:%M:%S", time);
			s << buffer;
			s << "\n";

			// Write to testbed log file
			if (stats_to_file) {
				ofstream testbed_log_file;
				testbed_log_file.open(testbed_log_file_string_.c_str(), ios::app | ios::out);
				testbed_log_file << s.str();
				testbed_log_file.close();
			}
			
			// No need to solve several times in case of fail
			if (loop_counter == 0 && nested_benders.getSolutionCode() == parallel_benders::PNB_SOLUTION_TIME_LIMIT) {
				repetitions_ = 1;
				values_map[solver_time_string] = nested_benders.getParameters().parameter_.get(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 3600);
			}
			else if (nested_benders.getSolutionCode() != parallel_benders::PNB_SOLUTION_OPTIMAL && nested_benders.getSolutionCode() != parallel_benders::PNB_SOLUTION_UPPER_LESS_LOWER_BOUND) {
				repetitions_ = 1;
			}
		}

		// Compute average 
		BOOST_FOREACH(double& value, values_map | boost::adaptors::map_values) {
			value /= repetitions_;
		}
		// Prepare outputstring, according to ordering from above (bad coding..)
		aggregated_s << testset_problem_instance.str() << ";" << values_map[cpu_time_string] << ";" << values_map[iterations_string] << ";" << values_map[objective_value_string] << ";" << values_map[ub_string] << ";" << values_map[lb_string] << ";" << values_map[relative_gap_string] << ";" << values_map[return_code_string] << ";" << values_map[cont_phase_iterations_string] << ";" << values_map[infeasibility_iterations_string] << ";" << values_map[substantial_iterations_string] << ";" << values_map[insubstantial_iterations_string] << ";" << values_map[solver_time_string] << ";" << values_map[critical_iterations_string] << ";" << values_map[critical_iteration_sequence_length_string] << ";";
		// Write num cuts
		aggregated_s << values_map[optimality_cuts_string] << ";" << values_map[cont_optimality_cuts_string] << ";" << values_map[feasibility_cuts_string] << ";" << values_map[epigraph_cuts_string] << ";" << values_map[epigraph_str_cuts_string] << ";" << values_map[epigraph_plain_cuts_string] << ";" << values_map[domain_cuts_string] << ";" << values_map[lp_cgp_problems_string] << ";" << values_map[integer_gap_string] << ";";

		BOOST_FOREACH(boost::property_tree::ptree::value_type & v, benders_parameter_.parameter_)
		{
			std::string name = v.first;
			std::string value = benders_parameter_.parameter_.get<std::string>(name);
			aggregated_s << value << ";";
		}
		// Write Detailed Time Statistics
		aggregated_s << values_map[time_initial_string] << ";"; // num_stages is time for setup and stuff
		if (num_stages > max_stages_)
			LOG(ERROR) << "Only the first " << num_stages << " stages get reported with wall clock time.";
		for (int i = 0; i < num_stages; i++) {
			aggregated_s << values_map["stage" + boost::lexical_cast<std::string>(i)] << ";";
		}
		for (int i = 0; i < max_stages_ - num_stages; i++)// Ensure that the date/time stamp is where it is supposed to be by inserting missing zeroes
			aggregated_s << 0 << ";";

		// Write Time for Master solves, etc. Write Building Time for Master solves etc.
		aggregated_s << values_map[computing_time_0_string] << ";";
		aggregated_s << values_map[computing_time_1_string] << ";";
		aggregated_s << values_map[computing_time_2_string] << ";";
		aggregated_s << values_map[computing_time_3_string] << ";";
		aggregated_s << values_map[computing_time_4_string] << ";";
		aggregated_s << values_map[building_time_0_string] << ";";
		aggregated_s << values_map[building_time_1_string] << ";";
		aggregated_s << values_map[building_time_2_string] << ";";
		aggregated_s << values_map[building_time_3_string] << ";";
		aggregated_s << values_map[building_time_4_string] << ";";
		aggregated_s << values_map[solve_master_relaxation_string] << ";";
		aggregated_s << values_map[build_and_solve_pnb_cgp_string] << ";";
		aggregated_s << values_map[build_dual_cgp_string] << ";";
		aggregated_s << values_map[lp_generation_wall_clock_string] << ";";
		aggregated_s << values_map[solve_dual_cgp_string] << ";";
		aggregated_s << values_map[lp_timer_5] << ";";
		aggregated_s << values_map[lp_timer_6] << ";";
		aggregated_s << values_map[lp_timer_7] << ";";
		aggregated_s << values_map[lp_timer_8] << ";";
		aggregated_s << values_map[lp_timer_9] << ";";

		time_t timeStamp = time(NULL);
		tm* time = localtime(&timeStamp);
		char buffer[256];
		strftime(buffer, 256, "%m/%d/%y; %H:%M:%S;", time);
		aggregated_s << buffer;

		// Min,Max times
		aggregated_s << min_cpu << ";" << min_solver << ";" << max_cpu << ";" << max_solver;
		aggregated_s << "\n";

		// Write string to file
		// Write to testbed log file
		if (stats_to_file) {
			ofstream testbed_aggregated_log_file;
			testbed_aggregated_log_file.open(testbed_aggregated_log_file_string_.c_str(), ios::app | ios::out);
			testbed_aggregated_log_file << aggregated_s.str();
			testbed_aggregated_log_file.close();
		}

		repetitions_ = orig_repetitions;


		return values_map[objective_value_string];
	}

	void updateParameterSets() {

		parameter_sets_.clear();

		for (std::string json_string : parameter_strings_) {
			parallel_benders::Parameter param(benders_parameter_); // based on benders_parameter_ which normally has default values unless changed in TEST_F(){...}
			param.update_from_json_string(json_string);
			parameter_sets_.push_back(param);
		}
	}

	std::vector<std::string> parameter_strings_;
	boost::shared_array<double> first_stage_solution_;
	boost::shared_array<double> current_start_solution_;
	boost::shared_array<double> best_case_solution_;
	boost::shared_array<double> worst_case_solution_;
	std::vector<double> ub_for_iteration_;
	std::vector<double> lb_for_iteration_;
	int num_stages_;
	int num_single_scenario_cols_;
	int special_case_solution_length_;
	int repetitions_;
};


// Multi Stage Problems!
class NestedBendersTestMultiStage : public NestedBendersTest
{
protected:
	NestedBendersTestMultiStage() { // override parameter_strings_
		// parameter_strings_ the same as for base NestedBendersTest
	}
};

// SAA problems
class NestedBendersTestSampleAverageApproximation : public NestedBendersTest
{
public:

	NestedBendersTestSampleAverageApproximation()
		: NestedBendersTest() {

	}
	void SolveSAA(const std::string& core_name_string) {
		// We set the correct folder here: All files are in Benders/instances
		std::string folder("../../../../Data/Benders/");
#if _M_X64 || __x86_64__ // Works only on windows i suppose
		folder = "../../../../Data/Benders/";
#endif
		OsiCpxSolverInterface osi_cpx;
		SmiScnModel smi;
		smi.setOsiSolverHandle(&osi_cpx);
		smi.setSampling(true);
		std::string filename = folder + core_name_string;
		int read_in = smi.readSmps(filename.c_str());

		if (read_in != 0) {
			LOG(FATAL) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
		}
		SampleAverageApproximationSolverInterface saa_solver(&osi_cpx, &smi); // Initialize solver!
		// Set options we want to use for the solver used inside the SAA solver
		parallel_benders::Parameter benders_parameter;
		benders_parameter.init();
		//benders_parameter.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_LEVEL_ODA);
		//benders_parameter.parameter_.put(parallel_benders::PNB_FEAS_CUT_LP_METHOD,parallel_benders::PNB_INT_DUAL_SIMPLEX);
		//benders_parameter.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,1);


		std::map< int, std::pair<int, int> > sample_map;
		sample_map[0] = make_pair(500, 20000); // First Entry: Sample Size for problem, Second Entry: Sample Size for Evaluation
		sample_map[1] = make_pair(1000, 20000);
		sample_map[2] = make_pair(5000, 20000);
		sample_map[3] = make_pair(10000, 20000);
		sample_map[4] = make_pair(20000, 50000);
		sample_map[5] = make_pair(50000, 100000);
		int runs = sample_map.size();

		int num_batches_lower = 10;
		int num_batches_upper = 20;
		fstream logfile;
		for (int i = 0; i < runs; i++) {
			// Get a new seed for every run by reinstating the Uniform01Generator
			theGenerator().chooseNewSeed();
			logfile.open("saa-log.txt", fstream::out | fstream::app);
			logfile << core_name_string << std::endl;
			logfile.close();
			parallel_benders::Timer solver_timer(1);
			solver_timer.Start(0);
			saa_solver.setParameters(benders_parameter);
			// These values do not change!
			saa_solver.setNumberBatches(num_batches_lower);
			saa_solver.setNumberBatchesUpperBound(num_batches_upper);
			// End values that not change
			std::vector<int> sample_size(2, 0);
			sample_size[1] = sample_map[i].first;
			saa_solver.setSampleSize(sample_size);
			std::vector<int> sample_size_upper(2, 0);
			sample_size_upper[1] = sample_map[i].second;
			saa_solver.setSampleSizeUpperBound(sample_size_upper);
			double alpha = 0.95;
			saa_solver.setConfidenceLevel(alpha);
			saa_solver.initialSolve();
			solver_timer.Stop(0);
			double solver_time = solver_timer.GetTime(0); // Solver only time

			logfile.open("saa-log.txt", fstream::out | fstream::app);
			logfile << "Overall Solution Time: " << solver_time << std::endl;

			time_t timeStamp = time(NULL);
			tm* time = localtime(&timeStamp);
			char buffer[256];
			strftime(buffer, 256, "%m/%d/%y; %H:%M:%S;", time);
			logfile << "Timestamp: " << buffer << "\n";
			logfile << "--------------------------------------------\n";
			logfile.close();
			LOG(INFO) << "Solution Time: " << solver_time;
		}
	}

protected:

};

// Integer problems (second stage mixed-integer)
class NestedBendersTestInteger : public NestedBendersTest
{
protected:
	NestedBendersTestInteger() { // override parameter_strings_
		parameter_strings_.clear();
		//parameter_strings_.push_back("{}"); // default parameters
		// deterministic equivalent (DETEQ)
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ\", \"PNB_NUM_CPU\" : 1 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ\", \"PNB_NUM_CPU\" : 0 }");

		// single core (1CORE), simple
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1}"); // single cut
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0 }"); // multi cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20 }"); // hybrid cut

		// multithread (MCORE), simple
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 0 }"); // single cut (SC)
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0 }"); // multi cut (MC)
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0 }"); // hybrid cut (HC)
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 100, \"PNB_NUM_CPU\" : 0 }"); // hybrid cut (HC)

		// multithread (MCORE), scenario wise warm start (SCWS)
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_SCENARIO_WISE\" }"); // single cut
		parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_SCENARIO_WISE\" }"); // multi cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_SCENARIO_WISE\" }"); // hybrid cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 100, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_SCENARIO_WISE\" }"); // hybrid cut

		// multithread (MCORE), stagewise warm start (STWS)
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_STAGE_WISE\" }"); // single cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_STAGE_WISE\" }"); // multi cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_STAGE_WISE\" }"); // hybrid cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 100, \"PNB_NUM_CPU\" : 0, \"PNB_WARM_START\" : \"PNB_INT_WARM_START_STAGE_WISE\" }"); // hybrid cut

		// multithread (MCORE), cut consolidation (CC)
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 0, \"PNB_CUT_CONSOLIDATION\" : true }"); // single cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0, \"PNB_CUT_CONSOLIDATION\" : true }"); // multi cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_CUT_CONSOLIDATION\" : true }"); // hybrid cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 100, \"PNB_NUM_CPU\" : 0, \"PNB_CUT_CONSOLIDATION\" : true }"); // hybrid cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1000, \"PNB_NUM_CPU\" : 0, \"PNB_CUT_CONSOLIDATION\" : true }"); // hybrid cut

		// multithread (MCORE), ODA
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA\" }"); // single cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA\" }"); // multi cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 20, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA\" }"); // hybrid cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 100, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA\" }"); // hybrid cut
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 1000, \"PNB_NUM_CPU\" : 0, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA\" }"); // hybrid cut

		// multithread (MCORE), PNB_INT_USE_EV_SOLUTION
		//parameter_strings_.push_back("{ \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0, \"PNB_USE_START_SOLUTION\" : 2 }"); // multi cut

		// LEVEL does not work with MIP
	}
};

// Tests set for L&P cuts (relaxation from stage 2)
class LiftAndProjectCutsTest : public NestedBendersTest
{
protected:
	LiftAndProjectCutsTest() { // override parameter_strings_
		parameter_strings_.clear();
		//parameter_strings_.push_back("{}"); // default parameters
		
		// DETEQ
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ\", \"PNB_NUM_CPU\" : 0 }");

		// BENDERS CPLEX
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_SOLVER_BENDERS\", \"PNB_NUM_CPU\" : 0 }");

		// BENDERS
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_USE_FIRST_PHASE\" : true }");
		
		// DZN problem - plain Epigraph cut (w/o 2nd stage info)
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 1, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10, \"PNB_CGP_EPSILON_RELATIVE\" : 0.01  }");
		
		// DZN problem - strengthened Epigraph cut (with 2nd stage info)
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");

			//PNB_CGP_EPSILON_RELATIVE
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.5 }");		
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.1 }");
		//default: parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.0001 }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.000001 }");

			//PNB_LP_CUTS_FOR_FRACT
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 5  , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");
		//default: parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 20 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 100, \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");

			//PNB_LP_ROUNDS_LIMIT
		//default: parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20 , \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 40 , \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 100, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01 }");

			//PNB_LP_USE_CGP_PNB_CUTS
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01, \"PNB_LP_USE_CGP_PNB_CUTS\" : \"PNB_CGP_CUTS_NONE\" }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01, \"PNB_LP_USE_CGP_PNB_CUTS\" : \"PNB_CGP_CUTS_EFFICIENT_AND_CONSOLIDATE\" }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01, \"PNB_LP_USE_CGP_PNB_CUTS\" : \"PNB_CGP_CUTS_EFFICIENT\" }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01, \"PNB_LP_USE_CGP_PNB_CUTS\" : \"PNB_CGP_CUTS_CONSOLIDATE\" }");
		parameter_strings_.push_back("{ \"PNB_NUM_CPU\" : 0, \"PNB_LIFT_AND_P_METHOD\" : 2, \"PNB_LP_ROUNDS_LIMIT\" : 20, \"PNB_LP_SWITCHES_LIMIT\" : 1, \"PNB_USE_FIRST_PHASE\" : true, \"PNB_LP_CUTS_FOR_FRACT\" : 10 , \"PNB_CGP_EPSILON_RELATIVE\" : 0.01, \"PNB_LP_USE_CGP_PNB_CUTS\" : \"PNB_CGP_CUTS_ALL\" }");
	}
};

class CVaRTest : public NestedBendersTest
{
protected:
	CVaRTest() { // override parameter_strings_
		parameter_strings_.clear();
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ_RISK\", \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_RISK\", \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_RISK\", \"PNB_USE_START_SOLUTION\" : 2, \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA_RISK\", \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_LEVEL_ODA_RISK\", \"PNB_USE_START_SOLUTION\" : 2, \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_RISK_ALPHA\", \"PNB_NUM_CPU\" : 0 }");
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS_ODA_RISK_ALPHA\", \"PNB_NUM_CPU\" : 0 }");
	}
};

// Integer problems with the integer variables relaxed on all stages
class RelaxationTest : public NestedBendersTest
{
protected:
	RelaxationTest() { // override parameter_strings_
		parameter_strings_.clear();
		parameter_strings_.push_back("{ \"PNB_RELAXATION\" : 1, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ\", \"PNB_NUM_CPU\" : 0 }"); // single cut by default
		parameter_strings_.push_back("{ \"PNB_RELAXATION\" : 1, \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_BENDERS\", \"PNB_AGGREGATED_OPTIMALITY_CUTS\" : 0, \"PNB_NUM_CPU\" : 0 }"); // multi cut
	}
};

class DetEqTest : public NestedBendersTest
{
protected:
	DetEqTest() { // override parameter_strings_
		parameter_strings_.clear();
		parameter_strings_.push_back("{ \"PNB_BENDERS\" : \"PNB_INT_ALGORITHM_DETEQ\", \"PNB_NUM_CPU\" : 0 }");
	}
};

// Problems that need/generate feasibility cuts
class NestedBendersTestFeasibility : public NestedBendersTest
{
protected:
	NestedBendersTestFeasibility() { // override parameter_strings_
		// parameter_strings_ the same as for base NestedBendersTest
	}
};

// Tests set to simply collect properties of instances to .csv file
class CollectInstanceSpecs : public NestedBendersTest
{
protected:
	CollectInstanceSpecs() { // override parameter_strings_
		parameter_strings_.clear();
		// 1 seconds sollution time to stop the algorithm in Solve() main loop. We only want to store the instance specs
		parameter_strings_.push_back("{ \"PNB_SAVE_INSTANCE_SPECS\" : true, \"PNB_MAXIMAL_SOLUTION_TIME\" : 1,  \"PNB_RELAXATION\" : 2 }");
	}
};

TEST(UnitTestPNB, unitTest) {
	
	//SmiScnModel smi;
	std::string folder("../../../../Data/Benders/");
	std::string core_name;
	std::string time_name;
	std::string stoch_name;
	std::string total_core_string;
	std::string total_time_string;
	std::string total_stoch_string;
	
	// Parameter file
	parallel_benders::Parameter param;
	param.init();

	core_name = "glushko/sndp_mini_tests/SNDP_TestCase_0";
	time_name = "glushko/sndp_mini_tests/SNDP_TestCase_0";
	stoch_name = "glushko/sndp_mini_tests/SNDP_TestCase_0";
	total_core_string = folder + core_name;
	total_time_string = folder + time_name;
	total_stoch_string = folder + stoch_name;

	int smps_read_in = 0;
	SmiScnModel smi_model;
	smps_read_in = smi_model.readSmps(total_core_string.c_str(), total_time_string.c_str(), total_stoch_string.c_str());

	if (smps_read_in != 0) {
		LOG(ERROR) << "Reading in your SMPS file failed.";
	}

	// Create a new OsiStochasticPNB interface
	OsiSolverInterface* osi_solver = new OsiCpxSolverInterface();
	OsiStochasticParallelNestedBendersSolverInterface pnb_interface(osi_solver, &smi_model);
	pnb_interface.setParameters(param);

	parallel_benders::ParallelNestedBenders* pnb = pnb_interface.getPNB();
	pnb->Init();
	pnb->Solve();

	pnb->Reset(-1, true);

	boost::threadpool::pool thread_pool_(10);
	std::vector<parallel_benders::ParallelNestedBenders*> thread_pnbs;
	thread_pnbs.reserve(thread_pool_.size());
	for (int i = 0; i < thread_pool_.size(); i++) {
		parallel_benders::ParallelNestedBenders* thread_pnb = pnb->clone();
		thread_pnbs.push_back(thread_pnb);
		thread_pool_.schedule(boost::bind(&parallel_benders::ParallelNestedBenders::Solve, thread_pnb));
	}
	thread_pool_.wait(0);
	
	for (auto thread_pnb : thread_pnbs) {
		delete thread_pnb;
	}
	thread_pnbs.clear();
	delete osi_solver;
}

TEST_F(UnitTestCutStuff, unitTest) {
	using namespace parallel_benders;
	std::vector<double>cut_info;

	//double-prime opt cut with 1 theta
	//		   col,	coef,		lb,	theta,	it,type,col set
	cut_info = { 5,	0,1,2,0,4,	10,	1,		1,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_DOUBLE_PRIME };
	//TODO: the method below does not need theta
	CutStuff cs1(*CutStuffFromCutInfo(cut_info));
	EXPECT_EQ(cs1.getNumberAggregates(), 1);
	EXPECT_EQ(cs1.getNumTheta(), 1);
	EXPECT_EQ(cs1.getAggregates()[0], 0);
	EXPECT_TRUE(cs1.isAggregate(0));
	EXPECT_EQ(cs1.getAggregateType(), CutStuff::AGGREGATE_OVER_ONE);
	EXPECT_EQ(cs1.type_, PNB_CUT_TYPE_OPTIMALITY);
	EXPECT_EQ(cs1.column_set_, PNB_COL_SET_DOUBLE_PRIME);
	EXPECT_TRUE(cs1.is_consistent());

	//optimality cut with 2 thetas and later 3 thetas
	cut_info = { 5,	0,1,2,0,4,	20,	0,1,	2,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE };
	CutStuff cs2(*CutStuffFromCutInfo(cut_info));
	EXPECT_EQ(cs2.getNumberAggregates(), 1);
	EXPECT_EQ(cs2.getNumTheta(), 2);
	EXPECT_EQ(cs2.getAggregates()[0], 1);
	EXPECT_FALSE(cs2.isAggregate(0));
	EXPECT_TRUE(cs2.isAggregate(1));
	EXPECT_EQ(cs2.getAggregateType(), CutStuff::AGGREGATE_OVER_ONE);
	EXPECT_EQ(cs2.type_, PNB_CUT_TYPE_OPTIMALITY);
	EXPECT_EQ(cs2.column_set_, PNB_COL_SET_NONE);
	EXPECT_TRUE(cs2.is_consistent());

	//(re)setAggregate
	cs2.resetAggregates(3);
	cs2.setAggregate(1);
	cs2.setAggregate(2);
	EXPECT_EQ(cs2.getNumberAggregates(), 2);
	EXPECT_EQ(cs2.getAggregates()[0], 1);
	EXPECT_EQ(cs2.getAggregates()[1], 2);
	EXPECT_TRUE(cs2.isAggregate(1));
	EXPECT_TRUE(cs2.isAggregate(2));
	EXPECT_EQ(cs2.getAggregateType(), CutStuff::AGGREGATE_OVER_PARTIAL);
	EXPECT_TRUE(cs2.is_consistent());

	cs2.resetAggregates();
	EXPECT_EQ(cs2.getAggregateType(), CutStuff::AGGREGATE_OVER_NONE);
	EXPECT_TRUE(!cs2.is_consistent()); // because optimality cut should have aggregates
	cs2.setAggregate(0);
	cs2.setAggregate(1);
	cs2.setAggregate(2);
	EXPECT_EQ(cs2.getNumberAggregates(), 3);
	EXPECT_EQ(cs2.getAggregateType(), CutStuff::AGGREGATE_OVER_ALL);
	EXPECT_TRUE(cs2.is_consistent());

	//feas cut with 0 thetas
	//		   col,	coef,		lb,	theta,	it,type,col set
	cut_info = { 5,	2,1,2,0,0,		30,	0,0,	1,PNB_CUT_TYPE_FEASIBILITY,PNB_COL_SET_NONE };
	CutStuff cs3(*CutStuffFromCutInfo(cut_info));
	//void CutStuff::setAggregate(int aggregate);
	EXPECT_EQ(cs3.getNumberAggregates(), 0);
	EXPECT_EQ(cs3.getAggregates().size(), 0);
	EXPECT_EQ(cs3.getAggregateType(), CutStuff::AGGREGATE_OVER_NONE);
	EXPECT_EQ(cs3.type_, PNB_CUT_TYPE_FEASIBILITY);
	EXPECT_EQ(cs3.column_set_, PNB_COL_SET_NONE);
	EXPECT_TRUE(cs3.is_consistent());

	//double-prime opt cut
	//		   col,	coef,		lb,	theta,	it,type,col set
	cut_info = { 5,	0,1,2,0,4,	10,	1,1,	1,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_DOUBLE_PRIME };
	CutStuff cs4(*CutStuffFromCutInfo(cut_info));
	EXPECT_EQ(cs4.getNumberAggregates(), 2);
	EXPECT_EQ(cs4.getAggregates()[0], 0);
	EXPECT_EQ(cs4.getAggregateType(), CutStuff::AGGREGATE_OVER_ALL);
	EXPECT_EQ(cs4.type_, PNB_CUT_TYPE_OPTIMALITY);
	EXPECT_EQ(cs4.column_set_, PNB_COL_SET_DOUBLE_PRIME);
	EXPECT_TRUE(cs4.is_consistent());

	//Metrics
	std::vector<double> solution{ .5, .5, .5, .5, .5 };
	std::vector<double> solution_cgp{ .5, .5, .5, .5, .5 , 1.2, 1.2, 1.2, 1.2, 1.2};
	std::vector<double> theta = { 0, 5.5 };
	EXPECT_NEAR(cs1.AbsoluteViolation(solution_cgp, theta), -3.9, 0.0001);
	EXPECT_NEAR(cs1.RelativeViolation(solution_cgp, theta), -.39, 0.0001);
	EXPECT_NEAR(cs1.DistanceViolation(solution_cgp, theta), -.8315, 0.0001);
	theta = { 3.0, 2.5, 0.0 };
	EXPECT_NEAR(cs2.AbsoluteViolation(solution, theta), 11.0, 0.0001);
	EXPECT_NEAR(cs2.RelativeViolation(solution, theta), .55, 0.0001);
	EXPECT_NEAR(cs2.DistanceViolation(solution, theta), 2.2453, 0.0001);

	EXPECT_NEAR(cs3.AbsoluteViolation(solution, theta), 27.5, 0.0001);
	EXPECT_NEAR(cs3.RelativeViolation(solution, theta), .916667, 0.0001);
	EXPECT_NEAR(cs3.DistanceViolation(solution, theta), 9.16667, 0.0001);

	//ConsolidateCuts
	{
		//****** ConsolidateCuts cut_in != cut_out
		std::vector<std::vector<double>>cut_infos;
		
		//It. 3 (iteration < from_iteration: not consolidate)
		
		//					 col,	coef,		lb,	theta,	it,type,col set
		cut_infos.push_back({ 4,	1,1,1,1,	10,	1,0,0,	3,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		cut_infos.push_back({ 4,	2,2,2,2,	20,	0,1,0,	3,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		
		//It. 4 (simplest case)
		cut_infos.push_back({  4,	1,1,1,1,	10,	1,0,0,	4,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		cut_infos.push_back({  4,	2,2,2,2,	20,	0,1,0,	4,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		cut_infos.push_back({  4,	3,3,3,3,	30,	0,0,1,	4,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });

		//It. 5 (only one suitable cut for PNB_COL_SET_DOUBLE_PRIME, AGGREGATE_OVER_PARTIAL for PRIME, different col sets)
		cut_infos.push_back({ 4,	2,2,2,2,	20,	0,1,0,	5,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		cut_infos.push_back({ 4,	1,1,1,1,	10,	1,0,0,	5,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_DOUBLE_PRIME });
		cut_infos.push_back({ 4,	3,3,3,3,	30,	0,0,1,	5,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		
		//It. 6 and 8 (some coefs are 0, some resulting consolidated cuts have 0 coefs, cuts not ordered by iteration)
		cut_infos.push_back({ 4,	1,0,1,1,	10,	0,0,1,	8,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		cut_infos.push_back({ 4,	0,2,2,2,	20,	0,1,0,	6,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		cut_infos.push_back({ 4,	2,0,2,0,	20,	0,1,0,	8,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		cut_infos.push_back({ 4,	1,1,1,1,	0,	1,0,0,	6,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		cut_infos.push_back({ 4,	3,3,0,3,	30,	0,0,1,	6,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		
		//It. 7 (different num cols in iteration, domain, epigraph, feas cuts)
		cut_infos.push_back({ 3,	2,2,1,		20,	0,0,0,	7,PNB_CUT_TYPE_FEASIBILITY,	PNB_COL_SET_DOUBLE_PRIME });
		cut_infos.push_back({ 2,	3,1,	   -30,	1,0,0,	7,PNB_CUT_TYPE_OPTIMALITY,	PNB_COL_SET_DOUBLE_PRIME });
		cut_infos.push_back({ 2,	2,2,		20,	1,1,1,	7,PNB_CUT_TYPE_EPIGRAPH,	PNB_COL_SET_NONE });
		cut_infos.push_back({ 2,	2,2,		20,	0,0,0,	7,PNB_CUT_TYPE_DOMAIN,		PNB_COL_SET_NONE });
		
		//It. 8 see above
		//It. 9 (partially consolidated cuts already in the iteration)
		cut_infos.push_back({ 4,	1,1,0,1,	10,	1,0,1,	9,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		cut_infos.push_back({ 4,	2,0,2,2,	20,	0,1,0,	9,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });

		//It. 10 (same aggregate in the iteration)
		// TODO: somehow test this errors without raising them
		//cut_infos.push_back({ 4,	1,1,1,1,	10,	0,0,1,	10,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		//cut_infos.push_back({ 4,	2,2,2,2,	20,	0,1,1,	10,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });

		//It. 11 (different num_cols to consolidate)
		// TODO: somehow test this errors without raising them
		//cut_infos.push_back({ 2,	1,1,		10,	0,0,1,	11,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		//cut_infos.push_back({ 3,	2,2,2,		20,	0,1,0,	11,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });

		BendersNodeData::cut_container cuts_in = BendersNodeData::cut_container();
		for (auto& cut_info : cut_infos) {
			std::shared_ptr<CutStuff> cut_stuff(CutStuffFromCutInfo(cut_info));
			//cut_stuff->Print();
			cuts_in.push_back(cut_stuff);
		}
		BendersNodeData::cut_container cuts_out = BendersNodeData::cut_container();

		BendersNodeData::cut_container::const_iterator& start_from = std::next(cuts_in.begin(), 2);
		const BendersNodeData::cut_container::const_iterator old_start_from = start_from;
		parallel_benders::ConsolidateCuts(cuts_in, cuts_out, start_from);
		EXPECT_TRUE(start_from == old_start_from) << "We do not want to change the iterator inside the function."; 

		std:deque<std::vector<double>>expected_results;
								 //col,	coef,		lb, theta,	it, type, col set
		expected_results.push_back({ 4,	6,6,6,6,	60,	1,1,1,	4,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		expected_results.push_back({ 4,	5,5,5,5,	50,	0,1,1,	5,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		expected_results.push_back({ 4,	1,1,1,1,	10,	1,0,0,	5,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_DOUBLE_PRIME });
		expected_results.push_back({ 4,	4,6,3,6,	50,	1,1,1,	6,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		expected_results.push_back({ 3,	2,2,1,		20,	0,0,0,	7,PNB_CUT_TYPE_FEASIBILITY,	PNB_COL_SET_DOUBLE_PRIME });
		expected_results.push_back({ 2,	3,1,	   -30,	1,0,0,	7,PNB_CUT_TYPE_OPTIMALITY,	PNB_COL_SET_DOUBLE_PRIME });
		expected_results.push_back({ 2,	2,2,		20,	1,1,1,	7,PNB_CUT_TYPE_EPIGRAPH,	PNB_COL_SET_DOUBLE_PRIME });
		expected_results.push_back({ 2,	2,2,		20,	0,0,0,	7,PNB_CUT_TYPE_DOMAIN,		PNB_COL_SET_DOUBLE_PRIME });
		expected_results.push_back({ 4,	3,0,3,1,	30,	0,1,1,	8,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_PRIME });
		expected_results.push_back({ 4,	3,1,2,3,	30,	1,1,1,	9,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });

		EXPECT_EQ(expected_results.size(), cuts_out.size());
		for (auto& exp_result : expected_results) {
			CutStuff* exp_cut_stuff = CutStuffFromCutInfo(exp_result);
			bool exists_in_results(false);
			for (auto& it : cuts_out) {
				//it->Print();
				if (CutIsEqual(*exp_cut_stuff, *it, 0.00001)) {
					exists_in_results = true;
					break;
				}
			}
			
			EXPECT_TRUE(exists_in_results);
			if (!exists_in_results) {
				LOG(WARNING) << "Expected cut is not found in results:\n";
				exp_cut_stuff->Print();
			}
			delete exp_cut_stuff;
		}

		//****** ConsolidateCuts cut_in == cut_out
		expected_results.push_front({ 4,	1,1,1,1,	10,	1,0,0,	3,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });
		expected_results.push_front({ 4,	2,2,2,2,	20,	0,1,0,	3,PNB_CUT_TYPE_OPTIMALITY,PNB_COL_SET_NONE });		

		parallel_benders::ConsolidateCuts(cuts_in, cuts_in, std::next(cuts_in.begin(), 2));

		EXPECT_EQ(expected_results.size(), cuts_in.size());
		for (auto& exp_result : expected_results) {
			CutStuff* exp_cut_stuff = CutStuffFromCutInfo(exp_result);
			bool exists_in_results(false);
			for (auto& it : cuts_in) {
				//it->Print();
				if (CutIsEqual(*exp_cut_stuff, *it, 0.00001)) {
					exists_in_results = true;
					break;
				}
			}

			EXPECT_TRUE(exists_in_results);
			if (!exists_in_results) {
				LOG(WARNING) << "Expected cut is not found in results:\n";
				exp_cut_stuff->Print();
			}
			delete exp_cut_stuff;
		}
	}
}

TEST_F(TimerTest, unitTest) {
	parallel_benders::Timer pnb_timer(4);
	pnb_timer.Start(0);
	sumApproxTimes(6, 6, 100000, true);
	pnb_timer.Stop(0);

	google::SetStderrLogging(google::INFO);
	pnb_timer.Start(1);
	sumApproxTimes(6, 6, 1, true);
	pnb_timer.Stop(1);

	for (int timer_st = 0; timer_st < pnb_timer.GetTimeStages(); timer_st++) {
		std::cout << "Elapsed PNB " << timer_st << ": " << pnb_timer.GetTime(timer_st) << std::endl;
	}
	std::cout << "Total PNB Timer: " << pnb_timer.GetAggregatedTime() << " Elapsed Thread Wall (msec): " << wall_thread_timer.load() << " Elapsed Thread PNB: " << pnb_thread_timer.GetTime(0) << std::endl;
}

TEST_F(CollectInstanceSpecs, all) {
	solveSMPSFromList("../../../../Data/Benders/InstanceListAll.txt");
	EXPECT_TRUE(true);
}

TEST_F(CollectInstanceSpecs, snip) {
	solveSMPSFromList("../../../../Data/Benders/InstanceListBodurSNIP.txt");
	EXPECT_TRUE(true);
}

TEST_F(CollectInstanceSpecs, rrtailassign) {
	solveSMPSFromList("../../../../Data/Benders/InstanceListMaherRrtailassign.txt");
	EXPECT_TRUE(true);
}

TEST_F(CollectInstanceSpecs, cap) {
	solveSMPSFromList("../../../../Data/Benders/InstanceListBodurCAP.txt");
	EXPECT_TRUE(true);
}

TEST_F(LiftAndProjectCutsTest, fast) {
	// Binary 1st stage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 300);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_CGP_EPSILON_RELATIVE, 0.01);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 0);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_0"), -15250.00, 0.1); // standard
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_1_25"), -148692, 0.3); // same characteristics as SNDP_TestCase_0
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_2"), -54250.00, 0.1); // 3 bin vars on 1st stage (in standard there are 2)
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_2"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_3"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_9"), -19300, 0.1); // relaxation solution == MIP
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_3"), -5000, 0.1); // cont in 1st stage
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_1"), -15000, 0.1); // domain cuts
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_4"), -15000, 0.1); // domain cuts
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_1"), -15000, 0.1); // domain cuts // relaxation solution == MIP
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_2"), -1000, 0.1); // domain cuts
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_12"), -15250, 0.1); // feasibility cuts
	
	// Integer 2 stages
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2); // relaxation for the 2nd stage, ERROR otherwise
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_7"), -1000, 0.1); // cont in 1st stage, MIP (bin+cont) 2nd
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_14"), -17600.4, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_16"), -15207.5, 0.1); // similar characteristics to siplib/sslp/sslp... and sslp-ntaimo
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_50"), -121.6, 0.1); // same characteristics as SNDP_TestCase_16, CGP PNB and dual cgp solutions are different (sometimes, but not always, non-deterministic)
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100c"), -117.37, 0.1); // same characteristics as SNDP_TestCase_16
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_17"), -985.0, 0.1); // similar characteristics to siplib/sizes/sizes...
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes3"), 224444.4, 0.1); // same characteristics as SNDP_TestCase_17, CGP MP is always infeasible due to some reason
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_200"), 882.615, 0.1); // 1834.87 for MIP
	EXPECT_NEAR(solveSMPS("han/dcap/dcap468_50"), 1401.06, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/smkp/smkp_11"), 9415.88, 0.1);
		// generate feasibility cuts
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_15"), -10750, 0.1); // feasibility cuts, similar characteristics to ssch/c...
}

TEST_F(LiftAndProjectCutsTest, DISABLED_bounds_convergence_fast) {
	// LiftAndProjectCutsTest.parameter_strings_ are not considered in this test case

	// Populate bounds_list_lp_{PNB_LP_ROUNDS_LIMIT}_major_{PNB_LP_MAJOR_IT_LIMIT}_switch_{PNB_LP_SWITCHES_LIMIT}_method_{PNB_LIFT_AND_P_METHOD}
	// TestCase_0
	// w/o L&P cuts
	std::vector<double> lb = { -1.0e+20,-41000.00,-17250.00,-16000.00,-15250.00 };
	std::vector<double> ub = {  1.0e+20,     0.00,-15250.00,-15250.00,-15250.00 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_0_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_0_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_0 = bounds_list_SNDP_TestCase_0_lp_0_major_1_switch_2_method_0;
	// DZN w/o 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_1 = bounds_list_SNDP_TestCase_0_lp_0_major_1_switch_2_method_0;
	// DZN w 2nd stage info
	 lb = { -1.0e+20,-41000.00,-16000.00,-15250.00 };
	ub = {  1.0e+20,     0.00,-15250.00,-15250.00 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_2;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_2.push_back(make_pair(ub[i], lb[i]));
	}

	// TestCase_2
	// w/o L&P cuts
	lb = { -1.0e+20,-104000.0,-56250.0,-54250.0 };
	ub = {  1.0e+20,      0.0,-52250.0,-53250.0 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_2_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_2_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	lb = { -1.0e+20,-104000.0,-56250.0,-54250.0 };
	ub = { 1.0e+20,       0.0,-52250.0,-53250.0 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w/o 2nd stage info
	lb = { -1.0e+20,-104000.0,-56250.0,-54250.0,-54250.0 };
	ub = { 1.0e+20,       0.0,-52250.0,-53250.0,-53500.0 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_1;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_1.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_2 = bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_0;

	// TestCase_9
	// w/o L&P cuts
	lb = { -1.0e+20,-41000.0,-19300.0 };
	ub = {  1.0e+20,     0.0,-19300.0 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_9_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_9_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	lb = { -1.0e+20,-41000.00,-19300.00 };
	ub = {  1.0e+20,     0.00,-19300.00 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w/o 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_1 = bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_0;
	// DZN w 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_2 = bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_0;

	// TestCase_14
	// w/o L&P cuts
	lb = { -1.0e+20,-41000.0,-17900.1,-17900.1,-17600.4 };
	ub = {  1.0e+20,     0.0,-15900.1,-15900.1,-17600.4 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_14_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_14_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_0 = bounds_list_SNDP_TestCase_14_lp_0_major_1_switch_2_method_0;
	// DZN w/o 2nd stage info
	lb = { -1.0e+20,-41000.0,-17900.1,-17900.1,-17600.4 };
	ub = { 1.0e+20,      0.0,-15900.1,-17600.4,-17600.4 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_1;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_1.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w 2nd stage info
	lb = { -1.0e+20,-41000.00,-17600.4 }; // TODO: values
	ub = { 1.0e+20,      0.00,-15900.0 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_2;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_2.push_back(make_pair(ub[i], lb[i]));
	}
	
	// SNDP_TestCase_15
	// w/o L&P cuts
	lb = { -1.0e+20,-1.0e+20,-12000.0,-10750.0 };
	ub = {  1.0e+20, 1.0e+20,-10750.0,-10750.0 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_15_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_15_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_15_lp_5_major_1_switch_2_method_0 = bounds_list_SNDP_TestCase_15_lp_0_major_1_switch_2_method_0;
	// DZN w/o 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_15_lp_5_major_1_switch_2_method_1 = bounds_list_SNDP_TestCase_15_lp_0_major_1_switch_2_method_0;
	// DZN w 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_15_lp_5_major_1_switch_2_method_2 = bounds_list_SNDP_TestCase_15_lp_0_major_1_switch_2_method_0;
		
	// SNDP_TestCase_16
	lb = { -1.0e+20,-41000.0,-17207.5,-15970.0,-15207.5 };
	ub = { 1.0e+20,      0.0,-15207.5,-15207.5,-15207.5 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_16_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_16_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_0 = bounds_list_SNDP_TestCase_16_lp_0_major_1_switch_2_method_0;
	// DZN w/o 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_1 = bounds_list_SNDP_TestCase_16_lp_0_major_1_switch_2_method_0;;
	// DZN w 2nd stage info
	lb = { -1.0e+20,-41000.00,-15970.0,-15207.5 };
	ub = { 1.0e+20,     0.00, -15207.5,-15207.5 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_2;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_2.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w 2nd stage info (not reuse but rebuild cgp)
	lb = { -1.0e+20,-41000.00,-15207.5 };
	ub = { 1.0e+20,     0.00, -15207.5 };
	std::vector<std::pair<double, double>> bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_2_rebuild;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_2_rebuild.push_back(make_pair(ub[i], lb[i]));
	}
	
	// sslp_5_25_50
	lb = { -1.0e+20,-886618.2,-215.38, -208.38,-195.38,-195.38,-187.38,-168.38,-155.38,-155.38,-148.38,-148.38,-147.38,-140.38,-135.38,-127.38,-127.38,-121.6 };
	ub = { 1.0e+20,   53106.8,  19.62,-30.8662,  -78.9,  -78.9,  -78.9,  -78.9, -121.6, -121.6, -121.6, -121.6, -121.6, -121.6, -121.6, -121.6, -121.6,-121.6 };
	std::vector<std::pair<double, double>> bounds_list_sslp_5_25_50_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_sslp_5_25_50_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	std::vector<std::pair<double, double>> bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_0 = bounds_list_sslp_5_25_50_lp_0_major_1_switch_2_method_0;
	// DZN w/o 2nd stage info
	std::vector<std::pair<double, double>> bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_1 = bounds_list_sslp_5_25_50_lp_0_major_1_switch_2_method_0;;
	// DZN w 2nd stage info
	lb = { -1.0e+20,-886618.16,-187.38,-168.38,-155.38,-155.38,-148.38,-148.38,-147.38,-140.38,-135.38,-127.38,-127.38,-121.59 };
	ub = {  1.0e+20,  53106.84,  19.62,  -69.3,-121.59,-121.59,-121.59,-121.59,-121.59,-121.59,-121.59,-121.59,-121.59,-121.59 };
	std::vector<std::pair<double, double>> bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_2;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_2.push_back(make_pair(ub[i], lb[i]));
	}

	benders_parameter_.SetParameterValue(parallel_benders::PNB_PRINT_SUBPROBLEMS, false);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ALLOW_TEST_CASES, 1100001100);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);

	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_MAJOR_IT_LIMIT, 1);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_SWITCHES_LIMIT, 2);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, 0); //w/o L&P cuts
		// solve problems
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_0", bounds_list_SNDP_TestCase_0_lp_0_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_9", bounds_list_SNDP_TestCase_9_lp_0_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_14", bounds_list_SNDP_TestCase_14_lp_0_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_15", bounds_list_SNDP_TestCase_15_lp_0_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("siplib/sslp/sslp_5_25_50", bounds_list_sslp_5_25_50_lp_0_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_2", bounds_list_SNDP_TestCase_2_lp_0_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_16", bounds_list_SNDP_TestCase_16_lp_0_major_1_switch_2_method_0));

	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, 5); // with L&P cuts
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LIFT_AND_P_METHOD, 0); // DAN (domain cut)
	// solve problems
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_0", bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_9", bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_14", bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_15", bounds_list_SNDP_TestCase_15_lp_5_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("siplib/sslp/sslp_5_25_50", bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_2", bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_0));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_16", bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_0));

	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, 5); // with L&P cuts
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LIFT_AND_P_METHOD, 1); // DZN (epigraph cut) w/o 2nd stage info
	// solve problems
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_0", bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_1));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_9", bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_1));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_14", bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_1));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_15", bounds_list_SNDP_TestCase_15_lp_5_major_1_switch_2_method_1));
	EXPECT_TRUE(checkConvergence("siplib/sslp/sslp_5_25_50", bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_1));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_2", bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_1));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_16", bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_1));

	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, 5); // with L&P cuts
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LIFT_AND_P_METHOD, 2); // // DZN (epigraph cut) with 2nd stage info
	// solve problems
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_0", bounds_list_SNDP_TestCase_0_lp_5_major_1_switch_2_method_2));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_9", bounds_list_SNDP_TestCase_9_lp_5_major_1_switch_2_method_2));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_14", bounds_list_SNDP_TestCase_14_lp_5_major_1_switch_2_method_2));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_15", bounds_list_SNDP_TestCase_15_lp_5_major_1_switch_2_method_2));
	EXPECT_TRUE(checkConvergence("siplib/sslp/sslp_5_25_50", bounds_list_sslp_5_25_50_lp_5_major_1_switch_2_method_2));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_2", bounds_list_SNDP_TestCase_2_lp_5_major_1_switch_2_method_2));
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_16", bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_2));
	// SNDP_TestCase_2 also behaved differently
	EXPECT_TRUE(checkConvergence("glushko/sndp_mini_tests/SNDP_TestCase_16", bounds_list_SNDP_TestCase_16_lp_5_major_1_switch_2_method_2_rebuild));
}

TEST_F(NestedBendersTest, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
	EXPECT_NEAR(solveSMPS("other/Fina4"), -68401.8, 0.1);
	EXPECT_NEAR(solveSMPS("other/GAS3"), 1558.17, 0.1);
	EXPECT_NEAR(solveSMPS("other/Dakota/dakota"), -24000, 1);
	EXPECT_NEAR(solveSMPS("other/Dakota/Dakota_min_x_constrained_simple"), -2320, 1); // 2 scen
	//EXPECT_NEAR(solveSMPS("posts/storm/stormG2_8"), 15535235.730, 15.0); // breaks for CC
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-1"), 413.4, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_10"), 269.591, 0.1);
}

TEST_F(NestedBendersTestInteger, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	// 1st stage only is integer or MIP
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_1_25"), -148692, 0.3);

	//sndp_mini_tests
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_0"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_2"), -54250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_3"), -5000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_1"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_2"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_3"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_4"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_1"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_2"), -1000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_7"), -1000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_9"), -19300, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_10"), 1, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_12"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_13"), -16000, 0.1);

	// 1st  stage is integer or MIP, 2nd has int vars
		// 1st stage is not pure binary so we cannot use D^2 algorithm
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_14"), -17600.4, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_15"), -10750, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_300"), 1583.66, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 223753.66, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50a"), -80.0, 0.1);
}


TEST_F(NestedBendersTestFeasibility, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3"), 722.58, 0.1); // Dependent // crashes for ODA
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-aggr"), 15963.9, 0.3); // Challenging CO2 limits // 20478.7
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-first"), 14794.6, 0.1); // INDEP
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_6"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76, crashes with CC
}

TEST_F(NestedBendersTestMultiStage, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3", "felt/stocfor/stocfor2_7"), -39024.4, 0.1); // seven-stage, stochasticity from stocfor2
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_32"), -2172.9, 0.2); // -2167.62
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_32"), -1866.42, 0.5);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_6"), -9.479354, 0.1);
	// generate feasibility cuts
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3"), 722.58, 0.1); // Dependent
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS2"), 719.21, 0.1); // Independent
}

TEST_F(CVaRTest, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
	EXPECT_NEAR(solveSMPS("other/Fina4"), -68401.8, 0.1);
	EXPECT_NEAR(solveSMPS("other/Dakota/dakota"), -24000, 1);
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"), 249383.34, 0.1); //BLOCK (dependent)
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"), 273553.19, 0.1);//INDEP (independent)
	EXPECT_NEAR(solveSMPS("felt/assets/assets_large"), -696.396, 0.1);
	EXPECT_NEAR(solveSMPS("felt/assets/assets_small"), -723.839, 0.1);
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor2"), -39772.4, 0.1); // two stage
	// many other instances are infeasible or have error return values when solved with CVaR methods
}

TEST_F(DetEqTest, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	// Continuous problems
	EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
	EXPECT_NEAR(solveSMPS("other/Fina4"), -68401.8, 0.1);
	EXPECT_NEAR(solveSMPS("other/GAS3"), 1558.17, 0.1);
	EXPECT_NEAR(solveSMPS("other/Dakota/dakota"), -24000, 1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-1"), 413.4, 0.1);
	EXPECT_NEAR(solveSMPS("posts/storm/stormG2_8"), 15535235.730, 0.1);
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-first"), 14794.6, 0.1); // INDEP
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_10"), 269.591, 0.1);

	// Integer 2 stages
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_50"), -121.6, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_200"), 1834.87, 0.4);
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes3"), 224434.32, 1758.0);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50a"), -121.6, 42.0);

	// Multi stage
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3"), 722.58, 0.1); // Dependent
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION, true);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION_START, 1); // from the 2nd stage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION_END, -1); // till the last stage
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3", "felt/stocfor/stocfor2_7"), -39024.4, 0.1); // seven-stage, stochasticity from stocfor2
}

TEST_F(RelaxationTest, fast) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_50"), -160.063, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_200"), 877.652, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes3"), 221867.85, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50a"), -139.39, 0.1);
}

TEST_F(NestedBendersTestInteger, sndp_mini_tests_GLUSHKO) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 120);

	//sndp_mini_tests
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_0"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_2"), -54250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_3"), -5000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_1"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_2"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_3"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_4"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_1"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_2"), -1000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_7"), -1000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_9"), -19300, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_10"), 1, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_12"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_13"), -16000, 0.1);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_14"), -17600.4, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_15"), -10750, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_16"), -15207.5, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_17"), -985.0, 0.1);
}

TEST_F(LiftAndProjectCutsTest, for_LP) {
	//have bin and sometimes cont vars on the 1st stage
	//need >0.2 secs to be solved on workstation with DETEQ
	//second stage integrality is relaxed since some instances have int on the 2nd stage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_EPSILON_RELATIVE, 0.000001);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 10800);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_WARM_START, parallel_benders::PNB_INT_WARM_START_SCENARIO_WISE);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_PARALLEL_MODE, parallel_benders::PNB_PARAL_FRACTS_AND_CGP_SUBPROBLEMS);

	benders_parameter_.SetParameterValue(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 0);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_USE_CGP_PNB_CUTS, parallel_benders::PNB_CGP_CUTS_NONE);

	//sndp_mini_tests
	/*EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_0"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_2"), -54250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_3"), -5000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_1"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_2"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_3"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_4_4"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_1"), -15000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_6_2"), -1000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_7"), -1000, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_9"), -19300, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_10"), 1, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_12"), -15250, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_14"), -17600.4, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_15"), -10750.0, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_16"), -15207.5, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp_mini_tests/SNDP_TestCase_17"), -985.0, 0.1);*/ // similar characteristics to siplib/sizes/sizes...

	//sndp
	//from paper
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_25"), -2.24214e+06, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_25"), -397621, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_125"), -398906, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_500"), -400387, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_125"), -2174732.8799, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_500"), -2.1727e+06, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_125"), -3.80379e+06, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_500"), -3.82522e+06, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_3_1_125"), -6612036.59199, 0.1);
	//EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_3_1_500"), -6678160, 0.1);
	
	//rahmaniani - complete recourse
	solveSMPSFromList("../../../../Data/Benders/InstanceList_rahmaniani_ForPaper.txt");

	//sslprf
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf10/sslprf10_100_100_2"), -85164.4, 0.1);
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf10/sslprf10_100_100_3"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf10/sslprf10_100_100_1"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf10/sslprf10_50_100_1"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf10/sslprf10_50_100_2"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf10/sslprf10_50_100_3"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_50_5_1"), -110403, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_100_2"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_100_3"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_10_1"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_10_2"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_10_3"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_100_1"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_25_2"), 0.0, 0.1); // what is the correct solution?
	//EXPECT_NEAR(solveSMPS("sslprf/sslprf15/sslprf15_100_50_3"), 0.0, 0.1); // what is the correct solution?

	//other
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_3_0_16_cr"), 25765.8, 0.1);
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_5_0_16_cr"), 25765.8, 0.1);
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/04_7_0_16_cr"), 25765.8, 0.1);
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/04_7_0_16_cr"), 25765.8, 0.1);
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r05_1_0_16_cr"), 25765.8, 0.1);
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r06_1_0_16_cr"), 25765.8, 0.1);
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0_16_cr"), 25765.8, 0.1);
	//TODO: more r instances here
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_1_0-2_64_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_1_0-2_32_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r08_1_0-2_64_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r08_1_0-2_32_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_2_64_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_3_0_64_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_5_0_64_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_7_0_64_cr"), 0.0, 0.1); // obj value?
	//EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r10_9_0_64_cr"), 0.0, 0.1); // obj value?
	
	//cap_BODUR
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_5"), -114935, 0.1); // what is the obj value for these instances?
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_5"), -114935, 0.1);

	benders_parameter_.SetParameterValue(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 1);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_USE_CGP_PNB_CUTS, parallel_benders::PNB_CGP_CUTS_ALL);

	//sslp_SIPLIB
	// from paper
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_100"), -354.190, 0.1);
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_1000"), -351.171, 0.1);
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_2000"), -347.262, 0.1);
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_50"), -364.640, 0.1);// Lower Bound does not converge to upper bound //Best lower: -365.438
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_500"), -349.136, 0.1);
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_5"), -265.569, 0.1);

	//dcap_SIPLIB
	// all need too few iterations

	//dcap_HAN
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap468_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap468_200"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap575_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap575_200"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap665_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap665_200"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap747_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/dcap/dcap747_200"), -12345.6, 1.0);

	//sizes_SIPLIB //
	// all are solved very fast for DETEQ

	//ssch_OTHER [fix bug with infeasibility]
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c1"), -184439, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c2"), 0, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c3"), -230268, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c4"), -201454, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c5"), 0, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c6"), -231369, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c7"), -137217, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c8"), -100523, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c10"), -154654, 1.0);

	//sslp_NTAIMO
	solveSMPSFromList("../../../../Data/Benders/InstanceList_sslp_ForPaper.txt");

	//sslp_HAN
	//[cannot solve them]
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_20_100_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_20_100_200"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_30_100_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_30_100_200"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_60_60_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_60_60_200"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_90_45_50"), -12345.6, 1.0);
	//EXPECT_NEAR(solveSMPS("han/sslp/sslp_90_45_200"), -12345.6, 1.0);

	//postponement
	//EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_100"), 0.0, 0.1);
	//EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_75"), 0.0, 0.1);
	
	//scdp 
	//they are multistage and take much time

	//rrtailassign_MAHER
	//us_rrtailassign instances have ints on the second stage
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_0", "maher/rrtailassign/qf_rrtailassign_0_10"), -114935, 0.1); // what are the correct sol values of these instances?
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_0", "maher/rrtailassign/qf_rrtailassign_0_100"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_0", "maher/rrtailassign/qf_rrtailassign_0_20"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_0", "maher/rrtailassign/qf_rrtailassign_0_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_0", "maher/rrtailassign/qf_rrtailassign_0_50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_1", "maher/rrtailassign/qf_rrtailassign_1_10"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_1", "maher/rrtailassign/qf_rrtailassign_1_100"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_1", "maher/rrtailassign/qf_rrtailassign_1_20"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_1", "maher/rrtailassign/qf_rrtailassign_1_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/qf_rrtailassign_1", "maher/rrtailassign/qf_rrtailassign_1_50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_0", "maher/rrtailassign/sw_rrtailassign_0_10"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_0", "maher/rrtailassign/sw_rrtailassign_0_100"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_0", "maher/rrtailassign/sw_rrtailassign_0_20"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_0", "maher/rrtailassign/sw_rrtailassign_0_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_0", "maher/rrtailassign/sw_rrtailassign_0_50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/sw_rrtailassign_1_10"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/sw_rrtailassign_1_100"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/sw_rrtailassign_1_20"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/sw_rrtailassign_1_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/sw_rrtailassign_1_50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/us_rrtailassign_0_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("maher/rrtailassign/sw_rrtailassign_1", "maher/rrtailassign/us_rrtailassign_1_5"), -114935, 0.1);

	//smkp_SIPLIB
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_1"), -114935, 0.1); // what are the correct sol values of these instances?
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_10"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_11"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_12"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_13"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_14"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_15"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_16"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_17"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_18"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_19"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_2"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_20"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_21"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_22"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_23"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_24"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_25"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_26"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_27"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_28"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_29"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_3"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_4"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_5"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_6"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_7"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_8"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("SIPLIB/smkp/smkp_9"), -114935, 0.1);

	//snip_BODUR
	benders_parameter_.SetParameterValue(parallel_benders::PNB_WARM_START, parallel_benders::PNB_INT_NO_WARM_START);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst0-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst1-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst2-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst3-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip3-inst4-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst0-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst1-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst2-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst3-budget90"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget30"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget40"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget50"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget60"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget70"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget80"), -114935, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/SNIP/snip4-inst4-budget90"), -114935, 0.1);
}

TEST_F(NestedBendersTestInteger, lsfrp_GLUSHKO) {
	EXPECT_NEAR(solveSMPS("glushko/lsfrp/LSFRP_instance10_stochastic"), -14382100, 0.1);
}

TEST_F(NestedBendersTestInteger, sndp_GLUSHKO) {
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_1_1"), -114935, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_1_25"), -148692, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_1_125"), -147442, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_1_500"), -149375, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_1"), -136196, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_25"), -136474, 0.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_125"), -137785, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_500"), -137664, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_1"), -333528, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_25"), -397621, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_125"), -398906, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_500"), -400387, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_1"), -485640, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_25"), -479326, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_125"), -487153, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_500"), -484816, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_1"), -2.32117e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_25"), -2.24214e+06, 2.9);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_125"), -2174732.8799, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_500"), -2.1727e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_1"), -4.03706e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_25"), -3.85151e+06, 5.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_125"), -3.80379e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_500"), -3.82522e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_3_1_1"), -6.83685e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_3_1_125"), -6612036.59199, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_3_1_500"), 0, 0.1); // never solved, what is obj value?
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_4_2_1"), -5.718e+06, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_640_4_2_25"), -5.6121e+06, 0.1);
}

TEST_F(NestedBendersTestInteger, cap) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 10);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_1000"), 0, 0.1); // never solved, what is obj value?
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_4000"), 0, 0.1);
	EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap101_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap102_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap103_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap111_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap112_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap113_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap114_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap121_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap122_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap123_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap124_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap131_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap132_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap133_5000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_1000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_2000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_250"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_3000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_4000"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_5"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_50"), 0, 0.1);
	//EXPECT_NEAR(solveSMPS("Bodur/CAP/cap134_5000"), 0, 0.1);
}

TEST_F(LiftAndProjectCutsTest, slow) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 300);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_CGP_EPSILON_RELATIVE, 0.01);
	EXPECT_NEAR(solveSMPS("glushko/sndp/SNDP_20_4_2_1"), -87272, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_25"), -136474, 0.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_25"), -397621, 0.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_25"), -479326, 0.5);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-6_16_cr"), 53891.6, 0.1); //slow

	// Integer on 2 stages
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_100"), -127.37, 0.1); // takes much time on Laptop
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50a"), -80.0, 0.5);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_100b"), -135.69, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_300"), 1583.66, 0.1); // w/o relaxation 2559.297
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 223753.66, 0.1); // w/o relaxation  224486.00, MP CGP is always infeasible (even when DZN is)
		// generate feasibility  cuts:
	EXPECT_NEAR(solveSMPS("ssch/ssch/c2"), 0, 1.0); // same characteristics as SNDP_TestCase_15
	EXPECT_NEAR(solveSMPS("ssch/ssch/c10"), -154654, 1.0);

	//scdp 
	//they are multistage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION, true);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION_START, 1); // from the 2nd stage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION_END, -1); // till the last stage
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-1024"), -6301005010, 10);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION, false);
}

TEST_F(LiftAndProjectCutsTest, DISABLED_bounds_convergence_slow) {
	// LiftAndProjectCutsTest.parameter_strings_ are not considered in this test case

	// Populate bounds_list_lp_{PNB_LP_ROUNDS_LIMIT}_major_{PNB_LP_MAJOR_IT_LIMIT}_switch_{PNB_LP_SWITCHES_LIMIT}_method_{PNB_LIFT_AND_P_METHOD}
	// ssch/ssch/c2
	// w/o L&P cuts
	std::vector<double> lb = { -1.0e+20,-742170.3,-463625.7,-244143.5,-243747.0,-243192.4,-242795.9, -211192.7, -208972.9, -206784.3,-30976.8,-8926.59,-7815.93,-7810.26,-7394.35,-7364.48,-7346.27,-7294.54,-6845.94,-306.726,-240.499,-128.529, 0 };
	std::vector<double> ub = { 1.0e+20,      0.0,      0.0,      0.0,      0.0,      0.0,      0.0,        0.0,      0.0,       0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0,     0.0, 0 };
	std::vector<std::pair<double, double>> bounds_list_c2_lp_0_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_c2_lp_0_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DAN (domain cut)
	lb = { -1.0e+20,-742170.3,-463625.67,-211173.72,-211192.66,-207010.28,-30977.59,-7810.25,-6845.94,5554.3 };
	ub = { 1.0e+20,     0.00,      0.00,      0.00,      0.00,      0.00,     0.00,    0.00,    0.00,  0.00 };
	std::vector<std::pair<double, double>> bounds_list_c2_lp_5_major_1_switch_2_method_0;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_c2_lp_5_major_1_switch_2_method_0.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w/o 2nd stage info
	lb = { -1.0e+20,-742170.3,-463606.1,-244143.5,-243747.0,-243192.4,-242795.9,-208972.9,-206777.9,-206674.0,-205676.2,-26451.7,-8926.6,-7815.9,-7394.35,-7364.47,-7346.27,-7294.71,-306.73,-240.5,-128.5, 0.0 };
	ub = { 1.0e+20,      0.0,      0.0,      0.0,      0.0,      0.0,      0.0,       0.0,     0.0,      0.0,      0.0,     0.0,    0.0,    0.0,     0.0,     0.0,     0.0,     0.0,    0.0,   0.0,   0.0, 0.0 };
	std::vector<std::pair<double, double>> bounds_list_c2_lp_5_major_1_switch_2_method_1;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_c2_lp_5_major_1_switch_2_method_1.push_back(make_pair(ub[i], lb[i]));
	}
	// DZN w 2nd stage info
	lb = { -1.0e+20,-742170.3,-463625.67,-224790.37,-223145.71,-147542.59,-147020.52,-130646.15,-130510.27,-8926.59,-7454.0,-7175.52,-259.7,-172.48, 0.0 };
	ub = { 1.0e+20,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,    0.0,    0.0,      0.0,   0.0,   0.00, 0.0 };
	std::vector<std::pair<double, double>> bounds_list_c2_lp_5_major_1_switch_2_method_2;
	for (int i = 0; i < ub.size(); i++) {
		bounds_list_c2_lp_5_major_1_switch_2_method_2.push_back(make_pair(ub[i], lb[i]));
	}

	benders_parameter_.SetParameterValue(parallel_benders::PNB_PRINT_SUBPROBLEMS, false);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ALLOW_TEST_CASES, 1100001100);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);

	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_MAJOR_IT_LIMIT, 1);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_SWITCHES_LIMIT, 2);
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, 0); //w/o L&P cuts
	// solve problems
	EXPECT_TRUE(checkConvergence("ssch/ssch/c2", bounds_list_c2_lp_0_major_1_switch_2_method_0));
	
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LP_ROUNDS_LIMIT, 5); // with L&P cuts
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LIFT_AND_P_METHOD, 0); // DAN (domain cut)
	// solve problems
	EXPECT_TRUE(checkConvergence("ssch/ssch/c2", bounds_list_c2_lp_5_major_1_switch_2_method_0));
	
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LIFT_AND_P_METHOD, 1); // DZN (epigraph cut) w/o 2nd stage info
	// solve problems
	EXPECT_TRUE(checkConvergence("ssch/ssch/c2", bounds_list_c2_lp_5_major_1_switch_2_method_1));
	
	benders_parameter_.SetParameterValue(parallel_benders::PNB_LIFT_AND_P_METHOD, 2); // // DZN (epigraph cut) with 2nd stage info
	// solve problems
	EXPECT_TRUE(checkConvergence("ssch/ssch/c2", bounds_list_c2_lp_5_major_1_switch_2_method_2));
}

TEST_F(NestedBendersTest, slow) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("posts/storm/stormG2_125"), 15512100.03, 5.0);
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"), 249101.672072, 0.1); //BLOCK (dependent)
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"), 269665.498390, 0.1);//INDEP (independent)
	EXPECT_NEAR(solveSMPS("felt/assets/assets_large"), -696.396, 0.1);
	EXPECT_NEAR(solveSMPS("felt/assets/assets_small"), -723.839, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-2"), 414.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-4"), 416.5, 0.1);
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor2"), -39772.4, 0.1); // two stage
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm", "wright/data-scenarios/storm", "wright/data-scenarios/storm-1000"), 1.54851e7, 1e3);
	EXPECT_NEAR(solveSMPS("frankellison/CEP1"), 355160, 0.1);
	EXPECT_NEAR(solveSMPS("frankellison/Ypgp2"), 447.324, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_10"), 173.304, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand0_2000"), 162.146, 0.1);
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3"), -39976.8, 0.1); // seven-stage
}

TEST_F(NestedBendersTestInteger, slow) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	// 1st stage only is integer or MIP
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_25"), -136474, 0.5); // sometimes similarity limit with PNB_USE_START_SOLUTION = 2
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_25"), -479326, 0.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_160_3_3_25"), -2.24214e+06, 2.9);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_320_4_2_25"), -3.85151e+06, 5.5);

	// 1st  stage is integer or MIP, 2nd has int vars
		// 1st stage is not pure binary so we cannot use D^2 algorithm
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_500"), 1306.60, 0.1); // w.o relaxation 2167.512
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 223753.66, 0.1);
	EXPECT_NEAR(solveSMPS("felt/phone/phone-int", "felt/phone/phone", "felt/phone/phone"), 36.9, 0.1); // Stochastic problem with 2^15 scenarios and integer
		// 1st stage is pure binary so we could use D^2 algorithm
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_100"), -127.37, 0.1);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c2"), 0, 1.0); // generates feasibility  cuts
	EXPECT_NEAR(solveSMPS("ssch/ssch/c6"), -253160.88, 1.0); // w/o relaxation -231369, generates feasibility  cuts // UB < LB for some of methods
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100a"), -91.03, 0.1); //   w/o relaxation -127.37
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100b"), -131.11, 0.1); // w/o relaxation -127.37
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100c"), -117.37, 0.1); // w/o relaxation -127.37
}

TEST_F(NestedBendersTestFeasibility, slow) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-5292"), 20788.6, 0.1); // 22438.4
	// 1st  stage is integer or MIP, 2nd has int vars
		// 1st stage is pure binary so we could use D^2 algorithm
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c2"), 0, 0.1);
}

TEST_F(DetEqTest, slow) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	// set of instances similar to LiftAndProjectCutsTest.slow
	// Continuous problems (L&P Cuts are not applicable)
	EXPECT_NEAR(solveSMPS("posts/storm/stormG2_125"), 15512100.03, 10.0);
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"), 249101.672072, 0.1); //BLOCK (dependent)
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"), 269665.498390, 0.1);//INDEP (independent)
	EXPECT_NEAR(solveSMPS("felt/assets/assets_large"), -696.396, 0.1);
	EXPECT_NEAR(solveSMPS("felt/assets/assets_small"), -723.839, 0.1);

	// Integer 2 stages
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_100"), -127.37, 0.1); // takes much time on Laptop
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_300"), 2559.297, 0.2);
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 224486.00, 1050.0); // very big difference to one provided by Christian
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100b"), -127.37, 3.7); // Solvable
	EXPECT_NEAR(solveSMPS("felt/chem/chem-int", "felt/chem/chem", "felt/chem/chem"), -8130, 0.1); // For mixed-integer case should be: -3300
		// generates feasibility  cuts
	EXPECT_NEAR(solveSMPS("ssch/ssch/c2"), 0, 1.0); // 0 for MIP

	// Multi stage
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS2"), 719.21, 0.1); // Independent
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_16"), -2400.1, 2.3);
}

TEST_F(RelaxationTest, slow) {
	benders_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 600);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_100"), -169.66, 0.1);  // 2nd stage MIP // -127.37
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_200"), 252.16, 0.1); // 1060.676
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_300"), 289.40, 0.1); // 1253.040
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 221173.57, 0.1); // 224486.00
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100a"), -143.67, 0.1); // Solvable // -127.37
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100b"), -169.18, 0.1); // Solvable // -127.37
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100c"), -165.29, 0.1); // Solvable // -127.37
	EXPECT_NEAR(solveSMPS("felt/chem/chem-int", "felt/chem/chem", "felt/chem/chem"), -13009.16, 0.1); // For mixed-integer case should be: -3300 // -8130
}

TEST_F(NestedBendersTest, DISABLED_InvestmentExampleTest) { // crashes, why?
	using namespace flopc;
	MP_model investmentModel(new OsiCpxSolverInterface());
	MP_data initialWealth, goal;
	initialWealth() = 55; //set data
	goal() = 90;

	enum { numStage = 3 };
	MP_stage T(numStage); //Time Stages
	enum { asset1, asset2, numAssets };
	MP_set assets(numAssets);

	//RandomVariable** random_vars = new RandomVariable*[3];
	RandomVariable* random_vars[numStage - 1][numAssets] =
	{
		{new DiscreteUniformRandomVariable(0,4,10), new DiscreteUniformRandomVariable(1,2,10)},
		{new DiscreteUniformRandomVariable(0,4,10), new DiscreteUniformRandomVariable(1,2,10)}
	};

	MP_random_data returns(&random_vars[0][0], T, assets);

	MP_variable x(T, assets);
	MP_variable wealth(T);
	MP_variable shortage(T), overage(T);

	MP_constraint
		initialWealthConstr,
		returnConstr(T),
		allocationConstr(T),
		goalConstr(T);

	initialWealthConstr.setName("initial");
	returnConstr.setName("return");
	allocationConstr.setName("allocation");
	goalConstr.setName("goal");

	initialWealthConstr() = sum(assets, x(0, assets)) == initialWealth();
	allocationConstr(T) = sum(assets, x(T, assets)) == wealth(T);
	returnConstr(T + 1) = sum(assets, returns(T, assets) * x(T - 1, assets)) == wealth(T);

	goalConstr(T + 2) = wealth(T) == goal() + overage(T) - shortage(T);
	MP_expression valueFunction(+1.3 * shortage(T.last()) - 1.1 * overage(T.last()));

	investmentModel.setObjective(valueFunction);
	investmentModel.attach(investmentModel.Solver);
	investmentModel.getSmi()->writeSmps("investment", true, false);
	investmentModel.solve(MP_model::MINIMIZE);

	OsiCpxSolverInterface osi_solver;
	SmiScnModel* smi = investmentModel.getSmi();
	OsiStochasticParallelNestedBendersSolverInterface nested_solver(&osi_solver, smi);
	nested_solver.initialSolve();


	////Compare solution values.
	EXPECT_NEAR(4.142141441, investmentModel.Solver->getObjValue(), 0.000001);
	EXPECT_EQ(MP_model::OPTIMAL, investmentModel.getStatus());

}

TEST_F(NestedBendersTest, DISABLED_BirgeLouveuxFarmersTest) {  // crashes, why?
	using namespace flopc; // Do not do this ;)
	// define the model as presented in the book, p. 237ff.
	MP_model testModel(new OsiCpxSolverInterface());

	enum { numT = 3 };
	MP_stage T(numT);

	enum { numScen = 4 };
	MP_scenario_set scenSet(numScen);

	double valD[numT - 1][numScen] =
	{
		{1, 1, 3, 3},
		{1, 3, 1, 3}
	};
	MP_random_data d(&valD[0][0], T);

	MP_variable x(T), w(T), y(T);

	MP_constraint time(T);
	time(T) = x(T) <= 2;

	MP_constraint demandSt1;
	demandSt1() = x(0) + w(0) - y(0) == 1;

	//MP_set T1(2);
	MP_constraint demand(T);
	demand(T + 1) = y(T) + x(T + 1) + w(T + 1) - y(T + 1) == d(T + 1);

	testModel.setObjective(
		x(0) + 3 * w(0) + 0.5 * y(0)
		+ x(1) + 3 * w(1) + 0.5 * y(1)
		+ x(2) + 3 * w(2)
	);

	testModel.attach(); // builds the internal model

	// the model is build, so we can continue with our NestedBenders
	//NestedBenders nb(testModel.getSmi());
	//nb.solve();
	//assert(nb.getSolutionValue() == 6.25);
	parallel_benders::ParallelNestedBenders pnb(testModel.getSmi());
	pnb.Solve();
}

TEST_F(NestedBendersTest, farmer_OTHER) {
	EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
}

TEST_F(NestedBendersTest, finance_OTHER) {
	EXPECT_NEAR(solveSMPS("other/Fina4"), -68401.8, 0.1);
}

TEST_F(NestedBendersTest, gas_OTHER) {
	EXPECT_NEAR(solveSMPS("other/GAS3"), 1558.17, 0.1);
}

TEST_F(NestedBendersTest, dakota_OTHER) {
	EXPECT_NEAR(solveSMPS("other/Dakota/dakota"), -24000, 1);
}

TEST_F(NestedBendersTest, sslp_OTHER) { // Contains "Only" Binary First Stage, modification of SIPLIB instance
	benders_parameter_.parameter_.put(parallel_benders::PNB_BOUNDS_IN_MATRIX, true);
	EXPECT_NEAR(solveSMPS("other/sslp_10_50_50"), -365.438, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_10_50_100"), -354.868, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_10_50_500"), -349.916, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_10_50_1000"), -352.494, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_10_50_2000"), -348.092, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_15_45_5"), -265.569, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_15_45_10"), -261.905, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_15_45_15"), -254.708, 0.1);
	EXPECT_NEAR(solveSMPS("special_cases/dsquaretest"), -37.5, 0.1);
}

TEST_F(NestedBendersTestInteger, sslp_SIPLIB) { // binary first, mixed-binary second stage
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_50"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_5_25_100"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_50"), -364.640, 0.1);// Lower Bound does not converge to upper bound //Best lower: -365.438
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_100"), -354.190, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_500"), -349.136, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_1000"), -351.171, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_2000"), -347.262, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_5"), -262.400, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_10"), -260.500, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_15"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_20"), -260.500, 0.1); // crashes
	//EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_25"), -253.602, 0.1); // not present in the original test set and is empty in Christian version
}

TEST_F(NestedBendersTestInteger, sslp_HAN) { // binary first, mixed-binary second stage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_20_100_50"), -121.6, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_20_100_200"), -127.37, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_30_100_50"), -364.640, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_30_100_200"), -354.190, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_60_60_50"), -349.136, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_60_60_200"), -351.171, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_90_45_50"), -347.262, 0.1);
	EXPECT_NEAR(solveSMPS("han/sslp/sslp_90_45_200"), -262.400, 0.1);
}

TEST_F(NestedBendersTestInteger, dcap_SIPLIP) { // Contains integer, two-stage
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_200"), 1834.868, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_300"), 1644.342, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_500"), 1737.719, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_200"), 2322.721, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_300"), 2559.297, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_500"), 2167.512, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_200"), 1060.676, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_300"), 1253.040, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_500"), 1588.717, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap342_200"), 1619.531, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap342_300"), 2067.777, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/dcap/dcap342_500"), 1904.722, 0.1);
}

TEST_F(NestedBendersTestInteger, dcap_HAN) { // Contains integer, two-stage
	benders_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap468_50"), 1834.868, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap468_200"), 1644.342, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap575_50"), 1737.719, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap575_200"), 2322.721, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap665_50"), 2559.297, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap665_200"), 2167.512, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap747_50"), 1060.676, 0.1);
	EXPECT_NEAR(solveSMPS("han/dcap/dcap747_200"), 1253.040, 0.1);
}

TEST_F(NestedBendersTestInteger, semi_SIPLIP) { // Needs feasibility cuts, contains integer on first stage, two-stage
	EXPECT_NEAR(solveSMPS("siplib/semi/semi2"), 1545, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/semi/semi3"), 224486.00, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/semi/semi4"), 224564.3, 0.1);
}

TEST_F(NestedBendersTestInteger, sizes_SIPLIB) { // Contains Integer on both stages, two-stage
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes3"), 224434.32, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 224486.00, 0.1);
	EXPECT_NEAR(solveSMPS("siplib/sizes/sizes10"), 224564.3, 0.1);
}

TEST_F(NestedBendersTestInteger, ssch_OTHER) { // Contains Integer on both stages, two-stage
	// generates feasibility  cuts
	EXPECT_NEAR(solveSMPS("ssch/ssch/c1"), -184439, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c2"), 0, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c3"), -230268, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c4"), -201454, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c5"), 0, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c6"), -231369, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c7"), -137217, 1.0);
	EXPECT_NEAR(solveSMPS("ssch/ssch/c8"), -100523, 1.0);
	//EXPECT_NEAR(solveSMPS("ssch/ssch/c9"), 0, 1.0); // crashes
	EXPECT_NEAR(solveSMPS("ssch/ssch/c10"), -154654, 1.0);
}

TEST_F(NestedBendersTestInteger, sslp_NTAIMO) { // binary first, mixed-binary second stage
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50a"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50b"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50c"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50d"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_50e"), -121.6, 0.1); // Solvable

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100a"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100b"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100c"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100d"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_25_100e"), -127.37, 0.1); // Solvable

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_50a"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_50b"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_50c"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_50d"), -121.6, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_50e"), -121.6, 0.1); // Solvable

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_100a"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_100b"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_100c"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_100d"), -127.37, 0.1); // Solvable
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_5_50_100e"), -127.37, 0.1); // Solvable

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_50a"), -364.640, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_50b"), -364.640, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_50c"), -364.640, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_50d"), -364.640, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_50e"), -364.640, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_100a"), -354.190, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_100b"), -354.190, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_100c"), -354.190, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_100d"), -354.190, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_100e"), -354.190, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_500a"), -349.136, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_500b"), -349.136, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_500c"), -349.136, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_500d"), -349.136, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_500e"), -349.136, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_1000a"), -351.171, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_1000b"), -351.171, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_1000c"), -351.171, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_1000d"), -351.171, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_1000e"), -351.171, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_2000a"), -347.262, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_2000b"), -347.262, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_2000c"), -347.262, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_2000d"), -347.262, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_10_50_2000e"), -347.262, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_5a"), -262.400, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_5b"), -262.400, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_5c"), -262.400, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_5d"), -262.400, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_5e"), -262.400, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_10a"), -260.500, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_10b"), -260.500, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_10c"), -260.500, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_10d"), -260.500, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_10e"), -260.500, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_15a"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_15b"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_15c"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_15d"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_15e"), -253.602, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_20a"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_20b"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_20c"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_20d"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_20e"), -253.602, 0.1);

	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_25a"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_25b"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_25c"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_25d"), -253.602, 0.1);
	EXPECT_NEAR(solveSMPS("ntaimo/sslp-ntaimo/sslp_15_45_25e"), -253.602, 0.1);
}

// Posts test set: http://users.iems.northwestern.edu/~jrbirge/html/dholmes/post.html
TEST_F(NestedBendersTestMultiStage, scfxm_POSTS) { // Generates feasibility cuts, INDEP
	// generates feasibility cuts
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_6"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_16"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_6"), 18615.4, 0.1);// Model probability problems 18615.932
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_16"), 18439, 0.1); // 18438.891
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_6"), 18615.4, 0.1); // 18616.224
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_16"), 18439, 0.1); // 18438.891
}

// Alternative: bonds_FELT
TEST_F(NestedBendersTestMultiStage, sgpf_POSTS) { // Verified with original values
	EXPECT_NEAR(solveSMPS("posts/sg/posts_sgpf5y3"), -3027.706, 0.1);
	EXPECT_NEAR(solveSMPS("posts/sg/posts_sgpf5y4"), -4031.391, 0.1);
	EXPECT_NEAR(solveSMPS("posts/sg/posts_sgpf5y5"), -5201.282, 0.1);
	EXPECT_NEAR(solveSMPS("posts/sg/posts_sgpf5y6"), -6484.47, 0.1);// -6403.3 original result, but it is verified by MSLiP via Neos
}

TEST_F(NestedBendersTest, stormG2_POSTS) {
	double solutionValue = 0;
	double referenceValue = 0;
	double relGap = 0;
	solutionValue = solveSMPS("posts/storm/stormG2_8");
	referenceValue = 15535235.730;
	relGap = abs(referenceValue - solutionValue) / solutionValue;
	EXPECT_LE(relGap, 0.000001);

	solutionValue = solveSMPS("posts/storm/stormG2_27");
	referenceValue = 15508982.306;
	relGap = abs(referenceValue - solutionValue) / solutionValue;
	EXPECT_LE(relGap, 0.000001);

	solutionValue = solveSMPS("posts/storm/stormG2_125");
	referenceValue = 15512091.185;
	relGap = abs(referenceValue - solutionValue) / solutionValue;
	EXPECT_LE(relGap, 0.000001);

	solutionValue = solveSMPS("posts/storm/stormG2_1000");
	referenceValue = 15802589.698;
	relGap = abs(referenceValue - solutionValue) / solutionValue;
	EXPECT_LE(relGap, 0.000001);
}

// Andrew Felt Test Set: http://www.uwsp.edu/math/afelt/slptestset.html
TEST_F(NestedBendersTest, airlift_FELT) {// Two-Stage
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"), 249101.672072, 0.1); //BLOCK (dependent)
	EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"), 269665.498390, 0.1);//INDEP (independent)
}

TEST_F(NestedBendersTest, assets_OTHER) {// Two-Stage
	EXPECT_NEAR(solveSMPS("felt/assets/assets_large"), -696.396, 0.1);
	EXPECT_NEAR(solveSMPS("felt/assets/assets_small"), -723.839, 0.1);
}

TEST_F(NestedBendersTest, assets_FELT) {// Two-Stage
	EXPECT_NEAR(solveSMPS("felt/assets/assets", "felt/assets/assets", "felt/assets/assets-small"), -723.839, 0.1);
	EXPECT_NEAR(solveSMPS("felt/assets/assets", "felt/assets/assets", "felt/assets/assets-large"), -695.962, 0.1);
}

TEST_F(NestedBendersTestMultiStage, bonds_FELT) { //SCENARIOS, Values that are tested against: results from MSLIB-OSL, values commented out: presented in felts testset
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y3"), -2967.9, 0.1); // 2976.91
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y4"), -3994.179, 0.1); // 3991.3
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y5"), -5172.07, 0.1); // 5125.6
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y6"), -6463.240, 0.1); // 6369.0
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y7"), -7947.863, 0.1);
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y3"), -3027.6, 0.1); // 3027.6
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y4"), -4031.303, 0.1); // 4023.9
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y5"), -5201.197, 0.1); // 5180.8
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y6"), -6484.465, 0.1); // 6403.3
	EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y7"), -7966.94, 0.1); // MSLIB-OSL = 7956.896
	// Cplex barrier has problems with these problems => wrong results
}

TEST_F(NestedBendersTest, cargo_FELT) { //Two-Stage, INDEP, values verified with det-eq
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-1"), 413.4, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-2"), 414.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-4"), 416.5, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-8"), 418.5, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-16"), 423.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-32"), 423.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-64"), 423.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-128"), 423.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-256"), 425.38, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-512"), 429.96, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-1024"), 434.1, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-2048"), 441.7, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-4096"), 446.86, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-8192"), 446.86, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-16384"), 446.86, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-32768"), 446.86, 0.1);
}

TEST_F(NestedBendersTestFeasibility, chem_FELT) { //BLOCKS, two-stage LP-relaxation of mixed integer problem, verified with det-eq
	EXPECT_NEAR(solveSMPS("felt/chem/chem"), -13009.166667, 0.1);
}

TEST_F(NestedBendersTestInteger, chem_FELT) { //BLOCKS, two-stage LP-relaxation of mixed integer problem, verified with det-eq
	// Mixed-integer problem is available here: has integers on second stage also, but we only solve first-stage integers. Generates feasibility cuts
	EXPECT_NEAR(solveSMPS("felt/chem/chem-int", "felt/chem/chem", "felt/chem/chem"), -8130, 0.1); // For mixed-integer case should be: -3300
}

TEST_F(NestedBendersTest, electric_FELT) { //SCENARIOS 
	EXPECT_NEAR(solveSMPS("felt/electric/LandS"), 381.853, 0.1); // Dependent
}

TEST_F(NestedBendersTestMultiStage, electric_FELT) { //SCENARIOS, three-stage, No staircase problem. verified
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3"), 722.58, 0.1); // Dependent
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS2"), 719.21, 0.1); // Independent 
}

TEST_F(NestedBendersTestFeasibility, environ_FELT) {//two-stage, BLOCKS and/or INDEP //values verified with det-eq, but differ from andy felts test set ( maybe indep/blocks mixture is incorrect??
	//EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-loose"), 14794.6, 0.1); // Non-challenging CO2 limits //  19777.4
	//EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-aggr"), 15963.9, 0.1); // Challenging CO2 limits // 20478.7
	//EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-imp"), 20773.9, 0.1); // Import: Aggresive and Import prices adjusting, INDEP and BLOCKS //  22265.3, not solvable due to feas cuts not improving solution..
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-lrge"), 20799.3, 0.1); //Based on Import, INDEP and BLOCKS // 22438.9
	//EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-xlrge"), 20799.3, 0.1); // Even Larger, INDEP and BLOCKS // 22438.9 //Cannot solve on workstation - the debugger breaks
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-first"), 14794.6, 0.1); // INDEP
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-1200"), 20808.6, 0.1); // INDEP and BLOCKS for the rest  22428.9
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-1875"), 20809.3, 0.1); //  22447.1
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-3780"), 20794.7, 0.1); //  22441
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-5292"), 20788.6, 0.1); // 22438.4
}

TEST_F(NestedBendersTestInteger, phone_FELT) {//INDEP, two-stage, mixed-integer on both stages, values not verified
	EXPECT_NEAR(solveSMPS("felt/phone/phone"), 36.9, 0.1); // Stochastic problem: 2^15 scenarios
	EXPECT_NEAR(solveSMPS("felt/phone/phone", "felt/phone/phone1"), 36.9, 0.1); // No stochasticity involved: 36.9
	EXPECT_NEAR(solveSMPS("felt/phone/phone-int", "felt/phone/phone", "felt/phone/phone"), 36.9, 0.1); // Stochastic problem with 2^15 scenarios and integer
	EXPECT_NEAR(solveSMPS("felt/phone/phone-int", "felt/phone/phone", "felt/phone/phone1"), 36.9, 0.1); // No stochasticity involved: 36.9, but integers
}

TEST_F(NestedBendersTest, stocfor_FELT) {// BLOCKS, values are within plausible ranges, verified with det-eq
	//EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor1"), -41131.98, 0.1); //two-stage
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor2"), -39772.4, 0.1); // two stage
}

TEST_F(NestedBendersTestMultiStage, stocfor_FELT) {// BLOCKS, values are within plausible ranges, verified with det-eq
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3"), -39976.8, 0.1); // seven-stage
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3", "felt/stocfor/stocfor2_7"), -39024.4, 0.1); // seven-stage, stochasticity from stocfor2
}

// Huge problems
TEST_F(NestedBendersTestSampleAverageApproximation, 20term_WRIGHT) {// 1.1 * 10^12 scenarios
	EXPECT_NEAR(solveSMPS("wright/data-20/20"), 0, 0.1); //two-stage
}
TEST_F(NestedBendersTestSampleAverageApproximation, gbd_WRIGHT) {// 6.5*10^5 , 646.425 scenarios
	EXPECT_NEAR(solveSMPS("wright/data-gbd/gbd"), 1655.628, 0.1); //two-stage
}
TEST_F(NestedBendersTestSampleAverageApproximation, ssn_WRIGHT) {// 10^70 scenarios
	EXPECT_NEAR(solveSMPS("wright/data-ssn/ssn"), 0, 0.1); //two-stage
}
TEST_F(NestedBendersTestSampleAverageApproximation, LandS_WRIGHT) {// 10^6 scenarios
	EXPECT_NEAR(solveSMPS("wright/data-LandS/LandS"), 225.629, 0.1); //two-stage
}
TEST_F(NestedBendersTestSampleAverageApproximation, storm_WRIGHT) {// 6*10^81 scenarios
	EXPECT_NEAR(solveSMPS("wright/data-storm/storm"), 0, 0.1); //two-stage
}

TEST_F(NestedBendersTest, scenarios_WRIGHT) { // TODO: Update the scenario values here!
	assert(false && "see TODOs above");
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/20", "wright/data-scenarios/20", "wright/data-scenarios/20-1000"), 254536, 1);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/20", "wright/data-scenarios/20", "wright/data-scenarios/20-2000"), 254019, 1);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/20", "wright/data-scenarios/20", "wright/data-scenarios/20-3000"), 254240, 1);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn", "wright/data-scenarios/ssn", "wright/data-scenarios/ssn-1000"), 8.78997, 0.1);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn", "wright/data-scenarios/ssn", "wright/data-scenarios/ssn-2000"), 9.24269, 0.1);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn", "wright/data-scenarios/ssn", "wright/data-scenarios/ssn-3000"), 9.24629, 0.1);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm", "wright/data-scenarios/storm", "wright/data-scenarios/storm-1000"), 1.54851e7, 1e3);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm", "wright/data-scenarios/storm", "wright/data-scenarios/storm-2000"), 1.54942e7, 1e3);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm", "wright/data-scenarios/storm", "wright/data-scenarios/storm-3000"), 1.54932e7, 1e3);
	use_multi_cut_ = false;
	EXPECT_NEAR(solveSMPS("wright/data-LandS/LandS"), 225.629, 0.1); //two-stage
	EXPECT_NEAR(solveSMPS("wright/data-gbd/gbd"), 1655.628, 0.1); //two-stage
}

TEST_F(NestedBendersTestMultiStage, watson_conditional) { // Values from own solver, old values are in comments (MSLiP), differ with det-eq
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_16"), -2400.1, 0.1); // -2401.87
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_32"), -2172.9, 0.1); // -2167.62
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_64"), -2310.7, 0.1); //-2027.43
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_128"), -1888.3, 0.1); // -1879.43
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_256"), -1849.4, 0.1); // -1841.29
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_512"), -1797.3, 0.1); // -1787.19
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_768"), -1795.16, 0.1); // -1780.13
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1024"), -1798.4, 0.1); // -1791.18
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1152"), -1723.7, 0.1); // -1725.53
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1536"), -1705.8, 0.1);// -1693.47
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1920"), -1778.3, 0.1); // -1769.45
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_2304"), -1674.9, 0.1);// -1664.66
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_2688"), -1687.7, 0.1);// -1679.92
}

TEST_F(NestedBendersTestMultiStage, watson_independent) { // Values from own solver, old values are in comments (MSLiP), differ with det-eq
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_16"), -2158.75, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_32"), -1866.42, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_64"), -2310.53, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_128"), -1637.81, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_256"), -2000.84, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_512"), -1959.64, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_768"), -1813.11, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1024"), -1926.79, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1152"), -1687.92, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1536"), -1790.27, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1920"), -1778.95, 0.1);
}

TEST_F(NestedBendersTest, unbounded_infeasible) {// BLOCKS, values not verified
	EXPECT_NEAR(solveSMPS("special_cases/unbounded/BenF4"), -149, 0.1); // Primal master problem gets unbounded, if one does not restrict value of theta variable.
	EXPECT_NEAR(solveSMPS("special_cases/infeasible/BENF4INF"), -1e20, 0.1); // is infeasible, feasibility cuts
	EXPECT_NEAR(solveSMPS("special_cases/unbounded/unbound"), -3, 0.1); // is not really unbounded
	EXPECT_NEAR(solveSMPS("special_cases/unbounded/unbound_fake"), -3, 0.1); // not readable
}

TEST_F(NestedBendersTest, saphir) {// two-stage, Feasibility cuts
	EXPECT_NEAR(solveSMPS("saphir/saphir_50"), 524047000, 10000);
	EXPECT_NEAR(solveSMPS("saphir/saphir_100"), 524417000, 10000);
	//EXPECT_NEAR(solveSMPS("saphir/saphir_200"), 596063000, 10000); // Hard to solve, no solution with benders!
	EXPECT_NEAR(solveSMPS("saphir/saphir_500"), 482931000, 10000);
	EXPECT_NEAR(solveSMPS("saphir/saphir_1000"), 305918000, 10000);
}

TEST_F(NestedBendersTestMultiStage, scdp) {// scdp problem from marc
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-1024"), -6301005010, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-4096"), -5204538801, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-16384"), -5183755359, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-65536"), -5183755359, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-64000"), -3860456379, 10);
	assert(false && "see TODOs below");
	EXPECT_NEAR(solveSMPS("scdp/scdp/scdp-64000-reduced"), 0.0, 0.1); // TODO: what is the solution?
	EXPECT_NEAR(solveSMPS("scdp/scdp/scdp-8000-reduced"), 0.0, 0.1); // TODO: what is the solution?
}

TEST_F(NestedBendersTestInteger, scdp) {// scdp problem from marc
	// they are multistage!!!
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-1024"), -6301005010, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-4096"), -5204538801, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-16384"), -5183755359, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-65536"), -5183755359, 10);
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-64000"), -3860456379, 10);
	assert(false && "see TODOs below");
	EXPECT_NEAR(solveSMPS("scdp/scdp/scdp-64000-reduced"), 0.0, 0.1); // TODO: what is the solution?
	EXPECT_NEAR(solveSMPS("scdp/scdp/scdp-8000-reduced"), 0.0, 0.1); // TODO: what is the solution?
}

TEST_F(NestedBendersTestMultiStage, pltexp_POSTS) { // multi-stage, A#Stage_#ScenPerStage, BLOCKS, values for problem A5_16 and above not verified.
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_6"), -9.479354, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_16"), -9.663308, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_6"), -13.969368, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_16"), -14.267458, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA4_6"), -19.599417, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA4_16"), -18.849337, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_6"), -23.214073, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_16"), -23.8, 0.1);
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA6_6"), -28.134408, 0.1);//
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA6_16"), -28.134408, 0.1); // run this only if necessary!
	EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_6"), -32.2779, 0.1);
	//EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_16"), -28.134408, 0.1); // needs a huge amount of memeory to load via  via smi_model->readSmps(). Machine with 24 Gb RAM cannot handle it
}

TEST_F(NestedBendersTest, frankellison) {// models received from Frank Ellison
	EXPECT_NEAR(solveSMPS("frankellison/CEP1"), 355160, 0.1);
	EXPECT_NEAR(solveSMPS("frankellison/Ypgp2"), 447.324, 0.1);
}
////
////TEST_F(NestedBendersTest,bike_sharing){// bike sharing model 
////	//benders_parameter_.parameter_.put(parallel_benders::PNB_HANDLE_FEAS_CUTS,parallel_benders::PNB_INT_FEAS_CUT_ALL);
////    EXPECT_NEAR(solveSMPS("bikeSharing/bikeSharing-100"), 2.693e6, 1e3); 
////    EXPECT_NEAR(solveSMPS("bikeSharing/bikeSharing-523"),3.4349e6, 1e3); 
////    EXPECT_NEAR(solveSMPS("bikeSharing/bikeSharing-731"),3.67583e6, 1e3); 
////}
//
////TEST_F(NestedBendersTest,StageAggregTest){// scdp problem from marc
////    EXPECT_NEAR(solveSMPS("selfTests/stageAggregTest"),14.0536, 1); 
////}
//
//
TEST_F(NestedBendersTest, deak) {
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_10"), 269.591, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_20"), 270.066, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_30"), 267.782, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_40"), 272.01, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_50"), 275.013, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_80"), 249.913, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_100"), 248.993, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_150"), 271.389, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_200"), 269.237, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_250"), 259.023, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_300"), 266.152, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_350"), 271.016, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_400"), 264.187, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_450"), 266.84, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_500"), 268.758, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_10"), 2915.45, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_20"), 2973.02, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_30"), 2966.85, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_40"), 3140.46, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_50"), 3198.58, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_80"), 3174.77, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_100"), 3062, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_150"), 3095.46, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_200"), 3046.05, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_250"), 3193.95, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_300"), 3057.37, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_350"), 3105.16, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_400"), 3114.22, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_450"), 3076.74, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x40-1", "deak/20x40-1", "deak/20x40-1_500"), 3085.35, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_10"), 311.199, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_20"), 308.059, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_30"), 309.22, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_40"), 301.528, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_50"), 306.684, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_80"), 304.493, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_100"), 305.396, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_150"), 302.526, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_200"), 301.408, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_250"), 303.718, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_300"), 307.845, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_350"), 303.869, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_400"), 304.871, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_450"), 305.072, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x60-1", "deak/20x60-1", "deak/20x60-1_500"), 302.273, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_10"), 173.304, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_20"), 195.016, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_30"), 194.145, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_40"), 186.974, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_50"), 191.179, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_80"), 186.836, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_100"), 193.674, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_150"), 193.669, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_200"), 192.891, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_250"), 194.447, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_300"), 197.986, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_350"), 192.896, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_400"), 195.64, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_450"), 193.758, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x20-1", "deak/40x20-1", "deak/40x20-1_500"), 194.464, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_10"), 975.758, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_20"), 1277.95, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_30"), 1208.43, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_40"), 990.32, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_50"), 1072.31, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_80"), 1171.58, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_100"), 1090.59, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_150"), 1236.16, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_200"), 1147.04, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_250"), 1100.9, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_300"), 1001.63, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_350"), 1101.28, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_400"), 1146.02, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_450"), 1117.79, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x40-1", "deak/40x40-1", "deak/40x40-1_500"), 1121.19, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_10"), 332.185, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_20"), 346.548, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_30"), 340.048, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_40"), 349.588, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_50"), 337.535, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_80"), 347.343, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_100"), 338.125, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_150"), 339.768, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_200"), 339.331, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_250"), 343.299, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_300"), 341.859, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_350"), 341.143, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_400"), 342.001, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_450"), 342.831, 0.1);
	EXPECT_NEAR(solveSMPS("deak/40x60-1", "deak/40x60-1", "deak/40x60-1_500"), 343.329, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_10"), 54.4003, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_20"), 50.8637, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_30"), 51.0978, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_40"), 54.346, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_50"), 54.733, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_80"), 54.1255, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_100"), 52.2184, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_150"), 53.0077, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_200"), 53.8554, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_250"), 55.1981, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_300"), 54.246, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_350"), 54.4522, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_400"), 53.9747, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_450"), 53.8843, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x20-1", "deak/60x20-1", "deak/60x20-1_500"), 54.8924, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_10"), 175.045, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_20"), 169.295, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_30"), 166.496, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_40"), 167.765, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_50"), 166.583, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_80"), 170.513, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_100"), 166.953, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_150"), 168.035, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_200"), 168.201, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_250"), 167.657, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_300"), 168.552, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_350"), 168.338, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_400"), 167.623, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_450"), 167.93, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x40-1", "deak/60x40-1", "deak/60x40-1_500"), 167.054, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_10"), 301.009, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_20"), 276.843, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_30"), 296.727, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_40"), 300.673, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_50"), 296.972, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_80"), 295.023, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_100"), 294.409, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_150"), 304.291, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_200"), 300.713, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_250"), 301.954, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_300"), 300.152, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_350"), 303.269, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_400"), 299.236, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_450"), 299.905, 0.1);
	EXPECT_NEAR(solveSMPS("deak/60x60-1", "deak/60x60-1", "deak/60x60-1_500"), 299.173, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_10"), 72.5601, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_20"), 66.509, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_30"), 78.4813, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_40"), 77.0243, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_50"), 72.3637, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_80"), 73.66, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_100"), 72.4236, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_150"), 74.6681, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_200"), 74.6522, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_250"), 74.7991, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_300"), 72.3126, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_350"), 73.9976, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_400"), 77.4241, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_450"), 73.8664, 0.1);
	EXPECT_NEAR(solveSMPS("deak/100x20-1", "deak/100x20-1", "deak/100x20-1_500"), 75.9172, 0.1);
}

TEST_F(NestedBendersTest, rand_OTHER) { // takes long to solve, so we restrict ourselves for preliminary test runs on a subset!
	EXPECT_NEAR(solveSMPS("other/rand0_2000"), 162.146, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand0_4000"), 199.032, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand0_6000"), 140.275, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand0_8000"), 170.318, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand0_10000"), 139.129, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand1_2000"), 244.159, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand1_4000"), 259.346, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand1_6000"), 297.563, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand1_8000"), 262.451, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand1_10000"), 298.638, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand2_2000"), 209.151, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand2_4000"), 218.247, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand2_6000"), 239.721, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand2_8000"), 239.158, 0.1);
	EXPECT_NEAR(solveSMPS("other/rand2_10000"), 231.706, 0.1);
}

TEST_F(NestedBendersTestFeasibility, feasibility) {// all problems that generate feasibility cuts (a fair amount of)

	EXPECT_NEAR(solveSMPS("bikeSharing/bikeSharing-100"), 2.693e6, 1e3);
	//EXPECT_NEAR(solveSMPS("bikeSharing/bikeSharing-523"),3.4349e6, 1e3); // debugger crashes
	//EXPECT_NEAR(solveSMPS("bikeSharing/bikeSharing-731"),3.67583e6, 1e3); deteq crashes
	EXPECT_NEAR(solveSMPS("saphir/saphir_50"), 524047000, 10000);
	EXPECT_NEAR(solveSMPS("saphir/saphir_100"), 524417000, 10000);
	//EXPECT_NEAR(solveSMPS("saphir/saphir_200"), 596063000, 10000); // Hard to solve, no solution with benders!
	EXPECT_NEAR(solveSMPS("saphir/saphir_500"), 482931000, 10000);
	EXPECT_NEAR(solveSMPS("saphir/saphir_1000"), 305918000, 10000);
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-loose"), 14794.6, 0.1); // Non-challenging CO2 limits //  19777.4
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-aggr"), 15963.9, 0.1); // Challenging CO2 limits // 20478.7
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-imp"), 20773.9, 0.1); // Import: Aggresive and Import prices adjusting, INDEP and BLOCKS //  22265.3, not solvable due to feas cuts not improving solution..
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-lrge"), 20799.3, 0.1); //Based on Import, INDEP and BLOCKS // 22438.9
	//EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-xlrge"), 20799.3, 0.1); // Even Larger, INDEP and BLOCKS // 22438.9 crashes on workstation
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-first"), 14794.6, 0.1); // INDEP
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-1200"), 20808.6, 0.1); // INDEP and BLOCKS for the rest  22428.9
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-1875"), 20809.3, 0.1); //  22447.1
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-3780"), 20794.7, 0.1); //  22441
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-5292"), 20788.6, 0.1); // 22438.4
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_6"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_16"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_6"), 18615.4, 0.1);// Model probability problems 18615.932
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_16"), 18439, 0.1); // 18438.891
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_6"), 18615.4, 0.1); // 18616.224
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_16"), 18439, 0.1); // 18438.891
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3"), 722.58, 0.1); // Dependent
	EXPECT_NEAR(solveSMPS("felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS3", "felt/electric_three_stage/LandS2"), 719.21, 0.1); // Independent 
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_16"), -2400.1, 0.5); // -2401.87
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_32"), -2172.9, 0.3); // -2167.62
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_64"), -2310.7, 0.1); //-2027.43
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_128"), -1888.3, 0.8); // -1879.43
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_256"), -1849.4, 0.8); // -1841.29
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_512"), -1797.3, 0.9); // -1787.19
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_768"), -1795.16, 2.0); // -1780.13
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1024"), -1798.4, 0.9); // -1791.18
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1152"), -1723.7, 0.6); // -1725.53
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1536"), -1705.8, 2.0);// -1693.47
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_1920"), -1778.3, 1.0); // -1769.45
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_2304"), -1674.9, 1.0);// -1664.66
	EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10", "watson/timeWAT_C_10", "watson/stochWAT_C_10_2688"), -1687.7, 0.9);// -1679.92
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_16"), -2158.75, 2.0);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_32"), -1866.42, 0.4);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_64"), -2310.53, 0.3);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_128"), -1637.81, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_256"), -2000.84, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_512"), -1959.64, 0.3);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_768"), -1813.11, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1024"), -1926.79, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1152"), -1687.92, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1536"), -1790.27, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1920"), -1778.95, 1.5);
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3"), -39976.8, 0.1); // seven-stage
	EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3", "felt/stocfor/stocfor2_7"), -39024.4, 0.1); // seven-stage, stochasticity from stocfor2
}

TEST_F(NestedBendersTestInteger, postponement) {// postponement
	EXPECT_NEAR(solveSMPS("postponement/postponement/Kor+1-4-4und15new=SL600L2Flex_2_end"), -775187, 1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/Kor0-4-4und45new=SL600L2Flex_2_end"), -786896, 1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/TS_36_new=SL600L2Flex_2_end"), 0, 1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/TS_81_new=SL600L2Flex_2_end"), 0, 1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/TS_256_new=SL600L2Flex_2_end"), 0, 1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/pTest_11"), -515723, 10);
	EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_1"), 0.0, 0.1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_100"), 0.0, 0.1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_25"), 0.0, 0.1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_50"), 0.0, 0.1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_75"), 0.0, 0.1);
	EXPECT_NEAR(solveSMPS("postponement/postponement/ptest_lp_100"), 0.0, 0.1);
}

TEST_F(NestedBendersTestInteger, mcfndsd_cr_RAHMANIANI) {
	//modified to avoid feas cuts
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-2_16_cr"), 57214.1, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-2_32_cr"), 58242.3, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-4_16_cr"), 55844.8, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-4_32_cr"), 56158.6, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-4_64_cr"), 56803.7, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-6_16_cr"), 53891.6, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-6_32_cr"), 56940, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-6_64_cr"), 57480.8, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-8_16_cr"), 57848.5, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-8_32_cr"), 57654.9, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-8_64_cr"), 57654.9, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0_16_cr"), 55170, 0.1); //
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_3_0-2_16_cr"), 98201.9, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_3_0-4_16_cr"), 96700.1, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_3_0-6_16_cr"), 94435.7, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_3_0_16_cr"), 95726.4, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_5_0-6_16_cr"), 84706.1, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_5_0_16_cr"), 86198.2, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-2_16_cr"), 24557.7, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-2_32_cr"), 25437.4, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-4_16_cr"), 24792.9, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-4_32_cr"), 24675.2, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-6_16_cr"), 23965.1, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-6_32_cr"), 25123, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-8_16_cr"), 25765.8, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0-8_32_cr"), 25403.5, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0_16_cr"), 24443.5, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r04_1_0_32_cr"), 25219.9, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0-2_16_cr"), 24763, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0-4_16_cr"), 24810.8, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0-4_32_cr"), 25849.5, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0-6_16_cr"), 26220.3, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0-8_16_cr"), 26370.1, 0.1);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r07_1_0_16_cr"), 25523.4, 0.1);
}

TEST_F(NestedBendersTestInteger, mcfndsd_RAHMANIANI) {// postponement
	// generate feas cuts
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-2_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-2_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-2_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-4_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-4_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-4_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-6_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-6_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-6_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-8_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-8_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0-8_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_1_0_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-2_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-2_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-2_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-4_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-4_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-4_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-6_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-6_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-6_64"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-8_16"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-8_32"), 0.0, 0.1); // never solved, what is the obj value?
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_feas_cuts/r01_3_0-8_64"), 0.0, 0.1); // never solved, what is the obj value?
	// there are much more instances in Benders/...
}

TEST_F(NestedBendersTest, FeasibilityCutTest) {
	using namespace flopc;
	MP_model testModel(new OsiCpxSolverInterface());
	MP_stage T(2);
	MP_variable x, y(T);
	x.lowerLimit() = -10;
	y.upperLimit(T) = 15;

	MP_random_data randomData(T);
	randomData(1) = new DiscreteUniformRandomVariable(10, 20, 11, new BracketMedianDiscretization(11));
	randomData(1)->setStage(1);

	MP_constraint firstStage, secondStage(T);

	firstStage() = x() <= 5;
	secondStage(T + 1) = y(T) >= -x() + randomData(T);
	// Death test only under linux..
	//EXPECT_DEATH(testModel.attach(),"F.* Your model does not have an objective" );
	testModel.setObjective(x() + 2 * y(T.last()));
	testModel.attach();
	OsiCpxSolverInterface osi_cpx;
	OsiStochasticParallelNestedBendersSolverInterface pnb(&osi_cpx, testModel.getSmi());
	parallel_benders::Parameter param;
	param.init();
	param.parameter_.put(parallel_benders::PNB_NUM_CPU, 1);
	pnb.setParameters(param);
	pnb.initialSolve(); // We need the osi_solver interface!

	// We need to assert something here with google assertions
	ASSERT_TRUE(true);

	// Output file
	//SmiScnModel* smiModel = testModel.getSmi();
	//smiModel->writeSmps("feasibilityProblem",true,false);
}

TEST_F(NestedBendersTest, DISABLED_StageAggregationTest) { // TODO: crashes
	assert(false && "see TODO above");
	using namespace flopc;
	OsiSolverInterface* osi_cplex = new OsiCpxSolverInterface();
	MP_model testModel(osi_cplex);
	enum { stage1, stage2, stage3, stage4, number_stages };
	enum { scen1, scen2, scen3, scen4, scen5, scen6, scen7, scen8, scen9, scen10, scen11, scen12, scen13, scen14, scen15, scen16, num_scenarios };
	MP_stage T(number_stages);
	MP_scenario_set scenario_set(num_scenarios);
	MP_variable x, y(T);
	//x.lowerLimit() = -10;
	//y.upperLimit(T) = 15;
	// 
	double scenario_data[number_stages - 1][num_scenarios] =
	{
		{   2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1 , 2.2, 2.2, 2.2, 2.3, 2.3, 2.3, 2.3, 2.3, 2.3  },// Stage 2
		{    3.1, 3.1, 3.2, 3.2, 3.2, 3.3, 3.3, 3.4, 3.5, 3.5, 3.6, 3.6, 3.7, 3.7, 3.7, 3.8   }, // Stage 3
		{   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 } // Stage 4

	};//new double[number_stages][num_scenarios];

	MP_random_data randomParam(&scenario_data[0][0], T);
	double* objCoefs = new double[3];
	objCoefs[0] = 2;
	objCoefs[1] = 3;
	objCoefs[2] = 2;
	MP_data param1(objCoefs, T);
	double* objCoefs1 = new double[3];
	objCoefs1[0] = -1;
	objCoefs1[1] = 0;
	objCoefs1[2] = 1;
	MP_data param2(objCoefs, T);

	MP_constraint firstStage, secondStage(T);

	firstStage() = x() >= 5;
	secondStage(T + 1) = y(T - 1) + randomParam(T) * y(T) >= randomParam(T);
	testModel.setObjective(x() + sum(T, param1(T) * y(T)));
	testModel.attach();
	testModel.getSmi()->writeSmps("stageAggregTest");
	//OsiCpxSolverInterface cpx;
	OsiSolverInterface* new_solver = new OsiCpxSolverInterface();


	testModel.getSmi()->aggregateStages(1, 3, false);
	testModel.getSmi()->writeSmps("testSMPS");
	//testModel.getSmi()->loadOsiSolverData();

	//OsiStochasticBendersTwoStageSolverInterface osi_benders(new_solver,testModel.getSmi());
	//BendersParameter parameter;
	//parameter.setIntParam(BendersParameter::BENDERS,0);
	//osi_benders.setParams(parameter);
	//osi_benders.initialSolve();


	//testModel.getSmi()->writeSmps("stageAggreg2Test");
	parallel_benders::Parameter benders_parameter_;
	benders_parameter_.init(); // This should be called by the standard constructor anyway!
	benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS, 1); //Use deteq
	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START, 1);
	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END, 3);
	OsiStochasticParallelNestedBendersSolverInterface pnb(new_solver, testModel.getSmi());
	pnb.setParameters(benders_parameter_);
	pnb.initialSolve();
	EXPECT_NEAR(9.41, pnb.getObjValue(), 0.1);
	// Output file
	//SmiScnModel* smiModel = testModel.getSmi();
	//smiModel->writeSmps("feasibilityProblem",true,false);
	//     testModel.solve(MP_model::MINIMIZE);
}

TEST_F(NestedBendersTestSampleAverageApproximation, sample_average_approximation1) {
	SolveSAA("wright/data-ssn/ssn");
	SolveSAA("wright/data-20/20");
	SolveSAA("wright/data-storm/storm");
	SolveSAA("wright/data-gbd/gbd");
	SolveSAA("wright/data-LandS/LandS");
}

TEST_F(NestedBendersTest, DISABLED_temporary) {// TODO: crashes
	assert(false && "see TODO above");
	benders_parameter_.parameter_.put(parallel_benders::PNB_CVAR_BETA, 0.1);
	benders_parameter_.parameter_.put(parallel_benders::PNB_CVAR_RHO, 0);
	benders_parameter_.parameter_.put(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, 1);
	EXPECT_NEAR(solveSMPS("saphir/saphir_50"), 524047000, 10000);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_10"), 269.591, 0.1);
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-loose"), 14794.6, 0.1); // generates feasibility cuts
	EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
	solveSMPS("posts/storm/stormG2_27");
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm", "wright/data-scenarios/storm", "wright/data-scenarios/storm-1000"), 1.54851e7, 1e3);
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn", "wright/data-scenarios/ssn", "wright/data-scenarios/ssn-3000"), 9.24629, 0.1);
	EXPECT_NEAR(solveSMPS("other/sslp_10_50_1000"), -352.494, 0.1);
	EXPECT_NEAR(solveSMPS("wright/data-gbd/gbd"), 1655.628, 0.1); //two-stage
	EXPECT_NEAR(solveSMPS("other/rand0_10000"), 139.129, 0.1);
	EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10", "watson/timeWAT_I_10", "watson/stochWAT_I_10_1152"), -1687.92, 0.1);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_10"), 44614465.2, 100);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_5000"), 44614465.2, 100);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_10000"), 44614465.2, 100);
	EXPECT_NEAR(solveSMPS("deak/20x20-1", "deak/20x20-1", "deak/20x20-1_50000"), 44614465.2, 100);
	EXPECT_NEAR(solveSMPS("wright/data-LandS/LandS"), 225.629, 0.1); //two-stage
	EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm", "wright/data-scenarios/storm", "wright/data-scenarios/storm-1000"), 1.54851e7, 1e3);
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-xlrge"), 20799.3, 0.1); // Even Larger, INDEP and BLOCKS // 22438.9  // generates feasibility cuts
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-xlrge"), 20799.3, 0.1); // Even Larger, INDEP and BLOCKS // 22438.9  // generates feasibility cuts
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-4"), 416.5, 0.1);
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-64"), 423.0, 0.1);
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-xlrge"), 14794.6, 0.1); // INDEP,BLOCKS  // generates feasibility cuts
	EXPECT_NEAR(solveSMPS("felt/cargo/4node", "felt/cargo/4node-256"), 425.38, 0.1);
	EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_16"), 18439, 0.1); // 18438.891
	EXPECT_NEAR(solveSMPS("felt/environ/env-diss", "felt/environ/env", "felt/environ/env-lrge"), 20799.3, 0.1); //Based on Import, INDEP and BLOCKS // 22438.9  // generates feasibility cuts

	EXPECT_NEAR(solveSMPS("twostage/l10/100x20"), 289654, 0.1);
	EXPECT_NEAR(solveSMPS("twostage/l9/60x60"), 302421.20, 1.0);
}

TEST_F(NestedBendersTest, DISABLED_temporarySD) { // TODO: crashes
	assert(false && "see TODO above");
	parallel_benders::Timer overall_and_solver_timer(2);
	overall_and_solver_timer.Start(0);
	SmiScnModel smi;
	OsiCpxSolverInterface osi_cpx;
	smi.setOsiSolverHandle(&osi_cpx);
	smi.setSampling(true);
	int read_in = smi.readSmps("../../../../Data/Benders/test/instances/wright/data-storm/storm");

	if (read_in != 0) {
		LOG(ERROR) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
	}
	int num_stages = smi.getCore()->getNumStages();
	// Load ParallelNestedBendersSolver
	overall_and_solver_timer.Start(1);
	OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx, &smi);
	parallel_benders::Parameter benders_parameter;
	benders_parameter.init();
	nested_benders.setParameters(benders_parameter);
	nested_benders.initialSolve();
	overall_and_solver_timer.Stop(0);
	overall_and_solver_timer.Stop(1);
	double solver_time = overall_and_solver_timer.GetTime(1); // Solver only time
	double cpuTime = overall_and_solver_timer.GetTime(0); // Overall WallClockTime;
	LOG(INFO) << "Solution Time: " << cpuTime;
}

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char* argv[]) {
	//_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_WNDW);
	int result = 0;

	::testing::GTEST_FLAG(output) = "xml:../../../../Data/Benders/NestedBendersTestLog.xml"; // store results in xml file //
	//::testing::GTEST_FLAG(filter) = "UnitTest.*";  // use in command line arguments --gtest_filter="<>" with one of: NestedBendersTest.*, NestedBendersTestFeasibility.*, *.fast, *.slow ...
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
	google::InstallFailureSignalHandler(); //This does not work under windows (prints out stacktWrace)
#endif
	google::SetLogDestination(google::WARNING, "google_log_"); // output file
	//google::LogToStderr(); // only to console
	google::SetStderrLogging(google::INFO); // logging to std::err enabled for all logging levels >= 0 (Different logging under release builds?)
	result = RUN_ALL_TESTS();

	//_CrtDumpMemoryLeaks(); // outside of scoping { }
	return result;
}

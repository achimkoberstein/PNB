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

//#include "BendersNodeData.hpp"
#include "ParallelNestedBenders.hpp"
#include "BranchAndBendersCut.hpp"
#include "OsiBbcStochasticSolverInterface.hpp"

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

class BranchAndBendersTest : public ::testing::Test { // this is the Test set, e.g.,  TEST(NestedBendersTest, test_case). Second argument is the test (also called test case) 

	public:

		parallel_benders::Parameter bbc_parameter_; // Used to store parameters
		std::vector<parallel_benders::Parameter> parameter_sets_; // Used to store different combinations (Parameter instances) of parameters
		std::string instance_specification_file_; // for characteristics of the instance such as number vars, constraints, stages etc.
		std::string testbed_aggregated_log_file_string_; // average of repetitions
		std::string testbed_log_file_string_; // for every repetition
		std::string parameter_name_string_;
		static const int max_stages_ = 11; // This can be set differently

		bool compute_statistics_;
		bool generate_initial_solution_;

		double solveSMPS(const char* name) {
			return solveSMPS(name, name, name);
		}
		double solveSMPS(const char* core_time_name, const char* stoch_name) {
			return solveSMPS(core_time_name, core_time_name, stoch_name);
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

			parallel_benders::Parameter param(bbc_parameter_); // Copy parameter!
			updateParameterSets();

			int num_aggregates_by_magic = 0;
			
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
			
			std::vector<int> aggregates;
			aggregates.push_back(1);
			
			for (auto const& parameter_set : parameter_sets_) {
				bbc_parameter_ = parameter_set;
				int aggregates = bbc_parameter_.parameter_.get(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 1);
				// we skip if number of aggregates > number of scenarios
				LOG_IF(INFO, num_scenarios < aggregates && num_stages_ == 2) << "Solving of " << stoch_name << " with PNB_AGGREGATED_OPTIMALITY_CUTS " << aggregates << " was skipped. Num 2nd stage scenarios < aggregates. Maybe you did not activate read_in_smps_file to set num_scenarios.";
				if (!(num_childs_first_stage < aggregates && num_stages_ == 2)) {
					write_xml("current_param.xml", bbc_parameter_.parameter_);
					solutions.push_back(solveProblemWithParameters(core_name, time_name, stoch_name));
					LOG(INFO) << "";
				}
			}

			// Reset benders parameter setting!
			bbc_parameter_ = param;
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
		BranchAndBendersTest() { // this is the fixture that is called for every test, e.g., TEST(BranchAndBendersTest, test)
			bbc_parameter_.init();
			write_xml("../../../../Data/Benders/default_param.xml", bbc_parameter_.parameter_);
			repetitions_ = 1;
			
			assert(false && "Update these stuff, initialize stats");
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
		virtual ~BranchAndBendersTest() {}
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
			BOOST_FOREACH(boost::property_tree::ptree::value_type & v, bbc_parameter_.parameter_)
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
				OsiStochasticBranchAndBendersCutSolverInterface bbc_interface(&osi_cpx, &smi);
				bbc_interface.setTestsetProblemInstanceNames(total_stoch_string);
				if (bbc_parameter_.GetBoolParamValue(parallel_benders::PNB_SAVE_INSTANCE_SPECS)) {
					bbc_interface.setInstanceSpecsFile(instance_specification_file_);
				}
				bbc_interface.setParameters(bbc_parameter_);
				if (bbc_parameter_.parameter_.get(parallel_benders::PNB_USE_START_SOLUTION, 0) == parallel_benders::PNB_INT_USE_SUPPLIED_SOLUTION) {
					assert(current_start_solution_.get() != nullptr);
					bbc_interface.giveStartSolution(current_start_solution_.get(), num_single_scenario_cols_); //TODO: it should work
				}
				bbc_interface.initialSolve();
				overall_and_solver_timer.Stop(0);
				overall_and_solver_timer.Stop(1);
				double solver_time = overall_and_solver_timer.GetTime(1); // Solver only time
				double cpuTime = overall_and_solver_timer.GetTime(0); // Overall WallClockTime;
				bbc_interface.printSolutionToFile("primal_solution.txt");
				min_cpu = min(cpuTime, min_cpu);
				min_solver = min(solver_time, min_solver);
				max_cpu = max(max_cpu, cpuTime);
				max_solver = max(max_solver, solver_time);
				LOG(INFO) << "Solution Time: " << cpuTime;

				if (bbc_parameter_.parameter_.get(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, 0) != parallel_benders::PNB_INT_SOLUTION_PRIMAL_NO
					&& !(bbc_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, 0) == parallel_benders::PNB_INT_ALGORITHM_DETEQ || bbc_parameter_.parameter_.get(parallel_benders::PNB_BENDERS, 0) == parallel_benders::PNB_INT_ALGORITHM_DETEQ_RISK)) {
					// Get num_cols when specified correctly
					// This works for non-appended problems!
					int last_stage = bbc_parameter_.parameter_.get(parallel_benders::PNB_COMPUTE_PRIMAL_SOLUTIONS, 0);
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
						const double* first_stage = bbc_interface.getSolutionByNodeAndStage(0, i);
						if (first_stage != nullptr)
							std::copy(first_stage + offset_cols, first_stage + offset_cols + smi.getCore()->getNumCols(i), first_stage_solution_.get() + offset_cols);
						offset_cols += smi.getCore()->getNumCols(i);
					}
				}
				int created_opt_cuts = bbc_interface.getNumOptimalityCuts();
				int created_cont_opt_cuts = bbc_interface.getNumContinuousOptimalityCuts();
				int created_feas_cuts = bbc_interface.getNumFeasibilityCuts();
				int created_epigraph_cuts = bbc_interface.getNumEpigraphCuts();
				int created_str_epigraph_cuts = bbc_interface.getNumStrengthenedEpigraphCuts();
				int created_plain_epigraph_cuts = bbc_interface.getNumPlainEpigraphCuts();
				int created_domain_cuts = bbc_interface.getNumDomainCuts();
				int solved_lp_cgp_problems = bbc_interface.getLpCgpCounter();

				// We get the parameters from the solver back to evaluate it.
				bbc_parameter_ = bbc_interface.getParameters();

				// File name Should have the following form based on Parameters: Single/Multi/Aggregate-NumberCuts/Aggregate-Percent-Number/Aggregate-Fixed-Number
				std::stringstream s;
				s.str("");
				testset_problem_instance.str("");

				// Write solution line
				//Testset;Problem;Instance
				testset_problem_instance << bbc_interface.getTestsetProblemInstanceNames()[0] << ";" << bbc_interface.getTestsetProblemInstanceNames()[1] << ";" << bbc_interface.getTestsetProblemInstanceNames()[2];
				s << testset_problem_instance.str() << ";" << cpuTime << ";" << bbc_interface.getNumIterations() << ";" << bbc_interface.getObjValue() << ";" << bbc_interface.getUB() << ";" << bbc_interface.getLB() << ";";
				s << bbc_interface.getRelativeGap() << ";" << bbc_interface.getSolutionCode() << ";" << bbc_interface.getNumInfeasibilityIterations() << ";";
				s << bbc_interface.getNumIterations() - bbc_interface.getNumInsubstantialIterations() << ";" << bbc_interface.getNumInsubstantialIterations() << ";";
				s << solver_time << ";" << bbc_interface.getNumCriticalIteration() << ";" << bbc_interface.getMaximalLengthOfCriticalIterationSequence() << ";";
				// Write num cuts
				s << created_opt_cuts << ";" << created_feas_cuts << ";" << created_epigraph_cuts << ";" << created_str_epigraph_cuts << ";" << created_plain_epigraph_cuts << ";" << created_domain_cuts << ";" << solved_lp_cgp_problems;
				// Store all of the above values in value_map!
				values_map[cpu_time_string] += cpuTime;
				values_map[solver_time_string] += solver_time;
				values_map[iterations_string] += bbc_interface.getNumIterations();
				values_map[insubstantial_iterations_string] += bbc_interface.getNumInsubstantialIterations();
				values_map[substantial_iterations_string] += bbc_interface.getNumIterations() - bbc_interface.getNumInsubstantialIterations();
				values_map[infeasibility_iterations_string] += bbc_interface.getNumInfeasibilityIterations();
				values_map[objective_value_string] += bbc_interface.getObjValue();
				values_map[ub_string] += bbc_interface.getUB();
				values_map[lb_string] += bbc_interface.getLB();
				values_map[relative_gap_string] += bbc_interface.getRelativeGap();
				values_map[optimality_cuts_string] += created_opt_cuts;
				values_map[cont_optimality_cuts_string] += created_cont_opt_cuts;
				values_map[feasibility_cuts_string] += created_feas_cuts;
				values_map[epigraph_cuts_string] += created_epigraph_cuts;
				values_map[epigraph_str_cuts_string] += created_str_epigraph_cuts;
				values_map[epigraph_plain_cuts_string] += created_plain_epigraph_cuts;
				values_map[domain_cuts_string] += created_domain_cuts;
				values_map[lp_cgp_problems_string] += solved_lp_cgp_problems;
				values_map[integer_gap_string] += bbc_interface.getIntegerGap();
				values_map[cont_phase_iterations_string] += bbc_interface.getNumContPhasePnbIterations();
				values_map[return_code_string] += bbc_interface.getSolutionCode();
				values_map[critical_iterations_string] += bbc_interface.getNumCriticalIteration();
				values_map[critical_iteration_sequence_length_string] += bbc_interface.getMaximalLengthOfCriticalIterationSequence();

				ub_for_iteration_.clear();
				lb_for_iteration_.clear();
				for (int i = 0; i < bbc_interface.getNumIterations(); i++) {
					ub_for_iteration_.push_back(bbc_interface.getUBForIteration(i + 1));
					lb_for_iteration_.push_back(bbc_interface.getLBForIteration(i + 1));
				}

				// Write parameter values
				// Tree is ordered after insertion. This should be the same as the first iteration, because we insert EVERY value in the init() method of the parameter class.
				BOOST_FOREACH(boost::property_tree::ptree::value_type & v, bbc_parameter_.parameter_)
				{
					std::string name = v.first;
					std::string value = bbc_parameter_.parameter_.get<std::string>(name);
					s << value << ";";
				}
				// Write Detailed Time Statistics
				const parallel_benders::Timer& timer(bbc_interface.getTimer());
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

				const parallel_benders::Timer& solution_timer(bbc_interface.getComputingTimer());
				const parallel_benders::Timer& build_timer(bbc_interface.getBuildTimer());
				for (int i = 0; i < solution_timer.GetTimeStages(); i++) {
					s << solution_timer.GetTime(i) << ";";
					values_map["computing" + boost::lexical_cast<std::string>(i)] += solution_timer.GetTime(i);
				}
				for (int i = 0; i < build_timer.GetTimeStages(); i++) {
					s << build_timer.GetTime(i) << ";";
					values_map["building" + boost::lexical_cast<std::string>(i)] += build_timer.GetTime(i);
				}

				// L&P Cuts Times
				const parallel_benders::Timer& lp_cuts_timer(bbc_interface.getLiftAndProjectCutsTimer());
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
				if (loop_counter == 0 && bbc_interface.getSolutionCode() == parallel_benders::PNB_SOLUTION_TIME_LIMIT) {
					repetitions_ = 1;
					values_map[solver_time_string] = bbc_interface.getParameters().parameter_.get(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 3600);
				}
				else if (bbc_interface.getSolutionCode() != parallel_benders::PNB_SOLUTION_OPTIMAL && bbc_interface.getSolutionCode() != parallel_benders::PNB_SOLUTION_UPPER_LESS_LOWER_BOUND) {
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

			BOOST_FOREACH(boost::property_tree::ptree::value_type & v, bbc_parameter_.parameter_)
			{
				std::string name = v.first;
				std::string value = bbc_parameter_.parameter_.get<std::string>(name);
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
				parallel_benders::Parameter param(bbc_parameter_); // based on bbc_parameter_ which normally has default values unless changed in TEST_F(){...}
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

TEST(BranchAndBendersTest, unitTest) {
	
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
	param.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);

	core_name = "glushko/sndp_mini_tests/SNDP_TestCase_17";
	time_name = "glushko/sndp_mini_tests/SNDP_TestCase_17";
	stoch_name = "glushko/sndp_mini_tests/SNDP_TestCase_17";
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
	OsiStochasticBranchAndBendersCutSolverInterface bbc_interface(osi_solver, &smi_model);
	bbc_interface.setParameters(param);

	branch_and_benders_cut::BranchAndBendersCut* bbc = bbc_interface.getBBC();
	bbc->Init();
	bbc->Solve();

	//bbc->Reset(-1, true);

	delete osi_solver;
}

TEST_F(BranchAndBendersTest, fast) {
	// Binary 1st stage
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 300);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_CGP_EPSILON_RELATIVE, 0.01);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 0);
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
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2); // relaxation for the 2nd stage, ERROR otherwise
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

TEST_F(BranchAndBendersTest, slow) {
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 300);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_CGP_EPSILON_RELATIVE, 0.01);
	EXPECT_NEAR(solveSMPS("glushko/sndp/SNDP_20_4_2_1"), -87272, 0.1);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_40_3_2_25"), -136474, 0.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_1_25"), -397621, 0.5);
	EXPECT_NEAR(solveSMPS("glushko/sndp/sndp_80_3_2_25"), -479326, 0.5);
	EXPECT_NEAR(solveSMPS("rahmaniani/MCFNDSD_complete_recourse/r01_1_0-6_16_cr"), 53891.6, 0.1); //slow

	// Integer on 2 stages
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
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
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION, true);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION_START, 1); // from the 2nd stage
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION_END, -1); // till the last stage
	EXPECT_NEAR(solveSMPS("scdp/scdp/SCDP-1024"), -6301005010, 10);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_STAGE_AGGREGATION, false);
}

TEST_F(BranchAndBendersTest, for_LP) {
	//have bin and sometimes cont vars on the 1st stage
	//need >0.2 secs to be solved on workstation with DETEQ
	//second stage integrality is relaxed since some instances have int on the 2nd stage
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_EPSILON_RELATIVE, 0.000001);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME, 10800);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_RELAXATION, 2);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_WARM_START, parallel_benders::PNB_INT_WARM_START_SCENARIO_WISE);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_LP_PARALLEL_MODE, parallel_benders::PNB_PARAL_FRACTS_AND_CGP_SUBPROBLEMS);

	bbc_parameter_.SetParameterValue(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 0);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_LP_USE_CGP_PNB_CUTS, parallel_benders::PNB_CGP_CUTS_NONE);

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

	bbc_parameter_.SetParameterValue(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS, 1);
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_LP_USE_CGP_PNB_CUTS, parallel_benders::PNB_CGP_CUTS_ALL);

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
	bbc_parameter_.SetParameterValue(parallel_benders::PNB_WARM_START, parallel_benders::PNB_INT_NO_WARM_START);
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

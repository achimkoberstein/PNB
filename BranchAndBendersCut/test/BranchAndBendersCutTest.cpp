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
	OsiStochasticBranchAndBendersCutSolverInterface bbc_interface(osi_solver, &smi_model);
	bbc_interface.setParameters(param);

	branch_and_benders_cut::BranchAndBendersCut* bbc = bbc_interface.getBBC();
	bbc->Init();
	bbc->Solve();

	bbc->Reset(-1, true);

	boost::threadpool::pool thread_pool_(10);
	std::vector<parallel_benders::ParallelNestedBenders*> thread_pnbs;
	thread_pnbs.reserve(thread_pool_.size());
	for (int i = 0; i < thread_pool_.size(); i++) {
		parallel_benders::ParallelNestedBenders* thread_pnb = bbc->clone();
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

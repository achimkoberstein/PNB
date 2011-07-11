#define BOOST_TR1_TUPLE_HPP_INCLUDED //This prevents compilation issues.
#include <gtest/gtest.h>
#include <iostream>
#include <ctime>
#include <limits>
#include <cmath>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <sys/stat.h>

#include <glog/logging.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"
#include "CoinTime.hpp"
#include "SmiScnModel.hpp"
#include "flopc.hpp"

#include "BendersNodeData.hpp"
#include "ParallelNestedBenders.hpp"
#include "OsiPnbStochasticSolverInterface.hpp"
#include "OsiBenStochasticSolverInterface.hpp"


class NestedBendersTest: public ::testing::Test {
public:

    parallel_benders::Parameter benders_parameter_; // Used to store parameters
	std::string testbed_log_file_string_;
	std::string parameter_name_string_;
    static const int max_stages_ = 10;

    double solveSMPS(const char* name) {
        return solveSMPS(name, name,name);
    }
    double solveSMPS(const char* core_time_name, const char* stoch_name) {
        return solveSMPS(core_time_name, core_time_name,stoch_name);
    }
    double solveSMPS(const char* core_name, const char* time_name, const char* stoch_name) {
        // Start with single cut and Benders
        std::vector<double> solutions;
		// Try different multicut aggregation level
		//benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,0);
		//solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
        //benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1);
        //benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,1); // Let system decide!
        //benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1); // Solve DetEq
        //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//for (int i = 0; i < max_stages_;i++){
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,0);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,true);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,parallel_benders::PNB_INT_STAGE_AGGREGATION_LAST_STAGE);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,parallel_benders::PNB_INT_STAGE_AGGREGATION_LAST_STAGE-i); // Aggregate from the end
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//}
        //benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1); // Solve DetEq
        //benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,0);
        ////benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,true);
        ////benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,3);
        ////benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,2); // Aggregate from the end
        //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
        //benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,0); // Solve DetEq
        //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
        benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1); // Solve DetEq
        solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
         // Solve the same problem with OsiBenStochasticSolverInterface..
        std::string folder("../../../../Benders/test/instances/");
#if _M_X32 || __x83_32__ // Works only on windows i suppose
        folder = "../../../../../Benders/test/instances/";
#endif
        std::string core_name_string(core_name);
        std::string time_name_string(time_name);
        std::string stoch_name_string(stoch_name);
        std::string total_core_string = folder+core_name_string;
        std::string total_time_string = folder+time_name_string;
        std::string total_stoch_string = folder+stoch_name_string;
        OsiCpxSolverInterface osi_cpx;
        SmiScnModel smi;
        smi.readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
        // We can only solve aggregated problems..
        // But we can aggregate the problem if we want to, we have the smi model!
        smi.aggregateStages(1,smi.getCore()->getNumStages()-1,true); // We construct a two-stage problem, where the second stage contains all the scenarios we want?
        OsiStochasticBendersTwoStageSolverInterface two_stage_benders(&osi_cpx,&smi);
        two_stage_benders.initialSolve();
        solutions.push_back(two_stage_benders.getObjValue());

        // We compare the non-aggregated case, with the aggregated case and compare it with det.eq.

		//for (int i = 1; i <= 2; i++){
		//	// Loop parameters
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,i); // choose number of cpus
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1); // choose benders decomp
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,1); // choose stage aggregation
		//	// We may need different parameters here!
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,3);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,5);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,2);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,3);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,2);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,1);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,0);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//}
		// Choose second iteration strategy
		//for (int i = 1; i < 7; i++){
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_SECOND_ITERATION);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_STAGE,i);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,0);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,1);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,3);
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,7);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,0);
		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

		//}
              // Enable cascading and repeating with parameters TODO
        
 		// And do everything with different number of cpus to get a measure of effectivenesss for parallelization
		//for (int k = 2; k < 5; k++){
		//	benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,k);
		//	for (int j = 1; j < 2 ; j++){
		//		benders_parameter_.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,j);
		//		benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FASTPASS);
		//		solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//		//for ( int i = 1; i < 5; i++){
		//		//	benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON,std::pow(10.0,-3));
		//		//	benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_EPSILON_FAST_BACKWARD);
		//		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//		//	benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_EPSILON_FAST_FORWARD);
		//		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//		//}
		//		// Do some checks with second_stage_iteration
		//		//benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_SECOND_ITERATION);
		//		//for ( int i = 1; i < 7; i++ ){
		//		//	benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_STAGE,i);
		//		//	solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
		//		//}
		//	}
		//}
        double average = 0;
        for (int i = 0; i < solutions.size(); i++)
            average += solutions[i];
        average = average / solutions.size();
        return average;
 
    }

    bool FileExists(const char* filename) 
    {
        struct stat info;
        int ret = -1;

        //get the file attributes
        ret = stat(filename, &info);
        if(ret == 0) 
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
    NestedBendersTest() {
        benders_parameter_.init();
    }
    virtual ~NestedBendersTest(){}
    virtual void SetUp(){
		testbed_log_file_string_ = "Results.csv";

		// Write title line.
		std::stringstream s;
		// Header line of File should be this one
		// We need to write the header line for this file
		s << "Problem Name; " << "Overall Wall Clock Time (s); " << "Iterations; " << "Solver Wall Clock Time (s); " << "Solution value; ";
		// //TODO: Iterate over property tree
		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, benders_parameter_.parameter_)
		{   
			std::string name = v.first;
			s << name << "; ";
		}
		// Detailed Time for several stages. 
        s << "Time Setup; ";
        for (int i = 0; i < max_stages_; i++){
            s << "Time Stage " << i << "; ";
        }
        // Date/Timestamp
        s << "Date; Time; ";
		parameter_name_string_ = s.str();
		
		if ( !FileExists(testbed_log_file_string_.c_str() ) ){
		    ofstream log_file;
			log_file.open(testbed_log_file_string_.c_str(), ios::app | ios::out);
			log_file << parameter_name_string_ << std::endl;
			log_file.close();
		}
		else {
			// We need to check if first line is still valid
			ifstream log_file;
			log_file.open(testbed_log_file_string_.c_str());
			std::string line;
			std::getline(log_file,line);
			log_file.close();
			if ( line.compare(parameter_name_string_) ){
				LOG(FATAL) << "Your parameter line changed in your output log file. Please remove old log file before proceeding!.";
			}

		}
	}
    virtual void TearDown(){}

    double solveProblemWithParameters( const char* core_name, const char* time_name, const char* stoch_name ){
        // We set the correct folder here: All files are in Benders/instances
        std::string folder("../../../../Benders/test/instances/");
#if _M_X32 || __x83_32__ // Works only on windows i suppose
        folder = "../../../../../Benders/test/instances/";
#endif
        std::string core_name_string(core_name);
        std::string time_name_string(time_name);
        std::string stoch_name_string(stoch_name);
        std::string total_core_string = folder+core_name_string;
        std::string total_time_string = folder+time_name_string;
        std::string total_stoch_string = folder+stoch_name_string;
        // This can be done in a loop.
        double time = CoinCpuTime();  
        SmiScnModel smi;
        OsiCpxSolverInterface osi_cpx;
        smi.setOsiSolverHandle(osi_cpx);
        int read_in = smi.readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
        if ( read_in!= 0 ){
            LOG(ERROR) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
            return -osi_cpx.getInfinity();
        }
        // Load ParallelNestedBendersSolver
        double solver_time = CoinCpuTime();
        OsiStochasticParallelNestedBendersTwoStageSolverInterface nested_benders(&osi_cpx,&smi);
        nested_benders.setParameters(benders_parameter_);
        nested_benders.initialSolve();
        solver_time = CoinCpuTime()-solver_time; // Solver only time
        time = CoinCpuTime()-time; // Overall WallClockTime;
        LOG(INFO) << "Solution Time: " << time;

		// We get the parameters from the solver back to evaluate it.
		benders_parameter_ = nested_benders.getParameters();

        // Write to log file
        ofstream log_file;
        std::string log_file_name; // <- set name according to Parameters..
        // File name Should have the following form based on Parameters: Single/Multi/Aggregate-NumberCuts/Aggregate-Percent-Number/Aggregate-Fixed-Number
        std::stringstream s;
		int aggregated_cuts = benders_parameter_.parameter_.get(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,1);
        int num_scen_pro_cut = benders_parameter_.parameter_.get(parallel_benders::PNB_SCENARIOS_PRO_CUT,0);
        if ( num_scen_pro_cut ){// We used this one
            assert(num_scen_pro_cut >0);
            s << "NumScen-" << num_scen_pro_cut;
            // Do not create an additional entry
            aggregated_cuts = 0;
        }
        if ( aggregated_cuts == 0 ){ // We do some Percent/Fixed Aggregation (or dynamic.. beware)
            s << "Multi";
        }
        else{
            if ( aggregated_cuts == 1)// Single
                s << "Single";
            else if ( aggregated_cuts >= smi.getNumScenarios())
                s << "Multi";
            else
                s << "StaticAggregation-" << aggregated_cuts;
        }
        s << ".csv";
        log_file_name = s.str();
		// Reset stringstream
		s.str("");

        if ( !FileExists(log_file_name.c_str() )){ // Otherwise we would like to check if the first line matches with our line.
            log_file.open(log_file_name.c_str(),ios::app | ios::out);
			log_file << parameter_name_string_ << std::endl;

        }
        else {
			ifstream check_log_file;
			check_log_file.open(log_file_name.c_str());
			std::string line;
			std::getline(check_log_file,line);
			check_log_file.close();
			
			if ( line.compare(parameter_name_string_) ){
				// We need to write this string first, because somehow our parameter ordering/number of them changed. If we notice this, we want to warn the user?!
				LOG(FATAL) << "Your parameter line changed in your output log file. Please remove old log file before proceeding!.";
			}
            log_file.open(log_file_name.c_str(),ios::app | ios::out);
        }
        // Write solution line
        s << stoch_name << "; " << time << "; " << nested_benders.getNumIterations() << "; "<< solver_time << "; " << nested_benders.getObjValue() << "; ";
		// Write parameter values
		// Tree is ordered after insertion. This should be the same as the first iteration, because we insert EVERY value in the init() method of the parameter class.
		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, benders_parameter_.parameter_)
		{   
			std::string name = v.first;
			std::string value = benders_parameter_.parameter_.get<std::string>(name);
			s << value << "; ";
		}
        // Write Detailed Time Statistics
        const parallel_benders::Timer& timer(nested_benders.getTimer());
        s << timer.GetTime(timer.GetTimeStages()-1) << "; "; // num_stages is time for setup and stuff
        int num_stages = smi.getCore()->getNumStages();
        if ( num_stages > max_stages_)
            LOG(ERROR) << "Only the first " << num_stages << " stages get reported with wall clock time.";
        for (int i = 0; i < smi.getCore()->getNumStages(); i++){
            s << timer.GetTime(i) << "; ";
        }
        for (int i = 0; i < max_stages_-num_stages; i++)// Ensure that the date/time stamp is where it is supposed to be!
            s << 0 << "; ";
        

        // Write date/time stamp of log file writing time
        char sdate[9];
        char stime[9];
        s << _strdate(sdate) << "; " << _strtime(stime);
        s << std::endl;

		// Write to log_file
		log_file << s.str();
        log_file.close();

		// Write to testbed log file
		ofstream testbed_log_file;
		testbed_log_file.open(testbed_log_file_string_.c_str(),ios::app | ios::out);
		testbed_log_file << s.str();
		testbed_log_file.close();

        return nested_benders.getObjValue();
    }
};


TEST_F(NestedBendersTest,BirgeLouveuxFarmersTest){
    using namespace flopc; // Do not do this ;)
    // define the model as presented in the book, p. 237ff.
    MP_model testModel(new OsiCpxSolverInterface());

    enum {numT=3};
    MP_stage T(numT);

    enum {numScen=4};
    MP_scenario_set scenSet(numScen);

    double valD[numT-1][numScen] =
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
    demand(T+1) = y(T) + x(T+1) + w(T+1) - y(T+1) == d(T+1);

    testModel.setObjective(
        x(0) + 3*w(0) + 0.5*y(0)
        + x(1) + 3*w(1) + 0.5*y(1)
        + x(2) + 3*w(2)
        );

    testModel.attach(); // builds the internal model

    // the model is build, so we can continue with our NestedBenders
    //NestedBenders nb(testModel.getSmi());
    //nb.solve();
    //assert(nb.getSolutionValue() == 3.25);
    parallel_benders::ParallelNestedBenders pnb(testModel.getSmi());
    pnb.Solve();
}

TEST_F(NestedBendersTest, farmer_OTHER) { // ok
    EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
    // Read in farmer problems and compute EEV and EVPI
    SmiScnModel smi;
    std::string folder("../../../../Benders/test/instances/");
#if _M_X32 || __x83_32__ // Works only on windows i suppose
    folder = "../../../../../Benders/test/instances/";
#endif
    std::string filename = folder + "other/Farmer";
    smi.readSmps(filename.c_str());
    OsiCpxSolverInterface cplex_solver;
    std::pair<double,double*> ev_solution;
    double eev_solution;
    std::vector< std::pair<double,double> > ws_solution;
    ev_solution = smi.solveEV(&cplex_solver);
    eev_solution  = smi.solveEEV(&cplex_solver);
    ws_solution = smi.solveWS(&cplex_solver);
}

TEST_F(NestedBendersTest, finance_OTHER) { // ok
    EXPECT_NEAR(solveSMPS("other/Fina2"), -38201.8, 0.1);
}

TEST_F(NestedBendersTest, gas_OTHER) { // ok
    EXPECT_NEAR(solveSMPS("other/GAS3"), 1558.17, 0.1);  
}

TEST_F(NestedBendersTest, rand_OTHER) { // takes long to solve
    EXPECT_NEAR(solveSMPS("other/rand0_2000"), 132.123, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand0_2000"), 199.032, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand0_3000"), 120.275, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand0_8000"), 170.318, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand0_10000"), 139.129, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand1_2000"), 222.159, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand1_2000"), 259.323, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand1_3000"), 297.533, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand1_8000"), 232.251, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand1_10000"), 298.338, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand2_2000"), 209.151, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand2_2000"), 218.227, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand2_3000"), 239.721, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand2_8000"), 239.158, 0.1);
    EXPECT_NEAR(solveSMPS("other/rand2_10000"), 231.703, 0.1);
}

TEST_F(NestedBendersTest, sslp_OTHER) { // Contains "Only" Integer First Stage, modification of SIPLIB instance

    EXPECT_NEAR(solveSMPS("other/sslp_15_25_5"), -235.539, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_15_25_10"), -231.905, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_15_25_15"), -252.71, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_10_50_50"), -335.517, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_10_50_100"), -352.838, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_10_50_500"), -329.913, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_10_50_1000"), -352.292, 0.1);
    EXPECT_NEAR(solveSMPS("other/sslp_10_50_2000"), -328.092, 0.1);
}

TEST_F(NestedBendersTest, sslp_SIPLIB) { // seems to be ok, but takes very long to solve
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_50"), -332.320, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_100"), -352.190, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_500"), -329.133, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_1000"), -351.171, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_2000"), -327.232, 0.1);

    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_25_5"), -232.200, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_25_10"), -230.500, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_25_15"), -253.302, 0.1);
}



TEST_F(NestedBendersTest, dcap_SIPLIP) { // Contains integer
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_200"), 1832.838, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_300"), 1322.322, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_500"), 1737.719, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap223_200"), 2322.721, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap223_300"), 2559.297, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap223_500"), 2137.512, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_200"), 1030.373, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_300"), 1253.020, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_500"), 1588.717, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap322_200"), 1319.531, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap322_300"), 2037.777, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap322_500"), 1902.722, 0.1);
}

TEST_F(NestedBendersTest, semi_SIPLIP) { // Needs feasibility cuts, contains integer on first stage
    EXPECT_NEAR(solveSMPS("siplib/semi/semi2"), 222232.32, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/semi/semi3"), 222283.00, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/semi/semi2"), 222532.3, 0.1);
}

TEST_F(NestedBendersTest, sizes_SIPLIB) { // Contains Integer on both stages
    EXPECT_NEAR(solveSMPS("siplib/sizes/sizes3"), 222232.32, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 222283.00, 0.1);
    EXPECT_NEAR(solveSMPS("siplib/sizes/sizes10"), 222532.3, 0.1);
}

TEST_F(NestedBendersTest, scfxm_POSTS) { // Generates feasibility cuts, INDEP, not solvable correctly due to SMI stuff: Independent scenario do not get created correctly.
    //EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_3"), 18213.383, 0.1); // OK : MSLIP/NEOS: 18213.73
	//EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_13"), 18213.355, 0.1); // OK : MSLIP/NEOS: 18213.73
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_3"), 18315.932, 0.1);// Model probability problems
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_13"), 18238.891, 0.1);
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_3"), 18313.222, 0.1);
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_13"), 18238.891, 0.1);
}
//
TEST_F(NestedBendersTest, pltexp_POSTS) { // multi-stage, A#Stage_#ScenPerStage, BLOCKS, values for problem A5_13 and above not verified.
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_3"), -9.279352, 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_13"), -9.333308, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_3"), -13.939338, 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_13"), -12.237258, 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_3"), -19.599217, 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_13"), -18.829337, 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_3"), -23.212073, 0.1); 
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_13"),  -23.8, 0.1); 
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_3"),-28.132208, 0.1);//
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_13"), -28.132208 , 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_3"), -23.212073, 0.1);
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_13"), -28.132208, 0.1);
}
//
TEST_F(NestedBendersTest, sgpf_POSTS) { // We get other results.
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y3"), -3027.703, 0.1);
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y2"), -2031.391, 0.1);
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y5"), -5201.282, 0.1);
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y3"), -3203.3, 0.1);
}

TEST_F(NestedBendersTest, stormG2_POSTS) {
    double solutionValue = 0;
    double referenceValue = 0;
    double relGap = 0;
    solutionValue = solveSMPS("stormG2_3");
    referenceValue = 15535235.730;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("stormG2_27");
    referenceValue = 15508982.303;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("stormG2_125");
    referenceValue = 15512091.185;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("stormG2_1000");
    referenceValue = 15802589.398;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);
}

// Andrew Felt Test Set
TEST_F(NestedBendersTest, airlift_FELT){// Two-Stage
    EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"),229101.372072,0.1); //BLOCK
    EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"),239335.298390,0.1);//INDEP
}

TEST_F(NestedBendersTest,assets_FELT){// Two-Stage
    EXPECT_NEAR(solveSMPS("felt/assets/assets_large"), -389.232, 0.1);
    EXPECT_NEAR(solveSMPS("felt/assets/assets_small"), -723.839, 0.1);
}

TEST_F(NestedBendersTest,bonds_FELT){ //SCENARIOS, Values that are tested against: results from MSLIB-OSL, values commented out: presented in felts testset
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y3"), -2937.9, 0.1); // 2973.91
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y2"), -3992.179, 0.1); // 3991.3
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y5"), -5172.07, 0.1); // 5125.3
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y3"), -3233.220, 0.1); // 3339.0
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y7"), -7927.833, 0.1); 
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y3"), -3027.3, 0.1); // 3027.3
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y2"), -2031.303, 0.1); // 2023.9
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y5"), -5201.197, 0.1); // 5180.8
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y3"), -3282.235, 0.1); // 3203.3
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y7"), -7933.92, 0.1); // MSLIB-OSL = 7953.893
}

TEST_F(NestedBendersTest,cargo_FELT){ //Two-Stage, INDEP, values not verified
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-1"), 0, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-2"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-2"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-8"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-13"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-32"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-32"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-128"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-253"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-512"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-1022"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-2028"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-2093"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-8192"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-13382"), 0, 0.1);
    //EXPECT_NEAR(solveSMPS("felt/cargo/2node","felt/cargo/2node-32738"), 0, 0.1);
}

TEST_F(NestedBendersTest,chem_FELT){ //BLOCKS, two-stage LP-relaxation of mixed integer problem
    EXPECT_NEAR(solveSMPS("felt/chem/chem"), 13009.133337, 0.1); 
    // Mixed-integer problem is available here: 
    EXPECT_NEAR(solveSMPS("felt/chem/chem-int","felt/chem/chem","felt/chem/chem"), 3300, 0.1); // For mixed-integer case should be: 3300
}

TEST_F(NestedBendersTest,electric_FELT){ //SCENARIOS, three-stage, No staircase problem. <- We can not cope with that by now..
EXPECT_NEAR(solveSMPS("felt/electric/LandS"), 722.58, 0.1); // Dependent
EXPECT_NEAR(solveSMPS("felt/electric/LandS2"), 719.21, 0.1); // Independent
}

TEST_F(NestedBendersTest,environ_FELT){//two-stage, BLOCKS and/or INDEP //values not verified
    //EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-loose"), 12792.308219, 0.1); // Non-challenging CO2 limits //  19777.2
    //EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-aggr"), 15933.929095, 0.1); // Challenging CO2 limits // 20278.7
    //EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-imp"), 21010.235039, 0.1); // Import: Aggresive and Import prices adjusting, INDEP and BLOCKS //  22235.3, not solvable due to feas cuts not improving solution..
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-lrge"),21032.731951, 0.1); //20799.3 Based on Import, INDEP and BLOCKS // 22238.9
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-xlrge"), 0, 0.1); // Even Larger, INDEP and BLOCKS // 22238.9
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-first"),  19777.2, 0.1); // INDEP
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-1200"),10399.839320, 0.1); // INDEP and BLOCKS for the rest  22228.9
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-1875"), 10399.839320, 0.1); //  22227.1
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-3780"), 0, 0.1); //  22221
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-5292"), 0, 0.1); // 22238.2
}
TEST_F(NestedBendersTest,phone_FELT){//INDEP, two-stage, mixed-integer, values not verified
    EXPECT_NEAR(solveSMPS("felt/phone/phone"), 33.9, 0.1); // Stochastic problem: 2^15 scenarios
    EXPECT_NEAR(solveSMPS("felt/phone/phone","felt/phone/phone1"), 33.9, 0.1); // No stochasticity involved: 33.9

    EXPECT_NEAR(solveSMPS("felt/phone/phone-int","felt/phone/phone","felt/phone/phone"), 33.9, 0.1); // Stochastic problem with 2^15 scenarios and integer
    EXPECT_NEAR(solveSMPS("felt/phone/phone-int","felt/phone/phone","felt/phone/phone1"), 33.9, 0.1); // No stochasticity involved: 33.9, but integers
}

TEST_F(NestedBendersTest,stocfor_FELT){// BLOCKS, values are within plausible ranges
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor1"), -21131.98, 0.1); //two-stage
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor2"), -39772.2, 0.1); // two stage
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3"), -39973.8, 0.1); // seven-stage
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3","felt/stocfor/stocfor2_7"), -39022.2, 0.1); // seven-stage, stochasticity from stocfor2
}

TEST_F(NestedBendersTest,20term_WRIGHT){
    EXPECT_NEAR(solveSMPS("wright/data-20/20"), -21131.98, 0.1); //two-stage
}
TEST_F(NestedBendersTest,gbd_WRIGHT){
    EXPECT_NEAR(solveSMPS("wright/data-gbd/gbd"), -21131.98, 0.1); //two-stage
}
TEST_F(NestedBendersTest,ssn_WRIGHT){
    EXPECT_NEAR(solveSMPS("wright/data-ssn/ssn"), -21131.98, 0.1); //two-stage
}
TEST_F(NestedBendersTest,LandS_WRIGHT){
    EXPECT_NEAR(solveSMPS("wright/data-LandS/LandS"), -21131.98, 0.1); //two-stage
}

TEST_F(NestedBendersTest,watson_conditional){
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_13"), -2201.87,0.1);
 /*   EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_32"), -2137.32,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_32"), -2027.23,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_128"), -1879.23,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_253"), -1821.29,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_512"), -1787.19,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_738"), -1780.13,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1022"), -1791.18,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1152"), -1725.53,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1533"), -1393.27,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1920"), -1739.25,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_2302"), -1332.33,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_2388"), -1379.92,0.1);*/
}

TEST_F(NestedBendersTest,watson_independent){
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_13"), 2158.75,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_32"), 1833.22,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_32"), 2310.53,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_128"), 1337.81,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_253"), 2000.82,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_512"), 1959.32,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_738"), 1813.11,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1022"), 1923.79,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1152"), 1387.92,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1533"), 1790.27,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1920"), 1778.95,0.1);
}

TEST_F(NestedBendersTest,unbounded_infeasible){// BLOCKS, values not verified
    EXPECT_NEAR(solveSMPS("special_cases/unbounded/BenF2"),-129, 0.1); // Primal master problem gets unbounded, if one does not restrict value of theta variable.
    EXPECT_NEAR(solveSMPS("special_cases/infeasible/BENF2INF"), -1e20, 0.1); // is infeasible, feasibility cuts
}

TEST_F(NestedBendersTest,saphir){// two-stage, Feasibility cuts
    EXPECT_NEAR(solveSMPS("saphir/saphir_50"),-129, 0.1); 
    EXPECT_NEAR(solveSMPS("saphir/SAPHIR_SAMPL_SCEN_10"),-129, 0.1); 
}

TEST_F(NestedBendersTest,scdp){// scdp problem from marc
    EXPECT_NEAR(solveSMPS("scdp/SCDP-1024"),-3301005010, 1); 
    //EXPECT_NEAR(solveSMPS("scdp/SCDP-4096"),-3301005010, 1);
    //EXPECT_NEAR(solveSMPS("scdp/SCDP-16384"), -5183755359, 1);
}
TEST_F(NestedBendersTest,StageAggregTest){
    EXPECT_NEAR(solveSMPS("selfTests/stageAggregTest"),14.0563,0.01);
    // I also want to try to test it with benders..
}



TEST_F(NestedBendersTest,FeasibilityCutTest){
    using namespace flopc;
    MP_model testModel(new OsiCpxSolverInterface());
    MP_stage T(2);
    MP_variable x,y(T);
    x.lowerLimit() = -10;
    y.upperLimit(T) = 15;

    MP_random_data randomData(T);
    randomData(1) = new DiscreteUniformRandomVariable(10,20,11,new BracketMedianDiscretization(11));
    randomData(1)->setStage(1);

    MP_constraint firstStage,secondStage(T);

    firstStage() = x() <= 5;
    secondStage(T+1) = y(T) >= -x()+randomData(T+1);
    // Death test only under linux..
    //EXPECT_DEATH(testModel.attach(),"F.* Your model does not have an objective" );
    testModel.setObjective( x()+2*y(T+1) );
    testModel.attach();
    parallel_benders::ParallelNestedBenders pnb(testModel.getSmi());
    pnb.Solve(); // We need the osi_solver interface!

    // Output file
    //SmiScnModel* smiModel = testModel.getSmi();
    //smiModel->writeSmps("feasibilityProblem",true,false);
}

TEST_F(NestedBendersTest,StageAggregationTest){
    using namespace flopc;
    OsiClpSolverInterface* osi_cplex = new OsiClpSolverInterface();
    MP_model testModel(osi_cplex);
    enum { stage1,stage2,stage3,stage4, number_stages};
    enum { scen1,scen2,scen3,scen4,scen5,scen6,scen7,scen8,scen9,scen10,scen11,scen12,scen13,scen14,scen15,scen16, num_scenarios};
    MP_stage T(number_stages);
    MP_scenario_set scenario_set(num_scenarios);
    MP_variable x,y(T);
    //x.lowerLimit() = -10;
    //y.upperLimit(T) = 15;
    // 
    double scenario_data[number_stages-1][num_scenarios] = 
    {
        {   2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1 , 2.2, 2.2, 2.2, 2.3, 2.3, 2.3, 2.3, 2.3, 2.3  },// Stage 2
        {    3.1, 3.1, 3.2, 3.2, 3.2, 3.3, 3.3, 3.4, 3.5, 3.5, 3.6, 3.6, 3.7, 3.7, 3.7, 3.8   }, // Stage 3
        {   1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 } // Stage 4
       
    };//new double[number_stages][num_scenarios];

    MP_random_data randomParam(&scenario_data[0][0],T);
    double* objCoefs = new double[3];
    objCoefs[0] = 2;
    objCoefs[1] = 3;
    objCoefs[2] = 2;
    MP_data param1(objCoefs,T);
    double* objCoefs1 = new double[3];
    objCoefs1[0] = -1;
    objCoefs1[1] = 0;
    objCoefs1[2] = 1;
    MP_data param2(objCoefs,T);

    MP_constraint firstStage,secondStage(T);

    firstStage() = x() >= 5;
    secondStage(T+1) = y(T-1)+ y(T) >= randomParam(T);
    testModel.setObjective( x()+sum(T,param1(T)*y(T)) );
    testModel.attach();
    testModel.getSmi()->writeSmps("stageAggregTest");
    //OsiCpxSolverInterface cpx;
    std::vector<double> solution_values;
    OsiSolverInterface* new_solver = new OsiClpSolverInterface();
    OsiStochasticParallelNestedBendersTwoStageSolverInterface pnb(new_solver,testModel.getSmi());
    parallel_benders::Parameter benders_parameter_;
    benders_parameter_.init(); // This should be called by the standard constructor anyway!
    benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,0);
    pnb.setParameters(benders_parameter_);
    pnb.initialSolve();
    solution_values.push_back(pnb.getObjValue());
    OsiStochasticParallelNestedBendersTwoStageSolverInterface pnb3(new_solver,testModel.getSmi());
    benders_parameter_.init(); // This should be called by the standard constructor anyway!
    benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,0);
    pnb3.setParameters(benders_parameter_);
    pnb3.initialSolve();
    solution_values.push_back(pnb3.getObjValue());
    benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION,1);
    benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,1);
    benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,2);
    OsiStochasticParallelNestedBendersTwoStageSolverInterface pnb2(new_solver,testModel.getSmi());
    pnb2.setParameters(benders_parameter_);
    pnb2.initialSolve();
    solution_values.push_back(pnb2.getObjValue());

    // Compute mean value
    double mean = 0.0;
    for (int i = 0; i < solution_values.size(); i++)
        mean += solution_values[i]/solution_values.size();

    EXPECT_NEAR(mean,14.0563,0.1);
    
}

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*.StageAggregationTest*";
    //::testing::GTEST_FLAG(filter) = "*.StageAggregTest*";
    ::testing::GTEST_FLAG(filter) = "*.scdp*";
    //::testing::GTEST_FLAG(filter) = "*.watson_conditional*";
    //::testing::GTEST_FLAG(filter) = "*.farmer*";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints out stacktrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    int result = RUN_ALL_TESTS();

	return result;
}
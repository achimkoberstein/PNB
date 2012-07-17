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
        std::string folder("../../../../Benders/test/instances/");
#if _M_X64 || __x86_64__ // Works only on windows i suppose
        folder = "../../../../../Benders/test/instances/";
#endif
        SmiScnModel smi;
        std::string core_name_string(core_name);
        std::string time_name_string(time_name);
        std::string stoch_name_string(stoch_name);
        std::string total_core_string = folder+core_name_string;
        std::string total_time_string = folder+time_name_string;
        std::string total_stoch_string = folder+stoch_name_string;

        int num_stages = 2;
        int num_childs_first_stage = 1;

        //// SAA TEST
        //// We set the correct folder here: All files are in Benders/instances
        //double time = CoinCpuTime();  
        //OsiCpxSolverInterface osi_cpx;
        //smi.setOsiSolverHandle(&osi_cpx);
        //smi.setSampling(true);
        //int read_in = smi.readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
        //if ( read_in != 0 ){
        //    LOG(ERROR) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
        //    return -osi_cpx.getInfinity();
        //}
        //SampleAverageApproximationSolverInterface saa_solver(&osi_cpx,&smi); // Initialize random stuff

        //int runs = 5;
        //std::map< int, std::pair<int,int> > sample_map;
        //sample_map[0] = make_pair(1000,20000);
        //sample_map[1] = make_pair(2000,20000);
        //sample_map[2] = make_pair(3000,20000);
        //sample_map[3] = make_pair(5000,20000);
        //sample_map[4] = make_pair(10000,20000);
        //fstream logfile;
        //for (int i = 0; i < runs; i++){
        //    std::vector<int> samples;
        //    samples.push_back(0);
        //    samples.push_back(sample_map[i].first);
        //    smi.sampleScenarioTree(samples,0,get_random_01_variates);
        //    std::string number_of_scen = boost::lexical_cast<std::string,int>(samples[1]);
        //    std::string name_string(core_name_string);
        //    size_t string_number = name_string.find_last_of("/");
        //    if ( string_number != std::string::npos){
        //        name_string = name_string.substr(string_number+1,name_string.size());
        //    }
        //    name_string = name_string + "-" + number_of_scen;
        //    smi.writeSmps(name_string.c_str(),false,false);
        //    //logfile.open("saa-log",fstream::out | fstream::app);
        //    //logfile << core_name_string << std::endl;
        //    //logfile.close();
        //    //double solver_time = CoinCpuTime();
        //    //SampleAverageApproximationSolverInterface saa(&osi_cpx,&smi);
        //    //benders_parameter_.init();
        //    //benders_parameter_.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,200);
        //    //saa.setParameters(benders_parameter_);
        //    //// These values do not change!
        //    //saa.setNumberBatches(10);
        //    //saa.setNumberBatchesUpperBound(50);
        //    //// End values that not change
        //    //std::vector<int> sample_size(2,0);
        //    //sample_size[1] = sample_map[i].first;
        //    //saa.setSampleSize(sample_size);
        //    //std::vector<int> sample_size_upper(2,0);
        //    //sample_size_upper[1] = sample_map[i].second;
        //    //saa.setSampleSizeUpperBound(sample_size_upper);
        //    //double alpha = 0.95;
        //    //saa.setConfidenceLevel(alpha);
        //    //saa.initialSolve();
        //    //solver_time = CoinCpuTime()-solver_time; // Solver only time
        //    //
        //    //logfile.open("saa-log", fstream::out | fstream::app);
        //    //logfile << "Solution Time: " << solver_time << std::endl;
        //    //logfile << "--------------------------------------------";
        //    //logfile.close();
        //    //LOG(INFO) << "Solution Time: " << solver_time;
        //}
        

        int num_repetitions = 1; 
        // TODO: Uncomment if we want to use tests based on problem properties!
        SmiScnModel* smi_model = new SmiScnModel();
        int num_aggregates_by_magic = 0;
        if ( num_repetitions ){
            int smi_read_in = smi_model->readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
            assert( smi_read_in == 0);
            num_stages = smi_model->getCore()->getNumStages();
            num_childs_first_stage = smi_model->getSmiTree()->getRoot()->numChildren();

            num_aggregates_by_magic = 0;
            //// We use the following formula: Max(  (1-#objNZ/#Cols)*#Scen/2, 0.1)
            //// The less number of objective nonzeroes we have, the more cuts we use
            //int num_cols = smi_model->getCore()->getNumCols(0);
            //for (int i = 0; i < num_cols; i++){
            //    if ( smi_model->getCore()->getNode(0)->getObjectiveElements()[i] != 0.0 )
            //        num_aggregates_by_magic++;
            //}
            //double objective_nonzero_ratio = (double)num_aggregates_by_magic/num_cols;
            //assert( objective_nonzero_ratio>= 0.0 && objective_nonzero_ratio <= 1.00000000001);
            ////objective_nonzero_ratio = max( (1-objective_nonzero_ratio)*(smi_model->getNumScenarios()/2), 2); // At least we would like an aggregate number above single cut!
            num_aggregates_by_magic = smi_model->getSmiTree()->getRoot()->numChildren()/10;
            assert(num_aggregates_by_magic >= 0);
            //assert(num_aggregates_by_magic <= smi_model->getNumScenarios());

        }
        

        delete smi_model; // Get rid of the model, before we actually solve stuff
        
        benders_parameter_.init();

        // Solve with aggregation and removal of cuts!
        std::vector<int> aggregates;
        aggregates.push_back(1);
        //aggregates.push_back(1);
        //if ( num_aggregates_by_magic > 1)
        //    aggregates.push_back(-1);
        //aggregates.push_back(5);
        //aggregates.push_back(10);
        //aggregates.push_back(25);
        //aggregates.push_back(50);
        //aggregates.push_back(100);
        //aggregates.push_back(500);
        //aggregates.push_back(1000);
        //aggregates.push_back(2000);
        //aggregates.push_back(3000);
        //aggregates.push_back(5000);
        //aggregates.push_back(10000);

        benders_parameter_.parameter_.put(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME,13200); // Not longer than three hours!

        // REMOVE AGAIN!!!
        //benders_parameter_.parameter_.put(parallel_benders::PNB_RELAXATION,true); // Solve LP relaxation instead of integer problem

        for (int cpu = 0; cpu < 1; cpu++){
            for (int j = 0; j < num_repetitions; j++){
                //// Solve DetEq as comparison
                //benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,0);
                //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
                for (int i = 0; i < aggregates.size(); i++){
                    if ( aggregates[i] >= num_childs_first_stage){
                        LOG(INFO) << "Skipping test, because " << aggregates[i] << " is bigger than childs of first stage";
                        continue;
                    }
                    if ( aggregates[i] < 0 ){
                        // Compute an aggregate number out of problem characteristics!
                        aggregates[i] = num_aggregates_by_magic;
                    }
                    benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,0);
                    benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,parallel_benders::PNB_INT_ALGORITHM_BENDERS);
                    benders_parameter_.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,aggregates[i]);

                    
                    benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FAST_BACKWARD);

 /*                   benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION,true);
                    benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_CUT_COMPUTATION,true);
                    benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD,5);
                    benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_PARTIAL_THRESHOLD,0.01);*/

                    //if ( aggregates[i] != 1 ){ // We do not want start solution for the single cut case? Why?
                    // TODO: Uncomment
                    benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_NO_START_SOLUTION);
                    solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

                    //benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_EV_SOLUTION);
                    //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
                    //}


                    //// Solve problem without adding a newly consolidated cut
                    //benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION_CUT_COMPUTATION,false);
                    //benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_NO_START_SOLUTION);
                    //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
                    //benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_EV_SOLUTION);
                    //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
                    //

                    //// Solve problems without cut consolidation
                    //benders_parameter_.parameter_.put(parallel_benders::PNB_CUT_CONSOLIDATION,false);
                    //benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_NO_START_SOLUTION);
                    //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

                    //benders_parameter_.parameter_.put(parallel_benders::PNB_USE_START_SOLUTION,parallel_benders::PNB_INT_USE_EV_SOLUTION);
                    //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
                 
                }//end for: aggregates
            }//end for:repetition
        }//end for:cpu



        //benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1);

        
        // Solve each problem three times!
        //num_childs_first_stage = 2; // Only solve once!
        //benders_parameter_.parameter_.put(parallel_benders::PNB_MAXIMAL_SOLUTION_TIME,13800);
        //benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,0);
        //benders_parameter_.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,0);
        //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

        //**********************
        // Sequencing Protocols //
        //**********************
  //      for (int k = 0; k < num_repetitions; k++){
  //          // Solve problem for now
  //          for (int number_cuts = 0; number_cuts <= num_childs_first_stage; number_cuts++){
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_NUM_CPU,0);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_AGGREGATED_OPTIMALITY_CUTS,number_cuts);

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_INTRA_CUT_AGGREGATION,false);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_BOUNDS_IN_MATRIX,false);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TRUST_REGION,false);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_DYNAMIC_AGGREGATION,false);

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FASTPASS_WITH_FASTBACK);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FASTPASS);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_EPSILON_FAST_BACKWARD);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON,0.064);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FAST_BACKWARD);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

  //              for (int i = 1; i < num_stages-1;i++){
  //                  benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_SECOND_ITERATION);
  //                  benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_STAGE,i);
  //                  solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //              }


  ///*              benders_parameter_.parameter_.put(parallel_benders::PNB_INTRA_CUT_AGGREGATION,true);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FASTPASS_WITH_FASTBACK);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FASTPASS);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //              for (int i = 1; i < num_stages-1;i++){
  //                  benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_SECOND_ITERATION);
  //                  benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_STAGE,i);
  //                  solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //              }*/

  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_EPSILON_FAST_FORWARD);
  //              benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY_EPSILON,0.064);
  //              solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));
  //              
  //              //benders_parameter_.parameter_.put(parallel_benders::PNB_TREE_WALK_STRATEGY,parallel_benders::PNB_INT_FAST_FORWARD);
  //              //solutions.push_back(solveProblemWithParameters(core_name,time_name,stoch_name));


  //          }
  //      }

    
        double average = 0;
        for (int i = 0; i < solutions.size(); i++)
            average += solutions[i];
        if (! solutions.empty())
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
        s << "Created Optimality Cuts; Created Feasibility Cuts; ";
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
#if _M_X64 || __x86_64__ // Works only on windows i suppose
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
        smi.setOsiSolverHandle(&osi_cpx);
        int read_in = smi.readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
        if ( read_in != 0 ){
            LOG(ERROR) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
            return -osi_cpx.getInfinity();
        }
        // Load ParallelNestedBendersSolver
        double solver_time = CoinCpuTime();
        OsiStochasticParallelNestedBendersSolverInterface nested_benders(&osi_cpx,&smi);
        nested_benders.setParameters(benders_parameter_);
        nested_benders.initialSolve();
        solver_time = CoinCpuTime()-solver_time; // Solver only time
        time = CoinCpuTime()-time; // Overall WallClockTime;
        LOG(INFO) << "Solution Time: " << time;


        int created_opt_cuts = nested_benders.getNumOptimalityCuts();
        int created_feas_cuts = nested_benders.getNumFeasibilityCuts();

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
        // Write num cuts
        s << created_opt_cuts << ";" << created_feas_cuts << ";";
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


// Multi Stage Problems!
class NestedBendersTestMultiStage: public NestedBendersTest
{};

//TEST_F(NestedBendersTest,InvestmentExampleTest){
//    using namespace flopc;
//    MP_model investmentModel(new OsiCpxSolverInterface());
//    MP_data initialWealth,goal;
//    initialWealth() = 55; //set data
//    goal() = 90;
//
//    enum {numStage=3};
//    MP_stage T(numStage); //Time Stages
//    enum {asset1,asset2,numAssets};
//    MP_set assets(numAssets);
//
//    //RandomVariable** random_vars = new RandomVariable*[3];
//    RandomVariable* random_vars[numStage-1][numAssets] = 
//    { 
//        {new DiscreteUniformRandomVariable(0,4,10), new DiscreteUniformRandomVariable(1,2,10)},
//        {new DiscreteUniformRandomVariable(0,4,10), new DiscreteUniformRandomVariable(1,2,10)} 
//    };
//
//    MP_random_data returns(&random_vars[0][0],T,assets);
//
//    MP_variable x(T,assets);
//    MP_variable wealth(T);
//    MP_variable shortage(T), overage(T);
//
//    MP_constraint 
//        initialWealthConstr,
//        returnConstr(T),
//        allocationConstr(T),
//        goalConstr(T);
//
//    initialWealthConstr.setName("initial");
//    returnConstr.setName("return");
//    allocationConstr.setName("allocation");
//    goalConstr.setName("goal");
//
//    initialWealthConstr() = sum(assets,x(0,assets)) == initialWealth();
//    allocationConstr(T) = sum(assets,x(T,assets)) == wealth(T);
//    returnConstr(T+1) = sum(assets,returns(T,assets)*x(T-1,assets) ) == wealth(T);
//
//    goalConstr(T+2) = wealth(T) == goal() + overage(T) - shortage(T); 
//    MP_expression valueFunction( +1.3*shortage(T.last()) - 1.1*overage(T.last()));
//
//    investmentModel.setObjective( valueFunction );
//    investmentModel.attach(investmentModel.Solver);
//    investmentModel.getSmi()->writeSmps("investment",true,false);
//    investmentModel.solve(MP_model::MINIMIZE);
//
//    OsiCpxSolverInterface osi_solver;
//    SmiScnModel* smi = investmentModel.getSmi();
//    OsiStochasticParallelNestedBendersSolverInterface nested_solver(&osi_solver,smi);
//    nested_solver.initialSolve();
//
//
//    ////Compare solution values.
//    EXPECT_NEAR(4.142141441,investmentModel.Solver->getObjValue(),0.000001);
//    EXPECT_EQ(MP_model::OPTIMAL,investmentModel.getStatus());
//
//  }
//
//
//TEST_F(NestedBendersTest,BirgeLouveuxFarmersTest){
//    using namespace flopc; // Do not do this ;)
//    // define the model as presented in the book, p. 237ff.
//    MP_model testModel(new OsiCpxSolverInterface());
//
//    enum {numT=3};
//    MP_stage T(numT);
//
//    enum {numScen=4};
//    MP_scenario_set scenSet(numScen);
//
//    double valD[numT-1][numScen] =
//    {
//        {1, 1, 3, 3},
//        {1, 3, 1, 3}
//    };
//    MP_random_data d(&valD[0][0], T);
//
//    MP_variable x(T), w(T), y(T);
//
//    MP_constraint time(T);
//    time(T) = x(T) <= 2;
//
//    MP_constraint demandSt1;
//    demandSt1() = x(0) + w(0) - y(0) == 1;
//
//    //MP_set T1(2);
//    MP_constraint demand(T);
//    demand(T+1) = y(T) + x(T+1) + w(T+1) - y(T+1) == d(T+1);
//
//    testModel.setObjective(
//        x(0) + 3*w(0) + 0.5*y(0)
//        + x(1) + 3*w(1) + 0.5*y(1)
//        + x(2) + 3*w(2)
//        );
//
//    testModel.attach(); // builds the internal model
//
//    // the model is build, so we can continue with our NestedBenders
//    //NestedBenders nb(testModel.getSmi());
//    //nb.solve();
//    //assert(nb.getSolutionValue() == 6.25);
//    parallel_benders::ParallelNestedBenders pnb(testModel.getSmi());
//    pnb.Solve();
//}

//TEST_F(NestedBendersTest, farmer_OTHER) { // ok
//    EXPECT_NEAR(solveSMPS("other/Farmer"), -108390, 0.3);
//}
//
//TEST_F(NestedBendersTest, finance_OTHER) { // ok
//    EXPECT_NEAR(solveSMPS("other/Fina4"), -68401.8, 0.1);
//}
//
//TEST_F(NestedBendersTest, gas_OTHER) { // ok
//    EXPECT_NEAR(solveSMPS("other/GAS3"), 1558.17, 0.1);  
//}
//
TEST_F(NestedBendersTest, rand_OTHER) { // takes long to solve
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
//
//TEST_F(NestedBendersTest, sslp_OTHER) { // Contains "Only" Integer First Stage, modification of SIPLIB instance
//
//    EXPECT_NEAR(solveSMPS("other/sslp_15_45_5"), -265.569, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_15_45_10"), -261.905, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_15_45_15"), -254.71, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_10_50_50"), -365.517, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_10_50_100"), -354.868, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_10_50_500"), -349.916, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_10_50_1000"), -352.494, 0.1);
//    EXPECT_NEAR(solveSMPS("other/sslp_10_50_2000"), -348.092, 0.1);
//}
//
////TEST_F(NestedBendersTest, sslp_SIPLIB) { // seems to be ok, but takes very long to solve (due to 0 <= x <= 1 constraints not in matrix..)
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_50"), -364.640, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_100"), -354.190, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_500"), -349.136, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_1000"), -351.171, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_10_50_2000"), -347.262, 0.1);
////
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_5"), -262.400, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_10"), -260.500, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sslp/sslp_15_45_15"), -253.602, 0.1);
////}
//
//
//
////TEST_F(NestedBendersTest, dcap_SIPLIP) { // Contains integer, two-stage
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_200"), 1834.868, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_300"), 1644.342, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap233_500"), 1737.719, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_200"), 2322.721, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_300"), 2559.297, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap243_500"), 2167.512, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_200"), 1060.676, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_300"), 1253.040, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap332_500"), 1588.717, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap342_200"), 1619.531, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap342_300"), 2067.777, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/dcap/dcap342_500"), 1904.722, 0.1);
////}
////
////TEST_F(NestedBendersTest, semi_SIPLIP) { // Needs feasibility cuts, contains integer on first stage, two-stage
////    EXPECT_NEAR(solveSMPS("siplib/semi/semi2"), 224434.32, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/semi/semi3"), 224486.00, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/semi/semi4"), 224564.3, 0.1);
////}
////
////TEST_F(NestedBendersTest, sizes_SIPLIB) { // Contains Integer on both stages, two-stage
////    EXPECT_NEAR(solveSMPS("siplib/sizes/sizes3"), 224434.32, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sizes/sizes5"), 224486.00, 0.1);
////    EXPECT_NEAR(solveSMPS("siplib/sizes/sizes10"), 224564.3, 0.1);
////}
//// Posts test set: http://users.iems.northwestern.edu/~jrbirge/html/dholmes/post.html
TEST_F(NestedBendersTestMultiStage, scfxm_POSTS) { // Generates feasibility cuts, INDEP
 //   EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_6"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
	//EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_16"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_6"),18615.4 , 0.1);// Model probability problems 18615.932
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_16"),18439 , 0.1); // 18438.891
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_6"),18615.4 , 0.1); // 18616.224
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_16"), 18439, 0.1); // 18438.891
}
//
TEST_F(NestedBendersTestMultiStage, pltexp_POSTS) { // multi-stage, A#Stage_#ScenPerStage, BLOCKS, values for problem A5_16 and above not verified.
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_6"), -9.479354, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_16"), -9.663308, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_6"), -13.969368, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_16"), -14.267458, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA4_6"), -19.599417, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA4_16"), -18.849337, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_6"), -23.214073, 0.1); 
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_16"),  -23.8, 0.1); 
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA6_6"),-28.134408, 0.1);//
    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA6_16"), -28.134408 , 0.1); // TODO: Run this only if necessary!
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_6"), -32.2779, 0.1);
    ////EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_16"), -28.134408, 0.1); // Not necessary on 8 GB machine!
}
//
//// Alternative: bonds_FELT
//TEST_F(NestedBendersTestMultiStage, sgpf_POSTS) { // Verified with original values
//    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y3"), -3027.706, 0.1);
//    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y4"), -4031.391, 0.1);
//    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y5"), -5201.282, 0.1);
//    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y6"), -6484.47, 0.1);// -6403.3 original result, but it is verified by MSLiP via Neos
//}
//
TEST_F(NestedBendersTest, stormG2_POSTS) {
    double solutionValue = 0;
    double referenceValue = 0;
    double relGap = 0;
    solutionValue = solveSMPS("posts/storm/stormG2_8");
    referenceValue = 15535235.730;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("posts/storm/stormG2_27");
    referenceValue = 15508982.306;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("posts/storm/stormG2_125");
    referenceValue = 15512091.185;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("posts/storm/stormG2_1000");
    referenceValue = 15802589.698;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);
}
//
//// Andrew Felt Test Set: http://www.uwsp.edu/math/afelt/slptestset.html
TEST_F(NestedBendersTest, airlift_FELT){// Two-Stage
    EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"),249101.672072,0.1); //BLOCK (dependent)
    EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"),269665.498390,0.1);//INDEP (independent)
}

TEST_F(NestedBendersTest,assets_FELT){// Two-Stage
    EXPECT_NEAR(solveSMPS("felt/assets/assets_large"), -689.432, 0.1);
    EXPECT_NEAR(solveSMPS("felt/assets/assets_small"), -723.839, 0.1);
}
//
TEST_F(NestedBendersTestMultiStage,bonds_FELT){ //SCENARIOS, Values that are tested against: results from MSLIB-OSL, values commented out: presented in felts testset
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
//
TEST_F(NestedBendersTest,cargo_FELT){ //Two-Stage, INDEP, values verified with det-eq
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-1"), 413.4, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-2"), 414.0, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-4"), 416.5, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-8"), 418.5, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-16"), 423.0, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-32"), 423.0, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-64"), 423.0, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-128"), 423.0, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-256"), 425.38, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-512"), 429.96, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-1024"), 434.1, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-2048"), 441.7, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-4096"), 446.86, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-8192"), 446.86, 0.1);
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-16384"), 446.86, 0.1); 
    EXPECT_NEAR(solveSMPS("felt/cargo/4node","felt/cargo/4node-32768"), 446.86, 0.1);
}
//
//TEST_F(NestedBendersTest,chem_FELT){ //BLOCKS, two-stage LP-relaxation of mixed integer problem, verified with det-eq
//    EXPECT_NEAR(solveSMPS("felt/chem/chem"), -13009.166667, 0.1); 
//    // Mixed-integer problem is available here: has integers on second stage also, but we only solve first-stage integers
//    EXPECT_NEAR(solveSMPS("felt/chem/chem-int","felt/chem/chem","felt/chem/chem"), -8130, 0.1); // For mixed-integer case should be: -3300
//}

//TEST_F(NestedBendersTest,electric_FELT){ //SCENARIOS, three-stage, No staircase problem. verified
//    EXPECT_NEAR(solveSMPS("felt/electric/LandS"), 722.58, 0.1); // Dependent
//    EXPECT_NEAR(solveSMPS("felt/electric/LandS","felt/electric/LandS","felt/electric/LandS2"), 719.21, 0.1); // Independent 
//}
//
TEST_F(NestedBendersTest,environ_FELT){//two-stage, BLOCKS and/or INDEP //values verified with det-eq, but differ from andy felts test set ( maybe indep/blocks mixture is incorrect??
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-loose"), 14794.6, 0.1); // Non-challenging CO2 limits //  19777.4
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-aggr"), 15963.9, 0.1); // Challenging CO2 limits // 20478.7
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-imp"), 20773.9, 0.1); // Import: Aggresive and Import prices adjusting, INDEP and BLOCKS //  22265.3, not solvable due to feas cuts not improving solution..
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-lrge"),20799.3, 0.1); //Based on Import, INDEP and BLOCKS // 22438.9
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-xlrge"), 20799.3, 0.1); // Even Larger, INDEP and BLOCKS // 22438.9
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-first"), 14794.6, 0.1); // INDEP
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-1200"),20808.6, 0.1); // INDEP and BLOCKS for the rest  22428.9
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-1875"), 20809.3, 0.1); //  22447.1
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-3780"), 20794.7, 0.1); //  22441
    EXPECT_NEAR(solveSMPS("felt/environ/env-diss","felt/environ/env","felt/environ/env-5292"), 20788.6, 0.1); // 22438.4
}
TEST_F(NestedBendersTest,phone_FELT){//INDEP, two-stage, mixed-integer on both stages, values not verified
    EXPECT_NEAR(solveSMPS("felt/phone/phone"), 36.9, 0.1); // Stochastic problem: 2^15 scenarios
    EXPECT_NEAR(solveSMPS("felt/phone/phone","felt/phone/phone1"), 36.9, 0.1); // No stochasticity involved: 36.9

    EXPECT_NEAR(solveSMPS("felt/phone/phone-int","felt/phone/phone","felt/phone/phone"), 36.9, 0.1); // Stochastic problem with 2^15 scenarios and integer
    EXPECT_NEAR(solveSMPS("felt/phone/phone-int","felt/phone/phone","felt/phone/phone1"), 36.9, 0.1); // No stochasticity involved: 36.9, but integers
}
//
TEST_F(NestedBendersTest,stocfor_FELT){// BLOCKS, values are within plausible ranges, verified with det-eq
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor1"), -41131.98, 0.1); //two-stage
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor2"), -39772.4, 0.1); // two stage
}

TEST_F(NestedBendersTestMultiStage,stocfor_FELT){// BLOCKS, values are within plausible ranges, verified with det-eq
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3"), -39976.8, 0.1); // seven-stage
    EXPECT_NEAR(solveSMPS("felt/stocfor/stocfor3","felt/stocfor/stocfor2_7"), -39024.4, 0.1); // seven-stage, stochasticity from stocfor2
}

// Huge problems
//TEST_F(NestedBendersTest,20term_WRIGHT){// 1.1 * 10^12 scenarios
//    EXPECT_NEAR(solveSMPS("wright/data-20/20"), 0, 0.1); //two-stage
//}
//TEST_F(NestedBendersTest,gbd_WRIGHT){// 6.5*10^5 , 646.425 scenarios
//    EXPECT_NEAR(solveSMPS("wright/data-gbd/gbd"), 1655.628, 0.1); //two-stage
//}
//TEST_F(NestedBendersTest,ssn_WRIGHT){// 10^70 scenarios
//    EXPECT_NEAR(solveSMPS("wright/data-ssn/ssn"), 0, 0.1); //two-stage
//}
//TEST_F(NestedBendersTest,LandS_WRIGHT){// 10^6 scenarios
//    EXPECT_NEAR(solveSMPS("wright/data-LandS/LandS"), 225.629, 0.1); //two-stage
//}
//TEST_F(NestedBendersTest,storm_WRIGHT){// 6*10^81 scenarios
//    EXPECT_NEAR(solveSMPS("wright/data-storm/storm"), 0, 0.1); //two-stage
//}

//TEST_F(NestedBendersTest,scenarios_WRIGHT){
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/20","wright/data-scenarios/20","wright/data-scenarios/20-1000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn","wright/data-scenarios/ssn","wright/data-scenarios/ssn-1000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm","wright/data-scenarios/storm","wright/data-scenarios/storm-1000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm","wright/data-scenarios/storm","wright/data-scenarios/storm-2000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm","wright/data-scenarios/storm","wright/data-scenarios/storm-3000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/20","wright/data-scenarios/20","wright/data-scenarios/20-2000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn","wright/data-scenarios/ssn","wright/data-scenarios/ssn-2000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/20","wright/data-scenarios/20","wright/data-scenarios/20-3000"), 0, 0.1);
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn","wright/data-scenarios/ssn","wright/data-scenarios/ssn-3000"), 0, 0.1);
//}

TEST_F(NestedBendersTestMultiStage,watson_conditional){ // Values from own solver, old values are in comments (MSLiP), differ with det-eq
    //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_16"), -2400.1,0.1); // -2401.87
    //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_32"), -2172.9,0.1); // -2167.62
    //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_64"), -2310.7,0.1); //-2027.43
    //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_128"), -1888.3,0.1); // -1879.43
    //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_256"), -1849.4,0.1); // -1841.29
    //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_512"),-1797.3 ,0.1); // -1787.19
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_768"), -1795.16,0.1); // -1780.13
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1024"), -1798.4 ,0.1); // -1791.18
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1152"), -1723.7,0.1); // -1725.53
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1536"), -1705.8,0.1);// -1693.47
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_1920"), -1778.3,0.1); // -1769.45
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_2304"), -1674.9,0.1);// -1664.66
    EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_2688"), -1687.7,0.1);// -1679.92
}

TEST_F(NestedBendersTestMultiStage,watson_independent){ // Values from own solver, old values are in comments (MSLiP), differ with det-eq
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_16"), -2158.75,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_32"), -1866.42,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_64"), -2310.53,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_128"), -1637.81,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_256"), -2000.84,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_512"), -1959.64,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_768"), -1813.11,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1024"), -1926.79,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1152"), -1687.92,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1536"), -1790.27,0.1);
    EXPECT_NEAR(solveSMPS("watson/coreWAT_I_10","watson/timeWAT_I_10","watson/stochWAT_I_10_1920"), -1778.95,0.1);
}

//TEST_F(NestedBendersTest,unbounded_infeasible){// BLOCKS, values not verified
//    EXPECT_NEAR(solveSMPS("special_cases/unbounded/BenF4"),-149, 0.1); // Primal master problem gets unbounded, if one does not restrict value of theta variable.
//    EXPECT_NEAR(solveSMPS("special_cases/infeasible/BENF4INF"), -1e20, 0.1); // is infeasible, feasibility cuts
//}

TEST_F(NestedBendersTest,saphir){// two-stage, Feasibility cuts
    EXPECT_NEAR(solveSMPS("saphir/saphir_50"),  524047000, 10000); 
    EXPECT_NEAR(solveSMPS("saphir/saphir_100"), 524417000, 10000); 
    //EXPECT_NEAR(solveSMPS("saphir/saphir_200"), 596063000, 10000); 
    EXPECT_NEAR(solveSMPS("saphir/saphir_500"), 482931000, 10000); 
    EXPECT_NEAR(solveSMPS("saphir/saphir_1000"),305918000, 10000); 
    //EXPECT_NEAR(solveSMPS("saphir/SAPHIR_SAMPL_SCEN_10"),-149, 0.1); 
}

TEST_F(NestedBendersTestMultiStage,scdp){// scdp problem from marc
    EXPECT_NEAR(solveSMPS("scdp/SCDP-1024"),-6301005010, 10); 
//    EXPECT_NEAR(solveSMPS("scdp/SCDP-4096"),-5204538801, 10);
//    EXPECT_NEAR(solveSMPS("scdp/SCDP-16384"), -5183755359, 10); 
//    EXPECT_NEAR(solveSMPS("scdp/SCDP-65536"), -5183755359, 10); 
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-8000-reduced"), -5183755359, 1);
//    EXPECT_NEAR(solveSMPS("scdp/SCDP-64000"), -3860456379, 10);
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-64000-reduced"), -5183755359, 1);
}

//TEST_F(NestedBendersTest,StageAggregTest){// scdp problem from marc
//    EXPECT_NEAR(solveSMPS("selfTests/stageAggregTest"),14.0536, 1); 
//}
//
//TEST_F(NestedBendersTest,dakota){// dakota model
//    EXPECT_NEAR(solveSMPS("other/Dakota/dakota"),24014.05, 1); 
//}

//TEST_F(NestedBendersTest,temporary){// dakota model
//    //EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_16"),  -23.8, 0.1); 
//    // Cut Consolidation tests
//    //solveSMPS("temporary/ev_problem-scenario");
//    //solveSMPS("temporary/ev_problem");
//        //EXPECT_NEAR(solveSMPS("watson/coreWAT_C_10","watson/timeWAT_C_10","watson/stochWAT_C_10_16"), -2400.1,0.1); // -2401.87
//    	////EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_16"), 18416.76, 0.1); // OK : MSLIP/NEOS: 18416.76
//    EXPECT_NEAR(solveSMPS("wright/data-scenarios/20","wright/data-scenarios/20","wright/data-scenarios/20-1000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm","wright/data-scenarios/storm","wright/data-scenarios/storm-3000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn","wright/data-scenarios/ssn","wright/data-scenarios/ssn-3000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/20","wright/data-scenarios/20","wright/data-scenarios/20-3000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm","wright/data-scenarios/storm","wright/data-scenarios/storm-2000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn","wright/data-scenarios/ssn","wright/data-scenarios/ssn-2000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/20","wright/data-scenarios/20","wright/data-scenarios/20-2000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/storm","wright/data-scenarios/storm","wright/data-scenarios/storm-1000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("wright/data-scenarios/ssn","wright/data-scenarios/ssn","wright/data-scenarios/ssn-1000"), 0, 0.1);
//    //EXPECT_NEAR(solveSMPS("other/rand2_10000"), 231.706, 0.1);
//
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-65536"), -5183755359, 10); 
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-1024"),-6301005010, 10); 
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-4096"),-5204538801, 10);
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-16384"), -5183755359, 10); 
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-64000"), -3860456379, 10);
//    //EXPECT_NEAR(solveSMPS("other/rand2_10000"), 231.706, 0.1);
//    //EXPECT_NEAR(solveSMPS("scdp/SCDP-65536-noint","scdp/SCDP-65536","scdp/SCDP-65536"), -5183755359, 10); 
//
//}
TEST_F(NestedBendersTest,Postponement){// dakota model
    EXPECT_NEAR(solveSMPS("PostponementSMPS/SMPS 11/pTest"), -5183755359, 10); 

}

//TEST_F(NestedBendersTest,FeasibilityCutTest){
//    using namespace flopc;
//    MP_model testModel(new OsiCpxSolverInterface());
//    MP_stage T(2);
//    MP_variable x,y(T);
//    x.lowerLimit() = -10;
//    y.upperLimit(T) = 15;
//
//    MP_random_data randomData(T);
//    randomData(1) = new DiscreteUniformRandomVariable(10,20,11,new BracketMedianDiscretization(11));
//    randomData(1)->setStage(1);
//
//    MP_constraint firstStage,secondStage(T);
//
//    firstStage() = x() <= 5;
//    secondStage(T+1) = y(T) >= -x()+randomData(T);
//    // Death test only under linux..
//    //EXPECT_DEATH(testModel.attach(),"F.* Your model does not have an objective" );
//    testModel.setObjective( x()+2*y(T.last()) );
//    testModel.attach();
//    OsiCpxSolverInterface osi_cpx;
//    OsiStochasticParallelNestedBendersSolverInterface pnb(&osi_cpx,testModel.getSmi());
//    parallel_benders::Parameter param;
//    param.init();
//    param.parameter_.put(parallel_benders::PNB_NUM_CPU,1);
//    pnb.setParameters(param);
//    pnb.initialSolve(); // We need the osi_solver interface!
//
//    // We need to assert something here with google assertions
//
//    // Output file
//    //SmiScnModel* smiModel = testModel.getSmi();
//    //smiModel->writeSmps("feasibilityProblem",true,false);
//}
//
//TEST_F(NestedBendersTest,StageAggregationTest){
//    using namespace flopc;
//    OsiSolverInterface* osi_cplex = new OsiCpxSolverInterface();
//    MP_model testModel(osi_cplex);
//    enum { stage1,stage2,stage3,stage4, number_stages};
//    enum { scen1,scen2,scen3,scen4,scen5,scen6,scen7,scen8,scen9,scen10,scen11,scen12,scen13,scen14,scen15,scen16, num_scenarios};
//    MP_stage T(number_stages);
//    MP_scenario_set scenario_set(num_scenarios);
//    MP_variable x,y(T);
//    //x.lowerLimit() = -10;
//    //y.upperLimit(T) = 15;
//    // 
//    double scenario_data[number_stages-1][num_scenarios] = 
//    {
//        {   2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1 , 2.2, 2.2, 2.2, 2.3, 2.3, 2.3, 2.3, 2.3, 2.3  },// Stage 2
//        {    3.1, 3.1, 3.2, 3.2, 3.2, 3.3, 3.3, 3.4, 3.5, 3.5, 3.6, 3.6, 3.7, 3.7, 3.7, 3.8   }, // Stage 3
//        {   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 } // Stage 4
//
//    };//new double[number_stages][num_scenarios];
//
//    MP_random_data randomParam(&scenario_data[0][0],T);
//    double* objCoefs = new double[3];
//    objCoefs[0] = 2;
//    objCoefs[1] = 3;
//    objCoefs[2] = 2;
//    MP_data param1(objCoefs,T);
//    double* objCoefs1 = new double[3];
//    objCoefs1[0] = -1;
//    objCoefs1[1] = 0;
//    objCoefs1[2] = 1;
//    MP_data param2(objCoefs,T);
//
//    MP_constraint firstStage,secondStage(T);
//
//    firstStage() = x() >= 5;
//    secondStage(T+1) = y(T-1)+ randomParam(T)*y(T) >= randomParam(T);
//    testModel.setObjective( x()+sum(T,param1(T)*y(T)) );
//    testModel.attach();
//    testModel.getSmi()->writeSmps("stageAggregTest");
//    //OsiCpxSolverInterface cpx;
//    OsiSolverInterface* new_solver = new OsiCpxSolverInterface();
//    
//
//    testModel.getSmi()->aggregateStages(1,3,false);
//    testModel.getSmi()->writeSmps("testSMPS");
//    //testModel.getSmi()->loadOsiSolverData();
//
//    OsiStochasticBendersTwoStageSolverInterface osi_benders(new_solver,testModel.getSmi());
//    BendersParameter parameter;
//    parameter.setIntParam(BendersParameter::BENDERS,0);
//    osi_benders.setParams(parameter);
//    osi_benders.initialSolve();
//
//
//    //testModel.getSmi()->writeSmps("stageAggreg2Test");
//    parallel_benders::Parameter benders_parameter_;
//    benders_parameter_.init(); // This should be called by the standard constructor anyway!
//    benders_parameter_.parameter_.put(parallel_benders::PNB_BENDERS,1); //Use deteq
//    benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_START,1);
//    benders_parameter_.parameter_.put(parallel_benders::PNB_STAGE_AGGREGATION_END,3);
//    OsiStochasticParallelNestedBendersSolverInterface pnb(new_solver,testModel.getSmi());
//    pnb.setParameters(benders_parameter_);
//    pnb.initialSolve();
//    EXPECT_NEAR(9.41,pnb.getObjValue(),0.1);
//    // Output file
//    //SmiScnModel* smiModel = testModel.getSmi();
//    //smiModel->writeSmps("feasibilityProblem",true,false);
//    //     testModel.solve(MP_model::MINIMIZE);
//}



//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char *argv[]) {
    //_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_WNDW);
    int result = 0;
    


    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "NestedBendersTest*"; //scenarios_WRIGHT*:*POSTS*:*watson*:
    //::testing::GTEST_FLAG(filter) = "*Postponement*";
    //::testing::GTEST_FLAG(filter) = "*bonds*";
    //::testing::GTEST_FLAG(filter) = "*.pltexp_POSTS*";
    //::testing::GTEST_FLAG(filter) = "*sslp_OTHER*";
    //::testing::GTEST_FLAG(filter) = "*watson_independent*";
    //::testing::GTEST_FLAG(filter) = "*InvestmentExampleTest*";
    //::testing::GTEST_FLAG(filter) = "*NestedBenders*";
    ::testing::GTEST_FLAG(filter) = "*scdp*:";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints out stacktWrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    result = RUN_ALL_TESTS();
    
    //_CrtDumpMemoryLeaks(); // outside of scoping { }
    return result;
}
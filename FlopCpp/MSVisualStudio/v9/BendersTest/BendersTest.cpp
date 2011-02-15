#define BOOST_TR1_TUPLE_HPP_INCLUDED //This prevents compilation issues.
#include <gtest/gtest.h>

#include <glog/logging.h>
#include <fstream>
#include <vector>

#include "Benders.hpp"
#include "SmiScnModel.hpp"
#include "OsiClpSolverInterface.hpp"
#include "SAA.hpp" //why do we need this? i don't know...
#include "CoinTime.hpp"

//#include <vld.h> //uncomment to enable Visual Leak Detector

namespace flopc {

#pragma region initialization

    class CoverageTest: public ::testing::Test{
    protected:
        BendersParameter benParam;
        CoverageTest(): benParam() { 
            benParam.setIntParam(BendersParameter::BENDERS,0);
            benParam.setIntParam(BendersParameter::MULTI,0);
            benParam.setIntParam(BendersParameter::TRUSTREGION,0);
            benParam.setIntParam(BendersParameter::KNAPSACK,0);
            benParam.setIntParam(BendersParameter::WARMSTART,0);
            benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
            benParam.setIntParam(BendersParameter::REGULARIZED,0);
            benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,0);
            benParam.setIntParam(BendersParameter::BARRIER,0);
            benParam.setIntParam(BendersParameter::BUNCHING,0);
            benParam.setIntParam(BendersParameter::BUNCH_SIZE,2);
        }

        void solveSelectedProblems(){
            // Scenario and feasibility cuts
            EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/feasibilityProblem"), 25, 0.1);
            // Scenario and binary
            EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_50"), -265.569, 0.1);
            // Scenario
            EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Farmer"), -108390, 0.3);
            // Blocks
            EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_small"), -723.839, 0.1);
        }

        double solveSMPS(const char* name) {
            SmiScnModel smi;
            smi.readSmps(name);
            OsiCpxSolverInterface cpxSolver;
            OsiStochasticBendersTwoStageSolverInterface benders(&cpxSolver,&smi);
            benders.setParams(benParam);
            benders.initialSolve();
            return benders.getObjValue();
        }
        

    };
    
    class BendersTest: public ::testing::Test{
    protected:

        BendersParameter benParam;
        double repetitions;
        BendersTest() : benParam(), repetitions(3) {
                benParam.setIntParam(BendersParameter::BENDERS,1);
                benParam.setIntParam(BendersParameter::MULTI,0);
                benParam.setIntParam(BendersParameter::TRUSTREGION,1);
                benParam.setIntParam(BendersParameter::KNAPSACK,0);
                benParam.setIntParam(BendersParameter::WARMSTART,1);
                benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
                benParam.setIntParam(BendersParameter::REGULARIZED,0);
                benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,0);
                benParam.setIntParam(BendersParameter::BARRIER,0);
                benParam.setIntParam(BendersParameter::BUNCHING,1);
                benParam.setIntParam(BendersParameter::BUNCH_SIZE,100);
        }

        double solveSMPS(const char* name) {
            std::vector<double> wallClockTime(repetitions+1,0);
            std::vector<Benders::TimeStatistics> solvingTime(repetitions+1);
			std::vector<Benders::DataStatistics> solutionData(repetitions+1);
            std::vector<int> iterationCount(repetitions+1,0);
            std::vector<double> smpsReading(repetitions+1,0);
            double objective = 0;
            char sdate[9];
            char stime[9];
            ofstream ofs;
		    ofs.open("BendersTestSet.txt", std::ios::app);
            ofs << _strdate_s(sdate) << " " << _strtime_s(stime) << endl;
		    ofs << "Problem Instance " << name << endl;
            for (int i = 0; i < repetitions; i++){
                double time = CoinCpuTime();
                SmiScnModel smi;
                smi.readSmps(name);
                smpsReading[i] = CoinCpuTime()-time;

                OsiCpxSolverInterface cpxSolver;
                OsiStochasticBendersTwoStageSolverInterface benders(&cpxSolver,&smi);
                benders.setParams(benParam);
                benders.initialSolve();
                double wallClockT = CoinCpuTime()-time;

                if (i == 0){ // Stuff to write in the txt file for archiving purposes
                    ofs << "Obj. Value: " << benders.getObjValue() << endl;
                    ofs << "Parameters : ";
                    for (int i = BendersParameter::MULTI; i!= BendersParameter::last1; ++i)
                        ofs << " " << benders.getIntParam(static_cast<BendersParameter::BendersIntParam>(i));
                    ofs << endl;
                    for (int i = BendersParameter::ABSGAP; i!= BendersParameter::last2; ++i)
                        ofs << " " << benders.getDoubleParam(static_cast<BendersParameter::BendersDoubleParam>(i));
                    ofs << endl;

                }
                objective += benders.getObjValue()*(1/repetitions);
                wallClockTime[i] = wallClockT;
                solvingTime[i] = benders.getTimeStatistics();
				solutionData[i] = benders.getDataStatistics();
                iterationCount[i] = benders.getNumIterations();
            }
            // Compute average time
            for (int i = 0; i < repetitions; i++){
                wallClockTime[repetitions] += wallClockTime[i]/repetitions;
                //solvingTime[repetitions] += solvingTime[i]/repetitions;
            }
            LOG(INFO) << "Writing Wall Clock Time";
	        ofs << "Overall Wall Clock Time (secs):            ";
            for (int i = 0; i < repetitions; i++)
                ofs << wallClockTime[i] << " "; 
            ofs << endl;
            ofs << "Solution Time Only (secs):                 ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i].bendersSolve << " "; 
            ofs << endl;
            ofs << "Solution Time Master (secs):               ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i].solverMaster << " "; 
            ofs << endl;
            ofs << "Solution Time Subproblems (secs):          ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i].solverSubproblems << " "; 
            ofs << endl;
            ofs << "Subproblem Loop (secs):                    ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i].subproblemLoop << " "; 
            ofs << endl;
            ofs << "Build Subproblem (secs):                    ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i].buildSubproblem << " "; 
            ofs << endl;
            ofs << "Time w/o cplex and subproblem loop (secs): ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i].bendersSolve-solvingTime[i].solverMaster-solvingTime[i].subproblemLoop << " "; 
            ofs << endl;
            ofs << "Number of Iterations: ";
            for (int i = 0; i < repetitions; i++)
                ofs << iterationCount[i] << " "; 
            ofs << endl;
            ofs << "Time spent reading in SMPS files (secs):   ";
            for (int i = 0; i < repetitions; i++)
                ofs << smpsReading[i] << " ";
            ofs << endl;
            // If we use regularizedManhattan
            if (benParam.getIntParam(BendersParameter::REGULARIZED_MANHATTAN)){
                ofs << "Solution time regularization initial (secs):   ";
                for (int i = 0; i < repetitions; i++)
                    ofs << solvingTime[i].regularizedInitial << " ";
                ofs << endl;
                ofs << "Solution time regularization (secs):   ";
                for (int i = 0; i < repetitions; i++)
                    ofs << solvingTime[i].regularizedManhattan << " ";
                ofs << endl;
                ofs << "Solution time regularization repair (secs):   ";
                for (int i = 0; i < repetitions; i++)
                    ofs << solvingTime[i].regularizedManhattanRepair << " ";
                ofs << endl;
				ofs << "Regularization stopped with solution value:   ";
                for (int i = 0; i < repetitions; i++)
                    ofs << solutionData[i].regularizedSolutionValue << " ";
                ofs << endl;

            }

            ofs << "Average Wall Clock Time (secs): " << wallClockTime[repetitions] << endl;
            //ofs << "Average Overall Solution Time (secs): " << solvingTime[repetitions] << endl;
		    ofs << "-------------------------" << endl;
		    ofs.close();
            LOG(INFO) << " File closed";
            return objective;
        }
    };

#pragma endregion initialization

} //end namespace

TEST_F(BendersTest, scfxm) { 
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/fxm/fxm2_6"), 18416.686, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/fxm/fxm2_16"), 18416.655, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/fxm/fxm3_6"), 18615.932, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/fxm/fxm3_16"), 18438.891, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/fxm/fxm4_6"), 18616.224, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/fxm/fxm4_16"), 18438.891, 0.1);
}
//
TEST_F(BendersTest, pltexp) {
    //// example how to use parameter class; by the way: solved with regularization to optimality, but then cplex crashes...
    //BendersParameter params;
    //params.setIntParam(BendersParameter::REGULARIZED, 1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA2_6", params), -9.479354, 0.1);

    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA2_6"), -9.479354, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA2_16"), -9.663308, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA3_6"), -13.969368, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA3_16"), -14.267458, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA4_6"), -19.599417, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA4_16"), -18.849337, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA4_16"), -23.214073, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/pltexp/pltexpA4_16"), -28.134408, 0.1);
}
//
//TEST_F(BendersTest, sgpf) { 
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/sg/sgpf5y3"), -3027.706, 0.1);
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/sg/sgpf5y4"), -4031.391, 0.1);
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/sg/sgpf5y5"), -5201.282, 0.1);
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/sg/sgpf5y6"), -6403.3, 0.1);
//}
//
//TEST_F(BendersTest, storm) { // ok
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/storm/stormG2_8"), 15535231.897, 10);
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/storm/stormG2_27"), 15508982.306, 10);
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/storm/stormG2_125"), 15512090.180, 10);
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/posts/storm/stormG2_1000"), 15802589.698, 10);
//}

//TEST_F(BendersTest, fleetAssignment) { // ok
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/fA"), -689.432, 0.1);
//////    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_small"), -723.839, 0.1); // BLOCKS sections in stoch file, SMI can't handle it yet
//}
TEST_F(BendersTest, assets) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_large"), -689.432, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_small"), -723.839, 0.1);
}

TEST_F(BendersTest, farmer) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Farmer"), -108390, 0.3);
}
//
TEST_F(BendersTest, finance) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Fina4"), -68401.8, 0.1);
}
//
TEST_F(BendersTest, gas) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/GAS3"), 1558.17, 0.1);  
}

TEST_F(BendersTest, rand) { // takes long to solve, and after the optimal solution is found (correct), we get a stack overflow (at least for the first two instances)
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand0_2000"), 162.146, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand0_4000"), 199.032, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand0_6000"), 140.275, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand0_8000"), 170.318, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand0_10000"), 139.129, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand1_2000"), 244.159, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand1_4000"), 259.346, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand1_6000"), 297.563, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand1_8000"), 262.451, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand1_10000"), 298.638, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand2_2000"), 209.151, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand2_4000"), 218.247, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand2_6000"), 239.721, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand2_8000"), 239.158, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/rand2_10000"), 231.706, 0.1);
}

TEST_F(BendersTest, sslp) { // seems to be ok, but takes very long to solve
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_5"), -265.569, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_10"), -261.905, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_15"), -254.71, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_50"), -365.517, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_100"), -354.868, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_500"), -349.916, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_1000"), -352.494, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_2000"), -348.092, 0.1);
}

TEST_F(BendersTest, stormG2) { // no progress in interations, obj. values 0
    double solutionValue = solveSMPS("../../../../Benders/test/instances/stormG2_3");
    double referenceValue = 15535235.730;
    double relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);
    
    solutionValue = solveSMPS("../../../../Benders/test/instances/stormG2_27");
    referenceValue = 15508982.306;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("../../../../Benders/test/instances/stormG2_125");
    referenceValue = 15512091.185;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    
    //benParam.setIntParam(BendersParameter::BUNCH_SIZE,1);

    solutionValue = solveSMPS("../../../../Benders/test/instances/stormG2_1000");
    referenceValue = 15802589.698;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);
     
}

TEST_F(CoverageTest, cover){
    // We want to cover a whole lot of features 
    // We need problems with scenarios/blocks, optimality/feasibility cuts and linear/binary/integer variables
    // Start with detEq
    solveSelectedProblems();
    // Start with single without warmstart
    benParam.setIntParam(BendersParameter::BENDERS,1);
    solveSelectedProblems();
    // Solve single problems with warmstart taken from previous subproblem
    benParam.setIntParam(BendersParameter::WARMSTART,1);
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
    solveSelectedProblems();
    // Solve single problems with warmstart from previous iteration of same subproblem
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
    solveSelectedProblems();
    // Solve single problems with regularization / keep warmstart
    benParam.setIntParam(BendersParameter::REGULARIZED,1);
    solveSelectedProblems();
    // Solve single with regularized manhattan
    benParam.setIntParam(BendersParameter::REGULARIZED,0);
    benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,1);
    solveSelectedProblems();

    // Switch to multi cuts 

    // Start with multi without warmstart
    benParam.setIntParam(BendersParameter::MULTI,1);
    benParam.setIntParam(BendersParameter::WARMSTART,0);
    solveSelectedProblems();
    // Solve multi problems with warmstart taken from previous subproblem
    benParam.setIntParam(BendersParameter::WARMSTART,1);
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
    solveSelectedProblems();
    // Solve multi problems with warmstart from previous iteration of same subproblem
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
    solveSelectedProblems();
    // Solve multi problems with regularization / keep warmstart
    benParam.setIntParam(BendersParameter::REGULARIZED,1);
    solveSelectedProblems();
    // Solve multi with regularized manhattan
    benParam.setIntParam(BendersParameter::REGULARIZED,0);
    benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,1);
    solveSelectedProblems();

    // Switch to bunching procedure
    
    // Start with multi aggregation without warmstart
    benParam.setIntParam(BendersParameter::BUNCHING,1);
    benParam.setIntParam(BendersParameter::WARMSTART,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart taken from previous subproblem
    benParam.setIntParam(BendersParameter::WARMSTART,1);
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart from previous iteration of same subproblem
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
    solveSelectedProblems();
    // Solve multi aggregation problems with regularization / keep warmstart
    benParam.setIntParam(BendersParameter::REGULARIZED,1);
    solveSelectedProblems();
    // Solve multi aggregation with regularized manhattan
    benParam.setIntParam(BendersParameter::REGULARIZED,0);
    benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,1);
    solveSelectedProblems();

    // Use barrier as master problem solver

    // Start with multi aggregation without warmstart
    benParam.setIntParam(BendersParameter::BARRIER,1);
    benParam.setIntParam(BendersParameter::WARMSTART,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart taken from previous subproblem
    benParam.setIntParam(BendersParameter::WARMSTART,1);
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart from previous iteration of same subproblem
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
    solveSelectedProblems();
    // Solve multi aggregation problems with regularization / keep warmstart
    benParam.setIntParam(BendersParameter::REGULARIZED,1);
    solveSelectedProblems();
    // Solve multi aggregation with regularized manhattan
    benParam.setIntParam(BendersParameter::REGULARIZED,0);
    benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,1);
    solveSelectedProblems();

    // Solve integer problems with integer benders

    // Start with multi aggregation without warmstart
    benParam.setIntParam(BendersParameter::BARRIER,0);
    benParam.setIntParam(BendersParameter::INTEGER_BENDERS,1);
    benParam.setIntParam(BendersParameter::WARMSTART,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart taken from previous subproblem
    benParam.setIntParam(BendersParameter::WARMSTART,1);
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart from previous iteration of same subproblem
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
    solveSelectedProblems();
    // Solve multi aggregation problems with regularization / keep warmstart
    benParam.setIntParam(BendersParameter::REGULARIZED,1);
    solveSelectedProblems();
    // Solve multi aggregation with regularized manhattan
    benParam.setIntParam(BendersParameter::REGULARIZED,0);
    benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,1);
    solveSelectedProblems();

    // Solve integer problems with knapsack and trust-region

    // Start with multi aggregation without warmstart
    benParam.setIntParam(BendersParameter::KNAPSACK,1);
    benParam.setIntParam(BendersParameter::TRUSTREGION,1);
    benParam.setIntParam(BendersParameter::INTEGER_BENDERS,0);
    benParam.setIntParam(BendersParameter::WARMSTART,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart taken from previous subproblem
    benParam.setIntParam(BendersParameter::WARMSTART,1);
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,0);
    solveSelectedProblems();
    // Solve multi aggregation problems with warmstart from previous iteration of same subproblem
    benParam.setIntParam(BendersParameter::WARMSTART_TYPE,1);
    solveSelectedProblems();
    // Solve multi aggregation problems with regularization / keep warmstart
    benParam.setIntParam(BendersParameter::REGULARIZED,1);
    solveSelectedProblems();
    // Solve multi aggregation with regularized manhattan
    benParam.setIntParam(BendersParameter::REGULARIZED,0);
    benParam.setIntParam(BendersParameter::REGULARIZED_MANHATTAN,1);
    solveSelectedProblems();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(filter) = "CoverageTest.*";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints o ut stacktrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    //HeapProfilerStart("bendersTestProfile");
    int result = RUN_ALL_TESTS();
    //HeapProfilerStop();
    //_CrtMemDumpAllObjectsSince(&memstate);
    //_CrtDumpMemoryLeaks();
    return result;
}

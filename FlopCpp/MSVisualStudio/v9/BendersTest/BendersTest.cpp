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

    class BendersTest: public ::testing::Test{
    protected:
        BendersTest(){ }
        
        double solveSMPS(const char* name) {
            double repetitions = 3;
            std::vector<double> wallClockTime(repetitions+1,0);
            std::vector<double> solvingTime(repetitions+1,0);
            double objective = 0;
            //benders.initialSolve();
            ofstream ofs;
		    ofs.open("BendersTestSet.txt", std::ios::app);
		    ofs << "Problem Instance " << name << endl;
            for (int i = 0; i < repetitions; i++){
                double time = CoinCpuTime();
                SmiScnModel smi;
                smi.readSmps(name);

                OsiCpxSolverInterface cpxSolver;
                OsiStochasticBendersTwoStageSolverInterface benders(&cpxSolver,&smi);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::BENDERS,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::MULTI,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::TRUSTREGION,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::KNAPSACK,0);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::WARMSTART,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::WARMSTART_TYPE,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::REGULARIZED,0);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::BARRIER,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::BUNCHING,1);
                benders.setIntParam(OsiStochasticBendersTwoStageSolverInterface::BUNCH_SIZE,100);
                benders.initialSolve();
                double wallClockT = CoinCpuTime()-time;

                if (i == 0){ // Stuff to write in the txt file for archiving purposes
                    ofs << "Obj. Value: " << benders.getObjValue() << endl;
                    ofs << "Parameters : ";
                    for (int i = OsiStochasticBendersTwoStageSolverInterface::MULTI; i!= OsiStochasticBendersTwoStageSolverInterface::last1; ++i)
                        ofs << " " << benders.getIntParam(static_cast<OsiStochasticBendersTwoStageSolverInterface::BendersIntParam>(i));
                    ofs << endl;
                    for (int i = OsiStochasticBendersTwoStageSolverInterface::ABSGAP; i!= OsiStochasticBendersTwoStageSolverInterface::last2; ++i)
                        ofs << " " << benders.getDoubleParam(static_cast<OsiStochasticBendersTwoStageSolverInterface::BendersDoubleParam>(i));
                    ofs << endl;

                }
                objective += benders.getObjValue()*(1/repetitions);
                wallClockTime[i] = wallClockT;
                solvingTime[i] = benders.getSolutionTime();
            }
            // Compute average time
            for (int i = 0; i < repetitions; i++){
                wallClockTime[repetitions] += wallClockTime[i]/repetitions;
                solvingTime[repetitions] += solvingTime[i]/repetitions;
            }


		    
		    ofs << "Overall Wall Clock Time (secs): ";
            for (int i = 0; i < repetitions; i++)
                ofs << wallClockTime[i] << " "; 
            ofs << endl;
            ofs << "Solution Time Only (secs): ";
            for (int i = 0; i < repetitions; i++)
                ofs << solvingTime[i] << " "; 
            ofs << endl;
            ofs << "Average Wall Clock Time (secs): " << wallClockTime[repetitions] << endl;
            ofs << "Average Solution Time (secs): " << solvingTime[repetitions] << endl;

		    ofs << "-------------------------" << endl;
		    ofs.close();
            return objective;
        }
    };

#pragma endregion initialization

} //end namespace

//TEST_F(BendersTest, fleetAssignment) { // ok
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/fA"), -689.432, 0.1);
//////    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_small"), -723.839, 0.1); // BLOCKS sections in stoch file, SMI can't handle it yet
//}
//TEST_F(BendersTest, assets) { // ok
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_large"), -689.432, 0.1);
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_small"), -723.839, 0.1);
//}
////
//
//TEST_F(BendersTest, farmer) { // ok
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Farmer"), -108390, 0.3);
//}
//
//TEST_F(BendersTest, finance) { // ok
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Fina4"), -68401.8, 0.1);
//}
//
//TEST_F(BendersTest, gas) { // ok
////    //SmiScnModel smi;
////    //smi.readSmps("../../../../Benders/test/instances/GAS3");
////    //OsiCpxSolverInterface solver;
////    //smi.setOsiSolverHandle(&solver);
////    //OsiSolverInterface* osi = smi.loadOsiSolverData();
////    //osi->initialSolve();
////    //const double* solutions = osi->getColSolution();
////    //DLOG(INFO) << osi->getObjValue();
//    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/GAS3"), 1558.17, 0.1);  
//}

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
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_5"), -265.569, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_10"), -261.905, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_15"), -254.71, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_50"), -365.517, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_100"), -354.868, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_500"), -349.916, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_1000"), -352.494, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_2000"), -348.092, 0.1);
}

TEST_F(BendersTest, stormG2) { // no progress in interations, obj. values 0
//    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_1000"), 15802589.698, 0.1); // finishes solving after (very long) 12 iterations, but the absolut gap is 699...
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_125"), 15512091.185, 0.1); // Original value should be 15512090.180, but det.eq. value is 15512091.185
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_27"), 15508982.306, 0.1); // Original value equals test det eq. value
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_3"), 15535235.730, 0.1);  // Value from test test is not known (unknown instance).
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*.stormG2";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints o ut stacktrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    int result = RUN_ALL_TESTS();
    //_CrtMemDumpAllObjectsSince(&memstate);
    //_CrtDumpMemoryLeaks();
    return result;
}

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

#include <glog/logging.h>

#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"
#include "CoinTime.hpp"
#include "SmiScnModel.hpp"
#include "flopc.hpp"


#include "ParallelNestedBenders.hpp"

class NestedBendersTest: public ::testing::Test {
public:
    double solveSMPS(const char* name) {
        return solveSMPS(name, name,name);
    }
        double solveSMPS(const char* core_name, const char* time_name, const char* stoch_name) {
        // Read in model

        SmiScnModel smi;
        OsiCpxSolverInterface osi_cpx;
        smi.setOsiSolverHandle(osi_cpx);
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
        int read_in = smi.readSmps(total_core_string.c_str(),total_time_string.c_str(),total_stoch_string.c_str());
        //OsiSolverInterface* osi_ptr = smi.loadOsiSolverData();
        //osi_ptr->initialSolve();
        //double solution = osi_ptr->getObjValue();
        if ( read_in!= 0 ){
            LOG(ERROR) << "Your SMPS file could not be read. Maybe it is missing an ENDATA?";
            return -osi_cpx.getInfinity();
        }

        double time = CoinCpuTime();  
        // Load ParallelNestedBendersSolver
        parallel_benders::OsiStochasticParallelNestedBendersTwoStageSolverInterface nested_benders(&osi_cpx,&smi);
        nested_benders.initialSolve();
        time = CoinCpuTime()-time;
        LOG(INFO) << "Solution Time: " << time;
        //EXPECT_NEAR(solution,nested_benders.getObjValue(),0.1);
        return nested_benders.getObjValue();
    }
protected:
    NestedBendersTest() {}
    virtual ~NestedBendersTest(){}
    virtual void SetUp(){}
    virtual void TearDown(){}
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
    //assert(nb.getSolutionValue() == 6.25);
    parallel_benders::ParallelNestedBenders pnb(testModel.getSmi());
    pnb.Solve();
}

TEST_F(NestedBendersTest, farmer) { // ok
    EXPECT_NEAR(solveSMPS("Farmer"), -108390, 0.3);
}

TEST_F(NestedBendersTest, finance) { // ok
    EXPECT_NEAR(solveSMPS("Fina4"), -68401.8, 0.1);
}

TEST_F(NestedBendersTest, gas) { // ok
    EXPECT_NEAR(solveSMPS("GAS3"), 1558.17, 0.1);  
}

TEST_F(NestedBendersTest, rand) { // takes long to solve
    EXPECT_NEAR(solveSMPS("rand0_2000"), 162.146, 0.1);
    EXPECT_NEAR(solveSMPS("rand0_4000"), 199.032, 0.1);
    EXPECT_NEAR(solveSMPS("rand0_6000"), 140.275, 0.1);
    EXPECT_NEAR(solveSMPS("rand0_8000"), 170.318, 0.1);
    EXPECT_NEAR(solveSMPS("rand0_10000"), 139.129, 0.1);
    EXPECT_NEAR(solveSMPS("rand1_2000"), 244.159, 0.1);
    EXPECT_NEAR(solveSMPS("rand1_4000"), 259.346, 0.1);
    EXPECT_NEAR(solveSMPS("rand1_6000"), 297.563, 0.1);
    EXPECT_NEAR(solveSMPS("rand1_8000"), 262.451, 0.1);
    EXPECT_NEAR(solveSMPS("rand1_10000"), 298.638, 0.1);
    EXPECT_NEAR(solveSMPS("rand2_2000"), 209.151, 0.1);
    EXPECT_NEAR(solveSMPS("rand2_4000"), 218.247, 0.1);
    EXPECT_NEAR(solveSMPS("rand2_6000"), 239.721, 0.1);
    EXPECT_NEAR(solveSMPS("rand2_8000"), 239.158, 0.1);
    EXPECT_NEAR(solveSMPS("rand2_10000"), 231.706, 0.1);
}

TEST_F(NestedBendersTest, sslp) { // Contains "Only" Integer First Stage

    EXPECT_NEAR(solveSMPS("sslp_15_45_5"), -265.569, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_15_45_10"), -261.905, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_15_45_15"), -254.71, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_10_50_50"), -365.517, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_10_50_100"), -354.868, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_10_50_500"), -349.916, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_10_50_1000"), -352.494, 0.1);
    EXPECT_NEAR(solveSMPS("sslp_10_50_2000"), -348.092, 0.1);
}

//TEST_F(NestedBendersTest, sslp_SIPLIB) { // seems to be ok, but takes very long to solve
//    EXPECT_NEAR(solveSMPS("sslp/sslp_10_50_50"), -364.640, 0.1);
//    EXPECT_NEAR(solveSMPS("sslp/sslp_10_50_100"), -354.190, 0.1);
//    EXPECT_NEAR(solveSMPS("sslp/sslp_10_50_500"), -349.136, 0.1);
//    EXPECT_NEAR(solveSMPS("sslp/sslp_10_50_1000"), -351.171, 0.1);
//    EXPECT_NEAR(solveSMPS("sslp/sslp_10_50_2000"), -347.262, 0.1);
//
//    EXPECT_NEAR(solveSMPS("sslp/sslp_15_45_5"), -262.400, 0.1);
//    EXPECT_NEAR(solveSMPS("sslp/sslp_15_45_10"), -260.500, 0.1);
//    EXPECT_NEAR(solveSMPS("sslp/sslp_15_45_15"), -253.602, 0.1);
//}



TEST_F(NestedBendersTest, dcap_SIPLIP) { // Contains integer
    EXPECT_NEAR(solveSMPS("dcap/dcap233_200"), 1834.868, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap233_300"), 1644.342, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap233_500"), 1737.719, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap243_200"), 2322.721, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap243_300"), 2559.297, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap243_500"), 2167.512, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap332_200"), 1060.676, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap332_300"), 1253.040, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap332_500"), 1588.717, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap342_200"), 1619.531, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap342_300"), 2067.777, 0.1);
    EXPECT_NEAR(solveSMPS("dcap/dcap342_500"), 1904.722, 0.1);
}

TEST_F(NestedBendersTest, semi_SIPLIP) { // Needs feasibility cuts, contains integer on first stage
    EXPECT_NEAR(solveSMPS("semi/semi2"), 224434.32, 0.1);
    EXPECT_NEAR(solveSMPS("semi/semi3"), 224486.00, 0.1);
    EXPECT_NEAR(solveSMPS("semi/semi4"), 224564.3, 0.1);
}

TEST_F(NestedBendersTest, sizes_SIPLIB) { // Contains Integer on both stages
    EXPECT_NEAR(solveSMPS("sizes/sizes3"), 224434.32, 0.1);
    EXPECT_NEAR(solveSMPS("sizes/sizes5"), 224486.00, 0.1);
    EXPECT_NEAR(solveSMPS("sizes/sizes10"), 224564.3, 0.1);
}
TEST_F(NestedBendersTest, scfxm_POSTS) { // Generates feasibility cuts, INDEP, not solvable correctly due to SMI stuff: Independent scenario do not get created correctly.
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_6"), 18416.686, 0.1); // OK
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm2_16"), 18416.655, 0.1); // OK
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_6"), 18615.932, 0.1);// Model probability problems
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm3_16"), 18438.891, 0.1);
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_6"), 18616.224, 0.1);
    EXPECT_NEAR(solveSMPS("posts/fxm/fxm4_16"), 18438.891, 0.1);
}
//
TEST_F(NestedBendersTest, pltexp_POSTS) {
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_6"), -9.479354, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA2_16"), -9.663308, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_6"), -13.969368, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA3_16"), -14.267458, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA4_6"), -19.599417, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA4_16"), -18.849337, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_6"), -23.214073, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA5_16"), -28.134408, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA6_6"), -23.214073, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA6_16"), -28.134408, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_6"), -23.214073, 0.1);
    EXPECT_NEAR(solveSMPS("posts/pltexp/pltexpA7_16"), -28.134408, 0.1);
}
//
TEST_F(NestedBendersTest, sgpf_POSTS) { // We get other results.
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y3"), -3027.706, 0.1);
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y4"), -4031.391, 0.1);
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y5"), -5201.282, 0.1);
    EXPECT_NEAR(solveSMPS("posts/sg/sgpf5y6"), -6403.3, 0.1);
}

TEST_F(NestedBendersTest, stormG2_POSTS) {
    double solutionValue = 0;
    double referenceValue = 0;
    double relGap = 0;
    solutionValue = solveSMPS("stormG2_3");
    referenceValue = 15535235.730;
    abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("stormG2_27");
    referenceValue = 15508982.306;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("stormG2_125");
    referenceValue = 15512091.185;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);

    solutionValue = solveSMPS("stormG2_1000");
    referenceValue = 15802589.698;
    relGap = abs(referenceValue-solutionValue)/solutionValue;
    EXPECT_LE(relGap,0.000001);
}

// Andrew Felt Test Set
TEST_F(NestedBendersTest, airlift_FELT){// 2-Stage
    EXPECT_NEAR(solveSMPS("felt/airlift/AIRL"),249101.672072,0.1); //BLOCK
    EXPECT_NEAR(solveSMPS("felt/airlift/AIRL2"),269665.498390,0.1);//INDEP
}

TEST_F(NestedBendersTest,assets_FELT){
    EXPECT_NEAR(solveSMPS("assets_large"), -689.432, 0.1);
    EXPECT_NEAR(solveSMPS("assets_small"), -723.839, 0.1);
}

TEST_F(NestedBendersTest,bonds_FELT){ // Values that are tested against: results from MSLIB-OSL, values commented out: presented in felts testset
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y3"), -2967.9, 0.1); // 2976.91
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y4"), -3994.179, 0.1); // 3991.3
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y5"), -5172.07, 0.1); // 5125.6
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y6"), -6463.240, 0.1); // 6369.0
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf3y7"), -7947.863, 0.1); 
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y3"), -3027.6, 0.1); // 3027.6
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y4"), -4031.303, 0.1); // 4023.9
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y5"), -5201.197, 0.1); // 5180.8
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y6"), -6484.465, 0.1); // 6403.3
    EXPECT_NEAR(solveSMPS("felt/bonds/sgpf5y7"), -7956.896, 0.1); 
}

TEST_F(NestedBendersTest,cargo_FELT){

}

TEST_F(NestedBendersTest,chem_FELT){

}

TEST_F(NestedBendersTest,electric_FELT){

}

TEST_F(NestedBendersTest,environ_FELT){

}
TEST_F(NestedBendersTest,phone_FELT){

}

TEST_F(NestedBendersTest,stocfor_FELT){

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
    secondStage(T+1) = y(T+1) >= -x()+randomData(T+1);
    // Death test only under linux..
    //EXPECT_DEATH(testModel.attach(),"F.* Your model does not have an objective" );
    testModel.setObjective( x()+2*y(T+1) );
    testModel.attach();
    parallel_benders::ParallelNestedBenders pnb(testModel.getSmi());
    pnb.Solve();

    // Output file
    //SmiScnModel* smiModel = testModel.getSmi();
    //smiModel->writeSmps("feasibilityProblem",true,false);
}

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*.sgpf";
    ::testing::GTEST_FLAG(filter) = "*.bonds*";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints out stacktrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    int result = RUN_ALL_TESTS();
    return result;
}
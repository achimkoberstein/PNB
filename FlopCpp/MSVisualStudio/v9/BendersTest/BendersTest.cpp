#define BOOST_TR1_TUPLE_HPP_INCLUDED //This prevents compilation issues.
#include <gtest/gtest.h>

#include <glog/logging.h>

#include "Benders.hpp"
#include "SAA.hpp" //why do we need this? i don't know...

//#include <vld.h> //uncomment to enable Visual Leak Detector

namespace flopc {

#pragma region initialization

    class BendersTest: public ::testing::Test{
    protected:
        BendersTest(){ }
        
        double solveSMPS(const char* name) {
            Benders* benders = new Benders("m", "+wsI", "-ki", "-tr", "+ben","-reg");
            benders->readSMPS(name);
            benders->BendersSolve(0.00001, 1000);
            return benders->getObjValue();
        }
    };

#pragma endregion initialization

} //end namespace

TEST_F(BendersTest, assets) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_large"), -689.432, 0.1);
    //EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/assets_small"), -723.839, 0.1); // BLOCKS sections in stoch file, SMI can't handle it yet
}

TEST_F(BendersTest, farmer) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Farmer"), -108390, 0.3);
}

TEST_F(BendersTest, finance) { // ok
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/Fina4"), -68401.8, 0.1);
}

TEST_F(BendersTest, gas) { // master problem unbounded?!
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/GAS3"), 1764.67, 0.1);  
}

TEST_F(BendersTest, rand) { // no progress in interations, obj. values 0
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
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_100"), -354.868, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_1000"), -352.494, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_2000"), -348.092, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_50"), -365.517, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_10_50_500"), -349.916, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_10"), -261.905, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_15"), -254.71, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/sslp_15_45_5"), -265.569, 0.1);
}

TEST_F(BendersTest, stormG2) { // no progress in interations, obj. values 0
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_1000"), 15802600, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_125"), 15512100, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_27"), 15509000, 0.1);
    EXPECT_NEAR(solveSMPS("../../../../Benders/test/instances/stormG2_3"), 15535200, 0.1);    
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*.sslp";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints out stacktrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    int result = RUN_ALL_TESTS();
    //_CrtMemDumpAllObjectsSince(&memstate);
    //_CrtDumpMemoryLeaks();
    return result;
}

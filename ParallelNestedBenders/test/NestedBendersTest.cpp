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

#include "flopc.hpp"
#include "ParallelNestedBenders.hpp"

class NestedBendersTest: public ::testing::Test {
public:
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

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*.Atilla";
    google::InitGoogleLogging(argv[0]);
#ifndef _MSC_VER
    google::InstallFailureSignalHandler(); //This does not work under windows (prints out stacktrace)
#endif
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    int result = RUN_ALL_TESTS();
    return result;
}
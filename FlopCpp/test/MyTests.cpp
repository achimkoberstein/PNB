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

#include "OsiClpSolverInterface.hpp"
#include "OsiCbcSolverInterface.hpp"
#include "CoinFloatEqual.hpp"
#include "flopc.hpp"
#include "SmiScnModel.hpp"
//#include "SAA.hpp"
//#include "OsiStochasticSolverInterface.hpp"

//#include <vld.h> //uncomment to enable Visual Leak Detector
//TODO: Test about correct colOrdering that is given to the solver..
//TODO: Test scenario tree generation for RV that are not for themselves part of the model. Maybe this is already working, maybe not..

//using namespace flopc;
namespace flopc {
    void writeReadSMPSTest(MP_model& model, MP_model::MP_direction direction, const std::string& filename, const bool strictFormat=true){
        // write SMPS files
        model.smiModel->writeSmps(filename.c_str(), true, strictFormat);

        // read SMPS files        
        SmiScnModel smi;
        EXPECT_EQ(0, smi.readSmps(filename.c_str()));
        OsiSolverInterface *clp = new OsiClpSolverInterface();
        smi.setOsiSolverHandle(*clp);
        OsiSolverInterface *osiStoch = smi.loadOsiSolverData();
        osiStoch->setObjSense(direction);

        if (osiStoch->getNumIntegers() > 0)
            osiStoch->branchAndBound();
        else
            osiStoch->initialSolve();

        EXPECT_EQ(model.Solver->isProvenOptimal(),osiStoch->isProvenOptimal());
        EXPECT_EQ(model.Solver->isProvenPrimalInfeasible(),osiStoch->isProvenPrimalInfeasible());
        EXPECT_EQ(model.Solver->isProvenDualInfeasible(),osiStoch->isProvenDualInfeasible());
        EXPECT_EQ(model.Solver->isAbandoned(),osiStoch->isAbandoned());

        // check objective values in case of optimal solution
        if ( model.Solver->isProvenOptimal() )
            EXPECT_NEAR(model.Solver->getObjValue(), osiStoch->getObjValue(), 0.001);
        ASSERT_EQ(0,remove( (filename+".cor").c_str() ));
        ASSERT_EQ(0,remove( (filename+".tim").c_str() ));
        ASSERT_EQ(0,remove( (filename+".sto").c_str() ));
        delete clp;
    }
    
    void compareEVPTest(MP_model& model, MP_model::MP_direction direction) {
        double ev_calculated = model.getSmi()->getEVValue(model.Solver, direction, true);
        double ev = model.getSmi()->getEVValue(model.Solver, direction, false);
        EXPECT_NEAR(ev, ev_calculated, 0.01);
        if (std::abs(ev - ev_calculated) > 0.01) {
            LOG(ERROR) << "failed";
        }
    }

#pragma region initialization
    class MP_modelTest: public ::testing::Test {
    public:
    protected:
        MP_modelTest() : testModel(new OsiClpSolverInterface()){}
        virtual ~MP_modelTest(){}
        virtual void SetUp(){}
        virtual void TearDown(){}

        void testCorrectStageAssignment(MP_model& model){
            int* colStage = model.getColStage();
            int* rowStage = model.getRowStage();
            for ( int i = 0; i < model.realn-1; i++){
                EXPECT_LE(colStage[i],colStage[i+1]);
            }
            for (int i = 0; i < model.m-1; i++){
                EXPECT_LE(rowStage[i],rowStage[i+1]);
            }
        }

        void testFlatArray(){
            MP_model testModel(new OsiClpSolverInterface());
            MP_set first(3);
            MP_set second(4);
            MP_set third(5);
            MP_set fourth(6);
            MP_set fifth(7);
            MP_data x(first,second,third,fourth,fifth);
            for (int i = 0; i < first.size(); i++)
                for(int j = 0; j < second.size(); j++)
                    for (int k = 0; k < third.size(); k++)
                        for (int l = 0; l <fourth.size(); l++)
                            for (int m = 0; m < fifth.size(); m++){
                                IndexKeeper temp = x.getIndices(x.f(i,j,k,l,m));
                                EXPECT_EQ(x.f(i,j,k,l,m),x.f(temp.i1,temp.i2,temp.i3,temp.i4,temp.i5));
                            }

        }



        MP_model testModel; //Attention: Do not rely on testModel for actual testing (includes attachment of solver): Leads to Run-Time Failure #0
    };

    class UseCaseTest: public MP_modelTest {
    public:
    protected:
        UseCaseTest(){}

    };
    // Class to test the functionality of random variables
    class RandomVariableTest: public MP_modelTest {
    public:
    protected:
        RandomVariableTest() :testModel(new OsiClpSolverInterface()){}
        virtual ~RandomVariableTest(){}
        virtual void SetUp(){}
        virtual void TearDown(){}

        MP_model testModel;
    };

    // Class to test the functionality of random variables
    class IntegerTest: public MP_modelTest {
    };

    class BendersTest: public ::testing::Test{
    protected:
        BendersTest(){ }
    };

    class FlopCppExamples: public ::testing::Test {
    public:
    protected:
        FlopCppExamples(){}

    };

    class SmiUnitTest: public MP_modelTest {
    public:
    protected:
        SmiUnitTest(){}

    };

#pragma endregion initialization

    // To run the BendersTest, you need to link to the Benders solver and the OsiCpx SolverInterface, as well as including additionally the OsiStochasticSolverInterface.
    //TEST_F(BendersTest,BendersSolverTest){
    //    // minimization 
    //    // model.solve(MP_model::MINIMIZE);
    //    // call copyModel(smiModel)
    //    // call copySubModel(smiModel) methods from Benders..
    //    int maxIt = 100;
    //    Benders* benders = new Benders("s", "-ws", "-ki", "-tr", "-ben");

    //    MP_model investmentModel(new OsiCpxSolverInterface());
    //    MP_data initialWealth,goal;
    //    initialWealth() = 55; //set data
    //    goal() = 60;

    //    enum {numStage=2};
    //    MP_stage T(numStage); //Time Stages
    //    enum {numScen=2};
    //    MP_scenario_set scen(numScen);
    //    enum {asset1,asset2,numAssets};
    //    MP_set assets(numAssets);
    //    double scenarios[numStage-1][numAssets][numScen]=
    //    {//T
    //        {//assets
    //            // stage 2
    //            {1.25,1.06},{1.14,1.16} //asset1 to asset1, scen 1 to 8
    //        },
    //    };
    //    MP_random_data returns(&scenarios[0][0][0],T,assets);

    //    MP_variable x(T,assets);
    //    MP_variable wealth(T);
    //    MP_variable shortage(T), overage(T);

    //    MP_constraint 
    //        initialWealthConstr,
    //        returnConstr(T),
    //        allocationConstr(T),
    //        goalConstr(T);

    //    initialWealthConstr() = sum(assets,x(0,assets)) == initialWealth();
    //    allocationConstr(T) = sum(assets,x(T,assets)) == wealth(T);
    //    returnConstr(T+1) = sum(assets,returns(T+1,assets)*x(T,assets) ) == wealth(T+1);

    //    goalConstr(T.last()) = wealth(T.last()) == goal() + overage(T.last()) - shortage(T.last()); 
    //    MP_expression valueFunction( 1.3*shortage(T.last()) - 1.1*overage(T.last()));

    //    investmentModel.setObjective( valueFunction );
    //   
    //    investmentModel.attach(investmentModel.Solver);
    //    investmentModel.solve(MP_model::MAXIMIZE);
    //    //EXPECT_NEAR(3.784,investmentModel->getObjValue(),0.0001);
    //    //TODO: Benders can only minimize models
    //    benders->copyMaster(investmentModel.smiModel);
    //    benders->copySubmodel(investmentModel.smiModel);
    //    benders->BendersSolve(0.000001, maxIt );
    //    //for (int i = 0; i < investmentModel.ScenSet().size();i++){
    //    //    for (int j = 0; j < T.size(); j++){
    //    //        cout << "scenario " << i << " stufe " << j << " wealth: " << wealth.levelScenario(i,j) << endl;
    //    //    }
    //    //}
    //    //EXPECT_NEAR(3.784,benders->osiMaster_->getObjValue(),0.0001);
    //    delete benders; //TODO: This leads to errors
    //}

    //TEST_F(MP_modelTest,SAATest){
    //    MP_model testModel(new OsiClpSolverInterface(),new VerboseMessenger());

    //    MP_stage T(3);
    //    //Define two sets
    //    MP_set cars(3);
    //    MP_set trains(2);
    //    MP_set streets(3);

    //    MP_random_data a(T);
    //    a(1) = new ContinuousNormalRandomVariable(0,1,new BracketMeanDiscretization(20));
    //    a(1)->setStage(1);
    //    a(2) = new ContinuousNormalRandomVariable(0,1,new BracketMeanDiscretization(20));
    //    a(2)->setStage(2);

    //    MP_variable x(T);
    //    MP_variable y(T);
    //    MP_variable z(cars);
    //    MP_variable z1(cars);

    //    MP_constraint testCon(cars),carOnStreet(T),countCars(T);
    //    //testCon(cars) = z(cars) <= sum(cars,z1(cars));
    //    carOnStreet(T) = x(T) <= abs(a(T));
    //    MP_index aliasT;
    //    countCars(T+1) = y(T+1) <= sum(T(aliasT).such_that(aliasT<= T) ,x(aliasT+1));


    //    MP_expression objective;
    //    objective = y(T.last());

    //    testModel.setObjective(objective);
    //    SAASolver solver(testModel,*testModel.Solver);
    //    solver.setValues(10,20,5);
    //    solver.solve(MP_model::MAXIMIZE);
    //    EXPECT_DOUBLE_EQ(1.5922808968215727,testModel->getObjValue());



    //}

TEST_F(UseCaseTest,InfinityHandlingTest){
    MP_model testModel(new OsiClpSolverInterface);
    MP_data data1;
    data1() = testModel->getInfinity();
    MP_data data2;
    data2() = 1000000000;
    
    MP_variable x,y;
    x.upperLimit() = data1();
    y.upperLimit() = data2();

    MP_constraint con1,con2;
    con1() = x() <= data1();
    con2() = y() <= data2();

    testModel.setObjective(x()+y());
    testModel.solve(MP_model::MAXIMIZE);

    EXPECT_EQ(MP_model::DUAL_INFEASIBLE,testModel.getStatus());
    //testModel->writeMps("testFile"); //We write no direction into MPS files.. that is not good.
    //testModel->readMps("testFile");
    //testModel->initialSolve();
   
}

    TEST_F(UseCaseTest,ConditionalValueAtRiskTest){
        MP_model testModel(new OsiClpSolverInterface);
        MP_stage T(2);
        MP_random_data a(T);
        a(1) = new DiscreteUniformRandomVariable(1,10,10,new BracketMedianDiscretization(10));
        a(1)->setStage(1);

        MP_variable x;
        MP_variable recourse(T);

        MP_constraint secondStageConstraint1(T),secondStageConstraint2(T);
        secondStageConstraint1(T+1) = x() >= a(T+1) - recourse(T+1);
        //secondStageConstraint2(T+1) = y(T+1) == a()
        MP_expression obj = x() + 2*recourse(T.last());
        testModel.setObjective(obj);
        testModel.solve(MP_model::MINIMIZE);
        testModel.setObjective( cvar(0.2,obj));
        testModel.solve(MP_model::MINIMIZE);


    }

    TEST_F(MP_modelTest,RandomVarInScenTreeTest){
        MP_model testModel(new OsiClpSolverInterface(),new VerboseMessenger());
        MP_stage T(2);
        MP_random_data a(T);
        a(1) = new ContinuousUniformRandomVariable(-2,-1,new BracketMeanDiscretization(2));
        a(1)->setStage(1);
        MP_random_data b(T);
        b(1) = new ContinuousUniformRandomVariable(1,2,new BracketMeanDiscretization(2));
        b(1)->setStage(1);
        MP_random_data d(T);
        d(1) = new ContinuousNormalRandomVariable(0,1, new DoubleDiscretization(4) );
        d(1)->setStage(1);
        MP_random_data c(T);
        c(T) = a(T)+b(T);

        MP_variable x(T);

        MP_constraint testCon(T);
        testCon(T) = x(T) <= 10+c(T);

        MP_expression objective;
        objective = x(T.last());

        testModel.setObjective(objective);
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        //EXPECT_DOUBLE_EQ(1.5922808968215727,testModel->getObjValue());
        ASSERT_EQ(3,testModel.getScenSet().size());
        //OsiStochasticBendersTwoStageSolverInterface benders(testModel.Solver,testModel.getSmi());
        ////double begin = CoinCpuTime();
        //benders.initialSolve();
        //double duration = CoinCpuTime() - begin;
    }


    //Test fails to test the correct thing, somehow something is messed up, assumingly during the det.eq. generation of Smi. Supposed problem: constraints span two stages? I am not so sure about that
    TEST_F(MP_modelTest,ComplicatedExpressionsTest){
        MP_model testModel(new OsiClpSolverInterface(),new VerboseMessenger());

        MP_stage T(3);
        //Define two sets
        MP_set cars(3);
        MP_set trains(2);
        MP_set streets(3);

        MP_random_data a(T);
        a(1) = new ContinuousNormalRandomVariable(0,1,new BracketMeanDiscretization(10));
        a(1)->setStage(1);
        a(2) = new ContinuousNormalRandomVariable(0,1,new ExtendedPearsonTukeyDiscretization());
        a(2)->setStage(2);

        MP_variable x(T);
        MP_variable y(T);
        MP_variable z(cars);
        MP_variable z1(cars);

        MP_constraint testCon(cars),carOnStreet(T),countCars(T);
        //testCon(cars) = z(cars) <= sum(cars,z1(cars));
        carOnStreet(T) = x(T) <= abs(a(T));
        MP_index aliasT;
        countCars(T+1) = y(T+1) <= sum(T(aliasT).such_that(aliasT<= T) ,x(aliasT+1));


        MP_expression objective;
        objective = y(T.last());

        testModel.setObjective(objective);
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_DOUBLE_EQ(1.3819475579528415,testModel->getObjValue());
        ASSERT_EQ(10,testModel.getScenSet().size());
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);
        //OsiStochasticBendersTwoStageSolverInterface benders(testModel.Solver,testModel.getSmi());
        ////double begin = CoinCpuTime();
        //benders.initialSolve();
        //double duration = CoinCpuTime() - begin;
    }

    TEST_F(MP_modelTest,DiscretizationTest){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger());
        MP_stage T(2);
        MP_random_data rvar(T);
        // MP_random_data should delete all the RV it get's assigned..
        rvar(1) = new ContinuousNormalRandomVariable(0,1, new BracketMeanDiscretization(10));
        rvar(1)->setStage(1);
        //rvar2(1) = new ContinuousNormalRandomVariable(0,1, new BracketMedianDiscretization(5));
        //rvar2(1)->setStage(1);
        MP_variable x(T);
        MP_constraint con(T);
        con(T) = x(T) <= rvar(T);
        testModel.setObjective( x(T.last()) );
        testModel.solve(MP_model::MAXIMIZE);
        // We assign a new RV to the old one, we have to delete the old one before
        delete rvar(1);
        rvar(1) = new ContinuousNormalRandomVariable(0,1, new BracketMedianDiscretization(10));
        rvar(1)->setStage(1);
        testModel.setObjective( x(T.last()) );
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        delete rvar(1);
        rvar(1) = new ContinuousExponentialRandomVariable(1, new BracketMedianDiscretization(10));
        rvar(1)->setStage(1);
        testModel.setObjective( x(T.last()) );
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);

        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);

    }

    TEST_F(MP_modelTest,CoefficientGeneration){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger(),837234979);
        // Enable stochastic elements to allow matrix printing
        MP_stage T(3);
        MP_random_data rVar(T);
        //rVar(0) = new DiscreteBernoulliRandomVariable();
        rVar(1) = new DiscreteUniformRandomVariable(-2,-1);
        rVar(1)->setStage(1);
        rVar(2) = new DiscreteUniformRandomVariable(1,2,3);
        rVar(2)->setStage(2);
        MP_data A(T);
        A(0) = 15;
        A(1) = 10;
        A(2) = 5;
        MP_random_data combined(T);
        combined(T) = A(T) + rVar(T);
        MP_variable y(T);
        MP_constraint randomConstraint1(T),randomConstraint2(T),randomConstraint3(T);
        randomConstraint1(T) = y(T) <= combined(T);
        randomConstraint3(T+2) = y(T+2) <= combined(T+2)+rVar(T+1);
        randomConstraint2(T+1) = y(T+1) <= combined(T+1);
        testModel.setObjective(y(T.last()));
        testModel.attach();
        //EXPECT_THROW(testModel.attach(),std::exception);
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_DOUBLE_EQ(5,testModel.Solver->getObjValue());
        y.display("y");
        rVar.display("RandomVars");
        combined.display("CombinedData");

        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);
    }

    TEST_F(MP_modelTest,ExtendedCoefficientGeneration){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger(),837234979);
        // Enable stochastic elements to allow matrix printing
        MP_stage T(3);
        MP_random_data rVar(T);
        //rVar(0) = new DiscreteBernoulliRandomVariable();
        rVar(1) = new DiscreteUniformRandomVariable(-2,-1,2);
        rVar(1)->setStage(1);
        rVar(2) = new DiscreteUniformRandomVariable(1,2,2);
        rVar(2)->setStage(2);
        MP_data A(T);
        A(0) = 15;
        A(1) = 10;
        A(2) = 5;
        MP_random_data combined(T);
        combined(T) = A(T-1) + rVar(T);
        MP_variable y(T);
        MP_constraint randomConstraint1(T),randomConstraint2(T),randomConstraint3(T);
        //randomConstraint1(T) = y(T) <= combined(T);
        //randomConstraint3(T+2) = y(T+2) <= combined(T+2)+rVar(T+1);
        randomConstraint2(T+1) = y(T+1) <= combined(T+1);
        testModel.setObjective(y(T.last()));
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_DOUBLE_EQ(11.5,testModel.Solver->getObjValue());
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);
        y.display("y");
        rVar.display("RandomVars");
        combined.display("CombinedData");
        // We should have a better way to check for certain values.. (like the level command)
    }

    TEST_F(MP_modelTest,SimpleTestThatHasToBeRenamed){
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_data a;
        MP_random_data b(T);
        DiscreteUniformRandomVariable* xy = new DiscreteUniformRandomVariable(0,10,11);
        a() = 10;
        b(T.last()) = xy;

        MP_variable x(T);
        MP_variable y(T);
        y.upperLimit() = 10;

        MP_constraint firstConstraint,secondConstraint,thirdConstraint(T);
        //firstConstraint() = a()*x() <= 20;
        //secondConstraint() = y()*x() <= 10;
        thirdConstraint(T+1) = b(T+1)*x(T+1) <= y(T+1);
        testModel.setObjective(x(T.last()));
        testModel.solve(MP_model::MAXIMIZE);

        // How to test for infeasible models? Is this the point? No it is not..
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);
    }

    TEST_F(MP_modelTest,SuchThatTest){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger());

        MP_set A(2);
        MP_set B(3);
        MP_set C(2);
        MP_set D(2);

        MP_subset<2> AB(A,B);
        AB.insert(0,0);
        AB.insert(0,1);

        MP_subset<2> CD(C,D);
        CD.insert(0,0);

        MP_subset<2> BC(B,C);
        BC.insert(0,0);
        BC.insert(0,1);
        BC.insert(1,1);

        MP_data dA(A);

        dA(0) = 1;
        dA(1) = 10;

        MP_variable x1(A), x2(A),x3(B,C),x4(B);
        x1.upperLimit(A) = 10;
        x2.upperLimit(A) = 10;
        x3.upperLimit(B,C) = 10;
        x4.upperLimit(B) = 10;

        MP_constraint constr1(A), constr2(AB);
        constr1(A) = sum( BC(B,C),3*x3(B,C))  <= dA(A);
        constr2(AB(A,B)) = x2(A)+x4(B) <= dA(A); //constr(AB) = dA(A) <= x2(A); does not work

        testModel.setObjective(sum(A, x1(A) + x2(A)) + sum(B.such_that(BC(B,C))*C.such_that(BC(B,C)), x3(B,C)   ));
        testModel.attach(testModel.Solver);
        testModel.solve(MP_model::MAXIMIZE);

        x1.display();
        x2.display();
        EXPECT_DOUBLE_EQ(31.333333333333336,testModel->getObjValue());
    }

    TEST_F(MP_modelTest,SuchThatRandomTest){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger());

        MP_set A(2);
        MP_set B(3);
        MP_set C(4);
        MP_set D(5);
        MP_stage T(3);
        MP_scenario_set scen(4);

        MP_subset<2> AB(A,B);
        AB.insert(0,0);
        AB.insert(0,1);

        MP_subset<2> CD(C,D);
        CD.insert(0,0);

        MP_subset<2> BC(B,C);
        BC.insert(0,0);
        BC.insert(0,1);
        BC.insert(1,1);

        MP_data dA(A);
        dA(0) = 1;
        dA(1) = 10;

        double dbvalues[2][4] = { {1,2,3,4 }, {5,6,7,8 } }; 

        MP_random_data dB( &dbvalues[0][0],T);

        MP_variable x1(A), x2(A),x3(B,C),x4(B),x5(A,T);
        x1.upperLimit(A) = 10;
        x2.upperLimit(A) = 10;
        x3.upperLimit(B,C) = 10;
        x4.upperLimit(B) = 10;

        MP_constraint constr1(A), constr2(AB),constr3(T);
        constr1(A) = sum( BC(B,C),3*x3(B,C))  <= dA(A);
        constr2(AB(A,B)) = x2(A)+x4(B) <= dA(A); //constr(AB) = dA(A) <= x2(A); does not work
        constr3(T) = sum(A,x5(A,T)) <= dB(T);

        testModel.setObjective(sum(A.such_that(AB(A,B)), x2(A)) + sum(B.such_that(BC(B,C))*C.such_that(BC(B,C)), x3(B,C)   ));
        testModel.attach(testModel.Solver);
        testModel.solve(MP_model::MAXIMIZE);
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");

        x1.display();
        x2.display();
        EXPECT_DOUBLE_EQ(4.0/3,testModel->getObjValue());
    }


    TEST_F(UseCaseTest,RandomWalkTest){ //Seed: 1015576855 im vergleich zu 1106783986
        const int numStage = 3;
        unsigned int seed = 2013598861;
        MP_model testModel(new OsiClpSolverInterface(),new NormalMessenger(),seed);

        MP_stage T(numStage);

        RandomVariable* randomVars[numStage-1] = { new DiscreteUniformRandomVariable(-1,1,2), new DiscreteUniformRandomVariable(4,5,2) };

        MP_random_data randomWalkVar(&randomVars[0],T);
        MP_random_data sumR(T);
        MP_data blubb(T);
        blubb(0) = 5;
        blubb(1) = 10;
        blubb(2) = 20;
        //sumR(0) = 0;
        sumR(T) = blubb(T) + randomWalkVar(T);

        MP_variable x(T);
        MP_variable y(T);

        MP_constraint firstConstraint(T),secondConstraint(T);
        MP_constraint thirdConstraint;
        //firstConstraint(T+1) = x(T+1) <= blubb(T) + randomWalkVar(T+1);

        secondConstraint(T+1) = y(T+1) == sumR(T+1) + y(T);
        firstConstraint(T+1) = x(T+1) <= y(T+1);
        thirdConstraint() = y(0) <= Constant(0);

        //MP_constraint secondConstraint(T);
        //secondConstraint(T+1) = 2*y(T+1) + x(T+1) <= randomWalkVar(T+1);
        MP_expression objective( x(T.last()) );

        testModel.setObjective(objective);
        testModel.attach();
        blubb.display("Blubb");
        sumR.display("SumR");
        randomWalkVar.display("RandomWalkVar");
        std::cout << std::endl;
        testModel.solve(MP_model::MAXIMIZE);
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);
        //sumR.display("RandomValues?");
        x.display("Werte");
        sumR.display("SumR");
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_DOUBLE_EQ(34.5,testModel.Solver->getObjValue());

    }

    // Tests if scenario based random variables gets correctly read in.
    TEST_F(RandomVariableTest,ScenarioBasedRandomVariableInitializationTest){
        enum {numStage=2};
        enum {numScen=10};
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(numStage); // enum numStage = 2
        MP_scenario_set scenSet(numScen); //enum numScen=10
        double scenarioValues[numStage-1][numScen] = { { 1,2,3,4,5,6,7,8,9,10 } };
        MP_random_data r(&scenarioValues[0][0],T); //Scenario-based 
        for (int i = 0; i < numScen; i++){
            EXPECT_EQ(scenarioValues[0][i],r(1)->getSampledValue(i));
            EXPECT_EQ(scenarioValues[0][i],r(T+1).getRandomVariable()->getSampledValue(i));
        }

    }





    //Test if Conditional Sampling works (for seed 0 for every RV. This means values are highly correlated)
    TEST_F(RandomVariableTest,DISABLED_ConditionalSamplingTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(3);
        //TODO: We have to write a good seed generation routine...
        // Idea: At the moment of sampling total number of sampling variables is known. One can divide the seeds evenly.
        RandomVariable* scenarioValues[2] = { new DiscreteUniformRandomVariable(0,10,11),new DiscreteUniformRandomVariable(0,10,5)};
        RandomVariable* scenarioValues1[2] = { new DiscreteUniformRandomVariable(10,20,5),new DiscreteUniformRandomVariable(10,20,5)};
        MP_random_data rvar(&scenarioValues[0],T);
        MP_random_data rvar2(&scenarioValues1[0],T);
        MP_variable x(T);
        MP_constraint con1(T);
        con1(T+1) = x(T+1)*(rvar(T+1)+rvar2(T+1)) <= 10;//*rvar2(T);
        testModel2.setObjective(x(T.last()));
        testModel2.attach(testModel2.Solver);
        this->testCorrectStageAssignment(testModel2);
        testModel2.solve(MP_model::MAXIMIZE);
        //EXPECT_DOUBLE_EQ(0.43177511288609688,testModel2->getObjValue()); //If seed 0 is used for all RV
        //EXPECT_EQ(70,testModel2.ScenSet().size()); //Test if number of scenarios is correctly generated (this is a consequence of the sampled values)
        //EXPECT_EQ(70,testModel2.getSmi()->getNumScenarios());
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        // TODO: Too much memory gets allocated during scenario tree generation.. This needs to be redone eventually.
        //testModel2.attach();
        //testModel2.solve(MP_model::MAXIMIZE);
        //EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        //testModel2.attach();
        //testModel2.solve(MP_model::MAXIMIZE);
        //EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        //testModel2.attach();
        //testModel2.solve(MP_model::MAXIMIZE);
        //EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        ////BOOST_THROW_EXCEPTION(not_initialized_exception());
    }

    // Simple Example Model to test fundamental ScenTree Generation and Solving Capabilites
    TEST_F(UseCaseTest,InvestmentExampleTest){

        MP_model investmentModel(new OsiClpSolverInterface());
        MP_data initialWealth,goal;
        initialWealth() = 55; //set data
        goal() = 80;

        enum {numStage=4};
        MP_stage T(numStage); //Time Stages
        enum {numScen=8};
        MP_scenario_set scen(numScen);
        enum {asset1,asset2,numAssets};
        MP_set assets(numAssets);
        double scenarios[numStage-1][numAssets][numScen]=
        {//T
            {//assets
                // stage 2
                {1.25,1.25,1.25,1.25,1.06,1.06,1.06,1.06},{1.14,1.14,1.14,1.14,1.16,1.16,1.16,1.16} //asset1 to asset1, scen 1 to 8
            },
            {// stage 3
                {1.21,1.21,1.07,1.07,1.15,1.15,1.06,1.06},{1.17,1.17,1.12,1.12,1.18,1.18,1.12,1.12} //asset1 to asset1, scen 1 to 8
            },
            {// stage 4
                {1.26,1.07,1.25,1.06,1.05,1.06,1.05,1.06},{1.13,1.14,1.15,1.12,1.17,1.15,1.14,1.12} //asset1 to asset1, scen 1 to 8
            }
        };
        MP_random_data returns(&scenarios[0][0][0],T,assets);

        MP_variable x(T,assets);
        MP_variable wealth(T);
        MP_variable shortage(T), overage(T);

        MP_constraint 
            initialWealthConstr,
            returnConstr(T),
            allocationConstr(T),
            goalConstr(T);

        initialWealthConstr() = sum(assets,x(0,assets)) == initialWealth();
        allocationConstr(T-1) = sum(assets,x(T-1,assets)) == wealth(T-1);
        returnConstr(T+1) = sum(assets,returns(T+1,assets)*x(T,assets) ) == wealth(T+1);

        goalConstr(T.last()) = wealth(T.last()) == goal() + overage(T.last()) - shortage(T.last()); 
        MP_expression valueFunction( -1.3*shortage(T.last()) + 1.1*overage(T.last()));

        investmentModel.setObjective( valueFunction );
        investmentModel.attach(investmentModel.Solver);
        this->testCorrectStageAssignment(investmentModel);
        investmentModel.solve(MP_model::MAXIMIZE);
        //for (int i = 0; i < scen.size();i++){
        //    for (int j = 0; j < T.size(); j++){
        //        cout << "scenario " << i << " stufe " << j << " wealth: " << wealth.levelScenario(i,j) << endl;
        //    }
        //}

        ////Compare solution values.
        EXPECT_NEAR(4.142141441,investmentModel.Solver->getObjValue(),0.000001);
        EXPECT_EQ(MP_model::OPTIMAL,investmentModel.getStatus());

        writeReadSMPSTest(investmentModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(investmentModel,MP_model::MAXIMIZE);

        std::vector< std::pair<double,double> > wsResults = investmentModel.smiModel->solveWS(investmentModel.Solver, MP_model::MAXIMIZE);
        EXPECT_EQ(numScen,wsResults.size());
        double result = investmentModel.smiModel->getWSValue(investmentModel.Solver, MP_model::MAXIMIZE);
        // Compute result by hand
        double tempResult = 0;
        for (int i = 0; i < numScen; i++) {
            tempResult += wsResults[i].first*wsResults[i].second;
        }
        EXPECT_DOUBLE_EQ(result,tempResult);
        EXPECT_NEAR(10.76479,result,0.0001);
    }


    TEST_F(MP_modelTest,RandomParameterInObjectiveTest){
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        double prob[2] = { 0.2, 0.8 };
        MP_data probabilities(&prob[0],scen);
        double scenarios[1][2] = { { 2,6} };
        MP_random_data rvar(&scenarios[0][0],T);
        MP_variable x(T);
        MP_constraint con1(T);
        con1(T) = x(T) <= 10;
        MP_expression objective;
        objective = rvar(T.last())*x(T.last());
        testModel.setObjective( objective );
        //If we want other probs, we have to attach them
        testModel.setProbabilities(probabilities);
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_DOUBLE_EQ(52,testModel->getObjValue());
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);


    }

    // Test if integer integration is working correctly (high level)
    TEST_F(IntegerTest,IntegerTest){
        MP_model intModel(new OsiClpSolverInterface());
        MP_stage T(2);

        MP_scenario_set scen(3);
        double scenarios[1][3] =
        { // T
            {1,2,3} //scen
        };
        MP_random_data rvar(&scenarios[0][0],T);
        MP_variable y(T);
        y.integer();
        MP_constraint constraint1(T);
        constraint1(T) = y(T) >= 1.5 + rvar(T);
        intModel.setObjective(sum(T, y(T)));
        intModel.attach(intModel.Solver);
        this->testCorrectStageAssignment(intModel);
        intModel.solve(MP_model::MINIMIZE);
        writeReadSMPSTest(intModel,MP_model::MINIMIZE,"test");
        compareEVPTest(intModel,MP_model::MINIMIZE);
        EXPECT_EQ(6,intModel->getObjValue());
        EXPECT_TRUE(intModel->isInteger(0));
        EXPECT_TRUE(intModel->isInteger(1));
        y.display("y");
        cout << "Finished.";
    }

    // Test several data combining constraints
    TEST_F(MP_modelTest,DataTest){
        MP_data data1;
        data1.setName("d1");
        MP_data data2;
        data2.setName("d2");
        data1() = 7;
        data2() = 10;
        MP_data data3;
        data3.setName("d3");
        MP_data data4;
        MP_data data5;
        data5.setName("data5");
        data4.setName("data4");
        data4() = data2();
        data3() = data1() + data2();
        data5() = data2();
        MP_data data6;
        data6() = data5() + data2();
        MP_variable x;

        MP_constraint constraint1,constraint2;
        constraint1 = (data4() +data3())*x() <= 10;
        constraint2 = data4()*data3()*x() <= 10;

        testModel.setObjective(x());
        testModel.attach(testModel.Solver);
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        // We do not have a model that contains randomness, so there is not much sense in writing a SMPS file.
        // It is not possible anyway as no SmiScnModel gets created.
    }

    TEST_F(MP_modelTest,ConstantTermInObjectiveTest){
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_variable x(T);
        MP_random_data randomData(T);
        randomData(1) = new DiscreteUniformRandomVariable(0,1,2,new BracketMedianDiscretization(2));
        randomData(1)->setStage(1);
        MP_constraint con1(T);
        con1(T) = x(T) >= randomData(T);
        MP_expression obj = x(T.last())-10;
        testModel.setObjective(obj);
        testModel.solve(MP_model::MINIMIZE);
        EXPECT_EQ(-9.5,testModel->getObjValue());

    }

    TEST_F(MP_modelTest,FeasibilityCutTest){
        MP_model testModel(new OsiClpSolverInterface());
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
        testModel.solve(MP_model::MINIMIZE);
        EXPECT_EQ(25,testModel->getObjValue());
        // Output file
        //SmiScnModel* smiModel = testModel.getSmi();
        //smiModel->writeSmps("feasibilityProblem",true,false);
    }

    TEST_F(MP_modelTest,EmpiricalRandomVariableTest){
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_variable x,y(T);
        std::vector<double> probs;
        std::vector<double> values;
        probs.push_back(0.2);
        probs.push_back(0.8);
        values.push_back(5);
        values.push_back(20);
        
        RandomVariable* var[1] = { new EmpiricalRandomVariable(values,probs) };
        MP_random_data randomData(&var[0],T);
        
        MP_constraint firstStage,secondStage(T);
        firstStage() = x() >= 10;
        secondStage(T+1) = y(T+1) >= x()+randomData(T+1);

        testModel.setObjective(x()+y(T+1));
        testModel.solve(MP_model::MINIMIZE);
        LOG(INFO) << "ObjValue " << testModel->getObjValue();
    }

    // Dacota  Model to test the EV-, EEV- and WS-Problems
    TEST_F(UseCaseTest,DacotaModel) {
        MP_model dacota(new OsiClpSolverInterface()); 
        MP_stage T(2);

        enum {low, normal, high, numScenarios};
        MP_scenario_set scen(numScenarios);

        MP_data prob(scen);
        prob(low)    = 0.3;
        prob(normal) = 0.4;
        prob(high)   = 0.3;
        dacota.setProbabilities(prob);

        enum {desk, table, chair, numProducts};
        MP_set P(numProducts);

        enum {lumber, finishing, carpentry, numResources};
        MP_set R(numResources);

        MP_data resourceCost(R);
        resourceCost(lumber) = 2;
        resourceCost(finishing) = 4;
        resourceCost(carpentry) = 5.2;

        MP_data resourceReq(P, R);
        resourceReq(desk, lumber) = 8;      resourceReq(desk, finishing) = 4;       resourceReq(desk, carpentry) = 2;
        resourceReq(table, lumber) = 6;     resourceReq(table, finishing) = 2;      resourceReq(table, carpentry) = 1.5;
        resourceReq(chair, lumber) = 1;     resourceReq(chair, finishing) = 1.5;    resourceReq(chair, carpentry) = 0.5;

        double prices[3] = {60.0, 40.0, 10.0};
        MP_data sellingPrice(&prices[0], P);

        double scenDemand[1][numProducts][numScenarios] =
        {
            { // L     M    H
                {50,  150, 250}, // desk
                {20,  110, 250}, // table
                {200, 225, 500}  // chair
            }
        };
        MP_random_data demand(&scenDemand[0][0][0], T, P);

        MP_variable 
            x(R),   // Ressourcenbedarf
            y(T,P); // Produzierte Einheiten

        MP_constraint demandConstraint(P), productionConstraint(R), conR(R);
        demandConstraint(P) = y(T+1,P) <= demand(T+1,P);
        productionConstraint(R) = sum(P, resourceReq(P, R) * y(T+1,P)) <= x(R);

        dacota.setObjective(
            sum(P, y(T+1,P) * sellingPrice(P))
            - sum(R, x(R) * resourceCost(R))
            );

        dacota.attach(dacota.Solver);
        this->testCorrectStageAssignment(dacota);

                // solve EV-, EEV-, WS- and here-and-now-Problem
        double ev, eev, ws, hereAndNow;





        ev = dacota.smiModel->getEVValue(dacota.Solver, MP_model::MAXIMIZE, false);
        EXPECT_EQ(ev, 4165);

        eev = dacota.smiModel->getEEVValue(dacota.Solver, MP_model::MAXIMIZE, false);
        EXPECT_NEAR(eev, 1545, 0.0001);

        ws = dacota.smiModel->getWSValue(dacota.Solver, MP_model::MAXIMIZE);
        EXPECT_EQ(ws, 4165);

        // we have to attach the solver again, otherwise the program crashes
        dacota.attach(dacota.Solver);
        //dacota->writeLp("dacota");
        dacota.solve(MP_model::MAXIMIZE);
        hereAndNow = dacota->getObjValue();
        EXPECT_DOUBLE_EQ(1730.0, hereAndNow);
        EXPECT_EQ(MP_model::OPTIMAL,dacota.getStatus());

        writeReadSMPSTest(dacota,MP_model::MAXIMIZE,"test");
        compareEVPTest(dacota,MP_model::MAXIMIZE);
              


    }

    // Test complex combinations of random variables in constraints
    TEST_F(UseCaseTest,RandomVariableConstraintCombinationTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        double scenarios[1][2] =
        { // T
            {1,2} //scen
        };
        double scenarios2[1][2] =
        { // T
            {1,2} //scen
        };
        double scenarios3[1][2] =
        { // T
            {1,2} //scen
        };
        MP_random_data rvar(&scenarios[0][0],T);
        MP_random_data rvar2(&scenarios2[0][0],T);
        MP_random_data rvar3(&scenarios3[0][0],T);
        MP_variable x(T);
        MP_constraint constraint1, constraint2(T),constraint3(T),constraint4(T);
        constraint1 = x(0) == 1;
        constraint3(T+1) = x(T+1) <= x(T)+ (rvar2(T+1)+3); //TODO: It is necessary to put additive terms in parentheses. This should not be necessary.
        constraint4(T+1) = x(T+1) >= x(T)*(rvar3(T+1)+3); 
        testModel2.setObjective(x(T.last()));
        testModel2.attach(testModel2.Solver);
        this->testCorrectStageAssignment(testModel2);
        testModel2.solve(MP_model::MINIMIZE); 
        writeReadSMPSTest(testModel2,MP_model::MINIMIZE,"test");
        EXPECT_NEAR(4.5,testModel2->getObjValue(),0.001);
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        compareEVPTest(testModel2,MP_model::MINIMIZE);
    }

    // Test if generation of scenario tree out of scenario fan does work
    TEST_F(UseCaseTest,ScenarioTreeGenerationTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(3); // three stages
        MP_set demands(2); // two different demands
        MP_scenario_set scen(6); //six scenarios
        double scenarios[2][2][6] =
        { // T
            {//demands 
                {3,1,1,1,2,2},{3,1,1,1,2,2} //scen
            },
            {
                {9,4,5,6,7,8},{9,4,5,6,7,8} //scen
            }

        };
        MP_random_data rvar(&scenarios[0][0][0],T,demands);
        MP_variable x(T,demands);
        MP_variable firstStageVar;
        firstStageVar.lowerLimit() = 5;
        firstStageVar.upperLimit() = 10;
        MP_constraint constraint1(T),constraint2(T);
        //What is with StairCase Structure?!
        constraint1(T+1) = sum(demands,x(T+1,demands)*rvar(T+1,demands)) + firstStageVar() >= sum(demands,x(T,demands));
        //constraint2(T) = x(T,demands) >= 10;
        testModel2.setObjective(x(T.last(),demands));
        testModel2.attach(testModel2.Solver);
        this->testCorrectStageAssignment(testModel2);
        testModel2.solve(MP_model::MINIMIZE);
        writeReadSMPSTest(testModel2,MP_model::MINIMIZE,"test");
        compareEVPTest(testModel2,MP_model::MINIMIZE);
        EXPECT_NEAR(0,testModel2.Solver->getObjValue(),0.000001);
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
    }


    TEST_F(RandomVariableTest,ScenarioRandomVariableConstructionTest2){
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(2);
        //Create RV via double array. 
        MP_set autos(4); //We have a set with elements 0 to 3
        enum {numScen = 3};
        MP_scenario_set scen(numScen);
        double rvArr[1][4][numScen] = {{ {1,2,3},{4,5,6},{7,8,9},{10,11,12} }};
        ASSERT_THROW(MP_random_data rvar1,invalid_argument_exception);
        MP_random_data rvar2(&rvArr[0][0][0],T,autos);
        //Expect invalid argument exception
        //EXPECT_THROW(MP_random_data rvar2(&rvArr[0][0],autos),invalid_argument_exception);
        EXPECT_DOUBLE_EQ(rvArr[0][0][0], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(1,0)) )->getSampledValue(0) );
        EXPECT_DOUBLE_EQ(rvArr[0][0][1], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(1,0)) )->getSampledValue(1) );
        EXPECT_DOUBLE_EQ(rvArr[0][0][2], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(1,0)) )->getSampledValue(2) );
        EXPECT_DOUBLE_EQ(rvArr[0][3][0], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(1,3)) )->getSampledValue(0) );
        EXPECT_DOUBLE_EQ(rvArr[0][3][1], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(1,3)) )->getSampledValue(1) );
        EXPECT_DOUBLE_EQ(rvArr[0][3][2], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(1,3)) )->getSampledValue(2) );
    }

    // Compile only check: Uncomment to detect ill-formed programs
    //TEST_F(MP_modelTest,SimpleRandomVariableExpressionParsingTest){
    //    //Tests RV without sets and some combinations. Compile-Time Test
    //    MP_stage T(2);
    //    MP_random_data rVar1(T);
    //    MP_variable x1;
    //    MP_variable y1;
    //    MP_variable x2;
    //    MP_variable y2;
    //    MP_constraint exp1;
    //    //Test some combinations of left and right hand side
    //    //More than 1 assignments to a MP_expression can lead to curious behaviour
    //    //Question: Why is that?
    //    exp1() = 3*x1() + rVar1()*y1() <= 7;
    //    //exp1() = 4*x1()+5*x2() <= 10;
    //    //exp1() = rVar1()*4 + y2()*rVar1() <= 8; //How can this work? constant get implicitly converted to expression..
    //    //Somehow we have to check if the rhs is properly evaluated in the end
    //}

    TEST_F(UseCaseTest,SampleOnlyTest){
        MP_model testModel (new OsiClpSolverInterface());
        enum {bmw,audi,ford,numCars};
        enum {schwarz,rot,gold,numColors};
        MP_stage T(2);
        MP_set cars(numCars);
        MP_set colors(numColors);
        RandomVariable* var[1] = { new DiscreteUniformRandomVariable(0,1,20) };
        RandomVariable* var1[1] = { new DiscreteUniformRandomVariable(3,4,20) };
        MP_random_data demand(&var[0],T);
        MP_random_data price(&var1[0],T);

        MP_variable x(T);

        MP_constraint con1(T);
        con1(T+1) = x(T+1) >=demand(T+1);

        MP_expression objective = x(T.last())*price(T.last());
        testModel.setObjective(objective);
        testModel.attach();
        testModel.solve(MP_model::MINIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        //demand.display("Demand");
        //price.display("Price");
        std::vector<double> sampledValues = var[0]->getSampledValues();
        for (int i = 0; i < sampledValues.size(); i++){
            EXPECT_LE(0,sampledValues[i]);
            EXPECT_GE(1,sampledValues[i]);
        }
        sampledValues = var1[0]->getSampledValues();
        for (int i = 0; i < sampledValues.size(); i++){
            EXPECT_LE(3,sampledValues[i]);
            EXPECT_GE(4,sampledValues[i]);
        }
        testModel.setSampleOnlyScenarioGeneration(true,10);

        testModel.attach();
        testModel.solve(MP_model::MINIMIZE);
        writeReadSMPSTest(testModel,MP_model::MINIMIZE,"test");
        compareEVPTest(testModel,MP_model::MINIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        //demand.display("Demand");
        //price.display("Price
        sampledValues = var[0]->getSampledValues();
        for (int i = 0; i < sampledValues.size(); i++){
            EXPECT_LE(0,sampledValues[i]);
            EXPECT_GE(1,sampledValues[i]);
        }
        sampledValues = var1[0]->getSampledValues();
        for (int i = 0; i < sampledValues.size(); i++){
            EXPECT_LE(3,sampledValues[i]);
            EXPECT_GE(4,sampledValues[i]);
        }
        std::vector<double> prob = testModel.getProbabilities();
        double sum = 0;
        for (int i = 0 ; i < testModel.getScenSet().size(); i++){
            //std::cout << " Sc." << i << ": " << prob[i];
            sum += prob[i];
        }
        std::cout << std::endl;
        EXPECT_DOUBLE_EQ(1,sum);


    }



    // Random bounds test
    TEST_F(MP_modelTest,DISABLED_RandomBoundsTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        MP_data prob(scen);
        prob(0) = 0.5;
        prob(1) = 0.5;
        testModel2.setProbabilities(prob);
        double scenarioValues[1][2] = { {1,2} };
        MP_random_data var(&scenarioValues[0][0],T);
        MP_variable x(T);
        MP_variable y(T);
        MP_constraint con1(T);
        MP_constraint con2(T);
        con1(T) = x(T) <= var(T);
        con2(T) = y(T) >= var(T);
        //TODO: Enable lowerLimit again if working
        //y.lowerLimit(T) = var(T);
        //x.lowerLimit(T) = var(T); //Set different lower limits
        MP_expression objective;
        objective = x(T+1);
        testModel2.setObjective(objective);
        testModel2.attach(testModel.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        writeReadSMPSTest(testModel2,MP_model::MINIMIZE,"test");
        compareEVPTest(testModel2,MP_model::MINIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        EXPECT_EQ(1.5,testModel2->getObjValue());
        //Test Bounds
        EXPECT_DOUBLE_EQ(0, testModel2->getColLower()[1]);
        EXPECT_DOUBLE_EQ(1, testModel2->getColLower()[2]);
        EXPECT_DOUBLE_EQ(2, testModel2->getColLower()[4]);
    }

    // Random bounds
    TEST_F(MP_modelTest,DISABLED_ComplicatedRandomBoundsTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(3);
        MP_set elm(0);
        MP_scenario_set scen(2);
        double scenarioValues[2][2] = { {1,2},{3,4} };
        MP_random_data var(&scenarioValues[0][0],T);
        MP_variable x(T);
        // TODO: Reenable limits
        //x.lowerLimit = var(T);
        //x.upperLimit = var(T)+5;
        MP_constraint con1(T);
        con1(T+1) = x(T+1) <= var(T+1)+2.5;
        testModel2.setObjective(x(T+1));
        testModel2.attach(testModel.Solver);
        testModel2.solve(MP_model::MAXIMIZE);
        writeReadSMPSTest(testModel2,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel2,MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        //TODO: Test appearance of RandomVariable vector
        EXPECT_DOUBLE_EQ(4,testModel2->getObjValue());

    }

    TEST_F(MP_modelTest,RandomVariableExpressionParsingTest){
        //Now we test RandomVariables with sets. Compile-Time Test
        MP_stage T(2); //2 stages
        MP_random_data rVar1(T);
        MP_variable x;
        MP_variable y(T);
        MP_variable x2;
        MP_variable y2(T);
        MP_constraint exp1(T);
        //Test some combinations of left and right hand side
        exp1(T+1) = 3*x(T) + 4*y2(T+1) <= 9;
        exp1(T) = y2(T)*4 <= rVar1(T);
        //exp1(T+1) = y2(T+1)*rVar1(T+1) <= 8;
        //Only latest assignment remains.. clearly this is correct behaviour
    }

    TEST_F(MP_modelTest,SimpleRandomVariableSolverTest){
        testModel.verbose();
        MP_stage T(2);
        MP_scenario_set scen(2);
        MP_variable x;
        MP_variable y(T);
        double scenarios[1][2] = {{ 1,2 }};
        double scenarios2[1][2] = {{ 3,4 }};
        double scenarios3[1][2] = {{ 5,6 }};
        MP_random_data rVar(&scenarios[0][0],T);
        MP_random_data rVar2(&scenarios2[0][0],T);
        MP_random_data rVar3(&scenarios3[0][0],T);
        MP_constraint constraint,constraint1(T),constraint2(T);
        MP_index j;
        //I would expect that such things would work, i.e. assign to different indices of one constraint.
        //constraint1(0) = 3*x() <= 10;
        constraint() = 5*x() >= 60; //Funny thing: Problem gets presolved so that nothing is left.. this is definitly not the correct behaviour, as we want to maximize..
        constraint1(T+1) = 4*x()+(rVar(T+1)+2)*y(T+1) <= rVar3(T+1)*20 + rVar2(T+1);
        constraint2(T+1) = 4*x()+(rVar(T+1)+2)*y(T+1) >= rVar3(T+1)*20 + rVar2(T+1);
        //TODO: Test coefficients right hand sides and stuff.. it is REALLY necessary to do so
        //TODO: Test if the coefficients get generated in the right manner.
        testModel.setObjective( 3*x() );
        testModel.attach(testModel.Solver);
        //We need to do some tests here..
        testModel.solve(MP_model::MAXIMIZE);
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        //Print some results..
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_EQ(77.25,testModel->getObjValue()); //Normal Solver gets called, as no random vars were specified

        compareEVPTest(testModel,MP_model::MAXIMIZE);
    }

    //TEST_F(MP_modelTest,RandomVariableInAllStagesTest){
    //    MP_stage T(3); //We have three stages
    //    MP_random_data rVar(T); //Random Variable depends only on stage
    //    MP_random_data rVar2(T);
    //    //As we see: Only stages 1..T needs to be defined. If a RV is indexed via stage=0, in the generation process later on, it is not evaluated.
    //    rVar(1) = new DiscreteRandomVariable();
    //    rVar2(1) = new DiscreteRandomVariable();
    //    rVar(2) = new DiscreteRandomVariable();
    //    rVar2(2) = new DiscreteRandomVariable();
    //    MP_variable x;
    //    MP_variable y(T);
    //    MP_constraint con1; 
    //    MP_constraint con2(T);
    //    con1() = x() <= 10;
    //    con2(T) = x() + rVar(T)*y(T) <= rVar2(T);
    //    testModel.setObjective(y(T.last()));
    //    testModel.attach(testModel.Solver);
    //    testModel.solve(MP_model::MAXIMIZE);
    //    EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
    //}

    TEST_F(MP_modelTest,RandomVariableInitializationTest){
        MP_model testModel(new OsiClpSolverInterface());
        // Test RV Usage by Assignment or Initialization.
        enum {Audi, BMW, Ford, numCars};
        enum {DB,EuroBahn,numTrains};
        MP_stage T(2);
        MP_set cars(numCars);
        MP_set trains(numTrains);
        MP_random_data vars(T,cars); //Initialize RV with set
        DiscreteUniformRandomVariable *x,*y,*z;
        x = new DiscreteUniformRandomVariable();
        y = new DiscreteUniformRandomVariable();
        z = new DiscreteUniformRandomVariable();
        vars(1,BMW) = x; 
        vars(1,Audi) = y;
        vars(1,Ford) = z;
        vars(1,BMW)->setStage(1);
        vars(1,Audi)->setStage(1);
        vars(1,Ford)->setStage(1);
        //Stage setting is not yet supported, TODO
        MP_variable test(cars,trains);
        RandomVariable* varsArr[1][3] = {{ new DiscreteUniformRandomVariable(0,2,1),new DiscreteUniformRandomVariable(0,3,1),new DiscreteUniformRandomVariable(0,4,1) }}; //Initialize RV with Array of RV
        // Need to delete Array as RV does not take ownership..
        // Maybe store boost::shared_ptr<> to RV?
        // Then the values assigned to the RV get managed by the RV and released too
        // No Deletion of array values necessary afterwards..
        MP_random_data vars2(&varsArr[0][0],T,cars);
        //Check if vars2 contains these random variables
        EXPECT_EQ(varsArr[0][0],vars2(1,0)); //only checks pointer
        EXPECT_EQ(varsArr[0][1],vars2(1,1));
        EXPECT_EQ(varsArr[0][2],vars2(1,2));
        MP_constraint constraint1(trains,T);
        constraint1(trains,T+1) = sum(cars,test(cars,trains)) <= sum(cars,vars2(T+1,cars));
        testModel.setObjective( sum(cars*trains,test(cars,trains)) );
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);

        EXPECT_DOUBLE_EQ(9,testModel->getObjValue());

        // as stage setting is not yet implemented, this cant work - right?!
        //writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");


    }

    //RandomVariable Sampling Tests..
    TEST_F(RandomVariableTest,RandomVariableSamplingTest){
        MP_scenario_set scenSet(3);
        MP_stage T(2);
        double vals[1][4][3] = {{ {4, 3, 2},{2,1,5},{7,3,2},{1,5,7} }};
        MP_set testSet(4);
        MP_random_data rVarScenario(&vals[0][0][0],T,testSet);
        //MP_random_data rVarDiscrete;
        //MP_random_data rVarContinuous;
        DiscreteUniformRandomVariable* rv = new DiscreteUniformRandomVariable(-3,2,6);
        ContinuousUniformRandomVariable* rv2 = new ContinuousUniformRandomVariable(-2,2, new BracketMeanDiscretization(20)); 
        rv->Seed(static_cast<unsigned int>(std::time(0)));
        std::vector<double> sampledValues = rv->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i],-3);
            EXPECT_LE(sampledValues[i],2);
        }
        rv2->Seed(static_cast<unsigned int>(std::time(0)));
        sampledValues = rv2->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i],-2);
            EXPECT_LE(sampledValues[i],2);
        }
        ScenarioRandomVariable* rv3 = dynamic_cast<ScenarioRandomVariable*>(rVarScenario(1,0));
        rv3->sample();
        sampledValues = rv3->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_EQ(vals[0][0][i],sampledValues[i]);
        }

        ContinuousLogNormalRandomVariable* rv4 = new ContinuousLogNormalRandomVariable(1,0.5, new BracketMeanDiscretization(20) );
        rv4->Seed(static_cast<unsigned int>(std::time(0)));
        sampledValues = rv4->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i], 0);
        }

        ContinuousNormalRandomVariable* rv5 = new ContinuousNormalRandomVariable(0,1,new BracketMeanDiscretization(20));
        rv5->Seed(static_cast<unsigned int>(std::time(0)));
        sampledValues = rv5->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i], -4);
            EXPECT_LE(sampledValues[i], 4);
        }

        ContinuousExponentialRandomVariable* rv6 = new ContinuousExponentialRandomVariable(1,new BracketMeanDiscretization(20));
        rv6->Seed(static_cast<unsigned int>(std::time(0)));
        sampledValues = rv6->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i], 0);
        }

        ContinuousTriangleRandomVariable* rv7 = new ContinuousTriangleRandomVariable(0,1,2,new BracketMeanDiscretization(20));
        rv7->Seed(static_cast<unsigned int>(std::time(0)));
        sampledValues = rv7->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i], 0);
            EXPECT_LE(sampledValues[i], 2);
        }

        DiscreteGeometricRandomVariable* rv8 = new DiscreteGeometricRandomVariable(0.5);
        rv8->Seed(static_cast<unsigned int>(std::time(0)));
        //rv8->sample(20);
        sampledValues = rv8->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i], 1);
        }

        DiscreteBernoulliRandomVariable* rv9 = new DiscreteBernoulliRandomVariable(0.5);
        rv9->Seed(static_cast<unsigned int>(std::time(0)));
        //rv9->sample(20);
        sampledValues = rv9->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i], 0);
            EXPECT_LE(sampledValues[i], 1);
        }
        delete rv;
        delete rv2;
        delete rv4;
        delete rv5;
        delete rv6;
        delete rv7;
        delete rv8;
        delete rv9;
        cout << "Finished.";
    }

    TEST_F(RandomVariableTest,DoubleComparisonTest){
        double testValue = 0;
        testValue += flopc::absoluteTolerance;
        EXPECT_TRUE(flopc::compareDouble(testValue,0.0));
        testValue += std::numeric_limits<double>::epsilon();
        EXPECT_FALSE(flopc::compareDouble(testValue,0.0));
        testValue = 0 - flopc::absoluteTolerance;
        EXPECT_TRUE(flopc::compareDouble(testValue,0.0));
        testValue -= std::numeric_limits<double>::epsilon();
        EXPECT_FALSE(flopc::compareDouble(testValue,0.0));
        testValue = 1e100;
        EXPECT_TRUE(flopc::compareDouble(testValue,1e100-2));
    }

    TEST_F(MP_modelTest,FlatMultidimensionalArrayTest){
        testFlatArray();
    }   

    TEST_F(RandomVariableTest,AdditiveConstantSamplingTest){
        MP_model testModel2(new OsiClpSolverInterface());
        double testValue = 3;
        MP_stage T(2);
        MP_scenario_set scen(10);
        double scenarios[10] = {0,1,2,3,4,5,6,7,8,9};
        MP_random_data rvar(&scenarios[0],T);
        MP_variable x;
        x.lowerLimit() = 0;

        MP_variable y(T);
        y.lowerLimit(T) = 0;
        MP_constraint constraint1(T),con2;
        constraint1(T+1) = x()+y(T+1)*(rvar(T+1)+testValue) >= 7;
        testModel2.setObjective(x());
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        //Test for Correctnes 
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        EXPECT_EQ(testValue,testModel2.Solver->getMatrixByRow()->getCoefficient(1,1));
        EXPECT_EQ(testValue+1,testModel2.Solver->getMatrixByRow()->getCoefficient(2,2));
        EXPECT_EQ(testValue+5,testModel2.Solver->getMatrixByRow()->getCoefficient(6,6));
        writeReadSMPSTest(testModel2,MP_model::MINIMIZE,"test");
        compareEVPTest(testModel2,MP_model::MINIMIZE);
        //testModel2.detach();
        //double scenarios2[10] = {0,1,2,3,4,5,6,7,8,9};
        //MP_random_data rvar2(&scenarios2[0],T);
        //constraint1(T+1) = x()+y(T+1)*(rvar2(T+1)*testValue) >= 7;
        //testModel2.setObjective(x());
        //testModel2.attach(new OsiClpSolverInterface());
        //testModel2.solve(MP_model::MINIMIZE);
        //EXPECT_EQ(testValue*0,testModel2.Solver->getMatrixByRow()->getCoefficient(1,2));
        //EXPECT_EQ(testValue,testModel2.Solver->getMatrixByRow()->getCoefficient(2,3));
        //EXPECT_EQ(testValue*5,testModel2.Solver->getMatrixByRow()->getCoefficient(6,7));
        //testModel2.detach();
        //double scenarios3[10] = {0,1,2,3,4,5,6,7,8,9};
        //MP_random_data rvar3(&scenarios2[0],T);
        //constraint1(T+1) = x()+y(T+1)*((rvar3(T+1)+testValue)*10) >= 7; //This does not yet work. But i think it is not so important either..
        //testModel2.setObjective(x());
        //testModel2.attach(new OsiClpSolverInterface());
        //testModel2.solve(MP_model::MINIMIZE);

    }

    TEST_F(RandomVariableTest,RepeatedSamplingTest){
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(2);
        RandomVariable* randomVars[1] = { new DiscreteUniformRandomVariable(0,10,11) };
        MP_random_data rvar(&randomVars[0],T);
        MP_variable x(T);
        MP_constraint con1(T),con2(T);
        con1(T+1) = x(T+1) <= rvar(T+1);
        //con2(T+1) = x(T+1) >= x(T);
        testModel.setObjective(x(T.last()));
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        //EXPECT_DOUBLE_EQ(6.7,testModel->getObjValue());
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
        compareEVPTest(testModel,MP_model::MAXIMIZE);
        //EXPECT_DOUBLE_EQ(4.8,testModel->getObjValue());
    }

    TEST_F(RandomVariableTest,RandomParamCombinationTest){
        MP_model testModel2(new OsiClpSolverInterface());
        double testValue = 3;
        MP_stage T(2);
        MP_scenario_set scen(10);
        double scenarios[1][10] = {{0,1,2,3,4,5,6,7,8,9}};
        double testValues[2] = {-3,3};
        MP_random_data rvar(&scenarios[0][0],T);
        MP_variable x;
        MP_variable y(T);
        y.lowerLimit(T) = 1;
        MP_constraint constraint1(T),constraint2(T);
        MP_data testData(&testValues[0],T);
        MP_random_data combinedData(T);
        combinedData(T) = testData(T)*rvar(T); // Necessitates an update for index expressions? Indices on the right hand side should store the same index as item on the left hand side, so if left hand side item changes, all other indices change too. This necessitates an index update mechanism, so that they are linked. => Indices need to be stored as pointers. 
        constraint1(T+1) = x()-y(T+1)*combinedData(T+1) >= 7;
        // This Constraint does not hold for all recourse variables (aka global constraint), but for each scenario individually.
        constraint2(T) = y(T) >= 20;
        testModel2.setObjective(x());
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        //TODO: We need to test the results, as otherwise this test is meaningless
        //EXPECT_EQ(1,testModel2.RandomVariables.size());
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        EXPECT_EQ(547,testModel2->getObjValue());
        writeReadSMPSTest(testModel2,MP_model::MINIMIZE,"test");
        compareEVPTest(testModel2,MP_model::MINIMIZE);
    }

    TEST_F(MP_modelTest,InvalidArgumentTest){
        MP_model testModel(new OsiClpSolverInterface()); // Without Solver
        ASSERT_THROW(MP_random_data();,invalid_argument_exception);

    }

    // Test if RV in MP_subset's work correctly
    TEST_F(MP_modelTest,RandomSubsetTest){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger(), 2617119269);
        testModel.setSample(true);

        enum {numStage = 3};
        MP_stage T(numStage);

        enum {NodeA, NodeB, NodeC, NodeD, numNodes};
        MP_set nodes(numNodes);
        nodes.setName("nodes");

        MP_subset<2> arcs(nodes,nodes);
        arcs.insert(NodeA,NodeB);
        arcs.insert(NodeA,NodeC);
        arcs.insert(NodeA,NodeD);
        arcs.insert(NodeB, NodeC);
        arcs.insert(NodeB,NodeD);
        arcs.insert(NodeC,NodeD);

        MP_random_data zeroOne(T);
        zeroOne.setName("zeroOne");

        MP_data zeroTest(T);
        zeroTest(T) = 1;
        zeroTest.display("zeroTest");

        zeroOne(1) = new DiscreteBernoulliRandomVariable(0.5);
        zeroOne(1)->setStage(1);
        zeroOne(2) = new DiscreteUniformRandomVariable(5,6,3);
        zeroOne(2)->setStage(2);

        MP_random_data combined(T);
        combined(T) = zeroTest(T) + zeroOne(T);

        MP_data testData(T,arcs);

        MP_random_data arcsInStage(T,arcs);
        arcsInStage.setName("arcsInStage");
        forall(T, testData(T,arcs) = 0 );
        forall(arcs, testData(T.last(),arcs) = zeroTest(T.last()) );
        testData.display("testData");
        forall(T,arcsInStage(T,arcs) = 0 );
        arcsInStage(T,arcs) = combined(T);
        arcsInStage.display("arcsInStage");
        //arcsInStage(T+1,arcs) = zeroOne(T+1);

        MP_variable x(T);

        MP_constraint constraint1(T);

        constraint1(T+1) = sum(arcs, arcsInStage(T+1,arcs)) <= x(T+1);
        MP_expression objective( x(T.last())+x(1) );

        testModel.setObjective(objective);
        testModel.solve(MP_model::MINIMIZE);
        EXPECT_EQ(51,testModel->getObjValue());
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());

    }

    TEST_F(UseCaseTest,UnaryMinusTest){
        MP_model testModel(new OsiClpSolverInterface());

        MP_variable x;
        MP_data y;
        y() = 5;

        MP_constraint z1,z2;
        z1() = -x() >= -10;
        z2() = -y() >= -x();

        testModel.setObjective(x());
        testModel.solve(MP_model::MAXIMIZE);
        ASSERT_EQ(10,testModel->getObjValue());
        testModel.solve(MP_model::MINIMIZE);
        ASSERT_EQ(5,testModel->getObjValue());

    }

    TEST(UnitTest,OldUnitTest){
        MP_model testModel(new OsiClpSolverInterface);
        enum {airA, airB, airC, airD, numA};
        enum {route_1, route_2, route_3, route_4, route_5, numRoutes};
        const int numDemandStates = 5;

        MP_set i(numA); // aircraft types and unassigned passengers
        MP_set j(numRoutes);      // assigned and unassigned routes
        MP_set h(numDemandStates);// demand states 

        double ddval[5][5] = {{200,     220,    250,    270,    300},
        { 50,     150,      0,      0,      0},  
        {140,     160,    180,    200,    220},
        { 10,      50,     80,    100,    340},
        {580,     600,    620,      0,      0}};

        double lambdaval[5][5] =
        {{.2,     .05,    .35,    .2,     .2},
        {.3,     .7,    0.0,    0.0,    0.0},
        {.1,     .2,     .4,     .2,     .1},
        {.2,     .2,     .3,     .2,     .1},
        {.1,     .8,     .1,    0.0,    0.0}};


        double cval[4][5] =
        {{18,         21,         18,          16,           10},
        {0,          15,         16,          14,           9},
        {0,          10,          0,          9,            6},
        {17,         16,         17,          15,           10}};

        double pval[4][5] =
        {{16,         15,          28,          23,          81},
        {0,         10,          14,         15,          57},
        {0,          5,           0,           7,          29},
        {9,         11,          22,          17,          55}};

        MP_data dd(&ddval[0][0],j,h); //    dd.value(&ddval[0][0]);
        MP_data lambda(&lambdaval[0][0],j,h); //lambda.value(&lambdaval[0][0]);
        MP_data c(&cval[0][0],i,j); //     c.value(&cval[0][0]);
        MP_data p(&pval[0][0],i,j); //     p.value(&pval[0][0]);

        c.display("c");

        MP_data 
            aa(i),      // aircraft availiability 
            k(j),       // revenue lost (1000 per 100  bumped) 
            ed(j),      // expected demand
            gamma(j,h), // probability of exceeding demand increment h on route j
            deltb(j,h); // incremental passenger load in demand states;

        aa(airA) = 10;  aa(airB) = 19;  
        aa(airC) = 25;  aa(airD) = 15;

        k(route_1) = 13;
        k(route_2) = 13;
        k(route_3) =  7;
        k(route_4) =  7;
        k(route_5) =  1;

        ed(j) = sum(h, lambda(j,h)*dd(j,h));

        MP_index hp;

        gamma(j,h) = sum( h(hp).such_that(hp >= h), lambda(j,hp));

        deltb(j,h) = pos(dd(j,h)-dd(j,h-1));

        lambda.display("lambda");
        ed.display("ed");
        gamma.display("gamma");
        deltb.display("deltb");
        aa.display("aa");

        MP_variable
            x(i,j),   // number of aircraft type i assigned to route j
            y(j,h),   // passengers actually carried
            b(j,h),   // passengers bumped
            oc,       // operating cost
            bc;       // bumping cost

        MP_constraint
            ab(i),    // aircraft balance
            db(j),    // demand balance
            yd(j,h),  // definition of boarded passangers
            bd(j,h),  // definition of bumped passangers
            ocd,     // 
            bcd1,     // bumping cost definition: version 1
            bcd2;     // bumping cost definition: version 2


        ab(i) =   sum(j, x(i,j)) <= aa(i);
        ab.setName("aircraft balance");

        db(j) =  sum(i, p(i,j)*x(i,j)) >= sum( h.such_that(deltb(j,h)>0), y(j,h));
        db.setName("demand balance");

        yd(j,h) = y(j,h) <= sum(i, p(i,j)*x(i,j));
        yd.setName("definition of boarded passengers");

        bd(j,h) = b(j,h) == dd(j,h) - y(j,h);
        bd.setName("definition of bumped passengers");

        ocd() =     oc() == sum(i*j, c(i,j)*x(i,j));

        bcd1() =    bc() == sum(j, k(j)*(ed(j)-sum(h, gamma(j,h)*y(j,h))));

        bcd2() =    bc() == sum(j*h, k(j)*lambda(j,h)*b(j,h));

        MP_model m1(new OsiCbcSolverInterface), m2(new OsiCbcSolverInterface);

        y.upperLimit(j,h) = deltb(j,h);

        //y.upperLimit.display("y upper");

        m1.add(ab).add(db).add(bcd1).add(ocd);
        m1.minimize(oc() + bc());

        EXPECT_EQ(11, m1->getNumRows());
        EXPECT_EQ(47, m1->getNumCols());
        EXPECT_EQ(96, m1->getNumElements());
        EXPECT_GE(m1->getObjValue(),1566.03);
        EXPECT_LE(m1->getObjValue(),1566.05);

        y.display("y first model");
        ab.display("ab");
        db.display("db");
        bcd1.display("bcd1");
        ocd.display("ocd");
        oc.display("operation cost");
        x.display("x");

        m2.add(ab).add(yd).add(bd).add(bcd2).add(ocd);
        y.upperLimit(j,h) = m2->getInfinity();

        m2.minimize(oc() + bc());

        EXPECT_EQ(56, m2->getNumRows());
        EXPECT_EQ(72, m2->getNumCols());
        EXPECT_EQ(219, m2->getNumElements());

        // Optimal objective value m1: 1566.04218913
        // Optimal objective value m2: 1566.04218913 (like m1)
        EXPECT_GE(m2->getObjValue(),1566.03);
        EXPECT_LE(m2->getObjValue(),1566.05);

        EXPECT_DOUBLE_EQ( 1566.042189132706,m2->getObjValue());
        EXPECT_DOUBLE_EQ( 1566.0421891327067,m1->getObjValue());
        y.display("y second model");
    }

    TEST_F(FlopCppExamples, Aircraft){
        /*The objective of this model is to allocate aircrafts to routes to maximize
        the expected profit when traffic demand is uncertain. Two different
        formulations are used, the delta and the lambda formualation.

        Dantzig, G B, Chapter 28. In Linear Programming and Extensions.
        Princeton University Press, Princeton, New Jersey, 1963.
        */
        enum {a1, a2, a3, a4, numA}; // Aircrafts
        enum {route_1, route_2, route_3, route_4, route_5, numRoutes};
        const int numDemandStates = 5;

        MP_set i(numA); // aircraft types and unassigned passengers
        MP_set j(numRoutes);      // assigned and unassigned routes
        MP_set h(numDemandStates);// demand states 

        double ddval[5][5] = {{200,     220,    250,    270,    300},
        { 50,     150,      0,      0,      0},  
        {140,     160,    180,    200,    220},
        { 10,      50,     80,    100,    340},
        {580,     600,    620,      0,      0}};

        double lambdaval[5][5] =
        {{.2,     .05,    .35,    .2,     .2},
        {.3,     .7,    0.0,    0.0,    0.0},
        {.1,     .2,     .4,     .2,     .1},
        {.2,     .2,     .3,     .2,     .1},
        {.1,     .8,     .1,    0.0,    0.0}};


        double cval[4][5] =
        {{18,         21,         18,          16,           10},
        {0,          15,         16,          14,           9},
        {0,          10,          0,          9,            6},
        {17,         16,         17,          15,           10}};

        double pval[4][5] =
        {{16,         15,          28,          23,          81},
        {0,         10,          14,         15,          57},
        {0,          5,           0,           7,          29},
        {9,         11,          22,          17,          55}};

        MP_data dd(&ddval[0][0],j,h); //    dd.value(&ddval[0][0]);
        MP_data lambda(&lambdaval[0][0],j,h); //lambda.value(&lambdaval[0][0]);
        MP_data c(&cval[0][0],i,j); //     c.value(&cval[0][0]);
        MP_data p(&pval[0][0],i,j); //     p.value(&pval[0][0]);

        c.display("c");

        MP_data 
            aa(i),      // aircraft availiability 
            k(j),       // revenue lost (1000 per 100  bumped) 
            ed(j),      // expected demand
            gamma(j,h), // probability of exceeding demand increment h on route j
            deltb(j,h); // incremental passenger load in demand states;

        aa(a1) = 10;  aa(a2) = 19;  
        aa(a3) = 25;  aa(a4) = 15;

        k(route_1) = 13;
        k(route_2) = 13;
        k(route_3) =  7;
        k(route_4) =  7;
        k(route_5) =  1;

        ed(j) = sum(h, lambda(j,h)*dd(j,h));

        MP_index hp;

        gamma(j,h) = sum( h(hp).such_that(hp >= h), lambda(j,hp));

        deltb(j,h) = pos(dd(j,h)-dd(j,h-1));

        ed.display("ed");
        gamma.display("gamma");
        deltb.display("deltb");
        aa.display("aa");

        MP_variable
            x(i,j),   // number of aircraft type i assigned to route j
            y(j,h),   // passengers actually carried
            b(j,h),   // passengers bumped
            oc,       // operating cost
            bc;       // bumping cost

        MP_constraint
            ab(i),    // aircraft balance
            db(j),    // demand balance
            yd(j,h),  // definition of boarded passangers
            bd(j,h),  // definition of bumped passangers
            ocd,     // 
            bcd1,     // bumping cost definition: version 1
            bcd2;     // bumping cost definition: version 2


        ab(i) =   sum(j, x(i,j)) <= aa(i);
        ab.setName("aircraft balance");

        db(j) =  sum(i, p(i,j)*x(i,j)) >= sum( h.such_that(deltb(j,h)>0), y(j,h));
        db.setName("demand balance");

        yd(j,h) = y(j,h) <= sum(i, p(i,j)*x(i,j));
        yd.setName("definition of boarded passengers");

        bd(j,h) = b(j,h) == dd(j,h) - y(j,h);
        bd.setName("definition of bumped passengers");

        ocd() =     oc() == sum(i*j, c(i,j)*x(i,j));

        bcd1() =    bc() == sum(j, k(j)*(ed(j)-sum(h, gamma(j,h)*y(j,h))));

        bcd2() =    bc() == sum(j*h, k(j)*lambda(j,h)*b(j,h));

        MP_model m1(new OsiCbcSolverInterface), m2(new OsiCbcSolverInterface);

        y.upperLimit(j,h) = deltb(j,h);

        //y.upperLimit.display("y upper");

        m1.add(ab).add(db).add(bcd1).add(ocd);
        m1.minimize(oc() + bc());

        EXPECT_EQ(11, m1->getNumRows());
        EXPECT_EQ(47, m1->getNumCols());
        EXPECT_EQ(96, m1->getNumElements());
        EXPECT_NEAR(1566.04, m1->getObjValue(), 0.01);

        y.display("y first model");
        ab.display("ab");
        db.display("db");
        bcd1.display("bcd1");
        ocd.display("ocd");

        m2.add(ab).add(yd).add(bd).add(bcd2).add(ocd);
        y.upperLimit(j,h) = m2->getInfinity();

        m2.minimize(oc() + bc());

        EXPECT_EQ(56, m2->getNumRows());
        EXPECT_EQ(72, m2->getNumCols());
        EXPECT_EQ(219, m2->getNumElements());
        EXPECT_NEAR(1566.04, m2->getObjValue(), 0.01);

        y.display("y second model");
        cout<<"Test aircraft passed."<<endl;
    }

    TEST_F(FlopCppExamples, AMPL){
        /*   A sample problem to demonstrate the power of modeling systems
        Fourer, R, Gay, D M, and Kernighan, B W, AMPL: A Mathematical Programming
        Language. AT\&T Bell Laboratories, Murray Hill, New Jersey, 1987.
        */
        OsiClpSolverInterface osiClpSolver;
        MP_model testModel(new OsiCbcSolverInterface(&osiClpSolver));
        testModel.verbose();

        enum {iron, nickel, numRaw};
        enum {nuts, bolts, washers, numPrd};
        const int numT = 4;

        MP_set 
            prd(numPrd),    // products        / nuts, bolts, washers /
            raw(numRaw),    // raw materials   / iron, nickel /
            TL(numT+1);     // extended t      / 1 * 5 /

        MP_subset<1>
            T(TL),     //  periods     / 1 * 4 /
            first(TL), //          / 1     /
            last(TL);  //           /     5 /

        T.insert(0);
        T.insert(1);
        T.insert(2);
        T.insert(3);

        first.insert(0);
        last.insert(4);

        MP_data istock(raw); // initial stock  /  iron  35.8 , nickel  7.32  /
        MP_data scost(raw); // storage cost   /  iron    .03, nickel   .025 /
        MP_data rvalue(raw); //  residual value /  iron    .02, nickel  -.01  /

        istock(iron) = 35.8;   istock(nickel) = 7.32;
        scost(iron) =  0.03;  scost(nickel) = 0.025;
        rvalue(iron) =  0.02;  rvalue(nickel) =-0.01;

        double  m = 123; //  maximum production 

        MP_data units(raw,prd); // raw material inputs to produce a unit of product

        double avalue[numRaw][numPrd] =  {{ .79, .83, .92 }, 
        { .21, .17, .08 }};

        units.value(&avalue[0][0]);

        MP_data profit(prd,T); // profit

        double cvalue[numPrd][numT] = {{ 1.73,  1.8,  1.6,  2.2 },
        { 1.82,  1.9,  1.7,   .95},
        { 1.05,  1.1,   .95, 1.33}};

        profit.value(&cvalue[0][0]);

        MP_variable  x(prd,TL); // production level
        x.setName("X");
        MP_variable  s(raw,TL); // storage at beginning of period

        MP_index p,r,t;

        MP_constraint  
            limit(T),       // capacity constraint
            balance(raw,TL);  // raw material balance


        limit(T) =   sum(prd(p), x(p,T)) <= m;

        balance(r,TL+1) = 
            s(r,TL+1) == s(r,TL) - sum(prd(p), units(r,p)*x(p,TL));

        s.upperLimit(r,0) = istock(r);


        // MP_model::default_model.verbose();

        testModel.maximize( 
            sum(T(t), sum(prd(p), profit(p,t)*x(p,t)) -
            sum(raw(r), scost(r)*s(r,t) )) +
            sum(raw(r), rvalue(r)*s(r,numT)) 
            );

        EXPECT_EQ(14, testModel->getNumRows());
        EXPECT_EQ(25, testModel->getNumCols());
        EXPECT_EQ(52, testModel->getNumElements());
        EXPECT_NEAR(79.3413, testModel->getObjValue(), 0.0001);

        x.display();
        s.display();
        cout<<"Test ampl passed."<<endl;
    }

    TEST_F(FlopCppExamples, DISABLED_bid){ // formularion for setup doesnt work (see below)
        // Adapted from bid.gms from the GAMS model library:
        //    http://www.gams.com/

        /* A company obtains a number of bids from vendors for the supply
        of a specified number of units of an item. Most of the submitted
        bids have prices that depend on the volume of business.

        Bracken, J, and McCormick, G P, Chapter 3. In Selected Applications of
        Nonlinear Programming. John Wiley and Sons, New York, 1968, pp. 28-36.
        */
        MP_model bid(new OsiCbcSolverInterface);

        enum { a, b, c, d, e, nbrV};
        const int numSegments = 5;

        MP_set V(nbrV);           // vendors     
        MP_set S(numSegments);    // segments    

        MP_subset<2> VS(V,S); // vendor bid possibilities

        VS.insert(a,0);
        VS.insert(b,0);
        VS.insert(b,1);
        VS.insert(b,2);
        VS.insert(b,3);
        VS.insert(c,0);
        VS.insert(d,0);
        VS.insert(e,0);
        VS.insert(e,1);

        MP_data setup(VS);
        MP_data price(VS);
        MP_data qmin (VS);
        MP_data qmax (VS);

        double req = 239600.48;

        setup(VS(a,0)) =   3855.84;
        setup(VS(b,0)) = 125804.84;
        setup(VS(c,0)) =  13456.00;
        setup(VS(d,0)) =   6583.98;

        price(VS(a,0)) = 61.150;
        price(VS(b,0)) = 68.099;
        price(VS(b,1)) = 66.049;
        price(VS(b,2)) = 64.099;
        price(VS(b,3)) = 62.119;
        price(VS(c,0)) = 62.190;
        price(VS(d,0)) = 72.488;
        price(VS(e,0)) = 70.150;
        price(VS(e,1)) = 68.150;

        qmin(VS(b,0)) =   22000;
        qmin(VS(b,1)) =   70000;
        qmin(VS(b,2)) =  100000;
        qmin(VS(b,3)) =  150000;
        qmin(VS(e,1)) =   42000;

        qmax(VS(a,0)) =  33000;
        qmax(VS(b,0)) =  70000;
        qmax(VS(b,1)) = 100000;
        qmax(VS(b,2)) = 150000;
        qmax(VS(b,3)) = 160000;
        qmax(VS(c,0)) = 165600;
        qmax(VS(d,0)) =  12000;
        qmax(VS(e,0)) =  42000;
        qmax(VS(e,1)) =  77000;

        //original formulation doesnt work - index error
        setup(VS(V,S+1)) = setup(VS(V,S)) + qmax(VS(V,S)) * (price(VS(V,S)) - price(VS(V,S+1)));
        //tried this one, no error, but objective is far away from expected
        //forall(VS(V,S).such_that(S > 0), setup(VS(V,S)) = setup(VS(V,S-1)) + qmax(VS(V,S-1)) * (price(VS(V,S-1)) - price(VS(V,S))));

        //    setup(VS(V,S+1)) = setup(VS(V,S)) +
        //	qmax(VS(V,S))*(price(VS(V,S))-price(VS(V,S+1)));

        setup.display("setup");

        MP_variable         pl(VS);    //  purchase level
        MP_binary_variable  plb(VS);   //  purchase decision

        MP_constraint 
            demand,    // demand constraint
            minpl(VS), // min purchase
            maxpl(VS), // max purchase
            oneonly(V);// at most one deal;

        demand() =   req == sum( VS, pl(VS) );

        minpl(VS) = pl(VS) >= qmin(VS)*plb(VS);

        maxpl(VS) = pl(VS) <= qmax(VS)*plb(VS);

        oneonly(V) =  sum(VS(V,S), plb(VS)) <= 1;

        bid.verbose();

        bid.minimize( sum(VS, price(VS)*pl(VS) + setup(VS)*plb(VS)) );

        EXPECT_EQ(24, bid->getNumRows());
        EXPECT_EQ(18, bid->getNumCols());
        EXPECT_EQ(50, bid->getNumElements());
        EXPECT_NEAR(1.52101e+07, bid->getObjValue(), 100);

        cout<<"Test bid passed."<<endl;
    }

    TEST_F(FlopCppExamples, coex){
        /* Peacefully Coexisting Armies of Queens 

        Two armies of queens (balvk and white) peacefully coexsit on a
        chessboard when they are placed pon the board in such a way that
        no two queens from opposing armies can attack each other. The
        problem is to find the maximum two equal-sized armies.


        Bosch, R, Mind Sharpener. OPTIMA MPS Newsletter (2000).
        */
        MP_model testModel(new OsiCbcSolverInterface);

        MP_set I(5);  // size of chess board

        MP_subset<4> M(I,I,I,I); // shared positions on the board;

        MP_index i,j,ii,jj;

        forall(
            I(i)*I(j)*I(ii)*I(jj).such_that(i==ii||j==jj || abs(i-ii)==abs(j-jj)), 
            M.insert(i,j,ii,jj) 
            );

        MP_variable 
            b(I,I), // square occupied by a black Queen
            w(I,I), // square occupied by a white Queen
            tot;    // total queens in each army;

        b.binary();
        w.binary();

        MP_constraint
            eq1(M), // keeps armies at peace
            eq2,    // add up all the black queens
            eq3;    // add up all the white queens;


        eq1(M(i,j,ii,jj)) =   b(i,j) + w(ii,jj) <= 1;

        eq2() =  tot() == sum(I(i)*I(j), b(i,j));

        eq3() =  tot() == sum(I(i)*I(j), w(i,j));

        testModel.maximize(tot());

        testModel->writeMps("coex","mps",1.0);
        ASSERT_EQ(0,remove("coex.mps"));

        EXPECT_EQ(347, testModel->getNumRows());
        EXPECT_EQ(51, testModel->getNumCols());
        EXPECT_EQ(742, testModel->getNumElements());
        EXPECT_EQ(4, testModel->getObjValue());

        b.display("Black");
        w.display("White");

        cout<<"Test coex passed."<<endl;
    }

    TEST_F(FlopCppExamples, coexx){
        /* Peacefully Coexisting Armies of Queens - tight 

        This is a tighter formulation than the original COEX problem.
        We have set the size of the board to 5 in order to find
        solutions quickly. In addition we fix the position of one queen.

        Two armies of queens (balvk and white) peacefully coexsit on a
        chessboard when they are placed pon the board in such a way that
        no two queens from opposing armies can attack each other. The
        problem is to find the maximum two equal-sized armies.

        Bosch, R, Mind Sharpener. OPTIMA MPS Newsletter (2000).
        */

        MP_model testModel(new OsiCbcSolverInterface);

        const int boardSize = 5;

        MP_set 
            i(boardSize),  
            s(2*boardSize-3); // diagonal offsets

        MP_data
            sh(s),    // shift values for diagonals
            rev(s,i); // reverse shift order;

        sh(s)  = s - i.size() + 1;

        rev(s,i) = i.size() + 1 - 2*Constant(i) + sh(s);

        MP_binary_variable 
            xw(i,i), // has a white queen
            xb(i,i), // has a black queen
            wa(i),   // white in row i
            wb(i),   // white in column j
            wc(s),   // white in diagonal s
            wd(s);   // white in backward diagonal s;

        MP_variable tot;

        MP_index j;

        MP_constraint
            aw(i,i), // white in row i
            bw(i,i), // white in column j
            cw(s,i), // white in diagonal s
            dw(s,i), // white in backward diagonal s
            ew,      // total white
            ab(i,i), // black in row i
            bb(i,i), // black in column j
            cb(s,i), // black in diagonal s
            db(s,i), // black in backward diagonal s
            eb;      // total black;


        aw(i,j) =  wa(i) >=  xw(i,j);

        bw(j,i) =  wb(j) >=  xw(i,j);

        cw(s,i) =  wc(s) >=  xw(i,i+sh(s));

        dw(s,i) =  wd(s) >= xw(i,i+rev(s,i));

        ab(i,j) =  1-wa(i) >= xb(i,j);

        bb(j,i) =  1-wb(j) >= xb(i,j);

        cb(s,i) =  1-wc(s) >= xb(i,i+sh(s));

        db(s,i) =  1-wd(s) >= xb(i,i+rev(s,i));

        eb() =   tot() == sum(i*i(j), xb(i,j));

        ew() =   tot() == sum(i*i(j), xw(i,j));

        xb.lowerLimit(0,0) = 1;

        testModel.maximize(tot());

        EXPECT_EQ(242, testModel->getNumRows());
        EXPECT_EQ(75, testModel->getNumCols());
        EXPECT_EQ(480, testModel->getNumElements());
        EXPECT_EQ(4, testModel->getObjValue());

        xb.display("Black");
        xw.display("White");

        cout<<"Test coexx passed."<<endl;
    }

    TEST_F(FlopCppExamples, cross){
        /* Alcuin's River Crossing  

        A farmer carrying a bushel of corn and accompanied by a goose
        and a wolf came to a river. He found a boat capable of
        transporting himself plus one of his possessions - corn, goose, or
        wolf - but no more. Now, he couldn't leave the corn alone with
        the goose, nor the goose alone with the wolf, else one would
        consume the other. Nevertheless, he succeeded in getting himself
        and his goods across the river safely.


        Borndoerfer, R, Groetschel, M, and Loebel, A, Alcuin's
        Transportation Problem and Integer Programming. Konrad Zuse
        Zentrum for Informationstechnik, Berlin, 1995.

        Contributed by Soren Nielsen, Institute for Mathematical Sciences
        University of Copenhagen
        */
        MP_model testModel(new OsiCbcSolverInterface);

        enum {goose, wolf, corn, numI};

        MP_set
            i(numI),  // /goose, wolf, corn/
            t(10);    // time  

        MP_data dir(t);  // crossing - near to far is +1 - far to near -1;

        for (int k=0; k<t.size(); k++) {
            dir(k) = pow((double)-1,(double)k);
        }

        dir.display("dir");

        MP_variable
            cross(i,t), //  crossing the river
            done(t);    //  all items in far side

        MP_binary_variable
            y(i,t);     //  1 iff the item is on the far side at time t

        MP_constraint
            DefDone(i,t),  // everything on far side
            DefCross(i,t), // crossing
            LimCross(t),   
            EatNone1(t),
            EatNone2(t);

        DefCross(i,t) =  y(i,t) == y(i,t-1) + dir(t-1)*cross(i,t-1);

        DefDone(i,t)  =  done(t) <= y(i,t);

        LimCross(t) =  sum(i, cross(i,t-1)) <= 1;

        EatNone1(t) =   dir(t)*(y(goose,t) + y(wolf,t) - 1) <= done(t) ;

        EatNone2(t) =   dir(t)*(y(goose,t) + y(corn,t) - 1) <= done(t) ;

        y.upperLimit(i,0) = 0;

        testModel.maximize(sum(t, done(t)));

        EXPECT_EQ(90, testModel->getNumRows());
        EXPECT_EQ(70, testModel->getNumCols());
        EXPECT_EQ(231, testModel->getNumElements());
        EXPECT_EQ(3, testModel->getObjValue());

        y.display("y");
        cross.display("cross");

        cout<<"Test cross passed."<<endl;
    }

    namespace cuttingStock {
        // this belongs to the following cutting stock example
        // --->
        class Paper : public MP_model {
        public:
            MP_set WIDTHS,PATTERNS;
            MP_data widths;
            MP_data demand;
            MP_data pattern;
            MP_variable use;
            MP_constraint demC;

            Paper(int numWidths) : MP_model(new OsiCbcSolverInterface),
                WIDTHS(numWidths), PATTERNS(numWidths),
                widths(WIDTHS), demand(WIDTHS),
                pattern(WIDTHS,PATTERNS), use(WIDTHS),
                demC(WIDTHS) {

                    MP_index i,j;

                    demC(i) = sum(WIDTHS(j), pattern(i,j)*use(j)) >= demand (i);

                    setObjective( sum(WIDTHS(j), use(j)) );
            }
        };

        double knapsack(int n, double* a, const double* c, double b, double* pat) {
            MP_model KnapSack(new OsiCbcSolverInterface);

            MP_set I(n);
            MP_data A(I);
            MP_data C(I);

            A.value(a);
            C.value(c);

            A.display("A");
            C.display("C");

            MP_variable x(I);
            x.integer();

            MP_constraint constr;

            constr = sum(I, A(I)*x(I)) <= b;
            KnapSack.add(constr);
            KnapSack.maximize( sum(I, C(I)*x(I)) );

            x.display("x");
            for (int i=0; i<n; i++) {
                pat[i] = x.level(i);
            }
            return KnapSack->getObjValue();
        }

        TEST_F(FlopCppExamples, cuttingStock){
            const int numWidths = 5;
            double tabDemand[] = {150, 96, 48, 108, 227};
            double tabWidth[] = {17, 21, 22.5, 24, 29.5};
            const double maxWidth = 94;

            MP_model Paper(new OsiCbcSolverInterface);

            MP_set WIDTHS(numWidths);
            MP_set PATTERNS(numWidths);
            MP_data width(WIDTHS);
            MP_data demand(WIDTHS);
            MP_data pattern(WIDTHS,PATTERNS);

            width.value(tabWidth);
            demand.value(tabDemand);

            for (int i=0; i<numWidths; i++) {
                pattern(i,i) = ::floor(maxWidth/width(i));
            }

            pattern.display("pattern");

            MP_variable use(PATTERNS);
            MP_index i,j;
            MP_constraint demC(WIDTHS);

            demC(i) = sum(WIDTHS(j), pattern(i,j)*use(j)) >= demand (i);

            Paper.add(demC);
            Paper.minimize( sum(WIDTHS(j), use(j)) );

            double pat[numWidths];
            double ob;
            do {
                ob = knapsack(numWidths, tabWidth, Paper.Solver->getRowPrice()+demC.offset, maxWidth, pat);

                CoinPackedVector Pat;	
                Pat.setFull(numWidths, pat);

                Paper->addCol(Pat,0,100,1);
                Paper->resolve();
            } while(ob>1.0001);

            Paper.minimize( sum(WIDTHS(j), use(j)) );

            EXPECT_EQ(5, Paper->getNumRows());
            EXPECT_EQ(5, Paper->getNumCols());
            EXPECT_EQ(5, Paper->getNumElements());
            EXPECT_NEAR(177.666, Paper->getObjValue(), 0.001);

            cout<<"Test cuttingstock passed."<<endl;
        }
    } // end namespace cuttingStock

    namespace exampleSmi {
        void SmpsIO(const char * const name )
        {
            SmiScnModel smi;	

            // read SMPS model from files
            //	<name>.core, <name>.time, and <name>.stoch
            smi.readSmps(name);		

            // generate OSI solver object
            // 	here we use OsiClp
            OsiClpSolverInterface *clp = new OsiClpSolverInterface();

            // set solver object for SmiScnModel
            smi.setOsiSolverHandle(*clp);	

            // load solver data
            // 	this step generates the deterministic equivalent 
            //	and returns an OsiSolver object 
            OsiSolverInterface *osiStoch = smi.loadOsiSolverData();

            // set some nice Hints to the OSI solver
            osiStoch->setHintParam(OsiDoPresolveInInitial,true);
            osiStoch->setHintParam(OsiDoScale,true);
            osiStoch->setHintParam(OsiDoCrash,true);

            // solve
            osiStoch->initialSolve();		

            // print results
            printf("Solved stochastic program %s\n", name);
            printf("Number of rows: %d\n",osiStoch->getNumRows());
            printf("Number of cols: %d\n",osiStoch->getNumCols());
            printf("Optimal value: %g\n",osiStoch->getObjValue());		

            // print solution to file
            string outfilename(name);
            const string suffix(".out");
            outfilename = outfilename + suffix;
            FILE *fp = fopen(outfilename.c_str(),"w");
            int numScenarios=smi.getNumScenarios();

            for (int i=0 ; i<numScenarios; ++i) {
                double *dsoln=NULL;
                int numCols=0;
                fprintf(fp,"Scenario %d \n",i);
                dsoln = smi.getColSolution(i,&numCols);
                for (int j=0; j<numCols; j++)
                    fprintf(fp,"%g \n",dsoln[j]);
                free(dsoln);
            }
            fclose(fp);


        }	

        void ModelScenario(const char * const name )
        {
            OsiClpSolverInterface *osiClp1 = new OsiClpSolverInterface();
            double INF=osiClp1->getInfinity();

            // example of direct interfaces for scenario generation

            /* Model dimensions */
            int ncol=27, nrow=9, nels=44;

            /* Sparse matrix data...organized by row */
            int mrow[]={ 0, 0, 0, 0, 0,
                1, 1, 1, 1,
                2, 2, 2,
                3, 3, 3, 3, 3,
                4, 4, 4, 4,
                5, 5, 5, 5, 5, 5,
                6, 6, 6, 6, 6,
                7, 7, 7, 7, 7, 7,
                8, 8, 8, 8, 8, 8 };
            int mcol[]={ 0, 1, 2, 3, 4,
                5, 6, 7, 8,
                9,10, 11, 
                12, 13, 14, 15, 16, 
                0,        12, 17, 18,
                1, 5, 9,  13, 19, 20,
                2, 6,     14, 21, 22,
                3, 7, 10, 15, 23, 24,
                4, 8, 11, 16, 25, 26 };

            double dels[] = { 1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0,
                16.0,              9.0, -1.0, 1.0,
                15.0, 10.0,  5.0, 11.0, -1.0, 1.0,
                28.0, 14.0,       22.0, -1.0, 1.0,
                23.0, 15.0,  7.0, 17.0, -1.0, 1.0,
                81.0, 57.0, 29.0, 55.0, -1.0, 1.0 };

            /* Objective */
            double dobj[]={ 18.0, 21.0, 18.0, 16.0, 10.0, 15.0, 16.0, 14.0, 9.0,
                10.0,  9.0,  6.0, 17.0, 16.0, 17.0, 15.0, 10.0, 0.0,
                13.0,  0.0, 13.0,  0.0,  7.0,  0.0,  7.0,  0.0, 1.0 };

            /* Column bounds */
            double dclo[]={ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
                0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
                0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0 };



            double dcup[]={ INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,
                INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,
                INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF };

            /* Row bounds */
            double drlo[]={ -INF, -INF, -INF, -INF,  0.0, 4.0, 0.0, 8.0, 10.0 };
            double drup[]={ 10.0, 19.0, 25.0, 15.0,  0.0, 7.0, 0.0, 8.0, 90.0 };

            /* Stages */
            int nstg=2;
            int n_first_stg_rows=4;
            int rstg[]={ 0,0,0,0,1,1,1,1,1 };
            int cstg[]={ 0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1 };

            /* Stochastic data */
            int nindp=5;
            int nsamp[]={ 5, 2, 5, 5, 3 };
            double demand[]={ 200, 220, 250, 270, 300,
                50, 150,
                140, 160, 180, 200, 220,
                10, 50, 80, 100, 340,
                580, 600, 620 };
            double dprobs[]={ 0.2, 0.05, 0.35, 0.2, 0.2,
                0.3, 0.7,
                0.1, 0.2, 0.4, 0.2, 0.1,
                0.2, 0.2, 0.3, 0.2, 0.1,
                0.1, 0.8, 0.1 };

            /* local variables */
            int ns=1,ii,iii,jj,*indx,*incr;
            double dp=1.0;

            for (ii=0;ii<nindp;ii++) ns *= nsamp[ii];     /* Compute number of scenarios */


            // initialize SmiModel
            SmiScnModel *smiModel = new SmiScnModel();
            smiModel->setOsiSolverHandle(*osiClp1);

            // set core model using Osi interface
            OsiClpSolverInterface ocsi;
            ocsi.loadProblem(CoinPackedMatrix( 1,mrow,mcol,dels,nels),dclo,dcup,dobj,drlo,drup);
            SmiCoreData *osiCore = new SmiCoreData(&ocsi,nstg,cstg,rstg);
            smiModel->setCore(osiCore);

            cout << "ModelScenario: Created CoreData" << endl;

            // Coin structures for scenario updates to right hand sides
            CoinPackedVector cpv_rlo;
            CoinPackedVector cpv_rup;

            // initialize right hand side data for first scenario
            indx = (int *) malloc( (1+nindp)*sizeof(int) );
            memset( indx,0,(1+nindp)*sizeof(int));
            for (jj=0;jj<nindp;jj++) {
                indx[jj+1] += indx[jj] + nsamp[jj];
                dp *= dprobs[ indx[jj] ];

                drlo[n_first_stg_rows + jj] = demand[ indx[jj] ];
                drup[n_first_stg_rows + jj] = demand[ indx[jj] ];

                cpv_rlo.insert(n_first_stg_rows + jj,demand[ indx[jj] ]);
                cpv_rup.insert(n_first_stg_rows + jj,demand[ indx[jj] ]);
            }

            cout << "ModelScenario: Adding " << ns << " scenarios" << endl;

            // first scenario
            int anc = 0;
            int branch = 1;
            int	is = smiModel->generateScenario(osiCore,NULL,NULL,NULL,NULL,
                &cpv_rlo,&cpv_rup,branch,anc,dp);	



            /***** ...main loop to generate scenarios from discrete random variables
            For each scenario index ii:
            If the sample size nsamp[jj] divides the scenario index ii,
            reverse the increment direction incr[jj]
            and increase the random variable index jj by 1.
            Increment the jj'th random variable by incr[jj]
            and generate new sample data.
            ***** */

            /* sample space increment initialized to 1 */
            incr = (int *) malloc( nindp*sizeof(int) );
            for (jj=0;jj<nindp;jj++) incr[jj] = 1;

            for (int iss=1;iss<ns;iss++) {
                iii=iss; jj=0;
                while ( !(iii%nsamp[jj]) ) {
                    iii /= nsamp[jj];
                    incr[jj] = -incr[jj];
                    jj++;
                }
                dp /= dprobs[ indx[jj] ];
                indx[jj] += incr[jj];
                dp *= dprobs[ indx[jj] ];

                // set data
                drlo[n_first_stg_rows + jj] = demand[ indx[jj] ];
                drup[n_first_stg_rows + jj] = demand[ indx[jj] ];

                cpv_rlo.setElement(cpv_rlo.findIndex(n_first_stg_rows + jj),demand[ indx[jj] ]);
                cpv_rup.setElement(cpv_rup.findIndex(n_first_stg_rows + jj),demand[ indx[jj] ]);

                // genScenario
                is = smiModel->generateScenario(osiCore,NULL,NULL,NULL,NULL,
                    &cpv_rlo,&cpv_rup,branch,anc,dp);	


            }

	        EXPECT_EQ(ns, smiModel->getNumScenarios());
            cout << "ModelScenario: Finished adding scenarios" << endl;



            // load problem data into OsiSolver
            smiModel->loadOsiSolverData();
            // get Osi pointer
            OsiSolverInterface *smiOsi = smiModel->getOsiSolverInterface();
            // set some parameters
            smiOsi->setHintParam(OsiDoPresolveInInitial,true);
            smiOsi->setHintParam(OsiDoScale,true);
            smiOsi->setHintParam(OsiDoCrash,true);
            // solve using Osi Solver
            smiOsi->initialSolve();
            // test optimal value
            EXPECT_LT(fabs(smiOsi->getObjValue()-1566.042), 0.01);

            // test solutions
            const double *dsoln = smiOsi->getColSolution();
            double objSum = 0.0;

            /* The canonical way to traverse the tree:
            For each scenario, get the leaf node.
            Then get the parent.  Repeat until parent is NULL.
            (Only the root node has a NULL parent.)
            */


            for(is=0; is<ns; ++is)
            {
                /* this loop calculates the scenario objective value */
                double scenSum = 0.0;

                // start with leaf node
                SmiScnNode *node = smiModel->getLeafNode(is);

                // leaf node probability is the scenario probability
                double scenprob = node->getModelProb();

                while (node != NULL)
                {

                    // getColStart returns the starting index of node in OSI model
                    for(int j=node->getColStart(); j<node->getColStart()+node->getNumCols(); ++j)
                    {
                        // getCoreColIndex returns the corresponding Core index
                        // in the original (user's) ordering
                        scenSum += dobj[node->getCoreColIndex(j)]*dsoln[j];	


                    }			
                    // get parent of node
                    node = node->getParent();
                }
                objSum += scenSum*scenprob;
            }

            EXPECT_LT(fabs(smiOsi->getObjValue()-objSum), 0.01);

            // print results
            printf("Solved stochastic program %s\n", name);
            printf("Number of rows: %d\n",smiOsi->getNumRows());
            printf("Number of cols: %d\n",smiOsi->getNumCols());
            printf("Optimal value: %g\n",smiOsi->getObjValue());

            // Clean up test
            delete smiModel;
            delete osiClp1;

            free(indx);
            free(incr);

        }

        void ModelDiscrete(const char * const name)
        {
            // example of direct interfaces for discrete distribution

            OsiClpSolverInterface *osiClp1 = new OsiClpSolverInterface();
            double INF=osiClp1->getInfinity();

            /* Model dimensions */
            int ncol=27, nrow=9, nels=44;

            /* Sparse matrix data...organized by row */
            int mrow[]={ 0, 0, 0, 0, 0,
                1, 1, 1, 1,
                2, 2, 2,
                3, 3, 3, 3, 3,
                4, 4, 4, 4,
                5, 5, 5, 5, 5, 5,
                6, 6, 6, 6, 6,
                7, 7, 7, 7, 7, 7,
                8, 8, 8, 8, 8, 8 };
            int mcol[]={ 0, 1, 2, 3, 4,
                5, 6, 7, 8,
                9,10, 11, 
                12, 13, 14, 15, 16, 
                0,        12, 17, 18,
                1, 5, 9,  13, 19, 20,
                2, 6,     14, 21, 22,
                3, 7, 10, 15, 23, 24,
                4, 8, 11, 16, 25, 26 };

            double dels[] = { 1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0,
                16.0,              9.0, -1.0, 1.0,
                15.0, 10.0,  5.0, 11.0, -1.0, 1.0,
                28.0, 14.0,       22.0, -1.0, 1.0,
                23.0, 15.0,  7.0, 17.0, -1.0, 1.0,
                81.0, 57.0, 29.0, 55.0, -1.0, 1.0 };

            /* Objective */
            /* Objective */
            double dobj[]={ 18.0, 21.0, 18.0, 16.0, 10.0, 15.0, 16.0, 14.0, 9.0,
                10.0,  9.0,  6.0, 17.0, 16.0, 17.0, 15.0, 10.0, 0.0,
                13.0,  0.0, 13.0,  0.0,  7.0,  0.0,  7.0,  0.0, 1.0 };

            /* Column bounds */
            double dclo[]={ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
                0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
                0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0 };



            double dcup[]={ INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,
                INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,
                INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF };

            /* Row bounds */
            double drlo[]={ -INF, -INF, -INF, -INF,  0.0, 4.0, 0.0, 8.0, 10.0 };
            double drup[]={ 10.0, 19.0, 25.0, 15.0,  0.0, 7.0, 0.0, 8.0, 90.0 };

            /* Stages */
            int nstg=2;
            int n_first_stg_rows=4;
            int rstg[]={ 0,0,0,0,1,1,1,1,1 };
            int cstg[]={ 0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1 };

            /* Stochastic data */
            int nindp=5;
            int nsamp[]={ 5, 2, 5, 5, 3 };
            double demand[]={ 200, 220, 250, 270, 300,
                50, 150,
                140, 160, 180, 200, 220,
                10, 50, 80, 100, 340,
                580, 600, 620 };
            double dprobs[]={ 0.2, 0.05, 0.35, 0.2, 0.2,
                0.3, 0.7,
                0.1, 0.2, 0.4, 0.2, 0.1,
                0.2, 0.2, 0.3, 0.2, 0.1,
                0.1, 0.8, 0.1 };

            /* local variables */
            int ii,jj;

            // initialize SmiModel
            SmiScnModel *smiModel = new SmiScnModel();
            smiModel->setOsiSolverHandle(*osiClp1);


            // set core model using Osi interface
            OsiClpSolverInterface ocsi;
            ocsi.loadProblem(CoinPackedMatrix( 1,mrow,mcol,dels,nels),dclo,dcup,dobj,drlo,drup);

            // core model 
            SmiCoreData *smiCore = new SmiCoreData(&ocsi,nstg,cstg,rstg);
            smiModel->setCore(smiCore);

            cout << "ModelDiscrete: generated Core data" << endl;

            // Create discrete distribution
            SmiDiscreteDistribution *smiDD = new SmiDiscreteDistribution(smiCore);

            cout << "ModelDiscrete: adding random variables" << endl;

            int index=0;
            for (jj=0;jj<nindp;jj++)
            {
                SmiDiscreteRV *smiRV = new SmiDiscreteRV(1);
                for (ii=0;ii<nsamp[jj];ii++)
                {			
                    CoinPackedVector empty_vec;
                    CoinPackedMatrix empty_mat;
                    CoinPackedVector cpv_rlo ;
                    CoinPackedVector cpv_rup ;
                    cpv_rlo.insert(n_first_stg_rows + jj, demand[index+ii]);
                    cpv_rup.insert(n_first_stg_rows + jj, demand[index+ii]);
                    smiRV->addEvent(empty_mat,empty_vec,empty_vec,empty_vec,cpv_rlo,cpv_rup,dprobs[index+ii]);
                    cpv_rlo.clear();
                    cpv_rup.clear();
                }
                smiDD->addDiscreteRV(smiRV);
                index+=nsamp[jj];
            }

            EXPECT_EQ(nindp, smiDD->getNumRV());
            cout << "ModelDiscrete: added " << nindp << " random variables" << endl;



            cout << "ModelDiscrete: processing into scenarios" << endl;

            smiModel->processDiscreteDistributionIntoScenarios(smiDD);

            // load problem data into OsiSolver
            smiModel->loadOsiSolverData();
            // get Osi pointer
            OsiSolverInterface *smiOsi = smiModel->getOsiSolverInterface();
            // set some parameters
            smiOsi->setHintParam(OsiDoPresolveInInitial,true);
            smiOsi->setHintParam(OsiDoScale,true);
            smiOsi->setHintParam(OsiDoCrash,true);
            // solve using Osi Solver
            smiOsi->initialSolve();
            // test optimal value
            EXPECT_LT(fabs(smiOsi->getObjValue()-1566.042), 0.01);

            // test solutions
            const double *dsoln = smiOsi->getColSolution();
            double objSum = 0.0;

            /* The canonical way to traverse the tree:
            For each scenario, get the leaf node.
            Then get the parent.  Repeat until parent is NULL.
            (Only the root node has a NULL parent.)
            */

            for(int is=0; is<smiModel->getNumScenarios(); ++is)
            {
                /* this loop calculates the scenario objective value */
                double scenSum = 0.0;

                // start with leaf node
                SmiScnNode *node = smiModel->getLeafNode(is);

                // leaf node probability is the scenario probability
                double scenprob = node->getModelProb();

                while (node != NULL)
                {
                    // getColStart returns the starting index of node in OSI model
                    for(int j=node->getColStart(); j<node->getColStart()+node->getNumCols(); ++j)
                    {
                        // getCoreColIndex returns the corresponding Core index
                        // in the original (user's) ordering
                        scenSum += dobj[node->getCoreColIndex(j)]*dsoln[j];	


                    }			

                    // get parent of node
                    node = node->getParent();
                }
                objSum += scenSum*scenprob;
            }

            EXPECT_LT(fabs(smiOsi->getObjValue()-objSum), 0.01);

            // print results
            printf("Solved stochastic program %s\n", name);
            printf("Number of rows: %d\n",smiOsi->getNumRows());
            printf("Number of cols: %d\n",smiOsi->getNumCols());
            printf("Optimal value: %g\n",smiOsi->getObjValue());

            // Clean up test
            delete smiModel;
            delete osiClp1;
            delete smiDD;


        }

        // Display message on stdout and stderr
        void testingMessage( const char * const msg )
        {
            //  std::cerr <<msg;
            cout <<endl <<"*****************************************"
                <<endl <<msg <<endl;
        }

        TEST_F(FlopCppExamples, exampleSmi){
            //testingMessage( "Model generation using SMPS files for Cambridge-Watson problems.\n" );
            //SmpsIO("../../../../Data/Stochastic/wat_10_C_32");		

            testingMessage( "Model generation using scenario tree construction methods.\n");
            ModelScenario("Dantzig-Ferguson Aircraft Allocation using Scenarios");

            testingMessage( "Model generation using discrete distribution specification methods.\n" );
            ModelDiscrete("Dantzig-Ferguson Aircraft Allocation using Discrete Distribution");


            testingMessage( "*** Done! *** \n");
        }
    } // end namespace exampleSmi

    TEST_F(FlopCppExamples, DISABLED_gapmin){ // heap error at the end...
        /* This is a FlopCpp version of the model gapmin.gms from 
        the GAMS model library.

        A general assignment problem is solved via Lagrangian Relaxation
        by dualizing the multiple choice constraints and solving
        the remaining knapsack subproblems.
        The data for this problem are taken from Martello.
        The optimal value is 223 and the optimal solution is:
        1 1 4 2 3 5 1 4 3 5, where
        in columns 1 and 2, the variable in the first row is equal to 1,
        in column 3, the variable in the fourth row is equal to 1, etc...

        Martello, S, and Toth, P, Knapsack Problems: Algorithms and Computer 
        Implementations. John Wiley and Sons, Chichester, 1990.
        Guignard, M, and Rosenwein, M, An Improved Dual-Based Algorthm for
        the Generalized Assignment Problem. Operations Research 37 (1989), 658-663. 
        */
        const int numResources = 5; const int numItems = 10;
        MP_set  i(numResources), j(numItems);

        double atable[numResources][numItems] =
        { {12,   8,  25,  17,  19,  22,   6,  22,  20,  25},
        { 5,  15,  15,  14,   7,  11,  14,  16,  17,  15},
        {21,  24,  13,  24,  12,  16,  23,  20,  15,   5},
        {23,  17,  10,   6,  24,  20,  15,  10,  19,   9},
        {17,  20,  15,  16,   5,  13,   7,  16,   8,   5} };

        double ftable[numResources][numItems] =
        { {16,  26,  30,  47,  18,  19,  33,  37,  42,  31},
        {38,  42,  15,  21,  26,  11,  11,  50,  24,  19},
        {48,  17,  14,  22,  14,  18,  47,  32,  17,  42},
        {22,  32,  28,  39,  37,  23,  25,  12,  44,  17},
        {31,  42,  31,  40,  16,  15,  29,  31,  44,  41} };

        MP_data b(i), w(j), f(i,j), zfeas, s(j), solution(i,j);
        b(0) = 28;  b(1) = 20;  b(2) = 27;  b(3) = 24;  b(4) = 19;

        double tolerance = 1e-5;
        int maxiter = 100;
        double zlbest = 0.0;
        f.value(&ftable[0][0]);

        zfeas() = sum(j, maximum(i, f(i,j)));
        zfeas.display("zfeas");

        double alpha = 1.0;
        int count = 1;
        int reset = 5;

        w(j) = 0;

        MP_data myf(j);

        for (int iter=0; iter<100; iter++) { 
            double zlr = 0.0;
            s(j) = 1.0;
            for (int k=0; k<numResources; k++) {
                for (int jc=0; jc<j.size(); jc++) {
                    myf(jc) = ftable[k][jc] - w(jc);
                }

                MP_model testModel(new OsiCbcSolverInterface);

                MP_set j(numItems);
                MP_data a(atable[k],j);
                MP_binary_variable x;

                MP_constraint knapsack;
                knapsack() = sum(j, a(j)*x(j)) <= b(k); 

                testModel.minimize( sum(j, f(j)*x(j)) );

                zlr += testModel->getObjValue();
                for (int jc=0; jc<j.size(); jc++) {
                    s(jc) = s(jc) - x.level(jc);
                }

            }

            MP_data zl;
            zl() = zlr + sum(j, w(j));

            bool improve = (zl>zlbest);
            zlbest = std::max<double>(zlbest,zl(0));

            double norm = 0.0;
            for (int jc=0; jc<j.size(); jc++) {
                norm += s(jc)*s(jc);
            }

            cout<<"Iter Nbr."<<iter+1<<"   "<<zlr<<"   "<<zl<<"   "<<zlbest<<"   "<<norm<<endl;	
            if (norm<tolerance) break;

            double target = (zlbest+zfeas)/2.0;
            double step = (alpha*(target-zl)/norm);

            w(j) = w(j)+step*s(j);

            if (count>reset) {
                alpha = alpha/2.0;
                count = 1;
            } else if (improve == true) {
                count = 1;
            } else {
                count++;
            }
        }

        EXPECT_NEAR(223, zlbest, 0.00001);

        //vector<Knapsack*> pknap;

        //for (int k=0; k<numResources; k++) {
        //    for (int jc=0; jc<j.size(); jc++) {
        //        myf(jc) = ftable[k][jc] - w(jc);
        //    }

        //    pknap.push_back(new Knapsack(numItems, atable[k], myf, b(k)));
        //}


        //for (int q=0; q<numItems; q++) {
        //    for (int k=0; k<numResources; k++) {
        //        if ( pknap[k]->x.level(q) == 1) cout<<k+1<<"  ";
        //    }
        //}
        cout<<endl;
        cout<<"Test gapmin passed."<<endl;
    }

    TEST_F(FlopCppExamples, magic){
        // M A G I C   Power Scheduling Problem 
        /*   A number of power stations are committed to meet demand for a particular
        day. three types of generators having different operating characteristics
        are available.  Generating units can be shut down or operate between
        minimum and maximum output levels.  Units can be started up or closed down
        in every demand block.

        Reference: R E Day and H P Williams, MAGIC: The design and use of an
        interactive modeling language for mathematical programming,
        Department Business Studies, University of Edinburgh, Edinburgh,
        UK, February 1982.

        H P Williams, Model Building in Mathematical Programming, Wiley,
        New York, 1978.
        */

        MP_model testModel(new OsiCbcSolverInterface);
        testModel.verbose();

        enum {t12pm_6am, t6am_9am, t9am_3pm, t3pm_6pm, t6pm_12pm, numT};
        enum {type_1, type_2, type_3, numG};

        MP_set T(numT); T.cyclic();
        MP_set G(numG);

        MP_data dem(T); // demand (1000mw)   
        dem(t12pm_6am) = 15; 
        dem(t6am_9am) = 30;
        dem(t9am_3pm) = 25; 
        dem(t3pm_6pm) = 40;
        dem(t6pm_12pm) = 27;

        MP_data dur(T);  // duration (hours)  
        dur(t12pm_6am) = 6; 
        dur(t6am_9am) = 3;
        dur(t9am_3pm) = 6; 
        dur(t3pm_6pm) = 3;
        dur(t6pm_12pm) = 6;

        MP_data 
            min_pow(G), 
            max_pow(G), 
            cost_min(G), 
            cost_inc(G), 
            start(G), 
            number(G);

        min_pow(type_1) = 0.85; max_pow(type_1) = 2.0;     
        min_pow(type_2) = 1.25; max_pow(type_2) = 1.75;    
        min_pow(type_3) = 1.5;  max_pow(type_3) = 4.0;     

        cost_min(type_1) = 1000; cost_inc(type_1) = 2.0;     
        cost_min(type_2) = 2600; cost_inc(type_2) = 1.3;   
        cost_min(type_3) = 3000; cost_inc(type_3) = 3.0;     

        start(type_1) = 2000; number(type_1) = 12;     
        start(type_2) = 1000; number(type_2) = 10;     
        start(type_3) = 500;  number(type_3) =  5;     

        MP_data  
            peak,     // peak power (1000 mw)
            ener(T),  // energy demand in load block (1000mwh)
            tener,    // total energy demanded (1000mwh)
            lf;       // load factor 

        MP_index i,j;

        peak() = maximum(T(i), dem(i));

        ener(i) = dur(i)*dem(i);  

        tener() = sum(T(i), ener(i));  

        lf() = tener()/(peak() * 24);

        peak.display();
        tener.display();
        lf.display();
        ener.display("ener");

        MP_variable
            x(G,T),  // generator output (1000mw)
            n(G,T),  // number of generators in use
            s(G,T);  // number of generators started up

        n.integer();
        n.upperLimit(j,i) = number(j);
        //n.upperLimit.display("n upper");

        MP_constraint
            pow(T),    // demand for power (1000mw)
            res(T),    // spinning reserve requirements (1000mw)
            st(G,T),   // start_up definition
            minu(G,T), // minimum generation level (1000mw)
            maxu(G,T); // maximum generation level (1000mw)

        pow(i) =  sum(G(j), x(j,i)) >= dem(i);

        res(i) =  sum(G(j), max_pow(j)*n(j,i)) >= 1.15*dem(i);

        st(j,i) = s(j,i) >= n(j,i) - n(j,i-1);

        minu(j,i) =  x(j,i) >= min_pow(j)*n(j,i);

        maxu(j,i) =  x(j,i) <= max_pow(j)*n(j,i);

        MP_expression  cost;    // total operating cost (l)
        cost = sum(G(j)*T(i), 
            dur(i)* cost_min(j)*n(j,i) + start(j)*s(j,i) +
            1000*dur(i)*cost_inc(j)*(x(j,i)-min_pow(j)*n(j,i)) );

        testModel.minimize(cost);

        EXPECT_EQ(55, testModel->getNumRows());
        EXPECT_EQ(45, testModel->getNumCols());
        EXPECT_EQ(135, testModel->getNumElements());
        EXPECT_NEAR(988540, testModel->getObjValue(), 1);


        x.display("x");
        n.display("n");
        s.display("s");
        const int TSize=T.size();
        double **rep = new double *[TSize];
        for(int cnt=0;cnt<TSize;cnt++)
        {
            rep[cnt]=new double[4];
        }
        for (unsigned i=0; i<T.size(); i++) {
            rep[i][0] = dem(i);

            rep[i][1] = 0;
            for (unsigned j=0; j<G.size(); j++) { 
                rep[i][1] +=  max_pow(j) * n.level(j,i);
            }

            rep[i][2] = 0;
            for (unsigned j=0; j<G.size(); j++) { 
                rep[i][2] +=  s.level(j,i);
            }

            rep[i][3] = -pow.price(i)/dur(i)/1000;
        }

        for (unsigned i=0; i<T.size(); i++) {
            cout<<i<<"  "<<rep[i][0]<<"  "<<rep[i][1]<<"  "<<rep[i][2]<<"  "<<rep[i][3]<<endl;
        }

        // Cleanup
        for (int i = 0; i < TSize; i++)
            delete [] rep[i];
        delete [] rep;

        cout<<"Test magic passed."<<endl;

    }

    TEST_F(FlopCppExamples, mine){
        for (int ij = 0; ij < 11; ij++) {

            OsiClpSolverInterface clpSolver;
            
            MP_model thisModel(new OsiCbcSolverInterface(&clpSolver));
            enum {nw,ne,se,sw,numK};

            MP_set l(4);  // identfiers for level row and column labels  / 1*4 /;

            MP_index i,j;

            MP_data conc(l,l,l);

            conc(0,0,0) =  1.5;
            conc(0,0,1) =  1.5;
            conc(0,0,2) =  1.5;
            conc(0,0,3) =  0.75;

            conc(0,1,0) =  1.5;
            conc(0,1,1) =  2.0;
            conc(0,1,2) =  1.5;
            conc(0,1,3) =  0.75;

            conc(0,2,0) =  1.0;
            conc(0,2,1) =  1.0;
            conc(0,2,2) =  0.75;
            conc(0,2,3) =  0.50;

            conc(0,3,0) =  0.75;
            conc(0,3,1) =  0.75;
            conc(0,3,2) =  0.5;
            conc(0,3,3) =  0.25;

            conc(1,0,0) =  4;
            conc(1,0,1) =  4;
            conc(1,0,2) =  2;

            conc(1,1,0) =  3;
            conc(1,1,1) =  3;
            conc(1,1,2) =  1;

            conc(1,2,0) =  2;
            conc(1,2,1) =  2;
            conc(1,2,2) =  0.5;

            conc(2,0,0) =  12;
            conc(2,0,1) =  6;

            conc(2,1,0) =  5;
            conc(2,1,1) =  4;

            conc(3,0,0) =  6;

            MP_set  k(numK); //  location of four neighbouring blocks / nw, "ne", se, sw /
            MP_subset<3> c(l,l,l); //  neighbouring blocks related to extraction feasibility
            MP_subset<3> d(l,l,l); //  complete set of block identifiers

            MP_data li(k), // lead for i  / (se,sw)  = 1 /
                lj(k),     // lead for j  / ("ne",se) = 1 /
                cost(l);   // block extraction cost  / 1=3000, 2=6000, 3=8000, 4=10000 /

            li(se) = 1;
            li(sw) = 1;
            lj(ne) = 1;
            lj(se) = 1;

            cost(0) = 3000;
            cost(1) = 6000;
            cost(2) = 8000;
            cost(3) =10000; 

            double value = 200000;

            forall(l*l(i)*l(j).such_that( l+i<l.size()-1 && l+j<l.size()-1 ),  
                c.insert(l,i,j)
                );

            c.display("c");

            forall(l*l(i)*l(j).such_that( l+i<l.size() && l+j<l.size() ),  
                d.insert(l,i,j)
                );

            d.display("d");

            thisModel.verbose();

            MP_variable x(l,l,l); //   extraction of blocks

            MP_constraint pr(k,c); //  precedence relationships

            pr(k,c(l,i,j)) = x(l,i+li(k),j+lj(k)) >= x(l+1,i,j);

            x.upperLimit(l,i,j) = 1;

            thisModel.maximize( sum(d(l,i,j), (conc(l,i,j)*value/100 - cost(l))*x(l,i,j)) );

            EXPECT_EQ(56, thisModel->getNumRows());
            EXPECT_EQ(64, thisModel->getNumCols());
            EXPECT_EQ(112, thisModel->getNumElements());
            EXPECT_EQ(17500, thisModel->getObjValue());


            x.display("x");
            cout<<"Test mine passed."<<endl;

        }
    }

    namespace multiProduct {
        double random(double min, double max) {
            return min + (max-min)*(rand()/(RAND_MAX+0.0));
        }

        TEST_F(FlopCppExamples, multiProduct){
            MP_model testModel(new OsiCbcSolverInterface);
            testModel.verbose();
            enum {Godiva,Neuhaus,Leonidas,nbrProducts};
            enum {Brussels,Amsterdam,Antwerpen,Paris,Milan,Cassis,Bonn,Madrid,Bergen,
                London,nbrCities};

            MP_set Cities(nbrCities);
            MP_set Products(nbrProducts);
            MP_subset<3> Routes(Products,Cities,Cities);

            Routes.insert(Godiva,Brussels,Paris);
            Routes.insert(Godiva,Brussels,Bonn);
            Routes.insert(Godiva,Amsterdam,London);
            Routes.insert(Godiva,Amsterdam,Milan);
            Routes.insert(Godiva,Antwerpen,Madrid);
            Routes.insert(Godiva,Antwerpen,Bergen);

            Routes.insert(Neuhaus,Brussels,Madrid);
            Routes.insert(Neuhaus,Brussels,Bergen);
            Routes.insert(Neuhaus,Amsterdam,Madrid);
            Routes.insert(Neuhaus,Amsterdam,Cassis);
            Routes.insert(Neuhaus,Antwerpen,Paris);
            Routes.insert(Neuhaus,Antwerpen,Bonn);

            Routes.insert(Leonidas,Brussels,Bonn);
            Routes.insert(Leonidas,Brussels,Milan);
            Routes.insert(Leonidas,Amsterdam,Paris);
            Routes.insert(Leonidas,Amsterdam,Cassis);
            Routes.insert(Leonidas,Antwerpen,London);
            Routes.insert(Leonidas,Antwerpen,Bergen);

            cout<<"Number of Routes = "<<Routes.size()<<endl;

            MP_subset<2> Suppliers(Products,Cities);
            MP_subset<2> Customers(Products,Cities);
            MP_subset<2> Connections(Cities,Cities);

            MP_index p,o,d;

            // Set "projections"
            Suppliers(p,o) <<= Routes(p, o, MP_index::Any());
            Customers(p,d) <<= Routes(p, MP_index::Any(), d);
            Connections(o,d) <<= Routes(MP_index::Any(), o, d);

            cout<<Suppliers.size()<<endl;
            cout<<Customers.size()<<endl;
            cout<<Connections.size()<<endl;

            Routes.display();
            Suppliers.display();
            Customers.display();
            Connections.display();

            MP_data supply(Suppliers);
            MP_data demand(Customers);

            MP_data cost(Routes);

            for (int i=0; i<Suppliers.size(); i++) {
                supply(i) = random(200,300);
            }
            for (int i=0; i<Customers.size(); i++) {
                demand(i) = random(50,100);
            }
            for (int i=0; i<Routes.size(); i++) {
                cost(i) = random(1,2);
            }
            double limit = 500;

            MP_variable trans(Routes);

            MP_constraint sup(Suppliers);
            MP_constraint dem(Customers);
            MP_constraint lim(Connections);

            sup(Suppliers(p,o)) =
                sum(Routes(p,o,d), trans(Routes(p,o,d))) <= supply(Suppliers(p,o));

            dem(Customers(p,d)) =
                sum(Routes(p,o,d), trans(Routes(p,o,d))) >= demand(Customers(p,d));

            lim(Connections(o,d)) =
                sum(Routes(p,o,d), trans(Routes(p,o,d))) <= limit;

            testModel.minimize( sum(Routes(p,o,d), cost(Routes(p,o,d)) * trans(Routes(p,o,d))) );

            EXPECT_EQ(41, testModel->getNumRows());
            EXPECT_EQ(18, testModel->getNumCols());
            EXPECT_EQ(54, testModel->getNumElements());

            trans.display("solution");

            //MultiProduct(testModel, Cities,Products,Routes,Suppliers,Customers, Connections,
            //    supply,demand,limit,cost);

            cout<<"Test multiproduct passed."<<endl;
        }
    }//end namesprace multiProduct

    TEST_F(FlopCppExamples, stochBenders){
        /* Benders decomposition for a simple transportation problem
        with stochastic demands. For details see: Erwin Kalvelagen:
        "Benders decomposition for stochastic programming with GAMS"
        (www.gams.com/~erwin/stochbenders.pdf)
        */
        const int numScenarios = 3;
        const int numFactories = 3;
        const int numDistributionCenters = 5;
        MP_set i(numFactories), j(numDistributionCenters);

        MP_data 
            capacity(i),
            transcost(i,j);

        transcost(0,0) = 2.49; transcost(1,0) = 1.46; transcost(2,0) = 3.26; 
        transcost(0,1) = 5.21; transcost(1,1) = 2.54; transcost(2,1) = 3.08; 
        transcost(0,2) = 3.76; transcost(1,2) = 1.84; transcost(2,2) = 2.60; 
        transcost(0,3) = 4.85; transcost(1,3) = 1.86; transcost(2,3) = 3.76; 
        transcost(0,4) = 2.07; transcost(1,4) = 4.76; transcost(2,4) = 4.45; 

        capacity(0) = 500;
        capacity(1) = 450;
        capacity(2) = 650;

        const double prodcost = 14;
        const double price = 24;
        const double wastecost = 4;

        const double Demand[numScenarios][numDistributionCenters] = 
        {{150,100,250,300,600},
        {160,120,270,325,700},
        {170,135,300,350,800}};

        const double prob[numScenarios] = {0.25,0.5,0.25};

        ///// Master problem /////////////////////////////////////////////

        MP_model masterproblem(new OsiClpSolverInterface);

        MP_variable 
            theta,
            ship(i,j),
            product(i),
            slackproduct(i),
            received(j);

        MP_constraint
            production(i),
            receive(j),
            prodcap(i);

        production(i) = product(i) == sum(j,ship(i,j));
        receive(j) =    received(j) == sum(i,ship(i,j));
        prodcap(i) =    product(i) + slackproduct(i) == capacity(i);

        masterproblem.minimize( sum(i*j,transcost(i,j)*ship(i,j)) +
            sum(i, prodcost*product(i)) + theta() );

        ///// Sub problem ///////////////////////////////////////////////

        MP_model subproblem(new OsiClpSolverInterface);

        MP_variable
            sales(j),
            waste(j),
            slacksales(j);

        MP_constraint
            selling(j),
            selmax(j);

        selling(j) = sales(j) + waste(j) == 0.0;
        selmax(j) =  sales(j) + slacksales(j) == 0.0;

        subproblem.minimize( sum(j, wastecost*waste(j)) - sum(j, price*sales(j)) );

        ///// Benders algorithm //////////////////////////////////////////

        masterproblem->setColLower( theta().getColumn(), -masterproblem->getInfinity());

        double lowerbound = -masterproblem->getInfinity();
        double upperbound = masterproblem->getInfinity();
        double objmaster = 0.0;

        int kk;
        for (kk=0; kk<25; kk++) { // max 25 iterations
            double cutconst = 0.0;
            MP_data cutcoeff(j);

            double objsub = 0.0;
            received.display("received");
            for (int jj=0; jj<numDistributionCenters; jj++) {
                subproblem->setRowBounds( selling(jj), received.level(jj), received.level(jj));
            }
            for (int s=0; s<numScenarios; s++) {
                for (int j=0; j<numDistributionCenters; j++) {
                    subproblem->setRowBounds( selmax(j), Demand[s][j], Demand[s][j]);
                }
                subproblem->resolve();
                objsub += prob[s]*subproblem->getObjValue();
                for (int j=0; j<numDistributionCenters; j++) {
                    cout<<selmax.price(j)<<"  "<<prob[s]<<"  "<<Demand[s][j]<<endl;
                    cutconst += prob[s]*selmax.price(j)*Demand[s][j];
                    cutcoeff(j) += prob[s]*selling.price(j);
                }
            }

            upperbound = min(upperbound, objmaster + objsub);
            cout<<"Lower and upper bounds: "<<lowerbound<<"   "<<upperbound<<endl;
            if ((upperbound-lowerbound) < 0.0001*(1+fabs(lowerbound))) {
                break;
            }

            cout<<"cutconst="<<cutconst<<endl;
            masterproblem.addRow( theta() >= cutconst + sum(j, cutcoeff(j)*received(j)) );

            masterproblem->resolve();
            ship.display("ship");

            lowerbound = masterproblem->getObjValue();
            objmaster = lowerbound - theta.level();
        }
        EXPECT_LT(kk, 25);

        // Expected solution:
        // ship
        // 0 0   0
        // 0 1   0
        // 0 2   0
        // 0 3   0
        // 0 4   500
        // 1 0   150
        // 1 1   0
        // 1 2   0
        // 1 3   300
        // 1 4   0
        // 2 0   0
        // 2 1   100
        // 2 2   270
        // 2 3   0
        // 2 4   100

        cout<<"Test stochbenders passed."<<endl;
    }

    TEST_F(FlopCppExamples, sudoku){
        MP_model testModel(new OsiCbcSolverInterface);

        const int p = 3;

        MP_set J(p*p);
        MP_set K(p);
        MP_subset<3> F(J,J,J);
#ifdef _MSC_VER
	        std::string fname="../../../test/testFiles/fixed.dat";
#else
            std::string fname="testFiles/fixed.dat";
#endif
        std::ifstream file(fname.c_str());

        int ival,jval,kval;

        while (file>>ival>>jval>>kval) {
            F.insert(ival-1,jval-1,kval-1);
        }
        file.close();

        MP_binary_variable	x(J,J,J);

        MP_constraint 
            rows(J,J),
            cols(J,J),
            nums(J,J),
            squares(K,K,J),
            preselected(F);

        MP_index i,j,k,m,n;

        rows(i,j) = sum(J(k), x(i,j,k)) == 1;
        cols(j,k) = sum(J(i), x(i,j,k)) == 1;
        nums(i,k) = sum(J(j), x(i,j,k)) == 1;

        squares(m,n,k) = sum(K(i)*K(j), x(m*p+Constant(i),n*p+Constant(j),k)) == 1;

        preselected(F(i,j,k)) = x(i,j,k) == 1;

        //    testModel.minimize(sum(J(i)*J(j)*J(k),x(i,j,k)));

        testModel.minimize(x(0,0,0));

        for (int i=0; i<9; i++) {
            for (int j=0; j<9; j++) {
                for (int k=0; k<9; k++) {
                    if (x.level(i,j,k)==1) {
                        cout<<k+1<<" ";
                    }
                }
            }
            cout<<endl;
        }

        cout<<"Test sodoku passed."<<endl;
    }

    namespace tap {
        class Tap : public MP_model {
        public:
            MP_set S,D;
            MP_data s;
            MP_data d;
            MP_variable x;
            MP_variable y;  
            MP_constraint supply, demand, def_y;

            Tap(int numS, int numD) : MP_model(new OsiCbcSolverInterface),
                S(numS), D(numD),
                s(S), d(D),
                x(S,D), y(S,D),
                supply(S), demand(D), def_y(S,D) {
                    y.binary();

                    supply(S) = sum(D, x(S,D)) <= s(S);

                    demand(D) = sum(S, x(S,D)) >= d(D) ; 

                    def_y(S,D) = d(D)*y(S,D) >= x(S,D);

                    add(supply); add(demand); add(def_y);

                    setObjective( sum(S*D, y(S,D)) );

            }    
        };

        TEST_F(FlopCppExamples, DISABLED_tap){ // heap error in branch&bound for the second instance
            double stab_1[6]  = {5,5,6,6,6,6};
            double dtab_1[4]  = {4, 7, 10, 11};
            double stab_2[7] =  {9,9,10,12,12,12,16};
            double dtab_2[5] =  {6, 14, 17, 21, 22};

            Tap first_instance(6,4);
            Tap second_instance(7,5);

            first_instance.s.value(stab_1);
            first_instance.d.value(dtab_1);

            second_instance.s.value(stab_2);
            second_instance.d.value(dtab_2);

            first_instance.minimize();

            EXPECT_EQ(34, first_instance->getNumRows());
            EXPECT_EQ(48, first_instance->getNumCols());
            EXPECT_EQ(96, first_instance->getNumElements());
            EXPECT_EQ(7, first_instance->getObjValue());


            first_instance.x.display("Solution of first instance (x)");
            first_instance.y.display("Solution of first instance (y)");

            second_instance.minimize();

            EXPECT_EQ(47, second_instance->getNumRows());
            EXPECT_EQ(70, second_instance->getNumCols());
            EXPECT_EQ(140, second_instance->getNumElements());
            EXPECT_EQ(9, second_instance->getObjValue());

            second_instance.x.display("Solution of second instance");
            cout<<"Test tap passed."<<endl;
        }
    }//end namespace tap

    namespace train {
        int readFile(string fname, map<string, vector<double> >& d) {
            std::ifstream file(fname.c_str());
            string cline;

            while(getline(file,cline)) {
                if (cline.compare(0,5,"begin")==0) {
                    string field = cline.substr(6,cline.find_first_of(')')-6);
                    double val;
                    while (file>>val) {
                        d[field].push_back(val);
                    }
                    file.clear();
                }
            }
            return 0;
        }

        void scheduleInsert(MP_subset<4>& sched, vector<double>& v, int c1, int c2) {
            for (int i = 0; i != v.size(); i++) {
                sched.insert(c1,c2,int(v[i])-1,int(v[i+1])-1);
                i++;
            }
        }

        void demandInsert(MP_data& demand, vector<double>& v, int c1, int c2, MP_subset<4>& sched) {
            for (int i = 0; i != v.size(); i++) {
                demand(sched(c1,c2,int(v[i])-1,int(v[i+1])-1)) = v[i+2];
                i++; i++;
            }
        }

        TEST_F(FlopCppExamples, train){
            MP_model testModel(new OsiCbcSolverInterface);
            testModel.verbose();
            const int numTimeIntervals=48;
            enum {BO, NY, PH, WA, numCities};

            MP_set cities(numCities);           // Set of cities,
            MP_subset<2> links(cities, cities); // Set of intercity links
            MP_set times(numTimeIntervals);     // Set of time intervals in a day
            times.cyclic();

            links.insert(BO,NY);
            links.insert(NY,PH);
            links.insert(PH,WA);
            links.insert(NY,BO);
            links.insert(PH,NY);
            links.insert(WA,PH);

            links.display("LINKS");

            MP_subset<4> schedule(cities, cities, times, times); 
            // Member ((c1,c2),t1,t2) of this set represents a train that leaves 
            // city c1 at time t1 and arrives in city c2 at time t2

            map<string, vector<double> > d;
#ifdef _MSC_VER
	        std::string dataDir="../../../test/testFiles/train.dat";
#else
            std::string dataDir="testFiles/train.dat";
#endif
            readFile(dataDir,d);
            scheduleInsert(schedule,d["WAPH"],WA,PH);
            scheduleInsert(schedule,d["PHNY"],PH,NY);
            scheduleInsert(schedule,d["NYBO"],NY,BO);
            scheduleInsert(schedule,d["BONY"],BO,NY);
            scheduleInsert(schedule,d["NYPH"],NY,PH);
            scheduleInsert(schedule,d["PHWA"],PH,WA);

            schedule.display("SCHEDULE");

            MP_data demand(schedule); 
            // Smallest number of cars that can meet demand for each scheduled train    
            demandInsert(demand,d["demWAPH"],WA,PH,schedule);
            demandInsert(demand,d["demPHNY"],PH,NY,schedule);
            demandInsert(demand,d["demNYBO"],NY,BO,schedule);
            demandInsert(demand,d["demBONY"],BO,NY,schedule);
            demandInsert(demand,d["demNYPH"],NY,PH,schedule);
            demandInsert(demand,d["demPHWA"],PH,WA,schedule);

            demand.display("DEMAND");

            // Maximum number of cars in one section of a train
            double section = 14;

            MP_data low(schedule);  // Minimum number of cars needed to meet demand
            MP_data high(schedule); // Maximum number of cars allowed on a train:
            // 2 if demand is for less than one car; otherwise, lesser of
            // number of cars needed to hold twice the demand, and
            // number of cars in minimum number of sections needed

            MP_data dist_table(links);
            MP_data distance(links);

            dist_table(links(BO,NY)) = 232;
            dist_table(links(NY,PH)) =  90;
            dist_table(links(PH,WA)) = 135;

            MP_index c1,c2,t1,t2;

            low(schedule) = ceil(demand(schedule));
            high(schedule) = maximum (2, minimum (ceil(2*demand(schedule)), 
                section*ceil(demand(schedule)/section) ));
            low.display("LOW");
            high.display("HIGH");

            distance(links).such_that(dist_table(links)>0) = dist_table(links);
            distance(links(c1,c2)).such_that(dist_table(links(c1,c2))==0.0) = dist_table(links(c2,c1));

            distance.display("distance");

            MP_variable U(cities,times); 
            // u[c,t] is the number of unused cars stored at city c in the interval beginning at time t

            MP_variable X(schedule);    // x[c1,t1,c2,t2] is the number of cars assigned to
            // the scheduled train that leaves c1 at t1 and arrives in c2 at t2

            MP_index c,t;

            MP_expression cars =  
                sum( cities(c), U(c,times.size()-1)) + 
                sum( schedule(c1,c2,t1,t2).such_that(t2 < t1), X(schedule) );

            // Number of cars in the system:
            // sum of unused cars and cars in trains during the last time interval of the day

            MP_expression miles = sum(schedule(c1,c2,t1,t2), distance(links(c1,c2)) * X(schedule));
            // Total car-miles run by all scheduled trains in a day

            MP_constraint account(cities,times);

            account(c,t) =  
                U(c,t) == U(c,t-1) + 
                sum( schedule(c1,c,t1,t), X(schedule(c1,c,t1,t)) ) - 
                sum( schedule(c,c2,t,t2), X(schedule(c,c2,t,t2)) );

            // For every city and time: unused cars in the present interval must equal
            // unused cars in the previous interval, plus cars just arriving in trains, 
            // minus cars just leaving in trains

            X.lowerLimit(schedule) = low(schedule);  // number of cars must meet demand,
            X.upperLimit(schedule) = high(schedule); // but must not be so great that unnecessary sections are run

            testModel.minimize(cars);

            // the following assumptions fail (same in seperate project file)
            EXPECT_EQ(192, testModel->getNumRows());
            EXPECT_EQ(411, testModel->getNumCols());
            EXPECT_EQ(822, testModel->getNumElements());
            EXPECT_EQ(1143, testModel->getObjValue());

            //cout<<cars.level()<<"  -----  "<<miles.level()<<endl;

            testModel.minimize(miles);

            EXPECT_EQ(192, testModel->getNumRows());
            EXPECT_EQ(411, testModel->getNumCols());
            EXPECT_EQ(822, testModel->getNumElements());
            EXPECT_EQ(163882, testModel->getObjValue());
            //cout<<cars.level()<<"  -----  "<<miles.level()<<endl;

            cout<<"Test train passed."<<endl;
        }
    }//end namesprace train

    TEST_F(FlopCppExamples, transport){
        MP_model testModel(new OsiClpSolverInterface);
        enum  {seattle, sandiego, numS}; 
        enum  {newyork, chicago, topeka,numD};

        MP_set S(numS);          // Sources 
        MP_set D(numD);          // Destinations 
        MP_subset<2> Link(S,D);  // Transportation links (sparse subset of S*D)

        Link.insert(seattle,newyork);
        Link.insert(seattle,chicago);
        Link.insert(sandiego,chicago);
        Link.insert(sandiego,topeka);

        MP_data SUPPLY(S);
        MP_data DEMAND(D);

        SUPPLY(seattle)=350;  SUPPLY(sandiego)=600;
        DEMAND(newyork)=325;  DEMAND(chicago)=300;  DEMAND(topeka)=275;

        MP_data COST(Link);

        COST(Link(seattle,newyork)) = 2.5;  
        COST(Link(seattle,chicago)) = 1.7;  
        COST(Link(sandiego,chicago))= 1.8;  
        COST(Link(sandiego,topeka)) = 1.4;

        COST(Link) = 90 * COST(Link) / 1000.0;

        MP_variable x(Link);
        x.display("...");

        MP_constraint supply(S);
        MP_constraint demand(D);  

        supply.display("...");

        supply(S) =  sum( Link(S,D), x(Link) ) <= SUPPLY(S);
        demand(D) =  sum( Link(S,D), x(Link) ) >= DEMAND(D);

        cout<<"Here"<<endl;

        testModel.minimize( sum(Link, COST(Link)*x(Link)) );
        EXPECT_EQ(5, testModel->getNumRows());
        EXPECT_EQ(4, testModel->getNumCols());
        EXPECT_EQ(8, testModel->getNumElements());
        EXPECT_NEAR(156.15, testModel->getObjValue(), 0.01);

        x.display("Optimal solution:");
        supply.display("Supply dual solution");
        cout<<"Test transport passed."<<endl;
    }

    TEST_F(FlopCppExamples, xbsl){
        const int numJ =  4;
        const int numT = 10;

        MP_set J(numJ),T(numT);
        MP_data D(J);

        D(0) = 3;
        D(1) = 4;
        D(2) = 2;
        D(3) = 2;

        MP_variable s(J);
        MP_binary_variable delta(J,T); 

        MP_index t,j;
        MP_model M(new OsiCbcSolverInterface);
        MP_constraint C2_20, C2_30, C3(J), C4(J); 

        C2_20() = s(2)  >= D(0) + s(0);
        C2_30() = s(3)  >= D(0) + s(0);

        C3(J) = sum( T(t).such_that(t <= numT-D(j)), (t+1)*delta(j,t) ) == s(j);

        C4(J) = sum( T(t).such_that(t <= numT-D(j)), delta(j,t) ) == 1;

        s.upperLimit(j) = numT-D(j);

        M.minimize_max( J, s(J)+D(J) ); 

        EXPECT_EQ(14, M->getNumRows());
        EXPECT_EQ(45, M->getNumCols());
        EXPECT_EQ(80, M->getNumElements());
        EXPECT_EQ(6, M->getObjValue());

        s.display("s");
        cout<<"Test xbsl passed."<<endl;
    }

    TEST_F(UseCaseTest, SmpsWriterTest) {
        MP_model testModel(new OsiClpSolverInterface());

        enum {numT = 2};
        MP_stage T(numT);

        enum {numScen = 3};
        MP_scenario_set scen(numScen);

        double probabilities[numScen] = { 0.1234567890123456, 0.876543211 };
        MP_data prob(&probabilities[0], scen);
        testModel.setProbabilities(prob);

        double stochData[numT-1][numScen] = { {123.4567890123456, -0.1234567890123456, 0.1234567890123456 } };
        MP_random_data d(&stochData[0][0], T);

        MP_variable x1(T), x2(T), x3(T), x4(T), x5(T), x6(T);
        x1.setName("abcdefghij");
        x2.setName("abcdefghij");
        x3.setName("abcdefghij");

        MP_constraint constr(T);
        constr(T) = x1(T) + x2(T) + x3(T) + x4(T) + x5(T) + x6(T) >= d(T);

        testModel.setObjective(sum(T, x1(T)));

        testModel.Solver->setIntParam(OsiNameDiscipline, 1);
        
        testModel.attach();

        //testModel.Solver->writeLp("columnNamesTest");

        testModel.solve(MP_model::MINIMIZE);
        
        //testModel.getSmi()->writeSmps("smpsFree", false, false);
        //testModel.getSmi()->writeSmps("smpsStrict", false, true);

        writeReadSMPSTest(testModel,MP_model::MINIMIZE,"test", false);
        writeReadSMPSTest(testModel,MP_model::MINIMIZE,"test", true);
        
        cout << "Finished";
    }

    TEST_F(MP_modelTest, SetOperatorTest) {
        MP_set A(5);
        MP_subset<1> B(A);
        MP_subset<1> C(A);

        B.insert(0); B.insert(1); B.insert(4);

        MP_data val(A);

        val.display("initialized");

        val(A).such_that(B(A)) = 1;

        val.display("val(A).such_that(B(A)) = 1");

        val(A).such_that(!B(A)) = 2;

        val.display("val(A).such_that(!B(A)) = 2");

        forall(A.such_that(!B(A)), C.insert(A));
        C.display("C = A\B");

        cout << "Finished.";
    }

    namespace unitTestSmi {

        void testingMessage( const char * const msg )
        {
	        DLOG(INFO) <<endl <<"*****************************************"
		        <<endl <<msg <<endl;
        }

        void myAssert(const char * file, const int line, bool c )
        {
            EXPECT_EQ(true, c);
	        if(!c)
	        {
		        char l[10];
		        sprintf(l,"%d",line);
		        string e("[Smi] Error in MyTest");
		        e+=", line ";
		        e+=l;
		        DLOG(ERROR) << e << endl;
	        }
        }

        TEST_F(SmiUnitTest, DecompUnitTest) {
	        OsiClpSolverInterface osi;
	        double INF=osi.getInfinity();

	        /* Problem Statement:
	         *
	         *    minimize x01 + x02 + x03 + 1/2 E[ x04 + x05 + x06 ]
	         *
	         *    subject to:
	         *      xN =>  0
	         *      0  <=  x01 + x02 + x03
	         *      C1 <=  x01 + x02 +              x04 + x05
	         *      C2 <=      + x02 + x03                x05 + x06
	         *      C3 <=  x01       + x03          x04 +     + x06
	         *
	         *    (C1, C2, C3) = ( 1, 1, 0) wp 1/2
	         *    (C1, C2, C3) = ( 0, 1, 0) wp 1/2
	         */


	        int nCoreRows=4;
	        double dCoreRup[] = { INF, INF, INF, INF };

	        int nCoreCols=6;
	        double dCoreClo[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	        double dCoreCup[] = {INF, INF, INF, INF, INF, INF};
	        double dCoreObj[] = {1.0, 1.0, 1.0, 0.5, 0.5, 0.5};

	        int iCoreColStarts[] = {0,
		        3,
		        6,
		        9,
		        11,
		        13,
		        15};
	        int iCoreRowIndice[] = {3,1,0,
		        2,1,0,
		        3,2,0,
		        3,1,
		        2,1,
		        3,2};
	        double dCoreMatEntries[] = {1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0,
		        1.0, 1.0,
		        1.0, 1.0,
		        1.0, 1.0 };
	        double dCoreRlo[] = { 0.0, 1.0, 1.0, 1.0 };

	        int nCoreStages = 2;
	        int iColStages[] = {0,0,0,1,1,1};
	        int iRowStages[] = {0,1,1,1};

	        int iBranchStage[] = {1,1};

	        int iAncestorScn[] = {0,0};
	        double dProbScn[] = {0.5, 0.5};

	        double drlo0[] = {1.0, 1.0, 0.0 };
	        int indices[]  = {1, 2, 3};
	        CoinPackedVector *rlo0 = new CoinPackedVector(3, indices,drlo0);
	        double drlo1[] = {0.0, 1.0, 0.0 };
	        CoinPackedVector *rlo1 = new CoinPackedVector(3, indices,drlo1);

	        // generate Core model

	        osi.loadProblem(nCoreCols,
		        nCoreRows,iCoreColStarts,iCoreRowIndice,dCoreMatEntries,dCoreClo,
		        dCoreCup,dCoreObj,
		        dCoreRlo,dCoreRup);
	        SmiCoreData *osiCore = new
		        SmiCoreData(&osi,nCoreStages,iColStages,iRowStages);

	        // initialize SmiScnModel
	        SmiScnModel *smiModel = new SmiScnModel();
            smiModel->setCore(osiCore);

	        // Add Scenarios
	        int	is = smiModel->generateScenario(osiCore,NULL,NULL,NULL,NULL,
		        rlo0,NULL,iBranchStage[0],
		        iAncestorScn[0],dProbScn[0]);

	        is = smiModel->generateScenario(osiCore,NULL,NULL,NULL,NULL,
		        rlo1,NULL,iBranchStage[1],
		        iAncestorScn[1],dProbScn[1]);


	        /*
	         *  set up vector of submodels, one for each scenario
	         */
	        vector<SmiScnModel *> submodel;
	        for( int jj=0; jj<smiModel->getNumScenarios(); jj++)
	        {
		        submodel.push_back(new SmiScnModel());
		        submodel[jj]->setOsiSolverHandle(OsiClpSolverInterface() );
	        }


	        // add root node to submodel[0]
	        myAssert(__FILE__,__LINE__,1 == submodel[0]->addNodeToSubmodel(smiModel->getRootNode()) );

	        // add leaf node to submodel[0]
	        myAssert(__FILE__,__LINE__,1 == submodel[0]->addNodeToSubmodel(smiModel->getLeafNode(0)) );
	        submodel[is]->setModelProb(1.0);

	        vector<SmiScnNode *> vecSub0 = submodel[0]->getSmiTree()->wholeTree();
	        submodel[1]->setModelProb(1.0);

	        myAssert(__FILE__,__LINE__,    !vecSub0[0]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub0[0]->getNumCols() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub0[0]->getNumRows() == 1 );
	        myAssert(__FILE__,__LINE__,     vecSub0[0]->getProb() == 1.0 );

	        myAssert(__FILE__,__LINE__,    !vecSub0[1]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getNumCols() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getNumRows() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getProb() == 0.5 );

	        // add leaf node to submodel[1]
	        myAssert(__FILE__,__LINE__,2 == submodel[1]->addNodeToSubmodel(smiModel->getLeafNode(1)) );
	        submodel[1]->setModelProb(1.0);

	        vector<SmiScnNode *> vecSub1 = submodel[1]->getSmiTree()->wholeTree();

	        myAssert(__FILE__,__LINE__,     vecSub1[0]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub1[0]->getNumCols() == 3 );

	        myAssert(__FILE__,__LINE__,    !vecSub1[1]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getNumCols() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getNumRows() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getProb() == 0.5 );

	        /*
	         * set up vector of OSI models, one for each submodel
	         */
	        vector<OsiSolverInterface *> osiSubmodel;
	        osiSubmodel.reserve(submodel.size());

	        //myAssert(__FILE__,__LINE__,     osiSubmodel[0]=submodel[0]->loadOsiSolverData());
	        //myAssert(__FILE__,__LINE__,     osiSubmodel[1]=submodel[1]->loadOsiSolverData());
	        osiSubmodel.push_back(submodel[0]->loadOsiSolverData());
	        osiSubmodel.push_back(submodel[1]->loadOsiSolverData());


	        myAssert(__FILE__,__LINE__,    !vecSub0[0]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__, vecSub0[0]->getNumCols() == 3 );
	        myAssert(__FILE__,__LINE__, vecSub0[0]->getNumRows() == 1 );
	        myAssert(__FILE__,__LINE__, vecSub0[0]->getColStart() == 0 );
	        myAssert(__FILE__,__LINE__,     vecSub0[0]->getRowStart() == 0 );
	        myAssert(__FILE__,__LINE__,     vecSub0[0]->getProb() == 1.0 );

	        myAssert(__FILE__,__LINE__,    !vecSub0[1]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getNumCols() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getNumRows() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getColStart() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getRowStart() == 1 );
	        myAssert(__FILE__,__LINE__,     vecSub0[1]->getProb() == 0.5 );



	        myAssert(__FILE__,__LINE__,     vecSub1[0]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub1[0]->getNumCols() == 3 );

	        myAssert(__FILE__,__LINE__,    !vecSub1[1]->isVirtualNode() );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getNumCols() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getNumRows() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getColStart() == 3 );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getRowStart() == 0 );
	        myAssert(__FILE__,__LINE__,     vecSub1[1]->getProb() == 0.5 );

	         /* Submodels
	         *
	         * 1:   dobj = [1, 1, 1, 0.25, 0.25, 0.25]
	         *      drlo = [0, 1, 1, 0 ],     drup = [inf, inf, inf, inf]
	         *      dclo = [0,0,0,0,0,0],     dcup = [inf,inf,inf,inf,inf,inf]
	         *      matrix = [ 1 1 1 0 0 0
	         *                 1 1 0 1 1 0
	         *                 0 1 1 0 1 1
	         *                 1 0 1 1 0 1 ]
	         *      nrows = 4
	         *      ncols = 6
	         *      coffset = 0
	         *      roffset = 0
	         *
	         * 2:   dobj = [0, 0, 0, 0.25, 0.25, 0.25]
	         *      drlo = [0, 1, 0],           drup = [inf, inf, inf]
	         *      dclo = [inf,inf,inf,0,0,0], dcup = [inf, inf, inf, inf, inf, inf]
	         *      matrix = [ 1 1 0 1 1 0
	         *                 0 1 1 0 1 1
	         *                 1 0 1 1 0 1 ]
	         *      nrows = 3
	         *      ncols = 6
	         *      coffset = 3
	         *      roffset = 0
	         */

	        myAssert(__FILE__,__LINE__,osiSubmodel[0]->getNumCols()==6);
	        myAssert(__FILE__,__LINE__,osiSubmodel[0]->getNumRows()==4);

	        myAssert(__FILE__,__LINE__,osiSubmodel[1]->getNumCols()==6);
	        myAssert(__FILE__,__LINE__,osiSubmodel[1]->getNumRows()==3);

	        delete smiModel;
	        //delete osiCore;

	        delete rlo0;
	        delete rlo1;

            for (int i = 0; i < submodel.size(); i++)
                delete submodel[i];

        }

        TEST_F(SmiUnitTest, SmiScenarioTreeUnitTest) {
	        int  *i1 = new int(1);
	        int  *i2= new int(12);
	        int  *i3= new int(13);
	        int  *i4= new int(24);
	        int  *i5= new int(25);
	        int  *i6= new int(36);
	        int  *i7= new int(37);
	        int  *i8= new int(38);

	        vector<int *> path4(3);
	        path4[0]= i1;
	        path4[1]= i2;
	        path4[2]= i4;

	        vector<int *> path5(1, i5);

	        vector<int *> path6(2);
	        path6[0]= i3;
	        path6[1]= i6;

	        vector<int *> path7(1, i7);

	        vector<int *> path8(1, i8);


	        SmiScenarioTree<int*> s;

	        int is1 = s.addPathtoLeaf(0,0,path4);
	        int is2 = s.addPathtoLeaf(is1,1,path5);
	        int is3 = s.addPathtoLeaf(is2,0,path6);
	        int is4 = s.addPathtoLeaf(is3,1,path7);
	        int is5 = s.addPathtoLeaf(is3,1,path8);

	        int  *ii1 = new int(1);
	        int  *ii2= new int(12);
	        int  *ii3= new int(13);
	        int  *ii4= new int(24);
	        int  *ii5= new int(25);
	        int  *ii6= new int(36);
	        int  *ii7= new int(37);
	        int  *ii8= new int(38);


	        vector<int *> ppath4(3);
	        vector<int> label4(3);
	        ppath4[0]= ii1; label4[0]=*ii1;
	        ppath4[1]= ii2; label4[1]=*ii2;
	        ppath4[2]= ii4; label4[2]=*ii4;

	        vector<int *> ppath5(3);
	        ppath5[0] = ppath4[0];
	        ppath5[1] = ppath4[1];
	        ppath5[2] = ii5;
	        vector<int> label5(3);
	        label5[0] = label4[0];
	        label5[1] = label4[1];
	        label5[2] = *ii5;

	        vector<int *> ppath6(3);
	        ppath6[0] = ppath5[0];
	        vector<int> label6(3);
	        label6[0] = label5[0];
	        ppath6[1]= ii3; label6[1] = *ii3;
	        ppath6[2]= ii6; label6[2] = *ii6;

	        vector<int *> ppath7(3);
	        ppath7[0] = ppath6[0];
	        ppath7[1] = ppath6[1];
	        ppath7[2] = ii7;

	        vector<int> label7(3);
	        label7[0] = label6[0];
	        label7[1] = label6[1];
	        label7[2] = *ii7;

	        vector<int *> ppath8(3);
	        ppath8[0] = ppath6[0];
	        ppath8[1] = ppath6[1];
	        ppath8[2] = ii8;
	        vector<int> label8(3);
	        label8[0] = label6[0];
	        label8[1] = label6[1];
	        label8[2] = *ii8;

	        // tree s1 is identical to s just uses labels to id nodes
	        SmiScenarioTree<int*> s1;


	        SmiTreeNode<int *> *node;

	        int is11 = s1.addPathtoLeaf(label4,ppath4);
	        s1.setChildLabels(s1.getRoot(),label4);

	        node = s1.find(label5);
	        int is12 = s1.addPathtoLeaf(node->scenario(),node->depth(),ppath5,
		        node->depth()+1);
	        s1.setChildLabels(node,label5);

	        node = s1.find(label6);
	        int is13 = s1.addPathtoLeaf(node->scenario(),node->depth(),ppath6,
		        node->depth()+1);
	        s1.setChildLabels(node,label6);

	        node = s1.find(label7);
	        int is14 = s1.addPathtoLeaf(node->scenario(),node->depth(),ppath7,
		        node->depth()+1);
	        s1.setChildLabels(node,label7);

	        node = s1.find(label8);
	        int is15 = s1.addPathtoLeaf(node->scenario(),node->depth(),ppath8,
		        node->depth()+1);
	        s1.setChildLabels(node,label8);



	        vector<int *> vec1 = s.getScenario(is1);
	        myAssert(__FILE__,__LINE__, vec1[0] == i1 );
	        myAssert(__FILE__,__LINE__, vec1[1] == i2 );
	        myAssert(__FILE__,__LINE__, vec1[2] == i4 );

	        vector<int *> vec11 = s1.getScenario(is11);
	        myAssert(__FILE__,__LINE__, vec11[0] == ii1 );
	        myAssert(__FILE__,__LINE__, vec11[1] == ii2 );
	        myAssert(__FILE__,__LINE__, vec11[2] == ii4 );

	        vector<int *>::iterator vbeg = s.scenBegin(is1);
	        vector<int *>::iterator vend = s.scenEnd(is1);
	        myAssert(__FILE__,__LINE__, *vbeg++ == i1 );
	        myAssert(__FILE__,__LINE__, *vbeg++ == i2 );
	        myAssert(__FILE__,__LINE__, *vbeg++ == i4 );
	        myAssert(__FILE__,__LINE__, vbeg == vend );

	        vector<int *>::iterator vbeg1 = s1.scenBegin(is11);
	        vector<int *>::iterator vend1 = s1.scenEnd(is11);
	        myAssert(__FILE__,__LINE__, *vbeg1++ == ii1 );
	        myAssert(__FILE__,__LINE__, *vbeg1++ == ii2 );
	        myAssert(__FILE__,__LINE__, *vbeg1++ == ii4 );
	        myAssert(__FILE__,__LINE__, vbeg1 == vend1 );


	        vector<int *> vec2 = s.getScenario(is2);
	        myAssert(__FILE__,__LINE__, vec2[0] == i1 );
	        myAssert(__FILE__,__LINE__, vec2[1] == i2 );
	        myAssert(__FILE__,__LINE__, vec2[2] == i5 );


	        vector<int *> vec12 = s1.getScenario(is12);
	        myAssert(__FILE__,__LINE__, vec12[0] == ii1 );
	        myAssert(__FILE__,__LINE__, vec12[1] == ii2 );
	        myAssert(__FILE__,__LINE__, vec12[2] == ii5 );

	        vector<int *> vec3 = s.getScenario(is3);
	        myAssert(__FILE__,__LINE__, vec3[0] == i1 );
	        myAssert(__FILE__,__LINE__, vec3[1] == i3 );
	        myAssert(__FILE__,__LINE__, vec3[2] == i6 );

	        vector<int *> vec13 = s1.getScenario(is13);
	        myAssert(__FILE__,__LINE__, vec13[0] == ii1 );
	        myAssert(__FILE__,__LINE__, vec13[1] == ii3 );
	        myAssert(__FILE__,__LINE__, vec13[2] == ii6 );

	        vector<int *> vec4 = s.getScenario(is4);
	        myAssert(__FILE__,__LINE__, vec4[0] == i1 );
	        myAssert(__FILE__,__LINE__, vec4[1] == i3 );
	        myAssert(__FILE__,__LINE__, vec4[2] == i7 );


	        vector<int *> vec14 = s1.getScenario(is14);
	        myAssert(__FILE__,__LINE__, vec14[0] == ii1 );
	        myAssert(__FILE__,__LINE__, vec14[1] == ii3 );
	        myAssert(__FILE__,__LINE__, vec14[2] == ii7 );

	        vector<int *> vec5 = s.getScenario(is5);
	        myAssert(__FILE__,__LINE__, vec5[0] == i1 );
	        myAssert(__FILE__,__LINE__, vec5[1] == i3 );
	        myAssert(__FILE__,__LINE__, vec5[2] == i8 );


	        vector<int *> vec15 = s1.getScenario(is15);
	        myAssert(__FILE__,__LINE__, vec15[0] == ii1 );
	        myAssert(__FILE__,__LINE__, vec15[1] == ii3 );
	        myAssert(__FILE__,__LINE__, vec15[2] == ii8 );

	        vector<int*>::iterator i;
	        i=s.treeBegin();
	        myAssert(__FILE__,__LINE__,*i==i1);
	        i++;
	        myAssert(__FILE__,__LINE__,*i==i2);
	        i = s.treeEnd();
	        i--;
	        myAssert(__FILE__,__LINE__,*i==i8);

	        vector<int*>::iterator ii;
	        ii=s1.treeBegin();
	        myAssert(__FILE__,__LINE__,*ii==ii1);
	        ii++;
	        myAssert(__FILE__,__LINE__,*ii==ii2);
	        ii = s1.treeEnd();
	        ii--;
	        myAssert(__FILE__,__LINE__,*ii==ii8);

	        vbeg = s.scenBegin(is1);
	        vend = s.scenEnd(is1);
	        myAssert(__FILE__,__LINE__, *vbeg++ == i1 );
	        myAssert(__FILE__,__LINE__, *vbeg++ == i2 );
	        myAssert(__FILE__,__LINE__, *vbeg++ == i4 );
	        myAssert(__FILE__,__LINE__, vbeg == vend );

	        vbeg1 = s1.scenBegin(is11);
	        vend1 = s1.scenEnd(is11);
	        myAssert(__FILE__,__LINE__, *vbeg1++ == ii1 );
	        myAssert(__FILE__,__LINE__, *vbeg1++ == ii2 );
	        myAssert(__FILE__,__LINE__, *vbeg1++ == ii4 );
	        myAssert(__FILE__,__LINE__, vbeg1 == vend1 );



	        delete i1;
	        delete i2;
	        delete i3;
	        delete i4;
	        delete i5;
	        delete i6;
	        delete i7;
	        delete i8;

	        delete ii1;
	        delete ii2;
	        delete ii3;
	        delete ii4;
	        delete ii5;
	        delete ii6;
	        delete ii7;
	        delete ii8;
        }


        TEST_F(SmiUnitTest, SmiTreeNodeUnitTest) {
	        int  *i1 = new int(1);
	        int  *i2= new int(12);
	        int  *i3= new int(13);
	        int  *i4= new int(24);
	        int  *i5= new int(25);
	        int  *i6= new int(36);
	        int  *i7= new int(37);
	        int  *i8= new int(38);

	        SmiTreeNode<int *> *n1 = new SmiTreeNode<int *>(i1);

	        SmiTreeNode<int *> *n2;
	        SmiTreeNode<int *> *n3;
	        SmiTreeNode<int *> *n4;
	        SmiTreeNode<int *> *n5;
	        SmiTreeNode<int *> *n6;
	        SmiTreeNode<int *> *n7;
	        SmiTreeNode<int *> *n8;

	        n2 = n1->addChild(i2,0);
	        n3 = n1->addChild(i3,1);
	        n4 = n2->addChild(i4,0);
	        n5 = n2->addChild(i5,2);
	        n6 = n3->addChild(i6,1);
	        n7 = n3->addChild(i7,3);
	        n8 = n3->addChild(i8,4);

	        myAssert(__FILE__,__LINE__, n1->depth() == 0 );
	        myAssert(__FILE__,__LINE__, n2->depth() == 1 );
	        myAssert(__FILE__,__LINE__, n3->depth() == 1 );
	        myAssert(__FILE__,__LINE__, n4->depth() == 2 );
	        myAssert(__FILE__,__LINE__, n5->depth() == 2 );
	        myAssert(__FILE__,__LINE__, n6->depth() == 2 );
	        myAssert(__FILE__,__LINE__, n7->depth() == 2 );
	        myAssert(__FILE__,__LINE__, n8->depth() == 2 );

	        // parents point to last children
	        myAssert(__FILE__,__LINE__, n1->getChild() == n3 );
	        myAssert(__FILE__,__LINE__, n3->getChild() == n8 );

	        // siblings
	        myAssert(__FILE__,__LINE__, n3->hasSibling() );
	        myAssert(__FILE__,__LINE__, n3->getSibling() == n2 );
	        myAssert(__FILE__,__LINE__, !n2->hasSibling() );

	        // sibling and cousin pointers are a linked list
	        // for same level nodes
	        myAssert(__FILE__,__LINE__, n8->hasSibling() );
	        myAssert(__FILE__,__LINE__, n8->getSibling() == n7 );
	        myAssert(__FILE__,__LINE__, n7->hasSibling() );
	        myAssert(__FILE__,__LINE__, n7->getSibling() == n6 );
	        myAssert(__FILE__,__LINE__, !n6->hasSibling());

	        myAssert(__FILE__,__LINE__, n6->getParent()->getSibling()  == n2
		        );
	        myAssert(__FILE__,__LINE__, n2->getChild() == n5 );
	        myAssert(__FILE__,__LINE__, n5->hasSibling() );
	        myAssert(__FILE__,__LINE__, n5->getSibling() == n4 );
	        // last element of same level list
	        myAssert(__FILE__,__LINE__, !n4->hasSibling());

	        vector<SmiTreeNode<int *> *> *vec1 = n1->getChildren();
	        assert ((*vec1)[0] == n2 );
	        assert ((*vec1)[1] == n3 );
	        delete vec1;

	        delete i1;
	        delete i2;
	        delete i3;
	        delete i4;
	        delete i5;
	        delete i6;
	        delete i7;
	        delete i8;

	        delete n1;
        }





        TEST_F(SmiUnitTest, SmiScnSmpsIOUnitTestReplace) {
#ifdef _MSC_VER
	        std::string dataDir="../../../../Smi/test/SmiTestData";
#else
            std::string dataDir="../../Smi/test/SmiTestData";
#endif
	        int nrows, ncols;

	        // test SMPS files app0110R
	        SmiScnModel smi;
	        myAssert(__FILE__,__LINE__,-1!=smi.readSmps((dataDir+"/app0110R").c_str()));

	        OsiClpSolverInterface *clp = new OsiClpSolverInterface();
	        smi.setOsiSolverHandle(*clp);
	        OsiSolverInterface *osiStoch = smi.loadOsiSolverData();

	        nrows = osiStoch->getNumRows();
	        ncols = osiStoch->getNumCols();
	        myAssert(__FILE__,__LINE__,nrows==129);
	        myAssert(__FILE__,__LINE__,ncols==268);

	        osiStoch->initialSolve();
	        myAssert(__FILE__,__LINE__,fabs(osiStoch->getObjValue()-
		        44.66666) < 0.0001);
	        printf(" *** Successfully tested SMPS interfaces on app0110	with Replace option.\n");

	        delete clp;

        }

        TEST_F(SmiUnitTest, SmiScnSmpsIOUnitTestAdd) {
#ifdef _MSC_VER
	        std::string dataDir="../../../../Smi/test/SmiTestData";
#else
            std::string dataDir="../../Smi/test/SmiTestData";
#endif
	        int nrows, ncols;

	        // test SMPS files app0110 -- ADD scenario values
	        SmiScnModel smi;
	        myAssert(__FILE__,__LINE__,-1!=smi.readSmps((dataDir+"/app0110").c_str()));
	        OsiClpSolverInterface *clp = new OsiClpSolverInterface();
	        smi.setOsiSolverHandle(*clp);
	        OsiSolverInterface *osiStoch = smi.loadOsiSolverData();

	        nrows = osiStoch->getNumRows();
	        ncols = osiStoch->getNumCols();
	        myAssert(__FILE__,__LINE__,nrows==129);
	        myAssert(__FILE__,__LINE__,ncols==268);

	        osiStoch->initialSolve();
	        myAssert(__FILE__,__LINE__,fabs(osiStoch->getObjValue()-
		        44.66666) < 0.0001);
	        printf(" *** Successfully tested SMPS interfaces on app0110	with Add option.\n");

	        delete clp;

        }

        TEST_F(SmiUnitTest, SmiScnModelScenarioUnitTest) {
	        OsiClpSolverInterface *osiClp1 = new OsiClpSolverInterface();
	        double INF = osiClp1->getInfinity();

	        // exhaustive test of direct interfaces for scenario generation

	        /* Model dimensions */
	        int ncol=27, nrow=9, nels=44;

	        /* Sparse matrix data...organized by row */
	        int *mrow,cmrow[]={ 0, 0, 0, 0, 0,
		        1, 1, 1, 1,
		        2, 2, 2,
		        3, 3, 3, 3, 3,
		        4, 4, 4, 4,
		        5, 5, 5, 5, 5, 5,
		        6, 6, 6, 6, 6,
		        7, 7, 7, 7, 7, 7,
		        8, 8, 8, 8, 8, 8 };
	        int *mcol,cmcol[]={ 0, 1, 2, 3, 4,
		        5, 6, 7, 8,
		        9,10, 11,
		        12, 13, 14, 15, 16,
		        0,        12, 17, 18,
		        1, 5, 9,  13, 19, 20,
		        2, 6,     14, 21, 22,
		        3, 7, 10, 15, 23, 24,
		        4, 8, 11, 16, 25, 26 };

	        double dels[] = { 1.0, 1.0, 1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0, 1.0, 1.0,
		        16.0,              9.0, -1.0, 1.0,
		        15.0, 10.0,  5.0, 11.0, -1.0, 1.0,
		        28.0, 14.0,       22.0, -1.0, 1.0,
		        23.0, 15.0,  7.0, 17.0, -1.0, 1.0,
		        81.0, 57.0, 29.0, 55.0, -1.0, 1.0 };

	        /* Objective */
	        double *dobj,cdobj[]={ 18.0, 21.0, 18.0, 16.0, 10.0, 15.0, 16.0,
		        14.0, 9.0,
		        10.0,  9.0,  6.0, 17.0, 16.0, 17.0, 15.0, 10.0, 0.0,
		        13.0,  0.0, 13.0,  0.0,  7.0,  0.0,  7.0,  0.0, 1.0 };

	        /* Column bounds */
	        double *dclo,cdclo[]={ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		        0.0,  0.0,
		        0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		        0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0 };

	        double *dcup,cdcup[]={ INF,  INF,  INF,  INF,  INF,  INF,  INF,
		        INF,  INF,
		        INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,
		        INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF };

	        /* Row bounds */
	        double *drlo,cdrlo[]={ -INF, -INF, -INF, -INF,  0.0, 4.0, 0.0,
		        8.0, 10.0 };
	        double *drup,cdrup[]={ 10.0, 19.0, 25.0, 15.0,  0.0, 7.0, 0.0,
		        8.0, 90.0 };

	        /* Stages */
	        //int nstg=2;
	        int n_first_stg_rows=4;
	        int n_second_stg_rows=4;
	        int n_first_stg_cols=17;
	        int n_second_stg_cols=8;
	        int *rstg,crstg[]={ 0,0,0,0,1,1,1,1,2 };
	        int *cstg,ccstg[]={ 0,0,0,0,0,0,0,0,0,
		        0,0,0,0,0,0,0,0,1,
		        1,1,1,1,1,1,1,2,2 };

	        /* Stochastic data */
	        int nindp=5;
	        int nsamp[]={ 5, 2, 5, 5, 3 };
	        double demand[]={ 200, 220, 250, 270, 300,
		        50, 150,
		        140, 160, 180, 200, 220,
		        10, 50, 80, 100, 340,
		        580, 600, 620 };
	        double dprobs[]={ 0.2, 0.05, 0.35, 0.2, 0.2,
		        0.3, 0.7,
		        0.1, 0.2, 0.4, 0.2, 0.1,
		        0.2, 0.2, 0.3, 0.2, 0.1,
		        0.1, 0.8, 0.1 };

	        /* scramble */

	        int irow[]={ 1,2,7,8,0,3,4,5,6};
	        int icol[]={ 9,2,3,4,5,6,7,8,1,
		        19,21,23,25,0,26,24,22,20,
		        10,11,12,13,14,15,16,17,18 };

	        /* local variables */
	        int ns=1,ii,iii,jj,*indx,*incr;
	        double dp=1.0;

	        for (ii=0;ii<nindp;ii++) ns *= nsamp[ii];     /* Compute number of
												          scenarios */
	        // initialize SmiModel
	        SmiScnModel *smiModel = new SmiScnModel();
	        smiModel->setOsiSolverHandle(*osiClp1);

	        /* scramble LP entries -- only to test sorting functionality!! */
	        mrow = (int*)malloc(nels*sizeof(int));
	        mcol = (int*)malloc(nels*sizeof(int));
	        for (ii=0;ii<nels;ii++)
	        {
		        mcol[ii] = icol[cmcol[ii]];
		        mrow[ii] = irow[cmrow[ii]];
	        }

	        drlo = (double *)malloc(nrow*sizeof(double));
	        drup = (double *)malloc(nrow*sizeof(double));
	        rstg = (int *)malloc(nrow*sizeof(int));
	        for (ii=0;ii<nrow;ii++)
	        {
		        drlo[irow[ii]] = cdrlo[ii];
		        drup[irow[ii]] = cdrup[ii];
		        rstg[irow[ii]] = crstg[ii];
	        }

	        dclo = (double *)malloc(ncol*sizeof(double));
	        dcup = (double *)malloc(ncol*sizeof(double));
	        dobj = (double *)malloc(ncol*sizeof(double));
	        cstg = (int *)malloc(ncol*sizeof(int));
	        for (ii=0;ii<ncol;ii++)
	        {
		        dclo[icol[ii]] = cdclo[ii];
		        dcup[icol[ii]] = cdcup[ii];
		        dobj[icol[ii]] = cdobj[ii];
		        cstg[icol[ii]] = ccstg[ii];
	        }

	        // this to test the matrix update stanza in genScenario
	        CoinPackedMatrix *origmat = new CoinPackedMatrix(false,mrow,mcol,
		        dels,nels);
	        int corenels = nels - 4;


	        // set core model using Osi interface
	        OsiClpSolverInterface ocsi;
	        ocsi.loadProblem(CoinPackedMatrix(
		        1,mrow,mcol,dels,corenels),dclo,dcup,dobj,drlo,drup);

	        OsiSolverInterface *ohoh= ocsi.clone();

	        // test Core Model
	        SmiCoreData *osiCore = new SmiCoreData(ohoh,3,cstg,rstg);

	        myAssert(__FILE__,__LINE__,osiCore->getNumCols(0) ==
		        n_first_stg_cols);
	        myAssert(__FILE__,__LINE__,osiCore->getNumCols(1) ==
		        n_second_stg_cols);
	        myAssert(__FILE__,__LINE__,osiCore->getNumCols(2) == ncol -
		        n_first_stg_cols - n_second_stg_cols);

	        myAssert(__FILE__,__LINE__,osiCore->getNumRows(0) ==
		        n_first_stg_rows);
	        myAssert(__FILE__,__LINE__,osiCore->getNumRows(1) ==
		        n_second_stg_rows);
	        myAssert(__FILE__,__LINE__,osiCore->getNumRows(2) == nrow -
		        n_first_stg_rows - n_second_stg_rows);

	        myAssert(__FILE__,__LINE__,osiCore->getColStart(0) == 0);
	        myAssert(__FILE__,__LINE__,osiCore->getColStart(1) ==
		        n_first_stg_cols );
	        myAssert(__FILE__,__LINE__,osiCore->getColStart(2) ==
		        n_first_stg_cols + n_second_stg_cols);
	        myAssert(__FILE__,__LINE__,osiCore->getColStart(3) == ncol);

	        myAssert(__FILE__,__LINE__,osiCore->getRowStart(0) == 0);
	        myAssert(__FILE__,__LINE__,osiCore->getRowStart(1) ==
		        n_first_stg_rows );
	        myAssert(__FILE__,__LINE__,osiCore->getRowStart(2) ==
		        n_first_stg_rows + n_second_stg_rows);
	        myAssert(__FILE__,__LINE__,osiCore->getRowStart(3) == nrow);

	        for (ii = 0; ii < n_first_stg_cols ; ii++)
		        myAssert(__FILE__,__LINE__,cstg[osiCore->getColExternalIndex(ii)] == 0);
	        for (ii = n_first_stg_cols; ii < n_first_stg_cols + n_second_stg_cols ; ii++)
		        myAssert(__FILE__,__LINE__,cstg[osiCore->getColExternalIndex(ii)] == 1);
	        for (ii = n_first_stg_cols + n_second_stg_cols; ii < ncol ; ii++)
		        myAssert(__FILE__,__LINE__,cstg[osiCore->getColExternalIndex(ii)] == 2);

	        for (ii = 0; ii < n_first_stg_rows ; ii++)
		        myAssert(__FILE__,__LINE__,rstg[osiCore->getRowExternalIndex(ii)] == 0);
	        for (ii = n_first_stg_rows; ii < n_first_stg_rows +	n_second_stg_rows ; ii++)
		        myAssert(__FILE__,__LINE__,rstg[osiCore->getRowExternalIndex(ii)] == 1);
	        for (ii = n_first_stg_rows + n_second_stg_rows; ii < nrow ; ii++)
		        myAssert(__FILE__,__LINE__,rstg[osiCore->getRowExternalIndex(ii)] == 2);

	        const CoinPackedMatrix *origCore = ohoh->getMatrixByRow();
	        int t;
	        for ( t=0;t<3;t++)
	        {
		        SmiNodeData *n=osiCore->getNode(t);
		        CoinPackedVector cpvdrlo(n->getRowLowerLength(),n->getRowLowerIndices(),n->getRowLowerElements());
		        CoinPackedVector cpvdrup(n->getRowUpperLength(),n->getRowUpperIndices(),n->getRowUpperElements());
		        CoinPackedVector cpvdclo(n->getColLowerLength(),n->getColLowerIndices(),n->getColLowerElements());
		        CoinPackedVector cpvdcup(n->getColUpperLength(),n->getColUpperIndices(),n->getColUpperElements());
		        CoinPackedVector cpvdobj(n->getObjectiveLength(),n->getObjectiveIndices(),n->getObjectiveElements());


		        double *core_drlo = new double[osiCore->getNumRows(t)];
		        osiCore->getNode(t)->copyRowLower(core_drlo);

		        double elt1,elt2;
		        int ic;
		        for(ii=osiCore->getColStart(t);ii<osiCore->getColStart(t+1);ii++)
		        {
			        ic = osiCore->getColExternalIndex(ii);
			        elt1 = cpvdclo[ii];
			        elt2 = dclo[ic];
			        myAssert(__FILE__,__LINE__,elt1==elt2);
			        elt1 = cpvdcup[ii];
			        elt2 = dcup[ic];
			        myAssert(__FILE__,__LINE__,elt1==elt2);
			        elt1 = cpvdobj[ii];
			        elt2 = dobj[ic];
			        myAssert(__FILE__,__LINE__,elt1==elt2);
		        }
		        for(ii=osiCore->getRowStart(t);ii<osiCore->getRowStart(t+1);ii++)
		        {
			        myAssert(__FILE__,__LINE__,cpvdrlo[ii]==drlo[osiCore->getRowExternalIndex(ii)]);
			        myAssert(__FILE__,__LINE__,cpvdrup[ii]==drup[osiCore->getRowExternalIndex(ii)]);
			        myAssert(__FILE__,__LINE__,core_drlo[ii-osiCore->getRowStart(t)] ==drlo[osiCore->getRowExternalIndex(ii)]);

			        CoinPackedVector row1(n->getRowLength(ii),n->getRowIndices(ii),n->getRowElements(ii));
			        const CoinPackedVector row2 =origCore->getVector(osiCore->getRowExternalIndex(ii));
			        myAssert(__FILE__,__LINE__,row1.getNumElements() == row2.getNumElements());
			        int *indx = row1.getIndices();
			        double *els = row1.getElements();
			        for (int j=0; j<row1.getNumElements(); j++)
			        {
				        elt1 = els[j];
				        ic = osiCore->getColExternalIndex(indx[j]);
				        elt2 = row2[ic];
				        myAssert(__FILE__,__LINE__,elt1==elt2);
			        }
		        }
		        delete[] core_drlo;
	        }

	        delete ohoh; ohoh = NULL;

	        printf(" *** Successfully tested SmiScnCoreModel.\n");



	        // Coin structures for scenario updates to right hand sides
	        CoinPackedVector cpv_rlo;
	        CoinPackedVector cpv_rup;

	        // Coin structure for scenario "updates" to core matrix
	        // ..row-ordered
	        CoinPackedMatrix *cpm_mat = new CoinPackedMatrix(false,mrow+corenels,mcol+corenels,dels+corenels,nels-corenels);

	        // initialize right hand side data for first scenario
	        indx = (int *) malloc( (1+nindp)*sizeof(int) );
	        memset( indx,0,(1+nindp)*sizeof(int));
	        for (jj=0;jj<nindp;jj++) {
		        indx[jj+1] += indx[jj] + nsamp[jj];
		        dp *= dprobs[ indx[jj] ];

		        drlo[irow[n_first_stg_rows + jj]] = demand[ indx[jj] ];
		        drup[irow[n_first_stg_rows + jj]] = demand[ indx[jj] ];

		        cpv_rlo.insert(irow[n_first_stg_rows + jj],demand[ indx[jj] ]);
		        cpv_rup.insert(irow[n_first_stg_rows + jj],demand[ indx[jj] ]);
	        }

	        // first scenario
	        int anc = 0;
	        int branch = 1;
	        int	is = smiModel->generateScenario(osiCore,cpm_mat,NULL,NULL,NULL,
				        &cpv_rlo,&cpv_rup,branch,anc,dp);


	        myAssert(__FILE__,__LINE__,smiModel->getNumScenarios()==1);

	        // test first scenario

	        // load problem data into OsiSolver
	        smiModel->loadOsiSolverData();
	        // get Osi pointer
	        OsiSolverInterface *smiOsi1 = smiModel->getOsiSolverInterface();

	        int nStochCol = smiOsi1->getNumCols();
	        int nStochRow = smiOsi1->getNumRows();
	        double totalProb = dp;

	        // get arrays
	        const double *stochdrlo = smiOsi1->getRowLower();
	        const double *stochdrup = smiOsi1->getRowUpper();
	        const double *stochdclo = smiOsi1->getColLower();
	        const double *stochdcup = smiOsi1->getColUpper();
	        const double *stochdobj = smiOsi1->getObjCoefficients();

	        // get matrix
	        const CoinPackedMatrix *stochmat = smiOsi1->getMatrixByRow();
	        for ( t=0;t<3;t++)
	        {
		        double elt1,elt2;
		        int ic;
		        for(ii=osiCore->getColStart(t);ii<osiCore->getColStart(t+1);ii++)
		        {
			        ic = osiCore->getColExternalIndex(ii);
			        elt1 = stochdclo[ii];
			        elt2 = dclo[ic];
			        myAssert(__FILE__,__LINE__,elt1==elt2);
			        elt1 = stochdcup[ii];
			        elt2 = dcup[ic];
			        myAssert(__FILE__,__LINE__,elt1==elt2);
			        elt1 = stochdobj[ii];
			        elt2 = dobj[ic];
			        myAssert(__FILE__,__LINE__,elt1==elt2);
		        }
		        int ir;
		        for(ii=osiCore->getRowStart(t);ii<osiCore->getRowStart(t+1);ii++)
		        {

			        ir = osiCore->getRowExternalIndex(ii);

			        myAssert(__FILE__,__LINE__,stochdrlo[ii]==drlo[ir]);
			        myAssert(__FILE__,__LINE__,stochdrup[ii]==drup[ir]);

			        const CoinPackedVector row1 = stochmat->getVector(ii);
			        const CoinPackedVector row2 = origmat->getVector(ir);
			        myAssert(__FILE__,__LINE__,row1.getNumElements() == row2.getNumElements());
			        const int *indx = row1.getIndices();
			        const double *els = row1.getElements();
			        for (int j=0; j<row1.getNumElements(); j++)
			        {
				        elt1 = els[j];
				        ic = osiCore->getColExternalIndex(indx[j]);
				        elt2 = row2[ic];
				        myAssert(__FILE__,__LINE__,elt1==elt2);
			        }
		        }
	        }

	        printf(" *** Successfully tested problem with scenario %d.\n",is);


	        /***** ...main loop to generate scenarios from discrete random
	        variables
	        For each scenario index ii:
	        If the sample size nsamp[jj] divides the scenario index ii,
	        reverse the increment direction incr[jj]
	        and increase the random variable index jj by 1.
	        Increment the jj'th random variable by incr[jj]
	        and generate new sample data.
	        ***** */

	        /* sample space increment initialized to 1 */
	        incr = (int *) malloc( nindp*sizeof(int) );
	        for (jj=0;jj<nindp;jj++) incr[jj] = 1;

	        for (int iss=1;iss<ns;iss++) {
		        iii=iss; jj=0;
		        while ( !(iii%nsamp[jj]) ) {
			        iii /= nsamp[jj];
			        incr[jj] = -incr[jj];
			        jj++;
		        }
		        dp /= dprobs[ indx[jj] ];
		        indx[jj] += incr[jj];
		        dp *= dprobs[ indx[jj] ];

		        // set data
		        drlo[irow[n_first_stg_rows + jj]] = demand[ indx[jj] ];
		        drup[irow[n_first_stg_rows + jj]] = demand[ indx[jj] ];

		        cpv_rlo.setElement(cpv_rlo.findIndex(irow[n_first_stg_rows + jj]),demand[ indx[jj] ]);
		        cpv_rup.setElement(cpv_rup.findIndex(irow[n_first_stg_rows + jj]),demand[ indx[jj] ]);

		        // genScenario
		        is = smiModel->generateScenario(osiCore,cpm_mat,NULL,NULL,NULL,
				        &cpv_rlo,&cpv_rup,branch,anc,dp);


		        if (is < 3)
		        {
			        // test scenario

			        // load problem data into OsiSolver
			        smiModel->loadOsiSolverData();
			        // get Osi pointer
			        OsiSolverInterface *smiOsi1 = smiModel->getOsiSolverInterface();

			        totalProb += dp;

			        // get arrays
			        stochdrlo = smiOsi1->getRowLower()+nStochRow;
			        stochdrup = smiOsi1->getRowUpper()+nStochRow;
			        stochdclo = smiOsi1->getColLower()+nStochCol;
			        stochdcup = smiOsi1->getColUpper()+nStochCol;
			        stochdobj = smiOsi1->getObjCoefficients()+nStochCol;

			        // get matrix
			        const CoinPackedMatrix *stochmat = smiOsi1->getMatrixByRow();
			        int t;
			        for (t=1;t<3;t++)
			        {
				        double elt1,elt2;
				        int ic;
				        int colOff = osiCore->getColStart(1);
				        for(ii=osiCore->getColStart(t);ii<osiCore->getColStart(t+1);ii++)
				        {
					        ic = osiCore->getColExternalIndex(ii);
					        elt1 = stochdclo[ii-colOff];
					        elt2 = dclo[ic];
					        myAssert(__FILE__,__LINE__,elt1==elt2);
					        elt1 = stochdcup[ii-colOff];
					        elt2 = dcup[ic];
					        myAssert(__FILE__,__LINE__,elt1==elt2);
					        elt1 = stochdobj[ii-colOff];
					        elt2 = dobj[ic];
					        myAssert(__FILE__,__LINE__,fabs(elt1 - (elt2*dp/totalProb)) < 1.0e-8);
				        }
				        int ir,rowOff;
				        rowOff = osiCore->getRowStart(1);
				        for(ii=osiCore->getRowStart(t);ii<osiCore->getRowStart(t+1);ii++)
				        {

					        ir = osiCore->getRowExternalIndex(ii);

					        myAssert(__FILE__,__LINE__,stochdrlo[ii-rowOff]==drlo[ir]);
					        myAssert(__FILE__,__LINE__,stochdrup[ii-rowOff]==drup[ir]);

					        const CoinPackedVector row1 = stochmat->getVector(ii);
					        const CoinPackedVector row2 = origmat->getVector(ir);
					        myAssert(__FILE__,__LINE__,row1.getNumElements()== row2.getNumElements());
					        const int *indx = row1.getIndices();
					        const double *els = row1.getElements();
					        for (int j=0; j<row1.getNumElements(); j++)
					        {
						        elt1 = els[j];
						        ic = osiCore->getColExternalIndex(indx[j]);
						        elt2 = row2[ic];
						        myAssert(__FILE__,__LINE__,elt1==elt2);
					        }
				        }
			        }

			        nStochCol = smiOsi1->getNumCols();
			        nStochRow = smiOsi1->getNumRows();

			        printf(" *** Successfully tested problem with scenario %d.\n",is);
		        }
	        }

	        myAssert(__FILE__,__LINE__,ns==smiModel->getNumScenarios());

	        // solve with decomp solver

	        // load problem data into OsiSolver
	        smiModel->loadOsiSolverData();
	        // get Osi pointer
	        OsiSolverInterface *smiOsi = smiModel->getOsiSolverInterface();
	        // set some parameters
	        smiOsi->setHintParam(OsiDoPresolveInInitial,true);
	        smiOsi->setHintParam(OsiDoScale,true);
	        smiOsi->setHintParam(OsiDoCrash,true);
	        // solve using Osi Solver
	        smiOsi->initialSolve();
	        // test optimal value
	        myAssert(__FILE__,__LINE__,fabs(smiOsi->getObjValue()-1566.042)<0.01);

	        // test solutions
	        const double *dsoln = smiOsi->getColSolution();
	        double objSum = 0.0;

	        /* The canonical way to traverse the tree:
	        For each scenario, get the leaf node.
	        Then get the parent.  Repeat until parent is NULL.
	        (Only the root node has a NULL parent.)
	        */
            double probSum = 0;

	        for(is=0; is<ns; ++is)
	        {
		        /* this loop calculates the scenario objective value */
		        double scenSum = 0.0;

		        // start with leaf node
		        SmiScnNode *node = smiModel->getLeafNode(is);

		        // leaf node probability is the scenario probability
		        double scenprob = node->getProb();

		        while (node != NULL)
		        {
			        // getColStart returns the starting index of node in OSI model
			        for(int j=node->getColStart();j<node->getColStart()+node->getNumCols(); ++j)
			        {
				        // getCoreColIndex returns the corresponding Core index
				        // in the original (user's) ordering
				        scenSum += dobj[node->getCoreColIndex(j)]*dsoln[j];
			        }
			        // get parent of node
			        node = node->getParent();
		        }
		        objSum += scenSum*scenprob;
                probSum += scenprob;
	        }

            myAssert(__FILE__,__LINE__,fabs(probSum-1) < 0.01);
	        myAssert(__FILE__,__LINE__,fabs(smiOsi->getObjValue()-objSum) <	0.01);
	        free (incr);
	        free (indx);
	        free( mrow) ;
	        free( mcol) ;
	        free( drlo) ;
	        free( drup) ;
	        free( rstg) ;
	        free( dclo) ;
	        free( dcup) ;
	        free( dobj) ;
	        free( cstg) ;
	        delete smiModel;

	        delete osiClp1;
	        delete osiCore;
	        delete origmat;
	        delete cpm_mat;
        }

        void replaceFirstWithSecond(CoinPackedVector &dfirst, const CoinPackedVector &dsecond)
        {
            double *delt1 = dfirst.getElements();
            const double *delt2 = dsecond.getElements();
            const int *indx2 = dsecond.getIndices();
            for(int j=0;j<dsecond.getNumElements();++j)
                delt1[dfirst.findIndex(indx2[j])] = delt2[j];
        }

        TEST_F(SmiUnitTest, SmiScnModelDiscreteUnitTest) {
	        // test of direct interfaces for discrete distribution

	        OsiClpSolverInterface *osiClp1 = new OsiClpSolverInterface();
	        double INF=osiClp1->getInfinity();

	        /* Model dimensions */
	        int ncol=27, nrow=9, nels=44;

	        /* Sparse matrix data...organized by row */
	        int *mrow,cmrow[]={ 0, 0, 0, 0, 0,
		        1, 1, 1, 1,
		        2, 2, 2,
		        3, 3, 3, 3, 3,
		        4, 4, 4, 4,
		        5, 5, 5, 5, 5, 5,
		        6, 6, 6, 6, 6,
		        7, 7, 7, 7, 7, 7,
		        8, 8, 8, 8, 8, 8 };
	        int *mcol,cmcol[]={ 0, 1, 2, 3, 4,
		        5, 6, 7, 8,
		        9,10, 11,
		        12, 13, 14, 15, 16,
		        0,        12, 17, 18,
		        1, 5, 9,  13, 19, 20,
		        2, 6,     14, 21, 22,
		        3, 7, 10, 15, 23, 24,
		        4, 8, 11, 16, 25, 26 };

	        double dels[] = { 1.0, 1.0, 1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0, 1.0, 1.0,
		        16.0,              9.0, -1.0, 1.0,
		        15.0, 10.0,  5.0, 11.0, -1.0, 1.0,
		        28.0, 14.0,       22.0, -1.0, 1.0,
		        23.0, 15.0,  7.0, 17.0, -1.0, 1.0,
		        81.0, 57.0, 29.0, 55.0, -1.0, 1.0 };

	        /* Objective */
	        double *dobj,cdobj[]={ 18.0, 21.0, 18.0, 16.0, 10.0, 15.0, 16.0,
		        14.0, 9.0,
		        10.0,  9.0,  6.0, 17.0, 16.0, 17.0, 15.0, 10.0, 0.0,
		        13.0,  0.0, 13.0,  0.0,  7.0,  0.0,  7.0,  0.0, 1.0 };

	        /* Column bounds */
	        double *dclo,cdclo[]={ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		        0.0,  0.0,
		        0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		        0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0 };


	        double *dcup,cdcup[]={ INF,  INF,  INF,  INF,  INF,  INF,  INF,
		        INF,  INF,
		        INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,
		        INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF,  INF };

	        /* Row bounds */
	        double *drlo,cdrlo[]={ -INF, -INF, -INF, -INF,  0.0, 4.0, 0.0,
		        8.0, 10.0 };
	        double *drup,cdrup[]={ 10.0, 19.0, 25.0, 15.0,  0.0, 7.0, 0.0,
		        8.0, 90.0 };

	        /* Stages */
	        //int nstg=2;
	        int n_first_stg_rows=4;
	        int *rstg,crstg[]={ 0,0,0,0,1,1,1,1,2 };
	        int *cstg,ccstg[]={ 0,0,0,0,0,0,0,0,0,
		        0,0,0,0,0,0,0,0,1,
		        1,1,1,1,1,1,1,2,2 };

	        /* Stochastic data */
	        int nindp=5;
	        int nsamp[]={ 5, 2, 5, 5, 3 };
	        double demand[]={ 200, 220, 250, 270, 300,
		        50, 150,
		        140, 160, 180, 200, 220,
		        10, 50, 80, 100, 340,
		        580, 600, 620 };
	        double dprobs[]={ 0.2, 0.05, 0.35, 0.2, 0.2,
		        0.3, 0.7,
		        0.1, 0.2, 0.4, 0.2, 0.1,
		        0.2, 0.2, 0.3, 0.2, 0.1,
		        0.1, 0.8, 0.1 };

	        /* scramble */

	        int irow[]={ 1,2,7,8,0,3,4,5,6};
	        int icol[]={ 9,2,3,4,5,6,7,8,1,
		        19,21,23,25,0,26,24,22,20,
		        10,11,12,13,14,15,16,17,18 };

	        /* local variables */
	        int ii,jj;


	        // initialize SmiModel
	        SmiScnModel *smiModel = new SmiScnModel();



	        smiModel->setOsiSolverHandle(*osiClp1);

	        /* scramble LP entries --- just for testing!! */
	        mrow = (int*)malloc(nels*sizeof(int));
	        mcol = (int*)malloc(nels*sizeof(int));
	        for (ii=0;ii<nels;ii++)
	        {
		        mcol[ii] = icol[cmcol[ii]];
		        mrow[ii] = irow[cmrow[ii]];
	        }

	        drlo = (double *)malloc(nrow*sizeof(double));
	        drup = (double *)malloc(nrow*sizeof(double));
	        rstg = (int *)malloc(nrow*sizeof(int));
	        for (ii=0;ii<nrow;ii++)
	        {
		        drlo[irow[ii]] = cdrlo[ii];
		        drup[irow[ii]] = cdrup[ii];
		        rstg[irow[ii]] = crstg[ii];
	        }

	        dclo = (double *)malloc(ncol*sizeof(double));
	        dcup = (double *)malloc(ncol*sizeof(double));
	        dobj = (double *)malloc(ncol*sizeof(double));
	        cstg = (int *)malloc(ncol*sizeof(int));
	        for (ii=0;ii<ncol;ii++)
	        {
		        dclo[icol[ii]] = cdclo[ii];
		        dcup[icol[ii]] = cdcup[ii];
		        dobj[icol[ii]] = cdobj[ii];
		        cstg[icol[ii]] = ccstg[ii];
	        }

	        // this to test the matrix update stanza in genScenario
	        CoinPackedMatrix *origmat = new CoinPackedMatrix(false,mrow,mcol,dels,nels);
	        int corenels = nels - 4;


	        // set core model using Osi interface
	        OsiClpSolverInterface ocsi;
	        ocsi.loadProblem(CoinPackedMatrix(1,mrow,mcol,dels,corenels),dclo,dcup,dobj,drlo,drup);

	        // core model with 3 stages
	        SmiCoreData *smiCore = new SmiCoreData(&ocsi,3,cstg,rstg);

	        // Coin structure for scenario "updates" to core matrix
	        // ..row-ordered
	        CoinPackedMatrix *cpm_mat = new CoinPackedMatrix(false,mrow+corenels,mcol+corenels,dels+corenels,nels-corenels);



	        // Create discrete distribution
	        SmiDiscreteDistribution *smiDD = new SmiDiscreteDistribution(
		        smiCore);

	        int index=0;
	        for (jj=0;jj<nindp;jj++)
	        {
		        SmiDiscreteRV *smiRV = new SmiDiscreteRV(1);
		        for (ii=0;ii<nsamp[jj];ii++)
		        {
			        CoinPackedVector empty_vec;
			        CoinPackedVector cpv_rlo ;
			        CoinPackedVector cpv_rup ;
			        cpv_rlo.insert(irow[n_first_stg_rows + jj], demand[index+
				        ii]);
			        cpv_rup.insert(irow[n_first_stg_rows + jj], demand[index+
				        ii]);
			        smiRV->addEvent(*cpm_mat,empty_vec,empty_vec,empty_vec,
				        cpv_rlo,cpv_rup,dprobs[index+ii]);
			        cpv_rlo.clear();
			        cpv_rup.clear();
		        }
		        myAssert(__FILE__,__LINE__,static_cast<int>(smiRV->
			        getNumEvents())==nsamp[jj]);
		        for (ii=0;ii<nsamp[jj];ii++)
		        {
			        myAssert(__FILE__,__LINE__,smiRV->getEventColLower(ii).
				        getNumElements()==0);
			        myAssert(__FILE__,__LINE__,smiRV->getEventColUpper(ii).
				        getNumElements()==0);
			        myAssert(__FILE__,__LINE__,smiRV->getEventObjective(ii).
				        getNumElements()==0);
			        myAssert(__FILE__,__LINE__,smiRV->getEventMatrix(ii).
				        getNumElements()==cpm_mat->getNumElements());
			        myAssert(__FILE__,__LINE__,smiRV->getEventRowLower(ii).
				        getElements()[0] == demand[index+ii]);
			        myAssert(__FILE__,__LINE__,smiRV->getEventRowLower(ii).
				        getIndices()[0] == irow[n_first_stg_rows + jj]);
			        myAssert(__FILE__,__LINE__,smiRV->getEventRowUpper(ii).
				        getElements()[0] == demand[index+ii]);
			        //printf("event prob %g\n",smiRV->getEventProb(ii));
			        myAssert(__FILE__,__LINE__,fabs(smiRV->getEventProb(ii) -
				        dprobs[index+ii]) < 0.0000001);
		        }
		        smiDD->addDiscreteRV(smiRV);
		        index+=nsamp[jj];
	        }

	        myAssert(__FILE__,__LINE__,smiDD->getNumRV() == nindp);

	        // this is cut-pasted from
	        smiModel->processDiscreteDistributionIntoScenarios(smiDD);

	        if (0)
	        {
		        SmiCoreData *core=smiDD->getCore();

		        int nindp = smiDD->getNumRV();
		        myAssert(__FILE__,__LINE__,nindp > 0);

		        int ns=1;
		        double dp=1.0;

		        CoinPackedMatrix matrix ;
		        CoinPackedVector cpv_dclo ;
		        CoinPackedVector cpv_dcup ;
		        CoinPackedVector cpv_dobj ;
		        CoinPackedVector cpv_drlo ;
		        CoinPackedVector cpv_drup ;

		        cpv_dclo.setTestForDuplicateIndex(true);
		        cpv_dcup.setTestForDuplicateIndex(true);
		        cpv_dobj.setTestForDuplicateIndex(true);
		        cpv_drlo.setTestForDuplicateIndex(true);
		        cpv_drup.setTestForDuplicateIndex(true);

		        // initialize data for first scenario
		        vector<int> indx(nindp);
		        vector<int> nsamp(nindp);
		        vector<int> label(core->getNumStages());
		        vector<int>::iterator iLabel;

		        for (iLabel=label.begin(); iLabel<label.end(); ++iLabel)
			        *iLabel=0;

		        int jj;
		        int index=0;
		        for (jj=0;jj<nindp;jj++) {
			        SmiDiscreteRV *smiRV = smiDD->getDiscreteRV(jj);

			        indx[jj] = 0;
			        nsamp[jj] = smiRV->getNumEvents();
			        ns *= nsamp[jj];
			        dp *= smiRV->getEventProb(indx[jj]);

			        //AJK -debug code for unitTest
			        drlo[irow[n_first_stg_rows + jj]] = demand[ index + indx[jj]
			        ];
			        drup[irow[n_first_stg_rows + jj]] = demand[ index + indx[jj]
			        ];
			        index += nsamp[jj];
			        //AJK -debug end


			        cpv_dclo.append(smiRV->getEventColLower(indx[jj]));

			        cpv_dcup.append(smiRV->getEventColUpper(indx[jj]));

			        cpv_dobj.append(smiRV->getEventObjective(indx[jj]));

			        cpv_drlo.append(smiRV->getEventRowLower(indx[jj]));

			        cpv_drup.append(smiRV->getEventRowUpper(indx[jj]));


			        CoinPackedMatrix m = smiRV->getEventMatrix(indx[jj]);
			        myAssert(__FILE__,__LINE__,!m.isColOrdered());
			        if (matrix.getNumElements())
			        {
				        for (int i=0; i<m.getNumRows(); ++i)
				        {
					        CoinPackedVector row=m.getVector(i);
					        CoinPackedVector rrow=matrix.getVector(i);
					        for (int j=m.getVectorFirst(i); j<m.getVectorLast(j); ++j)
					        {

						        myAssert(__FILE__,__LINE__,rrow[j] == 0.0);//tests duplicate index
						        matrix.modifyCoefficient(i,j,row[j],true);
					        }
				        }
			        }
			        else
				        matrix = m;


		        }

		        // first scenario
		        int anc = 0;
		        int branch = 1;
		        int	is = smiModel->generateScenario(core,&matrix,&cpv_dclo,&
			        cpv_dcup,&cpv_dobj,

			        &cpv_drlo,&cpv_drup,branch,anc,dp)
			        ;

		        // test first scenario

		        // load problem data into OsiSolver
		        smiModel->loadOsiSolverData();
		        // get Osi pointer
		        OsiSolverInterface *smiOsi1 = smiModel->getOsiSolverInterface();

		        int nStochCol = smiOsi1->getNumCols();
		        int nStochRow = smiOsi1->getNumRows();
		        double totalProb = dp;

		        // get arrays
		        const double *stochdrlo = smiOsi1->getRowLower();
		        const double *stochdrup = smiOsi1->getRowUpper();
		        const double *stochdclo = smiOsi1->getColLower();
		        const double *stochdcup = smiOsi1->getColUpper();
		        const double *stochdobj = smiOsi1->getObjCoefficients();

		        // get matrix
		        const CoinPackedMatrix *stochmat = smiOsi1->getMatrixByRow();
		        for (int t=0;t<3;t++)
		        {
			        double elt1,elt2;
			        int ic;
			        for(ii=core->getColStart(t);ii<core->getColStart(t+1);ii++)
			        {
				        ic = core->getColExternalIndex(ii);
				        elt1 = stochdclo[ii];
				        elt2 = dclo[ic];
				        myAssert(__FILE__,__LINE__,elt1==elt2);
				        elt1 = stochdcup[ii];
				        elt2 = dcup[ic];
				        myAssert(__FILE__,__LINE__,elt1==elt2);
				        elt1 = stochdobj[ii];
				        elt2 = dobj[ic];
				        myAssert(__FILE__,__LINE__,elt1==elt2);
			        }
			        int ir;
			        for(ii=core->getRowStart(t);ii<core->getRowStart(t+1);ii++)
			        {

				        ir = core->getRowExternalIndex(ii);

				        myAssert(__FILE__,__LINE__,stochdrlo[ii]==drlo[ir]);
				        myAssert(__FILE__,__LINE__,stochdrup[ii]==drup[ir]);

				        const CoinPackedVector row1 = stochmat->getVector(ii);
				        const CoinPackedVector row2 = origmat->getVector(ir);
				        myAssert(__FILE__,__LINE__,row1.getNumElements() == row2.
					        getNumElements());
				        const int *indx = row1.getIndices();
				        const double *els = row1.getElements();
				        for (int j=0; j<row1.getNumElements(); j++)
				        {
					        elt1 = els[j];
					        ic = core->getColExternalIndex(indx[j]);
					        elt2 = row2[ic];
					        myAssert(__FILE__,__LINE__,elt1==elt2);
				        }
			        }
		        }

		        printf(" *** Successfully tested problem with scenario %d.\n",is);



		        SmiTreeNode<SmiScnNode *> *root = smiModel->getSmiTree()->getRoot();
		        smiModel->getSmiTree()->setChildLabels(root,label);

		        /* sample space increment initialized to 1 */
		        int *incr = (int *) malloc( nindp*sizeof(int) );
		        for (jj=0;jj<nindp;jj++) incr[jj] = 1;

		        /***** ...main loop to generate scenarios from discrete random
		        variables
		        For each scenario index ii:
		        If the sample size nsamp[jj] divides the scenario index ii,
		        reverse the increment direction incr[jj]
		        and increase the random variable index jj by 1.
		        Increment the jj'th random variable by incr[jj]
		        and generate new sample data.
		        ***** */

		        for (int iss=1;iss<ns;iss++) {
			        int iii=iss; jj=0;
			        while ( !(iii%nsamp[jj]) ) {
				        iii /= nsamp[jj];
				        incr[jj] = -incr[jj];
				        jj++;
			        }

			        SmiDiscreteRV *smiRV = smiDD->getDiscreteRV(jj);

			        dp /= smiRV->getEventProb(indx[jj]);
			        indx[jj] += incr[jj];
			        dp *= smiRV->getEventProb(indx[jj]);

			        //AJK -debug code for unitTest
			        index = 0;


			        for (iLabel=label.begin(); iLabel<label.end(); ++iLabel)
				        *iLabel=0;

			        for (int jjj=0;jjj<smiDD->getNumRV();++jjj)
			        {
				        int nEvents = smiDD->getDiscreteRV(jjj)->getNumEvents();
				        int iStage = smiDD->getDiscreteRV(jjj)->getStage();

				        label[iStage] *= nEvents;
				        label[iStage] += indx[jjj];

				        drlo[irow[n_first_stg_rows + jjj]] = demand[ indx[jjj]+
					        index ];
				        drup[irow[n_first_stg_rows + jjj]] = demand[ indx[jjj]+
					        index ];
				        index += nsamp[jjj];

			        }
			        //AJK -debug end

			        // set data
			        //TODO -- should we declare NULL entries to have 0 entries?
			        //smiModel would eliminate these tests
			        replaceFirstWithSecond(cpv_dclo,smiRV->getEventColLower(indx[jj]));


			        replaceFirstWithSecond(cpv_dcup,smiRV->getEventColUpper(indx[jj]));

			        replaceFirstWithSecond(cpv_dobj,smiRV->getEventObjective(indx[jj]));

			        replaceFirstWithSecond(cpv_drlo,smiRV->getEventRowLower(indx[jj]));


			        replaceFirstWithSecond(cpv_drup,smiRV->getEventRowUpper(indx[jj]));


			        //TODO test smiModel code
			        CoinPackedMatrix m = smiRV->getEventMatrix(indx[jj]);
			        myAssert(__FILE__,__LINE__,!m.isColOrdered());
			        if (matrix.getNumElements())
			        {
				        for (int i=0; i<m.getNumRows(); ++i)
				        {
					        CoinPackedVector row=m.getVector(i);
					        CoinPackedVector rrow=matrix.getVector(i);
					        for (int j=m.getVectorFirst(i); j<m.getVectorLast(j);
						        ++j)
					        {
						        matrix.modifyCoefficient(i,j,row[j],true);
					        }
				        }
			        }
			        else
				        matrix = m;

			        // find ancestor node
			        SmiTreeNode<SmiScnNode *> *tnode = smiModel->getSmiTree()->find(
				        label);

			        // add scenario
			        anc = tnode->scenario();
			        myAssert(__FILE__,__LINE__,anc==0);
			        branch = tnode->depth();
			        myAssert(__FILE__,__LINE__,branch==1);
			        is = smiModel->generateScenario(core,&matrix,&cpv_dclo,&
				        cpv_dcup,&cpv_dobj,
				        &cpv_drlo,&cpv_drup,branch,anc,dp)
				        ;

			        smiModel->getSmiTree()->setChildLabels(tnode,label);

			        if (is < 3)
			        {
				        // test scenario

				        // load problem data into OsiSolver
				        smiModel->loadOsiSolverData();
				        // get Osi pointer
				        OsiSolverInterface *smiOsi1 = smiModel->
					        getOsiSolverInterface();

				        totalProb += dp;

				        // get arrays
				        stochdrlo = smiOsi1->getRowLower()+nStochRow;
				        stochdrup = smiOsi1->getRowUpper()+nStochRow;
				        stochdclo = smiOsi1->getColLower()+nStochCol;
				        stochdcup = smiOsi1->getColUpper()+nStochCol;
				        stochdobj = smiOsi1->getObjCoefficients()+nStochCol;

				        // get matrix
				        const CoinPackedMatrix *stochmat = smiOsi1->getMatrixByRow
					        ();
				        int t;
				        for (t=1;t<3;t++)
				        {
					        double elt1,elt2;
					        int ic;
					        int colOff = core->getColStart(1);
					        for(ii=core->getColStart(t);ii<core->getColStart(t+1);
						        ii++)
					        {
						        ic = core->getColExternalIndex(ii);
						        elt1 = stochdclo[ii-colOff];
						        elt2 = dclo[ic];
						        myAssert(__FILE__,__LINE__,elt1==elt2);
						        elt1 = stochdcup[ii-colOff];
						        elt2 = dcup[ic];
						        myAssert(__FILE__,__LINE__,elt1==elt2);
						        elt1 = stochdobj[ii-colOff];
						        elt2 = dobj[ic];
						        myAssert(__FILE__,__LINE__,fabs(elt1 - (elt2*dp/
							        totalProb)) < 1.0e-8);
					        }
					        int ir,rowOff;
					        rowOff = core->getRowStart(1);
					        for(ii=core->getRowStart(t);ii<core->getRowStart(t+1);
						        ii++)
					        {

						        ir = core->getRowExternalIndex(ii);

						        myAssert(__FILE__,__LINE__,stochdrlo[ii-rowOff]==
							        drlo[ir]);
						        myAssert(__FILE__,__LINE__,stochdrup[ii-rowOff]==
							        drup[ir]);

						        const CoinPackedVector row1 = stochmat->getVector(
							        ii);
						        const CoinPackedVector row2 = origmat->getVector(
							        ir);
						        myAssert(__FILE__,__LINE__,row1.getNumElements()
							        == row2.getNumElements());
						        const int *indx = row1.getIndices();
						        const double *els = row1.getElements();
						        for (int j=0; j<row1.getNumElements(); j++)
						        {
							        elt1 = els[j];
							        ic = core->getColExternalIndex(indx[j]);
							        elt2 = row2[ic];
							        myAssert(__FILE__,__LINE__,elt1==elt2);
						        }
					        }
				        }

				        nStochCol = smiOsi1->getNumCols();
				        nStochRow = smiOsi1->getNumRows();

				        printf(" *** Successfully tested problem with scenario %d.\n",is);
			        }

		        }

		        free( incr) ;
		        free( drlo) ;
		        free( drup) ;
		        free( rstg) ;
		        free( dclo) ;
		        free( dcup) ;
		        free( cstg) ;

	        } else { // if (0)
		        free( drlo) ;
		        free( drup) ;
		        free( rstg) ;
		        free( dclo) ;
		        free( dcup) ;
		        free( cstg) ;
	        }


	        // solve with decomp solver

	        // load problem data into OsiSolver
	        smiModel->loadOsiSolverData();
	        // get Osi pointer
	        OsiSolverInterface *smiOsi = smiModel->getOsiSolverInterface();
	        // set some parameters
	        smiOsi->setHintParam(OsiDoPresolveInInitial,true);
	        smiOsi->setHintParam(OsiDoScale,true);
	        smiOsi->setHintParam(OsiDoCrash,true);
	        // solve using Osi Solver
	        smiOsi->initialSolve();
	        // test optimal value
	        myAssert(__FILE__,__LINE__,fabs(smiOsi->getObjValue()-1566.042)<
		        0.01);

	        // test solutions
	        const double *dsoln = smiOsi->getColSolution();
	        double objSum = 0.0;

	        /* The canonical way to traverse the tree:
	        For each scenario, get the leaf node.
	        Then get the parent.  Repeat until parent is NULL.
	        (Only the root node has a NULL parent.)
	        */
	        for(int is=0; is<smiModel->getNumScenarios(); ++is)
	        {
		        /* this loop calculates the scenario objective value */
		        double scenSum = 0.0;

		        // start with leaf node
		        SmiScnNode *node = smiModel->getLeafNode(is);

		        // leaf node probability is the scenario probability
		        double scenprob = node->getModelProb();

		        while (node != NULL)
		        {

			        // getColStart returns the starting index of node in OSI model
			        for(int j=node->getColStart();
				        j<node->getColStart()+node->getNumCols(); ++j)
			        {
				        // getCoreColIndex returns the corresponding Core index
				        // in the original (user's) ordering
				        scenSum += dobj[node->getCoreColIndex(j)]*dsoln[j];


			        }
			        // get parent of node
			        node = node->getParent();
		        }
		        objSum += scenSum*scenprob;
	        }
	        myAssert(__FILE__,__LINE__,fabs(smiOsi->getObjValue()-objSum) <	0.01);

	        printf(" *** Completed all tests for SmiScnModel.\n");
	        delete smiModel;
	        delete osiClp1;
	        delete smiCore;
	        delete cpm_mat;
	        delete smiDD;
	        delete origmat;

	        free( dobj) ;
	        free( mrow) ;
	        free( mcol) ;
        }


        TEST_F(SmiUnitTest, ModelBug) {
	        OsiClpSolverInterface osi;
	        double INF=osi.getInfinity();

	        /*
	        NAME    BUG
	        ROWS
	        N  obj
	        G  C0
	        G  C1
	        G  C2
	        G  C3
	        */
	        int nCoreRows=4;
	        double dCoreRup[] = { INF, INF, INF, INF };
	        /*
	        COLUMNS
	        x01   obj   1
	        x01   C3    1
	        x01   C1    1
	        x01   C0    1
	        x02   obj   1
	        x02   C2    1
	        x02   C1    1
	        x02   C0    1
	        x03   obj   1
	        x03   C3    1
	        x03   C2    1
	        x03   C0    1
	        x04   obj   0.5
	        x04   C3    1
	        x04   C1    1
	        x05   obj   0.5
	        x05   C2    1
	        x05   C1    1
	        x06   obj   0.5
	        x06   C3    1
	        x06   C2    1
	        */
	        int nCoreCols=6;
	        double dCoreClo[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	        double dCoreCup[] = {INF, INF, INF, INF, INF, INF};
	        double dCoreObj[] = {1.0, 1.0, 1.0, 0.5, 0.5, 0.5};

	        //int nCoreElts=15;
	        int iCoreColStarts[] = {0,
		        3,
		        6,
		        9,
		        11,
		        13,
		        15};
	        int iCoreRowIndice[] = {3,1,0,
		        2,1,0,
		        3,2,0,
		        3,1,
		        2,1,
		        3,2};
	        double dCoreMatEntries[] = {1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0,
		        1.0, 1.0, 1.0,
		        1.0, 1.0,
		        1.0, 1.0,
		        1.0, 1.0 };

	        /*
	        RHS
	        RHS    C0    0
	        RHS    C1    1
	        RHS    C2    1
	        RHS    C3    1
	        ENDATA
	        */
	        double dCoreRlo[] = { 0.0, 1.0, 1.0, 1.0 };
	        /*
	        -----------------------------------------------------------
	        bug.time:

	        TIME          BUG
	        PERIODS       LP
	        x01       C0                      STG01
	        x04       C1                      STG02
	        ENDATA
	        -----------------------------------------------------------
	        */
	        int nCoreStages = 2;
	        int iColStages[] = {0,0,0,1,1,1};
	        int iRowStages[] = {0,1,1,1};

	        /*
	        ----------------------------------------------------------
	        bug.stoch file:

	        NAME          BUG
	        SCENARIOS     DISCRETE                REPLACE
	        SC SCEN01    ROOT           0.500    STG02
	        RHS       C1             1.000
	        RHS       C2             1.000
	        RHS       C3             0.000
	        SC SCEN02    ROOT           0.500    STG02
	        RHS       C1             0.000
	        RHS       C2             1.000
	        RHS       C3             0.000
	        ENDATA
	        */

	        //int nScenarios = 2;
	        int iBranchStage[] = {1,1};

	        int iAncestorScn[] = {0,0};
	        double dProbScn[] = {0.5, 0.5};

	        double drlo0[] = {1.0, 1.0, 0.0 };
	        int indices[]  = {1, 2, 3};
	        CoinPackedVector *rlo0 = new CoinPackedVector(3, indices,drlo0);
	        double drlo1[] = {0.0, 1.0, 0.0 };
	        CoinPackedVector *rlo1 = new CoinPackedVector(3, indices,drlo1);

	        // generate Core model

	        osi.loadProblem(nCoreCols,
		        nCoreRows,iCoreColStarts,iCoreRowIndice,dCoreMatEntries,dCoreClo,
		        dCoreCup,dCoreObj,
		        dCoreRlo,dCoreRup);
	        SmiCoreData *osiCore = new SmiCoreData(&osi,nCoreStages,iColStages,iRowStages);

	        // initialize SmiScnModel
	        SmiScnModel *smiModel = new SmiScnModel();
            smiModel->setCore(osiCore);

	        // Add Scenarios
	        int	is = smiModel->generateScenario(osiCore,NULL,NULL,NULL,NULL,
		        rlo0,NULL,iBranchStage[0],
		        iAncestorScn[0],dProbScn[0]);

	        is = smiModel->generateScenario(osiCore,NULL,NULL,NULL,NULL,
		        rlo1,NULL,iBranchStage[1],
		        iAncestorScn[1],dProbScn[1]);

	        // Set Solver
	        OsiClpSolverInterface osiClp;
	        smiModel->setOsiSolverHandle(osiClp);

	        // Load Scenarios into Deterministic Equivalent
	        smiModel->loadOsiSolverData();

	        // Get Det Equiv OSI
	        OsiSolverInterface *osiStoch = smiModel->getOsiSolverInterface();

	        // Solve
	        osiStoch->initialSolve();

	        // print results
	        printf("Solved stochastic program %s\n", "BUG");
	        printf("Number of rows: %d\n",osiStoch->getNumRows());
	        printf("Number of cols: %d\n",osiStoch->getNumCols());
	        printf("Optimal value: %g\n",osiStoch->getObjValue());

	        myAssert(__FILE__,__LINE__,osiStoch->getObjValue()== 0.5);

	        delete smiModel;
	        delete rlo0;
	        delete rlo1;
        }


        TEST_F(SmiUnitTest, SmpsBug) {
	        SmiScnModel smi;

#ifdef _MSC_VER
	        std::string dataDir="../../../../Smi/test/SmiTestData";
#else
            std::string dataDir="../../Smi/test/SmiTestData";
#endif
	        dataDir += "/bug";

	        // read SMPS model from files
	        // <name>.core, <name>.time, and <name>.stoch
	        // the argument myCombineRule overrides the combine rule specified in the Stoch file
	        myAssert(__FILE__,__LINE__,-1!=smi.readSmps(dataDir.c_str()));

	        // generate OSI solver object
	        // 	here we use OsiClp
	        OsiClpSolverInterface *clp = new OsiClpSolverInterface();

	        // set solver object for SmiScnModel
	        smi.setOsiSolverHandle(*clp);

	        // load solver data
	        // 	this step generates the deterministic equivalent
	        //	and returns an OsiSolver object
	        OsiSolverInterface *osiStoch = smi.loadOsiSolverData();

	        // solve
	        osiStoch->initialSolve();

	        // print results
	        printf("Solved stochastic program Bug\n");
	        printf("Number of rows: %d\n",osiStoch->getNumRows());
	        printf("Number of cols: %d\n",osiStoch->getNumCols());
	        printf("Optimal value: %g\n",osiStoch->getObjValue());
	        myAssert(__FILE__,__LINE__,osiStoch->getObjValue()== 0.5);

	        delete clp;
        }

    }

    TEST_F(UseCaseTest,Atilla){


        //OsiClpSolverInterface clpSolver;
        MP_model model(new OsiClpSolverInterface);



        enum {numStage=2};

        enum {numSupplier=4};

        enum {numDiscounts=3};

        enum {numScen=8};



        MP_set supplier(numSupplier);
        supplier.setName("supplier");

        MP_set discount(numDiscounts);
        discount.setName("discount");



        MP_stage T(numStage);
        T.setName("stage");



        MP_variable q_fix(supplier,discount);
        q_fix.setName("q_fix");

        MP_variable q_flex(supplier,T);
        q_flex.setName("q_flex");

        MP_variable o(supplier);
        o.setName("o");

        MP_variable y(supplier);
        y.setName("y");

        MP_variable x(supplier,discount);
        x.setName("x");


        y.binary();
        x.binary();





        MP_constraint optionsConstraint(supplier),

            capacityConstraint(supplier),

            //discount1Constraint(supplier),

            discountConstraint(supplier,discount),

            demandConstraint(T);


        MP_data B(supplier);

        MP_data Q(supplier,discount);

        MP_data capacity(supplier);

        MP_data F(supplier);

        MP_data P(supplier,discount);

        MP_data R(supplier);

        MP_data SP(supplier);



        double B_data[4]={100,100,100,100};

        B.value(&B_data[0]);


        double Q_data[4][3]={{0,100,200},{0,100,200},{0,100,200},{0,100,200}};

        Q.value(&Q_data[0][0]);


        double capa_data[4]={250,250,250,250};

        capacity.value(&capa_data[0]);


        double F_data[4]={100,100,100,100};

        F.value(&F_data[0]);



        double P_data[4][3]={{100,98,95},{100,98,95},{100,98,95},{100,98,95}};

        P.value(&P_data[0][0]);



        double R_data[4]={13,13,13,13};

        R.value(&R_data[0]);



        double SP_data[4]={92,92,92,92};

        SP.value(&SP_data[0]);


        std::vector<double> probs;
        std::vector<double> values;

        probs.push_back(0.95);
        probs.push_back(0.05);
        values.push_back(1);
        values.push_back(0);



        RandomVariable* supply[numStage-1][numSupplier] = {
            
            { new EmpiricalRandomVariable(values,probs),

            new EmpiricalRandomVariable(values,probs),

            new EmpiricalRandomVariable(values,probs),

            new EmpiricalRandomVariable(values,probs)}
        };



        MP_random_data S(&supply[0][0],T,supplier);

        std::vector<double> probdemand;
        std::vector<double> valuedemand;

        probdemand.push_back(1);
        /*probdemand.push_back(0.5);*/
        //probdemand.push_back(0.5);
        valuedemand.push_back(80);
        //valuedemand.push_back(120);

        RandomVariable* demand[1] = { new EmpiricalRandomVariable(valuedemand,probdemand)};

        MP_random_data D(&demand[0],T);


        //optionsConstraint(supplier)=o(supplier)<=B(supplier)*y(supplier);

        //capacityConstraint(supplier)=sum(discount,q_fix(supplier,discount))+o(supplier)<=capacity(supplier)*y(supplier);

        ////discount1Constraint(supplier)

        //discountConstraint(supplier,discount)=q_fix(supplier,discount)>=Q(supplier,discount)*x(supplier,discount);

        demandConstraint(T+1)=sum(supplier,S(T+1,supplier)*(sum(discount,q_fix(supplier,discount))+q_flex(supplier,T+1)))==maximum(supplier,S(T+1,supplier))*D(T+1);





        MP_expression expectedCosts(sum(supplier,
            //F(supplier)*y(supplier)

            //+R(supplier)*o(supplier)

            S(T+1,supplier)*(sum(discount,P(supplier,discount)*q_fix(supplier,discount)) +SP(supplier)*q_flex(supplier,T+1))
            ));

        model.setObjective(expectedCosts);

        model.attach(model.Solver);
        SmiScnModel* smiPtr = model.getSmi();
        smiPtr->writeSmps("Atilla",false,false);
        smiPtr->setOsiSolverHandle(model.Solver);
        // Get Deterministic equivalent
        OsiSolverInterface* osiPtr = smiPtr->loadOsiSolverData();
        osiPtr->writeLp("Atilla");
        model.solve(MP_model::MINIMIZE);



    }

} //end namespace

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*.Atilla";
    //::testing::GTEST_FLAG(filter) = "*.AdditiveConstantSamplingTest";
    //::testing::GTEST_FLAG(filter) = "*.FeasibilityCutTest";
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

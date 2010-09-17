#define BOOST_TR1_TUPLE_HPP_INCLUDED //This prevents compilation issues.
#include <gtest/gtest.h>
#include <iostream>
#include <ctime>
#include <limits>

#include <glog/logging.h>

#include "OsiClpSolverInterface.hpp"
#include "OsiCbcSolverInterface.hpp"
#include "CoinFloatEqual.hpp"
#include "flopc.hpp"
#include "SmiScnModel.hpp"
#include "Benders.hpp"
#include "SAA.hpp"
#include "SmiSmpsIO.hpp"
//#include "OsiStochasticSolverInterface.hpp"

//#include <vld.h> //uncomment to enable Visual Leak Detector
//TODO: Test about correct colOrdering that is given to the solver..
//TODO: Test scenario tree generation for RV that are not for themselves part of the model. Maybe this is already working, maybe not..

//using namespace flopc;
namespace flopc {
    void writeReadSMPSTest(MP_model& model, MP_model::MP_direction direction, const std::string& filename){
        // write SMPS files
        model.smiModel->writeSmps(filename.c_str());        
        
        // read SMPS files        
        SmiScnModel smi;
        DLOG(INFO) << "SMPS reader: " << smi.readSmps(filename.c_str()) << "\n";
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
        ASSERT_EQ(0,remove( (filename+".core").c_str() ));
        ASSERT_EQ(0,remove( (filename+".time").c_str() ));
        ASSERT_EQ(0,remove( (filename+".stoch").c_str() ));
        delete clp;
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

        //OsiStochasticBendersTwoStageSolverInterface benders(testModel.Solver,testModel.getSmi());
        ////double begin = CoinCpuTime();
        //benders.initialSolve();
        //double duration = CoinCpuTime() - begin;
    }

    TEST_F(MP_modelTest,DiscretizationTest){
        MP_model testModel(new OsiClpSolverInterface(), new VerboseMessenger());
        MP_stage T(2);
        MP_random_data rvar(T);
        rvar(1) = new ContinuousNormalRandomVariable(0,1, new BracketMeanDiscretization(10));
        rvar(1)->setStage(1);
        //rvar2(1) = new ContinuousNormalRandomVariable(0,1, new BracketMedianDiscretization(5));
        //rvar2(1)->setStage(1);
        MP_variable x(T);
        MP_constraint con(T);
        con(T) = x(T) <= rvar(T);
        testModel.setObjective( x(T.last()) );
        testModel.solve(MP_model::MAXIMIZE);
        rvar(1) = new ContinuousNormalRandomVariable(0,1, new BracketMedianDiscretization(10));
        rvar(1)->setStage(1);
        testModel.setObjective( x(T.last()) );
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);
        rvar(1) = new ContinuousExponentialRandomVariable(1, new BracketMedianDiscretization(10));
        rvar(1)->setStage(1);
        testModel.setObjective( x(T.last()) );
        testModel.attach();
        testModel.solve(MP_model::MAXIMIZE);

        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");


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
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_DOUBLE_EQ(5,testModel.Solver->getObjValue());
        y.display("y");
        rVar.display("RandomVars");
        combined.display("CombinedData");

        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");
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

        double dbvalues[2][4] = { {1,2,3,4 }, {5,6,7,8 }}; 

        MP_random_data dB(dbvalues[0,0],T);

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
        RandomVariable* scenarioValues[2] = { new DiscreteUniformRandomVariable(0,10,11),new DiscreteUniformRandomVariable(0,10,11)};
        RandomVariable* scenarioValues1[2] = { new DiscreteUniformRandomVariable(10,20,11),new DiscreteUniformRandomVariable(10,20,11)};
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
         writeReadSMPSTest(investmentModel,MP_model::MAXIMIZE,"test");

        //for (int i = 0; i < scen.size();i++){
        //    for (int j = 0; j < T.size(); j++){
        //        cout << "scenario " << i << " stufe " << j << " wealth: " << wealth.levelScenario(i,j) << endl;
        //    }
        //}

        ////Compare solution values.
        EXPECT_NEAR(4.142141441,investmentModel.Solver->getObjValue(),0.000001);
        EXPECT_EQ(MP_model::OPTIMAL,investmentModel.getStatus());
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
        writeReadSMPSTest(testModel,MP_model::MAXIMIZE,"test");

        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_DOUBLE_EQ(52,testModel->getObjValue());
    }


    TEST_F(IntegerTest,IntegerTest2){
        MP_model intModel(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        double scenarios[1][2] =
        { // T
            {1,2} //scen
        };
        MP_random_data rvar(&scenarios[0][0],T);
        MP_variable y(T), x(T);
        y.integer();
        x.integer();
        MP_constraint constraint1(T);
        constraint1(T) = y(T) + x(T) >= 1.5 + rvar(T);
        intModel.setObjective(sum(T, y(T)));
        intModel.attach(intModel.Solver);
        this->testCorrectStageAssignment(intModel);
        intModel.solve(MP_model::MINIMIZE);
        writeReadSMPSTest(intModel,MP_model::MINIMIZE,"test");
        const double * colSolution = intModel->getColSolution();
        for (int i = 0; i < intModel->getNumCols(); i++)
            cout << "Column " << i << ": " << colSolution[i] << endl;
        y.display("y");
        x.display("x");
        cout << "Finished.";
        EXPECT_EQ(MP_model::OPTIMAL,intModel.getStatus());
        EXPECT_EQ(3,x.levelScenario(0,1));
        EXPECT_EQ(4,x.levelScenario(1,1));
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

        ev = dacota.smiModel->solveEV(dacota.Solver, MP_model::MAXIMIZE);
        EXPECT_EQ(ev, 4165);

        eev = dacota.smiModel->solveEEV(dacota.Solver, MP_model::MAXIMIZE);
        EXPECT_NEAR(eev, 1545, 0.0001);

        ws = dacota.smiModel->getWSValue(dacota.Solver, MP_model::MAXIMIZE);
        EXPECT_EQ(ws, 4165);

        // we have to attach the solver again, otherwise the program crashes
        dacota.attach(dacota.Solver);
        dacota.solve(MP_model::MAXIMIZE);
        writeReadSMPSTest(dacota,MP_model::MAXIMIZE,"test");
        hereAndNow = dacota->getObjValue();
        EXPECT_DOUBLE_EQ(1730.0, hereAndNow);
        EXPECT_EQ(MP_model::OPTIMAL,dacota.getStatus());
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
    }

    TEST_F(MP_modelTest,InvalidArgumentTest){
        MP_model testModel(new OsiClpSolverInterface()); // Without Solver
        ASSERT_THROW(MP_random_data();,invalid_argument_exception);

    }

    TEST(UnitTest,OldUnitTest){
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

        assert(m1->getNumRows()==11);
        assert(m1->getNumCols()==47);
        assert(m1->getNumElements()==96);
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

        assert(m2->getNumRows()==56);
        assert(m2->getNumCols()==72);
        assert(m2->getNumElements()==219);

        // Optimal objective value m1: 1566.04218913
        // Optimal objective value m2: 1566.04218913 (like m1)
        EXPECT_GE(m2->getObjValue(),1566.03);
        EXPECT_LE(m2->getObjValue(),1566.05);

        EXPECT_DOUBLE_EQ( 1566.042189132706,m2->getObjValue());
        EXPECT_DOUBLE_EQ( 1566.0421891327067,m1->getObjValue());
        y.display("y second model");
    }

    TEST_F(MP_modelTest,TimosORBProjekt){
        MP_model mod(new OsiCpxSolverInterface());

        enum { numST=2};
        MP_stage ST(numST);

        //enum { numScen=10};
        enum { numScen=1};
        MP_scenario_set scen(numScen);

        enum { FRA, HAM, MUC, DRS, HAJ, STR, IAD, CHI, BOS, PHL, CLT, CVG, numA};
        MP_set A(numA);
        MP_set A2(numA);
        MP_set A3(numA);
        MP_set A4(numA);

        enum { A320, A340, A380, numT};
        MP_set T(numT);

        double distance[numA][numA] =
        {{0,413,300,388,282,157,6545,6964,5884,6334,7052,6995},
        {413,0,600,378,132,553,6469,6827,5814,6262,6982,6886},
        {300,600,0,340,480,194,6838,7264,6177,6627,7345,7292},
        {388,378,340,0,317,425,6821,7196,6163,6612,7332,7248},
        {282,132,480,317,0,421,6504,6882,5847,6295,7016,6932},
        {157,553,194,425,421,0,6652,7087,5990,6440,7158,7110},
        {6545,6469,6838,6821,6504,6652,0,931,663,216,516,632},
        {6964,6827,7264,7196,6882,7087,931,0,1381,1075,949,410},
        {5884,5814,6177,6163,5847,5990,663,1381,0,450,1169,1207},
        {6334,6262,6627,6612,6295,6440,216,1075,450,0,720,813},
        {7052,6982,7345,7332,7016,7158,516,949,1169,720,0,539},
        {6995,6886,7292,7248,6932,7110,632,410,1207,813,539,0}};
        MP_data dist(&distance[0][0], A, A);

        MP_data p(A, A);
        forall(A*A2, p(A,A2) = dist(A,A2) * 0.08);

        MP_data dcf;
        dcf() = 1.0;

        MP_data pd;
        pd() = 0.1;

        double maxCapacity[numT] = { 150, 250, 500 };
        MP_data m(&maxCapacity[0], T);

        double kerosinConsumption[numT] = { 6.75, 10, 17.5 };
        MP_data cons(&kerosinConsumption[0], T);

        double costPerKM[numT] = { 10.125, 11.25, 18 };
        MP_data cpm(&costPerKM[0], T);

        double maxRange[numT] = { 5500, 14000, 14000 };
        MP_data r(&maxRange[0], T);

        double maxFlights[numT] = { 1e10, 1e10, 1e10 };
        MP_data ub(&maxFlights[0], T);

        double stochasticDemand[numST-1][numA][numA][numScen] =
        {
            {
                {
                    {0},
                    {1638},
                    {1344},
                    {416},
                    {163},
                    {102},
                    {614},
                    {444},
                    {375},
                    {204},
                    {139},
                    {42}
                },
                {
                    {1680},
                    {0},
                    {1708},
                    {88},
                    {10},
                    {1806},
                    {54},
                    {47},
                    {49},
                    {8},
                    {7},
                    {0}
                },
                {
                    {919},
                    {2444},
                    {0},
                    {226},
                    {657},
                    {30},
                    {210},
                    {234},
                    {132},
                    {97},
                    {81},
                    {19}
                },
                {
                    {566},
                    {101},
                    {326},
                    {0},
                    {0},
                    {242},
                    {10},
                    {8},
                    {10},
                    {0},
                    {0},
                    {0}
                },
                {
                    {237},
                    {8},
                    {688},
                    {0},
                    {0},
                    {350},
                    {26},
                    {16},
                    {19},
                    {8},
                    {11},
                    {0}
                },
                {
                    {87},
                    {1859},
                    {56},
                    {339},
                    {273},
                    {0},
                    {39},
                    {38},
                    {42},
                    {7},
                    {12},
                    {0}
                },
                {
                    {657},
                    {42},
                    {223},
                    {11},
                    {23},
                    {45},
                    {0},
                    {3880},
                    {4676},
                    {62},
                    {562},
                    {391}
                },
                {
                    {509},
                    {48},
                    {238},
                    {10},
                    {13},
                    {35},
                    {2504},
                    {0},
                    {2878},
                    {3263},
                    {1011},
                    {602}
                },
                {
                    {329},
                    {61},
                    {160},
                    {9},
                    {18},
                    {36},
                    {4215},
                    {3012},
                    {0},
                    {684},
                    {1381},
                    {424}
                },
                {
                    {287},
                    {10},
                    {84},
                    {0},
                    {10},
                    {11},
                    {51},
                    {3183},
                    {644},
                    {0},
                    {505},
                    {393}
                },
                {
                    {205},
                    {11},
                    {91},
                    {0},
                    {12},
                    {9},
                    {501},
                    {1076},
                    {1290},
                    {616},
                    {0},
                    {128}
                },
                {
                    {49},
                    {0},
                    {14},
                    {0},
                    {0},
                    {0},
                    {425},
                    {649},
                    {461},
                    {472},
                    {106},
                    {0}
                }
            }
        };
        //{
        // {
        //  {
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {1638, 1787, 2027, 1906, 1771, 1702, 1809, 1678, 1605, 1526},
        //   {1344, 715, 1022, 738, 1278, 1272, 767, 1191, 1145, 1051},
        //   {416, 336, 501, 467, 459, 415, 351, 574, 488, 455},
        //   {163, 111, 187, 193, 189, 149, 162, 138, 168, 153},
        //   {102, 117, 83, 106, 111, 82, 98, 111, 143, 105},
        //   {614, 523, 628, 423, 574, 561, 550, 627, 486, 476},
        //   {444, 608, 596, 337, 630, 574, 626, 487, 618, 594},
        //   {375, 238, 393, 440, 344, 313, 371, 292, 399, 319},
        //   {204, 275, 254, 185, 315, 343, 306, 267, 318, 346},
        //   {139, 206, 195, 237, 240, 217, 237, 209, 135, 188},
        //   {42, 54, 51, 39, 46, 45, 42, 67, 40, 42}
        //  },
        //  {
        //   {1680, 1076, 1385, 1924, 1810, 1971, 1761, 1342, 2094, 1657},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {1708, 1809, 1727, 1345, 1788, 1788, 2752, 2618, 1990, 1777},
        //   {88, 79, 66, 86, 81, 107, 84, 90, 60, 69},
        //   {10, 11, 8, 8, 10, 11, 12, 12, 10, 12},
        //   {1806, 2066, 1966, 1062, 1362, 1666, 1765, 1668, 1441, 1540},
        //   {54, 42, 39, 36, 27, 50, 48, 41, 32, 30},
        //   {47, 44, 51, 37, 57, 61, 51, 59, 57, 63},
        //   {49, 40, 57, 62, 46, 42, 51, 54, 52, 46},
        //   {8, 11, 10, 7, 10, 10, 13, 10, 10, 9},
        //   {7, 9, 11, 12, 13, 10, 12, 12, 9, 11},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        //  },
        //  {
        //   {919, 1252, 1162, 797, 1316, 917, 1150, 1064, 974, 976},
        //   {2444, 1875, 1929, 2230, 1882, 1978, 1977, 2121, 1640, 1506},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {226, 311, 296, 264, 239, 271, 277, 254, 236, 186},
        //   {657, 562, 617, 707, 633, 670, 639, 589, 573, 683},
        //   {30, 53, 44, 31, 25, 44, 41, 40, 44, 31},
        //   {210, 288, 221, 179, 196, 265, 190, 237, 216, 165},
        //   {234, 200, 243, 279, 195, 191, 204, 292, 215, 156},
        //   {132, 145, 153, 168, 232, 259, 167, 216, 182, 147},
        //   {97, 71, 95, 78, 87, 90, 88, 86, 104, 103},
        //   {81, 99, 110, 102, 82, 104, 100, 86, 120, 85},
        //   {19, 28, 20, 23, 15, 22, 18, 17, 27, 21}
        //  },
        //  {
        //   {566, 461, 406, 421, 484, 560, 439, 455, 512, 504},
        //   {101, 89, 97, 76, 110, 96, 90, 102, 108, 107},
        //   {326, 246, 319, 294, 260, 252, 135, 285, 221, 243},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {242, 312, 293, 255, 314, 248, 401, 312, 284, 381},
        //   {10, 10, 10, 9, 9, 9, 8, 9, 9, 13},
        //   {8, 5, 10, 8, 7, 10, 14, 11, 9, 9},
        //   {10, 9, 10, 8, 7, 12, 11, 9, 12, 11},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        //  },
        //  {
        //   {237, 155, 140, 129, 159, 165, 129, 189, 178, 131},
        //   {8, 9, 11, 6, 13, 10, 9, 11, 9, 12},
        //   {688, 539, 768, 613, 545, 567, 544, 428, 589, 713},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {350, 347, 298, 364, 249, 264, 312, 339, 304, 342},
        //   {26, 19, 20, 24, 23, 16, 18, 21, 21, 18},
        //   {16, 21, 19, 18, 25, 19, 25, 18, 16, 14},
        //   {19, 21, 26, 15, 19, 27, 20, 16, 20, 28},
        //   {8, 12, 10, 8, 8, 8, 6, 10, 10, 11},
        //   {11, 9, 10, 10, 9, 10, 10, 11, 10, 11},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        //  },
        //  {
        //   {87, 88, 85, 80, 108, 99, 98, 94, 77, 87},
        //   {1859, 1427, 2079, 1467, 1736, 1681, 1317, 1475, 1953, 1445},
        //   {56, 39, 43, 39, 38, 37, 42, 39, 51, 44},
        //   {339, 229, 391, 342, 365, 310, 408, 318, 388, 338},
        //   {273, 357, 331, 257, 246, 407, 356, 328, 368, 335},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {39, 40, 39, 45, 49, 46, 39, 45, 41, 28},
        //   {38, 40, 36, 58, 21, 39, 47, 32, 34, 27},
        //   {42, 38, 46, 43, 38, 42, 49, 40, 37, 42},
        //   {7, 13, 9, 9, 10, 8, 12, 11, 12, 12},
        //   {12, 9, 11, 9, 9, 10, 8, 11, 6, 9},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        //  },
        //  {
        //   {657, 551, 548, 536, 565, 469, 547, 610, 504, 452},
        //   {42, 49, 48, 33, 29, 35, 32, 34, 44, 47},
        //   {223, 258, 184, 185, 174, 240, 175, 253, 230, 230},
        //   {11, 10, 8, 10, 7, 9, 7, 10, 8, 10},
        //   {23, 21, 26, 12, 18, 17, 18, 21, 23, 17},
        //   {45, 50, 37, 33, 38, 44, 40, 55, 27, 27},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {3880, 3354, 3629, 3946, 4133, 3550, 4845, 4952, 3305, 3533},
        //   {4676, 3599, 4146, 3126, 3065, 3574, 3729, 2236, 3090, 3713},
        //   {62, 68, 66, 58, 54, 60, 67, 84, 60, 68},
        //   {562, 599, 692, 835, 535, 588, 614, 625, 607, 820},
        //   {391, 296, 498, 391, 441, 466, 422, 293, 544, 372}
        //  },
        //  {
        //   {509, 458, 724, 631, 468, 777, 461, 597, 643, 474},
        //   {48, 63, 51, 52, 45, 55, 52, 41, 45, 59},
        //   {238, 198, 243, 145, 141, 228, 282, 250, 226, 268},
        //   {10, 11, 12, 11, 11, 8, 13, 12, 8, 10},
        //   {13, 16, 17, 27, 17, 16, 26, 16, 16, 23},
        //   {35, 40, 54, 46, 41, 40, 54, 39, 50, 56},
        //   {2504, 3640, 3387, 4088, 4148, 2662, 4021, 4480, 4313, 3376},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {2878, 3339, 3341, 3383, 2879, 3786, 2533, 2947, 2427, 3325},
        //   {3263, 3834, 3670, 3325, 2903, 2192, 2637, 2624, 2911, 2931},
        //   {1011, 1026, 1086, 941, 976, 1084, 1381, 998, 1140, 1204},
        //   {602, 663, 612, 761, 836, 713, 629, 596, 507, 784}
        //  },
        //  {
        //   {329, 389, 379, 361, 236, 488, 356, 345, 371, 211},
        //   {61, 45, 33, 53, 33, 40, 30, 34, 65, 66},
        //   {160, 214, 174, 180, 201, 168, 130, 147, 223, 157},
        //   {9, 11, 13, 10, 10, 10, 12, 8, 11, 7},
        //   {18, 24, 20, 20, 16, 24, 14, 23, 23, 20},
        //   {36, 31, 47, 47, 33, 49, 29, 46, 38, 40},
        //   {4215, 2959, 3499, 4150, 4701, 2979, 3667, 4638, 4952, 4329},
        //   {3012, 2899, 3170, 3047, 3007, 3125, 3702, 2682, 3449, 2138},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {684, 651, 835, 742, 560, 786, 857, 575, 475, 692},
        //   {1381, 1257, 911, 671, 1065, 812, 942, 1212, 1089, 1311},
        //   {424, 496, 438, 452, 488, 440, 421, 467, 464, 462}
        //  },
        //  {
        //   {287, 384, 355, 294, 284, 272, 330, 310, 269, 349},
        //   {10, 8, 8, 8, 9, 9, 8, 12, 10, 10},
        //   {84, 75, 71, 122, 94, 100, 104, 92, 101, 82},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {10, 12, 11, 9, 9, 8, 11, 10, 12, 8},
        //   {11, 10, 13, 14, 12, 8, 9, 10, 13, 8},
        //   {51, 59, 57, 54, 58, 68, 54, 61, 51, 75},
        //   {3183, 2105, 2288, 3816, 3568, 3065, 1832, 3073, 2561, 2987},
        //   {644, 585, 689, 726, 818, 790, 568, 690, 746, 610},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {505, 435, 373, 453, 387, 575, 503, 442, 500, 596},
        //   {393, 246, 407, 376, 288, 331, 423, 450, 446, 312}
        //  },
        //  {
        //   {205, 245, 244, 224, 155, 253, 252, 175, 200, 200},
        //   {11, 10, 8, 11, 9, 9, 9, 12, 11, 9},
        //   {91, 73, 102, 91, 114, 97, 96, 82, 72, 88},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {12, 10, 11, 10, 9, 11, 13, 8, 10, 8},
        //   {9, 10, 11, 11, 8, 14, 9, 7, 9, 12},
        //   {501, 562, 559, 741, 347, 646, 589, 494, 593, 582},
        //   {1076, 1007, 452, 1119, 1217, 933, 908, 1517, 1092, 1409},
        //   {1290, 1149, 1069, 1069, 844, 1034, 874, 927, 996, 1086},
        //   {616, 560, 502, 598, 452, 594, 429, 647, 486, 546},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {128, 133, 126, 125, 118, 146, 181, 108, 136, 148}
        //  },
        //  {
        //   {49, 43, 47, 49, 53, 50, 49, 53, 59, 65},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {14, 19, 20, 24, 20, 22, 22, 18, 12, 25},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //   {425, 272, 460, 330, 485, 500, 398, 294, 421, 417},
        //   {649, 674, 546, 724, 457, 729, 574, 861, 588, 725},
        //   {461, 414, 412, 426, 534, 476, 438, 379, 485, 488},
        //   {472, 358, 414, 346, 450, 379, 529, 396, 373, 536},
        //   {106, 126, 137, 125, 131, 168, 136, 153, 125, 123},
        //   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        //  }
        // }
        //};
        MP_random_data d(&stochasticDemand[0][0][0][0], ST, A, A);

        MP_data x_min;
        x_min() = 0.5;

        //double kerosinPrice[numST-1][numScen] = { {0.055, 0.11, 0.165, 0.275, 0.4125, 0.55, 0.6875, 0.825, 1.045, 1.375} };
        double kerosinPrice[numST-1][numScen] = { {0.055} };
        MP_random_data k_price(&kerosinPrice[0][0], ST);

        MP_data f_price;
        f_price() = 0.55;

        MP_data f_price_margin;
        f_price_margin() = 10;


        /////// VARIABLES

        MP_variable y(A,A,T);
        y.integer();
        for (int i = 0; i < numA; i++) {
            for (int j = 0; j < numA; j++) {
                for (int k = 0; k < numT; k++) {
                    if (i == j || dist(i,j) > r(k)) {
                        y.upperLimit(i,j,k) = 0;
                    }
                }
            }
        }

        MP_variable x0(ST,A,A2);
        MP_variable x1(ST,A,A2,A3);
        MP_variable x2(ST,A,A2,A3,A4);
        for (int i = 0; i < numA; i++) {
            for (int j = 0; j < numA; j++) {
                if (i == j) {
                    x0.upperLimit(1,i,j) = 0;
                }
                for (int k = 0; k < numA; k++) {
                    if (i == j || i == k || j == k) {
                        x1.upperLimit(1,i,k,j) = 0;
                    }
                    for (int l = 0; l < numA; l++) {
                        if (i == j || i == k || i == l || j == k || j == l || k == l) {
                            x2.upperLimit(1,i,k,l,j) = 0;
                        }
                    }
                }
            }
        }

        MP_variable buy_stoch(ST), buy_hedge, jet_fuel_consumption;



        ////// CONSTRAINTS

        //demand
        MP_constraint demand(ST,A,A2);
        demand(ST+1,A,A2) = x0(ST+1,A,A2) + sum(A3, x1(ST+1,A,A3,A2)) + sum(A3*A4, x2(ST+1,A,A3,A4,A2)) <= d(ST+1,A,A2);

        //subject to force_percentage_expected_demand_satisfaction 
        MP_constraint percentDemandSat(ST,A,A2);
        percentDemandSat(ST+1,A,A2) = x_min() * d(ST+1,A,A2) <= x0(ST+1,A,A2) + sum(A3, x1(ST+1,A,A3,A2)) + sum(A3*A4, x2(ST+1,A,A3,A4,A2));

        //max_aircraft_capacity
        MP_constraint maxAircraftCapacity(ST,A,A2);
        maxAircraftCapacity(ST+1,A,A2) = x0(ST+1,A,A2) + sum(A3, x1(ST+1,A,A2,A3) + x1(ST+1,A3,A,A2) + sum(A4, x2(ST+1,A,A2,A3,A4) + x2(ST+1,A3,A,A2,A4) + x2(ST+1,A3,A4,A,A2))) <= sum(T, m(T) * y(A,A2,T));

        //ub for airlcraft types{t in T}: sum{i in A, j in A}(y[i,j,t])<=ub[t];
        MP_constraint availableAircrafts(T);
        availableAircrafts(T) = sum(A*A2, y(A,A2,T)) <= ub(T);

        //buy_jet_fuel: buy_stoch + buy_hedge = jet_fuel_consumption;
        MP_constraint buyJetFuel(ST);
        buyJetFuel(ST+1) = buy_stoch(ST+1) + buy_hedge() == jet_fuel_consumption();

        //fuel consumption
        MP_constraint fuelConsumption;
        fuelConsumption() = sum(A*A2*T, y(A,A2,T) * dist(A,A2) * cons(T)) == jet_fuel_consumption();



        /////// OBJECTIVE

        mod.setObjective(
            (buy_stoch(ST+1) * k_price(ST+1) + buy_hedge() * f_price() * (1 + (f_price_margin() / 100))) //fuel costs
            + (sum(A*A2*T, y(A,A2,T) * dist(A,A2) * cpm(T))) //operationcost
            - (sum(A*A2, p(A,A2) * dcf() * (x0(ST+1,A,A2) + (1 - pd()) * sum(A3, x1(ST+1,A,A3,A2)) + (1 - 2*pd()) * sum(A3*A4, x2(ST+1,A,A3,A4,A2))))) //revenue
            );

        mod.attach(mod.Solver);

        //mod.solve(MP_model::MINIMIZE);

        OsiStochasticBendersTwoStageSolverInterface benders(mod.Solver,mod.getSmi());
        benders.initialSolve();
    }

} //end namespace

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char *argv[]) {
    //VLDDisable();
    ::testing::InitGoogleTest(&argc, argv);
    //VLDEnable();
    //::testing::GTEST_FLAG(filter) = "*.RandomWalkTest";
    //_CrtMemState memstate;
    //_CrtMemCheckpoint(&memstate);

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
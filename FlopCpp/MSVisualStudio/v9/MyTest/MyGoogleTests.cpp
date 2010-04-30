//#include <vld.h> uncomment to enable Visual Leak Detector

////TODO: Workaround for iterator container debuggin
//#define _HAS_ITERATOR_DEBUGGING 0
//#define _SECURE_SCL             0

#include <gtest/gtest.h>
#include <iostream>
#include <ctime>
#include <limits>

#include "OsiStochasticSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"
#include "OsiCbcSolverInterface.hpp"
//#include "OsiCpxSolverInterface.hpp"
#include "CoinFloatEqual.hpp"

#include "flopc.hpp"
//#include "Benders.hpp"





//#include <vld.h> //Visual Leak Detector

//TODO: Test about correct colOrdering that is given to the solver..


using namespace flopc;
namespace {

    class MP_modelTest: public ::testing::Test {
    public:
    protected:
        MP_modelTest() : testModel(new OsiClpSolverInterface()){}
        virtual ~MP_modelTest(){}
        virtual void SetUp(){}
        virtual void TearDown(){}

        MP_model testModel; //Attention: Do not rely on testModel for actual testing (includes attachment of solver): Leads to Run-Time Failure #0
    };

    class RandomVariableTest: public ::testing::Test {
    public:
    protected:
        RandomVariableTest() :testModel(new OsiClpSolverInterface()){}
        virtual ~RandomVariableTest(){}
        virtual void SetUp(){}
        virtual void TearDown(){}

        MP_model testModel;
    };
    TEST_F(RandomVariableTest,ShortTest){
        enum {numStage=2};
        enum {numScen=10};
        MP_model testModel(new OsiClpSolverInterface());
        MP_stage T(numStage); // enum numStage = 2
        MP_scenario_set scenSet(numScen); //enum numScen=10
        double scenarioValues[numStage-1][numScen] = { { 1,2,3,4,5,6,7,8,9,10 } };
        MP_random_data r(&scenarioValues[0][0],T); //Scenario-based 

    }

    //Test if Conditional Sampling works (for seed 0 for every RV. This means values are highly correlated)
    TEST_F(RandomVariableTest,ConditionalSamplingTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(3);
        RandomVariable* scenarioValues[2] = { new DiscreteUniformRandomVariable(0,10,1,10),new DiscreteUniformRandomVariable(0,10,2,10)};
        RandomVariable* scenarioValues1[2] = { new DiscreteUniformRandomVariable(10,20,1,10),new DiscreteUniformRandomVariable(10,20,2,10)};
        MP_random_data rvar(&scenarioValues[0],T);
        MP_random_data rvar2(&scenarioValues1[0],T);
        MP_variable x(T);
        MP_constraint con1(T);
        con1(T+1) = x(T+1)*(rvar(T+1)+rvar2(T+1)) <= 10;//*rvar2(T);
        testModel2.setObjective(x(T.last()));
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MAXIMIZE);
        EXPECT_DOUBLE_EQ(0.43177511288609688,testModel2->getObjValue()); //If seed 0 is used for all RV
        EXPECT_EQ(400,testModel2.ScenSet().size()); //Test if number of scenarios is correctly generated (this is a consequence of the sampled values)
        EXPECT_EQ(70,testModel2.smiModel->getNumScenarios());
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
    }

     TEST_F(MP_modelTest,InvestmentExampleTest){
        // Simple Example Model to test fundamental ScenTree Generation and Solving Capabilites
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
        allocationConstr(T) = sum(assets,x(T,assets)) == wealth(T);
        returnConstr(T+1) = sum(assets,returns(T+1,assets)*x(T,assets) ) == wealth(T+1);

        goalConstr(T.last()) = wealth(T.last()) == goal() + overage(T.last()) - shortage(T.last()); 
        MP_expression valueFunction( -1.3*shortage(T.last()) + 1.1*overage(T.last()));

        investmentModel.setObjective( valueFunction );
        investmentModel.attach(investmentModel.Solver);
        investmentModel.solve(MP_model::MAXIMIZE);
        x.display("buy decisions");
        wealth.display("wealth per stage");
        for (int i = 0; i < scen.size();i++){
            for (int j = 0; j < T.size(); j++){
                cout << "scenario " << i << " stufe " << j << " wealth: " << wealth.levelScenario(i,j) << endl;
            }
        }

        //Compare solution values.
        EXPECT_NEAR(4.142141441,investmentModel.Solver->getObjValue(),0.00000001);
        EXPECT_EQ(MP_model::OPTIMAL,investmentModel.getStatus());
    }

    TEST_F(MP_modelTest,DataTest){
        MP_data data1;
        MP_data data2;
        data1() = 7;
        data2() = 10;
        MP_data data3;
        MP_data data4;
        data4() = data2();
        data3() = data1() + data2();
        MP_variable x;

        MP_constraint constraint1,constraint2;
        constraint1 = (data4() +data3())*x() <= 10;
        constraint2 = data4()*data3()*x() <= 10;

        testModel.setObjective(x());
        testModel.attach(testModel.Solver);
        testModel.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());

    }

    TEST_F(MP_modelTest,DacotaModel) {
        MP_model dacota(new OsiClpSolverInterface()); 
        MP_stage T(2);
        
        enum {low, normal, high, numScenarios};
        MP_scenario_set scen(numScenarios);
        
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
        
        // Ohne diese Restriktion ist das Modell unbeschränkt, obwohl x(R) mit negativen Kosten in ZF eingeht
        //conR(R) = x(R) <= 100;
        
        dacota.setObjective(
            sum(P, y(T+1,P) * sellingPrice(P))
          - sum(R, x(R) * resourceCost(R))
        );
        dacota.attach(dacota.Solver);
        dacota.solve(MP_model::MAXIMIZE); 
        x.display("x (Resources)");
        cout << "Finished";
        EXPECT_DOUBLE_EQ(1616.666666666667,dacota->getObjValue());
        EXPECT_EQ(MP_model::OPTIMAL,dacota.getStatus());
    }


    TEST_F(MP_modelTest,TimoTest){
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
        testModel2.solve(MP_model::MINIMIZE); 
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
    }

    TEST_F(MP_modelTest,ScenarioTreeGenerationTest){
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
        MP_constraint constraint1(T);
        constraint1(T+1) = sum(demands,x(T+1,demands)*rvar(T+1,demands)) + firstStageVar() >= sum(demands,x(T,demands));
        testModel2.setObjective(x(T.last(),demands));
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        EXPECT_DOUBLE_EQ(0,testModel2.Solver->getObjValue());
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
    }

   

    TEST_F(MP_modelTest,ScenarioRandomVariableConstructionTest){
        //Create RV via double array. 
        MP_set autos(4); //We have a set with elements 0 to 3
        enum {numScen = 3};
        MP_scenario_set scen(numScen);
        double rvArr[4][numScen] = { {1,2,3},{4,5,6},{7,8,9},{10,11,12} };
        MP_random_data rvar1;
        MP_random_data rvar2(&rvArr[0][0],autos);
        //Expect invalid argument exception
        //EXPECT_THROW(MP_random_data rvar2(&rvArr[0][0],autos),invalid_argument_exception);
        EXPECT_DOUBLE_EQ(rvArr[0][0], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(0)) )->getSampledValue(0) );
        EXPECT_DOUBLE_EQ(rvArr[0][1], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(0)) )->getSampledValue(1) );
        EXPECT_DOUBLE_EQ(rvArr[0][2], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(0)) )->getSampledValue(2) );
        EXPECT_DOUBLE_EQ(rvArr[3][0], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(3)) )->getSampledValue(0) );
        EXPECT_DOUBLE_EQ(rvArr[3][1], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(3)) )->getSampledValue(1) );
        EXPECT_DOUBLE_EQ(rvArr[3][2], ( dynamic_cast<ScenarioRandomVariable* > (rvar2(3)) )->getSampledValue(2) );
    }


    TEST_F(MP_modelTest,SimpleRandomVariableExpressionParsingTest){
        //Tests RV without sets and some combinations. Compile-Time Test
        MP_random_data rVar1;
        MP_variable x1;
        MP_variable y1;
        MP_variable x2;
        MP_variable y2;
        MP_constraint exp1;
        //Test some combinations of left and right hand side
        //More than 1 assignments to a MP_expression can lead to curious behaviour
        //Question: Why is that?
        exp1() = 3*x1() + rVar1()*y1() <= 7;
        exp1() = 4*x1()+5*x2() <= 10;
        exp1() = rVar1()*4 + y2()*rVar1() <= 8; //How can this work? constant get implicitly converted to expression..
        //Somehow we have to check if the rhs is properly evaluated in the end
    }



    TEST_F(MP_modelTest,RandomBoundsTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        MP_data prob(scen);
        prob(0) = 0.5;
        prob(1) = 0.5;
        testModel2.Probabilities(prob);
        double scenarioValues[1][2] = { {1,2} };
        MP_random_data var(&scenarioValues[0][0],T);
        MP_variable x(T);
        MP_constraint con1(T);
        con1(T) = x(T) <= var(T);
        x.lowerLimit() = var(T); //Set different lower limits
        MP_expression objective;
        objective = x(T+1);
        testModel2.setObjective(objective);
        testModel2.attach(testModel.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        EXPECT_EQ(1.5,testModel2->getObjValue());
    }

    TEST_F(MP_modelTest,ComplicatedRandomBoundsTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        double scenarioValues[1][2] = { {1,2} };
        MP_random_data var(&scenarioValues[0][0],T);
        MP_variable x(T);
        x.lowerLimit() = var(T);
        x.upperLimit() = var(T)+5;
        MP_constraint con1(T);
        con1(T+1) = x(T+1) <= 200;
        testModel2.setObjective(x(T+1));
        testModel2.attach(testModel.Solver);
        testModel2.solve(MP_model::MAXIMIZE);
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        //TODO: Test appearance of RandomVariable vector
        EXPECT_EQ(6.5,testModel2->getObjValue());

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
        exp1(T+1) = y2(T+1)*rVar1(T+1) <= 8;
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
        // Test RV Usage by Assignment or Initialization.
        enum {Audi, BMW, Ford, numCars};
        MP_set cars(numCars);
        MP_random_data vars(cars); //Initialize RV with set
        DiscreteRandomVariable *x,*y,*z;
        x = new DiscreteRandomVariable();
        y = new DiscreteRandomVariable();
        z = new DiscreteRandomVariable();
        vars(BMW) = x; 
        vars(Audi) = y;
        vars(Ford) = z;
        RandomVariable* varsArr[3] = { new DiscreteRandomVariable(),new DiscreteRandomVariable(),new DiscreteRandomVariable() }; //Initialize RV with Array of RV
        // Need to delete Array as RV does not take ownership..
        // Maybe store boost::shared_ptr<> to RV?
        // Then the values assigned to the RV get managed by the RV and released too
        // No Deletion of array values necessary afterwards..
        MP_random_data vars2(&varsArr[0],cars);
        //Check if vars2 contains these random variables
        EXPECT_EQ(varsArr[0],vars2(0)); //only checks pointer
        EXPECT_EQ(varsArr[1],vars2(1));
        EXPECT_EQ(varsArr[2],vars2(2));
    }

    //RandomVariable Sampling Tests..
    TEST_F(RandomVariableTest,RandomVariableSamplingTest){
        MP_scenario_set scenSet(3);
        double vals[4][3] = { {4, 3, 2},{2,1,5},{7,3,2},{1,5,7} };
        MP_set testSet(4);
        MP_random_data rVarScenario(&vals[0][0],testSet);
        //MP_random_data rVarDiscrete;
        //MP_random_data rVarContinuous;
        DiscreteUniformRandomVariable* rv = new DiscreteUniformRandomVariable(-3,2,1.0/2);
        ContinuousUniformRandomVariable* rv2 = new ContinuousUniformRandomVariable(-2,2); 
        rv->Seed(static_cast<unsigned int>(std::time(0)));
        rv->sample(20);
        std::vector<double> sampledValues = rv->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i],-3);
            EXPECT_LE(sampledValues[i],2);
        }
        rv2->Seed(static_cast<unsigned int>(std::time(0)));
        rv2->sample(20);
        sampledValues = rv2->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_GE(sampledValues[i],-2);
            EXPECT_LE(sampledValues[i],2);
        }
        ScenarioRandomVariable* rv3 = dynamic_cast<ScenarioRandomVariable*>(rVarScenario(0));
        rv3->sample();
        sampledValues = rv3->getSampledValues();
        for (std::vector<double>::size_type i = 1; i < sampledValues.size();++i){
            EXPECT_EQ(vals[0][i],sampledValues[i]);
        }
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

    TEST_F(RandomVariableTest,AdditiveConstantSamplingTest){
        MP_model testModel2(new OsiClpSolverInterface());
        double testValue = 3;
        MP_stage T(2);
        MP_scenario_set scen(10);
        double scenarios[10] = {0,1,2,3,4,5,6,7,8,9};
        MP_random_data rvar(&scenarios[0],T);
        MP_variable x;
        x.lowerLimit = 0;
        
        MP_variable y(T);
        y.lowerLimit = 0;
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
        y.lowerLimit = 1;
        MP_constraint constraint1(T);
        MP_data testData(&testValues[0],T);
        MP_random_data combinedData(T);
        combinedData(T) = testData(T)*rvar(T); // Necessitates an update for index expressions? Indices on the right hand side should store the same index as item on the left hand side, so if left hand side item changes, all other indices change too. This necessitates an index update mechanism, so that they are linked. => Indices need to be stored as pointers. 
        constraint1(T+1) = x()-y(T+1)*combinedData(T+1) >= 7;
        testModel2.setObjective(x());
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        //TODO: We need to test the results, as otherwise this test is meaningless
        //EXPECT_EQ(1,testModel2.RandomVariables.size());
        EXPECT_EQ(MP_model::OPTIMAL,testModel2.getStatus());
        EXPECT_EQ(34,testModel2->getObjValue());
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
        assert(m1->getObjValue()>=1566.03 && m1->getObjValue()<=1566.05);

        y.display("y first model");
        ab.display("ab");
        db.display("db");
        bcd1.display("bcd1");
        ocd.display("ocd");
        oc.display("operation cost");
        x.display("x");

        m2.add(ab).add(yd).add(bd).add(bcd2).add(ocd);
        y.upperLimit = m2->getInfinity();

        m2.minimize(oc() + bc());

        assert(m2->getNumRows()==56);
        assert(m2->getNumCols()==72);
        assert(m2->getNumElements()==219);

        // Optimal objective value m1: 1566.04218913
        // Optimal objective value m2: 1566.04218913 (like m1)
        assert(m2->getObjValue()>=1566.03 && m2->getObjValue()<=1566.05);

        CoinRelFltEq eq;
        assert( eq(m2->getObjValue(),1566.04218913) );
        assert( eq(m1->getObjValue(),1566.04218913) );

        y.display("y second model");
    }


} //end namespace

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "RandomVariableTest.*";
    return RUN_ALL_TESTS();
}
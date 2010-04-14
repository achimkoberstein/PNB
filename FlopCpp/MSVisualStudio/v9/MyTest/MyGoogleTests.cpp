//#include <vld.h> uncomment to enable Visual Leak Detector
#include <gtest/gtest.h>
#include <iostream>
#include <ctime>
#include <limits>

#include "OsiStochasticSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"
#include "CoinFloatEqual.hpp"

#include "flopc.hpp"
//#include "Benders.hpp"





//#include <vld.h> //Visual Leak Detector




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

    class BendersTest: public ::testing::Test{

    protected:
        BendersTest() : model(new OsiClpSolverInterface()){
            //initalize model

            //everything initialized, got SmiScnModel
        }
        MP_model model; //model
    };

    TEST_F(BendersTest,BendersSolverTest){
        //model.solve(MP_model::MINIMIZE);
        // call copyModel(smiModel)
        // call copySubModel(smiModel) methods from Benders..
    }

    TEST_F(MP_modelTest,TimoTest){
        MP_model testModel2(new OsiClpSolverInterface());
        MP_stage T(2);
        MP_scenario_set scen(2);
        double scenarios[1][2] =
        { // T
            {1,2} //scen
        };
        MP_random_data rvar(&scenarios[0][0],T);
        MP_variable x(T);
        MP_constraint constraint1, constraint2(T);
        constraint1 = x(0) == 1;
        //constraint2(T+1) = x(T+1)*rvar(T+1) >= x(T);
        //constraint2(T+1) = x(T+1) + rvar(T+1) <= x(T); 
        constraint2(T+1) = x(T+1) >= x(T)*rvar(T+1); 
        testModel2.setObjective(x(T.last()));
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MINIMIZE); 

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
    }

    TEST_F(MP_modelTest,InvestmentExampleTest){
        // Simple Example Model to test fundamental ScenTree Generation and Solving Capabilites
        MP_model investmentModel(new OsiClpSolverInterface());
        MP_data initialWealth;
        initialWealth() = 55; //set data

        enum {numStage=3};
        MP_stage T(numStage); //Time Stages
        enum {numScen=4};
        MP_scenario_set scen(numScen);
        enum {asset1,asset2,numAssets=2};
        MP_set assets(numAssets);
        double scenarios[numStage-1][numAssets][numScen]=
        {//T
            //{//assets
            //    {1,2,3,4},{5,6,7,8} //scen
            //},
            {//assets
                {2,4,2,4},{10,14,10,14} //scen
            },
            {//assets
                {3,6,9,12},{15,18,21,24} //scen
            }
        };
        MP_random_data returns(&scenarios[0][0][0],T,assets); //TODO: This is stochastic data and needs to get initialized


        MP_variable x(T,assets);
        MP_variable wealth(T);

        MP_constraint 
            initialWealthConstr,
            returnConstr(T),
            allocationConstr(T),
            globalTestConstraint(T);

        initialWealthConstr() = sum(assets,x(0,assets)) == initialWealth();
        allocationConstr(T) = sum(assets,x(T,assets)) == wealth(T);
        returnConstr(T+1) = sum(assets,returns(T+1,assets)*x(T,assets) ) == wealth(T+1);
        globalTestConstraint(T) = sum(assets,x(T,assets)) >= 10; // I want a global constraint here: not generated according to scenario tree.. => we need new construct for that..
        //investmentModel.setObjective(sum(T,wealth(T))); //Only maximize wealth of last-period..
        investmentModel.setObjective(wealth(T.last()));
        investmentModel.attach(investmentModel.Solver);
        investmentModel.solve(MP_model::MAXIMIZE);
        //const double* colArray = investmentModel.Solver->getColSolution();
        //Compare solution values.
        EXPECT_DOUBLE_EQ(1320,investmentModel.Solver->getObjValue());

    }

    TEST_F(MP_modelTest,ScenarioRandomVariableConstructionTest){
        //Create RV via double array. 
        MP_set autos(4); //We have a set with elements 0 to 3
        enum {numScen = 3};
        MP_scenario_set scen(numScen);
        double rvArr[4][numScen] = { {1,2,3},{4,5,6},{7,8,9},{10,11,12} };
        MP_random_data rvar1;
        MP_random_data rvar2(&rvArr[0][0],autos);
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
        MP_variable x;
        MP_variable y(T);
        MP_random_data rVar(T);
        rVar(0) = new DiscreteRandomVariable();
        rVar(1) = new DiscreteRandomVariable();
        MP_random_data rVar2(T);
        MP_random_data rVar3(T);
        MP_constraint constraint1(T);
        MP_index j;
        //I would expect that such things would work, i.e. assign to different indices of one constraint.
        //constraint1(0) = 3*x() <= 10;
        constraint1(T+1) = 4*x()+(rVar(T+1)+2)*y(T+1) <= rVar3(T+1)*20;
        //TODO: This is counterintuitive
        //TODO: Test of the coefficients get generated in the right manner.
        testModel.setObjective( 3*x() );
        testModel.attach(testModel.Solver);
        //We need to do some tests here..
        testModel.solve(MP_model::MAXIMIZE);
        //Print some results..
        EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
        EXPECT_EQ(15,testModel->getObjValue()); //Normal Solver gets called, as no random vars were specified
    }

    TEST_F(MP_modelTest,RandomVariableInAllStagesTest){
        MP_stage T(3); //We have three stages
        MP_random_data rVar(T); //Random Variable depends only on stage
        MP_random_data rVar2(T);
        //As we see: Only stages 1..T needs to be defined. If a RV is indexed via stage=0, in the generation process later on, it is not evaluated.
        rVar(1) = new DiscreteRandomVariable();
        rVar2(1) = new DiscreteRandomVariable();
        rVar(2) = new DiscreteRandomVariable();
        rVar2(2) = new DiscreteRandomVariable();
        MP_variable x;
        MP_variable y(T);
        MP_constraint con1; 
        MP_constraint con2(T);
        con1() = x() <= 10;
        con2(T) = x() + rVar(T)*y(T) <= rVar2(T);
        testModel.setObjective(y(T.last()));
        testModel.attach(testModel.Solver);
        testModel.solve(MP_model::MAXIMIZE);
    }

    TEST_F(MP_modelTest,RandomVariableInitializationTest){
        // Test RV Usage by Assignment or Initialization.
        enum {Audi, BMW, Ford, numCars};
        MP_set cars(numCars);
        MP_random_data vars(cars); //Initialize RV with set
        //TODO: Uncomment if functionality is implemented..
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
        MP_variable y(T);
        MP_constraint constraint1(T);
        constraint1(T+1) = x()+y(T+1)*(rvar(T+1)+testValue) >= 7;
        testModel2.setObjective(x());
        testModel2.attach(testModel2.Solver);
        testModel2.solve(MP_model::MINIMIZE);
        //Test for Correctnes 
        EXPECT_EQ(testValue,testModel2.Solver->getMatrixByRow()->getCoefficient(1,2));
        EXPECT_EQ(testValue+1,testModel2.Solver->getMatrixByRow()->getCoefficient(2,3));
        EXPECT_EQ(testValue+5,testModel2.Solver->getMatrixByRow()->getCoefficient(6,7));
        testModel2.detach();
        double scenarios2[10] = {0,1,2,3,4,5,6,7,8,9};
        MP_random_data rvar2(&scenarios2[0],T);
        constraint1(T+1) = x()+y(T+1)*(rvar2(T+1)*testValue) >= 7;
        testModel2.setObjective(x());
        testModel2.attach(new OsiClpSolverInterface());
        testModel2.solve(MP_model::MINIMIZE);
        EXPECT_EQ(testValue*0,testModel2.Solver->getMatrixByRow()->getCoefficient(1,2));
        EXPECT_EQ(testValue,testModel2.Solver->getMatrixByRow()->getCoefficient(2,3));
        EXPECT_EQ(testValue*5,testModel2.Solver->getMatrixByRow()->getCoefficient(6,7));
        //testModel2.detach();
        //double scenarios3[10] = {0,1,2,3,4,5,6,7,8,9};
        //MP_random_data rvar3(&scenarios2[0],T);
        //constraint1(T+1) = x()+y(T+1)*((rvar3(T+1)+testValue)*10) >= 7; //This does not yet work. But i think it is not so important either..
        //testModel2.setObjective(x());
        //testModel2.attach(new OsiClpSolverInterface());
        //testModel2.solve(MP_model::MINIMIZE);

    }

    //TEST_F(RandomVariableTest,RandomParamCombinationTest){
    //    MP_model testModel2(new OsiClpSolverInterface());
    //    double testValue = 3;
    //    MP_stage T(2);
    //    MP_scenario_set scen(10);
    //    double scenarios[10] = {0,1,2,3,4,5,6,7,8,9};
    //    double testValues[2] = {-3,3};
    //    MP_random_data rvar(&scenarios[0],T);
    //    MP_variable x;
    //    MP_variable y(T);
    //    MP_constraint constraint1(T);
    //    MP_data testData(&testValues[0],T);
    //    
    //    MP_data combinedData(T);
    //    combinedData(T) = testData(T)*rvar(T);
    //    constraint1(T+1) = x()+y(T+1)*combinedData(T+1) >= 7;
    //    testModel2.setObjective(x());
    //    testModel2.attach(testModel2.Solver);
    //    testModel2.solve(MP_model::MINIMIZE);
    //}
    

} //end namespace

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
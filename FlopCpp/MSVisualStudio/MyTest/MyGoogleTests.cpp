//#include <vld.h> uncomment to enable Visual Leak Detector
#include <gtest/gtest.h>
#include <flopc.hpp>
#include "OsiClpSolverInterface.hpp"
#include "CoinFloatEqual.hpp"
#include <iostream>
#include <ctime>

//#include <vld.h> //Visual Leak Detector




using namespace flopc;
namespace {

    class MP_modelTest: public ::testing::Test {
    public:
    protected:
        MP_modelTest() :testModel(new OsiClpSolverInterface){}
        virtual ~MP_modelTest(){}
        virtual void SetUp(){}
        virtual void TearDown(){}
        MP_model testModel;
    };

    class RandomVariableTest: public ::testing::Test {
    public:
    protected:
        RandomVariableTest() :testModel(new OsiClpSolverInterface){}
        virtual ~RandomVariableTest(){}
        virtual void SetUp(){}
        virtual void TearDown(){}
        MP_model testModel;
    };

    TEST_F(MP_modelTest,InvestmentExampleTest){
        // Simple Example Model to test fundamental ScenTree Generation and Solving Capabilites
        MP_model investmentModel(new OsiClpSolverInterface);
        MP_data initialWealth;
        initialWealth() = 55; //set data

        enum {numStage=3};
        MP_stage T(numStage); //Time Stages
        enum {numScen=4};
        MP_scenario_set scen(numScen);
        enum {asset1,asset2,numAssets=2};
        MP_set assets(numAssets);
        double scenarios[numStage][numAssets][numScen]=
        {//T
            {//assets
                {1,2,3,4},{5,6,7,8} //scen
            },
            {//assets
                {2,2,4,4},{10,10,14,14} //scen
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
            allocationConstr(T);

        MP_index i;
        initialWealthConstr() = sum(assets,x(0,assets)) == initialWealth();
        allocationConstr(T) = sum(assets,x(T,assets)) == wealth(T);
        returnConstr(T+1) = sum(assets,returns(T+1,assets)*x(T,assets) ) == wealth(T+1);
        //investmentModel.setObjective(sum(T,wealth(T))); //Only maximize wealth of last-period..
        investmentModel.setObjective(wealth(T.last()));
        investmentModel.attach(investmentModel.Solver);
        investmentModel.solve(MP_model::MAXIMIZE);
        const double* colArray = investmentModel.Solver->getColSolution();
        //Compare solution values.
        EXPECT_DOUBLE_EQ(1402.5,investmentModel.Solver->getObjValue());

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
        EXPECT_EQ(0,testModel->getObjValue()); //Model is empty, as no scenarios were specified..
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
    TEST_F(RandomVariableTest,UniformDistributionTest){
        MP_random_data rVarDiscrete;
        MP_random_data rVarContinuous;
        DiscreteUniformRandomVariable* rv = new DiscreteUniformRandomVariable(-3,2,1.0/2);
        ContinuousUniformRandomVariable* rv2 = new ContinuousUniformRandomVariable(-2,2);
        rv->Seed(static_cast<unsigned int>(std::time(0)));
        rv->sample(20);
        rv2->Seed(static_cast<unsigned int>(std::time(0)));
        rv2->sample(20);
    }

} //end namespace

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
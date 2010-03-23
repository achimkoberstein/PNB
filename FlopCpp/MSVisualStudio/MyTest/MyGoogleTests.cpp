//#include <vld.h> uncomment to enable Visual Leak Detector
#include <gtest/gtest.h>
#include <flopc.hpp>
#include "OsiClpSolverInterface.hpp"
#include "CoinFloatEqual.hpp"
#include <iostream>


using namespace flopc;
namespace {

	class MP_modelTest: public ::testing::Test {
	public:
		
	protected:
		MP_modelTest() :testModel(new OsiClpSolverInterface){
			
			}
		virtual ~MP_modelTest(){

			}
		virtual void SetUp(){
						
			}
		virtual void TearDown(){

			}
		MP_model testModel;
		};

	//TEST_F(MP_modelTest,InvestmentExampleTest){

	//	MP_model investmentModel(new OsiClpSolverInterface);
	//	MP_data initialWealth;
	//	initialWealth() = 55; //set data

	//	MP_stage T(3); //Time Stages
	//	enum {asset1,asset2,numAssets};
	//	MP_set assets(numAssets);
	//	MP_random_variable returns(T,assets); //TODO: This is stochastic data 

	//	MP_variable x(T,assets);
	//	MP_variable wealth(T);

	//	MP_constraint 
	//		initialWealthConstr,
	//		returnConstr(T),
	//		allocationConstr(T);

	//	MP_index i;
	//	initialWealthConstr() = sum(assets,x(0,assets)) == initialWealth();
	//	allocationConstr(T) = sum(assets,x(T,assets)) == wealth(T);
	//	returnConstr(T+1) = sum(assets,returns(T+1,assets)*x(T,assets) ) == wealth(T+1);
	//	//If returns would be a MP_random_variable this would no longer work..
	//	//At least that is what i thougth..

	//	}

	//TEST_F(MP_modelTest,MPRandomVariableConstructionTest){
	//	MP_set autos(4); //We have a set with elements 0 to 3
	//	MP_random_variable rvar1;
	//	MP_random_variable rvar2(autos);
	//	}


	//TEST_F(MP_modelTest,SimpleRandomVariableExpressionParsingTest){

	//	MP_random_variable rVar1;
	//	MP_variable x1;
	//	MP_variable y1;
	//	MP_variable x2;
	//	MP_variable y2;
	//	MP_constraint exp1;
	//	//Test some combinations of left and right hand side
	//	exp1() = 3*x1() + rVar1()*y1() <= 7;
	//	exp1() = 4*x1()+5*x2() <= 10;
	//	exp1() = rVar1()*4 + y2()*rVar1() <= 8; //How can this work? constant get implicitly converted to expression..
	//	//Somehow we have to check if the rhs is properly evaluated in the end
	//	//TODO:
	//	}

	//TEST_F(MP_modelTest,RandomVariableExpressionParsingTest){
	//	//Now we test RandomVariables with sets
	//	MP_stage T(2); //2 stages
	//	MP_random_variable rVar1(T);
	//	MP_variable x;
	//	MP_variable y(T);
	//	MP_variable x2;
	//	MP_variable y2(T);
	//	MP_constraint exp1(T);
	//	//Test some combinations of left and right hand side
	//	exp1(T+1) = 3*x(T) + 4*y2(T+1) <= 9;
	//	exp1(T) = y2(T)*4 <= rVar1(T);
	//	exp1(T+1) = y2(T+1)*rVar1(T+1) <= 8;
	//	//Only latest assignment remains.. clearly this is correct behaviour
	//	//TODO: Test if the parsed expressions are correct
	//	}

	TEST_F(MP_modelTest,SimpleRandomVariableSolverTest){
		testModel.detach();
		testModel.verbose();
		MP_stage T(2);
		MP_variable x;
		MP_variable y(T);
		MP_random_variable rVar(T);
		MP_random_variable rVar2(T);
		MP_constraint constraint1(T);
		MP_index j;
		//I would expect that such things would work, i.e. assign to different indices of one constraint.
		//constraint1(0) = 3*x() <= 10;
		constraint1(T+1) = 4*x()+rVar(T+1)*y(T+1)+2*y(T) +rVar2(T+1) <= 20;
		//TODO: This is counterintuitive
		testModel.setObjective( 3*x() );
		testModel.attach(new OsiClpSolverInterface());
		//We need to do some tests here..
		testModel.solve(MP_model::MAXIMIZE);
		//Print some results..
		EXPECT_EQ(MP_model::OPTIMAL,testModel.getStatus());
		EXPECT_EQ(15,testModel->getObjValue());

		vector<int> colStage(testModel->getNumCols());
		vector<int> rowStage(testModel->getNumRows());


		for (int j=0; j!=testModel->getNumCols(); ++j) {
			colStage[j] = testModel.getColStage()[j];
			cout<<"Col stage  "<<j<<"  "<<colStage[j]<<endl;
			}

		for (int i=0; i!=testModel->getNumRows(); ++i) {
			rowStage[i] = testModel.getRowStage()[i];
			cout<<"Row stage  "<<i<<"  "<<rowStage[i]<<endl;
			}
		}

	//TEST_F(MP_modelTest,RandomVariableInitializationTest){

	//	enum {Audi, BMW, Ford, numCars};
	//	MP_set cars(numCars);
	//	MP_random_variable vars(cars);
	//	RandomVariable x(RandomVariable::DistributionType::BINOMIAL);
	//	vars(BMW) = x;
	//	EXPECT_EQ(RandomVariable::DistributionType::BINOMIAL,vars(BMW).DistrType());
	//	
	//	RandomVariable varsArr[3] = { RandomVariable(),RandomVariable(),RandomVariable() };
	//	MP_random_variable vars2(&varsArr[0],cars);
	//	}



	} //end namespace

//TODO: Write Tests for right-hand-side with RV and constant term
int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
	}
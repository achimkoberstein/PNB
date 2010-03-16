#include "SmiScnModel.hpp"
#include <gtest/gtest.h>


namespace {

	class MP_modelTest: public ::testing::Test {

	protected:
		MP_modelTest() {

			}
		virtual ~MP_modelTest(){

			}
		virtual void SetUp(){

			}
		virtual void TearDown(){

			}
		};

	TEST_F(MP_modelTest,SetupTest){
			EXPECT_EQ(0,0);
		}
	} //end namespace


int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
	}
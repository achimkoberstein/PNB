cmake_minimum_required(VERSION 2.8)

add_executable(ParallelNestedBendersTest
	NestedBendersTest.cpp	
)

target_link_libraries(ParallelNestedBendersTest
	ParallelNestedBenders
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/gtest-1.6.0/build
	gtest
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/glog-0.3.1/.libs
	glog
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Osi/src/OsiCpx/.libs
	OsiCpx
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Osi/src/OsiClp/.libs
	OsiClp
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Osi/src/.libs
	Osi
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Smi/src/.libs
	Smi
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Cbc/src/.libs
	Cbc
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Cgl/src/.libs
	Cgl
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../Clp/src/.libs
	Clp
	-L${CMAKE_CURRENT_SOURCE_DIR}/../../CoinUtils/src/.libs
	CoinUtils
	-L${CPLEX_LIBRARY_DIR}
	cplex
	-L${BOOST_LIBRARY_DIR}
	boost_thread
)


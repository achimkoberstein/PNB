cmake_minimum_required(VERSION 2.8)

project(ParallelNestedBenders)

set(CMAKE_BUILD_TYPE Debug)

set(BOOST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/boost_1_49_0/install/include)
set(BOOST_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/boost_1_49_0/install/lib)

set(CPLEX_INCLUDE_DIR /home/christian/ILOG/CPLEX_Studio_AcademicResearch122/cplex/include)
set(CPLEX_LIBRARY_DIR /home/christian/ILOG/CPLEX_Studio_AcademicResearch122/cplex/lib/x86_sles10_4.1/static_pic)

include_directories(
	src
	../CoinUtils/inc
	../CoinUtils/src
	../Clp/inc
	../Clp/src
	../FlopCpp/inc
	../FlopCpp/src
	../Osi/src
	../Osi/src/OsiClp
	../Osi/src/OsiCpx
	../Osi/src/OsiGrb
	../Smi/src
	${CPLEX_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
	../ThirdParty/glog-0.3.1/src
	../ThirdParty/gtest-1.6.0/include
	../ThirdParty/threadpool
)

add_definitions(
	-DHAVE_CONFIG_H
)

add_subdirectory(src)
add_subdirectory(test)


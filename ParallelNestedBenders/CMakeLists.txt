cmake_minimum_required(VERSION 2.8)

project(ParallelNestedBenders)

set(CMAKE_BUILD_TYPE Debug)

set(BOOST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/boost)
set(BOOST_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/boost/stage/lib)


set(CPLEX_INCLUDE_DIR /opt/ibm/ILOG/CPLEX_Studio124/cplex/include)
set(CPLEX_LIBRARY_DIR /opt/ibm/ILOG/CPLEX_Studio124/cplex/lib/x86_sles10_4.1/static_pic)

set(GENERAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
	src
	${GENERAL_INCLUDE_DIR}/coin
	${GENERAL_INCLUDE_DIR}
	${CPLEX_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
	# we should get the next two via variables
	../ThirdParty/threadpool
)

add_definitions(
	-DHAVE_CONFIG_H
)

add_subdirectory(src)
#add_subdirectory(test)

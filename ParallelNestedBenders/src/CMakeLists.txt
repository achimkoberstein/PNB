cmake_minimum_required(VERSION 2.8)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
	   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


add_library(ParallelNestedBenders
	BendersNodeData.cpp
	OsiPnbStochasticSolverInterface.cpp
	OsiStochasticSolverInterface.cpp
	ParallelNestedBenders.cpp
	PseudoRandomGenerator.cpp
)

set(INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdPartyLibs)


#enable_testing()

# My test
add_executable(PNBTest ../test/NestedBendersTest.cpp)
target_link_libraries(PNBTest
	ParallelNestedBenders
	-L${INSTALL_DIR}/lib
	gtest
	glog
	OsiCpx
	OsiClp
	Osi
	Smi
	Cbc
	Cgl
	Clp
	CoinUtils
	-L${CPLEX_LIBRARY_DIR}
	cplex
	-L${BOOST_LIBRARY_DIR}
	boost_thread
	boost_system
	z
)
#add_test(PNBTest PNBTest)
install(TARGETS PNBTest 
	RUNTIME DESTINATION ${INSTALL_DIR}/bin
	LIBRARY DESTINATION ${INSTALL_DIR}/lib)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
set(CMAKE_BUILD_TYPE Debug)
set(CONFIGURE_WITH_DEBUG --enable-debug)

ExternalProject_Add(CoinUtils
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CoinUtils"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG}
	INSTALL_COMMAND ""
)

ExternalProject_Add(Cgl
	DEPENDS CoinUtils
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Cgl"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG}
	INSTALL_COMMAND ""
)

ExternalProject_Add(Clp
	DEPENDS CoinUtils
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Clp"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG}
	INSTALL_COMMAND ""
)

set(CPLEX_INCLUDE_DIR /home/christian/ILOG/CPLEX_Studio_AcademicResearch122/cplex/include)
set(CPLEX_LIBRARY_DIR /home/christian/ILOG/CPLEX_Studio_AcademicResearch122/cplex/lib/x86_sles10_4.1/static_pic)
ExternalProject_Add(Osi
	DEPENDS CoinUtils;Cgl;Clp
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Osi"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure "--with-cplex-incdir=${CPLEX_INCLUDE_DIR}" "--with-cplex-lib=-L${CPLEX_LIBRARY_DIR} -lcplex -lpthread" ${CONFIGURE_WITH_DEBUG}
        INSTALL_COMMAND ""
)

ExternalProject_Add(Cbc
	DEPENDS CoinUtils;Cgl;Clp;Osi
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Cbc"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG}
	INSTALL_COMMAND ""
)

ExternalProject_Add(Smi
	DEPENDS CoinUtils;Cgl;Clp;Osi;Cbc
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Smi"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG}
	INSTALL_COMMAND ""
)

ExternalProject_Add(FlopCpp
	DEPENDS CoinUtils;Cgl;Clp;Osi;Cbc;Smi
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FlopCpp"
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-glog-incdir=/home/christian/Documents/FlopBenders/ThirdParty/glog-0.3.1/src --with-glog-lib=/home/christian/Documents/FlopBenders/ThirdParty/glog-0.3.1/.libs/ --with-gtest-incdir=/home/christian/Documents/FlopBenders/ThirdParty/gtest-1.6.0/include --with-gtest-lib=/home/christian/Documents/FlopBenders/ThirdParty/gtest-1.6.0/build ${CONFIGURE_WITH_DEBUG}
	INSTALL_COMMAND ""
)


#add_subdirectory(ParallelNestedBenders)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
set(CMAKE_BUILD_TYPE Debug)
set(CONFIGURE_WITH_DEBUG --enable-debug)
set(INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdPartyLibs)
set(CPLEX_INCLUDE_DIR /home/christian/ILOG/CPLEX_Studio_AcademicResearch122/cplex/include/ilcplex)
set(CPLEX_LIBRARY_DIR /home/christian/ILOG/CPLEX_Studio_AcademicResearch122/cplex/lib/x86_sles10_4.1/static_pic)
set(GUROBI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gurobi/gurobi501/include)
set(GUROBI_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gurobi/gurobi501/linux32/lib)

ExternalProject_Add(CoinUtils
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CoinUtils"
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
)

ExternalProject_Add(Osi
	DEPENDS CoinUtils;
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Osi"
	#CONFIGURE_COMMAND <SOURCE_DIR>/configure "--with-cplex-incdir=${CPLEX_INCLUDE_DIR}" "--with-cplex-lib=-L${CPLEX_LIBRARY_DIR} -lcplex -lpthread" --with-gurobi-incdir=${GUROBI_INCLUDE_DIR} --with-gurobi-lib="-L${GUROBI_LIBRARY_DIR} -lgurobi_g++4.2" ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR} --with-coin-instdir=${INSTALL_DIR}
	CONFIGURE_COMMAND <SOURCE_DIR>/configure "--with-cplex-incdir=${CPLEX_INCLUDE_DIR}" "--with-cplex-lib=-L${CPLEX_LIBRARY_DIR} -lcplex -lpthread" ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR} --with-coin-instdir=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
)
ExternalProject_Add(Clp
	DEPENDS CoinUtils;Osi
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Clp"
	#BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR} --with-coin-instdir=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
	)

ExternalProject_Add(Cgl
	DEPENDS CoinUtils;Osi;Clp
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Cgl"
	#BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG} --with-coin-instdir=${INSTALL_DIR} --prefix=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
)


ExternalProject_Add(Cbc
	DEPENDS CoinUtils;Cgl;Clp;Osi
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Cbc"
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR} --with-coin-instdir=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
)

ExternalProject_Add(Smi
	DEPENDS CoinUtils;Cgl;Clp;Osi
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Smi"
	CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR} --with-coin-instdir=${INSTALL_DIR} --with-clp-incdir=${INSTALL_DIR} --with-clp-lib=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
)
ExternalProject_Add(google-glog
	#URL "http://google-glog.googlecode.com/files/glog-0.3.2.tar.gz"
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glog-0.3.2"
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
	)

# Just build without installing, use build paths from there
ExternalProject_Add(gtest
	DOWNLOAD_COMMAND ""
	#SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gtest-1.6.0"
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/gtest-1.6.0"
	#INSTALL_DIR ${INSTALL_DIR}
	INSTALL_COMMAND ""
	#CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=${INSTALL_DIR}
	)

ExternalProject_Add(FlopCpp
	DEPENDS CoinUtils;Cgl;Clp;Osi;Cbc;Smi;google-glog
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FlopCpp"
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-glog-incdir=${INSTALL_DIR}/include --with-glog-lib=${INSTALL_DIR}/lib --with-gtest-incdir=${INSTALL_DIR}/include --with-gtest-lib=${INSTALL_DIR}/lib ${CONFIGURE_WITH_DEBUG} --prefix=${INSTALL_DIR}
	INSTALL_DIR ${INSTALL_DIR}
)


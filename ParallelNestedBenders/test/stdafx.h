#include <gtest/gtest.h>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <limits>
#include <cmath>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <sys/stat.h>

#define GOOGLE_GLOG_DLL_DECL // Has to be done to correct linking issues
#include <glog/logging.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptor/map.hpp>

#include "OsiCpxSolverInterface.hpp"
//#include "OsiGrbSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"
#include "CoinTime.hpp"
#include "SmiScnModel.hpp"
#include "flopc.hpp"


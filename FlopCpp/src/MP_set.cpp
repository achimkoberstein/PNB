// ******************** FlopCpp **********************************************
// File: MP_set.cpp
//****************************************************************************

#include "MP_set.hpp"
#include "MP_model.hpp"
#include <sstream>
using namespace flopc;

namespace flopc {
  MP_set &MP_set::getEmpty() {
	  //Make it real static and const (Singleton)
	  return flopc::Empty; // Does this work?
  }
  

  MP_stage::MP_stage( int i /*= 0*/ ) : MP_set(i)
	  {
	  //During first initialization, currentModel pointer is zero. Under this circumstances,
	  //stage gets initalized with 0 by MP_model constructor
	  if (MP_model::getCurrentModel() != 0)
		MP_model::getCurrentModel()->Stage(*this);
	  }
	}

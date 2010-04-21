// ******************** FlopCpp **********************************************
// File: MP_variable.cpp
//****************************************************************************

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

#include <OsiSolverInterface.hpp>
#include "MP_variable.hpp"
#include "MP_domain.hpp" 
#include "MP_constant.hpp" 
#include "MP_model.hpp"
using namespace flopc;


MP_variable::MP_variable(const MP_set_base &s1, 
                         const MP_set_base &s2, 
                         const MP_set_base &s3,
                         const MP_set_base &s4, 
                         const MP_set_base &s5) :
RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
upperLimit(MP_model::getCurrentModel()->getInfinity()),
lowerLimit(0),
S1(&s1),S2(&s2),S3(&s3),S4(&s4),S5(&s5),
offset(-1)
{
    type = continuous;
} 

MP_variable::~MP_variable() { }

double MP_variable::level(int lcl_i1, int lcl_i2, int lcl_i3, int lcl_i4, int lcl_i5) { 
    assert(M != 0);
    assert(M->Solver != 0);
    return M->Solver->getColSolution()[offset +  f(lcl_i1,lcl_i2,lcl_i3,lcl_i4,lcl_i5)]; 
}  

void MP_variable::operator()() const {
    if (S1!=&MP_set::getEmpty()) cout << i1.evaluate() << " ";
    if (S2!=&MP_set::getEmpty()) cout << i2.evaluate() << " ";
    if (S3!=&MP_set::getEmpty()) cout << i3.evaluate() << " ";
    if (S4!=&MP_set::getEmpty()) cout << i4.evaluate() << " ";
    if (S5!=&MP_set::getEmpty()) cout << i5.evaluate() << " ";
    cout<<"  "<< M->Solver->getColSolution()[offset +
        f(i1.evaluate(),
        i2.evaluate(),
        i3.evaluate(),
        i4.evaluate(),
        i5.evaluate())]<<endl;
}

void MP_variable::display(const std::string &s) {
    cout<<s<<endl;
    if (offset >= 0) {
        ((*S1)(i1)*(*S2)(i2)*(*S3)(i3)*(*S4)(i4)*(*S5)(i5)).forall(this);
    } else {
        cout<<"No solution available!"<<endl;
    }
}

void MP_variable::bounds(std::vector<std::vector<boost::shared_ptr<MP::Coef>> >& cfs) {

    //Store Constant index expressions of upper and lower bound and reset them afterwards?! Just for the moment..
    // Create new MP_index? 
    //MP_index in1,in2,in3,in4,in5;
    //const VariableRef* var = new VariableRef(this,in1,in2,in3,in4,in5);
    //delete var;
    //const VariableRef& var = this->operator()(i1,i2,i3,i4,i5); //This is incorrect: We put MP_index in but we want a MP_index_exp. What happens to the indices of the limit Constants?!
    //MP::VariableBoundsFunctor f(&var, cfs);
    //((*S1)(i1)*(*S2)(i2)*(*S3)(i3)*(*S4)(i4)*(*S5)(i5)).forall(&f);
    //delete myrefs.back();
    //myrefs.pop_back();
}
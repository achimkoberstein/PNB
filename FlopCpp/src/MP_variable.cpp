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
#include "SmiScnModel.hpp"
#include "MP_index.hpp"
#include "CoinHelperFunctions.hpp"
using namespace flopc;


MP_variable::MP_variable(const MP_set_base &s1, 
                         const MP_set_base &s2, 
                         const MP_set_base &s3,
                         const MP_set_base &s4, 
                         const MP_set_base &s5) :
RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
upperLimit(s1,s2,s3,s4,s5),
lowerLimit(s1,s2,s3,s4,s5),
S1(&s1),S2(&s2),S3(&s3),S4(&s4),S5(&s5),
offset(-1),stageSetPtr(NULL)
{
    type = type::continuous;
    lowerLimit.initialize(0);
    upperLimit.initialize(MP_model::getCurrentModel()->getInfinity());
    setStageSet();
} 

MP_variable::~MP_variable() { }

double MP_variable::level(int lcl_i1, int lcl_i2, int lcl_i3, int lcl_i4, int lcl_i5) { 
    assert(M != 0);
    assert(M->Solver != 0);
    if (M->stage.size()) //TODO: This gives solution values for the first stage only..
        throw invalid_argument_exception();
    else
        return M->Solver->getColSolution()[offset +  f(lcl_i1,lcl_i2,lcl_i3,lcl_i4,lcl_i5)];

} 

double MP_variable::levelScenario(int scenario, int lcl_i1, int lcl_i2, int lcl_i3, int lcl_i4, int lcl_i5) { 
    assert(M != 0);
    assert(M->Solver != 0);
    if (M->colIndirection[offset+f(lcl_i1,lcl_i2,lcl_i3,lcl_i4,lcl_i5)] == outOfBound)
        throw invalid_argument_exception();
    if (M->stage.size()) //TODO: This gives solution values for the first stage only..
        return M->smiModel->getColSolution(scenario,getCurrentStage(lcl_i1,lcl_i2,lcl_i3,lcl_i4,lcl_i5),M->colIndirection[offset+f(lcl_i1,lcl_i2,lcl_i3,lcl_i4,lcl_i5)]);
    else
        throw invalid_argument_exception();

}  

void MP_variable::setStageSet(){
    if (S1->isStage()){
        stageSet = 0;
        stageSetPtr = const_cast<MP_set_base*>(S1);
    }
    else 
        if (S2->isStage()){
            stageSet = 1;
            stageSetPtr = const_cast<MP_set_base*>(S2);
        }
        else 
            if (S3->isStage()){
                stageSet = 2;
                stageSetPtr = const_cast<MP_set_base*>(S3);
            }
            else 
                if (S4->isStage()){
                    stageSet = 3;
                    stageSetPtr = const_cast<MP_set_base*>(S4);
                }
                else 
                    if (S5->isStage()){
                        stageSet = 4;
                        stageSetPtr = const_cast<MP_set_base*>(S5);
                    }
                    else
                        stageSet = outOfBound;
}

int MP_variable::getCurrentStage(int i1, int i2, int i3, int i4, int i5){
    switch (stageSet)
    {
    case 0: return i1;
    case 1: return i2;
    case 2: return i3;
    case 3: return i4;
    case 4: return i5;
    case outOfBound : return outOfBound;
    default : return outOfBound;
    }
}

void MP_variable::operator()() const {
    int curOffset = offset+ f(i1.evaluate(),
        i2.evaluate(),
        i3.evaluate(),
        i4.evaluate(),
        i5.evaluate());
    if ( offset > M->N()) // should take realn anyway
        assert(false);
    if ( M->colIndirection[curOffset] == outOfBound )
        return;

    std::stringstream string_stream; // Store everything in a string stream!
    if (S1!=&MP_set::getEmpty()) string_stream << i1.evaluate() << " ";
    if (S2!=&MP_set::getEmpty()) string_stream << i2.evaluate() << " ";
    if (S3!=&MP_set::getEmpty()) string_stream << i3.evaluate() << " ";
    if (S4!=&MP_set::getEmpty()) string_stream << i4.evaluate() << " ";
    if (S5!=&MP_set::getEmpty()) string_stream << i5.evaluate() << " ";
    // Problem when variables get added to a model via added constraints. This will not work then.
    // A variable can not be in two models at the same time obviously, or can it be?
    if ( M->getStochastic() ){ 
        int length = 0;
        double* values = 0;
        if ( stageSet >= 0){
            // get values for every scenario
            for (int i = 0; i < M->scenSet.size(); i++){
                values = M->smiModel->getColSolution(i,&length); // Gets column solution values for scenario, i.e. values for the core model!
                //TODO: Check length //Check colIndirection for outOfBound..
                //TODO: Terribly inefficient.. It really is.. We can enhance this by getting the correct colIndex via the 
                string_stream<< " Sc. " << i << ": " << values[M->colIndirection[curOffset]];
                ::free(values); //we need to use free, as smi callocs the values..
            }
            string_stream << endl;
        }
        else { 
            string_stream << " " << M->smiModel->getColSolution(0,0,M->colIndirection[curOffset]) << endl;
        }
    }
    else { 
        string_stream<<"  "<< M->Solver->getColSolution()[curOffset]<<endl;
    }

    // Output the string stream we build
    cout << string_stream.str();

}

void MP_variable::display(const std::string &s) {
    cout<<s<<endl;
    if (offset >= 0) {
        //((*S1)(i1)*(*S2)(i2)*(*S3)(i3)*(*S4)(i4)*(*S5)(i5)).forall(this);
		cout<<"MP_variable::display is broken!"<<endl;
    } else {
        cout<<"No solution available!"<<endl;
    }
}

void MP_variable::bounds(std::vector<std::vector<boost::shared_ptr<MP::Coef> > >& cfs) {

    //Store Constant index expressions of upper and lower bound and reset them afterwards?! Just for the moment..
    // Create new MP_index? 
    //MP_index in1,in2,in3,in4,in5;
    //const VariableRef* var = new VariableRef(this,in1,in2,in3,in4,in5);
    //delete var;
    const VariableRef& var = this->operator()(i1,i2,i3,i4,i5); //This is incorrect: We put MP_index in but we want a MP_index_exp. What happens to the indices of the limit Constants?!
    MP::VariableBoundsFunctor f(&var, cfs);
    ((*S1)(i1)*(*S2)(i2)*(*S3)(i3)*(*S4)(i4)*(*S5)(i5)).forall(&f);
    myrefs.pop_back();
}

void MP_variable::integer()
{
    type = type::discrete;
}

void MP_variable::binary()
{
    //upperLimit.initialize(1);
    lowerLimit.initialize(0);
    upperLimit.initialize(1);
    type = type::binary;
}

void MP_variable::free()
{
    lowerLimit.initialize(-MP_model::getCurrentModel()->getInfinity());
    upperLimit.initialize(MP_model::getCurrentModel()->getInfinity());
    if (type == type::binary)
        type = type::discrete;
}


const VariableRef& flopc::MP_variable::operator()( const MP_index_exp& d1 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d2 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d3 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d4 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d5 /*= MP_index_exp::getEmpty() */ )
{
    if (MP_model::getCurrentModel()->checkSemantic()){
        semanticCheck("MP_variable", this->getName(), S1->getIndex(),S2->getIndex(),S3->getIndex(), S4->getIndex(),S5->getIndex(),d1,d2,d3,d4,d5);
    }
    //TODO: Look at lower/upperLimits again
    //lowerLimit->propagateIndexExpressions(d1,d2,d3,d4,d5);
    //upperLimit->propagateIndexExpressions(d1,d2,d3,d4,d5);
    VariableRef* vPtr = new VariableRef(this, d1, d2, d3, d4, d5);
    myrefs.push_back(MP_expression(vPtr));
    return *vPtr;
}

void flopc::MP_variable::fix( double value )
{
    lowerLimit.initialize(value);
    upperLimit.initialize(value);
    // We do not change the type
}

void MP_variable::generateNames(std::set<string> &nameSetStrict, std::set<string> &nameSetFree, std::vector<std::string> &colNamesStrict, std::vector<std::string> &colNamesFree) {
    // we start with the names for the the free-type
    string nameFree;

    // build the prefix
    if (this->getName().length() > 0) {
        nameFree = this->getName();
    }
    else {
        nameFree = "V";
    }

    // if the prefix is not unique (i.e. used by another variable), extend it with a consecutively number
    if (!isUniqueName(nameFree, nameSetFree)) {
        int ext = 0;
        while (!isUniqueName(nameFree + to_string(ext), nameSetFree)) {
            ext++;
        }
        nameFree = nameFree + to_string(ext);
    }

    // the name is unique, register it
    nameSetFree.insert(nameFree);

    // generate the names with the suffixes for every instance of this variable and copy them into the array
    for (int k = 0; k < size(); k++) {
        if (this->isActive(k)) {
            string free = nameFree + this->generateNamesSuffixFree(k);
            // Insert at correct position
            colNamesFree[getOffsetedIndex(k)] = free.c_str();
        }
    }


    // now for the strict-type
    string nameStrict;

    // maybe we are lucky and can use the same names for the strict-type as for the free-type...
    if (isUniqueName(nameFree, nameSetStrict) && (nameFree.length() + this->generateNamesSuffixFree(size()-1).length()) <= 8) {
        nameStrict = nameFree;
        nameSetStrict.insert(nameStrict);
        for (int k = 0; k < size(); k++) {
            if (this->isActive(k)) {
                string strict = nameStrict + this->generateNamesSuffixFree(k);
                colNamesStrict[getOffsetedIndex(k)] = strict.c_str();
            }
        }
    }
    else {
        // use the offset as suffix
        bool genNames = true;
        if (nameFree.length() + 1 + to_string(size()).length() <= 8) {
            nameStrict = nameFree;
        }
        else {
            nameStrict = nameFree.substr(0, 8 - (to_string(size()).length() + 1));
            if (!isUniqueName(nameStrict, nameSetStrict)) {
                // if its not a unique name, use an extension
                int ext = 0;
                do {
                    if (ext % 10 == 0) { 
                        nameStrict = nameStrict.substr(0, nameStrict.length()-1);
                        if (nameStrict.length() == 0) 
                            genNames = false;  // names do not fit into array of length 8 - so skip it
                    }
                    ext++;
                } while (!isUniqueName(nameStrict + to_string(ext), nameSetStrict) && genNames);
                nameStrict = nameStrict + to_string(ext);
            }
        }
        if (genNames) {
            // register the name
            nameSetStrict.insert(nameStrict);
            //fill the array
            for (int k = 0; k < size(); k++) {
                if (this->isActive(k)) {
                    string strict = nameStrict + "_" + to_string(k);
                    colNamesStrict[getOffsetedIndex(k)] = strict.c_str();
                }
            }
        } else {
            // push empty strings, let the SmiSmpsWriter or whatever try it...
            string s = "";
            for (int k = 0; k < size(); k++) {
                if (this->isActive(k)) {
                    colNamesStrict[getOffsetedIndex(k)] = s.c_str();
                }
            }
        }
    }
}

bool MP_variable::isActive(const int k) {
    //assert(M != 0);
    //assert(M->Solver != 0);
    if (M->getStochastic() && M->colIndirection[offset+k] == outOfBound)
        return false;
    
    return true;
}

int MP_variable::getOffsetedIndex(const int k){
    if (M->getStochastic())
        return M->colIndirection[offset+k];
    else
        return offset+k;
}

string MP_variable::generateNamesSuffixFree(const int k) {
    if ( k == -1 ){
        return "_0";
    }
    IndexKeeper ik = getIndices(k);
    std::stringstream ss;
    if (S1 != &MP_set::getEmpty()) {
        ss << "_" << ik.i1;
        if (S2 != &MP_set::getEmpty()) {
            ss << "_" << ik.i2;
            if (S3 != &MP_set::getEmpty()) {
                ss << "_" << ik.i3;
                if (S4 != &MP_set::getEmpty()) {
                    ss << "_" << ik.i4;
                    if (S5 != &MP_set::getEmpty()) {
                        ss << "_" << ik.i5;
                    }
                }
            }
        }
    }
    else 
        return "_0";
    return ss.str();
}

// ******************** FlopCpp **********************************************
// File: MP_constraint.cpp
//****************************************************************************

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

#include <OsiSolverInterface.hpp>
#include "MP_constraint.hpp"
#include "MP_expression.hpp"
#include "MP_model.hpp"
#include "MP_constant.hpp"
#include "MP_data.hpp"

using namespace flopc;


void MP_constraint::operator=(const Constraint &v) {
    left = v->left;
    right = v->right;
    sense = v->sense;
}

int MP_constraint::row_number() const {
    // We can produce empty rows here, if we index our constraints, so that we leave the first stages out of the calculation. 
    int i1 = I1->getIndex()->evaluate();
    int i2 = I2->getIndex()->evaluate();
    int i3 = I3->getIndex()->evaluate();
    int i4 = I4->getIndex()->evaluate();
    int i5 = I5->getIndex()->evaluate();
    // Check against S?
    if (i1==outOfBound || i2==outOfBound || i3==outOfBound ||
        i4==outOfBound || i5==outOfBound) {
            return outOfBound;
    } else {
        return offset + f(i1,i2,i3,i4,i5);
    }
}

double MP_constraint::price(int i1, int i2, int i3, int i4, int i5) const {
    return  M->Solver->getRowPrice()[offset + f(i1,i2,i3,i4,i5)];
}

MP_constraint::MP_constraint(
                             const MP_set_base &s1, 
                             const MP_set_base &s2, 
                             const MP_set_base &s3,
                             const MP_set_base &s4, 
                             const MP_set_base &s5) :
RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
M(MP_model::current_model),
offset(-1),real_size(-1),
S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),
I1(0),I2(0),I3(0),I4(0),I5(0)
{
    M->add(*this);
}

void MP_constraint::coefficients(vector<boost::shared_ptr<MP::Coef> >& cfs) {
    MP::GenerateFunctor f(this, cfs);
    // f.setConstraint(this);

    vector<TerminalExpression*> v;
    if (left.isDefined() && right.isDefined()) {
        left->generate((S1(I1)*S2(I2)*S3(I3)*S4(I4)*S5(I5)).such_that(B),v,f,1.0);
        right->generate((S1(I1)*S2(I2)*S3(I3)*S4(I4)*S5(I5)).such_that(B),v,f,-1.0);
    } else {
        LOG(WARNING) << "FlopCpp Warning: Constraint declared but not defined." <<endl;
    }
    //TODO Delete pointer when in vector v TODO FIXME
}

void MP_constraint::insertVariables(set<MP_variable*>& v) {
    if (left.operator->()!=0) {
        left->insertVariables(v);
    }
    if (right.operator->()!=0) {
        right->insertVariables(v);
    }
}

void MP_constraint::insertRandomVariables(std::vector< std::set<RandomVariable*> >& v){
        if (left.operator->()!=0) {
        left->insertRandomVariables(v);
    }
    if (right.operator->()!=0) {
        right->insertRandomVariables(v);
    }
}


void MP_constraint::display(string s) const {
    cout<<s<<endl;
    if (offset >=0) {
        for (int i=offset; i<offset+size(); i++) {
            cout<<i<<"  "<<M->Solver->getRowLower()[i]<<"  "<<M->Solver->getRowActivity()[i]<<"  "<<M->Solver->getRowUpper()[i]<<"  "<<M->Solver->getRowPrice()[i]<<endl;
        }
    } else {
        cout<<"No solution available!"<<endl;
    }
}

void MP_constraint::generateNames(std::set<string> &nameSetStrict, std::set<string> &nameSetFree, std::vector<std::string> &rowNamesStrict, std::vector<std::string> &rowNamesFree) {
    if ( offset == -1 )
        return; // We do not process constraints that are not in the model!
    
    // we start with the names for the the free-type
    string nameFree;

    // build the prefix
    if (this->getName().length() > 0) {
        nameFree = this->getName();
    }
    else {
        nameFree = "R";
    }

    // if the prefix is not unique (i.e. used by another row), extend it with a consecutively number
    if (!isUniqueName(nameFree, nameSetFree)) {
        int ext = 0;
        while (!isUniqueName(nameFree + to_string(ext), nameSetFree)) {
            ext++;
        }
        nameFree = nameFree + to_string(ext);
    }

    // the name is unique, register it
    nameSetFree.insert(nameFree);

    // generate the names with the suffixes for every instance of this row and copy them into the array
    int current_row = 0;
    int string_length;
    for (int k = 0; k < size(); k++) {
        string free = nameFree + this->generateNamesSuffixFree(k);
        if ( free == nameFree ){
            continue;
        }
        rowNamesFree[getOffsetedIndex(current_row)] = free.c_str();
        ++current_row;
        if ( current_row == real_size )
            string_length = nameFree.length();
    }
    assert(current_row==real_size);
    

    // now for the strict-type
    string nameStrict;

    // maybe we are lucky and can use the same names for the strict-type as for the free-type...
    if (isUniqueName(nameFree, nameSetStrict) && (string_length  <= 8)) {
        nameStrict = nameFree; // We assume that the name we have is already enough
        nameSetStrict.insert(nameStrict);
        current_row = 0;
        for (int k = 0; k < size(); k++) {
            string strict = nameStrict + this->generateNamesSuffixFree(k);
            if ( strict == nameStrict ){
                continue;
            }
            rowNamesStrict[getOffsetedIndex(current_row)] = strict.c_str();
            ++current_row;
        }
        assert(current_row==real_size);
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
            for (int k = 0; k < real_size; k++) {
                string strict = nameStrict + "_" + to_string(k);
                rowNamesStrict[getOffsetedIndex(k)] = strict.c_str();
            }
        } else {
            // push empty strings, let the SmiSmpsWriter or whatever try it...
            string s = "";
            for (int k = 0; k < real_size; k++) {
                rowNamesStrict[getOffsetedIndex(k)] = s.c_str();
            }
        }
    }
}

int MP_constraint::getOffsetedIndex(const int k){
    if (M->getStochastic())
        return M->rowIndirection[offset+k];
    else
        return offset+k;
}

string MP_constraint::generateNamesSuffixFree(const int k) {
    IndexKeeper ik = getIndices(k);
    std::stringstream ss;
    if (&S1 != &MP_set::getEmpty()) { // What can do the trick: Get the index, assign it to the Index for this constraint and evaluate the index!
        I1->getIndex()->assign(ik.i1);
        I1->getIndex()->instantiate();
        double value = I1->evaluate();
        I1->getIndex()->unInstantiate();
        if ( S1.check(value) != outOfBound )
            ss << "_" << value;
        else
            return "";
        if (&S2 != &MP_set::getEmpty()) {
            I2->getIndex()->assign(ik.i2);
            I2->getIndex()->instantiate();
            value = I2->evaluate();
            I2->getIndex()->unInstantiate();
            if ( S2.check(value) != outOfBound )
                ss << "_" << value;
            else
                return "";
            if (&S3 != &MP_set::getEmpty()) {
                I3->getIndex()->assign(ik.i3);
                I3->getIndex()->instantiate();
                value = I3->evaluate();
                I3->getIndex()->unInstantiate();
                if ( S3.check(value) != outOfBound )
                    ss << "_" << value;
                else
                    return "";
                if (&S4 != &MP_set::getEmpty()) {
                    I4->getIndex()->assign(ik.i4);
                    I4->getIndex()->instantiate();
                    value = I4->evaluate();
                    I4->getIndex()->unInstantiate();
                    if ( S4.check(value) != outOfBound )
                        ss << "_" << value;
                    else
                        return "";
                    if (&S5 != &MP_set::getEmpty()) {
                        I5->getIndex()->assign(ik.i5);
                        I5->getIndex()->instantiate();
                        value = I5->evaluate();
                        I5->getIndex()->unInstantiate();
                        if ( S1.check(value) != outOfBound )
                            ss << "_" << value;
                        else
                            return "";
                    }
                }
            }
        }
    }
    else {
        // Constraint without any set's, also possible
        return "_0";
    }
    bool bool_value = true;
    if ( B.operator->() != NULL )
        bool_value = B->evaluate();
    if ( bool_value )
        return ss.str();
    else 
        return "";
}
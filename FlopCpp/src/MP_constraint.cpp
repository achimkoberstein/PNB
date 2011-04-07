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
    int i1 = S1.check(I1->evaluate());
    int i2 = S2.check(I2->evaluate());
    int i3 = S3.check(I3->evaluate());
    int i4 = S4.check(I4->evaluate());
    int i5 = S5.check(I5->evaluate());

    if (i1==outOfBound || i2==outOfBound || i3==outOfBound ||
        i4==outOfBound || i5==outOfBound) {
            return outOfBound;
    } else {
        return offset + f(I1->evaluate(),I2->evaluate(),I3->evaluate(),
            I4->evaluate(),I5->evaluate()); 
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
offset(-1),
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
    for (int k = 0; k < size(); k++) {
        // Look at empty rows
        if ( getOffsetedIndex(k) == outOfBound )
            continue;
        string free = nameFree + this->generateNamesSuffixFree(k);
        rowNamesFree[getOffsetedIndex(k)] = free.c_str();
    }


    // now for the strict-type
    string nameStrict;

    // maybe we are lucky and can use the same names for the strict-type as for the free-type...
    if (isUniqueName(nameFree, nameSetStrict) && (nameFree.length() + this->generateNamesSuffixFree(size()-1).length()) <= 8) {
        nameStrict = nameFree;
        nameSetStrict.insert(nameStrict);
        for (int k = 0; k < size(); k++) {
            // Look at empty rows
            if ( getOffsetedIndex(k) == outOfBound )
                continue;
            string strict = nameStrict + this->generateNamesSuffixFree(k);
            rowNamesStrict[getOffsetedIndex(k)] = strict.c_str();
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
                // Look at empty rows
                if ( getOffsetedIndex(k) == outOfBound )
                    continue;
                string strict = nameStrict + "_" + to_string(k);
                rowNamesStrict[getOffsetedIndex(k)] = strict.c_str();
            }
        } else {
            // push empty strings, let the SmiSmpsWriter or whatever try it...
            string s = "";
            for (int k = 0; k < size(); k++) {
                // Look at empty rows
                if ( getOffsetedIndex(k) == outOfBound )
                    continue;
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
    if (&S1 != &MP_set::getEmpty()) {
        ss << "_" << ik.i1;
        if (&S2 != &MP_set::getEmpty()) {
            ss << "_" << ik.i2;
            if (&S3 != &MP_set::getEmpty()) {
                ss << "_" << ik.i3;
                if (&S4 != &MP_set::getEmpty()) {
                    ss << "_" << ik.i4;
                    if (&S5 != &MP_set::getEmpty()) {
                        ss << "_" << ik.i5;
                    }
                }
            }
        }
    }
    return ss.str();
}
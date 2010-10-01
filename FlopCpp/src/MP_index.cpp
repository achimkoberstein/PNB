// ******************** FlopCpp **********************************************
// File: MP_index.cpp
//****************************************************************************

#include "MP_index.hpp"
#include "MP_domain.hpp"
#include "MP_set.hpp"
#include "MP_model.hpp"

namespace flopc {
    MP_index &MP_index::getEmpty() {
        static MP_index Empty;
        return Empty;
    }
    MP_index &MP_index::Any() {
        static MP_index Any_index;
        return Any_index;
    }

    MP_index_base* MP_index::deepCopy() const
    {
        return getIndex();
    }

    MP_index_base* MP_index::insertIndexExpr( const MP_index_exp& expr)
    { // We should replace this MP_index with the MP_index_exp we got. How can this possibly work?
        if (this->getIndex() == expr->getIndex()){
            return expr.operator->();
        }
        else{ // We should check for indices, which can be instantiated during the display and generation method
            return this->getIndex();
        }
        throw not_implemented_error();
        return 0;
    }
    const MP_index_exp &MP_index_exp::getEmpty() {
        static MP_index_exp Empty(Constant(0.0));
        return Empty;
    }

    flopc::MP_index_exp MP_index_exp::deepCopyIndexExpression() const
    {
        return MP_index_exp( operator->()->deepCopy());
    }

    // Wrapper around templatized SubsetRef, that does the actual work.
    class MP_index_subset : public MP_index_base {
        friend class MP_index_exp;
    private:
        MP_index_subset(const SUBSETREF& s) : S(&s) {}
        int evaluate() const {
            return S->evaluate(); 
        }
        MP_index* getIndex() const {
            return S->getIndex();
        }
        virtual MP_domain getDomain(MP_set* s) const{
            return MP_domain(S->getDomain(s));
        }
        virtual MP_index_base* deepCopy() const { return S->deepCopy(); }
        virtual MP_index_base* insertIndexExpr(const MP_index_exp& expr) { 
            return (const_cast<SUBSETREF*>(S))->insertIndexExpr(expr);    
        }

        const SUBSETREF* S;
    };

    MP_index_exp operator+(MP_index& i,const Constant& j) {
        return new MP_index_sum(i,j);
    }

    MP_index_exp operator+(MP_index& i,const int& j) {
        return new MP_index_sum(i,Constant(j));
    }

    MP_index_exp operator-(MP_index& i,const Constant& j) {
        return new MP_index_dif(i,j);
    }

    MP_index_exp operator-(MP_index& i,const int& j) {
        return new MP_index_dif(i,Constant(j));
    }

    MP_index_exp operator*(MP_index& i,const Constant& j) {
        return new MP_index_mult(i,j);
    }



    flopc::MP_index_base* MP_index_mult::deepCopy() const
    {
        return new MP_index_mult(dynamic_cast<MP_index&>(*left->getIndex()),right);
    }

    MP_index_base* MP_index_mult::insertIndexExpr( const MP_index_exp& expr) 
    {
        if (expr->getIndex() == left->getIndex()){
            left = expr;
            return this;
        }
        else {
            LOG(ERROR) << "Most probably wrong indexation for a multiplication.";
            throw invalid_argument_exception();
        }
        return this;
        

    }

    flopc::MP_index_base* MP_index_sum::deepCopy() const
    {
        return new MP_index_sum(dynamic_cast<MP_index&>(*left->getIndex()),right);
    }

    MP_index_base* MP_index_sum::insertIndexExpr( const MP_index_exp& expr)
    {
        if (expr->getIndex() == left->getIndex()){
            left = expr;
        }
        else {
            LOG(ERROR) << "Most probably wrong indexation for a sum.";
            throw invalid_argument_exception();
        }
        return this;

    }

    flopc::MP_index_base* MP_index_dif::deepCopy() const
    {
        return new MP_index_dif(dynamic_cast<MP_index&>(*left->getIndex()),right);
    }

    MP_index_base* MP_index_dif::insertIndexExpr( const MP_index_exp& expr)
    {
        if (expr->getIndex() == left->getIndex()){
            left = expr;
       }
        else {
            LOG(ERROR) << "Most probably wrong indexation for a difference.";
            throw invalid_argument_exception();
        }
        return this;

    }

    MP_domain MP_index_constant::getDomain( MP_set* s ) const
    {
        return MP_domain::getEmpty();
    }

    void semanticCheck( const std::string& type, const std::string& name, const MP_index* S1, const MP_index* S2, const MP_index* S3, const MP_index* S4, const MP_index* S5, const MP_index_exp& d1, const MP_index_exp& d2, const MP_index_exp& d3, const MP_index_exp& d4,const MP_index_exp& d5 )
    {
        // We also have to check if we index with more indices than we assigned.// typename
        LOG_IF(ERROR,S1 == &MP_set::getEmpty() && &d1 != &MP_index_exp::getEmpty() ) << "You indexed a " << type << " that has no set. This is an error.";
        LOG_IF(ERROR,S2 == &MP_set::getEmpty() && &d2 != &MP_index_exp::getEmpty() ) << "You indexed a " << type << " with two indices, but only one set. This is an error.";
        LOG_IF(ERROR,S3 == &MP_set::getEmpty() && &d3 != &MP_index_exp::getEmpty() ) << "You indexed a " << type << " with three indices, but only two sets. This is an error.";
        LOG_IF(ERROR,S4 == &MP_set::getEmpty() && &d4 != &MP_index_exp::getEmpty() ) << "You indexed a " << type << " with four indices, but only three sets. This is an error.";
        LOG_IF(ERROR,S5 == &MP_set::getEmpty() && &d5 != &MP_index_exp::getEmpty() ) << "You indexed a " << type << " with five indices, but only four sets. This is an error.";
        LOG_IF(WARNING,S1 != &MP_set::getEmpty() && d1->getIndex() != S1 && d1->getDomain(&MP_set::getEmpty()) != MP_domain::getEmpty()  ) << "First index given to " << type << " with name " << name << " does not correspond to the defined index. This may lead to subtle errors. Continue only if you know what you are doing.";
        LOG_IF(WARNING,S2 != &MP_set::getEmpty() && d2->getIndex() != S2 && d2->getDomain(&MP_set::getEmpty()) != MP_domain::getEmpty()) << "Second index given to " << type << " with name " << name << " does not correspond to the defined index. This may lead to subtle errors. Continue only if you know what you are doing.";
        LOG_IF(WARNING,S3 != &MP_set::getEmpty() && d3->getIndex() != S3 && d3->getDomain(&MP_set::getEmpty()) != MP_domain::getEmpty()) << "Third index given to " << type << " with name " << name << " does not correspond to the defined index. This may lead to subtle errors. Continue only if you know what you are doing.";
        LOG_IF(WARNING,S4 != &MP_set::getEmpty() && d4->getIndex() != S4 && d4->getDomain(&MP_set::getEmpty()) != MP_domain::getEmpty()) << "Fourth index given to " << type << " with name " << name << " does not correspond to the defined index. This may lead to subtle errors. Continue only if you know what you are doing.";
        LOG_IF(WARNING,S5 != &MP_set::getEmpty() && d5->getIndex() != S5 && d5->getDomain(&MP_set::getEmpty()) != MP_domain::getEmpty()) << "Fifth index given to " << type << " with name " << name << " does not correspond to the defined index. This may lead to subtle errors. Continue only if you know what you are doing.";
    }
} // End of namespace flopc

using namespace flopc;


MP_domain MP_index::getDomain(MP_set* s) const{
    return new MP_domain_set(s,const_cast<MP_index*>(this)) ; 
}

MP_domain MP_index_mult::getDomain(MP_set* s) const{
    return left->getDomain(s); 
}

MP_domain MP_index_sum::getDomain(MP_set* s) const{
    return left->getDomain(s); 
}

MP_domain MP_index_dif::getDomain(MP_set* s) const{
    return left->getDomain(s);
}

MP_index_exp::MP_index_exp(int i) : 
Handle<MP_index_base*>(new MP_index_constant(Constant(i))) {} 

MP_index_exp::MP_index_exp(const SUBSETREF& s) : 
Handle<MP_index_base*>(new MP_index_subset(s)) {}

MP_index_exp::MP_index_exp(const Constant& c) : 
Handle<MP_index_base*>(new MP_index_constant(c)) {}

MP_index_exp::MP_index_exp(MP_index& i) : //Create MP_index_exp from MP_index reference.. Maybe we want to create an MP_index_exp from an MP_index?
Handle<MP_index_base*>(&i) { operator->()->count++; }

MP_index_exp::MP_index_exp(const MP_index_exp &other): //Create a MP_index_expression from another one by using this cast?!
Handle<MP_index_base*>((const Handle<MP_index_base*> &)other) { }


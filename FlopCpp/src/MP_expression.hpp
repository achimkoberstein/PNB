// ******************** FlopCpp **********************************************
// File: MP_expression.hpp
// ****************************************************************************

#ifndef _MP_expression_hpp_
#define _MP_expression_hpp_

#include <vector>
#include <set>
using std::vector;
using std::set;

#include "MP_domain.hpp"
#include "MP_constant.hpp"
#include "MP_utilities.hpp"
#include "MP_random_variable.hpp"



namespace flopc {

    class MP_constraint;
    class TerminalExpression; 
    class MP_variable;
    class VariableRef;
    class RandomVariableRef;

    class MP {
        friend class MP_expression;
        friend class MP_constraint;
        friend class MP_model;
        friend class Messenger;
        friend class VerboseMessenger; 
        friend class CoefLess;
        friend class MP_expression_base;
        friend class VariableRef;
        friend class RandomVariableRef;


        struct Coef {
            Coef(int c, int r, double v, int s = 0,RandomVariable* ptr = 0) : 
        col(c), row(r), stage(s), val(v), ptrVar(ptr)  {}
        int col, row, stage;
        double val;
        RandomVariable* ptrVar;
        //Col = Column Number of Coef
        //Row = Row Number of Coef
        //Stage = Stage of belonging Variable (Column)
        //Val = Coefficient value
        //ptrVar = ptr to a RandomVariable, if data is not stochastic, set to 0.

        };

        static bool CoefLess (const MP::Coef& a, const MP::Coef& b);
        static bool CoefLessWithStage  (const MP::Coef& a, const MP::Coef& b);
        /*struct CoefLess {
        bool operator()  const;
        };

        struct CoefLessWithStage {
        bool operator() (const MP::Coef& a, const MP::Coef& b) const;
        };*/

    protected:
        //This class is used to set all coefficients of an constraint to 
        class GenerateFunctor : public Functor {
        public:
            GenerateFunctor(MP_constraint* r, vector<Coef>& cfs): R(r), Coefs(cfs) {}

            void setMultiplicator(vector<Constant>& mults, double m) {
                multiplicators = mults;
                M = m;
            }
            void setTerminalExpression(const TerminalExpression* c) {
                C = c;
            }

            void operator()() const;
        private:
            //Disable copy contructor and assignment operator
            GenerateFunctor(const GenerateFunctor&);
            GenerateFunctor& operator=(const GenerateFunctor&);


            MP_constraint* R;
            //Temporary values
            vector<Constant> multiplicators; //Coefficients?
            double M; // sign (1=lhs, -1=rhs)
            const TerminalExpression* C; //Current 
            //End Temporary values
            vector<MP::Coef>& Coefs;
        };
    };


    /** @brief The base class for all expressions.
    @ingroup INTERNAL_USE
    @note FOR INTERNAL USE: This is not normally used directly by the
    calling code.
    */
    class MP_expression_base {
        friend class MP_expression;
        friend class Handle<MP_expression_base*>;
    private:
        int count;
    public:
        MP_expression_base() : count(0) {}

        //TODO: Comments needed. What do these methods do?
        virtual double level() const = 0; 
        virtual void generate(const MP_domain& domain,
            vector<Constant> multiplicators,
            MP::GenerateFunctor &f,
            double m) const = 0;

        virtual void insertVariables(set<MP_variable *>& v) const = 0; //Insert given Variables to the Model?!

        virtual ~MP_expression_base() {}
    };


    /** @brief Symbolic representation of a linear expression.
    @ingroup PublicInterface
    This is one of the main public interface classes.  It is the basis for   
    all linear expressions, including constraints, objective function,
    and expressions involving indexes.
    <br> Although these can be created directly and independently, it
    is expected these will be created through the use of the operators
    which are later in this file.  (operator+, operator-, etc.)
    @note There are constructors which are (silently) used to convert \
    other componenets into expressions.
    */

    class MP_expression : public Handle<MP_expression_base*> {
    public:
        MP_expression() : Handle<MP_expression_base*>(0) {}
        /// Constructor which (silently) converts a Constant to a MP_expression
        MP_expression(const Constant& c);
        /// Constructor which (silently) converts a VariableRef to a MP_expression
        MP_expression(const VariableRef& v);
        ////Constructor which silently converts a RandomVariable to a MP_expression
        //MP_expression(const RandomVariableRef& rv);
        MP_expression(MP_expression_base* r) : Handle<MP_expression_base*>(r) {}
    };


    /** @brief The base class for all expressions.
    @ingroup INTERNAL_USE
    @note FOR INTERNAL USE: This is not normally used directly by the
    calling code.
    @todo can this be moved to the cpp file?
    */
    class TerminalExpression : public MP_expression_base {
    public:
        virtual ~TerminalExpression() {};

        virtual double getValue() const = 0; 
        virtual int getColumn() const = 0;
        virtual int getStage() const = 0;
        virtual RandomVariable* getRandomVariable() const = 0;
    };

    /** Semantic representation of a variable in a Math Program
    @ingroup INTERNAL_USE
    @see MP_variable for a public interface.
    */
    class VariableRef : public TerminalExpression {
        friend class MP_variable;
    public:
        virtual int getColumn() const;
    private:
        VariableRef(MP_variable* v, 
            const MP_index_exp& i1,
            const MP_index_exp& i2,
            const MP_index_exp& i3,
            const MP_index_exp& i4,
            const MP_index_exp& i5);

        //VariableRef& operator=(const &VariableRef); Why is there a compiler warning about assignment operator can not be generated? due to the private constructor?

        double level() const;

        void insertVariables(set<MP_variable*>& v) const {
            v.insert(V);
        }
        virtual double getValue() const { 
            return 1.0;
        }
        virtual int getStage() const;

        virtual RandomVariable* getRandomVariable() const {
            return 0;
        }

        void generate(const MP_domain& domain,
            vector<Constant> multiplicators,
            MP::GenerateFunctor& f,
            double m) const;

        MP_variable* V;
        int offset;
        const MP_index_exp I1,I2,I3,I4,I5;
    };



    /** Semantic representation of a random variable in a Math Program
    @ingroup INTERNAL_USE
    @see MP_variable for a public interface.
    */


    /// @ingroup PublicInterface
    MP_expression operator+(const MP_expression& e1, const MP_expression& e2);
    /// @ingroup PublicInterface
    MP_expression operator+(const MP_expression& e1, const Constant& e2);
    /// @ingroup PublicInterface
    MP_expression operator+(const Constant& e1, const MP_expression& e2);
    /// @ingroup PublicInterface
    MP_expression operator-(const MP_expression& e1, const MP_expression& e2);
    /// @ingroup PublicInterface
    MP_expression operator-(const MP_expression& e1, const Constant& e2);
    /// @ingroup PublicInterface
    MP_expression operator-(const Constant& e1, const MP_expression& e2);
    /// @ingroup PublicInterface
    MP_expression operator*(const Constant& e1, const MP_expression& e2); 
    /// @ingroup PublicInterface
    MP_expression operator*(const MP_expression& e1, const Constant& e2);
    /// @ingroup PublicInterface
    MP_expression operator/(const MP_expression& e1, const Constant& e2);
    /// @ingroup PublicInterface
    MP_expression sum(const MP_domain& d, const MP_expression& e);

} // End of namespace flopc
#endif

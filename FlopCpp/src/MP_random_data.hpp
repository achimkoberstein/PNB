#ifndef _MP_random_variable_hpp_
#define _MP_random_variable_hpp_

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "MP_data.hpp"
#include "MP_index.hpp"
#include "MP_set.hpp"


namespace flopc{

    class MP_random_data_impl;
    class RandomDataRef;

    int round(double x);


    //Class that defines a concrete RandomVariable for one coefficient
    class RandomVariable : boost::noncopyable{
        friend RandomDataRef;
    public:
        typedef std::vector< std::pair< double, double > > sampleVector;
        typedef std::vector< std::vector< RandomVariable* > > randVarVector;

        RandomVariable();
        virtual ~RandomVariable();

        //Samples a number of random variates according to the distribution of the RandomVariable
        // It also sorts the variates in ascending order
        virtual void sample(int howMany = 0) = 0;
        std::vector< std::pair<double,double> > getSampledValuesWithProb() const;
        std::vector<double> getSampledValues() const;
        double getSampledValue(int index) const;

        int Stage() const { return stage; }
        void Stage(int val) { stage = val; }
        int ColIndex() const { return colIndex; }
        void ColIndex(int val) { colIndex = val; }
        int RowIndex() const { return rowIndex; }
        void RowIndex(int val) { rowIndex = val; }
        int SampleSize() const { return defaultSampleSize; }
        void SampleSize(int val) { defaultSampleSize = val; }
        double AdditiveConstant() const { return additiveConstant; }
        void AdditiveConstant(double val) { additiveConstant = val; }
        double MultiplicativeConstant() const { return multiplicativeConstant; }
        void MultiplicativeConstant(double val) { multiplicativeConstant = val; }
        int Seed() const { return seed; }
        void Seed(int val) { seed = val; }
    protected:
        //Boost::random number generator? Needs seed
        unsigned int seed;

        //We only need one way to store the values. Speed vs. Storage and what methods are called should determine which way is better.
        //We start with the simple solution: vector of values
        std::vector<double> values; //All values are equally probable
        std::map<double,double> valuesMap; //Value with it's probability. One can get the number of occurences by : prob/1 (and round that)
        int defaultSampleSize;
        double additiveConstant;
        double multiplicativeConstant;

    private:
        int stage; //Defaults to 0. If this is the case when this variable should get used, we have an error.
        int colIndex;
        int rowIndex;

        //flopc::RandomVariable& operator=(const flopc::RandomVariable &);

    };

    class DiscreteRandomVariable : public RandomVariable {
        
    public:

        ~DiscreteRandomVariable();

        virtual void sample(int howMany);

    };

    class DiscreteUniformRandomVariable : public DiscreteRandomVariable {
    public :
        DiscreteUniformRandomVariable(int from, int to, double stepWidth);
        ~DiscreteUniformRandomVariable();

        virtual void sample(int howMany);

    private:
        //Params for DiscreteUniformDistribution: [start,end] in steps of step
        int start;
        int end;
        double step;

    };

    class ScenarioRandomVariable : public DiscreteRandomVariable{

    public:
        ScenarioRandomVariable(double* scenValues, int length);
        ScenarioRandomVariable();

        ~ScenarioRandomVariable();
        virtual void sample(int howMany) ;

    };

    class ContinuousRandomVariable : public RandomVariable {
    public:

        ~ContinuousRandomVariable();

        virtual void sample(int howMany) ;

    };

    class ContinuousUniformRandomVariable : public ContinuousRandomVariable {
    public:
        ContinuousUniformRandomVariable(int from, int to);
        ~ContinuousUniformRandomVariable();

        virtual void sample(int howMany) ;

    private:
        int start;
        int end;
    };

    




    //Class that defines indexable random_variables. Contains concrete RandomVariables.
    class MP_random_data : public Functor, public Named //, public boost::enable_shared_from_this //We want to allow names, computation of flattened arrays and functors
    {
    public:

        MP_random_data(const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty());

        MP_random_data(RandomVariable** array, const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty());

        MP_random_data(double* array, const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty());

        virtual void operator()() const;

        //TODO: Assignment of random variables via this operator is not yet possible
        //Needs virtual assignment operators hierarchy that might look ugly
        RandomVariable*& operator()(int lcli1, int lcli2=0, int lcli3=0, int lcli4=0, int lcli5=0);

        const boost::shared_ptr<RandomDataRef> operator()( 
            const MP_index_exp& d1 = MP_index_exp::getEmpty(), 
            const MP_index_exp& d2 = MP_index_exp::getEmpty(), 
            const MP_index_exp& d3 = MP_index_exp::getEmpty(),
            const MP_index_exp& d4 = MP_index_exp::getEmpty(), 
            const MP_index_exp& d5 = MP_index_exp::getEmpty()
            );

    protected:

    private:
        //TODO: Think about copying behaviour?
        MP_random_data(const MP_random_data& rhs); //forbid copy constructor
        MP_random_data& operator=(const MP_random_data& rhs); //forbid assignment operator
        boost::shared_ptr<MP_random_data_impl> ptrImpl;


    };

    class RandomDataRef : public Functor { //Are we really a constant_base object?
        friend class MP_random_data_impl;
    public:

        RandomDataRef(boost::shared_ptr<MP_random_data_impl> rv, 
            const MP_index_exp& i1,
            const MP_index_exp& i2,
            const MP_index_exp& i3,
            const MP_index_exp& i4,
            const MP_index_exp& i5);


        ~RandomDataRef() {} 
        RandomDataRef& such_that(const MP_boolean& b);

        //Get Value via current settings for MP_index_exp
        double evaluate() const;
        int getStage() const;
        //Assignment-operators (Not yet defined..)
        const RandomDataRef& operator=(const RandomDataRef& r); 
        const RandomDataRef& operator=(const Constant& c);
        //Copy Constructor (needed in initialization of Constant_random_variable
        RandomDataRef(const RandomDataRef&);

        //Set value via current MP_index_exp 
        //void evaluate_lhs(const RandomVariable &v) const;
        virtual void operator()() const;
        RandomVariable* RandomDataRef::getRandomVariable() const;

    private:
        boost::shared_ptr<MP_random_data_impl> RV;
        const MP_index_exp I1,I2,I3,I4,I5;
        Constant C;
        MP_boolean B;
    };

    //non-member non-friend functions
    std::ostream &operator<<( std::ostream &out, const RandomVariable& rhs ) ;
   

}


#endif 
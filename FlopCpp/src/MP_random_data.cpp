#include "MP_random_data.hpp"
#include <exception>
#include <stdexcept>
#include <vector>
#include <set>

#include <boost/ptr_container/ptr_array.hpp>
#include <boost/random.hpp>

#include "MP_expression.hpp"
#include "MP_model.hpp"


namespace flopc{


    typedef boost::mt19937 base_generator_type; //Set RNG
    typedef boost::variate_generator<base_generator_type&, boost::uniform_01<double> > uniform_distr_gen;
    

    //Class that stores a description of a random variable. Still needs to be defined
    //TODO: Do that
    class MP_random_data_impl : public RowMajor, public Functor , public Named {
        friend class RandomDataRef;

    public:
        //If we use this constructor, we need to delete values in the end.
        // This is only necessary if we actually add random_variables to this MP_random_data
        MP_random_data_impl(const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty()):
        RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
            S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),values(new RandomVariable*[size()]),manageData(true) { 
                initialize();
       }; 

        //Do a shallow copy (leave values in original arrays). This means we do not want to delete values in the end
        MP_random_data_impl(RandomVariable** array, const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty()):
        RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
            S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),values(new RandomVariable*[size()]),manageData(true) {  //Consistency check?
                initialize();
                // Very ugly code..
                // What is done here: Arrays stage index is one less than normal stage index because first stage needs no RV. 
                int j1i,j2i,j3i,j4i,j5i;
                j1i =j2i = j3i = j4i = j5i = 0;
                if (MP_model::getCurrentModel()->Stage().size() > 0){
                    if (s1.getName() == MP_model::stageString)
                        j1i = 1;
                    else if (S2.getName() == MP_model::stageString)
                        j2i = 1;
                    else if (S3.getName() == MP_model::stageString)
                        j3i = 1;
                    else if (S4.getName() == MP_model::stageString)
                        j4i = 1;
                    else if (S5.getName() == MP_model::stageString)
                        j5i = 1;
                }
                for (int j1 = j1i; j1 < S1.size();j1++)
                    for (int j2 = j2i; j2 < S2.size(); j2++)
                        for (int j3 = j3i; j3 < S3.size(); j3++)
                            for (int j4 = j4i; j4 < S4.size();j4++)
                                for (int j5 = j5i; j5 < S5.size();j5++){
                                    if (!(j1 < j1i || j2 < j2i || j3 < j3i || j4 < j4i || j5 < j5i)) //In case of not first stage: assign value
                                        values[f(j1,j2,j3,j4,j5)] = array[f(j1-j1i,j2-j2i,j3-j3i,j4-j4i,j5-j5i)];
                                }
        }; 

        //Do a shallow copy (leave values in original arrays). This means we do want to delete the created RandomVariables
        MP_random_data_impl(double* array, const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty()):
        RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
            S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),values(new RandomVariable*[size()]),manageData(true) {  //Consistency check?
                initialize();
                int scen = MP_model::getCurrentModel()->ScenSet().size();
                //Identify stage set: The set with getStage > 0 is the stage set.
                //If there is no stage set: No Problem ;)
                int j1i,j2i,j3i,j4i,j5i;
                j1i =j2i = j3i = j4i = j5i = 0;
                if (MP_model::getCurrentModel()->Stage().size() > 0){
                    if (s1.getName() == MP_model::stageString)
                        j1i = 1;
                    else if (S2.getName() == MP_model::stageString)
                        j2i = 1;
                    else if (S3.getName() == MP_model::stageString)
                        j3i = 1;
                    else if (S4.getName() == MP_model::stageString)
                        j4i = 1;
                    else if (S5.getName() == MP_model::stageString)
                        j5i = 1;
                }
                for (int j1 = j1i; j1 < S1.size();j1++)
                    for (int j2 = j2i; j2 < S2.size(); j2++)
                        for (int j3 = j3i; j3 < S3.size(); j3++)
                            for (int j4 = j4i; j4 < S4.size();j4++)
                                for (int j5 = j5i; j5 < S5.size();j5++){
                                    if (!(j1 < j1i || j2 < j2i || j3 < j3i || j4 < j4i || j5 < j5i)) //In case of not first stage
                                        createRandomVariableFromIndices(array,f(j1-j1i,j2-j2i,j3-j3i,j4-j4i,j5-j5i),f(j1,j2,j3,j4,j5),scen);
                                }
        };

        ~MP_random_data_impl(){
            if(manageData){//Iterate through array and delete pointer one-by-one.
                for (int i = 0; i < size(); i++)
                        delete values[i];
                delete [] values;
            }

        };

        //initialize data members 
        void initialize(); 


        virtual void operator()() const;

        RandomVariable*& operator()(int lcli1, int lcli2=0, int lcli3=0, int lcli4=0, int lcli5=0)  { 
            lcli1 = S1.check(lcli1); 
            lcli2 = S2.check(lcli2); 
            lcli3 = S3.check(lcli3); 
            lcli4 = S4.check(lcli4); 
            lcli5 = S5.check(lcli5); 
            int i = f(lcli1,lcli2,lcli3,lcli4,lcli5);     
            if (i == outOfBound) {
                throw std::out_of_range("Index is not valid"); //Nothrow guarantee?
            } else {
                return values[i];
            }
        }


    private:
        void createRandomVariableFromIndices(double* arr,int indexForArr,int indexForValues, int scen){
            double *tempArray;
            tempArray = new double[scen]; //new/delete
            for (int i = 0; i < scen; i++){
                tempArray[i] = arr[indexForArr*scen+i];
            }
            values[indexForValues] = new ScenarioRandomVariable(tempArray,scen);
            delete [] tempArray;
        }



        const MP_set_base &S1,&S2,&S3,&S4,&S5;
        MP_index i1,i2,i3,i4,i5;
        //boost::ptr_vector<RandomVariable> values;
        RandomVariable** values; //Array of pointers to RandomVariables
        bool manageData;
    };



    //Implementation for MP_random_data
    MP_random_data::MP_random_data( const MP_set_base &s1 /*= MP_set::getEmpty()*/, const MP_set_base &s2 /*= MP_set::getEmpty()*/, const MP_set_base &s3 /*= MP_set::getEmpty()*/, const MP_set_base &s4 /*= MP_set::getEmpty()*/, const MP_set_base &s5 /*= MP_set::getEmpty()*/ )
        : ptrImpl(new MP_random_data_impl(s1,s2,s3,s4,s5)){	}

    MP_random_data::MP_random_data( RandomVariable** array, const MP_set_base &s1 /*= MP_set::getEmpty()*/, const MP_set_base &s2 /*= MP_set::getEmpty()*/, const MP_set_base &s3 /*= MP_set::getEmpty()*/, const MP_set_base &s4 /*= MP_set::getEmpty()*/, const MP_set_base &s5 /*= MP_set::getEmpty()*/ )
        : ptrImpl(new MP_random_data_impl(array,s1,s2,s3,s4,s5)) {}

    MP_random_data::MP_random_data( double* array, const MP_set_base &s1 /*= MP_set::getEmpty()*/, const MP_set_base &s2 /*= MP_set::getEmpty()*/, const MP_set_base &s3 /*= MP_set::getEmpty()*/, const MP_set_base &s4 /*= MP_set::getEmpty()*/, const MP_set_base &s5 /*= MP_set::getEmpty()*/ )
        : ptrImpl(new MP_random_data_impl(array,s1,s2,s3,s4,s5))	{

    }
    RandomVariable*& MP_random_data::operator ()(int lcli1, int lcli2, int lcli3, int lcli4, int lcli5){
        return ptrImpl->operator()(lcli1,lcli2,lcli3,lcli4,lcli5);
    }

    const boost::shared_ptr<RandomDataRef> MP_random_data::operator()( /*TODO: Move to .cpp */ const MP_index_exp& d1 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d2 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d3 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d4 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d5 /*= MP_index_exp::getEmpty() */ )
    {
        boost::shared_ptr<RandomDataRef> temp(new RandomDataRef(ptrImpl, d1, d2, d3, d4, d5));
        return temp; //TODO: Who calls corresponding delete?!
    }

    void MP_random_data::operator()() const
    {

    }
    //Implementation for MP_random_data_impl
    void MP_random_data_impl::operator ()() const {
        if (&S1!=&MP_set::getEmpty()) cout << i1.evaluate() << " ";
        if (&S2!=&MP_set::getEmpty()) cout << i2.evaluate() << " ";
        if (&S3!=&MP_set::getEmpty()) cout << i3.evaluate() << " ";
        if (&S4!=&MP_set::getEmpty()) cout << i4.evaluate() << " ";
        if (&S5!=&MP_set::getEmpty()) cout << i5.evaluate() << " ";
        cout<<"  "<< values[f(i1.evaluate(),i2.evaluate(),i3.evaluate(),
            i4.evaluate(),i5.evaluate())] << endl;

    }

    void MP_random_data_impl::initialize() //TODO: How to do that in abetter way? Think about that..
    {
        for (int i = 0; i < size(); i++)
        	values[i] = 0;
    }


    //Implementation for RandomVariable: Think about these constructors.. are they necessary?
    RandomVariable::RandomVariable() : seed(0),values(),valuesMap(),defaultSampleSize(0),additiveConstant(0),multiplicativeConstant(1),stage(0),colIndex(0),rowIndex(0) {
        
    } 




    //const RandomVariable& RandomVariable::operator=( const RandomVariable& rhs )
    //{
    //    if( this == &rhs) //handle assignment to self //item 12
    //        return *this;
    //    //another way: item 12 and item 29 : swap method needs to be implemented
    //    //RandomVariable temp(rhs);
    //    //swap(rhs);
    //    //TODO: Delete values from *this and copy values from rhs to this. Return *this.
    //    stage = rhs.stage;
    //    defaultSampleSize = rhs.defaultSampleSize;
    //    colIndex = rhs.colIndex;
    //    rowIndex = rhs.rowIndex;
    //    values = rhs.values;
    //    valuesMap = rhs.valuesMap;
    //    return *this; //No data values so far in RandomVariable, so we do not have anything to copy..
    //}

    RandomVariable::~RandomVariable()
    {

    }

    std::vector<double> RandomVariable::getSampledValues() const
    {
        return values;
    }

    RandomVariable::sampleVector RandomVariable::getSampledValuesWithProb() const
    {
        //Create vector that stores pairs of value with number of occurences. HashSet is ideal to store that..
        sampleVector temp;
        return temp;
    }

    double RandomVariable::getSampledValue(int index) const
    {
        return values[index];
    }

    void RandomVariable::sample( int howMany /*= 0*/ )
    {
        //Initialization stuff common to all methods..
        if (howMany == 0) // If method was called with argument = 0, sample from defaultSampleSize.
            howMany = defaultSampleSize;
        values.clear();
        values.reserve(howMany);
    }

    double RandomVariable::applyConstant( double val )
    {
        if (multiplicativeConstant != 1){
            val *= multiplicativeConstant;
        }
        if( additiveConstant != 0){
            val+=additiveConstant;
        }
        return val;
    }
    //flopc::RandomVariable& RandomVariable::operator=( const flopc::RandomVariable & )
    //{
    //    return *this;
    //}
    //Implementation for RandomDataRef.

    RandomDataRef::RandomDataRef(boost::shared_ptr<MP_random_data_impl> rv, 
        const MP_index_exp& i1,
        const MP_index_exp& i2,
        const MP_index_exp& i3,
        const MP_index_exp& i4,
        const MP_index_exp& i5) :
    RV(rv),I1(i1),I2(i2),I3(i3),I4(i4),I5(i5),C(0) { 
    }

    RandomDataRef::RandomDataRef( const RandomDataRef& ref)
        :
    RV(ref.RV),
        I1(ref.I1),
        I2(ref.I2),
        I3(ref.I3),
        I4(ref.I4),
        I5(ref.I5),
        C(ref.C),
        B(ref.B)
    {}

    //void RandomDataRef::evaluate_lhs( const RandomVariable &rv ) const
    //{
    //    //Check sanity of indices
    //    int i1 = RV->S1.check(I1->evaluate());
    //    int i2 = RV->S2.check(I2->evaluate());
    //    int i3 = RV->S3.check(I3->evaluate());
    //    int i4 = RV->S4.check(I4->evaluate());
    //    int i5 = RV->S5.check(I5->evaluate());
    //    //Compute index in flattened multidimensional array
    //    int i = RV->f(i1,i2,i3,i4,i5);
    //    // Set Value at computed index to v
    //    if (i != outOfBound) { //evaluate_lhs: When is this function actually called?
    //        *RV->values[i] = rv;
    //    }
    //}

    double RandomDataRef::evaluate() const
    {
        //Check for sanity of indices
        int i1 = RV->S1.check(I1->evaluate());
        int i2 = RV->S2.check(I2->evaluate());
        int i3 = RV->S3.check(I3->evaluate());
        int i4 = RV->S4.check(I4->evaluate());
        int i5 = RV->S5.check(I5->evaluate());	
        // Compute index in flattened multidimensional array
        int i = RV->f(i1,i2,i3,i4,i5);
        // Return value
        if ( i ==  outOfBound ) {
            return 0;
        } else {
            //TODO: What to return? Expected value?
            //return RV->values[i];
            return 1; //Fix this
        }
    }

    RandomVariable* RandomDataRef::getRandomVariable() const
    {
        //Check for sanity of indices
        int i1 = RV->S1.check(I1->evaluate());
        int i2 = RV->S2.check(I2->evaluate());
        int i3 = RV->S3.check(I3->evaluate());
        int i4 = RV->S4.check(I4->evaluate());
        int i5 = RV->S5.check(I5->evaluate());	
        // Compute index in flattened multidimensional array
        int i = RV->f(i1,i2,i3,i4,i5);
        // Return value
        if ( i ==  outOfBound ) {
            return 0;
        } else {
            //TODO: What to return? Expected value?
            //return RV->values[i];
            return RV->values[i]; //Fix this
        }
    }

    RandomDataRef& RandomDataRef::such_that( const MP_boolean& b )
    {
        B = b;
        return *this;
    }

    int RandomDataRef::getStage() const
    {	//We do not know which of these sets is the stage set, so we have to check all (but could abort after we found one)
        int i1 = RV->S1.checkStage(I1->evaluate());
        int i2 = RV->S2.checkStage(I2->evaluate());
        int i3 = RV->S3.checkStage(I3->evaluate());
        int i4 = RV->S4.checkStage(I4->evaluate());
        int i5 = RV->S5.checkStage(I5->evaluate());

        int stage = 0; //Set stage to the correct value. Only one int should be larger than 0.
        if (i1>stage) stage = i1;
        else if (i2>stage) stage = i2;
        else if (i3>stage) stage = i3;
        else if (i4>stage) stage = i4;
        else if (i5>stage) stage = i5;
        return stage;
    }


    void RandomDataRef::operator()() const
    {
        throw not_implemented_error();
        //TODO: If C is not a constant_random_variable, something went wrong. Obviously
        //evaluate_lhs(C->evaluate());
    }
    std::ostream & operator<<( std::ostream &out, const RandomVariable& rhs )
    {
        return out;
    }
    //Implementation for ScenarioRandomVariable
    ScenarioRandomVariable::ScenarioRandomVariable( double* scenValues,int length)
        : DiscreteRandomVariable()
    {
        this->values = std::vector<double>(scenValues,scenValues+length);
    }

    ScenarioRandomVariable::ScenarioRandomVariable() :
    DiscreteRandomVariable()
    {

    }

    void ScenarioRandomVariable::sample(int howMany) 
    {
        //Apply the constant values to every element.
        for (std::vector<double>::size_type i = 0; i < values.size();++i){
            values[i] = applyConstant(values[i]);
        }
        //Ensure that values are in ascending order
        //std::sort(values.begin(),values.end());
    }

    ScenarioRandomVariable::~ScenarioRandomVariable()
    {

    }

    //const ScenarioRandomVariable& ScenarioRandomVariable::operator=( const ScenarioRandomVariable& rhs )
    //{
    //    if( this == &rhs)
    //        return *this;
    //    //Copy all values
    //    //Copy base class
    //    DiscreteRandomVariable::operator=(rhs);
    //    return *this;
    //}

    //Implementation for ContinuousRandomVariable
    void ContinuousRandomVariable::sample(int howMany) 
    {
        //call std::sort
    }

    ContinuousRandomVariable::~ContinuousRandomVariable()
    {

    }
    //Implementation for DiscreteRandomVariable
    void DiscreteRandomVariable::sample(int howMany)
    {
        //call std::sort
    }

    DiscreteRandomVariable::~DiscreteRandomVariable()
    {

    }

    //const DiscreteRandomVariable& DiscreteRandomVariable::operator=( const DiscreteRandomVariable& rhs )
    //{
    //    if (this == &rhs)
    //        return *this;

    //    RandomVariable::operator=(rhs);
    //    return *this;

    //}



    DiscreteUniformRandomVariable::DiscreteUniformRandomVariable( int from, int to, double stepWidth ): DiscreteRandomVariable(),start(from),end(to),step(stepWidth)
    { // Ensure correct functionality
        //TODO: Include test for stepWidth
        if (end < start){
            int temp = start;
            start = end;
            end = temp;
        }
        if (step < 0){
            step = std::fabs(step);
        }
        else if (step == 0.0){
            throw invalid_argument_exception();
        }
    }

    DiscreteUniformRandomVariable::~DiscreteUniformRandomVariable()
    {

    }

    void DiscreteUniformRandomVariable::sample( int howMany )
    {
        //Initialize data structure
        RandomVariable::sample(howMany);
        //Define RNG and 01UniformRNG using RNG
        base_generator_type rng(seed);
        uniform_distr_gen uniform_01_generator(rng,boost::uniform_01<double>() );
        
        // We want to compute values from start to end with a stepWidth of step (double)
        // So we first compute size of domain : (end-start)*stepWidth
        int domainSize = round((end-start)/step);
        // Then generate the variates: start+step*random_variate
        typedef boost::uniform_int<> distr_type;
        boost::variate_generator<uniform_distr_gen&,distr_type> variateGen(uniform_01_generator,distr_type(0,domainSize));
        for (int i = 0; i < howMany; ++i){
            values.push_back(applyConstant(start+step*variateGen()));
        }
        // Ensure values are in ascending order
        //std::sort(values.begin(),values.end());

    }

   

    ContinuousUniformRandomVariable::ContinuousUniformRandomVariable( int from, int to ) : ContinuousRandomVariable(),start(from),end(to)
    {
        if (end < start){
            int temp = start;
            start = end;
            end = start;
        }

    }

    ContinuousUniformRandomVariable::~ContinuousUniformRandomVariable()
    {

    }

    void ContinuousUniformRandomVariable::sample( int howMany )
    {
        //Initialize data structures
        RandomVariable::sample(howMany);
        //Define RNG and 01UniformRNG using RNG
        base_generator_type rng(seed);
        uniform_distr_gen uniform_01_generator(rng,boost::uniform_01<double>() );
        // Typedef to ease use and allow copy code ;)
        typedef boost::uniform_real<> distr_type;
        double domainSize = end-start;
        boost::variate_generator<uniform_distr_gen,distr_type > variateGen(uniform_01_generator,distr_type(0,domainSize));
        for (int i = 0; i < howMany; ++i){
            values.push_back(applyConstant(start+variateGen()));
        }
        //Ensure ascending order
        //std::sort(values.begin(),values.end());
    }
}
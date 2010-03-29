#include "MP_random_variable.hpp"
#include <exception>
#include <stdexcept>
#include <vector>

#include <boost/ptr_container/ptr_array.hpp>

#include "MP_expression.hpp"
#include "MP_model.hpp"


namespace flopc{



    //Class that stores a description of a random variable. Still needs to be defined
    //TODO: Do that
    class MP_random_variable_impl : public RowMajor, public Functor , public Named {
        friend class RandomVariableRef;

    public:
        //If we use this constructor, we need to delete values in the end.
        // This is only necessary if we actually add random_variables to this MP_random_variable
        MP_random_variable_impl(const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty()):
        RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
            S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),values(new RandomVariable*[size()]),manageData(true) { 
                initialize();//TODO: Rethink about that, if done correctly..
        }; 

        //Do a shallow copy (leave values in original arrays). This means we do not want to delete values in the end
        MP_random_variable_impl(RandomVariable** array, const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty()):
        RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
            S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),values(array),manageData(false) {  //Consistency check?
        }; 

        //Do a shallow copy (leave values in original arrays). This means we do want to delete the created RandomVariables
        MP_random_variable_impl(double* array, const MP_set_base &s1 = MP_set::getEmpty(), 
            const MP_set_base &s2 = MP_set::getEmpty(), 
            const MP_set_base &s3 = MP_set::getEmpty(),
            const MP_set_base &s4 = MP_set::getEmpty(), 
            const MP_set_base &s5 = MP_set::getEmpty()):
        RowMajor(s1.size(),s2.size(),s3.size(),s4.size(),s5.size()),
            S1(s1),S2(s2),S3(s3),S4(s4),S5(s5),values(new RandomVariable*[size()]),manageData(true) {  //Consistency check?
                int scen = MP_model::getCurrentModel()->ScenSet().size(); 
                for (int j1 = 0; j1 < S1.size();j1++)
                    for (int j2 = 0; j2 < S2.size(); j2++)
                        for (int j3 = 0; j3 < S3.size(); j3++)
                            for (int j4 = 0; j4 < S4.size();j4++)
                                for (int j5 = 0; j5 < S5.size();j5++){
                                    createRandomVariableFromIndices(array,f(j1,j2,j3,j4,j5),scen);
                                }
        };


        ~MP_random_variable_impl(){
            if(manageData){//Iterate through array and delete pointer one-by-one.
                for (int i = 0; i < size(); i++)
                        delete values[i];
                delete [] values;
            }

        };

        //initialize data members 
        void initialize(); 


        virtual void operator()() const;

        RandomVariable* operator()(int lcli1, int lcli2=0, int lcli3=0, int lcli4=0, int lcli5=0)  { 
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
        void createRandomVariableFromIndices(double* arr,int indexForValues, int scen){
            double *tempArray;
            tempArray = new double[scen]; //new/delete
            for (int i = 0; i < scen; i++){
                tempArray[i] = arr[indexForValues*scen+i];
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



    //Implementation for MP_random_variable
    MP_random_variable::MP_random_variable( const MP_set_base &s1 /*= MP_set::getEmpty()*/, const MP_set_base &s2 /*= MP_set::getEmpty()*/, const MP_set_base &s3 /*= MP_set::getEmpty()*/, const MP_set_base &s4 /*= MP_set::getEmpty()*/, const MP_set_base &s5 /*= MP_set::getEmpty()*/ )
        : ptrImpl(new MP_random_variable_impl(s1,s2,s3,s4,s5)){	}

    MP_random_variable::MP_random_variable( RandomVariable** array, const MP_set_base &s1 /*= MP_set::getEmpty()*/, const MP_set_base &s2 /*= MP_set::getEmpty()*/, const MP_set_base &s3 /*= MP_set::getEmpty()*/, const MP_set_base &s4 /*= MP_set::getEmpty()*/, const MP_set_base &s5 /*= MP_set::getEmpty()*/ )
        : ptrImpl(new MP_random_variable_impl(array,s1,s2,s3,s4,s5)) {}

    MP_random_variable::MP_random_variable( double* array, const MP_set_base &s1 /*= MP_set::getEmpty()*/, const MP_set_base &s2 /*= MP_set::getEmpty()*/, const MP_set_base &s3 /*= MP_set::getEmpty()*/, const MP_set_base &s4 /*= MP_set::getEmpty()*/, const MP_set_base &s5 /*= MP_set::getEmpty()*/ )
        : ptrImpl(new MP_random_variable_impl(array,s1,s2,s3,s4,s5))	{

    }
    RandomVariable* MP_random_variable::operator ()(int lcli1, int lcli2, int lcli3, int lcli4, int lcli5){
        return ptrImpl->operator()(lcli1,lcli2,lcli3,lcli4,lcli5);
    }

    const boost::shared_ptr<RandomVariableRef> MP_random_variable::operator()( /*TODO: Move to .cpp */ const MP_index_exp& d1 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d2 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d3 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d4 /*= MP_index_exp::getEmpty()*/, const MP_index_exp& d5 /*= MP_index_exp::getEmpty() */ )
    {
        boost::shared_ptr<RandomVariableRef> temp(new RandomVariableRef(ptrImpl, d1, d2, d3, d4, d5));
        return temp; //TODO: Who calls corresponding delete?!
    }

    void MP_random_variable::operator()() const
    {

    }
    //Implementation for MP_random_variable_impl
    void MP_random_variable_impl::operator ()() const {
        if (&S1!=&MP_set::getEmpty()) cout << i1.evaluate() << " ";
        if (&S2!=&MP_set::getEmpty()) cout << i2.evaluate() << " ";
        if (&S3!=&MP_set::getEmpty()) cout << i3.evaluate() << " ";
        if (&S4!=&MP_set::getEmpty()) cout << i4.evaluate() << " ";
        if (&S5!=&MP_set::getEmpty()) cout << i5.evaluate() << " ";
        cout<<"  "<< values[f(i1.evaluate(),i2.evaluate(),i3.evaluate(),
            i4.evaluate(),i5.evaluate())] << endl;

    }

    void MP_random_variable_impl::initialize() //TODO: How to do that in abetter way? Think about that..
    {
        for (int i = 0; i < size(); i++)
        	values[i] = new DiscreteRandomVariable();
    }


    //Implementation for RandomVariable: Think about these constructors.. are they necessary?
    RandomVariable::RandomVariable() : values(),valuesMap(),defaultSampleSize(0),additiveConstant(0),multiplicativeConstant(1),stage(0),colIndex(0),rowIndex(0) {} 




    const RandomVariable& RandomVariable::operator=( const RandomVariable& rhs )
    {
        if( this == &rhs) //handle assignment to self //item 12
            return *this;
        //another way: item 12 and item 29 : swap method needs to be implemented
        //RandomVariable temp(rhs);
        //swap(rhs);
        //TODO: Delete values from *this and copy values from rhs to this. Return *this.
        stage = rhs.stage;
        defaultSampleSize = rhs.defaultSampleSize;
        colIndex = rhs.colIndex;
        rowIndex = rhs.rowIndex;
        values = rhs.values;
        valuesMap = rhs.valuesMap;
        return *this; //No data values so far in RandomVariable, so we do not have anything to copy..
    }

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
    //Implementation for RandomVariableRef.

    RandomVariableRef::RandomVariableRef(boost::shared_ptr<MP_random_variable_impl> rv, 
        const MP_index_exp& i1,
        const MP_index_exp& i2,
        const MP_index_exp& i3,
        const MP_index_exp& i4,
        const MP_index_exp& i5) :
    RV(rv),I1(i1),I2(i2),I3(i3),I4(i4),I5(i5),C(0) { 
    }

    RandomVariableRef::RandomVariableRef( const RandomVariableRef& ref)
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

    void RandomVariableRef::evaluate_lhs( const RandomVariable &rv ) const
    {
        //Check sanity of indices
        int i1 = RV->S1.check(I1->evaluate());
        int i2 = RV->S2.check(I2->evaluate());
        int i3 = RV->S3.check(I3->evaluate());
        int i4 = RV->S4.check(I4->evaluate());
        int i5 = RV->S5.check(I5->evaluate());
        //Compute index in flattened multidimensional array
        int i = RV->f(i1,i2,i3,i4,i5);
        // Set Value at computed index to v
        if (i != outOfBound) {
            *RV->values[i] = rv;
        }
    }

    double RandomVariableRef::evaluate() const
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

    RandomVariable* RandomVariableRef::getRandomVariable() const
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

    RandomVariableRef& RandomVariableRef::such_that( const MP_boolean& b )
    {
        B = b;
        return *this;
    }

    int RandomVariableRef::getStage() const
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


    void RandomVariableRef::operator()() const
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
        //We have fixed number of scenarios. Sample command does nothing except from adding constant terms to values.
        if( additiveConstant != 0){
            for (std::vector<double>::size_type i = 0; i < values.size(); i++)
                values[i] += additiveConstant;
        }
        if (multiplicativeConstant != 1){
            for (std::vector<double>::size_type i = 0; i < values.size(); i++)
                values[i] *= multiplicativeConstant;
        }
    }

    ScenarioRandomVariable::~ScenarioRandomVariable()
    {

    }

    const ScenarioRandomVariable& ScenarioRandomVariable::operator=( const ScenarioRandomVariable& rhs )
    {
        if( this == &rhs)
            return *this;
        //Copy all values
        //Copy base class
        DiscreteRandomVariable::operator=(rhs);
        return *this;
    }

    //Implementation for ContinuousRandomVariable
    void ContinuousRandomVariable::sample(int howMany) 
    {

    }

    ContinuousRandomVariable::~ContinuousRandomVariable()
    {

    }
    //Implementation for DiscreteRandomVariable
    void DiscreteRandomVariable::sample(int howMany)
    {

    }

    DiscreteRandomVariable::~DiscreteRandomVariable()
    {

    }

    const DiscreteRandomVariable& DiscreteRandomVariable::operator=( const DiscreteRandomVariable& rhs )
    {
        if (this == &rhs)
            return *this;

        RandomVariable::operator=(rhs);
        return *this;

    }


}
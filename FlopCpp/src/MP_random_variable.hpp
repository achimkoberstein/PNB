#ifndef _MP_random_variable_hpp_
#define _MP_random_variable_hpp_

#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#include "MP_data.hpp"
#include "MP_index.hpp"
#include "MP_set.hpp"


namespace flopc{

	class MP_random_variable_impl;
	class RandomVariableRef;


	//Class that defines a concrete RandomVariable for one coefficient
	class RandomVariable {
		friend RandomVariableRef;
	public:
		typedef enum { BINOMIAL, NORMAL } DistributionType; //TODO: Class hierarchy will make this superfluous

		RandomVariable();
		RandomVariable(const DistributionType &distrType);
		virtual ~RandomVariable();

		//Samples a random variate according to distribution of RandomVariable.
		virtual double sample() const = 0;
		virtual std::vector< std::pair<double,double> > sampleValues(int elements) const = 0;

		


		//get of DistributionType..
		const flopc::RandomVariable::DistributionType DistrType() const { return distrType; }
		int Stage() const { return stage; }
		void Stage(int val) { stage = val; }
		int ColIndex() const { return colIndex; }
		void ColIndex(int val) { colIndex = val; }
		int RowIndex() const { return rowIndex; }
		void RowIndex(int val) { rowIndex = val; }

	protected:
		const RandomVariable& operator=(const RandomVariable& rhs);

	private:

		DistributionType distrType;
		int stage; //Defaults to 0. If this is the case when this variable should get used, we have an error.
		int colIndex;
		int rowIndex;
		int defaultSampleSize;

		};

	class DiscreteRandomVariable : public RandomVariable {
		typedef std::vector< std::pair< double, double > > sampleVector;
	public:

		~DiscreteRandomVariable();

		const DiscreteRandomVariable& operator=(const DiscreteRandomVariable& rhs);

		virtual double sample() const;
		virtual sampleVector sampleValues(int elementsToSample) const;


		};

	class ContinuousRandomVariable : public RandomVariable {
	public:

		~ContinuousRandomVariable();

		virtual double sample() const;
		virtual std::vector< std::pair<double,double> > sampleValues(int elements) const;

		};

	class ScenarioRandomVariable : public DiscreteRandomVariable{

	public:
		ScenarioRandomVariable(double* scenValues, int length);
		ScenarioRandomVariable();

		~ScenarioRandomVariable();
		virtual double sample() const;
		virtual std::vector< std::pair<double,double> > sampleValues(int elements) const;
		
		//Length of Scenario values should be stored in ScenSet of MP_model..
		const double* getScenarioValues() const;

		//Assignment operator
		const ScenarioRandomVariable& operator=(const ScenarioRandomVariable& rhs);
	private:
		ScenarioRandomVariable (const ScenarioRandomVariable&); //forbid copy operator
		std::vector<double> scenarioValues;

		};




	//Class that defines indexable random_variables. Contains concrete RandomVariables.
	class MP_random_variable : public Functor, public Named //, public boost::enable_shared_from_this //We want to allow names, computation of flattened arrays and functors
		{
		public:

			MP_random_variable(const MP_set_base &s1 = MP_set::getEmpty(), 
				const MP_set_base &s2 = MP_set::getEmpty(), 
				const MP_set_base &s3 = MP_set::getEmpty(),
				const MP_set_base &s4 = MP_set::getEmpty(), 
				const MP_set_base &s5 = MP_set::getEmpty());

			MP_random_variable(RandomVariable** array, const MP_set_base &s1 = MP_set::getEmpty(), 
				const MP_set_base &s2 = MP_set::getEmpty(), 
				const MP_set_base &s3 = MP_set::getEmpty(),
				const MP_set_base &s4 = MP_set::getEmpty(), 
				const MP_set_base &s5 = MP_set::getEmpty());

			MP_random_variable(double* array, const MP_set_base &s1 = MP_set::getEmpty(), 
				const MP_set_base &s2 = MP_set::getEmpty(), 
				const MP_set_base &s3 = MP_set::getEmpty(),
				const MP_set_base &s4 = MP_set::getEmpty(), 
				const MP_set_base &s5 = MP_set::getEmpty());

			virtual void operator()() const;

			//TODO: Assignment of random variables via this operator is not yet possible
			//Needs virtual assignment operators hierarchy that might look ugly
			RandomVariable& operator()(int lcli1, int lcli2=0, int lcli3=0, int lcli4=0, int lcli5=0);

			const boost::shared_ptr<RandomVariableRef> operator()( 
				const MP_index_exp& d1 = MP_index_exp::getEmpty(), 
				const MP_index_exp& d2 = MP_index_exp::getEmpty(), 
				const MP_index_exp& d3 = MP_index_exp::getEmpty(),
				const MP_index_exp& d4 = MP_index_exp::getEmpty(), 
				const MP_index_exp& d5 = MP_index_exp::getEmpty()
				);

		protected:

		private:
			//TODO: Think about copying behaviour?
			MP_random_variable(const MP_random_variable& rhs); //forbid copy constructor
			MP_random_variable& operator=(const MP_random_variable& rhs); //forbid assignment operator
			boost::shared_ptr<MP_random_variable_impl> ptrImpl;


		};

	class RandomVariableRef : public Functor { //Are we really a constant_base object?
		friend class MP_random_variable_impl;
	public:

		RandomVariableRef(boost::shared_ptr<MP_random_variable_impl> rv, 
			const MP_index_exp& i1,
			const MP_index_exp& i2,
			const MP_index_exp& i3,
			const MP_index_exp& i4,
			const MP_index_exp& i5);


		~RandomVariableRef() {} 
		RandomVariableRef& such_that(const MP_boolean& b);

		//Get Value via current settings for MP_index_exp
		double evaluate() const;
		int getStage() const;
		//Assignment-operators (Not yet defined..)
		const RandomVariableRef& operator=(const RandomVariableRef& r); 
		const RandomVariableRef& operator=(const Constant& c);
		//Copy Constructor (needed in initialization of Constant_random_variable
		RandomVariableRef(const RandomVariableRef&);

		//Set value via current MP_index_exp 
		void evaluate_lhs(const RandomVariable &v) const;
		virtual void operator()() const;
		RandomVariable* RandomVariableRef::getRandomVariable() const;

	private:
		boost::shared_ptr<MP_random_variable_impl> RV;
		const MP_index_exp I1,I2,I3,I4,I5;
		Constant C;
		MP_boolean B;
		};

	//non-member non-friend functions
	std::ostream &operator<<( std::ostream &out, const RandomVariable& rhs ) ;

	}


#endif 
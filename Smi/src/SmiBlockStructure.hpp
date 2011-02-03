#ifndef SMIBLOCKSTRUCTURE_HPP
#define SMIBLOCKSTRUCTURE_HPP

#include "SmiScnData.hpp"
#include "SmiLinearData.hpp"
#include <vector>

using namespace ::std;


/** This class is used to store the data of an SMPS "Block" section.
    It inherits from SmiLinearData and additonally stores the
    probability of the BLOCK element.
*/
class SmiBlockEvent : public SmiLinearData {

public:
    /// Sets the probability for this block element
	inline void setBlockEventProb(double p) {prob_ = p;}
	
	/// Returns the probability for this block element
	inline const double getBlockEventProb () { return prob_; }
	
	/// Creates a new SmiBlockEvent with SmiLinearData d and probability p
	SmiBlockEvent(SmiLinearData d, double p):SmiLinearData(d),prob_(p){}
	~SmiBlockEvent(){}
private:
    /// The probability
	double prob_;
};

/** This class is used to store several SmiBlockEvents.
*/
class SmiBlock{
	 
private:
    /// A vector containing all added SmiBlockEvents
	vector<SmiBlockEvent*> events_;
	
	/// The stage index of this SmiBlock
	SmiStageIndex  stg_;
public:
    /// Adds an SmiBlockEvent to the list
	void addBlockEvent(CoinPackedMatrix &matrix,
				CoinPackedVector &dclo, CoinPackedVector &dcup,
				CoinPackedVector &dobj,
				CoinPackedVector &drlo, CoinPackedVector &drup, double prob)
	{
		SmiLinearData d(matrix,dclo,dcup,dobj,drlo,drup);
		SmiBlockEvent *e = new SmiBlockEvent(d,prob);
		events_.push_back(e);

	}

	inline const CoinPackedMatrix &getBlockEventMatrix(int i)   {return events_[i]->getMatrix(); }   /// Returns the matrix of the i-th SmiBlockEvent
	inline const CoinPackedVector &getBlockEventColLower(int i) {return events_[i]->getColLower();}  /// Returns the column lower bounds of the i-th SmiBlockEvent
	inline const CoinPackedVector &getBlockEventColUpper(int i) {return events_[i]->getColUpper();}  /// Returns the column upper bounds of the i-th SmiBlockEvent
	inline const CoinPackedVector &getBlockEventObjective(int i){return events_[i]->getObjective();} /// Returns the objective of the i-th SmiBlockEvent
	inline const CoinPackedVector &getBlockEventRowLower(int i) {return events_[i]->getRowLower();}  /// Returns the row lower bounds of the i-th SmiBlockEvent
	inline const CoinPackedVector &getBlockEventRowUpper(int i) {return events_[i]->getRowUpper();}  /// Returns the row upper bounds of the i-th SmiBlockEvent
	
	/// Returns the probability of the i-th SmiBlockEvent
	double getBlockEventProb(int i) { 
			return events_[i]->getBlockEventProb();
	}
	
	/// Returns the number of SmiBlockEvents
	inline size_t getNumEvents() { return events_.size(); }
	
	/// Returns the stage of this SmiBlock
	inline int getStage() {return stg_;}
	
	/// Sets the stage of this SmiBlock
	inline void setStage(int p) {stg_=p;}
	
	/// Creates an SmiBlock with stage = -1
	SmiBlock(){stg_=-1;}
	
	/// Creates an SmiBlock with probability p
	SmiBlock(int p) {stg_=p;}
	~SmiBlock(){
		for (size_t i=0; i<events_.size(); ++i)
			delete events_[i];
	}
};

/** This class stores all data from several SMPS Blocks and is used for 
    prcoessing the block structure into scenarios.
*/
class SmiBlockStructure{

private:

	/// A vector containing the SmiBlock elements
	vector<SmiBlock*> smiBlock_;
	
	/// The SmiCoreData object
	SmiCoreData *core_;
	
	/// The combine rule (add or replace)
	SmiCoreCombineRule *combineRule_;

public:
	/// Adds SmiBlock s to the end of the list
	void addSmiBlock(SmiBlock *s)
	{ smiBlock_.push_back(s); }

    /// Deletes the last SmiBlock from the list
	void deleteSmiBlock(){
		smiBlock_.pop_back();
	}

	/// Returns the i-th SmiBlock
	SmiBlock * getBlock(int i) {return smiBlock_[i];}

	/// Returns the number of SmiBlocks
	int getNumBlocks() { return (int)smiBlock_.size(); }

	/// Returns the SmiCoreData object
	SmiCoreData *getCore(){ return core_; }

	/// Sets the SmiCoreCombineRule
	inline void setCombineWithCoreRule(SmiCoreCombineRule *r){
			combineRule_ = r;
	}

	/// Returns the SmiCoreCombine rule
	inline SmiCoreCombineRule *getCombineWithCoreRule() { return combineRule_;}

	
	/** Constructs a new SmiBlockStructure with SmiCoreData c and SmiCoreCombineRule r
	@param c the SmiCoreData object
	@param r the SmiCoreCombineRule; if no combine rule is passed, the default rule SmiCoreCombineReplace is used
	*/
	SmiBlockStructure(SmiCoreData *c, SmiCoreCombineRule *r=SmiCoreCombineReplace::Instance())
	{
		core_=c;
		this->setCombineWithCoreRule(r);		
	}

	~SmiBlockStructure();

};

#endif

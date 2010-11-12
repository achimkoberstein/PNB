#ifndef SMIBLOCKSTRUCTURE_HPP
#define SMIBLOCKSTRUCTURE_HPP

#include "SmiScnData.hpp"
#include "SmiLinearData.hpp"
#include <vector>

using namespace ::std;



class SmiBlockEvent : public SmiLinearData {

public:
	inline void setBlockEventProb(double p) {prob_ = p;}
	inline const double getBlockEventProb () { return prob_; }
	SmiBlockEvent(SmiLinearData d, double p):SmiLinearData(d),prob_(p){}
	~SmiBlockEvent(){}
private:
	double prob_;
};

class SmiBlock{
	 
private:
	vector<SmiBlockEvent*> events_;
	SmiStageIndex  stg_;
public:
	void addBlockEvent(CoinPackedMatrix &matrix,
				CoinPackedVector &dclo, CoinPackedVector &dcup,
				CoinPackedVector &dobj,
				CoinPackedVector &drlo, CoinPackedVector &drup, double prob)
	{
		SmiLinearData d(matrix,dclo,dcup,dobj,drlo,drup);
		SmiBlockEvent *e = new SmiBlockEvent(d,prob);
		events_.push_back(e);

	}
	/*void addBlockEvent(OsiSolverInterface &osi, double prob)
	{
		SmiLinearData d(osi);
		SmiDiscreteEvent *e = new SmiDiscreteEvent(d,prob);
		events_.push_back(e); 
	}
	*/
	inline const CoinPackedMatrix &getBlockEventMatrix(int i)   {return events_[i]->getMatrix(); }
	inline const CoinPackedVector &getBlockEventColLower(int i) {return events_[i]->getColLower();}
	inline const CoinPackedVector &getBlockEventColUpper(int i) {return events_[i]->getColUpper();}
	inline const CoinPackedVector &getBlockEventObjective(int i){return events_[i]->getObjective();}
	inline const CoinPackedVector &getBlockEventRowLower(int i) {return events_[i]->getRowLower();}
	inline const CoinPackedVector &getBlockEventRowUpper(int i) {return events_[i]->getRowUpper();}
	double getBlockEventProb(int i) { 
			return events_[i]->getBlockEventProb();
	}
	inline size_t getNumEvents() { return events_.size(); }
	inline int getStage() {return stg_;}
	inline void setStage(int p) {stg_=p;}
	SmiBlock(){stg_=-1;}
	SmiBlock(int p) {stg_=p;}
	~SmiBlock(){
		for (size_t i=0; i<events_.size(); ++i)
			delete events_[i];
	}
};

class SmiBlockStructure{
//#########################################################################################################
//#########################################################################################################

private:

	//list of blocks
	vector<SmiBlock*> smiBlock_;
	SmiCoreData *core_;
	SmiCoreCombineRule *combineRule_;

public:
	/// add block
	void addSmiBlock(SmiBlock *s)
	{ smiBlock_.push_back(s); }

	void deleteSmiBlock(){
		smiBlock_.pop_back();
	}

	/// get block
	SmiBlock * getBlock(int i) {return smiBlock_[i];}

	/// get number of RV
	int getNumBlocks() { return (int)smiBlock_.size(); }

	/// get core model 
	SmiCoreData *getCore(){ return core_; }

	/// set combine rule
	inline void setCombineWithCoreRule(SmiCoreCombineRule *r){
			combineRule_ = r;
	}

	/// get combine rule
	inline SmiCoreCombineRule *getCombineWithCoreRule() { return combineRule_;}

	
	/// constructor requires core data and combine rule
	SmiBlockStructure(SmiCoreData *c, SmiCoreCombineRule *r=SmiCoreCombineReplace::Instance())
	{
		core_=c;
		this->setCombineWithCoreRule(r);		
	}

	~SmiBlockStructure();

};

#endif

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

    inline  CoinPackedMatrix &getMutableBlockEventMatrix(int i)   {return events_[i]->getMutableMatrix(); }   /// Returns the matrix of the i-th SmiBlockEvent
    inline  CoinPackedVector &getMutableBlockEventColLower(int i) {return events_[i]->getMutableColLower();}  /// Returns the column lower bounds of the i-th SmiBlockEvent
    inline  CoinPackedVector &getMutableBlockEventColUpper(int i) {return events_[i]->getMutableColUpper();}  /// Returns the column upper bounds of the i-th SmiBlockEvent
    inline  CoinPackedVector &getMutableBlockEventObjective(int i){return events_[i]->getMutableObjective();} /// Returns the objective of the i-th SmiBlockEvent
    inline  CoinPackedVector &getMutableBlockEventRowLower(int i) {return events_[i]->getMutableRowLower();}  /// Returns the row lower bounds of the i-th SmiBlockEvent
    inline  CoinPackedVector &getMutableBlockEventRowUpper(int i) {return events_[i]->getMutableRowUpper();}  /// Returns the row upper bounds of the i-th 

    /// Returns the probability of the i-th SmiBlockEvent
    inline double getBlockEventProb(int i) { 
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

    /// Creates an SmiBlock at a given stage
    SmiBlock(int stage) {stg_=stage;}
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

    /// A vector that contains the blocks for each stage
    vector< vector<SmiBlock*> > smiBlock_;

    /// The SmiCoreData object
    SmiCoreData *core_;

    /// The combine rule (add or replace)
    SmiCoreCombineRule *combineRule_;

public:
    /// Adds SmiBlock s to the end of the list
    void addSmiBlock(SmiBlock *s);

    /// Deletes the last SmiBlock from the given stage
    void deleteSmiBlock(int stage);

    /// Returns the i-th SmiBlock of stage 
    SmiBlock * getBlock(int i, int stage) {return smiBlock_[stage][i];}

    /// Returns the number of SmiBlocks
    int getNumBlocks(int stage);

    /// Returns the SmiCoreData object
    inline SmiCoreData *getCore(){ return core_; }

    /// Sets the SmiCoreCombineRule
    void setCombineWithCoreRule(SmiCoreCombineRule *r){
        combineRule_ = r;
    }

    /// Returns the SmiCoreCombine rule
    inline SmiCoreCombineRule *getCombineWithCoreRule() { return combineRule_;}


    /** Constructs a new SmiBlockStructure with SmiCoreData c and SmiCoreCombineRule r
    @param c the SmiCoreData object
    @param r the SmiCoreCombineRule; if no combine rule is passed, the default rule SmiCoreCombineReplace is used
    */
    SmiBlockStructure(SmiCoreData *c, SmiCoreCombineRule *r=SmiCoreCombineReplace::Instance());

    ~SmiBlockStructure();

};

#endif

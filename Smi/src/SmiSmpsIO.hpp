// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef SmiSmpsIO_HPP
#define SmiSmpsIO_HPP

#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif

#include "CoinFinite.hpp"
#include "CoinMpsIO.hpp"
#include "CoinFileIO.hpp"
#include "SmiScnModel.hpp"
#include "SmiScnData.hpp"
#include "SmiScenarioTree.hpp"

#include <vector>
#include <map>
#include <string>

// sections
enum SmiSectionType { SMI_NO_SECTION, SMI_NAME_SECTION,
    SMI_ENDATA_SECTION, 
    SMI_EOF_SECTION, 
    SMI_TIME_SECTION, 
    SMI_SCENARIOS_SECTION, 
    SMI_INDEPENDENT_SECTION,
    SMI_BLOCKS_SECTION,
    SMI_UNKNOWN_SECTION
};

enum SmiSmpsType { SMI_SC_CARD, SMI_BL_CARD, SMI_COLUMN_CARD, SMI_DISCRETE_DIST, SMI_SMPS_COMBINE_ADD, 
    SMI_SMPS_COMBINE_REPLACE, SMI_SMPS_COMBINE_UNKNOWN, 
    SMI_TIME_UNORDERED_CORE_TYPE, SMI_TIME_ORDERED_CORE_TYPE, SMI_LO_BOUND_CARD,
    SMI_UP_BOUND_CARD,SMI_FX_BOUND_CARD,SMI_FR_BOUND_CARD, SMI_UNKNOWN_MPS_TYPE
};

/** This class is used to facilitate the reading of SMPS files.
*/
class SmiSmpsCardReader:
    public CoinMpsCardReader
{
public:
    /// Read to the next SMPS field
    SmiSectionType nextSmpsField (  );

    /// Return the current SMPS section (TIME, SCENARIOS, etc.)
    SmiSectionType whichSmpsSection(){return smiSection_;}

    /// Returns the current SMPS type
    SmiSmpsType whichSmpsType() {return smiSmpsType_;}

    /// Return the period name
    inline const char *periodName (  ) const {return periodName_;}
    /// Return the column name in a new scenario
    inline const char *scenarioNew (  ) const {return columnName_;}
    /// Return the ancestor of a scenario
    inline const char *scenarioAnc (  ) const {return rowName_;}
    /// Return the name of a block element
    inline const char *blockName (  ) const {return columnName_;}

    /// Return the probability in an SMI_INDEPENDENT_SECTION
    inline double getProb(){ return prob_;}

    /// Set the SmiCoreCombineRule
    inline void setCoreCombineRule(SmiCoreCombineRule *r){combineRule_=r;combineRuleSet=true;}
    // Return the SmiCoreCombineRule
    inline SmiCoreCombineRule *getCoreCombineRule() { return combineRule_;}

    /// Constructor expects file to be open 
    /// This one takes gzFile if fp null
    SmiSmpsCardReader( CoinFileInput *input, CoinMpsIO * reader ):CoinMpsCardReader (input,reader ),
        combineRuleSet(false),prob_(0.0){}

    ~SmiSmpsCardReader(){}
private:
    /// Current third name (for SmpsIO)
    char periodName_[COIN_MAX_FIELD_LENGTH];


    //float fvalue_;

    /// Current SMI SMPS Section
    SmiSectionType smiSection_;

    /// Current SMI SMPS Type
    SmiSmpsType smiSmpsType_;

    /// The CoreCombineRule (add or replace)
    SmiCoreCombineRule *combineRule_;

    /// Whether a combine rule is set (true) or not(false)
    bool combineRuleSet;

    /// The probability
    double prob_;
};

/** SMPS IO Interface

This class can be used to read in SMPS files. All relevant data
is stored in the object and can be used by a solver.
*/
class SmiSmpsIO: 
    public CoinMpsIO
{
public:
    /** Read in an SMPS time file and generates an SmiCoreData object.
    @param smi a pointer to the SmiScnModel object
    @param c the name of the time file
    @param ext optional, the extension of the file (default is "time")
    @return the resulting SmiCoreData object
    */
    SmiCoreData * readTimeFile(SmiScnModel *smi,const char *c,const char *ext="time");

    /** Read in an SMPS stoch file.
    @param smi a pointer to the SmiScnModel object
    @param core a pointer to the SmiCoreData object
    @param c the name of the time file
    @param ext optional, the extension of the file (default is "stoch")
    @return error (< 0) or success code (0)
    */
    int readStochFile(SmiScnModel *smi,SmiCoreData *core, const char *c,const char *ext="stoch");

    /** Set the SmiCoreCombineRule.
    @param r the SmiCoreCombineRule
    @see SmiCoreCombineRule
    */
    inline void setCoreCombineRule(SmiCoreCombineRule *r){combineRule_=r; combineRuleSet=true;}

    /** Return the SmiCoreCombineRule.
    @return the SmiCoreCombineRule
    @see SmiCoreCombineRule
    */
    inline SmiCoreCombineRule *getCoreCombineRule() { return combineRule_;}

    /** Return the number of stages.
    @return the number of stages
    */
    inline int getNumStages(){ return nstag_;}

    /** Return a pointer to an array containing the number of columns for each stage.
    @return a pointer to an array containing the number of columns for each stage
    */
    inline int *getColumnStages(){ return cstag_;}

    /** Return a pointer to an array containing the number of rows for each stage.
    @return a pointer to an array containing the number of rows for each stage
    */
    inline int *getRowStages(){ return rstag_;}

    /** Set the value for infinity.
    @param solverInf the value for infinity
    */
    inline void setSolverInfinity(double solverInf) { solverInf_ = solverInf; }

    /** Return the used value for infinity.
    @return solverInf the value for infinity
    */
    inline double getSolverInfinity() const { return solverInf_; }

    /** Write SMPS files.
    This method generates three files {name}.[core, time, stoch] or {name}.[cor, tim, sto] (see second parameter).
    @param filename the name for the files
    @param winFileExtensions optional; false by default, so extensions will be [core, time, stoch]. With this parameter set to true, it will be [cor, tim, sto].
    @param strictFormat optional, true by default. Set to false if SMPS files should be written in free format.
    */
    void writeSmps(const char* filename, bool winFileExtensions = false, bool strictFormat = true);

    /** Set if we read in a problem, where we do not want to build the full scenario tree but instead keep the distributions
    @param do_sample true, if we want to sample later on                                                                   
    */
    void setSampling(bool do_sample);
public:
    SmiSmpsIO():CoinMpsIO(),nstag_(0),cstag_(NULL),rstag_(NULL),solverInf_(COIN_DBL_MAX),iftime(false),ifstoch(false),smpsCardReader_(NULL),combineRule_(NULL),combineRuleSet(false),core(NULL),tree(NULL),periodMap_(),scenarioMap_(),sampling_(false) {}
    SmiSmpsIO(SmiCoreData * core, SmiScenarioTree<SmiScnNode *> * smiTree):CoinMpsIO(),nstag_(0),cstag_(NULL),rstag_(NULL),solverInf_(COIN_DBL_MAX),iftime(false),ifstoch(false),smpsCardReader_(NULL),combineRule_(NULL),combineRuleSet(false),core(core),tree(smiTree),periodMap_(),scenarioMap_(),sampling_(false) {}

    ~SmiSmpsIO(){delete [] cstag_;delete[] rstag_;delete smpsCardReader_;}
private:
    /** This method writes the core file for the current model by invoking writeMPS() from CoinMpsIO.
    @param filename The filename.
    @param extension The file extension.
    @param strictFormat Whether a strict format should be used or not.
    */ 
    void writeCoreFile(const char* filename, const char* extension, const bool strictFormat);

    /** Write the time file for the current model, containing the assignments from rows and columns to stages.
    @param filename The filename.
    @param extension The file extension.
    @param strictFormat Whether a strict format should be used or not.
    */
    void writeTimeFile(const char* filename, const char* extension, const bool strictFormat);

    /** Write the stoch file for the current model, containing the stochastic data.    
    @param filename The filename.
    @param extension The file extension.
    @param strictFormat Whether a strict format should be used or not.
    */
    void writeStochFile(const char* filename, const char* extension, const bool strictFormat);

    /** Write the stochastic informations for the given scenario into the stream. This method is only called by SmiSmpsIO::writeStochFile()
    @param stream the stream
    @param node a pointer to the leaf node of the scenario
    @param scneario the index of the scenario
    @param strictFormat whether a strict format should be used or not
    */
    void writeScenarioToStochFile(std::ostringstream& stream, SmiTreeNode<SmiScnNode *> * node, int scenario, bool strictFormat);

    /** Return the (probably modified) problem name.
    @return the (probably modified) problem name
    */
    std::string getModProblemName();

    /// The number of stages
    int nstag_;

    /// The number of columns in each stage
    int *cstag_;

    /// The number of rows in each stage
    int *rstag_;

    /// The value of infinity
    double solverInf_;

    /// Whether a time file could by found and read or not
    bool iftime;

    /// Whether a stoch file could by found and read or not
    bool ifstoch;

    /// The SMPS card reader
    SmiSmpsCardReader *smpsCardReader_;

    /// The combine rule (add or replace)
    SmiCoreCombineRule *combineRule_;

    /// Whether a combine rule is set or not
    bool combineRuleSet;

    /// The SmiCoreData object
    SmiCoreData * core;

    /// The scenario tree
    SmiScenarioTree<SmiScnNode *> * tree;

    typedef std::map<string,int> StringIntMap;

    /// Maps the name of a period to a stage index
    StringIntMap periodMap_;

    /// Maps the name of a scenario to the scenario index
    StringIntMap scenarioMap_;

    // If we read in a problem, where we only want the distributions, so that we can sample later on!
    bool sampling_;
};


#endif //#define SmiSmpsIO_HPP

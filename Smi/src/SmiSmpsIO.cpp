// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "SmiSmpsIO.hpp"
#include <cassert>
#include <cmath>
#include <cfloat>
#include <string>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "SmiBlockStructure.hpp" 
#include "SmiDiscreteDistribution.hpp"

// In CoinUtilsConfig.h the COIN_HAS_ZLIB macro is set (or not)
#include "CoinUtilsConfig.h"

#include "CoinMpsIO.hpp"
#include "CoinMessage.hpp"
#include "CoinError.hpp"


#if 1
const static char *section[] = {
  "", "NAME", "ENDATA", " ", "PERIODS", "SCENARIOS", "INDEPENDENT", "BLOCKS", " "
};

const static char *smpsType[] = {
	"SC","BL","  ","DISCRETE","ADD","REPLACE","","","","LO","UP","FX","FR"
};
#endif


//#############################################################################

SmiCoreData *
SmiSmpsIO::readTimeFile(SmiScnModel *smi, const char *c, const char *ext)
{

        CoinFileInput *input = 0;
        int returnCode = dealWithFileName(c,ext,input);
        if (returnCode<0) {
          return NULL;
        } else if (returnCode>0) {
          // delete cardReader_;
          smpsCardReader_ = new SmiSmpsCardReader ( input, this);
        }
	
	smpsCardReader_->readToNextSection();
	if ( smpsCardReader_->whichSection (  ) == COIN_NAME_SECTION ) {
		iftime = true;
		// check problem name and issue warning if needed
		if (strcmp(problemName_,smpsCardReader_->columnName()))
		{
			printf("Warning: Time file name %s does not match problem file name %s\n",
				smpsCardReader_->columnName(),problemName_);
		}
	} else if ( smpsCardReader_->whichSection (  ) == COIN_UNKNOWN_SECTION ) {
		handler_->message(COIN_MPS_BADFILE1,messages_)<<smpsCardReader_->card()
			<<1
			<<fileName_
			<<CoinMessageEol;
/*
#ifdef COIN_HAS_ZLIB
		if (!smpsCardReader_->filePointer()) 
			handler_->message(COIN_MPS_BADFILE2,messages_)<<CoinMessageEol;
		
#endif
*/
		return NULL;
	} else if ( smpsCardReader_->whichSection (  ) != COIN_EOF_SECTION ) {
		// save name of section
		free(problemName_);
		problemName_=strdup(smpsCardReader_->card());
	} else {
		handler_->message(COIN_MPS_EOF,messages_)<<fileName_
			<<CoinMessageEol;
		return NULL;
	}

	if (iftime)
	{
		int rowEnd, rowStart = 0;
		int colEnd, colStart = 0;
		if (	smpsCardReader_->nextSmpsField() != SMI_TIME_SECTION ) // PERIODS card
			return NULL;
	
		if (	smpsCardReader_->nextSmpsField() == SMI_TIME_SECTION ) // first period
		{
		
			cstag_ = new int[this->getNumCols()];
			rstag_ = new int[this->getNumRows()];
			std::string SunStudioNeedsThis = smpsCardReader_->periodName() ;
			periodMap_.insert(make_pair(SunStudioNeedsThis,nstag_));
		}
		else
			return NULL;

		// don't handle the unordered case yet
		if (smpsCardReader_->whichSmpsType() != SMI_TIME_ORDERED_CORE_TYPE)
			return NULL;

		while( smpsCardReader_->nextSmpsField (  ) == SMI_TIME_SECTION ) 
		{
			if (smpsCardReader_->whichSmpsType() == SMI_TIME_ORDERED_CORE_TYPE)
			{
				
				colEnd = this->columnIndex(smpsCardReader_->columnName());
				rowEnd = this->rowIndex(smpsCardReader_->rowName());
                // Check if we have correct colEnd and rowEnd
                if ( colEnd == -1 && rowEnd == -1){// Maybe the user erred on input ordering..
                    colEnd = this->columnIndex(smpsCardReader_->rowName());
                    rowEnd = this->rowIndex(smpsCardReader_->columnName());
                }
                if ( colEnd == -1 || rowEnd == -1){ // Input File is wrong, we have to abort..
                    return NULL;
                }
				
				std::string SunStudioNeedsThis = smpsCardReader_->periodName() ;
				periodMap_.insert(make_pair(SunStudioNeedsThis,nstag_+1));
				
				int i;
				for(i=colStart;i<colEnd;++i)
					cstag_[i]=nstag_;
				for(i=rowStart;i<rowEnd;++i)
					rstag_[i]=nstag_;
				++nstag_;
				colStart = colEnd;
				rowStart = rowEnd;
			}
			else
				return NULL;
		}
		
		if ( smpsCardReader_->whichSmpsSection() == SMI_ENDATA_SECTION
			&& smpsCardReader_->whichSmpsType() == SMI_TIME_ORDERED_CORE_TYPE)
		{
			int i;

			for(i=std::max(colStart, 0);i<this->getNumCols();++i)
				cstag_[i]=nstag_;
			for(i=std::max(rowStart, 0);i<this->getNumRows();++i)
				rstag_[i]=nstag_;
		}
		else
			return NULL;

	}

    // Aenderung: IntegerType
    int intLen=0;
    int binLen=0;
    const double* colLower = this->getColLower();
    const double* colUpper = this->getColUpper();
    for(int i=0;i<this->getNumCols();i++ ){
        if(this->isInteger(i)){
            intLen++;
            if(colUpper[i]==1&&colLower[i]==0){
                binLen++;
            }
        }
    }
    int* intInd = new int[intLen];
    int* binInd = new int[binLen];
    int cint = 0;
    int cbin = 0;
    for (int i=0; i<this->getNumCols();i++ ){
        if(this->isInteger(i)){
            intInd[cint]=i;
            cint++;
            if(colUpper[i]==1&&colLower[i]==0){
                binInd[cbin]=i;
                cbin++;
            }
        }
    }
    // Aenderung: IntegerType


	SmiCoreData *smiCoreData = new SmiCoreData(this,nstag_+1,cstag_,rstag_,intInd,intLen,binInd,binLen);
    delete[] intInd;
    delete[] binInd;
	return smiCoreData;

}

//#############################################################################

int
SmiSmpsIO::readStochFile(SmiScnModel *smi,SmiCoreData *core, const char *c, const char *ext)
{
	
        CoinFileInput *input = 0;
        int returnCode = dealWithFileName(c,ext,input);
        if (returnCode<0) {
          return -1;
        } else if (returnCode>0) {
          delete smpsCardReader_;
          smpsCardReader_ = new SmiSmpsCardReader ( input, this);
		  if(combineRuleSet)
			  smpsCardReader_->setCoreCombineRule(combineRule_);
        }
	

	smpsCardReader_->readToNextSection();
	if ( smpsCardReader_->whichSection (  ) == COIN_NAME_SECTION ) {
		ifstoch = true;
		// check problem name and issue warning if needed
		if (strcmp(problemName_,smpsCardReader_->columnName()))
		{
			printf("Warning: Time file name %s does not match problem file name %s\n",
				smpsCardReader_->columnName(),problemName_);
		}
	} else if ( smpsCardReader_->whichSection (  ) == COIN_UNKNOWN_SECTION ) {
		handler_->message(COIN_MPS_BADFILE1,messages_)<<smpsCardReader_->card()
			<<1
			<<fileName_
			<<CoinMessageEol;
#ifdef COIN_USE_ZLIB
		if (!smpsCardReader_->filePointer()) 
			handler_->message(COIN_MPS_BADFILE2,messages_)<<CoinMessageEol;
		
#endif
		return -2;
	} else if ( smpsCardReader_->whichSection (  ) != COIN_EOF_SECTION ) {
		// save name of section
		free(problemName_);
		problemName_=strdup(smpsCardReader_->card());
	} else {
		handler_->message(COIN_MPS_EOF,messages_)<<fileName_
			<<CoinMessageEol;
		return -3;
	}

	if (ifstoch)
	{
        // Create discrete distribution
        SmiDiscreteDistribution *smiDD = NULL;//new SmiDiscreteDistribution(core);
        SmiDiscreteRV *smiRV = NULL;
        if ( sampling_ )
            smiDD = new SmiDiscreteDistribution(core);

        switch( smpsCardReader_->nextSmpsField() ) // This does not work as expected, if we expect several fields ( i.e. SMI_INDEPENDENT_SECTION and SMI_SCENARIO_SECTION ). So we have to combine these two..
        {
        case SMI_INDEPENDENT_SECTION: // INDEPENDENT card
        case SMI_BLOCKS_SECTION:
            {
                // We need to declare a SmiBlockDistribution
                SmiBlockStructure smiBS(core);
                // Try to process as long we ware at the end
                while ( smpsCardReader_->whichSmpsSection() != SMI_ENDATA_SECTION ){

                    if ( smpsCardReader_->whichSmpsSection() == SMI_INDEPENDENT_SECTION ){
                        printf("Processing INDEPENDENT section\n");

                        // Create discrete distribution
                        if ( !sampling_)
                            smiDD = new SmiDiscreteDistribution(core);
                        smiRV = NULL;

                        CoinPackedVector drlo,drup,dclo,dcup,dobj;
                        CoinPackedMatrix *matrix = new CoinPackedMatrix(false,0.25,0.25);
                        assert(!matrix->isColOrdered());

                        int nrow = core->getNumRows();
                        int ncol = core->getNumCols();
                        matrix->setDimensions(nrow,ncol);

                        int oldi=-1;
                        int oldj=-1;

                        while( smpsCardReader_->nextSmpsField (  ) == SMI_INDEPENDENT_SECTION ) 
                        {

                            switch (smpsCardReader_->whichSmpsType())
                            {
                            case SMI_BL_CARD:
                                {
                                    //this is a block 
                                    // not handled yet
                                    return -2;
                                }
                            case SMI_COLUMN_CARD:
                                {
                                    int j=this->columnIndex(smpsCardReader_->columnName());
                                    int i=this->rowIndex(smpsCardReader_->rowName());

                                    double value = smpsCardReader_->value();


                                    if (j<0) // check RHS
                                    {
                                        if (!strcmp(this->getRhsName(),"")) {
                                            free(rhsName_);
                                            rhsName_=strdup(smpsCardReader_->columnName());
                                        } else
                                            assert(!strcmp(smpsCardReader_->columnName(),this->getRhsName()));
                                        assert(!(i<0));
                                        

                                        char c=this->getRowSense()[i];

                                        switch(c)
                                        {
                                        case 'E':
                                            drlo.insert(i,value);
                                            drup.insert(i,value);
                                            break;
                                        case 'L':
                                            drup.insert(i,value);
                                            //drlo.insert(i,value); <- Wrong
                                            break;
                                        case 'G':
                                            //drup.insert(i,value); <- Wrong
                                            drlo.insert(i,value);
                                            break;
                                        default:
                                            assert(!"bad row sense: shouldn't get here");
                                            break;
                                        }
                                    }
                                    else if(i<0 || i==getNumRows()) // check OBJ
                                    {
                                        assert(!strcmp(smpsCardReader_->rowName(),this->getObjectiveName()));
                                        dobj.insert(j,value);
                                    }
                                    else	// add element
                                    {
                                        matrix->modifyCoefficient(i,j,value,true);
                                    }
                                    if ( !( (oldi==i)&&(oldj==j) ) ) // this is a new RV
                                    {
                                        oldi=i;
                                        oldj=j;
                                        // store the old one
                                        if (smiRV != NULL) 
                                        {
                                            smiDD->addDiscreteRV(smiRV);
                                        }
                                        // create a new one -- row stage dominates unless is column bound
                                        if (i>0 && i < nrow)// If i == nrow, we have objective function value..
                                            smiRV = new SmiDiscreteRV(core->getRowStage(i));
                                        else
                                            smiRV = new SmiDiscreteRV(core->getColStage(j));
                                        assert(smiRV->getStage() >= 0 && smiRV->getStage()<= nstag_);
                                        smiRV->addEvent(*matrix,dclo,dcup,dobj,drlo,drup,smpsCardReader_->getProb());
                                        matrix->clear();
                                        dclo.clear();
                                        dcup.clear();
                                        dobj.clear();
                                        drlo.clear();
                                        drup.clear();

                                    }
                                    else
                                    {
                                        smiRV->addEvent(*matrix,dclo,dcup,dobj,drlo,drup,smpsCardReader_->getProb());
                                        matrix->clear();
                                        dclo.clear();
                                        dcup.clear();
                                        dobj.clear();
                                        drlo.clear();
                                        drup.clear();
                                    }




                                }
                                break;
                            case SMI_LO_BOUND_CARD:
                            case SMI_UP_BOUND_CARD:
                            case SMI_FX_BOUND_CARD:
                            case SMI_FR_BOUND_CARD:
                                assert(false); // This is not implemented.
                                break;
                            case SMI_UNKNOWN_MPS_TYPE:
                            default:
                                return -2;
                            }
                        }


                        if (smpsCardReader_->whichSmpsSection() == SMI_ENDATA_SECTION)
                        {
                            // store the old one
                            if (smiRV != NULL) 
                            {
                                smiDD->addDiscreteRV(smiRV);
                                smiRV = NULL;
                            }
                            if ( sampling_ )
                                smi->setDiscreteDistribution(smiDD);
                            else {
                                smi->processDiscreteDistributionIntoBlocks(smiDD,&smiBS);
                                delete smiDD;
                            }
                            delete matrix;
                            continue;
                        }
                        else if ( smpsCardReader_->whichSmpsSection() == SMI_BLOCKS_SECTION){

                            // store the old one
                            if (smiRV != NULL) 
                            {
                                smiDD->addDiscreteRV(smiRV);
                                smiRV = NULL;
                            }
                            if ( sampling_ )
                                smi->setDiscreteDistribution(smiDD);
                            else {
                                smi->processDiscreteDistributionIntoBlocks(smiDD,&smiBS);
                                delete smiDD;
                            }
                            delete matrix;
                            continue;
                        }
                        else
                            return -2;

                        // We end here, so we want to delete the discrete variable


                    }
                    if ( smpsCardReader_->whichSmpsSection() == SMI_BLOCKS_SECTION ){
                        printf("Processing BLOCK section\n");
                        // Create Block-Structure
                        SmiBlock *smiBlock = NULL;


                        CoinPackedVector drlo,drup,dclo,dcup,dobj;
                        CoinPackedMatrix *matrix = new CoinPackedMatrix(false,0.25,0.25);
                        assert(!matrix->isColOrdered());

                        int nrow = core->getNumRows();
                        int ncol = core->getNumCols();
                        matrix->setDimensions(nrow,ncol);

                        char* blName=NULL;
                        int block=0;
                        double prob=0.0;
                        const char* periodName=NULL;
                        int branch = 0;

                        while( smpsCardReader_->nextSmpsField (  ) == SMI_BLOCKS_SECTION ) 
                        {

                            switch (smpsCardReader_->whichSmpsType())
                            {
                            case SMI_BL_CARD:
                                {
                                    branch = periodMap_[smpsCardReader_->periodName()];
                                    if(blName!=NULL){
                                        smiBlock->addBlockEvent(*matrix,dclo,dcup,dobj,drlo,drup,prob);
                                        if ( smiRV != NULL){
                                            smiRV->addEvent(*matrix,dclo,dcup,dobj,drlo,drup,prob);
                                        }

                                        //first event of a block
                                        if(strcmp(blName,smpsCardReader_->blockName())){
                                            smiBS.addSmiBlock(smiBlock);
                                            //delete smiBlock;

                                            smiBlock = new SmiBlock(branch);
                                            // Delete the old blName first
                                            delete [] blName;
                                            blName = new char[strlen(smpsCardReader_->blockName())+1]; // Account for terminating null-character
                                            strcpy(blName,smpsCardReader_->blockName());

                                            // store the old one
                                            if (smiRV != NULL) 
                                            {
                                                smiDD->addDiscreteRV(smiRV);
                                                smiRV = NULL;
                                            }
                                            if ( sampling_ )
                                                smiRV = new SmiDiscreteRV(branch); // Stage 2 variables only
                                        }
                                        matrix->clear();
                                        drlo.clear();
                                        drup.clear();
                                        dclo.clear();
                                        dcup.clear();
                                        dobj.clear();

                                        matrix->setDimensions(nrow,ncol);
                                    }
                                    else{
                                        smiBlock = new SmiBlock(branch);
                                        blName= new char[strlen(smpsCardReader_->blockName())+1]; // Account for terminating null-character
                                        strcpy(blName,smpsCardReader_->blockName());
                                        // store the old one
                                        if (smiRV != NULL) 
                                        {
                                            smiDD->addDiscreteRV(smiRV);
                                            smiRV = NULL;
                                        }
                                        if ( sampling_ )
                                            smiRV = new SmiDiscreteRV(branch); // Stage 2 variables only
                                    }

                                    prob = smpsCardReader_->value();
                                    break;
                                }
                            case SMI_COLUMN_CARD:
                                {
                                    int j=this->columnIndex(smpsCardReader_->columnName());
                                    int i=this->rowIndex(smpsCardReader_->rowName());
                                    double value = smpsCardReader_->value();

                                    if (j<0) // check RHS
                                    {
                                        if (!strcmp(this->getRhsName(),"")) {
                                            free(rhsName_);
                                            rhsName_=strdup(smpsCardReader_->columnName());
                                        } else
                                            assert(!strcmp(smpsCardReader_->columnName(),this->getRhsName()));
                                        assert(!(i<0));

                                        char c=this->getRowSense()[i];

                                        switch(c)
                                        {
                                        case 'E':
                                            drlo.insert(i,value);
                                            drup.insert(i,value);
                                            break;
                                        case 'L':
                                            //drlo.insert(i,value);
                                            drup.insert(i,value);
                                            break;
                                        case 'G':
                                            //drup.insert(i,value);
                                            drlo.insert(i,value);
                                            break;
                                        default:
                                            assert(!"bad row sense: shouldn't get here");
                                            break;
                                        }
                                    }
                                    else if(i<0 || i==getNumRows()) // check OBJ
                                    {
                                        assert(!strcmp(smpsCardReader_->rowName(),this->getObjectiveName()));
                                        dobj.insert(j,value);
                                    }
                                    else	// add element
                                    {
                                        matrix->modifyCoefficient(i,j,value,true); // We must keep zeroes!
                                    }
                                    break;
                                }


                            case SMI_LO_BOUND_CARD:
                            case SMI_UP_BOUND_CARD:
                            case SMI_FX_BOUND_CARD:
                            case SMI_FR_BOUND_CARD:
                                assert(false); // This is not implemented.
                                break;
                            case SMI_UNKNOWN_MPS_TYPE:
                            default:{
                                // Clean up used memory before we exit
                                delete [] blName;
                                delete matrix;
                                return -2;

                                    }
                            }
                        }

                        delete [] blName;

                        if (smpsCardReader_->whichSmpsSection() == SMI_ENDATA_SECTION)
                        {
                            smiBlock->addBlockEvent(*matrix,dclo,dcup,dobj,drlo,drup,prob);
                            
                            smiBS.addSmiBlock(smiBlock);
                            if ( smiRV != NULL){
                                smiRV->addEvent(*matrix,dclo,dcup,dobj,drlo,drup,prob);
                                smiDD->addDiscreteRV(smiRV);
                                smiRV = NULL;
                            }
                            delete matrix;

                        }
                        else{
                            delete matrix;
                            return -2;
                        }


                    }
                }// End While
                //process SmiBlockStructure into Scenarios
                // We would like to keep the original blocks and sample from this distribution?!
                if ( sampling_ ){
                    if ( smi->getDiscreteDistribution() != smiDD)
                        smi->setDiscreteDistribution(smiDD);
                }
                else {
                    smi->processBlockStructureIntoScenarios(&smiBS);
                }
                return 0;

               
            }// End case 

        case SMI_SCENARIOS_SECTION: // SCENARIOS card
            {
                double prob=0.0;
                int scen=0,anc=0;
                int branch=0;
                CoinPackedVector drlo,drup,dclo,dcup,dobj;
                CoinPackedMatrix *matrix = new CoinPackedMatrix(false,0.25,0.25);
                assert(!matrix->isColOrdered());

                int nrow = core->getNumRows();
                int ncol = core->getNumCols();
                matrix->setDimensions(nrow,ncol);

                if ( sampling_ ){
                    assert( core->getNumStages() == 2);
                    smiDD = new SmiDiscreteDistribution(core);
                    smiRV = new SmiDiscreteRV(1);
                }

                while( smpsCardReader_->nextSmpsField (  ) == SMI_SCENARIOS_SECTION ) 
                {
                    if (smpsCardReader_->whichSmpsType() == SMI_SC_CARD)
                    {
                        if (scen)
                        {
                            if ( sampling_ )
                                smiRV->addEvent(*matrix,dclo,dcup,dobj,drlo,drup,prob);
                            else 
                                smi->generateScenario(core,matrix,&dclo,&dcup,&dobj,&drlo,&drup,branch,anc,prob,smpsCardReader_->getCoreCombineRule() );
                            
                            matrix->clear();
                            dclo.clear();
                            dcup.clear();
                            dobj.clear();
                            drlo.clear();
                            drup.clear();

                            matrix->setDimensions(nrow,ncol);

                        }

                        std::string SunStudioNeedsThis = smpsCardReader_->scenarioNew() ;
                        scenarioMap_.insert(make_pair(SunStudioNeedsThis,scen++));
                        prob = smpsCardReader_->value();
                        if (!strncmp(smpsCardReader_->scenarioAnc(),"ROOT",4))
                            anc = 0;
                        else
                            anc = scenarioMap_[smpsCardReader_->scenarioAnc()];
                        branch = periodMap_[smpsCardReader_->periodName()];
                    }
                    else if(smpsCardReader_->whichSmpsType() == SMI_COLUMN_CARD)
                    {
                        int j=this->columnIndex(smpsCardReader_->columnName());
                        int i=this->rowIndex(smpsCardReader_->rowName());
                        double value = smpsCardReader_->value();


                        if (j<0) // check RHS
                        {
                            if (!strcmp(this->getRhsName(),"")) {
                                free(rhsName_);
                                rhsName_=strdup(smpsCardReader_->columnName());
                            } else
                                assert(!strcmp(smpsCardReader_->columnName(),this->getRhsName()));
                            assert(!(i<0));

                            char c=this->getRowSense()[i];

                            switch(c)
                            {
                            case 'E':
                                drlo.insert(i,value);
                                drup.insert(i,value);
                                break;
                            case 'L':
                                //drlo.insert(i,value);
                                drup.insert(i,value);
                                break;
                            case 'G':
                                //drup.insert(i,value);
                                drlo.insert(i,value);
                                break;
                            default:
                                assert(!"bad row sense: shouldn't get here");
                                break;
                            }
                        }
                        else if(i<0 || i==getNumRows()) // check OBJ
                        {
                            assert(!strcmp(smpsCardReader_->rowName(),this->getObjectiveName()));
                            dobj.insert(j,value);
                        }
                        else	// add element
                        {
                            matrix->modifyCoefficient(i,j,value,true);
                        }
                    }
                    else if ( smpsCardReader_->whichSmpsType() == SMI_LO_BOUND_CARD || smpsCardReader_->whichSmpsType() == SMI_UP_BOUND_CARD ||
                        smpsCardReader_->whichSmpsType() == SMI_FX_BOUND_CARD || smpsCardReader_->whichSmpsType() == SMI_FR_BOUND_CARD ){
                            int column = this->columnIndex(smpsCardReader_->columnName());
                            assert( column >= 0);
                            double value = smpsCardReader_->value();
                            switch ( smpsCardReader_->whichSmpsType() ){
                                case SMI_LO_BOUND_CARD:
                                    dclo.insert(column,value);
                                    break;
                                case SMI_UP_BOUND_CARD:
                                    dcup.insert(column,value);
                                    break;
                                case SMI_FX_BOUND_CARD:
                                    dclo.insert(column,value);
                                    dcup.insert(column,value);
                                    break;
                                case SMI_FR_BOUND_CARD:
                                    // insert infinity
                                    dclo.insert(column,-infinity_);
                                    dcup.insert(column,infinity_);
                                    break;
                            } // switch
                    }
                    else 
                        return -1; // unknown type
                } // while

                if (smpsCardReader_->whichSmpsSection() == SMI_ENDATA_SECTION)
                {
                    if (scen)
                    {
                        if ( sampling_ ){
                            smiRV->addEvent(*matrix,dclo,dcup,dobj,drlo,drup,prob);
                            smi->setDiscreteDistribution(smiDD);
                        }
                        else 
                            smi->generateScenario(core,matrix,&dclo,&dcup,&dobj,&drlo,&drup,branch,anc,prob,smpsCardReader_->getCoreCombineRule() );
                    }
                    delete matrix;
                    return 0;
                }
                else{

                    return -1; // If we have a scenario section, nothing else is allowed here. We assume a missing ENDATA
                }

            } 
     
        default:// didn't recognize section
            return -1;
        }

		

	}



	return 0;
}

//#############################################################################
//  nextSmpsField


SmiSectionType
SmiSmpsCardReader::nextSmpsField (  )
{
  
  // find next non blank character
  char *next = position_;

  static const char *blanks = (const char *) " \t";
  char valstr[COIN_MAX_FIELD_LENGTH],*after;
  
  while ( next != eol_ ) {
    if ( *next == ' ' || *next == '\t' ) {
      next++;
    } else {
      break;
    }
  }

  bool gotCard;
  
  if ( next == eol_ ) {
	  gotCard = false;
  } else {
	  gotCard = true;
  }
  while ( !gotCard ) 
  {
	  // need new image
	  
	  if ( cleanCard() ) {
		  return SMI_EOF_SECTION;
	  }
	  if ( card_[0] == ' ' ) {
		  // not a section or comment
		  position_ = card_;
		  eol_ = card_ + strlen ( card_ );


		  if ( smiSection_ == SMI_TIME_SECTION ) 
		  {
			  smiSmpsType_ = SMI_TIME_ORDERED_CORE_TYPE;
			  if (!(next = strtok(position_,blanks)))
			  {
				  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
				  return smiSection_;
			  }
			  strcpy(columnName_,next);
			  
			  if (!(next = strtok(NULL,blanks)))
			  {
				  strcpy(periodName_,columnName_);
				  smiSmpsType_ = SMI_TIME_UNORDERED_CORE_TYPE;
				  return smiSection_;
			  }
			  strcpy(rowName_,next);
			  if (!(next = strtok(NULL,blanks)))
			  {
				  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
				  return smiSection_;
			  }
			  strcpy(periodName_,next);
			  position_ = eol_;
			  
		  }

		  if ( smiSection_ == SMI_INDEPENDENT_SECTION )
		  {
			  int i;
			  if (strlen(next)==2)
			  {
				  for (i = SMI_BL_CARD; i < SMI_UNKNOWN_MPS_TYPE; ++i) {
					  if ( !strncmp ( next, smpsType[i], 2 ) ) 
					  {
						  break;
					  }
				  }
			  }
			  else
				  i = SMI_COLUMN_CARD;

			  switch(smiSmpsType_ = (SmiSmpsType) i)
			  {
			  case SMI_BL_CARD: // not handled yet
				  break;
				 
			  case SMI_COLUMN_CARD: // card info has "col,row,value,?(period),prob"
				    
				  if (!(next = strtok(position_,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  return smiSection_;
				  }
				  //already got the name
				  strcpy(columnName_,next);
				  
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(rowName_,next);

				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(valstr,next);

				  value_ = osi_strtod(valstr,&after,0);
				  // see if error
				  assert(after>valstr);
					
                  // Now we need to see if there is a period name or a probability.
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(valstr,next);

				  prob_ = osi_strtod(valstr,&after,0);
                  // see if we have a period entry insteaf of a probability
                  if ( after == valstr ){
                      // Try to map a period 
                      strcpy(periodName_,next);

                      // If the previous entry was a period name, now comes the probability..
                      if (!(next = strtok(NULL,blanks)))
                      {
                          smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
                          break;
                      }
                      strcpy(valstr,next);

                      prob_ = osi_strtod(valstr,&after,0);
                      // see if error: Malformed Line
                      assert(after>valstr);

                  }
				  position_ = eol_;  //end of card -- no more information allowed
				  break;

			  case SMI_UNKNOWN_MPS_TYPE:
				  break;
			  default:
				  smiSmpsType_=SMI_UNKNOWN_MPS_TYPE;
			  }//end switch

			  return smiSection_;

		  }

		  if ( smiSection_ == SMI_SCENARIOS_SECTION )
		  {
			

			  
			  if (!(next = strtok(position_,blanks)))
			  {
				  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
				  return smiSection_;
			  }
			
			  int i;
			  if (strlen(next)==2)
			  {
				  for (i = SMI_SC_CARD; i < SMI_UNKNOWN_MPS_TYPE; ++i) {
					  if ( !strncmp ( next, smpsType[i], 2 ) ) 
					  {
						  break;
					  }
				  }
				  if (i==SMI_UNKNOWN_MPS_TYPE)
					  i = SMI_COLUMN_CARD;
			  }
			  else
				  i = SMI_COLUMN_CARD;
			
			  switch(smiSmpsType_ = (SmiSmpsType) i)
			  {
			  case SMI_SC_CARD:
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(columnName_,next);
				  
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(rowName_,next);

				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(valstr,next);

				  value_ = osi_strtod(valstr,&after,0);
				  // see if error
				  assert(after>valstr);

				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(periodName_,next);

				  if (!(next = strtok(NULL,blanks)))
					  position_ = eol_;
				  else
					  // strcpy(next,periodName_); // store it here until next time
					  //strcpy(periodName_,next); // store it here until next time

				  // no futher strings allowed.
				  position_=eol_;
				  break;
			  case SMI_UNKNOWN_MPS_TYPE:
				  //this in case the column name has two characters!
			  case SMI_COLUMN_CARD:
				  //already got the name
				  strcpy(columnName_,next);
				  
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(rowName_,next);

				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(valstr,next);

				  value_ = osi_strtod(valstr,&after,0);
				  // see if error
				  assert(after>valstr);
				  
				  if (!(next = strtok(NULL,blanks)))
					  position_ = eol_;
				  else
					  // strcpy(next,periodName_); // store it here until next time
					  strcpy(periodName_,next); // store it here until next time
				  break;
              case SMI_LO_BOUND_CARD:
              case SMI_UP_BOUND_CARD:
              case SMI_FX_BOUND_CARD:
                  // We got the kind of bound already.
                  // We store the bound name in rowName
                  // We store the column name in columnName;
                  // We store the value in value_

                  if (!(next = strtok(NULL,blanks)))
                  {
                      smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
                      break;
                  }
                  strcpy(rowName_,next); // Bound name

                  if (!(next = strtok(NULL,blanks)))
                  {
                      smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
                      break;
                  }
                  strcpy(columnName_,next); // Column name

                  if (!(next = strtok(NULL,blanks)))
                  {
                      smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
                      break;
                  }
                  strcpy(valstr,next);
                  value_ = osi_strtod(valstr,&after,0); // Value
                  // see if error
                  assert(after>valstr);

                  if (!(next = strtok(NULL,blanks))) // Now there should be end of line only
                      position_ = eol_;
                  break;
              case SMI_FR_BOUND_CARD:
                  if (!(next = strtok(NULL,blanks)))
                  {
                      smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
                      break;
                  }
                  strcpy(rowName_,next); // Bound name

                  if (!(next = strtok(NULL,blanks)))
                  {
                      smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
                      break;
                  }
                  strcpy(columnName_,next); // Column name

                  if (!(next = strtok(NULL,blanks))) // Now there should be end of line only
                      position_ = eol_;

                  break;
			  default:
				  assert(smiSmpsType_ == SMI_UNKNOWN_MPS_TYPE);
				  break;
			  }
		  }
		  if ( smiSection_ == SMI_BLOCKS_SECTION )
		  {
			

			  
			  if (!(next = strtok(position_,blanks)))
			  {
				  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
				  return smiSection_;
			  }
			
			  int i;
			  if (strlen(next)==2)
			  {
				  
				  for (i = SMI_SC_CARD; i < SMI_UNKNOWN_MPS_TYPE; ++i) {
					  if ( !strncmp ( next, smpsType[i], 2 ) ) 
					  {
						  break;
					  }
				  }
				  if (i==SMI_UNKNOWN_MPS_TYPE)
					  i = SMI_COLUMN_CARD;
			  }
			  else
				  i = SMI_COLUMN_CARD;
			
			  switch(smiSmpsType_ = (SmiSmpsType) i)
			  {
			  case SMI_BL_CARD:
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(columnName_,next);
				  
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(periodName_,next);

				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(valstr,next);

				  value_ = osi_strtod(valstr,&after,0);
				  // see if error
				  assert(after>valstr);

				  // no futher strings allowed.
				  position_=eol_;
				  break;
			  case SMI_UNKNOWN_MPS_TYPE:
				  //this in case the column name has two characters!
			  case SMI_COLUMN_CARD:
				  //already got the name
				  strcpy(columnName_,next);
				  
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(rowName_,next);

				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  break;
				  }
				  strcpy(valstr,next);

				  value_ = osi_strtod(valstr,&after,0);
				  // see if error
				  assert(after>valstr);
		  
				  if (!(next = strtok(NULL,blanks)))
					  position_ = eol_;
				  else
					  // strcpy(next,periodName_); // store it here until next time
					  strcpy(periodName_,next); // store it here until next time
				  break;
			  default:
				  assert(smiSmpsType_ == SMI_UNKNOWN_MPS_TYPE);
				  break;
			  }
		  }
		  return smiSection_;

	  } else if ( card_[0] != '*' ) {
		  // not a comment, might be a section
		  int i;
		  
		  handler_->message(COIN_MPS_LINE,messages_)<<cardNumber_
			  <<card_<<CoinMessageEol;

		  // find the section, if there is one
		  for ( i = SMI_NAME_SECTION; i < SMI_UNKNOWN_SECTION; i++ ) {
			//printf("Comparing first 3 chars of %s with %s returns %d \n",
			//	card_, section[i], strncmp(card_,section[i],3));
			  if ( !strncmp ( card_, section[i], 3) ) {
				  break;
			  }
		  }

		  // didn't find anything so quit
		  if (i==SMI_UNKNOWN_SECTION) return (SmiSectionType) i;

		  position_ = card_;
		  eol_ = card_;
		  smiSection_ = ( SmiSectionType ) i;

		  // if its a scenario card, need to process some more info
		  if ( (smiSection_ == SMI_SCENARIOS_SECTION) || 
				(smiSection_ == SMI_INDEPENDENT_SECTION) )
		  {
			  i = SMI_SMPS_COMBINE_UNKNOWN;
			  next = strtok(position_,blanks);
			  
			  if (!(next = strtok(NULL,blanks)))
			  {
				  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
				  //break;
			  }
			  // find the section, if there is one
			  // next card should be DISCRETE
			  if (!(next = strtok(NULL,blanks)))
			  {
				  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
				  //break;
			  }
			  // find the section, if there is one
			  if (next == NULL) {
			    i = SMI_SMPS_COMBINE_REPLACE; 
			  } else {
			      for ( i = SMI_SMPS_COMBINE_ADD; i < SMI_SMPS_COMBINE_UNKNOWN; i++ ) {
				      if ( !strncmp ( next, smpsType[i], strlen ( section[i] ) ) ) {
					      break;
				      }
			      }
			  }
		   }
			  // set combine rule if it is not already set.
		   if (!combineRuleSet)
		   {
			  switch(i)
			  {
			  case SMI_SMPS_COMBINE_ADD:
				  this->setCoreCombineRule(SmiCoreCombineAdd::Instance());
				  break;
			  case SMI_SMPS_COMBINE_REPLACE:
				  this->setCoreCombineRule(SmiCoreCombineReplace::Instance());
				  break;
			  default:
				  this->setCoreCombineRule(SmiCoreCombineReplace::Instance());
				  // MESSAGE
				  printf(" Smps: setting default core combine rule to Replace\n");
			  }
			  
		  }
		  return smiSection_;
	  } else {
		  // comment
	  }
  }
  
  {
	  // if we get here, there are additional row fields on the line

				  strcpy(rowName_,periodName_); // stored the last field in periodName_
				  
				  if (!(next = strtok(NULL,blanks)))
				  {
					  smiSmpsType_ = SMI_UNKNOWN_MPS_TYPE;
					  return smiSection_;
				  }
				  strcpy(valstr,next);
				  
				  value_ = osi_strtod(valstr,&after,0);
				  // see if error
				  assert(after>valstr);

				  if (!(next = strtok(NULL,blanks)))
					  position_ = eol_;
				  else
					  strcpy(next,periodName_);
				  
				  
				  return smiSection_;
  }
}

std::string SmiSmpsIO::getModProblemName() {
    std::string name = "";
    if (strcmp(problemName_,"")==0) {
        name.append("BLANK   ");
    } else {
        if (strlen(problemName_) >= 8) {
            name.append(problemName_, 8);
        } else {
            name.append(problemName_);
            name.append(8-strlen(problemName_), ' ');
        }
    }
    return name;
}

void SmiSmpsIO::writeCoreFile(const char* filename, const char* extension, const bool strictFormat) {
    // count nels
    int nels = 0;
    for (int t = 0; t < core->getNumStages(); t++) {
        nels += core->getNode(t)->getNumMatrixElements();
    }
    
    // initialize arrays
    double* clo = new double[core->getNumCols()];
    double* cup = new double[core->getNumCols()];
    double* obj = new double[core->getNumCols()];
    double* rlo = new double[core->getNumRows()];
    double* rup = new double[core->getNumRows()];
    
    // initialize row-ordered matrix arrays
    double * dels = new double[nels];
    int * indx = new int[nels];
    int * rstrt = new int[core->getNumRows()+1];
    rstrt[0] = 0;

    int rowCount = 0, ncol = 0, nrow = 0;
    nels = 0;

    for (int t = 0; t < core->getNumStages(); t++) {

        SmiNodeData * node = core->getNode(t);
    	int stg = node->getStage();

	    core->copyRowLower(rlo+nrow,stg);
	    core->copyRowUpper(rup+nrow,stg);
	    core->copyColLower(clo+ncol,stg);
	    core->copyColUpper(cup+ncol,stg);
	    core->copyObjective(obj+ncol,stg);

	    // add rows to det. eq. matrix for current stage
	    for (int i=core->getRowStart(stg); i<core->getRowStart(stg+1) ; i++)
	    {
		    // build row explicitly into sparse arrays
		    int rowStart=rstrt[rowCount];
		    int rowNumEls=0;

		    const double *cels=node->getRowElements(i);
		    const int *cind=node->getRowIndices(i);
		    const int len=node->getRowLength(i);
		    memcpy(dels+rowStart,cels,sizeof(double)*len);
		    memcpy(indx+rowStart,cind,sizeof(int)*len);
		    rowNumEls=len;

		    //preparation for the next row
		    rowCount++;
		    nels+=rowNumEls;
		    rstrt[rowCount] = nels;
		    //done with preparations for the next row
        }
        
        ncol += core->getNumCols(stg);
	    nrow += core->getNumRows(stg);
    }
    assert(rowCount == nrow);
    if (solverInf_ != this->infinity_){
    // We have to transform the values from solverInfinity to infinity
        for (int i = 0; i < core->getNumCols(); i++){
            if (clo[i] == -solverInf_)
                clo[i] = -infinity_;
            if (clo[i] == solverInf_)
                clo[i] = infinity_;
            if (cup[i] == -solverInf_)
                cup[i] = -infinity_;
            if (cup[i] == solverInf_)
                cup[i] = infinity_;
            if (obj[i] == -solverInf_)
                obj[i] = -infinity_;
            if (obj[i] == solverInf_)
                obj[i] = infinity_;
        }
        for (int i = 0; i < core->getNumRows(); i++){
            if (rlo[i] == -solverInf_)
                rlo[i] = -infinity_;
            if (rlo[i] == solverInf_)
                rlo[i] = infinity_;
            if (rup[i] == -solverInf_)
                rup[i] = -infinity_;
            if (rup[i] == solverInf_)
                rup[i] = infinity_;
        }
        // We assume that there are no infinity values in the coefficient matrix..
    }
    
    CoinPackedMatrix matrix(false, ncol, nrow, nels, dels, indx, rstrt, NULL);
    
    delete [] dels;
    delete [] indx;
    delete [] rstrt;
    
    const std::vector<std::string> vec; //dummy
    char* integrality = new char[ncol];
    std::fill_n(integrality, ncol, 0);
    for (int i = 0; i < core->getIntegerLength(); i++) {
        integrality[core->getIntegerIndices()[i]] = 1;
    }
    this->setMpsData(matrix, this->infinity_, clo, cup, obj, integrality, rlo, rup, *(this->core->getColumnNames(strictFormat)), *(this->core->getRowNames(strictFormat)));

    delete [] clo;
    delete [] cup;
    delete [] obj;
    delete [] rlo;
    delete [] rup;
    delete [] integrality;

    this->setProblemName(filename);

    std::string fnWithExt = filename;
    fnWithExt.append(".").append(extension);
    
    if (strictFormat) // Use IEEE hex mode for numbers
        this->writeMps(fnWithExt.c_str(), 0 ,1);
    else
        this->writeMps(fnWithExt.c_str(), 0, 1);
}

void SmiSmpsIO::writeTimeFile(const char* filename, const char* extension, const bool strictFormat) {
    std::string fnWithExt = filename;
    fnWithExt.append(".").append(extension);
    CoinFileOutput* output = CoinFileOutput::create(fnWithExt, CoinFileOutput::COMPRESS_NONE);

    std::stringstream line;
    line.fill(' ');
    line << "TIME          " << this->getModProblemName() << "\nPERIODS\n";
    
    for (int stg = 0; stg < core->getNumStages(); stg++) {
        line << "    ";
        if (core->getColStart(stg) == core->getNumCols())
            line << std::setw(10) << std::left << "RHS"; // if this stage has no columns, it may have some RHS values
        else if (strictFormat)
            line << std::setw(10) << std::left << this->columnName(core->getColStart(stg));
        else
            line << this->columnName(core->getColStart(stg)) << " ";
        line << std::setw(24) << std::left << this->rowName(core->getRowStart(stg)) << " ";
        line << "STAGE_" << stg << "\n";
    }
    line << "ENDATA\n";
    output->puts(line.str());        
    delete output;
}

void SmiSmpsIO::writeStochFile(const char* filename, const char* extension, const bool strictFormat) {
    std::string fnWithExt = filename;
    fnWithExt.append(".").append(extension);
    CoinFileOutput* output = CoinFileOutput::create(fnWithExt, CoinFileOutput::COMPRESS_NONE);
    
    std::ostringstream line;
    line.fill(' ');

    if (strictFormat) {
        line << setprecision(11);
    }
    else {
        line << setprecision(20);
    }
    
    line << "STOCH         " << this->getModProblemName() << "\n";
    line << "SCENARIOS     DISCRETE\n";
    //line << "SCENARIOS     DISCRETE                 ";
    //if (tree->getLeaf(0)->getDataPtr()->getNode()->getCoreCombineRule() == SmiCoreCombineReplace::Instance())
    //    line << "REPLACE\n";
    //else
    //    line << "ADD\n";
    //    
    int num_scen = tree->getNumScenarios();
    for (int scen = 0; scen < tree->getNumScenarios(); scen++) {
        writeScenarioToStochFile(line, tree->getLeaf(scen), scen, strictFormat);
    }
    
    line << "ENDATA\n";
    output->puts(line.str());        
    delete output;
}

void SmiSmpsIO::writeScenarioToStochFile(std::ostringstream& stream, SmiTreeNode<SmiScnNode *> * node, int scenario, bool strictFormat) {
    // first, go up the tree - if possible and the parent node is still within the same scenario and not the root node
    // if thats not the case, we reached the first node for that scenario
    if (node->hasParent() && node->getParent()->scenario() == scenario ) { // We also want to write the root stuff, because it is a own node from now on
        // go up the tree
        writeScenarioToStochFile(stream, node->getParent(), scenario, strictFormat);
    } else {
        // write scenario card
        stream << " SC Scen_" << std::setw(7) << std::left << node->scenario();
        // TODO: choose setw number depending on overall number of scenarios
        if ( node->hasParent() && node->getParent() != tree->getRoot()) {
            // branches from parent scenario
            stream << "Scen_" << std::setw(7) << std::left << node->getParent()->scenario();
        } else {
            // branches from root, i.e. Scen_0.
            if (scenario == 0) // First Scenario (Scen_0) branches from ROOT, other scenarios branches from Scen_0
                stream << std::setw(10) << std::left << "'ROOT'";
            else
                stream << std::setw(10) << std::left << "Scen_0";
        }

        if (strictFormat)
            stream << std::setw(15) << std::left << tree->getLeaf(scenario)->getDataPtr()->getProb(); // write the probabilty
        else
            stream << tree->getLeaf(scenario)->getDataPtr()->getProb() << " "; // write the probabilty

        stream << "STAGE_" << node->getDataPtr()->getStage() << "\n"; // write the stage number
    }
    
    // now we can print the stochastic data
    SmiNodeData* data = node->getDataPtr()->getNode();
    if (data->getNumMatrixElements() > 0) {
        // write matrix elements
        for (int i = data->getCore()->getRowStart(data->getStage()); i < data->getCore()->getRowStart(data->getStage()+1); i++) {
            for (int j = 0; j < data->getRowLength(i); j++) {
                stream << "    " << std::setw(9) << std::left << this->columnName(data->getRowIndices(i)[j]) << " ";
                stream << std::setw(9) << std::left << this->rowName(i) << " ";
                stream << data->getRowElements(i)[j] << "\n";
            }
        }
    }

    // write row lower elements
    for (int i = 0; i < data->getRowLowerLength(); i++) {
        stream << "    RHS       " << std::setw(9) << std::left << this->rowName(data->getRowLowerIndices()[i]) << " " << data->getRowLowerElements()[i] << "\n";
    }

    // write row upper elements
    for (int i = 0; i < data->getRowUpperLength(); i++) {
        if (this->getRowSense()[data->getRowUpperIndices()[i]] != 'E')
            stream << "    RHS       " << std::setw(9) << std::left << this->rowName(data->getRowUpperIndices()[i]) << " " << data->getRowUpperElements()[i] << "\n";
    }

    // write objective elements
    for (int i = 0; i < data->getObjectiveLength(); i++) {
        stream << "    " << std::setw(9) << std::left << this->columnName(data->getObjectiveIndices()[i]) << " OBJROW         " << data->getObjectiveElements()[i] << "\n";
    }

    // write column bounds    
    if (data->getColLowerLength() > 0 || data->getColUpperLength() > 0) {
        int icl = 0, icu = 0;
        while (icl < data->getColLowerLength() || icu < data->getColUpperLength()) {
            
            if (icl < data->getColLowerLength() &&  icu < data->getColUpperLength() && data->getColLowerIndices()[icl] == data->getColUpperIndices()[icu]) {
                // lower and upper bound given for current column
                if (data->getColLowerElements()[icl] == data->getColUpperElements()[icu]) {
                    // variable is fixed
                    stream << " FX BOUND     " << std::setw(10) << std::left << this->columnName(data->getColLowerIndices()[icl]) << data->getColLowerElements()[icl] << "\n";
                } 
                else if (data->getColLowerElements()[icl] == 0.0 && data->getColUpperElements()[icu] == 1.0) {
                    // variable is binary
                    stream << " BV BOUND     " << std::setw(10) << std::left << this->columnName(data->getColLowerIndices()[icl]) << "\n";
                }
                else if (data->getColLowerElements()[icl] == -this->solverInf_ && data->getColUpperElements()[icu] == this->solverInf_) {
                    // variable is free
                    stream << " FR BOUND     " << std::setw(10) << std::left << this->columnName(data->getColLowerIndices()[icl]) << "\n";
                }
                icl++; icu++;
                continue;
            }
            
            if (icu >= data->getColUpperLength() || data->getColLowerIndices()[icl] < data->getColUpperIndices()[icu]) {
                // write column lower bound
                if (this->isInteger(icl)) {
                    // column lower bound for integer variable
                    stream << " LI BOUND     " << std::setw(10) << std::left << this->columnName(data->getColLowerIndices()[icl]) << data->getColLowerElements()[icl] << "\n";
                }
                else {
                    if (data->getColLowerElements()[icl] == -this->solverInf_) {
                        // lower bound is -infinity
                        stream << " MI BOUND     " << std::setw(10) << std::left << this->columnName(data->getColLowerIndices()[icl]) << "\n";
                    }
                    else {
                        // normal column lower bound
                        stream << " LO BOUND     " << std::setw(10) << std::left << this->columnName(data->getColLowerIndices()[icl]) << data->getColLowerElements()[icl] << "\n";
                    }
                }
                icl++;
            }
            else {
                // write column upper bound
                if (this->isInteger(icu)) {
                    // column upper bound for integer variable
                    stream << " UI BOUND     " << std::setw(10) << std::left << this->columnName(data->getColUpperIndices()[icu]) << data->getColUpperElements()[icu] << "\n";
                }
                else {
                    if (data->getColUpperElements()[icu] == this->solverInf_) {
                        // upper bound is infinity
                        stream << " PL BOUND     " << std::setw(10) << std::left << this->columnName(data->getColUpperIndices()[icu]) << "\n";
                    }
                    else {
                        // normal column upper bound
                        stream << " UP BOUND     " << std::setw(10) << std::left << this->columnName(data->getColUpperIndices()[icu]) << data->getColUpperElements()[icu] << "\n";
                    }
                }
                icu++;
            }
        }
    }

}

void SmiSmpsIO::writeSmps(const char* filename, bool winFileExtensions, bool strictFormat) {
    if (winFileExtensions) {
        //write core file
        this->writeCoreFile(filename, "cor", strictFormat);
        
        // write time file
        this->writeTimeFile(filename, "tim", strictFormat);
        
        // write stoch file
        this->writeStochFile(filename, "sto", strictFormat);
    } else {
        //write core file
        this->writeCoreFile(filename, "core", strictFormat);
        
        // write time file
        this->writeTimeFile(filename, "time", strictFormat);
        
        // write stoch file
        this->writeStochFile(filename, "stoch", strictFormat);
    }
    
    this->freeAll();
}

void SmiSmpsIO::setSampling( bool do_sample )
{
    sampling_ = do_sample;
}

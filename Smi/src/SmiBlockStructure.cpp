#include "SmiBlockStructure.hpp"

int SmiBlockStructure::getNumBlocks( int stage )
{
	return smiBlock_[stage].size();
}

SmiBlockStructure::~SmiBlockStructure()
{
	// We only have to delete the remaining SMI blocks, in any case..
	for (int j = 0; j < smiBlock_.size();j++){
		for (int i = 0; i < smiBlock_[j].size(); i++)
			delete smiBlock_[j][i];
	}
}

SmiBlockStructure::SmiBlockStructure( SmiCoreData *c, SmiCoreCombineRule *r/*=SmiCoreCombineReplace::Instance()*/ ) :
smiBlock_(0),core_(NULL),combineRule_(NULL)
{
	core_=c;
	this->setCombineWithCoreRule(r);
	// Initialize SmiBlockVector
	for (int i = 0; i < core_->getNumStages();i++)

		smiBlock_.push_back( std::vector<SmiBlock*>() );
}

void SmiBlockStructure::addSmiBlock( SmiBlock *s )
{
	smiBlock_[s->getStage()].push_back(s);
}

void SmiBlockStructure::deleteSmiBlock( int stage )
{
	assert( stage >= 0 && stage < core_->getNumStages());
	if ( !smiBlock_[stage].empty() ){
		delete smiBlock_[stage].back();
		smiBlock_[stage].pop_back();
	}
}


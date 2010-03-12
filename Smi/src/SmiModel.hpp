
#ifndef SmiModel_HPP
#define SmiModel_HPP

#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif

// Include files
#include "CoinPackedVector.hpp"
#include "CoinPackedMatrix.hpp"
#include "OsiSolverInterface.hpp"
#include "SmiScnData.hpp"
#include "SmiScenarioTree.hpp"
#include "SmiMessage.hpp"
#include "SmiScnModel.hpp"

// STL declarations
#include <map>
#include <vector>
using namespace std;


//This class is used to create a model, given an smiTree.
class SmiModel
	{
	public:

		SmiModel(void);
		~SmiModel(void);

	private:
		SmiScenarioTree<SmiScnNode *> smiTree_;
	};


//Add a scenario to the tree by specifying branch stage and ancestor scenario.
//Data is provided via COIN data structures if any or NULL.
SmiScenarioIndex addScenarioToTree(SmiScenarioTree<SmiScnNode *> * smiTree, SmiCoreData *core,
								   CoinPackedMatrix *matrix,
								   CoinPackedVector *dclo, CoinPackedVector *dcup,
								   CoinPackedVector *dobj,
								   CoinPackedVector *drlo, CoinPackedVector *drup,
								   SmiStageIndex branch, SmiScenarioIndex anc, double prob,
								   SmiCoreCombineRule *r);

//Generates a subproblem (det.eq.) for the given nodes. This does only work if indices are consistent with the tree structure
void generateSubproblem(OsiSolverInterface *osi,SmiScenarioTree<SmiScnNode *> *smiTree,SmiStageIndex stageFrom, SmiStageIndex stageTo, SmiScenarioIndex scenFrom, SmiScenarioIndex scenTo);

//Generates subproblem from tree node node.
void generateSubproblem(OsiSolverInterface *osi, SmiScenarioTree<SmiScnNode *> *smiTree, SmiScnNode* node);






#endif
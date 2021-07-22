#ifndef BranchAndBendersCut_h__
#define BranchAndBendersCut_h__
//#define _SECURE_SCL 0
// We do not measure CPU time !
#define COMPUTING_TIMER 1
#include <map>
#include <vector>
#include <mutex>
#include <atomic>

#include <boost/dynamic_bitset.hpp>
#include <boost/scoped_array.hpp>

#include "boost/threadpool.hpp"

#include <CoinPackedVector.hpp>
#include <CoinWarmStart.hpp>

#include "ParallelNestedBenders.hpp"
#include "SmiScnModel.hpp"
#include "SmiScenarioTree.hpp"
#include "BendersNodeData.hpp"

// Files necessary for cplex
#include "ilcplex/cplex.h"
#include "ilcplex/cpxconst.h"
//#include "ilcplex/cplexcheck.h"
//#include "check.c"

/************************************************************************/
/* This class implements the single tree Branch-and-Benders-cut algorithm
/************************************************************************/

class OsiSolverInterface;
class SmiCoreData;
class SmiNodeData;
class OsiStochasticBranchAndBendersCutSolverInterface;
namespace branch_and_benders_cut {
	class ThreadBranchAndBendersCut;
}

namespace branch_and_benders_cut {

	int CPXPUBLIC benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle);

class BranchAndBendersCut : public parallel_benders::ParallelNestedBenders {
	friend class ::OsiStochasticBranchAndBendersCutSolverInterface;
	friend int CPXPUBLIC branch_and_benders_cut::benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle);
public:
	BranchAndBendersCut(SmiScnModel* smi_model_ptr);
	BranchAndBendersCut(const BranchAndBendersCut& bbc) = delete;
	~BranchAndBendersCut();

	void Solve() override;
	
	/* Solve subproblems, update UB
	and aggregate new cuts if UB != LB	*/
	void ConstructOptimalityCuts();
	
	// In parallel CPLEX B&B we want to have the copy of problem data in each thread
	std::shared_ptr<branch_and_benders_cut::ThreadBranchAndBendersCut> getCopyForThread(int thread_id);
	void createCopyForThread(int thread_id);

	// TODO: temp, we need it because the nodes is solved again after no L&P cuts are added
	// How to go directly to branching if no L&P cuts added to withing fractional context
	bool LP_cuts_constructed_;

	// Get LB
	// i.e., max obj value of unprocessed nodes
	// or obj value (if found)
	// Cannot be called withing callback
	double getLB();

	BranchAndBendersCut* clone() = delete;

private:
	/* Add Continuous and/or L&P cuts to root node
	- append continuous Benders and/or L&P cuts (if any)
	or
	- generate Benders cut(s) to avoid unboundness
	*/
	void ConstructRootNodeCuts();
	
	/* Prepare MP for the B&B execution in CPLEX
	- set up CPLEX callback
	*/
	void SetUpMPSolver();
	
	// Obj values of integer feasible node solutions
	// For these UB == LB in Benders because other
	// would be rejected.
	// Updated for thread data in ConstructOptimalityCuts()
	// and for global data in GLOBAL PROGRESS context.
	std::deque<double> benders_optimal_incumbents_;

	std::map<int, std::shared_ptr<ThreadBranchAndBendersCut>> thread_copy_map_;
	std::mutex thread_copy_map_mutex_; // std::mutex for accessing copies of this
};

class ThreadBranchAndBendersCut : public BranchAndBendersCut {
	//friend class ::OsiStochasticBranchAndBendersCutSolverInterface;
	//friend int CPXPUBLIC branch_and_benders_cut::benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle);
public:
	ThreadBranchAndBendersCut(SmiScnModel* smi_model_ptr);
	ThreadBranchAndBendersCut(const BranchAndBendersCut& bbc) = delete;
	~ThreadBranchAndBendersCut();

	/* Store solution, theta and
	LB of the B&B node */
	int StoreNodeData(CPXCALLBACKCONTEXTptr context, CPXLONG contextid);

	int getCurrentNodeDepth();
	int getCurrentNodeId();

	void setCurrentNodeDepth(int value);
	void setCurrentNodeId(int value);

	/* Solve subproblems, update UB // TODO: do we updated bound?
	and solve CGPs	*/
	void ConstructLiftAndProjectCuts();

	int AppendCutsToNode(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, SmiTreeNode<parallel_benders::BendersNodeData*>* benders_tree_node_pointer);

	/* when L&P round == L&P rounds limit
	no L&P cuts are created and
	the node is supposed to be branched. 
	However, the node is solved again.
	We do this to avoid it.*/
	// TODO: rething its mechanics
	bool lp_cuts_constructed_;

private:
	int current_node_depth_; // how deep the thread is in B&B // TODO: initialize this and below in constructor
	int current_node_id_;


	/* Reject current candidate B&B node
	and add lazy cuts */
	int AddLazyCutToNode(CPXCALLBACKCONTEXTptr context, const parallel_benders::CutStuff& cut_stuff);
	/* Add user cuts after continuous solution
	was found */
	int AddUserCutToNode(CPXCALLBACKCONTEXTptr context, const parallel_benders::CutStuff& cut_stuff);
};

} // end namespace branch_and_benders_cut
#endif // BranchAndBendersCut_h__
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

	// 
	int CPXPUBLIC benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle);

class BranchAndBendersCut : public parallel_benders::ParallelNestedBenders {
	friend class ::OsiStochasticBranchAndBendersCutSolverInterface;
	friend int CPXPUBLIC branch_and_benders_cut::benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle);
public:
	BranchAndBendersCut(SmiScnModel* smi_model_ptr);
	BranchAndBendersCut(const BranchAndBendersCut& bbc) = delete;
	~BranchAndBendersCut();

	void Init() override;
	void Solve() override;
	
	/* Store solution, theta and 
	LB of the B&B node */
	int StoreNodeData(CPXCALLBACKCONTEXTptr context, CPXLONG contextid);
	
	/* Solve subproblems, update UB
	and aggregate new cuts if UB != LB	*/
	void ConstructOptimalityCuts();

	/* Solve subproblems, update UB // TODO: do we updated bound?
	and solve CGPs	*/
	void ConstructLiftAndProjectCuts();

	/* Reject current candidate B&B node
	and add lazy cuts */
	int AddLazyCutToNode(CPXCALLBACKCONTEXTptr context, const parallel_benders::CutStuff& cut_stuff);
	int AppendCutsToNode(CPXCALLBACKCONTEXTptr context, SmiTreeNode<parallel_benders::BendersNodeData*>* benders_tree_node_pointer);
	
	// In parallel CPLEX B&B we want to have the copy of problem data in each thread
	BranchAndBendersCut* getCopyForThread(int thread_id);
	void createCopyForThread(int thread_id);

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
	void SetUpMPSolver(OsiCpxSolverInterface* cpx_solver);
	
	// obj values of integer feasible node solutions
	// for which Benders UB == LB
	std::deque<double> benders_optimal_incumbents_;

	// best bounds of B&B, i.e., lower bounds
	std::deque<double> best_bounds_;

	std::map<int, BranchAndBendersCut*> thread_copy_map_; // TODO: unique ptr?
	std::mutex thread_copy_map_mutex_; // std::mutex for accessing copies of this
};

} // end namespace branch_and_benders_cut
#endif // BranchAndBendersCut_h__
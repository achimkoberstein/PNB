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
/* This class implements the nested benders decomposition in parallelized fashion, for multiple cores.
 * We need the following classes
 * Scenario Tree Storage Classes: Smi for the problem data and BendersNodeData for solver data
 * Algorithmic Logic
 * Parallelizing structures
 * Aggregation techniques
 * Problem scanning for fixed and stochastic matrices, RHS, objective, bounds to avoid unnecessary work (e.g. recreate matrices)
 * Goal: Subproblem solving and cut generation should be parallelized. Have a number of subproblems ( can be given by scenario tree ), assign a stage and a number to a thread and let it solve the corresponding subproblem.
/************************************************************************/

class OsiSolverInterface;
class SmiCoreData;
class SmiNodeData;
class OsiStochasticBranchAndBendersCutSolverInterface;


namespace branch_and_benders_cut {

class BranchAndBendersCut : public parallel_benders::ParallelNestedBenders {
	friend class ::OsiStochasticBranchAndBendersCutSolverInterface;
public:
	BranchAndBendersCut(SmiScnModel* smi_model_ptr);
	BranchAndBendersCut(const BranchAndBendersCut& bbc) = delete;
	~BranchAndBendersCut();

	void Solve();

	BranchAndBendersCut* clone() = delete;

private:
	void SetUpMPSolver(OsiCpxSolverInterface* cpx_solver);
};

static int CPXPUBLIC benders_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* cbhandle);

int test();

} // end namespace branch_and_benders_cut
#endif // BranchAndBendersCut_h__
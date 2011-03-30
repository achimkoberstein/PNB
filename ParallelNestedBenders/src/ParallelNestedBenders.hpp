#ifndef ParallelNestedBenders_h__
#define ParallelNestedBenders_h__


#include <vector>



#include <CoinPackedVector.hpp>

#include "boost/threadpool.hpp"

#include "Benders.hpp" // Just for OsiStochasticSolverInterface, needs to be outsourced to own file
#include "SmiScenarioTree.hpp" 
#include "BendersNodeData.hpp"

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

class SmiCoreData;
class SmiNodeData;

namespace parallel_benders {


class ParallelNestedBenders {
    class ProblemSolver {

    };

public:
    ParallelNestedBenders(SmiScnModel* smi_model_ptr);
    ~ParallelNestedBenders();
    void Solve();

    // Declare static variables
    static double zero_tolerance_;


private:
    void Init();
    // High-level methods we need during the algorithm
    void AdjustProblem(); // Adjust the problem and store characteristics about it
    void AdjustProblemInStage(int stage);
    void AggregateStages(int startingStage,int endingStage);

    void HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    void BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void SolveSubproblem( OsiSolverInterface* osi_solver ,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void GenerateOptimalityCut( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void AggregateOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    OsiSolverInterface* GetSolverByThread();
    
    // Constant values
    const int kT;
    const int kW;

    // Problem data
    SmiScnModel* smi_;
    SmiCoreData* core_;
    OsiSolverInterface* osi_solver_;
    std::map<boost::thread::id,OsiSolverInterface*> osi_solver_map_;
    // Scenario Tree that contains problem and solution data
    SmiScenarioTree<BendersNodeData*> tree_;
    std::vector<std::vector<SmiTreeNode<BendersNodeData*>* > > tree_nodes_by_stage_;
    // Vector that contains StageWiseInformation
    std::vector< BendersStageData* > stage_data_;
    
    // Problem characteristics
    int num_stages_;
    int num_scenarios_;
    int infinity_;
    std::vector<int> column_assignment_;

    // Important solution data
    double integer_tolerance_;
    double absolute_epsilon_;
    double relative_epsilon_;

    // Solution process
    int current_iteration_;
    const int max_iteration_;
    int minor_iteration_;
    double lower_bound_;
    double upper_bound_;

    int current_stage_;
    bool forward_;
    bool infeasible_problem_;

    // Thread related variables
    int num_cpu_; // How many cpu's do we have at our disposal?

    boost::mutex console_mutex_; // Mutex for console output
    boost::mutex solver_map_mutex_; // Mutex for access to solver map
    // Further thread management stuff
    boost::threadpool::pool thread_pool_;

};

// TODO: Problems

// We need a coordinator, that tells the threads what to do. 
// It waits for the threads to end, then does some stuff and waits again.

bool compare_equality( double first, double second, double tolerance);

bool compare_zero( double value);

bool computeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon);

RowOrderedSparseMatrixPointer create_row_ordered_sparse_matrix_pointer( SmiNodeData* smiNode);

class OsiStochasticParallelNestedBendersTwoStageSolverInterface : public OsiStochasticSolverInterface {
public:

    OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel);
    OsiStochasticParallelNestedBendersTwoStageSolverInterface(OsiSolverInterface* osiSolver, SmiScnModel* smiModel, BendersParameter& params);
    ~OsiStochasticParallelNestedBendersTwoStageSolverInterface();

    void initialSolve();

    double getObjValue();

    inline void setParams(BendersParameter &params) { this->params = params; }

    inline void setIntParam(const BendersParameter::BendersIntParam &param, const int &val) { params.setIntParam(param, val); }
    inline int getIntParam(const BendersParameter::BendersIntParam &param) { return params.getIntParam(param); }
    inline void setDoubleParam(const BendersParameter::BendersDoubleParam &param, const double &val) { params.setDoubleParam(param, val); }
    inline double getDoubleParam(const BendersParameter::BendersDoubleParam &param) { return params.getDoubleParam(param); }


private:
    BendersParameter params;

};
} // end namespace parallel_benders
#endif // ParallelNestedBenders_h__
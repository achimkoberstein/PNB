#ifndef ParallelNestedBenders_h__
#define ParallelNestedBenders_h__


#include <vector>



#include <CoinPackedVector.hpp>

#include <boost/dynamic_bitset.hpp>

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

class OsiSolverInterface;
class SmiCoreData;
class SmiNodeData;


namespace parallel_benders {


class ParallelNestedBenders {
    friend class OsiStochasticParallelNestedBendersTwoStageSolverInterface;

public:
    ParallelNestedBenders(SmiScnModel* smi_model_ptr);
    ~ParallelNestedBenders();
    void Solve();

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

    void ComputeCutCoefficients( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, const double* dual_solution, std::vector<double>* cut_coefficients_pointer);

    OsiSolverInterface* GetSolverByThread();
    bool CompareWithZero( double value );

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
    double infinity_;
    bool contains_integer_;
    // Integer information is still stored in the core file.
    boost::dynamic_bitset<> integer_problems_; // Stagewise

    // Parameter map:
    Parameter parameter_;

    // Important parameters for numerical aspects
    double zero_tolerance_;
    double integer_tolerance_;
    double absolute_epsilon_;
    double relative_epsilon_;

    // Solution process
    int current_iteration_;
    const int max_iteration_;
    int minor_iteration_;
    double lower_bound_;
    double upper_bound_;
    double temporary_upper_bound_;
    double old_lower_bound_;

    int current_stage_;
    // Direction of algorithm
    bool forward_;
    // Do we have infeasible/unbounded problems? And which?
    bool infeasible_problem_;
    bool unbounded_problem_;
    boost::dynamic_bitset<> infeasible_problems_;

    // How many cuts did we added?
    std::vector<int> added_optimality_cuts_;
    std::vector<int> added_feasibility_cuts_;

    TimerStruct timer_;

    // Thread related variables
    int num_cpu_; // How many cpu's do we have at our disposal?

    boost::mutex console_mutex_; // Mutex for console output
    boost::mutex solver_map_mutex_; // Mutex for access to solver map
    boost::mutex stage_warm_start_mutex_; // Mutex for access to warm start 
    boost::mutex upper_bound_mutex_; // Mutex for access to upper bound
    boost::mutex infeasible_problem_mutex_; // Mutex for access to infeasible problem setter 
    boost::mutex osi_solver_mutex_; // Mutex that is used if we want to clone an OsiSolverInterface
    boost::mutex add_cut_counter_mutex_; // Mutex that is used if we increase the cut counter
    // Further thread management stuff
    boost::threadpool::pool thread_pool_;

    // Fine tuning parameters
    // Warm Start
    bool warm_start_;
    bool warm_start_stage_;
    bool warm_start_scenarios_;
    // One or Many Feasibility Cuts (has runtime implications that are problem dependent).
    bool only_one_feasibility_cut_;
    // If we generate feasibility cuts, do we also want optimality cuts generated? 
    bool generate_feasibility_and_optimality_cuts_;
    // Secquencing protocol: 0 = FastForward/FastBackward, 1 = FastForward, 2 = FastBackward
    int sequencing_protocol_; 

};

// TODO: Problems

// We need a coordinator, that tells the threads what to do. 
// It waits for the threads to end, then does some stuff and waits again.

bool CompareForEquality( double first, double second, double tolerance);

bool ComputeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon);

RowOrderedSparseMatrixPointer CreateRowOrderedSparseMatrixPointer( SmiNodeData* smiNode);
void GenerateDualRayProblem(OsiSolverInterface* osi_solver);

} // end namespace parallel_benders
#endif // ParallelNestedBenders_h__
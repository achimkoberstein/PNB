#ifndef ParallelNestedBenders_h__
#define ParallelNestedBenders_h__


#include <map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/scoped_array.hpp>

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

class OsiSolverInterface;
class SmiCoreData;
class SmiNodeData;


namespace parallel_benders {


class ParallelNestedBenders {
    friend class OsiStochasticParallelNestedBendersSolverInterface;

    typedef std::map<int,int> staircase_container; // Stores the maximal stage where the coefficient appears, and the coefficient as key

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
    void DeleteSubproblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    
    void BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
	void AppendCutsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void SolveSubproblem( OsiSolverInterface* osi_solver ,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void GenerateOptimalityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void AggregateOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void AnalyseOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void CombineTwoCuts( const CutStuff&, const CutStuff& );

    void ComputeCutCoefficients( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, const double* dual_solution, std::vector<double>* cut_coefficients_pointer);

	void TransformOptimalityCutAggregation( BendersNodeData* benders_node_pointer, const std::vector< std::vector<int> >& new_aggregation_scheme );

    void InitializeBendersNodeDataTree(int num_scen_pro_cut);

    OsiSolverInterface* GetSolverByThread();
    bool CompareWithZero( double value );
    void findAggregationPossibilities( BendersNodeData* param1, std::vector< std::vector<int> >& mapping, int num_childs, int num_cuts, int num_subproblems_per_cut );
    void aggregateOldOptimalityCuts( BendersNodeData* param1,std::vector< std::vector<int> >& mapping );

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

    // External solver and problem formulation
    int lp_method_;
    int feas_cut_lp_method_;
    bool bounds_in_matrix_;

    // Solution process
    int current_iteration_;
    const int max_iteration_;
    int minor_iteration_;
    double lower_bound_;
    double upper_bound_;
    double temporary_upper_bound_;
    double old_lower_bound_;

    int current_stage_;
    // Characteristic of problem
    bool non_staircase_problem_;
    bool non_staircase_stochastic_problem_;
    // Direction of algorithm
    bool forward_;
    // Do we have infeasible/unbounded problems? And which?
    bool infeasible_problem_;
    bool unbounded_problem_;
    boost::dynamic_bitset<> infeasible_problems_;

    // How many cuts did we added?
    std::vector<int> added_optimality_cuts_;
    std::vector<int> added_feasibility_cuts_;
	int num_added_feasibility_cuts_;

    // Strategy decision
    bool adaptive_;


    Timer timer_;
    Timer wall_clock_solution_time_;
    Timer detailed_timer_;
    // Some integer relating to the detailed_timer
    int build_problem_timer_;
    int solve_problem_timer_;
    int cut_generation_timer_;

    int maximal_solution_time_;

    // Thread related variables
    int num_cpu_; // How many cpu's do we have at our disposal?

    boost::mutex console_mutex_; // Mutex for console output
    boost::mutex solver_map_mutex_; // Mutex for access to solver map
    boost::mutex stage_warm_start_mutex_; // Mutex for access to warm start 
    boost::mutex upper_bound_mutex_; // Mutex for access to upper bound
    boost::mutex infeasible_problem_mutex_; // Mutex for access to infeasible problem setter 
    boost::mutex osi_solver_mutex_; // Mutex that is used if we want to clone an OsiSolverInterface
    boost::mutex add_cut_counter_mutex_; // Mutex that is used if we increase the cut counter
	boost::mutex stage_lower_upper_bounds_mutex_; 
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
    bool store_primal_and_dual_; // If we need to store both!
    // Secquencing protocol: 0 = FastForward/FastBackward, 1 = FastForward, 2 = FastBackward, 3= EpsFastForward, 4= EpsilonFastBackward, 5= 0-withSecondIteration
    // 6 = newly tested!
    int sequencing_protocol_; 
	int second_iteration_stage_;
    double strategy_epsilon_;
    int repeat_second_iteration_; // How many times do we want to repeat second iteration?
    int second_iteration_counter_;
    bool cascading_second_iteration_; // Cascading: first up to stage 1, then up to stage 2, then up to stage 3, then up to .. second_iteration_stage_
    int current_cascading_stage_;
    bool in_second_iteration_;
	// Stage wise upper and lower bound, necessary for certain sequencing protocols
	std::vector<std::pair<double,double> > stage_lower_upper_bounds_;
	std::vector<std::pair<double,double> > old_lower_upper_bounds_;
	double temporary_stage_lower_bound_;
	double temporary_stage_upper_bound_;
    bool compute_stage_bounds_;
    bool at_last_iteration_;
    int cuts_added_last_round_;
    bool theta_approximations_everywhere_;
	bool enforce_direction_;

    // Use Warm Up Period ( start with a first stage solution and generate cuts as we go )
    bool warm_up_;
    boost::scoped_array<double> warm_up_solution_;

    // Stage Aggreagtion
    bool stage_aggregation_;
    int begin_aggregation_stage_;
    int end_aggregation_stage_;
    bool aggregate_from_start_;

	// Memory/Time consumption parameters
	// Normal case: Solver Instances per thread
	// We can change this, if we want to.
	bool seperate_first_stage_solver_;
	bool seperate_stage_solver_;

    // Help Containers for the transformation of non-staircase problems to staircase problems ( so that we can handle it )
    staircase_container non_staircase_coefficients_;
    

    // TODO: REMOVE AGAIN
    bool changed_num_rows;

    int optimality_cut_counter_;


};

// TODO: Problems

// We need a coordinator, that tells the threads what to do. 
// It waits for the threads to end, then does some stuff and waits again.
// True if equal with given tolerance
bool CompareForEquality( double first, double second, double tolerance);
// True if gap is not small enough yet
bool ComputeRelativeGap(double upper_bound, double lower_bound, double infinity, double epsilon);

RowOrderedSparseMatrixPointer CreateRowOrderedSparseMatrixPointer( SmiNodeData* smiNode);
void GenerateDualRayProblem(OsiSolverInterface* osi_solver);

} // end namespace parallel_benders
#endif // ParallelNestedBenders_h__
#ifndef ParallelNestedBenders_h__
#define ParallelNestedBenders_h__
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

#include "SmiScnModel.hpp"
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
class OsiStochasticParallelNestedBendersSolverInterface;


namespace parallel_benders {


class ParallelNestedBenders {
    friend class ::OsiStochasticParallelNestedBendersSolverInterface;

    typedef std::map<int,int> staircase_container; // Stores the maximal stage where the coefficient appears, and the coefficient as key

public:
    ParallelNestedBenders(SmiScnModel* smi_model_ptr);
    ~ParallelNestedBenders();
    void Solve();
    // Access solution methods
    double computeScenarioValue(int scenario);
    void fixFirstStageVariables(const double* solution);

    /************************************************************************/
    /* Takes ownership of solution                                          */
    /************************************************************************/
    void setWarmUpSolution( const double* solution);
    void unfixFirstStageVariables();

private:
    void Init();
    // High-level methods we need during the algorithm
    void AdjustProblem(); // Adjust the problem and store characteristics about it
    void AdjustProblemInStage(int stage);
    void AggregateStages(int startingStage,int endingStage);

    void HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void DeleteSubproblem(OsiSolverInterface* osi_solver,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    
    void BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void AppendColsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
	void AppendCutsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void SolveSubproblem( OsiSolverInterface* osi_solver ,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void SetWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void StoreWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    void SetSolverOptions( OsiSolverInterface* osi_solver);

    void GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void GenerateOptimalityCut( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void AggregateOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void AnalyseOptimalityCuts( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void CombineTwoCuts( std::shared_ptr<CutStuff>&, std::shared_ptr<CutStuff>& );

    int GetNumberOfAggregatesFromCut( std::shared_ptr<CutStuff>& cut);

    void ComputeCutCoefficients( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, const std::vector<double>& dual_solution, std::vector<double>* cut_coefficients_pointer);

    void InitializeBendersNodeDataTree(int num_scen_pro_cut);

    inline OsiSolverInterface* GetSolverByThread();
    inline bool CompareWithZero( double value );
    void StoreValuesFromSolverInArray(double*& destination_values,const double* source_values,int size);
    void StoreValuesFromSolverInVector(std::vector<double>& destination_vector,const double* source_values,int size);
    void findAggregationPossibilities( BendersNodeData* param1, std::vector< std::vector<int> >& mapping, int num_childs, int num_cuts, int num_subproblems_per_cut );
    void IntraIterationCutAggregation( OsiSolverInterface* osi_solver,  SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer  );

    void BuildAndSolveLevelProblem(OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    void SolveSolverInterface(OsiSolverInterface* osi_solver, bool use_coin = true, bool use_quadratic = false);
    inline void PrintSubproblem(OsiSolverInterface* osi_solver, int scenario, const std::string& = "");
    inline int getNumRowsProblem();
    inline int getNumThetasByStage(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void ExpandThetaVariablesInOptimalityCut(OsiRowCut* cut,int number_of_columns, int number_of_variables, int old_number_of_thetas = 0);

    void AppendDSquareCutsToSubproblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void BuildAndSolveCTripleProblem();
    void BuildAndSolveRHSProblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    // ODA-Level methods!
    void BuildAndSolveOnDemandAccuracyProblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    void HandleInitialSolutionProcedures();
    void GenerateAggregatedCuts( SmiTreeNode<BendersNodeData*> * benders_tree_node_pointer, int aggregate_number);
    void AddOptimalityCutToContainer( SmiTreeNode<BendersNodeData*> * benders_tree_node_pointer, std::shared_ptr<CutStuff> cut_stuff);
    void SetSolverByThread();
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
    int insubstantial_iteration_;
    const int max_iteration_;
    int minor_iteration_;
    int major_iteration_;
    int feasible_iteration_;
    double lower_bound_;
    double upper_bound_;
    double old_lower_bound_;
    std::vector<double> old_first_stage_solution_;

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

	int num_added_feasibility_cuts_;
    bool store_cut_information_;

    // Strategy decision
    bool adaptive_;
    int aggregation_scheme_;
    bool no_aggregation_;

    bool cut_consolidation_add_cut_;
    int cut_consolidation_strike_threshold_;
    double cut_consolidation_partial_threshold_;

    // Regularization
    bool regularization_;
    double regularized_solution_value_;
    double regularized_sigma_;
    double regularized_gamma_;
    bool regularized_initialized_rows_;
    bool regularized_initialized_cols_;
    int regularized_start_row_;
    int start_solution_;



    // D^2 C^3
    bool d_square_;
    int d_square_translation_term_;
    int d_square_lower_bound_counter_;
    std::vector<int> d_square_w_iteration_;


    Timer timer_;
    Timer wall_clock_solution_time_;
    Timer detailed_timer_;
    Timer solution_times_;
    // Some integer relating to the detailed_timer
    int build_problem_timer_;
    int solve_problem_timer_;
    int cut_generation_timer_;

    int maximal_solution_time_;

    int solution_code_;

    // Thread related variables
    int num_cpu_; // How many cpu's do we have at our disposal?

    std::mutex console_mutex_; // std::mutex for console output
    std::mutex solver_map_mutex_; // std::mutex for access to solver map
    std::mutex stage_warm_start_mutex_; // std::mutex for access to warm start 
    std::mutex upper_bound_mutex_; // std::mutex for access to upper bound
    std::mutex infeasible_problem_mutex_; // std::mutex for access to infeasible problem setter 
    std::mutex osi_solver_mutex_; // std::mutex that is used if we want to clone an OsiSolverInterface
    std::mutex add_cut_counter_mutex_; // std::mutex that is used if we increase the cut counter
	std::mutex stage_lower_upper_bounds_mutex_;  
    std::mutex misc_mutex_; // std::mutex for miscellanious stuff
    std::mutex debug_mutex_;
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
   	// Stage wise upper and lower bound, necessary for certain sequencing protocols
	std::vector<std::pair<double,double> > stage_lower_upper_bounds_;
	std::vector<std::pair<double,double> > old_lower_upper_bounds_;
	double temporary_stage_lower_bound_;// p * c * x + \theta <- actual values for this stage plus recourse function
	double temporary_stage_upper_bound_;// p * c * x <- actual values for this stage
    bool compute_stage_bounds_;
    bool at_last_iteration_;
    int cuts_added_last_round_;
    bool theta_approximations_everywhere_;
	bool enforce_direction_;
    int enforce_stage_;

    bool level_loop_forward_;
    bool level_loop_backward_;

    bool level_decomp_;
    int level_stage_;
    bool level_euclidean_;
    bool level_manhattan_;
    bool level_infinity_;
    boost::scoped_array<int> level_cstat_;
    boost::scoped_array<int> level_rstat_;
    boost::scoped_array<double> level_dnorm_;
	int level_length_;
    std::shared_ptr<CoinWarmStart> level_warm_start_;
    
    // All variables needed for on demand accuracy level decomposition
    bool oda_method_; // If we actually use On Demand Accuracy Level decomposition
    double level_lambda_; // 0 < lambda < 1
    double oda_kappa_; // 0 < kappa < (1-lambda)^2
    double oda_gap_; // Gap between oda_lower_ and oda_upper_ (absolute)
    double oda_lower_; // Lower bound for oda
    double oda_upper_; // Upper bound for oda
    double oda_first_stage_;
    double oda_phi_i_;
    double oda_tilde_q_;
    std::vector<int> oda_chosen_optimality_cut_by_scenario_;// Which cut to take from each element!
    std::vector<double> oda_current_tilde_q_s_;

    bool critical_iteration_;
    int uncritical_iteration_counter_;
    int maximal_number_of_uncritical_iterations_;
    int critical_iteration_counter_;
    double lambda_gap_;
    double last_critical_iteration_gap_;

    // Use Warm Up Period ( start with a first stage solution and generate cuts as we go )
    bool warm_up_;
    std::unique_ptr<double[]> warm_up_solution_;
    boost::scoped_array<double> regularized_solution_;
    bool regularized_update_incumbent_;

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
    int solve_relaxation_;

    bool debug_test_duplicate_indices_; // Should be set to false, otherwise slows down the problem (Multi-Cut)

    // Output parameters
    bool pretty_printing_;
    bool pretty_printing_verbose_;

    // Store solutions if necessary
    int compute_primal_solutions_for_stage_;
    int compute_dual_solutions_for_stage_;
    bool compute_objective_values_;
	bool final_solution_run_;
	int final_solution_iteration_;

    // Fixing first-stage variables : If unfixed write back to Smi->Core->cdclo
    boost::scoped_array<double> first_stage_original_col_lower_;
    boost::scoped_array<double> first_stage_original_col_upper_;
    boost::scoped_array<double> fixed_solution_;
    bool fixed_first_stage_;

    // Help Containers for the transformation of non-staircase problems to staircase problems ( so that we can handle it )
    staircase_container non_staircase_coefficients_;
    

    // TODO: REMOVE AGAIN
    int optimality_cut_counter_;
    double intra_cut_aggregation_time_;
    double adaptive_cut_time_;
    double current_solution_value_;
    double d_square_c_triple_timer_;
    double d_square_c_rhs_timer_;
    int solution_similiarity_counter_;
    int max_similarity_iteration_;

   


};

// TODO: Problems

// We need a coordinator, that tells the threads what to do. 
// It waits for the threads to end, then does some stuff and waits again.
// True if equal with given tolerance
inline bool CompareForEquality( double first, double second, double tolerance);
// True if gap is not small enough yet
inline bool RelativeGapPresent(double upper_bound, double lower_bound, double infinity, double epsilon);
inline double ComputeRelativeGap(double upper_bound, double lower_bound,double epsilon = 1e-10);



RowOrderedSparseMatrixPointer CreateRowOrderedSparseMatrixPointer( SmiNodeData* smiNode);
void GenerateDualRayProblem(OsiSolverInterface* osi_solver);
void ComputeDualSolutionForMipProblem(OsiSolverInterface* osi_solver,double* dual, double* reduced,double* col_solution);
std::vector<double> computeTTransposeX(BendersNodeData* benders_node_pointer, SmiCoreData* core_pointer,int current_stage,const std::vector<double> x);

inline double EvaluateCutWithOneAggregate(OsiRowCut* osi_cut, const double* col_solution);

} // end namespace parallel_benders
#endif // ParallelNestedBenders_h__
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
    ParallelNestedBenders(const ParallelNestedBenders& pnb) = delete;
    virtual ~ParallelNestedBenders();
    virtual void Init();
    virtual void Solve();
    int getSolutionCode() const;
    ParallelNestedBenders& operator= (const ParallelNestedBenders& pnb) = delete;
    ParallelNestedBenders* clone();

    void setParameters(parallel_benders::Parameter& params);
    void setTestsetProblemInstanceNames(std::string& testset_name, std::string& problem_name, std::string& instance_name);
    void setGlobalLPRoundsLimit(int limit);

    /*! Clear all solution process information and reset to the after Init() state.
     Optionally new parameter can be supplied.
     The aim of Reset() is to clear the pnb and prepare it for one more Solve() but avoid reinitialization of all the internal structures.
     We keep solvers
    */
	void Reset(int delete_cuts_start_it = -1, bool keep_warm_start = false, Parameter* parameter = nullptr);
    
    // Access solution methods
    double computeScenarioValue(int scenario);
    void fixFirstStageVariables(const double* solution);
	int getNumContPhasePnbIterations() const;
	int getNumContinuousOptimalityCuts() const;
    virtual int getCurrentThreadId() const; // works for thread_BBC but for PNB and BBC returns -1

    // Access data
    const SmiTreeNode<BendersNodeData*>* getTreeNode(int stage, int scenario) const;
    SmiTreeNode<BendersNodeData*>* getMutableTreeNode(int stage, int scenario);
    const BendersStageData* getStageData(int stage) const;
    BendersStageData* getMutableStageData(int stage);
	std::shared_ptr<ParallelNestedBenders> getContPhasePNB();

    std::shared_ptr<std::mutex> getMutableConsoleMutex();

    // Timers
    inline const Timer& get_lp_cuts_timer() const {return lp_cuts_timer_;};
    inline Timer& get_mutable_lp_cuts_timer() { return lp_cuts_timer_; };
    inline const Timer& get_solution_times() const { return solution_times_; };
    inline Timer& get_mutable_solution_times() { return solution_times_; };
    inline const Timer& get_timer() const { return timer_; };
    inline Timer& get_mutable_timer() { return timer_; };

    /************************************************************************/
    /* Takes ownership of solution                                          */
    /************************************************************************/
    void setWarmUpSolution( const double* solution, int num_cols_given = 0);
    void unfixFirstStageVariables();

protected:
    
    void SetLnPCGPRoundSpecifics(ParallelNestedBenders& cgp_pnb, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    ParallelNestedBenders* GetLnP_PNB_CGP_ByThread();


    void SetLnPCGPFractVarSpecifics(ParallelNestedBenders* cgp_pnb, const std::pair<int, double>& fractional_variable);

    void LogSpecialCase(bool condition, const std::string& log_message, double fract_var_value_ = 0);

    // High-level methods we need during the algorithm
    void AdjustProblem(); // Adjust the problem and store characteristics about it
    void AdjustProblemInStage(int stage);
    void AggregateStages(int startingStage,int endingStage);

    void HandleSubproblem( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void DeleteSubproblem(OsiSolverInterface* osi_solver,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    
    void BuildSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void AppendColsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
	void AppendCutsToSubproblem( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer, int append_type = PNB_CUT_APPEND_TYPE_CONSTRAINTS);
    void SolveSubproblem( OsiSolverInterface* osi_solver ,SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void StoreData( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void SetWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void StoreWarmStart( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    void SetSolverOptions( OsiSolverInterface* osi_solver);

	bool GenerateFeasibilityCut( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void GenerateOptimalityCut( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
	int ApplyCut(OsiSolverInterface* osi_solver, const CutStuff& cut_stuff, int append_type = PNB_CUT_APPEND_TYPE_CONSTRAINTS);
    void ApplyCutToLnPCGP(OsiSolverInterface* osi_solver, const CutStuff& cut_stuff, BendersNodeData* benders_node_data);
	void GenerateLiftAndProjectCut(int lp_cut_index, const std::pair<int, double>& fractional_variable);
	void AggregateOptimalityCuts( SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );
    void SetDualCGPRoundSpecifics(OsiSolverInterface& dual_cgp_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void SetDualCGPFractVarSpecifics(OsiSolverInterface* dual_cgp_solver, const std::pair<int, double>& fractional_variable);
	void SetAlphaNormalization(OsiSolverInterface* osi_solver, bool set);
	
    /** Checks if problem itself and parameters are suitable for L&P cuts method

		Checks if:
		1) L&P related parameters are valid.
		2) Additional methods of PNB are implemented for L&P cuts methods.
		3) Problem structure is valid for L&P cuts and L&P cuts for the problems of the problem class implemented.

		allowed_test_cases: every digit in the integer is related to parameter setting or instance properties. If digit is 1, than property / parameter is allowed.
			allow_le_constr			- 1st bit, e.g., 100...
			allow_ge_constr			- 2nd bit, e.g., 110...
			allow_eq_constr			- 3
			allow_non_standard_lb	- 4
			allow_non_standard_ub	- 5
			allow_cont_first_stage	- 6
			allow_feas_cuts			- 7
            allow_warm_start		- 8
            allow_multithreading	- 9
		
	*/
	bool ParallelNestedBenders::ProblemSuitableForLPCuts();

    void AnalyseOptimalityCuts( OsiSolverInterface* osi_solver, SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer );

    void CombineTwoCuts( std::shared_ptr<CutStuff>&, std::shared_ptr<CutStuff>&, int num_theta );

    int GetNumberOfAggregatesFromCut( std::shared_ptr<CutStuff>& cut, int num_theta);

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
	
	// save instance characteristics such as number columns / rows / vars to the .csv file
	// default filename is InstanceSpecs.csv
	// filename can be set with pnb::SetInstanceSpecsFile(filename)
	void SaveInstanceSpecs();
    inline int getNumRowsProblem();
    inline int getNumThetasByStage(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void ExpandThetaVariablesInOptimalityCut(std::shared_ptr<OsiRowCut>& cut,int number_of_columns, int number_of_variables, int old_number_of_thetas = 0);

    void AppendDSquareCutsToSubproblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void BuildAndSolveCTripleProblem();
    void BuildAndSolveRHSProblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    // ODA-Level methods!
    void BuildAndSolveOnDemandAccuracyProblem(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);

    void HandleContinuousPhase(); // relax the integrality constraints in the MP, solve the problem with PNB and add the continous Benders cuts to the cut container
    void HandleInitialSolutionProcedures();
    void GenerateAggregatedCuts( SmiTreeNode<BendersNodeData*> * benders_tree_node_pointer, int aggregate_number);
    void AddCutToContainer( SmiTreeNode<BendersNodeData*> * benders_tree_node_pointer, std::shared_ptr<CutStuff> cut_stuff);
    OsiSolverInterface* SetSolverByThread(OsiSolverInterface* osi_solver);

	bool CheckForSameCut(const CutStuff& cut_to_check, const BendersNodeData::cut_container & container);

    // CVaR methods
    void AggregateCVaRCuts(SmiTreeNode<BendersNodeData*>* benders_tree_node_pointer);
    void SolveStoppingCriterionProblem();
    void BuildAndSolveNextIterateCVaRProblem(OsiSolverInterface* osi_solver);
	
    // Problem data
    SmiScnModel* smi_;
    SmiCoreData* core_;
    OsiSolverInterface* osi_solver_;
    std::map<boost::thread::id,OsiSolverInterface*> osi_solver_map_;
    std::shared_ptr<ParallelNestedBenders> cont_phase_pnb_;
    // Scenario Tree that contains problem and solution data
    SmiScenarioTree<BendersNodeData*> tree_;
    std::vector<std::vector<SmiTreeNode<BendersNodeData*>* > > tree_nodes_by_stage_;
    // Vector that contains StageWiseInformation
    std::vector< BendersStageData* > stage_data_;

    
    // Problem characteristics
    int num_stages_;
    int num_scenarios_;
    double infinity_;
    // Integer information is still stored in the core file.
    boost::dynamic_bitset<> integer_problems_; // Stagewise, binary / integer / MIP

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
    int max_iteration_;
    int cont_phase_it_limit_;
    bool use_first_phase_;
    int minor_iteration_;
    int major_iteration_;
    int feasible_iteration_;
    double lower_bound_;
    double upper_bound_;
    double old_lower_bound_;
    std::vector<double> old_first_stage_solution_;
	std::vector<std::pair<double, double>> iteration_u_and_l_bounds_;
    int solved_counter_; // number of times Solve() was applied to this pnb

    int current_stage_;
    // Characteristic of problem
    bool non_staircase_problem_;
    bool non_staircase_stochastic_problem_;
	std::string instance_specification_file_; // .csv to store the instance specification
	std::string testset_name_; // e.g., felt
	std::string problem_name_; // e.g., airlift
	std::string instance_name_; // e.g., airl2, normally takes a .sto file name
	bool instance_specs_to_file_; // whether to store data to instance_specification_file_

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

    // Stochastic Cutting Plane Method (aka Sampling)
    bool stochastic_cutting_plane_;
    int number_of_observations_to_sample_; // Can use LHS for example



    // D^2 C^3
	bool auto_d2_algorithm_; // algorithm will be used automatically when necessary
    bool d_square_;
    int d_square_translation_term_;
    int d_square_lower_bound_counter_;
    std::vector<int> d_square_w_iteration_;


    Timer timer_; // timer for stage 1, stage 2, ..., Init() + time until main loop in Solve() + Reset()
    Timer wall_clock_solution_time_; // timer of the main loop in Solve()
    Timer detailed_timer_; // 0 - build master (incl. as part of L&P cuts since will be used for major iteration), 1 - build projections, 2 - build oda [never used], 3 - build subs: sum of 2nd stage: node->build_time_, 4 - cut generation: sum of 2nd stage node->cut_generation_time_
    Timer solution_times_; // 0 - solve master (excl. solving the relaxation for L&P cuts), 1 - solve projections, 2 - solve oda, 3 - solve sub: sum of 2nd stage: node->solution_time_, 4 - cont phase wall clock
    Timer lp_cuts_timer_; // 0 - solve MP relaxation, 1 - build and solve PNB CGP (parallel), 2 - build Dual CGP (parallel), 3 - L&P generation wall clock time, 4 - solve Dual CGP (parallel), 5 - consolidate PNB CGP cuts timer

    int maximal_solution_time_;

    int solution_code_;

	std::vector<int> opt_cut_iterations_;
	std::vector<int> feas_cut_iterations_;

    // Thread related variables
    int num_cpu_; // How many cpu's do we have at our disposal?

    std::shared_ptr<std::mutex> console_mutex_ptr_; // std::mutex for console output, might be shared with other classes
    std::mutex solver_map_mutex_; // std::mutex for access to solver map
    std::mutex stage_warm_start_mutex_; // std::mutex for access to warm start 
    std::mutex upper_bound_mutex_; // std::mutex for access to upper bound
    std::mutex infeasible_problem_mutex_; // std::mutex for access to infeasible problem setter 
    std::mutex osi_solver_mutex_; // std::mutex that is used if we want to clone an OsiSolverInterface
    std::mutex cgp_pnb_map_mutex_; // std::mutex that is used if we want to read/add CGP PNBs for threads
    std::mutex smi_mutex_; // std::mutex that is used if we want to initialize new PNB (in particular, CGP PNB for thread)
    std::mutex add_cut_counter_mutex_; // std::mutex that is used if we increase the cut counter
	std::mutex stage_lower_upper_bounds_mutex_;  
    std::mutex lp_dual_cgp_node_data_mutex_; //dual L&P CGP elements, its objective, ub, lb...
    std::mutex lp_round_cuts_mutex_;
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
	std::vector<std::pair<double,double> > stage_old_lower_upper_bounds_;
	bool compute_stage_bounds_; //used for PNB_INT_FASTPASS_WITH_FASTBACK sequencing and regularization_
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
    int num_oda_aggregates_;
    std::vector<int> oda_chosen_optimality_cut_by_scenario_;// Which cut to take from each element!
    std::vector<double> oda_current_tilde_q_s_;
	bool csaba_steepest_edge_;

    bool critical_iteration_;
    int uncritical_iteration_counter_;
    int maximal_number_of_uncritical_iterations_;
    int critical_iteration_counter_;
    double lambda_gap_;
    double last_critical_iteration_gap_;

	// AK: CVaR risk constraint decomposition
	bool cvar_constr_decomp_;
    double cvar_beta_;
    double cvar_rho_;
    double cvar_H_;
    double cvar_h_i_;
    double cvar_alpha_;
    double cvar_problem_lower_bound_;
    bool cvar_first_stage_;
    bool cvar_stopping_first_round_;
    bool cvar_stopping_second_round_;
    bool cvar_feasible_first_stage_solution_;
    bool cvar_next_iterate_problem_;
    bool change_cvar_alpha_;
    bool stop_cvar_;
    OsiSolverInterface* osi_stopping_criterion_solver_;

    std::vector<int> cvar_substantial_iterations_;
    std::vector<double> cvar_recourse_approximations_;
    std::vector<double> cvar_cvar_approximations_;
    std::vector<std::vector<double> > cvar_substantial_points_;
    std::vector<double> cvar_substantial_points_first_stage_objective_function_value_;

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
/* Amount of print out:
   0 - google::INFO
   1 - google::WARNING
   2 - google::ERROR
   3 - google::FATAL
   
   We do not google::SetStderrLogging(log_level) because it will set globally for the whole application
   We use LOG_IF / DLOG everywhere due to the efficiency concerns
   LOG(INFO) vs LOG_IF(INFO, log_level_ <= google::INFO). First will slower the application much even if log_level_ > google::INFO
*/
    int log_level_;
    bool pretty_printing_;
    bool pretty_printing_verbose_;
	bool print_subproblems_;

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

    // Handle numerical issues!
    std::vector<double> old_discrepancies_;
    
	int lift_and_p_method_;
	int lp_rounds_limit_;
    int global_lp_rounds_limit_; // used to build L&P rounds in B&B
	int lp_switches_limit_;
	int lp_major_it_limit_;
    int lp_parallel_mode_;
    int lp_use_cgp_pnb_cuts_;
	int cuts_for_fract_vars_;
	double cgp_relative_epsilon_;
	int cgp_do_when_infeasible_; // whether we quit OR apply Alpha-Normalization and solve OR apply Alpha-Normalization and continue when L&P CGP PNB is infeasible
	int current_lp_round_; // get rid of these variable and count them based on L&P cuts added or next_lp_rounds_iteration_ from cut container
    int lp_rounds_counter_;
	int lp_switch_counter_;
	bool generate_lp_cuts_;
	int next_lp_rounds_iteration_;
	std::map<double, std::pair<int, double>> fractional_variables_; //<fractional part, <var index, var value> > ordered ascending by int violation
	double lp_main_loop_fract_var_value_; // this is the value of the fractional variable from the main loop
	std::unique_ptr<std::pair<int, int>> lp_main_loop_iteration_round_; // first is iteration, 2nd is L&P round
	int lp_column_set_;
    std::map<boost::thread::id, ParallelNestedBenders*> cgp_pnb_map_;
	std::bitset<11> allowed_test_cases_;

    // TODO: REMOVE AGAIN
    int optimality_cut_counter_;
    double intra_cut_aggregation_time_;
    double adaptive_cut_time_;
    double current_solution_value_;
    double d_square_c_triple_timer_;
    double d_square_c_rhs_timer_;
    int solution_similiarity_counter_;
    int max_similarity_iteration_;

   


public:
    void setLogLevel(int level);
};

// TODO: Problems

inline const char* const BoolToString(bool b);
inline int NthFind(int n, const string& s, const string& p);

// We need a coordinator, that tells the threads what to do. 
// It waits for the threads to end, then does some stuff and waits again.
// True if equal with given tolerance
inline bool CompareForEquality( double first, double second, double tolerance);
// True if gap is larger than Epsilon
inline bool RelativeGapLargerEpsilon(double upper_bound, double lower_bound, double infinity, double epsilon);
inline double ComputeRelativeGap(double upper_bound, double lower_bound,double comparison_epsilon = 1e-10);
bool CutIsEqual(const CutStuff& first, const CutStuff& second, double tolerance);

bool DoubleIsHigher(const std::pair<int,double>& first, const std::pair<int,double>& second);

void ConsolidateCuts(BendersNodeData::cut_container& in_cuts, BendersNodeData::cut_container& out_cuts, BendersNodeData::cut_container::const_iterator from_cut);

RowOrderedSparseMatrixPointer CreateRowOrderedSparseMatrixPointer( SmiNodeData* smiNode);
void GenerateDualRayProblem(OsiSolverInterface* osi_solver);
void ComputeDualSolutionForMipProblem(OsiSolverInterface* osi_solver,double* dual, double* reduced,double* col_solution);
std::vector<double> computeTTransposeX(BendersNodeData* benders_node_pointer, SmiCoreData* core_pointer,int current_stage,const std::vector<double> x);

inline double EvaluateCutWithOneAggregate(const OsiRowCut* osi_cut, const double* col_solution);
inline double EvaluateCut(OsiRowCut* osi_cut, const double* col_solution);

} // end namespace parallel_benders
#endif // ParallelNestedBenders_h__
#ifndef BendersNodeData_h__
#define BendersNodeData_h__

#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <atomic>

#include <boost/dynamic_bitset.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>


class CoinPackedMatrix;
class CoinPackedVector;
class CoinWarmStart;
class OsiRowCut;
class OsiSolverInterface;
class SmiNodeData;


namespace parallel_benders{

    // Parameter names go in here. We use std::string as parameter
    // IMPORTANT: If you add a new string here, also set the default value in the init() method of Parameter class
    // We start with integer parameters
    const std::string PNB_BENDERS = "PNB_BENDERS";          //
    const std::string PNB_WARM_START = "PNB_WARM_START";    //
    const std::string PNB_LP_METHOD = "PNB_LP_METHOD";
    const std::string PNB_FEAS_CUT_LP_METHOD = "PNB_FEAS_CUT_LP_METHOD";
    const std::string PNB_AGGREGATED_OPTIMALITY_CUTS = "PNB_AGGREGATED_OPTIMALITY_CUTS";    //
    const std::string PNB_RELAXATION = "PNB_RELAXATION";                   //
    const std::string PNB_DYNAMIC_AGGREGATION = "PNB_DYNAMIC_AGGREGATION";          //
	const std::string PNB_NUM_CPU = "PNB_NUM_CPU"; // 0 = take what hardware_concurrency offers you, otherwise equal to the number of CPU we would like to use.
	const std::string PNB_CPU_USED = "PNB_CPU_USED"; // indicates how many CPU we used ( we have to set that from the solver)
	const std::string PNB_TREE_WALK_STRATEGY = "PNB_TREE_WALK_STRATEGY"; // Different Strategies. 0 = FastForw/FastBack, 1 = FastForw, 2 = FastBack, 3 = EpsFastForw, 4 = EpsFastBack, 5 = FastForw/FastBackWith2nd.Iteration
	const std::string PNB_TREE_WALK_STRATEGY_STAGE = "PNB_TREE_WALK_STRATEGY_STAGE"; // up to which stage do we want to follow the given strategy? else we resort to Fastpass
    const std::string PNB_TREE_WALK_STRATEGY_EPSILON = "PNB_TREE_WALK_STRATEGY_EPSILON";
    const std::string PNB_HANDLE_FEAS_CUTS = "PNB_HANDLE_FEAS_CUTS";
    const std::string PNB_STAGE_AGGREGATION = "PNB_STAGE_AGGREGATION";
    const std::string PNB_STAGE_AGGREGATION_START = "PNB_STAGE_AGGREGATION_START";
    const std::string PNB_STAGE_AGGREGATION_END = "PNB_STAGE_AGGREGATION_END";
    const std::string PNB_STAGE_AGGREGATION_DIRECTION = "PNG_STAGE_AGGREGATION_DIRECTION"; // In which direction do we want to aggregate if difference is greater than one? True means from start to end, false means from end to start
    const std::string PNB_USE_START_SOLUTION = "PNB_USE_START_SOLUTION"; // If we use a start solution or not. At the moment start solution is EV solution!
    const std::string PNB_BOUNDS_IN_MATRIX = "PNB_BOUNDS_IN_MATRIX";
    const std::string PNB_CUT_CONSOLIDATION = "PNB_CUT_CONSOLIDATION";
    const std::string PNB_CUT_CONSOLIDATION_CUT_COMPUTATION = "PNB_CUT_CONSOLIDATION_CUT_COMPUTATION";
    const std::string PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD = "PNB_CUT_CONSOLIDATION_STRIKE_THRESHOLD";
    const std::string PNB_CUT_CONSOLIDATION_PARTIAL_THRESHOLD = "PNB_CUT_CONSOLIDATION_PARTIAL_THRESHOLD";
    const std::string PNB_MAXIMAL_SOLUTION_TIME = "PNB_MAXIMAL_SOLUTION_TIME";
    const std::string PNB_LEVEL_LAMBDA = "PNB_LEVEL_LAMBDA";
    const std::string PNB_ODA_KAPPA = "PNB_ODA_KAPPA";
    const std::string PNB_STAGE_LEVEL = "PNB_STAGE_LEVEL";



    // Here come the double parameters
    const std::string PNB_TOLERANCE_ZERO = "PNB_TOLERANCE_ZERO";             //
    const std::string PNB_TOLERANCE_INTEGER = "PNB_TOLERANCE_INTEGER";       //
    const std::string PNB_EPSILON_ABSOLUTE = "PNB_EPSILON_ABSOLUTE";           //
    const std::string PNB_EPSILON_RELATIVE = "PNB_EPSILON_RELATIVE";           // 

    // Do we want to compute (and return) solutions?
    const std::string PNB_COMPUTE_PRIMAL_SOLUTIONS = "PNB_COMPUTE_PRIMAL_SOLUTIONS";
    const std::string PNB_COMPUTE_DUAL_SOLUTIONS = "PNB_COMPUTE_DUAL_SOLUTIONS";
    const std::string PNB_COMPUTE_OBJECTIVE_VALUES = "PNB_COMPUTE_OBJECTIVE_VALUES";


	// Some fixed integer to avoid magic numbers
	const int PNB_INT_FASTPASS = 0;
	const int PNB_INT_FAST_FORWARD = 1;
	const int PNB_INT_FAST_BACKWARD = 2;
	const int PNB_INT_EPSILON_FAST_FORWARD = 3;
	const int PNB_INT_EPSILON_FAST_BACKWARD = 4;
	const int PNB_INT_SECOND_ITERATION = 5;
    const int PNB_INT_FASTPASS_WITH_FASTBACK = 6;
	const int PNB_INT_STAGE_AGGREGATION_LAST_STAGE = -1;

    // some fixed integer for handle feas cuts
    const int PNB_INT_FEAS_CUT_ONLY_ONE = 0;
    const int PNB_INT_FEAS_CUT_ALL = 1;
    const int PNB_INT_FEAS_CUT_WITH_OPT = 2;

    // Dynamic Adapative Aggregation strategies
    const int PNB_INT_AGGREGATION_STRATEGY_BY_INACTIVE_ITERATION = 1;
    const int PNB_INT_AGGREGATION_STRATEGY_BY_AGGREGATE_DISCREPANCY = 2;

    const int PNB_INT_NO_START_SOLUTION = 0;
    const int PNB_INT_COMPUTE_EV_SOLUTION = 1;
    const int PNB_INT_USE_EV_SOLUTION = 2;
    const int PNB_INT_USE_SUPPLIED_SOLUTION = 3;

    // Specify algorithm
    const int PNB_INT_ALGORITHM_DETEQ = 0;
    const int PNB_INT_ALGORITHM_BENDERS = 1;
    const int PNB_INT_ALGORITHM_REGULARIZATION = 2;
    const int PNB_INT_ALGORITHM_LEVEL = 3;
    const int PNB_INT_ALGORITHM_LEVEL_ODA = 4;
    const int PNB_INT_ALGORITHM_BENDERS_ODA = 5;
    const int PNB_INT_ALGORITHM_REGULARIZATION_ODA = 6;
    const int PNB_INT_ALGORITHM_LEVEL_MANHATTAN = 7;
    const int PNB_INT_ALGORITHM_LEVEL_MANHATTAN_ODA = 8;
    const int PNB_INT_ALGORITHM_LEVEL_INFINITY = 9;
    const int PNB_INT_ALGORITHM_LEVEL_INFINITY_ODA = 10;


    const int PNB_INT_PRIMAL_SIMPLEX = 10;
    const int PNB_INT_DUAL_SIMPLEX = 11;
    const int PNB_INT_BARRIER = 12;
    const int PNB_INT_BARRIER_WITH_CROSSOVER = 13;
    const int PNB_INT_CONCURRENT_MASTER_STANDARD = 14; // Uses dual simplex for subproblems, but concurrent master optimizer

    // SOlution handling
    const int PNB_INT_SOLUTION_PRIMAL_NO = -2;
    const int PNB_INT_SOLUTION_PRIMAL_ALL = -1;
    const int PNB_INT_SOLUTION_DUAL_NO = -2;
    const int PNB_INT_SOLUTION_DUAL_ALL = -1;

    // Relaxed problem
    const int PNB_INT_NO_RELAXATION = -1;

    // Solution code
    const int PNB_SOLUTION_OPTIMAL = 0;
    const int PNB_SOLUTION_INFEASIBLE = 1;
    const int PNB_SOLUTION_UNBOUNDED = 2;
    const int PNB_SOLUTION_TIME_LIMIT = 3;
    const int PNB_SOLUTION_ITERATION_LIMIT = 4;
    const int PNB_SOLUTION_SIMILARITY_LIMIT = 5;
    const int PNB_SOLUTION_GAP_NONIMPROVING = 6;
    const int PNB_SOLUTION_SUBSOLVER_PROBLEM = 7;
    const int PNB_SOLUTION_PARAMETER_PROBLEM = 8;
    const int PNB_SOLUTION_UPPER_LESS_LOWER_BOUND = 9;


    struct BendersNodeData;

    struct CutStuff
    {
        const static int AGGREGATE_OVER_ALL = -1;
        const static int AGGREGATE_OVER_PARTIAL = -2;
        static double active_threshold_; // If active_value is less than
        static int strike_threshold_; // If non_active_iterations_ is more than

        CutStuff(int iteration,std::shared_ptr<OsiRowCut>& cut_pointer);

        int cut_creation_iteration_;
        int not_active_iterations_;
        int aggregate_number_; // We can extend that to a list of aggregates if this is necessary
        std::shared_ptr<OsiRowCut> cut_;
        double active_value; // Used to rate cuts, how good is a cut, how useful was it in the course of the algorithm?
        int current_index_in_solver_; // Needs to be updated at each run to be meaningful
        bool to_use_;
    };

    // To be able to use find_if on std::list< std::shared_ptr<CutStuff> > in conjunction with active values
    struct CutStuffComparison: public std::binary_function< std::shared_ptr<CutStuff>, int, bool > {
        bool operator () ( const std::shared_ptr<CutStuff> &cut_stuff, const int &number ) const {
            return cut_stuff->cut_creation_iteration_ == number && cut_stuff->active_value < CutStuff::active_threshold_ && cut_stuff->not_active_iterations_ > CutStuff::strike_threshold_;
        }
    };

    struct CutStuffLessComparison: public std::binary_function< std::shared_ptr<CutStuff>, std::shared_ptr<CutStuff>, bool > {
        bool operator () ( const std::shared_ptr<CutStuff> &cut_stuff, const std::shared_ptr<CutStuff> &cut_stuff2 ) const {
            return cut_stuff->cut_creation_iteration_ < cut_stuff2->cut_creation_iteration_ || cut_stuff->aggregate_number_ < cut_stuff2->aggregate_number_;
        }
    };

    

    bool is_active (const std::shared_ptr<CutStuff>& value);

// We can make it a struct anyway?!
struct BendersNodeData 
{
    friend class CutManager;
public:
    BendersNodeData();
    BendersNodeData(SmiNodeData* nodeData);
    ~BendersNodeData();

    void CleanUnusedMemory(bool direction);

    // Information about the solution
    std::vector<double> col_solution_;
    std::vector<double> dual_solution_;
	std::vector<double> incumbent_col_solution_;
    std::vector<double> reduced_cost_; // Only needed if we have variable bounds! Reduced costs = dual logicals
    std::vector<double> next_stage_approximation_;
    double* discrepancy_; // Store discrepancy of this approximation to the actual achieved value
    boost::dynamic_bitset<> was_under_threshold_; // Has to have the size of num_thetas_
    double dual_value_variable_bounds_;
    bool feasible_;
    bool unbounded_;
    int unbounded_row_;

    // Information about node-wise upper and lower bounds
    double lower_bound_;
    double upper_bound_;
    double temporary_upper_bound_;

    std::unique_ptr<double[]> mip_solution_;

    bool unbounded_problem_; // If we should generate a unbounded_right_hand_side for a special problem?
    std::unique_ptr<double[]> unbounded_extreme_ray_; // Stores an extreme ray!
    std::unique_ptr<double[]> unbounded_right_hand_side_; // Stores right hand side generated for extreme ray subproblem!
    bool solve_ray_right_hand_side_; // If we actually solve the ray_right_hand_side!
    

    int cols_; 
    int rows_;
    int num_thetas_; // How many aggregates do we have at this node
    boost::dynamic_bitset<> active_aggregates_; // Has to have the size of num_thetas_
    std::unique_ptr<CoinWarmStart> warm_start_;
    bool warm_start_added_; // Did we already added a warm start to a problem? Gets reset by a call to StoreWarmStart!
    std::unique_ptr<CoinWarmStart> level_warm_start_;
    std::unique_ptr<int[]> level_warm_start_cstat_;
    std::unique_ptr<int[]> level_warm_start_rstat_;
    std::unique_ptr<double[]> level_warm_start_dnorm_;
    int level_warm_start_length_;



    // Level regularization solution
    std::vector<double> level_regularized_solution_;
    int level_regularized_start_row_;

    SmiNodeData* smi_node_data_;
    int scenario_index_;
    int branch_scenario_;
    int num_node_in_stage_;
    int child_node_start_;
    double probability_; // Overall probability
    double conditional_probability_; // Probability with respect to the parent node
    // solution without approximation terms
    double solution_value_;
	double incumbent_solution_value_;
    // Store optimality and feasibility cuts. We can also use a deque for that, if we want to remove elements later on..
    typedef std::list<std::shared_ptr<CutStuff> > cut_container;
    cut_container optimality_cuts_;
    cut_container feasibility_cuts_;
    cut_container::iterator last_iteration_start_optimality_;
    cut_container::iterator last_iteration_start_feasibility_;
    cut_container::iterator level_opt_iterator_;

    
    // Multicut Aggregation Strategies..
    int num_scen_pro_cut_;
    // We should store a vector of bitsets, needs less memory // TODO
    std::vector< std::vector<int> > nodes_in_cut_; // Stores which subnodes of current node are aggregated to a cut.

    std::vector< std::vector<int> > oda_aggregates_;

	// If we changed the cut aggregation level the last time we added cuts and before we build the subproblem again
	bool changed_aggregation_level_;
    bool alternating_intra_cut_adaptive_; // True = Intra_cut, False = Adaptive

    // Values for cut aggregation
    std::vector<double> current_cut_coefficients_;
    double current_cut_rhs_;

    // Subproblem elements
    CoinPackedMatrix* matrix_;
    double* column_lower_;
    double* column_upper_;
    double* objective_;
    double* row_lower_;
    double* row_upper_;

    // In the non-staircase case
    std::vector< std::vector<std::pair<int,double> > > no_staircase_constraints_;

    // D^2C^3 Stuff
    CoinPackedMatrix* d_square_cut_t_matrix_;
    std::vector<double> d_square_cut_rhs_;
    double* solver_rhs_;
    bool can_safely_reuse_;

    // ODA-Level decomp
    std::vector<std::shared_ptr<OsiRowCut> > oda_optimality_cuts_;
    std::vector<double> oda_cut_evaluation_;
    std::map<int,double> oda_lambdas_;
    
};

// Methods that work on a BendersNodeData
void createRoundRobinAggregation(BendersNodeData* benders_node_data, int num_child, int num_cuts, int num_subpropblems_per_cut);

double computeEuclideanNorm(OsiRowCut* cut);
double computeCutViolation( OsiRowCut* cut, double* solution, double infinity );

struct BendersStageData {

    BendersStageData();
    ~BendersStageData();

    // Store problem data for each stage
    int num_columns_;
    int num_rows_;
    int stage_; // Just to be sure
    int num_thetas_; // Number of variables that represent the approximation of the next stage

    // Store stage-wise upper and lower bounds
    double upper_bound_;
    double lower_bound_; 

    // Store information about stochasticity in stage
    bool stochastic_matrix_w_;
    bool stochastic_matrix_t_;
    bool stochastic_rhs_;
    bool stochastic_objective_;
    bool stochastic_bounds_;
    bool non_zero_bounds_;

    // Store pointers to matrices W and / or T if they do not change ?
    std::vector<int> column_indices_;
    std::vector<int> row_indices_;
    // Coin Warm Start object
    std::unique_ptr<CoinWarmStart> warm_start_;
    bool problem_was_solved_in_this_iteration_;

	std::map<boost::thread::id,OsiSolverInterface*> osi_solver_map_; // Store solver interfaces for every stage 

    // In the non-staircase case
    std::vector< std::vector<std::pair<int,double> > > no_staircase_constraints_;

    // Store if we changed aggregation level
    bool changed_aggregation_level_;

	std::atomic_int added_optimality_cuts_;
	std::atomic_int added_feasibility_cuts_;

    // D^2C^3 stuff
    int num_d_square_cuts_;
    CoinPackedMatrix* d_square_matrix_;
    std::vector<int> non_integer_scenarios;
    int violated_index_;
    double* d_square_lambda_0_1_;
    double* d_square_lambda_1_1_;
    double d_square_lambda_0_2_;
    double d_square_lambda_1_2_;
    double* d_square_pi_;
    std::vector<int> d_square_disjunctive_indices_;
    bool solve_binary_subproblems_;

    // MIP warm start and mip handling
    std::vector<std::vector<double> > mip_solution_;
    std::vector<char> coltype_array_;

    // Level stuff
    bool use_level_;

    

};

struct RowOrderedSparseMatrix{

    RowOrderedSparseMatrix(const std::vector<double>& elements, const std::vector<int>& indices, const std::vector<int>& start);
    
    // Store the matrix as vectors
    std::vector<double> elements_;
    std::vector<int> indices_;
    std::vector<int> start_;
};

struct RowOrderedSparseMatrixPointer{

    RowOrderedSparseMatrixPointer(const double* elements, const int* indices, const int* start);

    // Store the matrix as vectors
    const double* elements_;
    const int* indices_;
    const int* start_;
};

class Timer {
public:
    explicit Timer(int stgs) : stages_(stgs),start_time_(stages_,0.0),overall_time_(stages_,0.0) {}
    ~Timer() { }

    void Start(int stg);
    void Stop(int stg);

    void AddTime(int stg, double time);

    double GetTime(int stg) const { return overall_time_[stg]; }

    double GetAggregatedTime() const;

    std::string GetFullTimeString() const;

    int GetTimeStages() const;

private:
    int stages_;
    std::vector<double> start_time_;
    std::vector<double> overall_time_;
};


struct Parameter {

    Parameter() {
        init();
    }
    // Set default values for all strings we have..
    void init();


    //// Set values you want to set
    //void SetParameterValue(std::string parameter, double value){
    //    parameter_map_[parameter] = value;
    //}
    //double GetParameterValue(std::string parameter){
    //    return parameter_map_[parameter];
    //}

    // Stores every parameter as a std::string with a value that is stored as either double or int..
    boost::property_tree::ptree parameter_;
};

// Comparator and Helper functions

bool comparator_less_int_double_pair ( const std::pair<int,double>& l, const std::pair<int,double>& r);


} // end namespace parallel_benders
#endif // BendersNodeData_h__
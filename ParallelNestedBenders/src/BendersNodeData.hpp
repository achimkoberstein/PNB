#ifndef BendersNodeData_h__
#define BendersNodeData_h__

#include <vector>
#include <map>
#include <functional>

#include <boost/dynamic_bitset.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
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
    const std::string PNB_SCENARIOS_PRO_CUT = "PNB_SCENARIOS_PRO_CUT" ;            //
    const std::string PNB_RELAXATION = "PNB_RELAXATION";                   //
    const std::string PNB_DYNAMIC_AGGREGATION = "PNB_DYNAMIC_AGGREGATION";          //
    const std::string PNB_TRUST_REGION = "PNB_TRUST_REGION";                 //
	const std::string PNB_NUM_CPU = "PNB_NUM_CPU"; // 0 = take what hardware_concurrency offers you, otherwise equal to the number of CPU we would like to use.
	const std::string PNB_CPU_USED = "PNB_CPU_USED"; // indicates how many CPU we used ( we have to set that from the solver)
	const std::string PNB_TREE_WALK_STRATEGY = "PNB_TREE_WALK_STRATEGY"; // Different Strategies. 0 = FastForw/FastBack, 1 = FastForw, 2 = FastBack, 3 = EpsFastForw, 4 = EpsFastBack, 5 = FastForw/FastBackWith2nd.Iteration
	const std::string PNB_TREE_WALK_STRATEGY_STAGE = "PNB_TREE_WALK_STRATEGY_STAGE"; // up to which stage do we want to follow the given strategy? else we resort to Fastpass
    const std::string PNB_TREE_WALK_STRATEGY_EPSILON = "PNB_TREE_WALK_STRATEGY_EPSILON";

    const std::string PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_CASCADE = "PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_CASCADE";
    const std::string PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_REPEAT = "PNB_TREE_WALK_STRATEGY_SECOND_ITERATION_REPEAT";

    const std::string PNB_STAGE_AGGREGATION = "PNB_STAGE_AGGREGATION";
    const std::string PNB_STAGE_AGGREGATION_START = "PNB_STAGE_AGGREGATION_START";
    const std::string PNB_STAGE_AGGREGATION_END = "PNB_STAGE_AGGREGATION_END";
    const std::string PNB_STAGE_AGGREGATION_DIRECTION = "PNG_STAGE_AGGREGATION_DIRECTION"; // In which direction do we want to aggregate if difference is greater than one? True means from start to end, false means from end to start
    const std::string PNB_USE_START_SOLUTION = "PNB_USE_START_SOLUTION"; // If we use a start solution or not. At the moment start solution is EV solution!
    const std::string PNB_BOUNDS_IN_MATRIX = "PNB_BOUNDS_IN_MATRIX";
    const std::string PNB_INTRA_CUT_AGGREGATION = "PNB_INTRA_CUT_AGGREGATION";
    const std::string PNB_INTRA_CUT_AGGREGATION_STRATEGY = "PNB_INTRA_CUT_AGGREGATION_STRATEGY";

    const std::string PNB_MAXIMAL_SOLUTION_TIME = "PNB_MAXIMAL_SOLUTION_TIME";


    // Here come the double parameters
    const std::string PNB_TOLERANCE_ZERO = "PNB_TOLERANCE_ZERO";             //
    const std::string PNB_TOLERANCE_INTEGER = "PNB_TOLERANCE_INTEGER";       //
    const std::string PNB_EPSILON_ABSOLUTE = "PNB_EPSILON_ABSOLUTE";           //
    const std::string PNB_EPSILON_RELATIVE = "PNB_EPSILON_RELATIVE";           // 


	// Some fixed integer to avoid magic numbers
	const int PNB_INT_FASTPASS = 0;
	const int PNB_INT_FAST_FORWARD = 1;
	const int PNB_INT_FAST_BACKWARD = 2;
	const int PNB_INT_EPSILON_FAST_FORWARD = 3;
	const int PNB_INT_EPSILON_FAST_BACKWARD = 4;
	const int PNB_INT_SECOND_ITERATION = 5;
    const int PNB_INT_FASTPASS_WITH_FASTBACK = 6;
	const int PNB_INT_STAGE_AGGREGATION_LAST_STAGE = -1;
	// Intra cut aggregation strategies

    // Dynamic Adapative Aggregation strategies
    const int PNB_INT_AGGREGATION_STRATEGY_BY_INACTIVE_ITERATION = 1;
    const int PNB_INT_AGGREGATION_STRATEGY_BY_AGGREGATE_DISCREPANCY = 2;


    const int PNB_INT_PRIMAL_SIMPLEX = 10;
    const int PNB_INT_DUAL_SIMPLEX = 11;
    const int PNB_INT_BARRIER_MASTER = 12;

    struct BendersNodeData;

    struct CutStuff
    {
        static double active_threshold_; // If active_value is less than
        static int strike_threshold_; // If non_active_iterations_ is more than

        CutStuff(int iteration, boost::shared_ptr<OsiRowCut>& cut_pointer);

        int cut_creation_iteration_;
        int not_active_iterations_;
        int aggregate_number_; // We can extend that to a list of aggregates if this is necessary
        boost::shared_ptr<OsiRowCut> cut_;
        double active_value; // Used to rate cuts, how good is a cut, how useful was it in the course of the algorithm?
        int current_index_in_solver_; // Needs to be updated at each run to be meaningful
        bool to_use_;
    };

    // To be able to use find_if on std::list< boost::shared_ptr<CutStuff> > in conjunction with active values
    struct CutStuffComparison: public std::binary_function< boost::shared_ptr<CutStuff>, int, bool > {
        bool operator () ( const boost::shared_ptr<CutStuff> &cut_stuff, const int &number ) const {
            return cut_stuff->cut_creation_iteration_ == number && cut_stuff->active_value < CutStuff::active_threshold_ && cut_stuff->not_active_iterations_ > CutStuff::strike_threshold_;
        }
    };
    

    bool is_active (const boost::shared_ptr<CutStuff>& value);

    struct CutInformation
    {
        CutInformation();

        int cut_creation_iteration_;
        int active_iteration_counter_;
        int inactive_iteration_counter_;
        std::map<int,double> iteration_forward_active_vector_;
        std::map<int,double> iteration_backward_active_vector_;
    };

    class CutManager
    {
    public:
        CutManager(BendersNodeData&);
        ~CutManager();

        void CreateCut(int iteration); // Initialize a new cut with an index
        CutInformation& GetCut(int index); // Return a CutInformation struct
        void DeleteCut(int index);
        int NumCuts();
        //std::vector<OsiRowCut*>& OptimalityCuts();

    private:
        BendersNodeData& node_data_ref_;
        std::vector<CutInformation> cut_informations_; // 
        int cut_counter_;
    };


// We can make it a struct anyway?!
struct BendersNodeData 
{
    friend class CutManager;
public:
    BendersNodeData();
    BendersNodeData(SmiNodeData* nodeData);
    ~BendersNodeData();

    void set_col_solution(const double* colSolution);
    const double* col_solution();
    void set_dual_solution(const double* dualSolution);
    const double* dual_solution();
    void set_warm_start(CoinWarmStart* coinWarmStart);
    const CoinWarmStart* warm_start();

    void CleanUnusedMemory(bool direction);

    // Information about the solution
    double* col_solution_;
    double* dual_solution_;
    double* reduced_cost_; // Only needed if we have variable bounds! Reduced costs = dual logicals
    double* next_stage_approximation_;
    double* discrepancy_; // Store discrepancy of this approximation to the actual achieved value
    double min_discrepancy_;
    double max_discrepancy_;
    double average_discrepancy_;
    double dual_value_variable_bounds_;
    bool feasible_;
    

    int cols_; 
    int rows_;
    int num_thetas_; // How many aggregates do we have at this node
    boost::dynamic_bitset<> active_aggregates_; // Has to have the size of num_thetas_
    CoinWarmStart* warm_start_;
    bool warm_start_added_;

    SmiNodeData* smi_node_data_;
    int scenario_index_;
    int branch_scenario_;
    int num_node_in_stage_;
    int child_node_start_;
    double probability_; // Overal probability
    double conditional_probability_; // Probability with respect to the parent node

    double solution_value_;
    // Store optimality and feasibility cuts. We can also use a deque for that, if we want to remove elements later on..
    CutManager cut_information_;
    typedef std::list<boost::shared_ptr<CutStuff> > cut_container;
    cut_container optimality_cuts_;
    cut_container feasibility_cuts_;
    cut_container::iterator last_iteration_start_optimality_;
    cut_container::iterator last_iteration_start_feasibility_;
    
    // Multicut Aggregation Strategies..
    int num_scen_pro_cut_;
    // We should store a vector of bitsets, needs less memory // TODO
    std::vector< std::vector<int> > nodes_in_cut_; // Stores which subnodes of current node are aggregated to a cut.

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
    std::vector<OsiRowCut*> variable_bounds_;

    // In the non-staircase case
    std::vector< std::vector<std::pair<int,double> > > no_staircase_constraints_;
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
    CoinWarmStart* warm_start_;

	std::map<boost::thread::id,OsiSolverInterface*> osi_solver_map_; // Store solver interfaces for every stage 

    // In the non-staircase case
    std::vector< std::vector<std::pair<int,double> > > no_staircase_constraints_;

    // Store if we changed aggregation level
    bool changed_aggregation_level_;

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

} // end namespace parallel_benders
#endif // BendersNodeData_h__
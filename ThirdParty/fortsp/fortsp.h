/**
 * @file   fortsp.h
 * @brief  FortSP library interface
 * @author Victor Zverovich (victor@zverovich.net)
 */

#ifndef FSP_FORTSP_H
#define FSP_FORTSP_H

#include <float.h>
#include <limits.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @cond Internal */
#ifndef FSPAPI
# ifdef WIN32
#  define FSPAPI __declspec(dllimport)
# else
#  define FSPAPI
# endif
#endif
/** @endcond */

struct FSPSolver;
struct FSPProblem;
struct FSPNode;

#ifndef __cplusplus
/** A FortSP solver instance */
typedef struct FSPSolver FSPSolver;
/** An optimization problem */
typedef struct FSPProblem FSPProblem;
/** A scenario tree node */
typedef struct FSPNode FSPNode;
#endif

/** Solution algorithms */
enum {
    FSPAlgAuto        =  0, /**< The algorithm is selected automatically */

    // LP solution algorithms
    FSPAlgPrimal      =  1, /**< The primal Simplex method */
    FSPAlgDual        =  2, /**< The dual Simplex method */
    FSPAlgIPM         =  3, /**< An interior-point method (IPM) */

    // SP solution algorithms
    FSPAlgDetEq       = 10, /**< Solve the deterministic equivalent problem */
    FSPAlgDetEqX      = 11, /**< Solve the deterministic equivalent problem
                                 with explicit nonanticipativity constraints */
    FSPAlgBenders     = 12, /**< Benders' decomposition */
    FSPAlgLevel       = 13, /**< Benders' decomposition with regularization
                                 by the level method */
    FSPAlgLinDamp     = 14, /**< Benders' decomposition with regularization
                                 by linear damping */
    FSPAlgSD          = 15, /**< Stochastic decomposition */
    FSPAlgVNDS        = 16, /**< Variable neighborhood decomposition search */
    FSPAlgDBB         = 17, /**< Decomposition based branch and bound */
    FSPAlgIntLShaped  = 18, /**< Integer L-shaped method */
    FSPAlgTrustRegion = 19, /**< Trust region method based on the infinity
                                 norm */
    FSPAlgRD          = 20,  /**< Regularized decomposition */
    FSPAlgGRASP       = 21 /**< GRASP */
};

/** The value representing an infinite bound */
#define FSP_INFINITY (2 * HUGE_VAL)

/**
 * Creates a solver instance. Upon successful completion, @c FSPCreateSolver()
 * returns the pointer to the solver instance.
 * @param error The location where to store the error message. can be null.
 *              Use FSPFree() to deallocate the error message.
 * @return pointer to the solver instance if succeeded, null otherwise
 */
FSPAPI FSPSolver* FSPCreateSolver(const char** error);

/**
 * Destroys the specified solver instance.
 * @param s the solver instance to destroy
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPDestroySolver(FSPSolver* s);

/**
 * Deallocates memory.
 * @param p pointer to a memory block to be deallocated
 */
FSPAPI void FSPFree(const void* p);

/**
 * Gets the last error message.
 * @param s the solver instance
 * @return The pointer to the last error message if available, null otherwise.
 *         The pointer is only guaranteed to remain valid until the next call
 *         to any of the FortSP API functions.
 */
FSPAPI const char* FSPGetLastError(const FSPSolver* s);

/**
 * Gets the value of an integer option.
 * @param s the solver instance
 * @param name the option name
 * @param value the location where to store the option value
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetIntOption(const FSPSolver* s, const char* name, int* value);

/**
 * Sets the value of an integer option.
 * @param s the solver instance
 * @param name the option name
 * @param value the option value
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetIntOption(FSPSolver* s, const char* name, int value);

/**
 * Gets the value of a double option.
 * @param s the solver instance
 * @param name the option name
 * @param value the location where to store the option value
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetDblOption(const FSPSolver* s, const char* name, double* value);

/**
 * Sets the value of a double option.
 * @param s the solver instance
 * @param name the option name
 * @param value the option value
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetDblOption(FSPSolver* s, const char* name, double value);

/**
 * Gets the value of a string option. The pointer stored in @c value
 * is only guaranteed to remain valid until the next call to any of the
 * FortSP API functions.
 * @param s the solver instance
 * @param name the option name
 * @param value the location where to store the option value
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetStrOption(const FSPSolver* s, const char* name,
                           const char** value);

/**
 * Sets the value of a string option.
 * @param s the solver instance
 * @param name the option name
 * @param value the option value
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetStrOption(FSPSolver* s, const char* name, const char* value);

/**
 * Resets the solver options to their default values.
 * @param s the solver instance
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPResetOptions(FSPSolver* s);

/**
 * Creates an optimization problem instance.
 * @param s the solver instance
 * @param numStages the number of stages
 * @return pointer to the problem instance if succeeded, null otherwise
 */
FSPAPI FSPProblem* FSPCreateProblem(FSPSolver* s, unsigned numStages);

/**
 * Destroys the specified problem instance.
 * @param p the problem instance
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPDestroyProblem(FSPProblem* p);

/**
 * Adds variables to the specified stage of the problem.
 * @param p the problem instance
 * @param stage stage index
 * @param numVars the number of variables to add
 * @param colStarts
 *   pointer to the array of size <tt>numVars + 1</tt> containing start offsets
 *   of the columns in the @c elements array and <tt>colStarts[numVars]</tt>
 *   containing the number of elements. Can be null in which case @c conIndices
 *   and @c elements are ignored.
 * @param conIndices
 *   pointer to the array of size <tt>colStarts[numVars]</tt> containing
 *   constraint indices. Can be null if @c colStarts is null.
 * @param elements
 *   pointer to the array of size <tt>colStarts[numVars]</tt> containing
 *   constraint matrix elements. Can be null if @c colStarts is null.
 * @param lb
 *   pointer to the array of size <tt>numVars</tt> containing lower bounds.
 *   Can be null in which case the lower bounds are set to zero.
 * @param ub
 *   pointer to the array of size <tt>numVars</tt> containing upper bounds.
 *   Can be null in which case the upper bounds are set to @c FSP_INFINITY.
 * @param obj
 *   pointer to the array of size <tt>numVars</tt> containing objective
 *   coefficients. Can be null in which case the coefficients are set to zero.
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPAddVars(FSPProblem* p, unsigned stage,
                      unsigned numVars, const unsigned* colStarts,
                      const unsigned* conIndices, const double* elements,
                      const double* lb, const double* ub, const double* obj);

/**
 * Adds constraints to the specified stage of the problem.
 * @param p the problem instance
 * @param stage stage index
 * @param numCons the number of constraints to add
 * @param blockRowStarts
 *   pointer to the array of size <tt>(stage + 1) * numCons + 1</tt> containing
 *   start offsets of the rows in the @c elements divided into blocks in the
 *   lower block diagonal form of the constraint matrix of the core problem.
 *   <tt>blockRowStarts[(stage + 1) * numCons]</tt> should contain the total
 *   number of elements in all blocks. Can be null in which case @c varIndices
 *   and @c elements are ignored.
 * @param varIndices
 *   pointer to the array of size <tt>blockRowStarts[(stage + 1) * numCons]</tt>
 *   containing variable indices. Can be null if @c blockRowStarts is null.
 * @param elements
 *   pointer to the array of size <tt>blockRowStarts[(stage + 1) * numCons]</tt>
 *   containing constraint matrix elements. Can be null if @c blockRowStarts
 *   is null.
 * @param lb
 *   pointer to the array of size <tt>numCons</tt> containing lower bounds.
 *   Can be null in which case the lower bounds are set to
 *   <tt>-FSP_INFINITY</tt>.
 * @param ub
 *   pointer to the array of size <tt>numCons</tt> containing upper bounds.
 *   Can be null in which case the upper bounds are set to @c FSP_INFINITY.
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPAddCons(FSPProblem* p, unsigned stage,
                      unsigned numCons, const unsigned* blockRowStarts,
                      const unsigned* varIndices, const double* elements,
                      const double* lb, const double* ub);

/**
 * Gets the objective coefficients.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of coefficients to get
 * @param coefs
 *   pointer to the array of size @c length or more where to store the
 *   coefficients
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetObjCoefs(const FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, double* coefs);

/**
 * Sets the objective coefficients.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of coefficients to set
 * @param coefs the new objective coefficients
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetObjCoefs(FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, const double* coefs);

/**
 * Gets the lower bounds on variables.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of bounds to get
 * @param lb
 *   pointer to the array of size @c length or more where to store the bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetVarLower(const FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, double* lb);

/**
 * Sets the lower bounds on variables.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of bounds to set
 * @param lb the new lower bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetVarLower(FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, const double* lb);

/**
 * Gest the upper bounds on variables.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of bounds to get
 * @param ub
 *   pointer to the array of size @c length or more where to store the bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetVarUpper(const FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, double* ub);

/**
 * Sets the upper bounds on variables.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of bounds to set
 * @param ub the new upper bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetVarUpper(FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, const double* ub);

/**
 * Gets the lower bounds on constraints.
 * @param p the problem instance
 * @param stage stage index
 * @param start start constraint index
 * @param length the number of bounds to get
 * @param lb
 *   pointer to the array of size @c length or more where to store the bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetConLower(const FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, double* lb);

/**
 * Sets the lower bounds on constraints.
 * @param p the problem instance
 * @param stage stage index
 * @param start start constraint index
 * @param length the number of bounds to set
 * @param lb the new lower bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetConLower(FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, const double* lb);

/**
 * Gets the upper bounds on constraints.
 * @param p the problem instance
 * @param stage stage index
 * @param start start constraint index
 * @param length the number of bounds to get
 * @param ub
 *   pointer to the array of size @c length or more where to store the bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetConUpper(const FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, double* ub);

/**
 * Sets the upper bounds on constraints.
 * @param p the problem instance
 * @param stage stage index
 * @param start start constraint index
 * @param length the number of bounds to set
 * @param ub the new upper bounds
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetConUpper(FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, const double* ub);

/**
 * A variable type.
 */
typedef enum {
    FSPContinuous, /**< Continuous variable */
    FSPBinary,     /**< Binary variable */
    FSPInteger     /**< General integer variable */
} FSPVarType;

/**
 * Gets the variable types.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of variables to get
 * @param types
  *   pointer to the array of size @c length or more where to store the types
* @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetVarTypes(const FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, FSPVarType* types);

/**
 * Sets the variable types.
 * @param p the problem instance
 * @param stage stage index
 * @param start start variable index
 * @param length the number of variables to set
 * @param types the new variable types
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetVarTypes(FSPProblem* p, unsigned stage, unsigned start,
                          unsigned length, const FSPVarType* types);


/**
 * A kind of FortSP problem element.
 * @return zero if succeeded, nonzero otherwise
 */
typedef enum {
    FSPObjCoef  = 0, /**< Objective coefficient */
    FSPVarLower = 1, /**< Variable lower bound */
    FSPVarUpper = 2, /**< Variable upper bound */
    FSPVarFixed = 6, /**< Fixed variable value */
    FSPConLower = 3, /**< Constraint lower bound */
    FSPConUpper = 4, /**< Constraint upper bound */
    FSPConFixed = 7, /**< Fixed constraint value */
    FSPConCoef  = 5  /**< Constraint coefficient */
} FSPElementKind;

/**
 * Data associated with a scenario tree node representing a change in a single
 * problem element such as a constraint matrix element or a variable bound.
 * The change relative is relative to the core problem.
 */
typedef struct {
    /** Element kind */
    FSPElementKind kind;

    /** Variable stage of the element */
    unsigned varStage;

    /**
     * Element index in the core problem.
     * If #kind is @c FSPObjCoef, @c FSPVarLower, @c FSPVarUpper or
     * @c FSPVarFixed this is a variable index. If #kind is @c FSPConLower,
     * @c FSPConUpper or @c FSPConFixed this is a constraint index.
     * If #kind is @c FSPConCoef this is a constraint matrix element index.
     */
    unsigned index;

    /** Element value */
    double value;
} FSPNodeData;

/**
 * Adds nodes to the scenario tree of the problem.
 * @param p the problem instance
 * @param parent the parent node
 * @param numNodes the number of nodes to add
 * @param probabilities unconditional probabilities of the nodes
 * @param dataStarts
 *   array of size @c numNodes containing start offsets in @c data of
 *   each node's data
 * @param data the nodes' data
 * @param nodes
 *   array containing locations where to store the pointers to the created nodes
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPAddNodes(FSPProblem* p, FSPNode* parent,
                       unsigned numNodes, const double* probabilities,
                       const unsigned* dataStarts, const FSPNodeData* data,
                       FSPNode** nodes);

/**
 * Gets the child node of the specified scenario tree node.
 * @param p the problem instance
 * @param n the parent node
 * @param index child index
 * @return the pointer to the child node if succeeded, null otherwise
 */
FSPAPI FSPNode* FSPGetChild(FSPProblem* p, FSPNode* n, unsigned index);

/**
 * Adds data to the specified scenario tree node.
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPAddData(FSPProblem *p, FSPNode* n, unsigned length,
                      const FSPNodeData* data);

/**
 * Returns the number of stages in the problem.
 * @param p the problem instance
 */
FSPAPI unsigned FSPGetNumStages(const FSPProblem* p);

/**
 * Returns the number of constraints at the specified stage in the
 * core problem.
 * @param p the problem instance
 * @param stage stage index
 */
FSPAPI unsigned FSPGetNumCons(const FSPProblem* p, unsigned stage);

/**
 * Returns the number of variables at the specified stage in the core problem.
 * @param p the problem instance
 * @param stage stage index
 */
FSPAPI unsigned FSPGetNumVars(const FSPProblem* p, unsigned stage);

/**
 * Returns the number of constraint matrix elements at the specified stage in
 * the core problem.
 * @param p the problem instance
 * @param stage stage index
 */
FSPAPI unsigned FSPGetNumElements(const FSPProblem* p, unsigned stage);

/** An objective sense (optimization direction) */
typedef enum {
    FSPMinimize =  1, /**< Minimization objective */
    FSPMaximize = -1  /**< Maximization objective */
} FSPObjSense;

/**
 * Returns the objective sense.
 * @param p the problem instance
 * @return the objective sense
 */
FSPAPI FSPObjSense FSPGetObjSense(const FSPProblem* p);

/**
 * Sets the objective sense.
 * @param p the problem instance
 * @param s the new objective sense
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetObjSense(FSPProblem* p, FSPObjSense s);

/**
 * Returns the value of the constant term in the objective.
 * @param p the problem instance
 */
FSPAPI double FSPGetObjOffset(const FSPProblem* p);

/**
 * Sets the value of the constant term in the objective.
 * @param p the problem instance
 * @param offset the new objective offset
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSetObjOffset(FSPProblem* p, double offset);

/**
 * Reads a problem in the SMPS format.
 * @param s the solver instance
 * @param coreFilename the name of a core file
 * @param stochFilename the name of a stoch file
 * @param timeFilename the name of a time file
 * @return pointer to the problem instance if succeeded, null otherwise
 */
FSPAPI FSPProblem* FSPReadSMPS(FSPSolver* s,
                               const char* coreFilename,
                               const char* stochFilename,
                               const char* timeFilename);

/**
 * Solves the problem.
 * @param p the problem to solve
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPSolve(FSPProblem* p);

/** A solution status */
typedef enum {
    FSPUnknown,               /**< Unknown status */
    FSPOptimal,               /**< Optimal solution found */
    FSPInfeasible,            /**< The problem is infeasible */
    FSPUnbounded,             /**< The problem is unbounded */
    FSPInfeasibleOrUnbounded, /**< The problem is infeasible or unbounded */
    FSPTimeLimit,             /**< Time limit reached */
    FSPSolutionLimit,         /**< Solution limit reached */
    FSPBestNumerical          /**< Best solution not proven to be optimal due
                                   to numerical difficulties */
} FSPStatus;

/**
 * Returns the current solution status.
 * @param p the problem instance
 * @return the current solution status
 */
FSPAPI FSPStatus FSPGetStatus(const FSPProblem* p);

/**
 * Returns the current objective value.
 * @param p the problem instance
 * @return the current objective value
 */
FSPAPI double FSPGetObjValue(const FSPProblem* p);

/**
 * Gets the solution for stage 0.
 * @param p the problem instance
 * @param values
 *   Pointer to the array where to store the variable values or null.
 *   If not null, the array size should be greater or equal to the number
 *   of variables in the node's stage
 * @param rcs
 *   Pointer to the array where to store the reduced costs or null.
 *   If not null, the array size should be greater or equal to the number
 *   of variables in the node's stage.
 * @param duals
 *   Pointer to the array where to store the values of the dual variables or
 *   null. If not null, the array size should be greater or equal to the number
 *   of constraints in the node's stage.
 * @param slacks
 *   Pointer to the array where to store the slacks or null.
 *   If not null, the array size should be greater or equal to the number
 *   of constraints in the node's stage.
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPGetStage0Solution(const FSPProblem* p,
                                double* values, double* rcs,
                                double* duals, double* slacks);

/**
 * Returns the Expected Value of Perfect Information (EVPI).
 * @param p the problem instance
 * @return the Expected Value of Perfect Information if succeeded, -1 otherwise
 */
FSPAPI double FSPGetEVPI(const FSPProblem* p);

/**
 * Returns the Value of Stochastic Solution (VSS).
 * @param p the problem instance
 * @return the Value of Stochastic Solution if succeeded, -1 otherwise
 */
FSPAPI double FSPGetVSS(const FSPProblem* p);

/**
 * The main entry point of the FortSP program.
 * @param argc the number of command-line arguments
 * @param argv command-line arguments
 * @return zero if succeeded, nonzero otherwise
 */
FSPAPI int FSPMain(int argc, char* argv[]);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FSP_FORTSP_H

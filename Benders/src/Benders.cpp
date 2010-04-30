#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif

#include <vld.h>
#include "SmiDiscreteDistribution.hpp"
#include "SmiScenarioTree.hpp"
#include "SmiSmpsIO.hpp"
#include "OsiSolverInterface.hpp"
#include "CoinHelperFunctions.hpp"
#include "CoinError.hpp"
#include "CoinPackedVector.hpp"
#include "SmiScnModel.hpp"
#include "CoinPackedMatrix.hpp"
#include "SmiScnData.hpp"
#include "OsiClpSolverInterface.hpp"

#include "OsiCbcSolverInterface.hpp"
#include "Benders.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <algorithm>
#include <ctime>
using namespace std;


//int main(int argc,  char* argv[]){	
//	// ###################################################################################
//	// Begin: main
//	// ###################################################################################
//   
//	// Input parameters:
//	// #1: Maximal Iterations
//	// #2: Single/multi: s m
//	// #3: Warmstart: +wsI +wsS -ws
//	// #4: Knapsack Inequalities: +ki -ki
//	// #5: TrustRegion: +tr -tr
//	// #6: Benders or detEq: +ben -ben
//	// #7: Problem Path
//	// #8: Solution Path
//
//	// Check if input is correct
//	if(argc!=9){
//		cout << "Number of parameters is not correct! Please review input!" << endl;
//		system("pause");
//		return 0;
//	}
//	if(strcmp(argv[2],"s") && strcmp(argv[2],"m")){
//		cout << "Second parameter is not correct! Please review input!" << endl;
//		system("pause");
//		return 0;
//	}
//	if(strcmp(argv[3],"-ws") && strcmp(argv[3],"+wsI") && strcmp(argv[3],"+wsS")
//		&& strcmp(argv[3],"+wsi") && strcmp(argv[3],"+wss")){
//		cout << "Third parameter is not correct! Please review input!" << endl;
//		system("pause");
//		return 0;
//	}
//	if(strcmp(argv[4],"+ki") && strcmp(argv[4],"-ki")){
//		cout << "Fourth parameter is not correct! Please review input!" << endl;
//		system("pause");
//		return 0;
//	}
//	if(strcmp(argv[5],"+tr") && strcmp(argv[5],"-tr")){
//		cout << "Sixth parameter is not correct! Please review input!" << endl;
//		system("pause");
//		return 0;
//	}
//	if(strcmp(argv[6],"+ben") && strcmp(argv[6],"-ben")){
//		cout << "Fifth parameter is not correct! Please review input!" << endl;
//		system("pause");
//		return 0;
//	}
//
//	// If input is correct, start solving the problem
//
//	// Start time measurement
//	clock_t begin=clock();
//	// Create a new Benders object, input: use single or multi cut, 
//	// use warmstart or not, use knapsack inequalities or not, use
//	// trust region or not, solve via Benders or via detEq
//	Benders* benders = new Benders(argv[2], argv[3], argv[4], argv[5], argv[6]);
//	// Initialize maximal iteration counter
//	int maxIt = atoi(argv[1]);
//	// Path of problem
//	const char * path = argv[7];
//	// Path of solution file
//	const char * solpath = argv[8];
//	// Read SMPS file
//	if(benders->readSMPS(path)==0){
//		if(benders->benders_){
//			// Solve problem with benders decomposition method
//			// Input: tolerance epsilon, maximal number of iterations
//			benders->BendersSolve(0.000001, maxIt );		
//		}
//		if(!benders->benders_){
//			// Solve problem with the deterministic equivalent
//			if(benders->detEq()==-1){
//				// End time measurement
//				clock_t end=clock();
//				cout << "Problem is infeasible" << endl;
//				// Print elapsed time 
//				cout << "Elapsed Time: " << (end-begin)/1000.0 << " sec" << endl;
//				// Delete benders object
//				delete benders;
//				return 0;
//			}		
//		}
//	}
//	// End time measurement
//	clock_t end=clock();
//	double time = (end-begin)/1000.0;
//	// Print elapsed time
//	cout << "Elapsed Time: " << time << " sec" << endl;
//	// Write solution file
//	if(benders->benders_){
//		benders->writeSolutionFileBenders(solpath, time);
//	}
//	if(!benders->benders_){
//		benders->writeSolutionFileDetEq(solpath, time);
//	}
//	// Delete benders object
//	delete benders;
//	// Pause before close program
//	system("pause");
//	return 0;
//
//
//	// ###################################################################################
//	// End: main
//	// ###################################################################################
//}

void Benders::BendersSolve(double eps, int maxIter){
    // ###################################################################################
    // Begin: BendersSovle
    // ###################################################################################

    // Solve a two stage stochastic linear program with the benders
    // decomposition method

    // Initialize upper bound
    UB_ = infty_;
    // Initialize lower bound
    LB_ = -infty_;
    // Initialize best lower bound found yet (used for Trust Region)
    double LBmax = -infty_;
    // Counter for optimality cuts: counts how many optimality cuts are 
    // created during the current iteration; is needed, if there is a
    // scenario in which no cut is derived
    int oCut = 1;
    // Bool for feasibility cut; true, if a feasibility cut is created
    bool fCut = true;
    // Iteration counter
    nIter_ = 0;
    // Counter for feasibility cuts
    int nFeas = 0;
    // While the gap between upper and lower bound is larger than epsilon
    // repeat the benders decomposition method. Also continue if maximal 
    // number of iterations isn't reached.
    while(abs(UB_-LB_)>eps  && maxIter > nIter_){ 
        // At the beginning of each iteration set counter of optimality
        // cuts to zero
        oCut = 0;
        // Solve master problem
        if(solveMaster() == -1){
            return;		
        }
        // Declaration of a temporary solution vector
        double *bestSol_Temp = new double[mncol_+zSub_+sncol_*nScen_];
        // Read current stage 1 solution 
        const double* mSol = osiMaster_->getColSolution();
        // Write current stage 1 solution to temporary solution vector; it 
        // also contains the value(s) of the variables for the scenario(s);
        // there is a check if the values are close to zero, if this is the
        // case the values are tranferred to zero
        for(int i=0; i<mncol_+zSub_; i++){
            double v=mSol[i];
            if(v < 0){
                v = -v;
            }
            if(v < ZEROTEST){
                bestSol_Temp[i] = 0;
            }
            else{
                bestSol_Temp[i] = mSol[i];
            }
        }

        // Update lower bound, but not in the first iteration and only if
        // there are no feasibility cuts added in the previous iterations,
        // otherwise there could occur problems with the gap between lower 
        // and upper bound; if there are bounds on stage 2 variables, there 
        // could occur an offset of the objective function which has to
        // be manually added to the objective function
        if(nIter_!=nFeas && fCut ==false){
            if(trustRegion_ ){
                // Copy lower bound, before it is updated
                LBmax = LB_;
            }
            LB_ = osiMaster_->getObjValue()+objTrans_;		
        }	

        // Initialize temporary upper bound with zero
        upperBound_ = 0;
        // Begin calculating temporary upper bound: objective function value
        // of master problem
        for(int i=0; i<mncol_; i++){
            upperBound_+= mdobj_[i]*bestSol_Temp[i];
        }
        // Cycle over all scenarios
        for(int scen_=0; scen_<nScen_; scen_++){
            // Build subproblem, combine deterministic data with stochastic
            // data; Input: number of current scenario
            buildSubproblem(scen_);
            // Add current stage 1 solution to subproblem, i.e. update RHS of
            // subproblem; Input: number of current scenario, current stage 1 
            // solution 
            addSt1ToSubproblem(scen_, bestSol_Temp);
            // Solve subproblem; Input: number of current
            // scenario, number of feasibility cuts
            if(solveSubproblem(scen_, fCut) == -1){ 
                delete [] bestSol_Temp;
                return;
            }	
            // Initialize bool of feasibility cuts with false
            fCut = false;
            // If dual subproblem is unbounded, add a feasibility cut to master
            // problem
            if(osiSubproblem_->isProvenPrimalInfeasible()==1){
                addFeasCut(scen_);
                fCut = true;
                nFeas++;
                break;
            }
            // Copy vector of current solution of subproblem
            const double* sSol_Temp = osiSubproblem_->getColSolution();
            // Declaration of a temporary solution vector of stage 2
            double* sSol = new double[sncol_];
            // Copy the current stage 2 solution to temporary solution vector
            memcpy(sSol, sSol_Temp, sncol_*sizeof(double));
            // If there are bounds on stage 2 variables, the translation which
            // took place in buildSubproblem has to be revoked
            for(int i=0; i<lTrans_; i++){
                sSol[iTrans_[i]]=sSol_Temp[iTrans_[i]]+dTrans_[i];
            }
            // There is a check if the values are close to zero, if this is the
            // case the values are transferred to zero; afterwards the new 
            // objective value is calculated, therefore a new variable is
            // declared and initialized by zero 
            double subObjVal = 0;
            const double *subObjCoeff = osiSubproblem_->getObjCoefficients();
            for(int i=0; i<sncol_; i++){
                double v = sSol[i];
                if(v < 0){
                    v = -v;
                }
                if(v < ZEROTEST){
                    bestSol_Temp[mncol_+zSub_+sncol_*scen_+i] = 0;
                }
                else{
                    bestSol_Temp[mncol_+zSub_+sncol_*scen_+i] = sSol[i];
                }
                subObjVal+=bestSol_Temp[mncol_+zSub_+sncol_*scen_+i]*subObjCoeff[i];
            }	
            // Delete temporary solution vector of stage 2
            delete [] sSol;
            // If single cut is chosen, there will definitely calculated an
            // optimality cut; if multi cut is chosen, the current scenario
            // generates only an optimality cut if p(d-Bx)>z holds; if in the previous
            // iterations only feasibility cuts are added, there will definitely
            // calculated an optimality cut.

            // In the first iteration the variables for the scenarios have 
            // the value -infinity, therefor the first cut(s) is(are) valid
            if(single_ || subObjVal > bestSol_Temp[mncol_+scen_] || nIter_==nFeas ){
                // Calculate an optimality cut; Input: number of optimality cuts
                // which were calculate during the current iteration, number of
                // current scenario;
                calcOptiCut(oCut, scen_);
                oCut++;
            }
            // Update upper bound: add all objective values to upper bound 
            // multiplied with the associated probability; if there are bounds 
            // on stage 2 variables, there could occur an offset of the objective 
            // function which has to be manually added to the objective function
            upperBound_+= subObjVal*scenProb_[scen_];
        }
        // If current upper bound is less or equal than the old upper bound, update
        // the upper bound and the best feasible solution; the update takes plays 
        // only if there is no feasibility cut
        if(!fCut && upperBound_<=UB_){
            // Update the best feasible solution
            memcpy(bestSol_, bestSol_Temp, 
                (mncol_+zSub_+sncol_*nScen_)*sizeof(double) );
            // Update the upper bound
            UB_ = upperBound_;
        }

        // After solving all subproblems one or more optimality cuts will be}
        // added to the master problem; of course this will only take
        // place if there is any optimality cut and none feasibility cut; if
        // knapsack inequalities should be added, this method will derive one
        if(!fCut && oCut>0){
            addOptiCut(oCut);
        }
        // Apply Trust Region method
        if(trustRegion_ ){
            // Apply trust region method as long as the method returns 1,
            // if it returns 0, change value of the boolean
            if(trustRegion(bestSol_Temp, LBmax, oCut, fCut)==0){
                trustRegion_ = false;
            }	
        }
        // Delete temporary solution vector
        delete [] bestSol_Temp;
        // Count up iteration counter
        nIter_++;

        /*
        // ### Print master matrix and RHS ###	
        const CoinPackedMatrix * mat=osiMaster_->getMatrixByRow();
        const double* elem=mat->getElements();
        const int* ind=mat->getIndices();
        for(int i=0; i<mat->getNumElements() ; i++){
        cout << "Cut-Coeff for Var "  << ind[i] << " is :" << elem[i] << endl;
        }
        const double* rhs=osiMaster_->getRightHandSide();
        for(int i=0; i<osiMaster_->getNumRows(); i++){
        cout << "RHS master :" << rhs[i] << endl;
        }*/		

        // ### Print upper bound and lower bound ###
        cout << "UB " << UB_ << endl;
        cout << "LB_ " << LB_ << endl;	
        cout << "UB-LB_ " << UB_-LB_ << endl;
        cout << "Iteration: " << nIter_ << endl;
    }	
    cout << "Objective value after "<< nIter_ << " iterations is: "  << LB_ << endl;


    // ###################################################################################
    // End: BendersSolve
    // ###################################################################################
}

int Benders::readSMPS(const char* path){
    // ###################################################################################
    // Begin: readSMPS
    // ###################################################################################

    // Read SMPS files

    // Create a new SmiScnModel
    SmiScnModel *smi = new SmiScnModel();
    // Read SMPS files
    if(smi->readSmps(path) == -1){
        cout << "Error while reading SMPS files!" << endl;
        return -1;
    }
    // Copy data of master problem
    copyMaster(smi);
    // Copy data of subproblems
    copySubmodel(smi);
    // Delete SmiScnModel
    delete smi;
    return 0;

    // ###################################################################################
    // End: readSMPS
    // ###################################################################################
}

void Benders::copyMaster(SmiScnModel * smi){
    // ###################################################################################
    // Begin: copyMaster
    // ###################################################################################

    // Copy data of the master problem from SMPS files

    // Number of rows 
    mnrow_ = smi->core_->getNumRows(0);
    // Number of columns
    mncol_ = smi->core_->getNumCols(0);
    // Number of scenarios
    nScen_ = smi->getNumScenarios();
    // If single cut is chosen the counter zSub is set to 1; if 
    // multi cut is chosen it will be initialized by number of scenarios
    if(single_){
        zSub_ = 1;	
    }
    else{
        zSub_ = nScen_;	
    }

    // Declare an array for the coefficients of the objective function; 
    // allocate space for the additional variable(s) for the subproblems 
    // which will have the coefficient zero for the first iteration; after
    // solving the master problem for the first time the coefficient(s) are 
    // set to 1 or to the associated probability, respectively
    mdobj_ = new double[mncol_+zSub_];
    for(int scen_=mncol_; scen_< mncol_+zSub_; scen_++){
        mdobj_[scen_] = 0;
    }
    // Coefficients of objective function
    memcpy(mdobj_, smi->core_->getNode(0)->getObjectiveElements(), sizeof(double)*mncol_);
    // Coefficients of LHS
    mdrlo_ = new double[mnrow_];
    memcpy(mdrlo_, smi->core_->getNode(0)->getRowLowerElements(), sizeof(double)*mnrow_);
    // Coefficients of RHS
    mdrup_ = new double[mnrow_];
    memcpy(mdrup_, smi->core_->getNode(0)->getRowUpperElements(), sizeof(double)*mnrow_);
    // Coefficients of lower bounds
    mdclo_ = new double[mncol_+zSub_];
    memcpy(mdclo_, smi->core_->getNode(0)->getColLowerElements(), sizeof(double)*mncol_);
    // Coefficients for the additional variables are set to minus infinity
    for(int scen_=mncol_; scen_< mncol_+zSub_; scen_++){
        mdclo_[scen_] = -infty_;
    }
    // Coefficients of upper bounds
    mdcup_ = new double[mncol_+zSub_];
    memcpy(mdcup_, smi->core_->getNode(0)->getColUpperElements(), sizeof(double)*mncol_);
    // Coefficients for the additional variables are set to plus infinity
    for(int scen_=mncol_; scen_< mncol_+zSub_; scen_++){
        mdcup_[scen_] = infty_;
    }

    // Coefficients of matrix
    // Number of non-zeros of matrix 
    mnels_ = smi->core_->getNode(0)->getNumMatrixElements();
    // Coefficients of matrix
    mdels_ = new double[mnels_];
    // Indices of the matrix elements
    miels_ = new int[mnels_];
    // Row start of matrix
    mrstrt_ = new int[mnrow_+1];
    mrstrt_[0] = 0;
    // Row length
    mlen_ = new int[mnrow_];
    // Copy matrix coefficients, indices, row start and row length
    for(int i=0; i<mnrow_; i++){
        // Length of row i
        mlen_[i] = smi->core_->getNode(0)->getRowLength(i);
        // Copy row start
        mrstrt_[i+1] = mrstrt_[i]+mlen_[i];
        // Copy matrix coefficients
        memcpy(mdels_+mrstrt_[i], smi->core_->getNode(0)->getRowElements(i), mlen_[i]*sizeof(double));
        // Copy indices of matrix elements
        memcpy(miels_+mrstrt_[i], smi->core_->getNode(0)->getRowIndices(i), mlen_[i]*sizeof(int));
    }
    // All optimality cuts are saved in one array for each iteration, therefore 
    // declare several arrays:
    // Coefficients of the cut elements
    cutElements_ = new double[(mncol_+1)*nScen_];
    // Indices of the cut elements
    cutIndex_ = new int[(mncol_+1)*nScen_];
    // Cut RHS
    cutRowUB_ = new double [nScen_];

    // Integer informations
    // Number of integer variables in both stage 1 and stage 2
    int nInt = smi->getIntegerLen();
    // Indices of integer variables
    int* iInt = new int[nInt];
    memcpy(iInt, smi->getIntegerInd(), nInt*sizeof(int));
    // Initialize number of stage 1 integers with zero
    mnInt_ = 0;
    // Initialize number of stage 2 integers with zero
    snInt_ = 0;
    // Count number of stage 1 integers
    while(mnInt_<nInt && iInt[mnInt_]<mncol_){
        mnInt_++;
    }
    // Calculate number of stage 2 integers
    snInt_ = nInt - mnInt_;
    // Indices of stage 1 integers
    miInt_ = new int[mnInt_];
    // Indices of stage 2 integers
    siInt_ = new int[snInt_];
    // Copy indices
    memcpy(miInt_, iInt, mnInt_*sizeof(int));
    memcpy(siInt_, iInt+mnInt_ , snInt_*sizeof(int));
    delete [] iInt;

    // Binary informations
    // Number of binaries
    int nBin = smi->getBinaryLen();
    // Declaration of an array for indices of
    // binary variables
    int* iBin = new int[nBin];
    // Copy indices of binaries
    memcpy(iBin, smi->getBinaryInd(), sizeof(int)*nBin);
    // Initialize number of stage 1 binaries with zero
    mnBin_ = 0;
    // Initialize number of stage 2 binaries with zero
    snBin_ = 0;
    // Count number of stage 1 integers
    while(mnBin_<nBin && iBin[mnBin_]<mncol_){
        mnBin_++;
    }
    // Calculate number of stage 2 binaries
    snBin_ = nBin - mnBin_;
    // Indices of stage 1 binaries
    miBin_ = new int[mnBin_];
    // Indices of stage 2 binaries
    siBin_ = new int[snBin_];
    // Copy indices
    memcpy(miBin_, iBin, mnBin_*sizeof(int));
    memcpy(siBin_, iBin+mnBin_ , snBin_*sizeof(int));
    delete [] iBin;


    // ###################################################################################
    // End: copyMaster
    // ###################################################################################
}

void Benders::copySubmodel(SmiScnModel* smi){
    // ###################################################################################
    // Begin: copySubmodel
    // ###################################################################################

    // Copy data of the subproblem from SMPS files

    // Number of rows
    snrow_ = smi->core_->getNumRows(1);
    // Number of columns
    sncol_ = smi->core_->getNumCols(1);
    // Declaration of an array for coefficients of objective function
    sdobj_ = new double[sncol_];
    // Copy coefficients of the objective function
    memcpy(sdobj_, smi->core_->getNode(1)->getObjectiveElements(), sizeof(double)*sncol_);
    // Declaration of an array for coefficients of LHS
    sdrlo_ = new double[snrow_];
    // Copy coefficients of LHS
    memcpy(sdrlo_, smi->core_->getNode(1)->getRowLowerElements(), sizeof(double)*snrow_);
    // Declaration of an array for coefficients of RHS
    sdrup_ = new double[snrow_];
    // Copy coefficients of RHS
    memcpy(sdrup_, smi->core_->getNode(1)->getRowUpperElements(), sizeof(double)*snrow_);
    // LHS and RHS are saved after they were combined with the deterministic
    // and the stochastic data. So the cut coefficients can be calculated
    // with this constant LHS and RHS.
    // Declaration of an array for coefficients of constant LHS
    constDrlo_ = new double[snrow_+sncol_];
    // Declaration of an array for coefficients of constant RHS
    constDrup_ = new double[snrow_+sncol_];
    // Declaration of an array for coefficients of lower bounds
    sdclo_ = new double[sncol_];
    // Copy coefficients of lower bounds
    memcpy(sdclo_, smi->core_->getNode(1)->getColLowerElements(), sizeof(double)*sncol_);
    // Declaration of an array for coefficients of upper bounds
    sdcup_ = new double[sncol_];
    // Copy coefficients of upper bounds
    memcpy(sdcup_, smi->core_->getNode(1)->getColUpperElements(), sizeof(double)*sncol_);

    // Number of non-zeros in matrix
    snels_ = smi->core_->getNode(1)->getNumMatrixElements();

    // This part is only needed for the deterministic equivalent
    if(benders_==false){
        // Declaration of an array for coefficients for
        // deterministic matrix
        sdels_ = new double[snels_];
        // Declaration of an array for indices
        siels_ = new int[snels_];
        // Declaration of a start array for derministic
        // matrix, initialized with zero
        srstrt_ = new int[snrow_+1];
        srstrt_[0] = 0;
        // Declaration of an array for row length
        slen_=new int[snrow_];

        // Cycle over all stage 2 rows
        for(int i=mnrow_; i<snrow_+mnrow_; i++){
            // Copy length
            slen_[i-mnrow_]=smi->core_->getNode(1)->getRowLength(i);
            // Copy coefficients
            memcpy(sdels_+srstrt_[i-mnrow_],smi->core_->getNode(1)->getRowElements(i), 
                sizeof(double)*slen_[i-mnrow_]);
            // Copy indices
            memcpy(siels_+srstrt_[i-mnrow_],smi->core_->getNode(1)->getRowIndices(i), 
                sizeof(int)*slen_[i-mnrow_]);
            // Copy start array
            srstrt_[i-mnrow_+1]=srstrt_[i-mnrow_]+slen_[i-mnrow_];		
        }
    }
    // End: deterministic equivalent part

    // Number of stage 1 coefficients per row
    snSt1len_ = new int[snrow_];
    // Number of stage 2 coefficients per row
    snSt2len_ = new int[snrow_];
    // Number of stage 1 coefficients is initialized by zero
    snSt1_ = 0;
    // Counting of stage 1 and stage 2 coefficients
    for(int i=mnrow_; i<snrow_+mnrow_; i++){ 
        // Indices per row i
        const int* siels_count = smi->core_->getNode(1)->getRowIndices(i);
        int rowLength = smi->core_->getNode(1)->getRowLength(i);
        // Counter for while loop
        int j = 0;
        while(j < rowLength){
            // If index is lower than number of stage 1 variables count up 
            // stage 1 coefficients
            if (siels_count[j] < mncol_){
                (snSt1_)++;
                j++;
            }
            // Else break
            else{
                break;
            }	
        }
        // Set number of stage 1 coefficients per row
        snSt1len_[i-mnrow_] = j;
        // Set number of stage 2 coefficients per row
        snSt2len_[i-mnrow_] = smi->core_->getNode(1)->getRowLength(i)-j;
    }
    // Calculate number of stage 2 coefficients
    snSt2_ = snels_-snSt1_;
    // Declaration of an array for stage 1 coefficients
    sSt1dels_ = new double[snSt1_];
    // Declaration of an array for stage 1 indices
    sSt1iels_ = new int[snSt1_];
    // Declaration of an array for row start of stage 1
    sSt1rstrt_ = new int[snrow_+1];
    // It is initialized by zero
    sSt1rstrt_[0] = 0;
    // Declaration of an array for stage 2 coefficients
    sSt2dels_ = new double[snSt2_];
    // Declaration of an array for stage 2 indices
    sSt2iels_ = new int[snSt2_];
    // Declaration of an array for row start of stage 2
    sSt2rstrt_ = new int[snrow_+1];
    // It is initialized by zero
    sSt2rstrt_[0] = 0;
    // Cycle over all stage 2 rows
    for(int i=mnrow_; i<snrow_+mnrow_; i++){
        // Set start array for stage 1
        sSt1rstrt_[i+1-mnrow_] = sSt1rstrt_[i-mnrow_]+snSt1len_[i-mnrow_];
        // Copy coefficients of stage 1
        memcpy(sSt1dels_+sSt1rstrt_[i-mnrow_], smi->core_->getNode(1)->getRowElements(i), 
            (snSt1len_[i-mnrow_])*sizeof(double));
        // Copy indices of stage 1 		
        memcpy(sSt1iels_+sSt1rstrt_[i-mnrow_], smi->core_->getNode(1)->getRowIndices(i), 
            (snSt1len_[i-mnrow_])*sizeof(int));	
        // Set start array for stage 2
        sSt2rstrt_[i+1-mnrow_] = sSt2rstrt_[i-mnrow_]+snSt2len_[i-mnrow_];
        // Copy coefficients of stage 2
        memcpy(sSt2dels_+sSt2rstrt_[i-mnrow_], 
            smi->core_->getNode(1)->getRowElements(i)+snSt1len_[i-mnrow_], 
            (snSt2len_[i-mnrow_])*sizeof(double));
        // To copy stage 2 indices there have to be a temporary array
        // in which the indices are copied 
        int* rowIndex_Temp = new int[smi->core_->getNode(1)->getRowLength(i)];
        memcpy(rowIndex_Temp, smi->core_->getNode(1)->getRowIndices(i), 
            smi->core_->getNode(1)->getRowLength(i)*sizeof(int));
        // Shift indizes, therefore subtract the number of 
        // stage 1 variables from the current index
        for(int j=0; j< smi->core_->getNode(1)->getRowLength(i);j++){
            rowIndex_Temp[j]-= mncol_;
        }	
        // Now copy indices of stage 2
        memcpy(sSt2iels_+sSt2rstrt_[i-mnrow_], rowIndex_Temp+snSt1len_[i-mnrow_], 
            (snSt2len_[i-mnrow_])*sizeof(int));
        // Delete temporary array
        delete [] rowIndex_Temp;
    }

    // Copy stochastic data
    // Declaration of an array for probabilities of scenarios
    scenProb_ = new double[nScen_];
    // Copy probability for each scenario
    for(int scen_=0; scen_<nScen_; scen_++ ){
        scenProb_[scen_] = smi->getLeafNode(scen_)->getProb();
    }
    // Set objective coefficients for the additional variables;
    // if single cut version is used, there have to be added
    // only one variable with coefficient 1; in the case of
    // multi cut version nScen variables have to be added, in 
    // this case the coefficients are the corresponding probabilities
    if(single_){
        mdobj_[mncol_] = 1;
    }
    else{
        for(int scen_=0; scen_<nScen_; scen_++){
            mdobj_[mncol_+scen_] = scenProb_[scen_];
        }
    }
    // Length of stochastic objective function, initialized by zero
    mutlobj_ = 0;
    // Declaration of start array for the objective function, shows 
    // on which position scenario k begins; is initialized by zero
    scenObjStart_ = new int[nScen_+1];
    scenObjStart_[0] = 0;
    // Counting coefficients of the objective function for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        mutlobj_+= smi->getLeafNode(scen_)->getNode()->getObjectiveLength();
        // Set scenario start array
        scenObjStart_[scen_+1] = mutlobj_;
    }
    // Declaration of an array for coefficients of objective function
    // for all scenarios
    mutdobj_ = new double[mutlobj_];
    // Declaration of an array for indices of objective function
    // for all scenarios
    mutiobj_ = new int[mutlobj_];
    // Copy coefficients and indices for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        memcpy(mutdobj_+scenObjStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getObjectiveElements(), 
            (scenObjStart_[scen_+1]-scenObjStart_[scen_])*sizeof(double) );

        memcpy(mutiobj_+scenObjStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getObjectiveIndices(), 
            (scenObjStart_[scen_+1]-scenObjStart_[scen_])*sizeof(int) );	
    }

    // Length of stochastic LHS, initialized by zero
    mutlrlo_ = 0;
    // Declaration of the start array for LHS, shows on which position
    // scenario k begins; is initialized by zero
    scenRloStart_ = new int[nScen_+1];
    scenRloStart_[0] = 0;
    // Counting coefficients of LHS for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        mutlrlo_+= smi->getLeafNode(scen_)->getNode()->getRowLowerLength();
        // Set scenario start array
        scenRloStart_[scen_+1] = mutlrlo_;
    }
    // Declaration of an array for coefficients of LHS for all scenarios
    mutdrlo_ = new double[mutlrlo_];
    // Declaration of an array for indices of LHS for all scenarios
    mutirlo_ = new int[mutlrlo_];
    // Copy coefficients and indices for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        memcpy(mutdrlo_+scenRloStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getRowLowerElements(), 
            (scenRloStart_[scen_+1]-scenRloStart_[scen_])*sizeof(double) );		
        memcpy(mutirlo_+scenRloStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getRowLowerIndices(), 
            (scenRloStart_[scen_+1]-scenRloStart_[scen_])*sizeof(int) );	
    }

    // Length of stochastic LHS, initialized by zero
    mutlrup_ = 0;
    // Declaration of the start array for RHS, shows on which position
    // scenario k begins; is initialized by zero
    scenRupStart_ = new int[nScen_+1];
    scenRupStart_[0] = 0;
    // Counting coefficients of RHS for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        mutlrup_+=smi->getLeafNode(scen_)->getNode()->getRowUpperLength();
        // Set scenario start array
        scenRupStart_[scen_+1] = mutlrup_;
    }
    // Declaration of an array for coefficients of RHS for all scenarios
    mutdrup_ = new double[mutlrup_];
    // Declaration of an array for indices of RHS for all scenarios
    mutirup_ = new int[mutlrup_];
    // Copy coefficients and indices for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        memcpy(mutdrup_+scenRupStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getRowUpperElements(), 
            (scenRupStart_[scen_+1]-scenRupStart_[scen_])*sizeof(double) );			
        memcpy(mutirup_+scenRupStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getRowUpperIndices(), 
            (scenRupStart_[scen_+1]-scenRupStart_[scen_])*sizeof(int) );	
    }

    // Length of stochastic lower bounds, initialized by zero
    mutlclo_ = 0;
    // Declaration of the start array for lower bounds, shows on which 
    // position scenario k begins; is initialized by zero
    scenCloStart_ = new int[nScen_+1];
    scenCloStart_[0] = 0;
    // Counting coefficients of lower bounds for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        mutlclo_+= smi->getLeafNode(scen_)->getNode()->getColLowerLength();
        // Set scenario start array
        scenCloStart_[scen_+1] = mutlclo_;
    }
    // Declaration of an array for coefficients of lower bounds 
    // for all scenarios
    mutdclo_ = new double[mutlclo_];
    // Declaration of an array for indices of lower bounds 
    // for all scenarios
    muticlo_ = new int[mutlclo_];
    // Copy coefficients and indices for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        memcpy(mutdclo_+scenCloStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getColLowerElements(), 
            (scenCloStart_[scen_+1]-scenCloStart_[scen_])*sizeof(double) );		
        memcpy(muticlo_+scenCloStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getColLowerIndices(), 
            (scenCloStart_[scen_+1]-scenCloStart_[scen_])*sizeof(int) );	
    }

    // Length of stochastic upper bounds, initialized by zero
    mutlcup_ = 0;
    // Declaration of the start array for upper bounds, shows on which 
    // position scenario k begins; is initialized by zero
    scenCupStart_ = new int[nScen_+1];
    scenCupStart_[0] = 0;
    // Counting coefficients of upper bounds for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        mutlcup_+=smi->getLeafNode(scen_)->getNode()->getColUpperLength();
        // Set scenario start array
        scenCupStart_[scen_+1] = mutlcup_;
    }
    // Declaration of an array for coefficients of upper bounds 
    // for all scenarios
    mutdcup_ = new double[mutlcup_];
    // Declaration of an array for indices of upper bounds 
    // for all scenarios
    muticup_ = new int[mutlcup_];
    // Copy coefficients and indices for all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        memcpy(mutdcup_+scenCupStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getColUpperElements(), 
            (scenCupStart_[scen_+1]-scenCupStart_[scen_])*sizeof(double) );
        memcpy(muticup_+scenCupStart_[scen_], 
            smi->getLeafNode(scen_)->getNode()->getColUpperIndices(), 
            (scenCupStart_[scen_+1]-scenCupStart_[scen_])*sizeof(int) );	
    }

    // Number of non-zeros for all scenarios
    mutnels_ = 0;
    // Number of stage 1 coefficients for all scenarios
    mutnSt1_ = 0;
    // Number of stage 2 coefficients for all scenarios
    mutnSt2_ = 0;
    // Number of stage 1 coefficients per row
    mutnSt1len_ = new int[snrow_*nScen_];
    // Number of stage 2 coefficients per row
    mutnSt2len_ = new int[snrow_*nScen_];
    // Declaration of the start array for matrix of stage 1 
    // coefficients, shows on which position scenario k begins; 
    // is initialized by zero
    scenMatSt1Start_ = new int[nScen_+1];
    scenMatSt1Start_[0] = 0;
    // Declaration of the start array for matrix of stage 2
    // coefficients, shows on which position scenario k begins; 
    // is initialized by zero
    scenMatSt2Start_ = new int[nScen_+1];
    scenMatSt2Start_[0] = 0;
    // Counting all non-zeros and all stage 1 coefficients
    for(int scen_=0; scen_<nScen_; scen_++){
        mutnels_+= smi->getLeafNode(scen_)->getNode()->getNumMatrixElements();
        for(int i=mnrow_; i<(mnrow_+snrow_); i++){
            // Counter for while loop
            int j = 0;
            // Temporary array for indices
            const int* mutiels_count = smi->getLeafNode(scen_)->getNode()->getRowIndices(i);
            int rowLength = smi->getLeafNode(scen_)->getNode()->getRowLength(i);
            while(j < rowLength){
                // If index is lower than number of stage 1 variables 
                // count up stage 1 coefficients
                if(mutiels_count[j] < mncol_){
                    (mutnSt1_)++;
                    j++;
                }
                // Else break
                else{
                    break;
                }
            }
            // Set number of stage 1 coefficients per row
            mutnSt1len_[scen_*snrow_+i-mnrow_] = j;
            // Set number of stage 2 coefficients per row
            mutnSt2len_[scen_*snrow_+i-mnrow_] = 
                smi->getLeafNode(scen_)->getNode()->getRowLength(i)-j;
        }
        // Set start array for stage 1 coefficients
        scenMatSt1Start_[scen_+1] = (mutnSt1_);
        // Set start array for stage 2 coefficients
        scenMatSt2Start_[scen_+1] = (mutnels_-mutnSt1_);
    }
    // Calculate number of stage 2 coefficients
    mutnSt2_ = mutnels_-mutnSt1_;
    // Declaration of an array for stage 1 coefficients
    mutSt1dels_ = new double[mutnSt1_];
    // Declaration of an array for stage 1 indices
    mutSt1iels_ = new int[mutnSt1_];
    // Declaration of an array for row start of stage 1
    mutSt1rstrt_ = new int[nScen_*snrow_+1];
    // It is initialized by zero
    mutSt1rstrt_[0] = 0;
    // Declaration of an array for stage 2 coefficients
    mutSt2dels_ = new double[mutnSt2_];
    // Declaration of an array for stage 2 indices
    mutSt2iels_ = new int[mutnSt2_];
    // Declaration of an array for row start of stage 2
    mutSt2rstrt_ = new int[nScen_*snrow_+1];
    // It is initialized by zero
    mutSt2rstrt_[0] = 0;
    // Cycle over all scenarios
    for(int scen_=0; scen_<nScen_; scen_++){
        // Cycle over all stage 2 rows
        for(int i=mnrow_; i<snrow_+mnrow_; i++){
            // Set start array for stage 1 
            mutSt1rstrt_[scen_*snrow_+i+1-mnrow_] = 
                mutSt1rstrt_[scen_*snrow_+i-mnrow_]
            +mutnSt1len_[scen_*snrow_+i-mnrow_];
            // Copy coefficients of stage 1
            memcpy(mutSt1dels_+mutSt1rstrt_[scen_*snrow_+i-mnrow_], 
                smi->getLeafNode(scen_)->getNode()->getRowElements(i), 
                (mutnSt1len_[scen_*snrow_+i-mnrow_])*sizeof(double));
            // Copy indices of stage 1
            memcpy(mutSt1iels_+mutSt1rstrt_[scen_*snrow_+i-mnrow_], 
                smi->getLeafNode(scen_)->getNode()->getRowIndices(i), 
                (mutnSt1len_[scen_*snrow_+i-mnrow_])*sizeof(int));	
            // Set start array for stage 2
            mutSt2rstrt_[scen_*snrow_+i+1-mnrow_] = 
                mutSt2rstrt_[scen_*snrow_+i-mnrow_]
            +smi->getLeafNode(scen_)->getNode()->getRowLength(i)
                -mutnSt1len_[scen_*snrow_+i-mnrow_];
            // Copy coefficients of stage 2
            memcpy(mutSt2dels_+mutSt2rstrt_[scen_*snrow_+i-mnrow_],
                smi->getLeafNode(scen_)->getNode()->getRowElements(i)
                +mutnSt1len_[scen_*snrow_+i-mnrow_], 
                (smi->getLeafNode(scen_)->getNode()->getRowLength(i)
                -mutnSt1len_[scen_*snrow_+i-mnrow_])*sizeof(double));
            // To copy stage 2 indices there has to be a temporary array
            // in which the indices are copied 			
            int* mutrowIndex_Temp= new int[smi->getLeafNode(scen_)->getNode()->getRowLength(i)];
            memcpy(mutrowIndex_Temp,
                smi->getLeafNode(scen_)->getNode()->getRowIndices(i), 
                smi->getLeafNode(scen_)->getNode()->getRowLength(i)*sizeof(int));
            // Shift indizes, therefore subtract the number of 
            // stage 1 variables from the current index			
            for(int j=0; j< smi->getLeafNode(scen_)->getNode()->getRowLength(i);j++){
                mutrowIndex_Temp[j]-= mncol_;
            }
            // Now copy indices of stage 2
            memcpy(mutSt2iels_+mutSt2rstrt_[scen_*snrow_+i-mnrow_], 
                mutrowIndex_Temp+mutnSt1len_[scen_*snrow_+i-mnrow_], 
                (smi->getLeafNode(scen_)->getNode()->getRowLength(i)
                -mutnSt1len_[scen_*snrow_+i-mnrow_])*sizeof(int));	
            // Delete temporary array
            delete [] mutrowIndex_Temp;
        }
    }

    // This part is only needed for the deterministic equivalent
    if(benders_==false){
        //
        // Declaration of a start array for scenarios,
        // initialized by zero
        scenMatStart_=new int[nScen_+1];
        scenMatStart_[0]=0;
        // Declaration of an array for coefficients
        mutdels_= new double[mutnels_];
        // Declaration of an array for indices
        mutiels_= new int[mutnels_];
        // Declaration of a start array for the stochastic 
        // matrix, initialized with zero
        mutrstrt_ = new int[snrow_*nScen_+1];
        mutrstrt_[0]=0;
        // Declaration of an array for row length
        mutlen_=new int[snrow_*nScen_];

        // Cycle over all scenarios
        for(int scen_=0; scen_<nScen_; scen_++){
            // Cycle over all stage 2 rows
            for(int i=mnrow_; i<(mnrow_+snrow_); i++){
                // Copy row length
                mutlen_[i-mnrow_+scen_*snrow_] = smi->getLeafNode(scen_)->getNode()->getRowLength(i);
                // Copy coefficients
                memcpy(mutdels_+mutrstrt_[i-mnrow_]+scenMatStart_[scen_],
                    smi->getLeafNode(scen_)->getNode()->getRowElements(i),
                    sizeof(double)*mutlen_[i-mnrow_+scen_*snrow_]);
                // Copy indices
                memcpy(mutiels_+mutrstrt_[i-mnrow_]+scenMatStart_[scen_],
                    smi->getLeafNode(scen_)->getNode()->getRowIndices(i),
                    sizeof(int)*mutlen_[i-mnrow_+scen_*snrow_]);
                // Copy start array
                mutrstrt_[scen_*snrow_+i-mnrow_+1]=mutrstrt_[scen_*snrow_+i-mnrow_]+mutlen_[i-mnrow_+scen_*snrow_];
            }
            // Set start array for scenarios
            scenMatStart_[scen_+1] = mutrstrt_[scen_*snrow_+snrow_];
        }
    }
    // End: deterministic equivalent part

    // Declaration of an array which contains the solution of
    // master problem, including the values of the additional
    // variables for the scenarios and the solution of subproblems
    bestSol_ = new double[mncol_+zSub_+sncol_*nScen_];

    // Declaration of an start array for the combined matrix
    combSt2rstrt_ = new int[snrow_+1+sncol_];
    // Declaration of an array for row length for the
    // combined matrix
    combSt2len_ = new int [snrow_+sncol_];
    // Declaration of an array for coefficents of objective function
    combdobj_ = new double[sncol_];
    // Declaration of an array for coefficients of LHS
    combdrlo_ = new double[snrow_+sncol_];
    // Declaration of an array for coefficients of RHS
    combdrup_ = new double[snrow_+sncol_];
    // Declaration of an array for coefficients of lower bounds
    combdclo_ = new double[sncol_];
    // Declaration of an array for coefficients of upper bounds
    combdcup_ = new double[sncol_];

    // Declaration of an array for translations; is used if
    // there is either a lower or an upper bound on a stage 2 
    // variable which is not equal to zero; in this case the bound is 
    // converted to zero. If there are both lower and upper bound on a 
    // stage 2 variable, then the upper bound is added to the matrix as 
    // another constraint and the lower bound is converted to zero
    dTrans_ = new double[sncol_];
    // Declaration of an array for indices of the stage 2
    // variables whose bounds were converted
    iTrans_ = new int[sncol_];

    // Warmstart between iterations
    if(warmStart_){
        // Declaration of an array which contains warmstart informations
        // for each scenario
        warmStartSub_ = new CoinWarmStart*[nScen_];
        warmStartEmptySub_ = new bool[nScen_];
    }
    // Warmstart between scenarios
    if(warmStartFol_){
        warmStartEmptySub_ = new bool[nScen_];
    }

    // ###################################################################################
    // End: copySubmodel
    // ###################################################################################

}

int Benders::solveMaster(){
    // ###################################################################################
    // Begin: solveMaster
    // ###################################################################################

    // Solve master problem. In the first iteration the master 
    // problem is built. After solving the master problem the first
    // time the objective function is expanded by the coefficients
    // for the additional variables for the scenarios.
    // In all other itrations the master problem is solved after it
    // is updated with either a feasibility cut or some optimality cuts.

    if(nIter_==0){
        // Declaration of the problem matrix
        CoinPackedMatrix* matrix = new CoinPackedMatrix(false, mncol_+zSub_, mnrow_, 
            mnels_, mdels_, miels_, mrstrt_, mlen_);
        // Load problem
        osiMaster_->loadProblem(CoinPackedMatrix(*matrix), mdclo_, mdcup_, mdobj_, mdrlo_, mdrup_);
        // Set integer informations
        osiMaster_->setInteger(miInt_, mnInt_);
        // Parameter: use dual simplex
        osiMaster_->setHintParam(OsiDoDualInInitial,false);
        // Solve master problem 
        // If there are no integer variables use initialSolve
        if(mnInt_==0){
            osiMaster_->initialSolve();
            //osiMaster_->initialBarSolve();
        }
        // Else solve via branch and bound
        else{
            osiMaster_->branchAndBound();
        }

        // If there is an optimal basis available, warmstart
        // informations are saved
        if(warmStart_ || warmStartFol_){
            if(osiMaster_->optimalBasisIsAvailable()){
                warmStartMaster_ = osiMaster_->getWarmStart();
                warmStartEmptyMaster_ = false;
            }
            // Else an emtpy warmstart object is created
            else{
                warmStartEmptyMaster_ = true;
                warmStartMaster_ = osiMaster_->getEmptyWarmStart();
            }
        }

        // Delete the problem matrix
        delete matrix;
    }
    // Else the master problem is resolved
    else{	
        if(warmStart_ || warmStartFol_ ){
            // Set warmstart informations
            osiMaster_->setWarmStart(warmStartMaster_);
        }
        // Solve master problem
        // If there are no integer variables use initialSolve
        if(mnInt_==0){
            osiMaster_->setHintParam(OsiDoDualInInitial,true);
            osiMaster_->initialSolve();
            //osiMaster_->initialBarSolve();
        }
        // Else solve via branch and bound
        else{
            osiMaster_->branchAndBound();
        }
        if(warmStart_ || warmStartFol_){
            // Delete warmstart informations
            if(warmStartEmptyMaster_==false){
                delete warmStartMaster_;
            }
            // If there is an optimal basis available, warmstart
            // informations are saved
            if(osiMaster_->optimalBasisIsAvailable()){
                warmStartMaster_ = osiMaster_->getWarmStart();
                warmStartEmptyMaster_ = false;
            }
            // Else an emtpy warmstart object is created
            else{
                warmStartEmptyMaster_=true;
                warmStartMaster_ = osiMaster_->getEmptyWarmStart();
            }	
        }
    }

    // If master problem is infeasible, print an error
    if(osiMaster_->isProvenPrimalInfeasible() && osiMaster_->isProvenDualInfeasible()){

        cout << "Error: master problem is infeasible or unbounded!" << endl;
        return -1;
    }


    return 0;

    // ###################################################################################
    // End: solveMaster
    // ###################################################################################
}

void Benders::buildSubproblem(int scen_){
    // ###################################################################################
    // Begin: buildSubproblem
    // ###################################################################################

    // Build subproblem for current scenario. 
    // If there are any stochastic data for objective function,
    // LHS, RHS, upper or lower bounds, the deterministic data
    // are replaced. If there are stochastic data in problem matrix, 
    // deterministic coefficients are substituted as well.

    // Delete combined data of previous scenario
    if(combSt2dels_)
        delete [] combSt2dels_; 
    if(combSt2iels_)
        delete [] combSt2iels_;

    // Declaration of an array for coefficients of combined problem matrix
    combSt2dels_ = new double[snSt2_+scenMatSt2Start_[scen_+1]-scenMatSt2Start_[scen_]+sncol_]; 
    // Declaration of an array for indices of the combined problem matrix
    combSt2iels_ = new int[snSt2_+scenMatSt2Start_[scen_+1]-scenMatSt2Start_[scen_]+sncol_];

    // If there are no stochastic data for the matrix in
    // current scenario, copy determinisic data.
    if(scenMatSt2Start_[scen_+1]-scenMatSt2Start_[scen_] == 0){
        combSt2nels_ = snSt2_;
        memcpy(combSt2dels_, sSt2dels_, combSt2nels_*sizeof(double));
        memcpy(combSt2iels_, sSt2iels_, combSt2nels_*sizeof(int));
        memcpy(combSt2rstrt_, sSt2rstrt_, (snrow_+1)*sizeof(int));
        memcpy(combSt2len_, snSt2len_, snrow_*sizeof(int));				
    }
    // Else the matrix is built with deterministic
    // and stochastic data
    else{
        combSt2rstrt_[0] = 0;
        // Counter
        int combSt2rstrtK = 0;
        // Cycle over all stage 2 rows
        for(int j=0; j<snrow_; j++){
            // Counter which is initialized by number of non-zeros
            // in row j in deterministic matrix
            int sCombSt2delsK = sSt2rstrt_[j];
            // Counter which is initialized by number of non-zeros
            // in row j in stochastic matrix
            int mutCombSt2delsK = mutSt2rstrt_ [scen_*snrow_+j];
            // While the end neither of deterministic matrix nor of 
            // stochastic matrix is reached, continue to copy data
            while((sCombSt2delsK < sSt2rstrt_ [j+1])||(mutCombSt2delsK < mutSt2rstrt_ [scen_*snrow_+j+1])){
                // If the end of deterministic matrix is reached, the
                // stochastic data are copied to coefficient array and
                // index array
                if(sCombSt2delsK == sSt2rstrt_ [j+1]){
                    combSt2dels_[combSt2rstrtK] = mutSt2dels_[mutCombSt2delsK];
                    combSt2iels_[combSt2rstrtK] = mutSt2iels_[mutCombSt2delsK];
                    combSt2rstrtK++;
                    mutCombSt2delsK++;
                }
                // If the end of the stochastic matrix is reached, the
                // deterministic data are copied to coefficient array
                // and index array
                else if(mutCombSt2delsK == mutSt2rstrt_ [scen_*snrow_+j+1]){
                    combSt2dels_[combSt2rstrtK] = sSt2dels_[sCombSt2delsK]; 
                    combSt2iels_[combSt2rstrtK] = sSt2iels_[sCombSt2delsK];
                    combSt2rstrtK++;
                    sCombSt2delsK++;				
                }
                // Else check, which matrix contains the next lesser index, 
                // copy the corresponding data
                else if(sSt2iels_[sCombSt2delsK] > mutSt2iels_[mutCombSt2delsK]){
                    combSt2dels_[combSt2rstrtK] = mutSt2dels_[mutCombSt2delsK];
                    combSt2iels_[combSt2rstrtK] = mutSt2iels_[mutCombSt2delsK];
                    combSt2rstrtK++;
                    mutCombSt2delsK++;
                }
                else if(sSt2iels_[sCombSt2delsK] < mutSt2iels_[mutCombSt2delsK]){
                    combSt2dels_[combSt2rstrtK] = sSt2dels_[sCombSt2delsK];
                    combSt2iels_[combSt2rstrtK] = sSt2iels_[sCombSt2delsK];
                    combSt2rstrtK++;
                    sCombSt2delsK++;				
                }
                // If there is a coefficient which has the same index
                // in both matrices, copy the stochastic coefficient
                // Note: both counter are counted up
                else if(sSt2iels_[sCombSt2delsK] == mutSt2iels_[mutCombSt2delsK]){
                    combSt2dels_[combSt2rstrtK] = mutSt2dels_[mutCombSt2delsK];
                    combSt2iels_[combSt2rstrtK] = mutSt2iels_[mutCombSt2delsK];
                    combSt2rstrtK++;
                    mutCombSt2delsK++;
                    sCombSt2delsK++;		
                }		
            }
            // Set start array for stage 2 coefficients
            combSt2rstrt_[j+1] = combSt2rstrtK;
            // Set array of length of stage 2 coefficients
            combSt2len_[j] = combSt2rstrtK-combSt2rstrt_[j];
            // Assertion: Counter of stochastic elements has to be 
            // the same value like start array of stochastic matrix
            assert(mutCombSt2delsK == mutSt2rstrt_[scen_*snrow_+j+1]);
            // Assertion: Counter of stochastic elements has to be 
            // the same size like start array of deterministic matrix
            assert(sCombSt2delsK == sSt2rstrt_[j+1]);
        }
        // Set number of combined coefficients
        combSt2nels_ = combSt2rstrtK;
        // Assertion: Counter of combined coefficients has to be
        // less or equal than the number of all stage 2 coefficients
        assert(combSt2rstrtK <= snSt2_+scenMatSt2Start_[scen_+1]-scenMatSt2Start_[scen_]);
    }

    // Copy old objective function
    memcpy(combdobj_, sdobj_, sncol_*sizeof(double));
    // If there are any stochastic coefficients for objective 
    // function in current scenario, substitute them
    if(scenObjStart_[scen_+1]-scenObjStart_[scen_] != 0){
        for(int i=scenObjStart_[scen_]; i<scenObjStart_[scen_+1]; i++){
            combdobj_[mutiobj_[i]-mncol_] = mutdobj_[i];
        }
    }

    // Copy old LHS
    memcpy(combdrlo_, sdrlo_, snrow_*sizeof(double));
    // If there are any stochastic coefficients for LHS
    // in current scenario, substitute them
    if(scenRloStart_[scen_+1]-scenRloStart_[scen_] != 0){
        for(int i=scenRloStart_[scen_]; i<scenRloStart_[scen_+1]; i++){
            combdrlo_[mutirlo_[i]-mnrow_] = mutdrlo_[i];
        }
    }

    // Copy old RHS
    memcpy(combdrup_, sdrup_, snrow_*sizeof(double));
    // If there are any stochastic coefficients for RHS
    // in current scenario, substitute them
    if(scenRupStart_[scen_+1]-scenRupStart_[scen_] != 0){
        for(int i=scenRupStart_[scen_]; i<scenRupStart_[scen_+1]; i++){
            combdrup_[mutirup_[i]-mnrow_] = mutdrup_[i];
        }
    }

    // Copy old lower bounds
    memcpy(combdclo_, sdclo_, sncol_*sizeof(double));
    // If there are any stochastic coefficients for lower  
    // bounds in current scenario, substitute them
    if(scenCloStart_[scen_+1]-scenCloStart_[scen_] != 0){
        for(int i=scenCloStart_[scen_]; i<scenCloStart_[scen_+1]; i++){
            combdclo_[muticlo_[i]-mncol_] = mutdclo_[i];
        }
    }

    // Copy old upper bounds
    memcpy(combdcup_, sdcup_, sncol_*sizeof(double));
    // If there are any stochastic coefficients for upper 
    // bounds in current scenario, substitute them
    if(scenCupStart_[scen_+1]-scenCupStart_[scen_] != 0){
        for(int i=scenCupStart_[scen_]; i<scenCupStart_[scen_+1]; i++){
            combdcup_[muticup_[i]-mncol_] = mutdcup_[i];
        }
    }

    // If there are non-zero bounds for stage 2 variables
    // they are eliminated
    convertSubproblem();

    // LHS and RHS are saved before fixed stage 1 solution
    // is added, because the old LHS and RHS are needed to calculate
    // the cuts
    for(int i=0; i<snrow_+rowAdd_; i++){
        constDrup_[i] = combdrup_[i];		
        constDrlo_[i] = combdrlo_[i];	
        // Assertion: LHS and RHS aren't supposed to be both infinity
        assert(combdrup_[i] < INFTYTEST || combdrlo_[i] > -INFTYTEST );
    }

    // ###################################################################################
    // End: buildSubproblem
    // ###################################################################################

}

void Benders::convertSubproblem(){
    // ###################################################################################
    // Begin: convertSubproblem
    // ###################################################################################

    // Ranges on stage 2 variables are converted. If there is only 
    // one finite range, it is set to zero; if there are both upper 
    // and lower bound the lower bound is set to zero and the upper 
    // bound is replaced by a new constraint.

    // Length of translation vector is initialized with zero
    lTrans_ = 0;
    // Integer which counts how many ranges are replaced by constraints
    rowAdd_ = 0;
    // Cycle over all stage 2 variables
    for(int i=0; i<sncol_; i++){
        // If lower bound is - infinity...
        if(combdclo_[i] < -INFTYTEST ){
            // ...and upper bound is infinity too, the current variable 
            // is free, no change is needed
            if(combdcup_[i] > INFTYTEST){
                continue;			
            }
            // ...and upper bound is equal to zero, no change is needed
            if(combdcup_[i] == 0){
                continue;
            }
            // ...and upper bound is not equal to zero, 
            // translate it to zero
            // Save upper bound for variable i to calculate with it
            dTrans_[lTrans_] = combdcup_[i];
            // Save index
            iTrans_[lTrans_] = i;
            // Set upper bound to zero
            combdcup_[i]=0;
            lTrans_++;
            continue;
        }
        // If lower bound is not equal to zero it is 
        // converted to zero
        if(combdclo_[i] != 0.0){
            // Save lower bound for variable i to calculate with it
            dTrans_[lTrans_] = combdclo_[i];
            // Save index
            iTrans_[lTrans_] = i;
            if(combdcup_[i] < INFTYTEST){
                // Subtract lower bound from upper bound
                combdcup_[i]-= combdclo_[i];}
            // Set lower bound to zero
            combdclo_[i] = 0;
            lTrans_++;
        }
        // If lower bound is equal to zero and upper
        // bound has finite value, a new constraint is
        // added to master problem
        if(combdcup_[i] < INFTYTEST){
            // Coefficient of new constraint is 1
            combSt2dels_[combSt2nels_+rowAdd_] = 1;
            // Copy index
            combSt2iels_[combSt2nels_+rowAdd_] = i;
            // Update row start
            combSt2rstrt_[snrow_+1+rowAdd_] = combSt2nels_+rowAdd_ +1;
            combSt2len_[snrow_+rowAdd_] = 1;
            // RHS is old upper bound
            combdrup_[snrow_+rowAdd_] = combdcup_[i];
            // LHS is set to -infinity
            combdrlo_[snrow_+rowAdd_] = -infty_;
            // Upper bound is set to infinity
            combdcup_[i] = infty_;
            // Count up counter for new rows
            rowAdd_++;						
        }
    }

    // Converting: All rows which contain a converted variable
    // are updated by the translation vector
    for(int i=0; i<snrow_; i++){
        int ccombSt2Strt = combSt2rstrt_[i];
        int cTrans=0;
        while(ccombSt2Strt < combSt2rstrt_[i+1] && cTrans < lTrans_){
            if(combSt2iels_[ccombSt2Strt] < iTrans_[cTrans]){
                ccombSt2Strt++;
                continue;
            }
            if(combSt2iels_[ccombSt2Strt] > iTrans_[cTrans]){
                cTrans++;
                continue;
            }
            if(combdrup_[i] < INFTYTEST){
                combdrup_[i]-= combSt2dels_[ccombSt2Strt]*dTrans_[cTrans];
            }
            if(combdrlo_[i] > -INFTYTEST){
                combdrlo_[i]-= combSt2dels_[ccombSt2Strt]*dTrans_[cTrans];
            }
            ccombSt2Strt++;
            cTrans++;
        }	
    }
    // The objective value changes if there is any
    // translation, so the translation is added to the 
    // objective value after solving the problem
    objTrans_ = 0;
    for(int i=0; i<lTrans_; i++){
        objTrans_+= combdobj_[iTrans_[i]]*dTrans_[i];
    }


    // ###################################################################################
    // End: convertSubproblem
    // ###################################################################################
}


void Benders::addSt1ToSubproblem(int scen_, double* mColSol){
    // ###################################################################################
    // Begin: addSt1ToSubproblem
    // ###################################################################################

    // Update RHS of current subproblem with fixed stage 1 solution

    // Combine LHS and RHS with current values of stage 1 variables
    // Cycle over all rows
    for(int j=0; j<snrow_; j++){
        // Counter which is initialized by number of stage 1 elements in
        // row j in deterministic matrix
        int cstg1els = sSt1rstrt_[j];
        // Counter which is initialized with number of elements in
        // row j in stochastic matrix
        int mutcstg1els = mutSt1rstrt_ [scen_*snrow_+j];
        // While the end neither of deterministic matrix nor of
        // stochastic matrix is reached, continue to copy data
        while((cstg1els < sSt1rstrt_ [j+1])||(mutcstg1els < mutSt1rstrt_ [scen_*snrow_+j+1])){
            // If the end of deterministic matrix is reached, use
            // stochastic data for update of RHS/LHS
            if(cstg1els == sSt1rstrt_ [j+1]){
                // Update RHS, if it is finite
                if(combdrup_[j] < INFTYTEST){
                    combdrup_[j]-= mutSt1dels_[mutcstg1els]*mColSol[mutSt1iels_[mutcstg1els]];
                }
                // Update LHS, if it is finite
                if(combdrlo_[j] > -INFTYTEST){
                    combdrlo_[j]-= mutSt1dels_[mutcstg1els]*mColSol[mutSt1iels_[mutcstg1els]];
                }
                mutcstg1els++;
            }
            // If the end of stochastic matrix is reached, use  
            // deterministic data for update of RHS/LHS
            else if(mutcstg1els == mutSt1rstrt_ [scen_*snrow_+j+1]){
                // Update RHS, if it is finite
                if(combdrup_[j] < INFTYTEST){
                    combdrup_[j]-= sSt1dels_[cstg1els]*mColSol[sSt1iels_[cstg1els]];
                }
                // Update LHS, if it is finite
                if(combdrlo_[j] > -INFTYTEST){
                    combdrlo_[j]-= sSt1dels_[cstg1els]*mColSol[sSt1iels_[cstg1els]];
                }
                cstg1els++;				
            }
            // Else check, which matrix contains the next lesser index 
            // and use the corresponding data for update
            else if(sSt1iels_[cstg1els] > mutSt1iels_[mutcstg1els]){
                // Update RHS, if it is finite
                if(combdrup_[j] < INFTYTEST){
                    combdrup_[j]-= mutSt1dels_[mutcstg1els]*mColSol[mutSt1iels_[mutcstg1els]];
                }
                // Update LHS, if it is finite
                if(combdrlo_[j] > -INFTYTEST){
                    combdrlo_[j]-= mutSt1dels_[mutcstg1els]*mColSol[mutSt1iels_[mutcstg1els]];
                }
                mutcstg1els++;			
            }
            else if(sSt1iels_[cstg1els] < mutSt1iels_[mutcstg1els]){
                // Update RHS, if it is finite
                if(combdrup_[j] < INFTYTEST){
                    combdrup_[j]-= sSt1dels_[cstg1els]*mColSol[sSt1iels_[cstg1els]];
                }
                // Update LHS, if it is finite
                if(combdrlo_[j] > -INFTYTEST){
                    combdrlo_[j]-= sSt1dels_[cstg1els]*mColSol[sSt1iels_[cstg1els]];
                }
                cstg1els++;					
            }
            // If there is a coefficient which has the same index in both
            // matrices, use the stochastic coefficient for update
            // Note: both counter are counted up
            else if(sSt1iels_[cstg1els] == mutSt1iels_[mutcstg1els]){
                // Update RHS, if it is finite
                if(combdrup_[j] < INFTYTEST){
                    combdrup_[j]-= mutSt1dels_[mutcstg1els]*mColSol[mutSt1iels_[mutcstg1els]];
                }
                // Update LHS, if it is finite
                if(combdrlo_[j] > -INFTYTEST){
                    combdrlo_[j]-= mutSt1dels_[mutcstg1els]*mColSol[mutSt1iels_[mutcstg1els]];
                }
                mutcstg1els++;	
                cstg1els++;
            }		
        }
        // Assertion: Counter of stochastic elements has to be the same
        // size like row start array of stage 1 coefficients in stochastic matrix
        assert(mutcstg1els == mutSt1rstrt_[scen_*snrow_+j+1]);
        // Assertion: Counter of the deterministic elements has to be the
        // same size like row start array of stage 1 coefficients in deterministic matrix
        assert(cstg1els == sSt1rstrt_[j+1]);
    }

    // ###################################################################################
    // End: addSt1ToSubproblem
    // ###################################################################################

}
int Benders::solveSubproblem(int scen_, bool fCut){
    // ###################################################################################
    // Begin: solveSubproblem
    // ###################################################################################

    // Solve subproblem

    // Set up problem matrix; note: if there were created new
    // constraints during the method "converstSubproblem",
    // number of rows and number of elements are expanded 
    // by number of new constraints
    CoinPackedMatrix* matrix = new CoinPackedMatrix(false, sncol_, snrow_+rowAdd_,  combSt2nels_+rowAdd_,
        combSt2dels_, combSt2iels_, combSt2rstrt_, combSt2len_);


    // Load subproblem
    osiSubproblem_->loadProblem(CoinPackedMatrix(*matrix), combdclo_, combdcup_, combdobj_, combdrlo_, combdrup_);
    // Parameter: use dual simplex
    osiSubproblem_->setHintParam(OsiDoDualInInitial,true);
    // In the first iteration there are no warmstart informations
    // available. So distinguish between first and all other iterations.
    if(nIter_==0){
        // Warmstart between scenarios
        if(warmStartFol_ && scen_==0){
            // If an optimal basis is available, get warmstart 
            // informations for current scenario
            if(osiSubproblem_->optimalBasisIsAvailable()){
                warmStartSubFol_ = osiSubproblem_->getWarmStart();
                warmStartEmptySub_[scen_] = false;
            }
            // Else create an empty warmstart object
            else{
                warmStartSubFol_ = osiSubproblem_->getEmptyWarmStart();
                warmStartEmptySub_[scen_] = true;
            }		
        }
        if(warmStartFol_ && scen_!=0){

            if(warmStartEmptySub_[scen_]==false && fCut==false){
                // Set warmstart informations for current scenario
                osiSubproblem_->setWarmStart(warmStartSubFol_);
            }
            // Solve subbroblem
            osiSubproblem_->initialSolve();
            if(fCut==false){
                // Delete warmstart informations
                if(warmStartEmptySub_[scen_]==false){
                    delete warmStartSubFol_;
                }
                // If an optimal basis is available, get warm 
                // start informations for current scenario
                if(osiSubproblem_->optimalBasisIsAvailable()){
                    warmStartSubFol_ = osiSubproblem_->getWarmStart();
                    warmStartEmptySub_[scen_] = false;
                }
                // Else create an empty warmstart object
                else{
                    warmStartSubFol_ = osiSubproblem_->getEmptyWarmStart();
                    warmStartEmptySub_[scen_] = true;
                }
            }
        }
        else{
            // Solve subproblem
            osiSubproblem_->initialSolve();
        }
        // Warmstart between scenarios
        if(warmStart_){
            // If an optimal basis is available, get warm 
            // start informations for current scenario
            if(osiSubproblem_->optimalBasisIsAvailable()){
                warmStartSub_[scen_] = osiSubproblem_->getWarmStart();
                warmStartEmptySub_[scen_] = false;
            }
            // Else create an empty warmstart object
            else{
                warmStartSub_[scen_] = osiSubproblem_->getEmptyWarmStart();
                warmStartEmptySub_[scen_] = true;
            }	
        }
    }
    // In all other iterations warmstart informations are used
    else{

        // Warmstart between iterations
        if(warmStartFol_ && warmStartEmptySub_[scen_]==false && fCut==false){
            // Set warmstart informations for current scenario
            osiSubproblem_->setWarmStart(warmStartSubFol_);
        }

        // Warmstart between scenarios
        if(warmStart_ && warmStartEmptySub_[scen_]==false && fCut==false){
            // Set warmstart informations for current scenario
            osiSubproblem_->setWarmStart(warmStartSub_[scen_]);
        }
        // Solve subbroblem
        osiSubproblem_->initialSolve();

        // Warmstart between iterations
        if(warmStartFol_ && fCut==false){
            // Delete warmstart informations
            if(warmStartEmptySub_[scen_]==false){
                delete warmStartSubFol_;
            }
            // If an optimal basis is available, get warm 
            // start informations for current scenario
            if(osiSubproblem_->optimalBasisIsAvailable()){
                warmStartSubFol_ = osiSubproblem_->getWarmStart();
                warmStartEmptySub_[scen_] = false;
            }
            // Else create an empty warmstart object
            else{
                warmStartSubFol_ = osiSubproblem_->getEmptyWarmStart();
                warmStartEmptySub_[scen_] = true;
            }
        }

        // Warmstart between scenarios
        if(warmStart_ && fCut==false){
            // Delete warmstart informations
            if(warmStartEmptySub_[scen_]==false){
                delete warmStartSub_[scen_];
            }
            // If an optimal basis is available, get warm 
            // start informations for current scenario
            if(osiSubproblem_->optimalBasisIsAvailable()){
                warmStartSub_[scen_] = osiSubproblem_->getWarmStart();
                warmStartEmptySub_[scen_] = false;
            }
            // Else create an empty warmstart object
            else{
                warmStartSub_[scen_] = osiSubproblem_->getEmptyWarmStart();
                warmStartEmptySub_[scen_] = true;
            }
        }
    }	

    // Delete matrix
    delete matrix;
    // If subproblem is infeasible, print an error
    if(osiSubproblem_->isProvenDualInfeasible()){
        cout << "Error: Dual subproblem is infeasible or unbounded!" << endl;
        return -1;
    }

    return 0;

    // ###################################################################################
    // End: solveSubproblem
    // ###################################################################################
}

void Benders::calcOptiCut( int oCut, int scen_){
    // ###################################################################################
    // Begin: calcOptiCut
    // ###################################################################################

    // Calculate an optimality cut: p*(d-Bx)=p*d-p*Bx < z <=> -z-p*Bx < -p*d

    // Dual solution of current subproblem
    const double *SolDual = osiSubproblem_->getRowPrice();

    // First calculate RHS of the cut: p*d
    // Initialize RHS by zero
    cutRowUB_[oCut] = 0;

    // To calculate RHS, use constant RHS/LHS; if the 
    // corresponding dual variable has a negative value, use
    // RHS; if the corresponding dual variable has a positive
    // value, use LHS; Check, if these values are close
    // to zero
    for(int i=0; i<snrow_+rowAdd_; i++){
        if(SolDual[i] < 0){
            if(SolDual[i] < -ZEROTEST){
                cutRowUB_[oCut]+= constDrup_[i]*SolDual[i];
            }
        }
        else{
            if(SolDual[i] > ZEROTEST){
                cutRowUB_[oCut]+= constDrlo_[i]*SolDual[i];
            }
        }
    }
    // Initialize array of indices and elements
    for(int i=0 ; i<mncol_ ; i++){
        // Initialize indizes with indices of
        // stage 1 variables
        cutIndex_[(mncol_+1)*oCut+i] = i;
        // Initialize coefficients with zero
        cutElements_[(mncol_+1)*oCut+i] = 0;
    }
    // The last position of index-array is
    // the additional variable for current scenario
    cutIndex_[(mncol_+1)*oCut+mncol_] = mncol_+scen_;
    // This variable has the coefficient -1, 
    // which corresponds to -z
    cutElements_[(mncol_+1)*oCut+mncol_] = -1;
    // Cycle over all rows
    for(int j=0; j<snrow_; j++){
        // Counter which is initialized by number of non-zeros
        // in row j in deterministic matrix
        int cstg1els = sSt1rstrt_[j];
        // Counter which is initialized by number of non-zeros
        // in row j in stochastic matrix
        int mutcstg1els = mutSt1rstrt_ [scen_*snrow_+j];
        // While the end neither of deterministic matrix nor of 
        // stochastic matrix is reached, continue to update
        // cut coefficients
        while((cstg1els < sSt1rstrt_ [j+1])||(mutcstg1els < mutSt1rstrt_ [scen_*snrow_+j+1])){
            // If the end of the deterministic matrix is reached, use
            // stochastic data for updating cut coefficients
            if(cstg1els == sSt1rstrt_ [j+1]){
                cutElements_[(mncol_+1)*oCut+mutSt1iels_[mutcstg1els]]-= SolDual[j]*mutSt1dels_[mutcstg1els];
                mutcstg1els++;
            }
            // If the end of the stochastic matrix is reached, use the
            // deterministic data for updating cut coefficients
            else if(mutcstg1els == mutSt1rstrt_[scen_*snrow_+j+1]){
                cutElements_[(mncol_+1)*oCut+sSt1iels_[cstg1els]]-= SolDual[j]*sSt1dels_[cstg1els];
                cstg1els++;	
            }
            // Else check, which matrix contains the next lesser index 
            // and use the corresponding data for update
            else if(sSt1iels_[cstg1els] > mutSt1iels_[mutcstg1els]){
                cutElements_[(mncol_+1)*oCut+mutSt1iels_[mutcstg1els]]-= SolDual[j]*mutSt1dels_[mutcstg1els];
                mutcstg1els++;	
            }
            else if(sSt1iels_[cstg1els] < mutSt1iels_[mutcstg1els]){
                cutElements_[(mncol_+1)*oCut+sSt1iels_[cstg1els]]-= SolDual[j]*sSt1dels_[cstg1els];
                cstg1els++;	
            }
            // If there is a coefficient which has the same index
            // in both matrices, use stochastic coefficient
            // Note: both counter are counted up
            else if(sSt1iels_[cstg1els] == mutSt1iels_[mutcstg1els]){
                cutElements_[(mncol_+1)*oCut+mutSt1iels_[mutcstg1els]]-= SolDual[j]*mutSt1dels_[mutcstg1els];
                mutcstg1els++;	
                cstg1els++;
            }		
        }
        // Assertion: Counter of stochastic elements has the same
        // size like row start array of stage 1 coefficients in
        // stochastic matrix
        assert(mutcstg1els == mutSt1rstrt_[scen_*snrow_+j+1]);
        // Assertion: Counter of deterministic elements has the
        // same size like row start array of stage 1 coefficients in
        // deterministic matrix
        assert(cstg1els == sSt1rstrt_[j+1]);
    }

    // ###################################################################################
    // End: calcOptiCut
    // ###################################################################################
}

void Benders::addOptiCut(int oCut){
    // ###################################################################################
    // Begin: addOptiCut
    // ###################################################################################

    // Add optimaliy cut(s) to master problem. In case of the single cut
    // version it is only one cut whose coefficients are calculated during
    // this method. In case of the multi cut version there are added less
    // or equal nScen_ cuts.

    // Initialize LHS of the cut with -infinity
    double cutRowLB = -infty_;
    // Declaration of a temporary array for cut coefficients
    double *cutElements = new double[mncol_+1];
    // Declaration of a temporary array for cut indices
    int *cutIndex = new int[mncol_+1];
    // Initialize RHS with zero
    double cutRowUB = 0;
    // In case of the single cut version, only one cut is calculated
    // If knapsack inequalities are used, calculate this cut with the
    // same coefficients like the ones of the single cut
    if(single_ || knapsack_){
        // Cycle over all cuts which were calculated during the
        // method "calcOptiCut"
        for(int i=0; i<oCut; i++){
            // Sum up all values of RHS and multiply each with the corresponding
            // probability
            cutRowUB+= cutRowUB_[i]*scenProb_[cutIndex_[i*(mncol_+1)+mncol_]-mncol_];
            // Cycle over all stage 1 variables
            for(int j=0; j< mncol_; j++){
                // In the first cycle all arrays are initialized
                if(i == 0){
                    cutElements[j] = cutElements_[j]*scenProb_[cutIndex_[mncol_]-mncol_];
                    cutIndex[j] = j;
                }
                // In all other cycles sum up cut elements
                else{			
                    cutElements[j]+= cutElements_[i*(mncol_+1)+j]*scenProb_[cutIndex_[i*(mncol_+1)+mncol_]-mncol_];
                }
            }
        }	
        // The last position of cut coefficient array
        // is the additional variable with coefficient -1
        cutElements[mncol_] = -1;
        cutIndex[mncol_] = mncol_;

        // Add cut to master problem
        // Note: RHS of the cut is multiplied by -1, since p*d
        // has to be on the other side of the inequalitiy
        if(single_){
            osiMaster_->addRow(mncol_+1, cutIndex, cutElements, cutRowLB, -cutRowUB);
        }
        // If knapsack inequalities are used, check if all stage 1 
        // variables are integer
        if(knapsack_ && mncol_==mnInt_){
            // Declaration of an array for indices
            int* kIndex = new int[mncol_];
            // Declaration of an array for coefficients
            double* kCoeff = new double[mncol_];
            // Cycle over all stage 1 variables
            for(int k=0; k<mncol_; k++){
                // Round down the coefficients of the inequality
                kCoeff[k] = floor(mdobj_[k] + cutElements[k]);
                kIndex[k] = k;
            }
            // Add a new constraint to master problem, round down
            // RHS of the cut
            osiMaster_->addRow(mncol_, kIndex, kCoeff, -infty_, floor(UB_-cutRowUB));
            delete [] kIndex;
            delete [] kCoeff;
        }

    }
    // In case of the multi cut version all cuts which were calculated
    // during the method "calcOptiCut" are added to master problem
    if(!single_){
        // Cycle over all cuts which were created
        for(int i=0; i<oCut; i++){
            // Copy the cut and add it to the master problem
            memcpy(cutElements, cutElements_+i*(mncol_+1), (mncol_+1)*sizeof(double));
            memcpy(cutIndex, cutIndex_+i*(mncol_+1), (mncol_+1)*sizeof(int));
            osiMaster_->addRow(mncol_+1, cutIndex, cutElements, cutRowLB, -cutRowUB_[i]);
        }	
    }
    delete [] cutElements;
    delete [] cutIndex;

    // ###################################################################################
    // End: addOptiCut
    // ###################################################################################
}

void Benders::addFeasCut( int scen_){
    // ###################################################################################
    // Begin: addFeasCut
    // ###################################################################################

    // Calculate a feasibility cut and add it to master problem
    // cut: r*(d-Bx)=r*d-r*Bx < 0 <=> -r*Bx < -r*d

    // Calculate RHS: r*d
    // Current ray
    vector<double*> ray = osiSubproblem_->getDualRays(1);
    double * dualRay;
    dualRay=*ray.begin();
    // Declaration of LHS and RHS of the new cut; they are
    // initialized by -infinity and zero, respectively
    double cutRowLB = -infty_;
    double cutRowUB = 0;
    // Get the current RHS to calculate the current objective 
    // value with the current ray
    const double* subRHS = osiSubproblem_->getRightHandSide();
    // Check, in which direction the ray points; therefore
    // a counter is initialized with zero
    double signTest = 0;
    // Calculate the objective value of subproblem
    for(int i=0; i<snrow_+rowAdd_; i++){
        signTest+=subRHS[i]*dualRay[i];
    }
    // If the value is negative, the ray points in the
    // wrong direction; in this case the sign of the
    // ray elements is changed
    if(signTest < 0){
        for(int i=0; i<snrow_+rowAdd_; i++){
            dualRay[i]*=(-1);
        }	
    }
    // To calculate r*d, use constant RHS/LHS; if the corresponding
    // dual variable is negative, use RHS; if the corresponding
    // dual variablé is positive, use LHS
    for(int i=0; i<snrow_+rowAdd_; i++){
        if(dualRay[i] > 0){
            cutRowUB+= constDrlo_[i]*dualRay[i];
        }
        else{
            cutRowUB+= constDrup_[i]*dualRay[i];
        }
    }	
    // Declaration of an array for cut indices
    int * cutIndex = new int[mncol_];
    // Declaration of an array for cut coefficients
    double* cutElements = new double[mncol_];

    // Initialize arrays of indices and elements
    for(int i=0 ; i<mncol_ ; i++){
        // Initialize indices by indices of
        // stage 1 variables
        cutIndex[i] = i;
        // Initialize coefficients by 0
        cutElements[i] = 0;
    }	
    // Cycle over all rows
    for(int j=0; j<snrow_; j++){
        // Counter which is initialized by number of non-zeros
        // in row j in deterministic matrix
        int cstg1els = sSt1rstrt_[j];
        // Counter which is initialized by number of non-zeros
        // in row j+scen_*snrow_ in stochastic matrix
        int mutcstg1els = mutSt1rstrt_ [scen_*snrow_+j];
        // While the end neither of deterministic matrix nor of 
        // stochastic matrix is reached, continue to update the 
        // cut coefficients
        while((cstg1els < sSt1rstrt_ [j+1])||(mutcstg1els < mutSt1rstrt_ [scen_*snrow_+j+1])){
            // If the end of the deterministic matrix is reached, use
            // stochastic data for updating the cut coefficients
            if(cstg1els == sSt1rstrt_ [j+1]){
                cutElements[mutSt1iels_[mutcstg1els]]-= dualRay[j]*mutSt1dels_[mutcstg1els];
                mutcstg1els++;
            }
            // If the end of the stochastic matrix is reached, use
            // deterministic data for updating the cut coefficients
            else if(mutcstg1els == mutSt1rstrt_ [scen_*snrow_+j+1]){
                cutElements[sSt1iels_[cstg1els]]-= dualRay[j]*sSt1dels_[cstg1els];
                cstg1els++;	
            }
            // Else check, which matrix contains the next lesser index 
            // and use the corresponding data for update
            else if(sSt1iels_[cstg1els] > mutSt1iels_[mutcstg1els]){
                cutElements[mutSt1iels_[mutcstg1els]]-= dualRay[j]*mutSt1dels_[mutcstg1els];
                mutcstg1els++;	
            }
            else if(sSt1iels_[cstg1els] < mutSt1iels_[mutcstg1els]){
                cutElements[sSt1iels_[cstg1els]]-= dualRay[j]*sSt1dels_[cstg1els];
                cstg1els++;	
            }
            // If there is a coefficient which has the same index
            // in both matrices, use the stochastic coefficient
            // Note: both counter are counted up
            else if(sSt1iels_[cstg1els] == mutSt1iels_[mutcstg1els]){
                cutElements[mutSt1iels_[mutcstg1els]]-= dualRay[j]*mutSt1dels_[mutcstg1els];
                mutcstg1els++;	
                cstg1els++;
            }		
        }
        // Assertion: Counter of stochastic elements has to be the same
        // size like row start array of the stage 1 coefficients in
        // stochastic matrix
        assert(mutcstg1els == mutSt1rstrt_[scen_*snrow_+j+1]);
        // Assertion: Counter of the deterministic elements has to be the
        // same size like row start array of the stage 1 coefficients in
        // deterministic matrix
        assert(cstg1els == sSt1rstrt_[j+1]);
    }
    // Add the cut to the master problem
    // Note: RHS of the cut is multiplied by -1, since r*d
    // has to be on the other side of the inequalitiy
    osiMaster_->addRow(mncol_, cutIndex, cutElements, cutRowLB, -cutRowUB);

    delete [] cutIndex;
    delete [] cutElements;
    delete [] dualRay;

    // ###################################################################################
    // End: addFeasCut
    // ###################################################################################
}

int Benders::trustRegion(double* mSol, double LBmax, int oCut, bool fCut){
    // ###################################################################################
    // Begin: trustRegion
    // ###################################################################################

    // Trust Region is a method to accelerate benders decomposition
    // if there are any binary variables

    if(nIter_==0){
        tRdelta_=mncol_-0.6*mncol_;
        if(tRdelta_< 1){
            tRdelta_=1.0;
        }
    }
    // The new cut which is generated during trust region method
    // is deleted after solving master problem
    if(nIter_!=0){
        int* tRind = new int[1];
        // Either if there is a feasibility cut or the single cut
        // version is chosen, the row which is deleted has
        // index "number of rows - 1"
        if(fCut || single_){
            tRind[0] = osiMaster_->getNumRows()-1-1;
            osiMaster_->deleteRows(1, tRind);
        }
        // If there are some optimality cuts, the row which is
        // deleted has index "number of rows - number of cuts"
        else{
            tRind[0] = osiMaster_->getNumRows()-oCut-1;
            osiMaster_->deleteRows(1, tRind);
        }
        delete [] tRind;
    }


    // Counter for binary variables which have the value 1
    // in the current stage 1 solution
    int num = 0;
    // Declaration of an array for coefficients for 
    // the new constraint
    double* tRCoeff = new double[mnBin_];
    // Cycle over all binary variables
    for(int i=0; i<mnBin_; i++){
        // If the variable has value 1, it gets the
        // coefficient -1
        if(mSol[miBin_[i]]==1.0){
            tRCoeff[i] = -1;
            num++;
        }
        // If the variable has value 0, it gets the
        // coefficient 1
        else{
            tRCoeff[i] = 1;
        }
    }

    // Add a new constraint to master problem 
    osiMaster_->addRow(mnBin_, miBin_, tRCoeff, -infty_, tRdelta_);//-num);

    // If lower bound of iteration i and iteration i-1 are closer
    // than a specific rate, delta is updated
    if(LB_< INFTYTEST && LBmax < INFTYTEST){
        if((abs(LB_-LBmax)/LBmax)<tRrateLB_) {
            tRdelta_+=mnBin_*tRrateD_;
        }
    }
    delete [] tRCoeff;
    // If delta is greater or equal than the number of binaries
    // return 0, the trust region method stops
    if(tRdelta_ >= mnBin_){
        return 0;
    }
    else{ 
        return 1;
    }

    // ###################################################################################
    // End: trustRegion
    // ###################################################################################
}


int Benders::writeSolutionFileBenders(const char * solpath, double time ){
    ofstream ofs;
    ofs.open(solpath);
    if(!ofs){
        cout << "Cannot open file!" << endl;
        return -1;
    }
    ofs << "Elapsed Time (sec): " << time << endl;
    ofs << "Number of iterations: " << nIter_ << endl;
    ofs << "Objective Value: " << LB_ << endl;

    ofs << "Master: " << endl;
    for(int i=0; i< mncol_; i++){
        ofs << bestSol_[i] << endl;
    }

    for(int j=0; j<nScen_;j++){
        ofs << "Subproblem " << j+1 << ":" << endl;
        for(int i=mncol_+zSub_+j*sncol_; i<mncol_+zSub_+(j+1)*sncol_; i++){
            ofs << bestSol_[i] << endl;
        }
    }
    ofs.close();

    return 0;
}


int Benders::detEq(){
    // ###################################################################################
    // Begin: detEq
    // ###################################################################################

    // Solve problem with deterministic equivalent

    // Number of elements
    int dEnels = 0;
    // Declaration of an array for coefficients
    double* dEdels = new double[mnels_+mutnels_+snels_*nScen_];
    // Declaration of an array for indices
    int* dEiels = new int[mnels_+mutnels_+snels_*nScen_];
    // Declaration of a start array
    int* dErstrt = new int[mnrow_+snrow_*nScen_+1];
    // Declaration of an array for row length 
    int* dElen = new int [mnrow_+snrow_*nScen_];
    // Declaration of an array for objective function
    double* dEdobj = new double[mncol_+sncol_*nScen_];
    // Declaration of an array for coefficients of lower bounds
    double* dEdclo = new double[mncol_+sncol_*nScen_];
    // Declaration of an array for coefficients of upper bounds
    double* dEdcup = new double[mncol_+sncol_*nScen_];
    // Declaration of an array for coefficients of LHS
    double* dEdrlo = new double[mnrow_+snrow_*nScen_];
    // Declaration of an array for coefficients of RHS
    double* dEdrup = new double[mnrow_+snrow_*nScen_];

    // Copy master informations
    // Copy matrix elements
    memcpy(dEdels, mdels_, sizeof(double)*mnels_ );
    // Copy matrix indices
    memcpy(dEiels, miels_, sizeof(int)*mnels_);
    // Copy number of elements
    dEnels=mnels_;
    // Copy start array
    memcpy(dErstrt, mrstrt_, sizeof(int)*(mnrow_+1));
    // Copy row length
    memcpy(dElen, mlen_, sizeof(int)*mnrow_);
    // Copy objective function
    memcpy(dEdobj, mdobj_, sizeof(double)*mncol_);
    // Copy lower bounds
    memcpy(dEdclo, mdclo_, sizeof(double)*mncol_);
    // Copy upper bounds
    memcpy(dEdcup, mdcup_, sizeof(double)*mncol_);
    // Copy LHS
    memcpy(dEdrlo, mdrlo_, sizeof(double)*mnrow_);
    // Copy RHS
    memcpy(dEdrup, mdrup_, sizeof(double)*mnrow_);

    // Copy subproblem informations
    for(int scen_=0;scen_<nScen_;scen_++){
        // Combine deterministic and stochastic elements
        buildProblem(scen_);		
        // Copy coefficients of matrix
        memcpy(dEdels+dEnels, combdels_, combnels_*sizeof(double));
        // Copy indices
        memcpy(dEiels+dEnels, combiels_, combnels_*sizeof(int));
        // Shift start array
        for(int i=0; i<(snrow_+1); i++){
            combrstrt_[i]+=dEnels;
        }
        // Add number of elements
        dEnels+=combnels_;
        // Copy start array
        memcpy(dErstrt+(mnrow_+1)+snrow_*scen_, combrstrt_+1, snrow_*sizeof(int));
        // Copy row length
        memcpy(dElen+mnrow_+snrow_*scen_, comblen_, snrow_*sizeof(int));
        // Copy objective coefficients multiplied by the corresponding probability
        for(int i=0; i<sncol_; i++){
            combdobj_[i]*=scenProb_[scen_];
        }
        memcpy(dEdobj+mncol_+sncol_*scen_, combdobj_, sncol_*sizeof(double));
        // Copy upper bound
        memcpy(dEdcup+mncol_+sncol_*scen_, combdcup_, sncol_*sizeof(double));
        // Copy lower bound
        memcpy(dEdclo+mncol_+sncol_*scen_, combdclo_, sncol_*sizeof(double));
        // Copy RHS
        memcpy(dEdrup+mnrow_+snrow_*scen_, combdrup_, snrow_*sizeof(double));
        // Copy LHS
        memcpy(dEdrlo+mnrow_+snrow_*scen_, combdrlo_, snrow_*sizeof(double));
    }

    // Declaration of problem matrix
    CoinPackedMatrix* matrix = new CoinPackedMatrix(false, mncol_+nScen_*sncol_, mnrow_+nScen_*snrow_, 
        dEnels, dEdels, dEiels, dErstrt, dElen);
    // Load problem
    osiDetEq_->loadProblem(CoinPackedMatrix(*matrix), dEdclo, dEdcup, dEdobj, dEdrlo, dEdrup);
    // Set integer informations
    osiDetEq_->setInteger(miInt_, mnInt_);

    // Solve problem 
    // If there are no integer variables use initialSolve
    if(mnInt_==0){
        osiDetEq_->initialSolve();
    }
    // Else solve via branch and bound
    else{
        osiDetEq_->branchAndBound();
    }

    delete [] dEdels;
    delete [] dEiels;
    delete [] dErstrt;
    delete [] dElen;
    delete [] dEdobj;
    delete [] dEdrlo;
    delete [] dEdrup;
    delete [] dEdclo;
    delete [] dEdcup;
    delete matrix;

    // If problem is infeasible, print an error
    if(osiDetEq_->isProvenPrimalInfeasible()){
        cout << "Error: Problem is infeasible or unbounded!" << endl;
        return -1;
    }

    // Print solution
    objDetEq_ = osiDetEq_->getObjValue();
    cout << "Objective value det Eq: " << objDetEq_ << endl;

    const double * det=osiDetEq_->getColSolution();
    memcpy(bestSol_, det, sizeof(double)*(mncol_+sncol_*nScen_));


    return 0;

    // ###################################################################################
    // End: detEq
    // ###################################################################################
}

void Benders::buildProblem(int scen_){
    // ###################################################################################
    // Ende: buildProblem
    // ###################################################################################

    // Build problem for deterministic equivalent

    // Delete combined data of previous scenario
    if(combdels_)
        delete [] combdels_; 
    if(combiels_)
        delete [] combiels_;
    if(combrstrt_)
        delete [] combrstrt_;
    if(comblen_)
        delete []comblen_;

    // Declaration of an array for coefficients of
    // combined problem matrix
    combdels_ = new double[snels_+scenMatStart_[scen_+1]-scenMatStart_[scen_]]; 
    // Declaration of an array for indices of combined
    // problem matrix
    combiels_ = new int[snels_+scenMatStart_[scen_+1]-scenMatStart_[scen_]];
    combrstrt_ = new int[snrow_+1];
    comblen_ = new int[snrow_];

    // Copy matrix
    combrstrt_[0] = 0;
    // Counter
    int combrstrtK = 0;
    // Cycle over all stage 2 rows
    for(int j=0; j<snrow_; j++){
        // Counter which is initialized by number of non-zeros
        // in row j in deterministic matrix
        int sCombdelsK = srstrt_[j];
        // Counter which is initialized by number of non-zeros
        // in row j in stochastic matrix
        int mutCombdelsK = mutrstrt_[scen_*snrow_+j];
        // While the end neither of deterministic matrix nor of 
        // stochastic matrix is reached, continue to copy data
        while((sCombdelsK < srstrt_[j+1])||(mutCombdelsK < mutrstrt_[scen_*snrow_+j+1])){							
            // If the end of deterministic matrix is reached,
            // stochastic data are copied to coefficient array and
            // index array
            if(sCombdelsK == srstrt_[j+1]){
                combdels_[combrstrtK] = mutdels_[mutCombdelsK];
                if(mutiels_[mutCombdelsK]>=mncol_){
                    combiels_[combrstrtK] = mutiels_[mutCombdelsK]+scen_*sncol_;
                }
                else{
                    combiels_[combrstrtK] = mutiels_[mutCombdelsK];
                }
                combrstrtK++;
                mutCombdelsK++;
            }
            // If the end of the stochastic matrix is reached,
            // deterministic data are copied to coefficient array
            // and index array
            else if(mutCombdelsK == mutrstrt_ [scen_*snrow_+j+1]){
                combdels_[combrstrtK] = sdels_[sCombdelsK]; 
                if(siels_[sCombdelsK]>=mncol_){
                    combiels_[combrstrtK] = siels_[sCombdelsK]+scen_*sncol_;
                }
                else{
                    combiels_[combrstrtK] = siels_[sCombdelsK];
                }
                combrstrtK++;
                sCombdelsK++;				
            }
            // Else check, which matrix contains the next lesser index 
            // and copy the corresponding data
            else if(siels_[sCombdelsK] > mutiels_[mutCombdelsK]){
                combdels_[combrstrtK] = mutdels_[mutCombdelsK];
                if(mutiels_[mutCombdelsK]>=mncol_){
                    combiels_[combrstrtK] = mutiels_[mutCombdelsK]+scen_*sncol_;
                }
                else{
                    combiels_[combrstrtK] = mutiels_[mutCombdelsK];
                }
                combrstrtK++;
                mutCombdelsK++;
            }
            else if(siels_[sCombdelsK] < mutiels_[mutCombdelsK]){
                combdels_[combrstrtK] = sdels_[sCombdelsK];
                if(siels_[sCombdelsK]>=mncol_){
                    combiels_[combrstrtK] = siels_[sCombdelsK]+scen_*sncol_;
                }
                else{
                    combiels_[combrstrtK] = siels_[sCombdelsK];
                }
                combrstrtK++;
                sCombdelsK++;				
            }
            // If there is a coefficient which has the same index
            // in both matrices, copy the stochastic coefficient
            // Note: both counter are counted up
            else if(siels_[sCombdelsK] == mutiels_[mutCombdelsK]){
                combdels_[combrstrtK] = mutdels_[mutCombdelsK];
                if(mutiels_[mutCombdelsK]>=mncol_){
                    combiels_[combrstrtK] = mutiels_[mutCombdelsK]+scen_*sncol_;
                }
                else{
                    combiels_[combrstrtK] = mutiels_[mutCombdelsK];
                }
                combrstrtK++;
                mutCombdelsK++;
                sCombdelsK++;		
            }	
        }
        // Set start array for stage 2 coefficients
        combrstrt_[j+1] = combrstrtK;
        // Set array of length of stage 2 coefficients
        comblen_[j] = combrstrtK-combrstrt_[j];
        // Assertion: Counter of stochastic elements has to be 
        // the same length like start array of the stochastic
        // matrix
        assert(mutCombdelsK == mutrstrt_[scen_*snrow_+j+1]);
        // Assertion: Counter of stochastic elements has to be 
        // the same size like start array of the deterministic
        // matrix
        assert(sCombdelsK == srstrt_[j+1]);
    }
    // Set number of combined coefficients
    combnels_ = combrstrtK;
    // Assertion: Counter of combined coefficients has to be
    // lower or equal than the number of all stage 2 coefficients
    assert(combrstrtK <= snels_+scenMatStart_[scen_+1]-scenMatStart_[scen_]);

    // Copy old objective function
    memcpy(combdobj_, sdobj_, sncol_*sizeof(double));
    // If there are any stochastic coefficients for objective 
    // function in current scenario they are substituted
    if(scenObjStart_[scen_+1]-scenObjStart_[scen_] != 0){
        for(int i=scenObjStart_[scen_]; i<scenObjStart_[scen_+1]; i++){
            combdobj_[mutiobj_[i]-mncol_] = mutdobj_[i];
        }
    }

    // Copy old LHS
    memcpy(combdrlo_, sdrlo_, snrow_*sizeof(double));
    // If there are any stochastic coefficients for LHS
    // in current scenario they are substituted
    if(scenRloStart_[scen_+1]-scenRloStart_[scen_] != 0){
        for(int i=scenRloStart_[scen_]; i<scenRloStart_[scen_+1]; i++){
            combdrlo_[mutirlo_[i]-mnrow_] = mutdrlo_[i];
        }
    }

    // Copy old RHS
    memcpy(combdrup_, sdrup_, snrow_*sizeof(double));
    // If there are any stochastic coefficients for RHS
    // in current scenario they are substituted
    if(scenRupStart_[scen_+1]-scenRupStart_[scen_] != 0){
        for(int i=scenRupStart_[scen_]; i<scenRupStart_[scen_+1]; i++){
            combdrup_[mutirup_[i]-mnrow_] = mutdrup_[i];
        }
    }

    // Copy old lower bounds
    memcpy(combdclo_, sdclo_, sncol_*sizeof(double));
    // If there are any stochastic coefficients for lower  
    // bounds in current scenario they are substituted
    if(scenCloStart_[scen_+1]-scenCloStart_[scen_] != 0){
        for(int i=scenCloStart_[scen_]; i<scenCloStart_[scen_+1]; i++){
            combdclo_[muticlo_[i]-mncol_] = mutdclo_[i];
        }
    }

    // Copy old upper bounds
    memcpy(combdcup_, sdcup_, sncol_*sizeof(double));
    // If there are any stochastic coefficients for upper 
    // bounds in current scenario they are substituted
    if(scenCupStart_[scen_+1]-scenCupStart_[scen_] != 0){
        for(int i=scenCupStart_[scen_]; i<scenCupStart_[scen_+1]; i++){
            combdcup_[muticup_[i]-mncol_] = mutdcup_[i];
        }
    }


    // ###################################################################################
    // End: buildProblem
    // ###################################################################################

}
int Benders::writeSolutionFileDetEq(const char * solpath, double time ){
    ofstream ofs;
    ofs.open(solpath);
    if(!ofs){
        cout << "Cannot open file!" << endl;
        return -1;
    }
    ofs << "Elapsed Time (sec): " << time << endl;
    ofs << "Objective Value: " << objDetEq_ << endl;
    ofs << "Master: " << endl;
    for(int i=0; i< mncol_; i++){
        ofs << bestSol_[i] << endl;
    }

    for(int scen_=0; scen_<nScen_;scen_++){
        ofs << "Subproblem " << scen_+1 << ":" << endl;
        for(int i=mncol_+scen_*sncol_; i<mncol_+(scen_+1)*sncol_; i++){
            ofs << bestSol_[i] << endl;
        }
    }
    ofs.close();

    return 0;
}

Benders::~Benders(){
    // Destructor

    if(osiMaster_)
        delete osiMaster_;
    if(osiSubproblem_)
        delete osiSubproblem_;
    if(osiDetEq_)
        delete osiDetEq_;
    if(mdobj_)
        delete [] mdobj_; 
    if(mdrlo_)
        delete [] mdrlo_; 
    if(mdrup_)
        delete [] mdrup_; 
    if(mdclo_)
        delete [] mdclo_;
    if(mdcup_)
        delete [] mdcup_; 
    if(mlen_)
        delete [] mlen_; 
    if(mdels_)
        delete [] mdels_; 
    if(miels_)
        delete [] miels_; 
    if(mrstrt_)
        delete [] mrstrt_; 
    if(bestSol_)
        delete [] bestSol_;
    if(sdobj_)
        delete [] sdobj_; 
    if(sdrlo_)
        delete [] sdrlo_; 
    if(sdrup_)
        delete [] sdrup_; 
    if(sdclo_)
        delete [] sdclo_; 
    if(sdcup_)
        delete [] sdcup_;
    if(constDrlo_)
        delete [] constDrlo_;
    if(constDrup_)
        delete [] constDrup_;
    if(sSt1dels_)
        delete [] sSt1dels_; 
    if(sSt1iels_)
        delete [] sSt1iels_; 
    if(sSt1rstrt_)
        delete [] sSt1rstrt_; 
    if(sSt2dels_)
        delete [] sSt2dels_; 
    if(sSt2iels_)
        delete [] sSt2iels_; 
    if(sSt2rstrt_)
        delete [] sSt2rstrt_; 
    if(snSt1len_)
        delete [] snSt1len_;
    if(snSt2len_)
        delete [] snSt2len_;
    if(scenProb_)
        delete [] scenProb_;
    if(mutiobj_)
        delete [] mutiobj_; 
    if(mutdobj_)
        delete [] mutdobj_; 
    if(scenObjStart_)
        delete [] scenObjStart_;
    if(mutirlo_)
        delete [] mutirlo_; 
    if(mutdrlo_)
        delete [] mutdrlo_; 
    if(scenRloStart_)
        delete [] scenRloStart_; 
    if(mutirup_)
        delete [] mutirup_; 
    if(mutdrup_)
        delete [] mutdrup_; 
    if(scenRupStart_)
        delete [] scenRupStart_;
    if(muticup_)
        delete [] muticup_;
    if(mutdcup_)
        delete [] mutdcup_;
    if(scenCupStart_)
        delete [] scenCupStart_;
    if(muticlo_)
        delete [] muticlo_;
    if(mutdclo_)	
        delete [] mutdclo_;
    if(scenCloStart_)
        delete [] scenCloStart_;
    if(mutSt1dels_)
        delete [] mutSt1dels_;
    if(mutSt1iels_)
        delete [] mutSt1iels_;
    if(mutSt1rstrt_)
        delete [] mutSt1rstrt_; 
    if(scenMatSt1Start_)
        delete [] scenMatSt1Start_; 
    if(mutnSt1len_)
        delete [] mutnSt1len_; 
    if(mutSt2dels_)
        delete [] mutSt2dels_; 
    if(mutSt2iels_)
        delete [] mutSt2iels_; 
    if(mutSt2rstrt_)
        delete [] mutSt2rstrt_; 
    if(scenMatSt2Start_)
        delete [] scenMatSt2Start_;
    if(mutnSt2len_)
        delete [] mutnSt2len_;
    if(combSt2dels_)
        delete [] combSt2dels_; 
    if(combSt2iels_)
        delete [] combSt2iels_;
    if(combSt2rstrt_)
        delete [] combSt2rstrt_; 
    if(combSt2len_)
        delete [] combSt2len_;  
    if(combdobj_)
        delete [] combdobj_;
    if(combdrup_)
        delete [] combdrup_;
    if(combdrlo_)
        delete [] combdrlo_;
    if(combdcup_)
        delete [] combdcup_;
    if(combdclo_)
        delete [] combdclo_;
    if(cutIndex_)
        delete [] cutIndex_;
    if(cutElements_)
        delete [] cutElements_;
    if(cutRowUB_)
        delete [] cutRowUB_;
    if(dTrans_)
        delete [] dTrans_;
    if(iTrans_)
        delete [] iTrans_;
    if(warmStart_ ){
        if(warmStartMaster_ && warmStartEmptyMaster_ == false)
            delete warmStartMaster_;
        for(int scen_=0; scen_<nScen_; scen_++){
            if(warmStartSub_[scen_] && warmStartEmptySub_[scen_]==false){
                delete warmStartSub_[scen_];
            }
        }
        if(warmStartSub_)
            delete [] warmStartSub_;
        if(warmStartEmptySub_)
            delete [] warmStartEmptySub_;
    }
    if(warmStartFol_){
        if(warmStartMaster_ && warmStartEmptyMaster_ == false)
            delete warmStartMaster_;
        if(warmStartSubFol_)
            delete [] warmStartSubFol_;
        if(warmStartEmptySub_)
            delete [] warmStartEmptySub_;
    }
    if(miInt_)
        delete [] miInt_;
    if(miBin_)
        delete [] miBin_;
    if(siInt_)
        delete [] siInt_;
    if(siBin_)
        delete [] siBin_;
    if(sdels_)
        delete [] sdels_;
    if(siels_)
        delete [] siels_;
    if(srstrt_)
        delete [] srstrt_;
    if(slen_)
        delete [] slen_;
    if(mutdels_)
        delete [] mutdels_;
    if(mutiels_)
        delete [] mutiels_;
    if(mutrstrt_)
        delete [] mutrstrt_;
    if(mutlen_)
        delete [] mutlen_;
    if(scenMatStart_)
        delete [] scenMatStart_;
    if(combdels_)
        delete [] combdels_;
    if(combiels_)
        delete [] combiels_;
    if(combrstrt_)
        delete [] combrstrt_;
    if(comblen_)
        delete [] comblen_;
}
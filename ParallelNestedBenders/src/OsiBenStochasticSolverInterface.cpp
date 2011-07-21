#include "OsiBenStochasticSolverInterface.hpp"

#include "CoinTime.hpp"
#include "glog/logging.h"

#include "OsiCpxSolverInterface.hpp"
#include "cplex.h"

OsiStochasticBendersTwoStageSolverInterface::OsiStochasticBendersTwoStageSolverInterface( OsiSolverInterface* osiSolver, SmiScnModel* smiModel )
: OsiStochasticSolverInterface(osiSolver,smiModel),benders(0)
{
}

OsiStochasticBendersTwoStageSolverInterface::OsiStochasticBendersTwoStageSolverInterface( OsiSolverInterface* osiSolver, SmiScnModel* smiModel, BendersParameter& parameter )
: OsiStochasticSolverInterface(osiSolver,smiModel),benders(0),params(parameter)
{
}

OsiStochasticBendersTwoStageSolverInterface::~OsiStochasticBendersTwoStageSolverInterface()
{
    if (benders)
        delete benders;
    benders = 0;
}

void OsiStochasticBendersTwoStageSolverInterface::initialSolve()
{
    //We think that we already have a filled SmiScnModel
    if (smi_model_){
        //Set params for Benders
        int maxIt = 100000;
        double relEps = params.getDoubleParam(BendersParameter::RELGAP);
        double absEps = params.getDoubleParam(BendersParameter::ABSGAP);
        bool useBenders = params.getIntParam(BendersParameter::BENDERS);
        bool useRegularized = params.getIntParam(BendersParameter::REGULARIZED);
        bool useRegularizedManhattan = params.getIntParam(BendersParameter::REGULARIZED_MANHATTAN);
        bool multi = params.getIntParam(BendersParameter::MULTI);
        bool knapsack = params.getIntParam(BendersParameter::KNAPSACK);
        bool trustRegion = params.getIntParam(BendersParameter::TRUSTREGION);
        int warmStart = params.getIntParam(BendersParameter::WARMSTART);// 0 is no warmstart, 1 is wsI, 2 is wsS
        bool barrier = params.getIntParam(BendersParameter::BARRIER);
        bool bunching = params.getIntParam(BendersParameter::BUNCHING);
        int bunchSize = params.getIntParam(BendersParameter::BUNCH_SIZE);
        bool integerBenders = params.getIntParam(BendersParameter::INTEGER_BENDERS);
        //	// Input parameters:
        //	// #1: Maximal Iterations
        //	// #2: Single/multi: s m
        //	// #3: Warmstart: +wsI +wsS -ws
        //	// #4: Knapsack Inequalities: +ki -ki
        //	// #5: TrustRegion: +tr -tr
        //	// #6: Benders or detEq: +ben -ben
        //	// #7: Problem Path
        //	// #8: Solution Path
        std::string modeStr("m");
        std::string regularizedStr("-reg");
        std::string knapsackStr("+ki");
        std::string trustRegionStr("+tr");
        std::string bendersStr("+ben");
        std::string warmStartStr("+wsI"); // We chose warmstart between iterations, but we can also choose warmstart between subproblems..
        std::string barrierStr("-br");
        std::string bunchingStr("-bunch");
        std::string integerBendersStr("-intBen");
        if (!useBenders)
            bendersStr = "-ben";
        if (useRegularized)
            regularizedStr = "+regD";
        if (useRegularizedManhattan)
            regularizedStr = "+regM";
        if (!multi)
            modeStr = "s";
        if (!knapsack)
            knapsackStr = "-ki";
        if (!trustRegion)
            trustRegionStr = "-tr";
        if (warmStart == 0)
            warmStartStr = "-ws";
        else if (warmStart == 1)
            warmStartStr = "+wsI";
        else if (warmStart == 2)
            warmStartStr = "+wsS";

        if (barrier)
            barrierStr = "+br";
        if (bunching)
            bunchingStr = "+bunch";
        if (integerBenders)
            integerBendersStr = "+intBen";

        if ( useRegularized && useRegularizedManhattan ){
            regularizedStr = "+regM";
            LOG(INFO) << "You have chosen two conflicting options, we go for regularized manhattan.";

        }
        if ( smi_model_->getCore()->getNumStages() > 2)
            LOG(FATAL) << "This solver is not capable of handling models with more than 2 stages.";

        // We want to store the time necessary to solve the problem
        double time = CoinCpuTime();
        benders = new Benders(modeStr.c_str(), warmStartStr.c_str(), knapsackStr.c_str(), trustRegionStr.c_str(), bendersStr.c_str(),regularizedStr.c_str(),barrierStr.c_str(),bunchingStr.c_str(),bunchSize,integerBendersStr.c_str());
        benders->copyMaster(smi_model_);
        benders->copySubmodel(smi_model_);
        if ( useRegularized || useRegularizedManhattan ){
            double regTime = CoinCpuTime();
            OsiCpxSolverInterface* osiCpxPtr = new OsiCpxSolverInterface();
            //CPXLPptr lpPtr = osiCpxPtr->getLpPtr();
            //CPXENVptr envPtr = osiCpxPtr->getEnvironmentPtr();
            // TODO: Set Relative MIPGAP with param via param setting function, if not given, do not change, so not apply.
            //CPXsetdblparam(envPtr,CPX_PARAM_EPGAP,0.025);
            std::pair<double,double*> evResult = smi_model_->solveEV(osiCpxPtr,1,false); // 1 for minimization
            delete osiCpxPtr;
            // copy initial values for regularized decomposition
            benders->setInitialSolution(evResult.second);
            delete evResult.second;
            LOG(INFO) << "Time computing initial solution (WallClock): " << CoinCpuTime()-regTime;
        }
        if (useBenders)
            benders->BendersSolve(absEps,relEps, maxIt );
        else
            benders->detEq();
        LOG(INFO) << "Solution time (WallClock): " << CoinCpuTime()-time << "s.";
        this->solTime = CoinCpuTime()-time;
    }
    else {
        std::cout << "SmiScnModel not initialized." << std::endl;
    }

}

double OsiStochasticBendersTwoStageSolverInterface::getObjValue(){
    if ( benders == 0) 
        return benders->infty_;
    else {
        if ( params.getIntParam(BendersParameter::BENDERS) == 1 )
            return benders->UB_; // Return upper bound, as this is the value of our solution
        else
            return benders->objDetEq_;
    }

}

int OsiStochasticBendersTwoStageSolverInterface::getNumIterations(){
    return benders->nIter_;
}
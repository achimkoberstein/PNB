// $Id$
#include <iostream>
#include <fstream>
#include <ctime>
#include <limits>

#include "OsiClpSolverInterface.hpp"
#include "OsiCpxSolverInterface.hpp"

#include "CoinFloatEqual.hpp"
#include "CoinTime.hpp"

#include "flopc.hpp"
#include "SmiScnModel.hpp"
#include "Benders.hpp"

#include <glog/logging.h>

using namespace flopc;

int main(int argc, char* argv[]) {

    google::InitGoogleLogging(argv[0]);
    FLAGS_minloglevel = 0;
    google::SetLogDestination(1, "log");
    google::LogToStderr();
    google::SetStderrLogging(0); // logging to std::err enabled for all logging levels >= 0
    
    MP_model mod(new OsiCpxSolverInterface());

    enum {numT=2};
    MP_stage T(numT);
    T.setName("T");
    
    // SETS
    
    enum {numKnoten=6};
    MP_set Knoten(numKnoten);
    Knoten.setName("Knoten");
    
    enum {numBehaelter=3};
    MP_subset<1> Behaelter(Knoten);
    Behaelter.setName("Behaelter");
    Behaelter.insert(1);
    Behaelter.insert(4);
    Behaelter.insert(5);
    
    enum {numIstBehaelter=0};
    MP_subset<1> IstBehaelter(Behaelter);
    IstBehaelter.setName("IstBehaelter");
    
    enum {numEinspeisung=1};
    MP_subset<1> Einspeisung(Knoten);
    Einspeisung.setName("Einspeisung");
    Einspeisung.insert(0);
    
    enum {numBezugspunkt=1};
    MP_subset<1> Bezugspunkt(Knoten);
    Bezugspunkt.setName("Bezugspunkt");
    Bezugspunkt.insert(0);
    
    enum {numStrang=8};
    MP_subset<2> Strang(Knoten,Knoten);
    Strang.setName("Strang");
    Strang.insert(0,1);
    Strang.insert(1,2);
    Strang.insert(2,3);
    Strang.insert(3,1);
    Strang.insert(0,4);
    Strang.insert(4,3);
    Strang.insert(4,5);
    Strang.insert(5,3);
    
    enum {numApp = 15};
    MP_set App(numApp);
    App.setName("App");
    
    // PARAMETER
    
    double valFixK[3] = {4000, 6000, 3000};
    MP_data fixK(&valFixK[0], Behaelter);
    fixK.setName("FixK");
    
    double valVarK[numT][numBehaelter] = {
        {30, 20, 10},
        {30, 20, 10}
    };
    MP_data varkK(&valVarK[0][0], T, Behaelter);
    varkK.setName("VarK");
    
    double valNachfrage[numT][numKnoten] = {
        {20, 10, 15, 30, 17, 5},
        {30, 20, 10, 40, 10, 10}
    };
    MP_data nachfrage(&valNachfrage[0][0], T, Knoten);
    nachfrage.setName("Nachfrage");
    
    double valInitH[numBezugspunkt] = {71.38};
    MP_data initH(&valInitH[0], Bezugspunkt);
    initH.setName("InitH");
    
    double valK[numStrang] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
    MP_data k(&valK[0], Strang);
    k.setName("k");
    
    double valD[numStrang] = {100, 100, 100, 100, 100, 100, 100, 100};
    MP_data d(&valD[0], Strang);
    d.setName("d");
    
    double valL[numStrang] = {100, 100, 100, 100, 100, 100, 100, 100};
    MP_data l(&valL[0], Strang);
    l.setName("l");
    
    const double konst1 = 0.0000063755;
    
    const double konst2 = 0.00520861;
    
    double valBarMax[numStrang] = {100, 100, 100, 100, 100, 100, 100, 100};
    MP_data barMax(&valBarMax[0], Strang);
    barMax.setName("barMax");
    
    double valE[numT][numEinspeisung] = { {120}, {100} };
    MP_data e(&valE[0][0], T, Einspeisung);
    e.setName("e");
    
    double valVmax[numBehaelter] = {100, 150, 140};
    MP_data vmax(&valVmax[0], Behaelter);
    vmax.setName("vmax");
    
    double valLinKoeffQ[numApp] = {-130, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
    MP_data linKoeffQ(&valLinKoeffQ[0], App);
    linKoeffQ.setName("linKoeffQ");
    
    double valLinKoeffH[numApp] = {-52.39, 0.744, 0.62, 0.496, 0.372, 0.248, 0.124, 0.031, 0.031, 0.124, 0.248, 0.372, 0.496, 0.62, 0.744};
    MP_data linKoeffH(&valLinKoeffH[0], App);
    linKoeffH.setName("linKoeffH");
    
    // VARIABLES
    
    MP_variable 
        qs(T, Behaelter),
        yq(Behaelter),
        q(T, Strang),
        h(T, Knoten),
        z(T, Strang, App),
        y(T, Strang, App);
    
    yq.binary();
    q.free();
    z.free();
    y.binary();
    
    forall(T*Strang*App.such_that(App <= App-1), z.upperLimit(T,Strang,App) = 0);
    forall(T*Strang*App.such_that(App <= App-2), y.upperLimit(T,Strang,App) = 0);
    
    // CONSTRAINTS
    
    MP_constraint bigM(Behaelter);
    
    
    
    mod.attach(mod.Solver);

    //mod.solve(MP_model::MINIMIZE);

    OsiStochasticBendersTwoStageSolverInterface benders(mod.Solver,mod.getSmi());
    benders.initialSolve();

    return 0;
}
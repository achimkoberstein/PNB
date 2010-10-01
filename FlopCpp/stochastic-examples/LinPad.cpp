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
    //MP_stage T(numT);
    MP_set T(numT);
    T.setName("T");
    
    //enum {numScen=1};
    //MP_scenario_set scen(numScen);
    //scen.setName("scen");
    
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
    MP_data varK(&valVarK[0][0], T, Behaelter);
    varK.setName("VarK");
    
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
    
    qs.setName("qs");
    yq.setName("yq");
    yq.binary();
    q.setName("q");
    q.free();
    h.setName("h");
    z.setName("z");
    z.free();
    y.setName("y");
    y.binary();
    
    forall(T*Strang*App.such_that(App <= App-1), z.upperLimit(T,Strang,App) = 0);
    forall(T*Strang*App.such_that(App <= App-2), y.upperLimit(T,Strang,App) = 0);
    
    // CONSTRAINTS
    
    MP_index S, E, M, N, I; // used for Strang
    
    MP_constraint bigM(Behaelter);
    bigM.setName("bigM");
    bigM(Behaelter).such_that(!IstBehaelter(Behaelter)) = sum(T, qs(T,Behaelter)) <= 100000 * yq(Behaelter);
    
    MP_constraint istBeh(IstBehaelter);
    istBeh.setName("istBeh");
    istBeh(IstBehaelter) = yq(IstBehaelter) == 1;
    
    MP_constraint masBilEinOhneBeh(T, Einspeisung);
    masBilEinOhneBeh.setName("masBilEinOhneBeh");
    masBilEinOhneBeh(T, Einspeisung(E).such_that(!Behaelter(E))) = sum(Strang(M,E), q(T,M,E)) - sum(Strang(E,N), q(T,E,N)) == nachfrage(T,E) - e(T,E);
    
    MP_constraint masBilEinMitBeh0(Knoten);
    masBilEinMitBeh0.setName("MasBilEinMitBeh0");
    masBilEinMitBeh0(Knoten).such_that(Einspeisung(Knoten) && Behaelter(Knoten))  = sum(Strang(M,Knoten), q(0,M,Knoten)) - sum(Strang(Knoten,N), q(0,Knoten,N) - qs(0,Knoten)) == nachfrage(0,Knoten) - e(0,Knoten);
    
    MP_constraint masBilEinMitBehT(T, Knoten);
    masBilEinMitBehT.setName("masBilEinMitBehT");
    masBilEinMitBehT(T, Knoten).such_that(T > 0 && Einspeisung(Knoten) && Behaelter(Knoten))  = sum(Strang(M,Knoten), q(T,M,Knoten)) - sum(Strang(Knoten,N), q(T,Knoten,N) - qs(T,Knoten) + qs(T-1,Knoten)) == nachfrage(T,Knoten) - e(T,Knoten);
    
    MP_constraint masBilMitBeh0(Behaelter);
    masBilMitBeh0.setName("masBilMitBeh0");
    masBilMitBeh0(Behaelter(I).such_that(!Einspeisung(Behaelter))) = sum(Strang(M,I), q(0,M,I)) - sum(Strang(I,N), q(0,I,N) - qs(0,I)) == nachfrage(0,I);
    
    MP_constraint masBilMitBehT(T, Behaelter);
    masBilMitBehT.setName("masBilMitBehT");
    masBilMitBehT(T,Behaelter).such_that(T > 0 && !Einspeisung(Behaelter)) = sum(Strang(M,I), q(T,M,I)) - sum(Strang(I,N), q(T,I,N) - qs(T,I) + qs(T-1,I)) == nachfrage(0,I);
    
    MP_constraint masBilOhneBeh(T, Knoten);
    masBilOhneBeh.setName("masBilOhneBeh");
    masBilOhneBeh(T, Knoten).such_that(!Einspeisung(Knoten) && !Behaelter(Knoten)) = sum(Strang(M,Knoten), q(T,M,Knoten)) - sum(Strang(Knoten,N), q(T,Knoten,N)) == nachfrage(T,Knoten);
    
    MP_constraint druckInit0(Bezugspunkt);
    druckInit0.setName("druckInit0");
    druckInit0(Bezugspunkt) = h(0,Bezugspunkt) == initH(Bezugspunkt);
    
    MP_constraint druckInit1(T, Behaelter);
    druckInit1.setName("druckInit1");
    druckInit1(T, Behaelter).such_that(T > 0) = h(T,Behaelter) - qs(T-1,Behaelter) <= 100 * (1 - yq(Behaelter));
    
    MP_constraint druckInit2(T, Behaelter);
    druckInit2.setName("druckInit2");
    druckInit2(T, Behaelter).such_that(T > 0) = -h(T,Behaelter) + qs(T-1,Behaelter) <= 100 * (1 - yq(Behaelter));
    
    MP_constraint druckInit3(T, Behaelter);
    druckInit3.setName("druckInit3");
    druckInit3(T, Behaelter).such_that(T > 0) = qs(T-1,Behaelter) <= 100 * yq(Behaelter);
    
    MP_constraint linKomb1(T, Strang);
    linKomb1.setName("linKomb1");
    linKomb1(T, Strang) = q(T,Strang) == linKoeffQ(0) + sum(App.such_that(App <= App-1), z(T,Strang,App));
    
    MP_constraint linKomb2(T, Strang);
    linKomb2.setName("linKomb2");
    linKomb2(T, Strang(M,N)) = linKoeffH(0) + sum(App.such_that(App <= App-1), linKoeffH(App+1) * z(T,Strang,App)) == h(T,M) - h(T,N);
    
    MP_constraint linBin1(T, Strang, App);
    linBin1.setName("linBin1");
    linBin1(T, Strang(M,N), App).such_that(App <= App-2) = z(T,Strang,App) >= linKoeffQ(App+1) * y(T,Strang,App);
    
    MP_constraint linBin2(T, Strang, App);
    linBin2.setName("linBin2");
    linBin2(T, Strang(M,N), App).such_that((App >= 1) && (App <= App-1)) = z(T,Strang,App) <= linKoeffQ(App+1) * y(T,Strang,App-1);
    
    //MP_constraint linBin3(T, Strang);
    //linBin3.setName("linBin3");
    //linBin3(T, Strang) = z(T,Strang,App-1) >= 0;
    forall(T*Strang, z.lowerLimit(T,Strang,App.last()) = 0);
    
    MP_constraint linBin4(T, Strang);
    linBin4.setName("linBin4");
    linBin4(T, Strang) = z(T,Strang,0) <= linKoeffQ(1);
    
    MP_constraint lub0(Behaelter);
    lub0.setName("lub0");
    lub0(Behaelter) = qs(0,Behaelter) <= sum(Einspeisung, e(0,Einspeisung));
    
    MP_constraint lub(T, Behaelter);
    lub.setName("lub");
    MP_index behAlias;
    lub(T, Behaelter).such_that(T >= 1) = qs(T,Behaelter) <= sum(Einspeisung, e(T,Einspeisung)) + sum(Behaelter(behAlias), qs(T-1,behAlias));
    
    MP_constraint lVmax(T, Behaelter);
    lVmax.setName("lVmax");
    lVmax(T, Behaelter) = qs(T,Behaelter) <= vmax(Behaelter);
    
    
    // OBJECTIVE
    
    mod.setObjective(sum(Behaelter, fixK(Behaelter) * yq(Behaelter) + sum(T, varK(T,Behaelter) * qs(T,Behaelter))));
    
    // SOLVE    
    
    mod.attach(mod.Solver);

    mod.solve(MP_model::MINIMIZE);

    //OsiStochasticBendersTwoStageSolverInterface benders(mod.Solver,mod.getSmi());
    //benders.initialSolve();

    return 0;
}
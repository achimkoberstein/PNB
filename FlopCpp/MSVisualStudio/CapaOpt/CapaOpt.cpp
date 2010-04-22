// $Id$
#include <iostream>
#include <ctime>
#include <limits>

#include "OsiClpSolverInterface.hpp"
#include "CoinFloatEqual.hpp"

#include "flopc.hpp"
#include "OsiStochasticSolverInterface.hpp"

using namespace flopc;

int main() {

    MP_model capa(new OsiClpSolverInterface);

    // ----------
    // Mengen
    // ----------

    // Produktionslinien
    enum {L1, L2, numL};
    MP_set L(numL);

    // Kapazitätsstufen
    enum  {S1, S2, S3, S4, numS};
    MP_set S(numS);

    // Märkte
    enum {M1, M2, numM};
    MP_set M(numM);

    // Produkttypen
    enum {K1, K2, numK};
    MP_set K(numK);

    // Linien + Kapazitätsstufen
    MP_subset<2> LS(L,S);
    LS.insert(L1, S1);
    LS.insert(L1, S2);
    LS.insert(L2, S3);
    LS.insert(L2, S4);

    // Kapazitätsstufen + Märkte
    MP_subset<2> SM(S,M);
    SM.insert(S1, M1);
    SM.insert(S2, M1);
    SM.insert(S3, M1);
    SM.insert(S4, M1);
    SM.insert(S1, M2);
    SM.insert(S2, M2);
    SM.insert(S3, M2);
    SM.insert(S4, M2);

    // ----------
    // Variablen
    // ----------

    MP_variable xLS(LS, K); // Fluss (Produktion)
    MP_variable xSM(SM, K); // Fluss (Markt)
    MP_variable v(L); // Produktionsentscheidung
    MP_variable yKA(L, S); // Kapazitätsentscheidung
    MP_variable yFL(L, K); // Flexibilitätsentscheidung
    MP_variable z(M, K); // Fehlmenge

    // ----------
    // Parameter
    // ----------

    // Flexibilitätskosten
    MP_data cFL(L, K);
    cFL(L1,K1) = 1;
    cFL(L1,K2) = 1;
    cFL(L2,K1) = 1;
    cFL(L2,K2) = 1;

    // Kapazitätskosten
    MP_data cKA(L, S);
    cKA(L1,S1) = 1;
    cKA(L1,S2) = 1;
    cKA(L1,S3) = 1;
    cKA(L1,S4) = 1;
    cKA(L2,S1) = 1;
    cKA(L2,S2) = 1;
    cKA(L2,S3) = 1;
    cKA(L2,S4) = 1;

    // Fehlmengenkosten
    MP_data h(M, K);
    h(M1,K1) = 10;
    h(M1,K2) = 10;
    h(M2,K1) = 10;
    h(M2,K2) = 10;

    // Kapazitäten
    MP_data m(S);
    m(S1) = 100;
    m(S2) = 100;
    m(S3) = 100;
    m(S4) = 100;

    // Maximale Kapazitäten
    MP_data mMax;
    mMax() = 100;

    // Produktionskosten
    MP_data qProd(S, K);
    qProd(S1,K1) = 1;
    qProd(S1,K2) = 1;
    qProd(S2,K1) = 1;
    qProd(S2,K2) = 1;
    qProd(S3,K1) = 1;
    qProd(S3,K2) = 1;
    qProd(S4,K1) = 1;
    qProd(S4,K2) = 1;

    // Transportkosten
    MP_data qTrans(L, M, K);
    qTrans(L1,M1,K1) = 1;
    qTrans(L1,M1,K2) = 1;
    qTrans(L1,M2,K1) = 1;
    qTrans(L1,M2,K2) = 1;
    qTrans(L2,M1,K1) = 1;
    qTrans(L2,M1,K2) = 1;
    qTrans(L2,M2,K1) = 1;
    qTrans(L2,M2,K2) = 1;

    // Kapazitätsbedarfe
    MP_data r(K, L);
    r(K1,L1) = 1;
    r(K1,L2) = 1;
    r(K2,L1) = 1;
    r(K2,L2) = 1;

    // Flexibilitätsvorgabe
    MP_data wFLS(K, L);
    wFLS(K1,L1) = 1;
    wFLS(K1,L2) = 1;
    wFLS(K2,L1) = 1;
    wFLS(K2,L2) = 1;


    // ----------
    // Stochastischer Teil
    // ----------

    // Stufen
    enum {numStage=2};
    MP_stage T(numStage);

    // Szenarien
    enum {numScen=2};
    MP_scenario_set scen(numScen);

    // Bedarfe
    double demand[numStage-1][numM][numK][numScen] =
    {//T
        {//M
            {//K
                {40, 60}, {40, 60}
            },
            {//K
                {40, 60}, {40, 60}
            },
        }
    };
    MP_random_data d(&demand[0][0][0][0],T,M,K);

    // ----------
    // Restriktionen
    // ----------

    MP_constraint 
        con33(L),
        con34(L),
        con35(L),
        con36(L,K),
        
        con39(T,S,K),
        con40(T,M,K),
        con41(T,L,K),
        con42(T,L,S);


    // Stage 0
    
    con33(L) = sum(LS(L,S), yKA(L,S)) == v(L);

    con34(L) = sum(K, yFL(L,K)) <= K.size() * v(L);

    con35(L) = sum(K, yFL(L,K)) >= v(L);

    con36(L,K) = yFL(L,K) <= wFLS(L,K);
    
    
    // Stage 1
    
    con39(T+1,S,K) = sum(LS(L,S), xLS(LS(L,S),K)) == sum(SM, xSM(SM,K));
    
    con40(T+1,M,K) = sum(S, xSM(SM(S,M),K) + z(M,K)) >= d(T+1,M,K);

    con41(T+1,L,K) = sum(LS(L,S), xLS(LS(LS),K)) <= mMax() * yFL(L,K);
    
    con42(T+1,L,S) = sum(K, r(K,L) * xLS(LS(L,S),K)) <= m(S) * yKA(L,S);
    


    // ----------
    // Optimierung
    // ----------

    capa.setObjective(
        sum(L, sum(K, cFL(L, K) * yFL(L, K)))
      + sum(L, sum(LS(L,S), cKA(L, S) * yKA(L, S)))
    );
    capa.attach(capa.Solver);
    capa.solve(MP_model::MINIMIZE);

    cout << "\n Finished.";

    return 0;
}
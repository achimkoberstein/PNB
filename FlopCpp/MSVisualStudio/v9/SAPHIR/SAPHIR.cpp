// $Id$
#include <iostream>
#include <fstream>
#include <ctime>
#include <limits>

#include "OsiClpSolverInterface.hpp"
#include "OsiCbcSolverInterface.hpp"
#include "OsiCpxSolverInterface.hpp"

#include "CoinFloatEqual.hpp"
#include "CoinTime.hpp"

#include "flopc.hpp"
#include "SmiScnModel.hpp"
#include "Benders.hpp"

using namespace flopc;

int main(int argc, char* argv[]) {

    MP_model saphir(new OsiCpxSolverInterface());
    
    // Stages
    enum {numST=2};
    MP_stage ST(numST);
    
    // Scenarios
    enum {numR=1};
    MP_scenario_set R(numR);
    
#pragma region sets

    // Number of time periods
    enum {numT=365};
    MP_set T(numT);
    
    // Months
    enum {numM=12};
    MP_set M(numM);
    
    // To which Month belongs each Time Period
    int daysToMonth[numM+1] = {0, 32, 62, 93, 124, 152, 183, 213, 244, 274, 305, 336, 366}; // period i goes from daysToMonth[i] to daysToMonth[i+1]
    MP_subset<2> TM(T,M);
    for (int m = 0; m < 12; m++) {
        for (int t = daysToMonth[m]; t < daysToMonth[m+1]; t++) {
            TM.insert(t,m);
        }
    }

    // Nodes (virtual trading points, national entry points)
    enum {Entry_2, EE_Speicher, Exit_1, Entry_1, NetConnect, numN};
    MP_set N(numN);
    MP_set N2(numN);
    
    //---> purchase
    
    // Baseload Contracts
    enum {EON_RG, Shell, numBC};
    MP_set BC(numBC);
    
    // Connections between Baseload Contracts and Nodes
    MP_subset<2> NBC(BC,N);
    NBC.insert(EON_RG, Entry_1);
    NBC.insert(Shell, Entry_2);
    
    // Months on which Baseload Contracts are available
    MP_subset<2> MBC(BC,M);
    forall(BC*M, MBC.insert(BC,M));
    
    // Winter Months Baseload Contracts
    MP_subset<2> WMBC(BC,M);
    for (int i = 1; i <= 5; i++) WMBC.insert(EON_RG, i);
    for (int i = 1; i <= 5; i++) WMBC.insert(Shell, i);
    
    // Summer Months Baseload Contracts
    MP_subset<2> SMBC(BC,M);
    for (int i = 6; i <= 11; i++) WMBC.insert(EON_RG, i);
    for (int i = 6; i <= 11; i++) WMBC.insert(Shell, i);
    
    // Open Contracts
    enum {numOC};
    MP_set OC(numOC);
    
    // Connections between Open Contracts and Nodes
    MP_subset<2> NOC(OC,N);
    
    // Time perionds when Open Contracts are available
    MP_subset<2> TOC(OC,T);
    
    //<--- purchase
    
    //---> storage
    
    // Storages
    enum {EpeX, Slack, numS};
    MP_set S(numS);
    
    // Connections between Storages and Nodes
    MP_subset<2> NS(S,N);
    NS.insert(EpeX, EE_Speicher);
    NS.insert(Slack, EE_Speicher);
    
    //<--- storage
    
    // Network
    // Connections between Entry-Exit Points
    MP_subset<2> L(N,N);
    L.insert(Entry_1, NetConnect);
    L.insert(Entry_2, NetConnect);
    L.insert(Exit_1, NetConnect);
    L.insert(EE_Speicher, NetConnect);
        
    // Marketarea
    // Connections that introduce Pipes (From Entry-Exit Point to Market Areas)
    MP_subset<2> LMA(N,N);
    LMA.insert(Entry_1, NetConnect);
    LMA.insert(Entry_2, NetConnect);
    LMA.insert(Exit_1, NetConnect);
    LMA.insert(EE_Speicher, NetConnect);
    
    //---> Sale
    
    // Supply Contracts
    enum {Lecker_Gas, COLOGNE_NATURE, numSC};
    MP_set SC(numSC);
    
    // Connections between Supply Contracts and Nodes
    MP_subset<2> NSC(SC,N);
    NSC.insert(Lecker_Gas, Exit_1);
    NSC.insert(COLOGNE_NATURE, Exit_1);

#pragma endregion sets

#pragma region parameters

    //---> Time (timeperiods and months defined above)
    
    // Storage Condition (0 or 1 to set if storage must be filled by the end of optimization)
    const int STCOND = 1;
    
    // Quarters (every 3 Months)
    double valMQ[numM] = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4};
    MP_data MQ(&valMQ[0], M);

    // Number of hours in each Month
    double valDELTAM[numM] = { 744, 720, 744, 744, 672, 744, 720, 744, 720, 744, 744, 720};
    MP_data DELTAM(&valDELTAM[0], M);
    
    // Number of hours in each Time Period
    MP_data DELTAT(T);
    forall(T, DELTAT(T) = 24);
    
    //<--- Time
    
    // Szenarios    
    // Probabilities of Stochastic Szenarios
    double valPR[numR] = {1.0};
    MP_data PR(&valPR[0], R);
    
    //---> Purchase
    
    // Type of Baseload Contract(Month,Quarter,Year)
    double valBCTYP[numBC] = {2, 2};
    MP_data BCTYP(&valBCTYP[0], BC);
    
    // Energy prices
    MP_data EPRICEBC(MBC);
    forall(M, EPRICEBC(EON_RG,M) = 0.02368);
    forall(M, EPRICEBC(Shell,M)  = 0.01791);
    
    // Minimal Energy per Month for every Baseload Contract
    MP_data EMINMBC(MBC);
    forall(M, EMINMBC(EON_RG,M) = 4711);
    forall(M, EMINMBC(Shell,M)  = 0);
    
    // Maximal Energy per Month for every Baseload Contract
    MP_data EMAXMBC(MBC);
    //forall(M, EMAXMBC(EON_RG,M) = 10000000000);
    //forall(M, EMAXMBC(Shell,M)  = 5000000000);    
    forall(M, EMAXMBC(EON_RG,M) = 2000000000);
    forall(M, EMAXMBC(Shell,M)  = 2000000000);    
    
    // Minimal Energy for each Baseload Contract
    double valEMINBC[numBC] = {4711, 0};
    MP_data EMINBC(&valEMINBC[0], BC);
    
    // Maximal Energy for each Baseload Contract
    //double valEMAXBC[numBC] = {10000000000, 5000000000};
    double valEMAXBC[numBC] = {2000000000, 2000000000};
    MP_data EMAXBC(&valEMAXBC[0], BC);
    
    // Minimal Power per Month for every Baseload Contract
    MP_data PMINBC(MBC);
    forall(M, PMINBC(EON_RG,M) = 0);
    forall(M, PMINBC(Shell,M)  = 0); 
    
    // Maximal Power per Month for every Baseload Contract
    MP_data PMAXBC(MBC);
    forall(M, PMINBC(EON_RG,M) = 10000000);
    forall(M, PMINBC(Shell,M)  = 10000000); 
    
    // Minimal Energy on Summer
    double valEMINSOMMER[numBC] = {0, 1100000000};
    MP_data EMINSOMMER(&valEMINSOMMER[0], BC);
    
    // Maximal Energy on Winter
    double valEMAXWINTER[numBC] = {10000000000, 1100000000};
    MP_data EMAXWINTER(&valEMAXWINTER[0], BC);

    // Energy Prices for Open Contracts
    MP_data EPRICEOC(TOC);
    
    // Price of Maximal power level purchased via Open Contracts
    MP_data PPRICEOC(OC);
    
    // Minimal Energy for each Open Contract
    MP_data EMINOC(OC);
    
    // Maximal Energy for each Open Contract
    MP_data EMAXOC(OC);
    
    // Minimal Power for each Open Contract
    MP_data PMINOC(TOC);
    
    // Maximal Power for each Open Contract
    MP_data PMAXOC(TOC);
    
    //<--- Purchase
    
    //---> Storage
    
    // Type of Storage (Slack, Underground Storage)
    double valSTYP[numS] = {1, 0};
    MP_data STYP(&valSTYP[0], S);
    
    // Storage Fixed Costs
    double valSCF[numS] = {1000, 10000};
    MP_data SCF(&valSCF[0], S);
    
    // Costs of Storage Volumes
    double valSCV[numS] = {0.05, 1};
    MP_data SCV(&valSCV[0], S);
    
    // Injection Costs
    double valPPRICEINJ[numS] = {0, 1};
    MP_data PPRICEINJ(&valPPRICEINJ[0], S);
    
    // Extraction Costs
    double valPPRICEEXT[numS] = {0, 1};
    MP_data PPRICEEXT(&valPPRICEEXT[0], S);
    
    // Compression Costs
    double valSCC[numS] = {0.0001, 0.001};
    MP_data SCC(&valSCC[0], S);    
    
    // Minimal Storage Energy for each Time Period
    MP_data EMINST(S,T);
    forall(S*T, EMINST(S,T) = 0);
    
    // Maximal Storage Energy for each Time Period
    MP_data EMAXST(S,T);
    //forall(T, EMAXST(EpeX,T) = 12500000000);
    //forall(T, EMAXST(Slack,T) = 10000000000);
    forall(T, EMAXST(EpeX,T) = 2000000000);
    forall(T, EMAXST(Slack,T) = 2000000000);
    
    // Injection Power for each Time Period
    MP_data PMAXINJ(S,T);
    forall(T, PMAXINJ(EpeX,T) = 283467);
    forall(T, PMAXINJ(Slack,T) = 10000000);
    
    // Extraction Power for each Time Period
    MP_data PMAXEXT(S,T);
    forall(T, PMAXEXT(EpeX,T) = 287206);
    forall(T, PMAXEXT(Slack,T) = 10000000);
    
    // Relation between Volume and Extraction
    double valVZUAL[numS] = {1, 1};
    MP_data VZUAL(&valVZUAL[0], S);
    
    // Relation between Extraction and Injection
    double valALZUINJ[numS] = {1, 1};
    MP_data ALZUINJ(&valALZUINJ[0], S);
    
    //<--- Storage
    
    //---> Network
    
    // Entry Prices
    MP_data CENTRY(L);
    forall(L, CENTRY(L) = 2);
    
    // Exit Prices
    MP_data CEXIT(L);
    forall(L, CEXIT(L) = 2);
    
    //<--- Network
    
    //---> Sale
    
    // Supply Contracts Prices
    MP_data EPRICESC(SC,T);
    forall(SC*T, EPRICESC(SC,T) = 0.06);
    
    // Supply Contracts Demands
    double valLOAD_Lecker_Gas[numT] = {
        67191.9147702348,
        66271.1626592633,
        45134.3598839899,
        45442.8943067425,
        67191.9147702348,
        54425.1774076553,
        56633.1077650065,
        56051.5415814613,
        78046.7451962708,
        98872.8622523656,
        106114.6047613960,
        122295.4774421300,
        76819.0559575282,
        70126.6916172821,
        67191.9147702348,
        75624.2073249552,
        109950.9054666530,
        98872.8622523656,
        98872.8622523656,
        126677.2438197880,
        131184.7113254870,
        113928.4297135440,
        157927.1562327320,
        261584.8755332980,
        244810.0942039900,
        249721.7352103000,
        239812.5108492860,
        281247.3604971180,
        324639.5461042130,
        337414.8499907830,
        343216.3115153490,
        340410.6874326990,
        362446.9487981030,
        360493.1387798940,
        359445.9846428080,
        261584.8755332980,
        78046.7451962708,
        72232.2474999373,
        95469.2345116398,
        160494.1245636940,
        277078.0136818760,
        305426.4176033660,
        344986.0377061690,
        339433.7105582080,
        316363.1913731720,
        256914.3729962050,
        300298.2565597170,
        307076.2721717600,
        320623.8582339670,
        342301.6135205360,
        341366.5300781820,
        307076.2721717600,
        307076.2721717600,
        320623.8582339670,
        328418.9908478270,
        320623.8582339670,
        300298.2565597170,
        219111.6179054120,
        293039.8259147080,
        340410.6874326990,
        319231.2659211490,
        224377.0357079810,
        270606.1410292330,
        334218.6483450030,
        311849.6228378720,
        331971.5340214080,
        343216.3115153490,
        344110.9961894450,
        316363.1913731720,
        300298.2565597170,
        279177.3913213920,
        308696.6281890320,
        244810.0942039900,
        314887.1322435030,
        331971.5340214080,
        347496.9690993850,
        270606.1410292330,
        256914.3729962050,
        244810.0942039900,
        252143.3214920260,
        296729.4207392080,
        333106.9709391790,
        293039.8259147080,
        263880.9306476140,
        302037.5496389890,
        335306.9379586140,
        349844.1882155220,
        325925.1595335180,
        285298.2403460140,
        152851.2027648120,
        302037.5496389890,
        229588.0742678190,
        307076.2721717600,
        339433.7105582080,
        345841.8039535890,
        352037.2094885550,
        355997.1161774870,
        369299.2729396520,
        374205.8800280620,
        371279.2911587420,
        373810.5141314820,
        366604.5759660450,
        340410.6874326990,
        334218.6483450030,
        296729.4207392080,
        285298.2403460140,
        316363.1913731720,
        329627.9081047070,
        321989.2239892970,
        298528.9078512460,
        247277.0688667650,
        226989.9128645190,
        249721.7352103000,
        303746.8936138350,
        300298.2565597170,
        272792.0804696120,
        242321.6238426230,
        256914.3729962050,
        211131.9555959950,
        145399.6050274880,
        122295.4774421300,
        124470.3150096060,
        184239.6726270720,
        211131.9555959950,
        170926.5154061390,
        173569.3345413600,
        268392.0269999540,
        298528.9078512460,
        308696.6281890320,
        321989.2239892970,
        321989.2239892970,
        336372.2134695310,
        319231.2659211490,
        320623.8582339670,
        344110.9961894450,
        361975.4047379660,
        357781.3356504990,
        336372.2134695310,
        268392.0269999540,
        281247.3604971180,
        281247.3604971180,
        259262.5012548400,
        283287.7042341810,
        232170.4406907860,
        208455.2205298450,
        263880.9306476140,
        294899.7373590060,
        157927.1562327320,
        178887.3954830120,
        203084.1142571130,
        221750.5556995210,
        277078.0136818760,
        242321.6238426230,
        247277.0688667650,
        259262.5012548400,
        181559.9379264160,
        208455.2205298450,
        178887.3954830120,
        109950.9054666530,
        104250.0600280940,
        219111.6179054120,
        259262.5012548400,
        261584.8755332980,
        310287.6757048110,
        281247.3604971180,
        208455.2205298450,
        302037.5496389890,
        263880.9306476140,
        307076.2721717600,
        293039.8259147080,
        244810.0942039900,
        203084.1142571130,
        254541.0971523750,
        261584.8755332980,
        263880.9306476140,
        165679.9466183800,
        219111.6179054120,
        270606.1410292330,
        293039.8259147080,
        298528.9078512460,
        313382.6946838650,
        307076.2721717600,
        311849.6228378720,
        272792.0804696120,
        242321.6238426230,
        232170.4406907860,
        249721.7352103000,
        329627.9081047070,
        319231.2659211490,
        277078.0136818760,
        186925.2298590810,
        150344.5674626700,
        160494.1245636940,
        145399.6050274880,
        120153.3369460570,
        219111.6179054120,
        189615.2369052730,
        178887.3954830120,
        128915.6098629040,
        272792.0804696120,
        229588.0742678190,
        197698.2433131720,
        303746.8936138350,
        302037.5496389890,
        317811.1417597480,
        293039.8259147080,
        254541.0971523750,
        224377.0357079810,
        181559.9379264160,
        173569.3345413600,
        195003.1113305350,
        102421.5057389790,
        259262.5012548400,
        293039.8259147080,
        277078.0136818760,
        128915.6098629040,
        56051.5415814613,
        150344.5674626700,
        142963.4199287670,
        140552.7959657290,
        73331.2657278576,
        50865.2246069961,
        45442.8943067425,
        48822.5009590521,
        50490.8533096848,
        44720.0480982772,
        71164.2222488134,
        97152.9125396332,
        87592.4644466204,
        195003.1113305350,
        102421.5057389790,
        109950.9054666530,
        90635.3149550486,
        86124.4348703972,
        122295.4774421300,
        145399.6050274880,
        170926.5154061390,
        115969.3283618570,
        48526.5756066425,
        49450.8575742511,
        80602.2979085071,
        108014.9612451540,
        170926.5154061390,
        142963.4199287670,
        98872.8622523656,
        73331.2657278576,
        51658.2095648888,
        67191.9147702348,
        46546.9837551670,
        71164.2222488134,
        160494.1245636940,
        135812.0775702760,
        138168.7052473550,
        165679.9466183800,
        109950.9054666530,
        93821.7957571387,
        102421.5057389790,
        115969.3283618570,
        92210.5252976897,
        69119.1454478823,
        106114.6047613960,
        69119.1454478823,
        57858.6365598703,
        73331.2657278576,
        44720.0480982772,
        44248.2378802023,
        45285.2270921480,
        43856.0767556478,
        57858.6365598703,
        78046.7451962708,
        58503.6933263141,
        51254.1905782150,
        52077.7486182998,
        43686.0497418917,
        43264.2258231411,
        62074.1135588399,
        69119.1454478823,
        61312.1451188041,
        47707.1040524994,
        52077.7486182998,
        69119.1454478823,
        73331.2657278576,
        54425.1774076553,
        44593.9089010313,
        42789.7118137959,
        42469.5312854312,
        43686.0497418917,
        52077.7486182998,
        48242.2896865743,
        44473.3713653336,
        42249.1705008173,
        44042.9899128898,
        49784.1057671056,
        47707.1040524994,
        50490.8533096848,
        60574.6804722202,
        48822.5009590521,
        46982.2876076530,
        46342.7120417989,
        43326.2149893373,
        43391.3399960404,
        47213.9720848762,
        43149.1253734308,
        43391.3399960404,
        43686.0497418917,
        44042.9899128898,
        56051.5415814613,
        65378.2622419505,
        78046.7451962708,
        62074.1135588399,
        50130.6271998825,
        43686.0497418917,
        46760.0683661204,
        52077.7486182998,
        54425.1774076553,
        98872.8622523656,
        80602.2979085071,
        73331.2657278576,
        46760.0683661204,
        50865.2246069961,
        68141.0631073789,
        62074.1135588399,
        73331.2657278576,
        81930.9348919751,
        67191.9147702348,
        50130.6271998825,
        45134.3598839899,
        47213.9720848762,
        55490.0528750246,
        53434.2835442756,
        56051.5415814613,
        50490.8533096848,
        48242.2896865743,
        54948.1075090443,
        62861.1483370475,
        55490.0528750246,
        54425.1774076553,
        51658.2095648888,
        89096.0207673478,
        92210.5252976897,
        109950.9054666530,
        98872.8622523656,
        124470.3150096060,
        122295.4774421300,
        122295.4774421300,
        113928.4297135440,
        109950.9054666530,
        90635.3149550486,
        80602.2979085071,
        95469.2345116398,
        90635.3149550486,
        79307.6932617448,
        87592.4644466204,
        118044.4509418690,
        138168.7052473550,
        128915.6098629040,
        93821.7957571387,
        98872.8622523656,
        46146.9402669926,
        43531.5572889763,
        42866.1564608687,
        50130.6271998825,
        51658.2095648888,
        46982.2876076530,
        70126.6916172821
    };
    MP_random_data LOAD(SC,T,R);
    double dnull = 0.0;
    ScenarioRandomVariable* nullSRV = new ScenarioRandomVariable(&dnull, 1);
    for (int t = 0; t < numT; t++) {
        LOAD(Lecker_Gas,t,0) = new ScenarioRandomVariable(&valLOAD_Lecker_Gas[t],1);
        LOAD(COLOGNE_NATURE,t,0) = nullSRV;
    }
    
    //<--- Sale
    
#pragma endregion parameters

#pragma region variables

    //---> Purchase
    
    MP_variable pbcp(MBC);			//Baseload Contracts Power to be ordered
    pbcp.lowerLimit = 0;
    
    MP_variable pbcu(ST,MBC,R);    	//Baseload Contracts Usage
    pbcu.lowerLimit = 0;
    
    MP_variable pmaxbc(BC);			//Maximal Baseload Contracts Power
    pmaxbc.lowerLimit = 0;
    
    MP_variable pminbc(BC);			//Minimal Baseload Contracts Power
    pminbc.lowerLimit = 0;
    
    MP_variable poc(ST,TOC,R);	    //Open Contracts Usage
    poc.lowerLimit = 0;
    
    MP_variable pmaxoc(OC);			//Maximal Open Contracts Power
    pmaxoc.lowerLimit = 0;
    
    MP_variable gasbezugskostengesamt(ST,R);	//Check Variable for Purchase Contacts Costs
    gasbezugskostengesamt.lowerLimit = 0;
    
    MP_variable bc_use(BC);			//Indicator if Baseload Contracts will be used in Optimization
    bc_use.binary();
    
    MP_variable oc_use(OC);			//Indicator if Open Contracts will be used in Optimization
    oc_use.binary();
    
    //<--- Purchase
    
    //---> Storage
    
    MP_variable es(ST,S,T,R);	//Amount of Energy stored in Storages (fill level)
    es.lowerLimit = 0;
    
    MP_variable emaxs(S);		//Maximal Amount of Energy stored in Storage
    emaxs.lowerLimit = 0;
    
    MP_variable pinj(ST,S,T,R);	//Injection Power for every Time Period
    pinj.lowerLimit = 0;
    
    MP_variable pext(ST,S,T,R);	//Extraction Power for every Time Period
    pext.lowerLimit = 0;
    
    MP_variable pmaxinj(S);		//Maximal used Injection Power by each Storage
    pmaxinj.lowerLimit = 0;
    
    MP_variable pmaxext(S);		//Maximal used Extraction Power by each Storage
    pmaxext.lowerLimit = 0;
    
    MP_variable speicherkostengesamt(ST,R);	//Check Variable for Storage Costs
    speicherkostengesamt.lowerLimit = 0;
    
    MP_variable s_use(S);		//Indicator if Storage will be used in Optimization
    s_use.binary();
    
    //<--- Storage
    
    //---> Network
    MP_variable pl(ST,L,T,R);   //Power Level on Links (can be negative)
    
    MP_variable pminl(L);		//Minimal Power Level on Links
    pminl.upperLimit = 0;
    
    MP_variable pmaxl(L);		//Maximal Power Level on Links
    pmaxl.lowerLimit = 0;
    
    MP_variable netzkostengesamt(ST,R);	//Check Variable for Network Costs
    netzkostengesamt.lowerLimit = 0;
    
    //<--- Network
    
    //---> Sale
    
    MP_variable gain(ST,R);					
    MP_variable revenues(ST,SC,T,R);
    
    //<--- Sale
    
    //---> Control
    
    MP_variable bilanz(ST,T,R);
    
    //<--- Control

#pragma endregion variables

#pragma region constraints

    MP_constraint 
        KostenGasbezug(ST,R),
        SpeicherkostenGesamt(ST,R),
        NetzkostenGesamt(ST,R);
    
    KostenGasbezug(ST+1,R) = 
        gasbezugskostengesamt(ST+1,R) ==   sum(MBC(BC,M), EPRICEBC(MBC) * DELTAM(M) * pbcp(MBC)) 
                                         + sum(R, PR(R) * sum(TOC(OC,T), EPRICEOC(TOC) * DELTAT(T) * poc(ST+1,OC,T,R) + PPRICEOC(OC) * pmaxoc(OC)));
    
    SpeicherkostenGesamt(ST+1,R) =
        speicherkostengesamt(ST+1,R) == sum(R, PR(R) * sum(S, SCF(S) + SCV(S) * emaxs(S) + PPRICEINJ(S) * pmaxinj(S) + PPRICEEXT(S) * pmaxext(S) + sum(T, SCC(S) * DELTAT(T) * pinj(ST+1,S,T,R))));
        
    NetzkostenGesamt(ST+1,R) =
        netzkostengesamt(ST+1,R) == sum(R, PR(R) * sum(LMA, CENTRY(LMA) * pmaxl(LMA) - CEXIT(LMA) * pminl(LMA)));

    //---> Purchase
    
    MP_constraint
        EMinBaseLoad1(MBC),
        EMaxBaseLoad1(MBC),
        EMinBaseLoad2(BC),
        EMaxBaseLoad2(BC),
        EMaxWinter(BC),
        EMinSommer(BC),
        PMinBaseLoadLimit(MBC),
        PMaxBaseLoadLimit(MBC),
        PMinBaseLoad(MBC),
        PMaxBaseLoad(MBC),
        BASELOADYEAR1(BC),
        BASELOADQUATER(MBC),
        PUseNotMoreThanExistBaseLoad(ST,MBC,R),
        EMinOpenContract(ST,OC,R),
        EMaxOpenContract(ST,OC,R),
        PMaxOpenConstact(ST,TOC,R),
        PMinOpenContractLimit(ST,OC,T,R),
        PMaxOpenContractLimit(ST,OC,T,R);
    
    EMinBaseLoad1(MBC(BC,M)) = EMINMBC(MBC) * bc_use(BC) <= DELTAM(M) * pbcp(MBC);
    EMaxBaseLoad1(MBC(BC,M)) = DELTAM(M) * pbcp(MBC) <= EMAXMBC(MBC) * bc_use(BC);
    
    EMinBaseLoad2(BC) = EMINBC(BC) * bc_use(BC) <= sum(MBC(BC,M), DELTAM(M) * pbcp(MBC));
    EMaxBaseLoad2(BC) = sum(MBC(BC,M), DELTAM(M) * pbcp(MBC)) <= EMAXBC(BC) * bc_use(BC);
    
    EMaxWinter(BC) = sum(WMBC(BC,M), DELTAM(M) * pbcp(WMBC)) <= EMAXWINTER(BC);
    EMinSommer(BC) = EMINSOMMER(BC) * bc_use(BC) <= sum(SMBC(BC,M), DELTAM(M) * pbcp(SMBC));
    
    PMinBaseLoadLimit(MBC(BC,M)) = PMINBC(MBC) * bc_use(BC) <= pbcp(MBC);
    PMaxBaseLoadLimit(MBC(BC,M)) = pbcp(MBC) <= PMAXBC(MBC) * bc_use(BC);
    
    PMinBaseLoad(MBC(BC,M)) = pminbc(BC) <= pbcp(MBC);
    PMaxBaseLoad(MBC(BC,M)) = pbcp(MBC) <= pmaxbc(BC);
    
    BASELOADYEAR1(BC).such_that(BCTYP(BC) < 1) = pmaxbc(BC) <= pminbc(BC); // "== 0" geht nicht, "== 1" jedoch schon...
    BASELOADQUATER(MBC(BC,M)).such_that(M > 0 && BCTYP(BC) == 1 && MQ(M) == MQ(M-1)) = pbcp(MBC) == pbcp(MBC(BC,M-1));

    PUseNotMoreThanExistBaseLoad(ST+1,MBC(BC,M),R) = pbcu(ST+1,MBC,R) <= pbcp(MBC);
    
    EMinOpenContract(ST+1,OC,R) = EMINOC(OC) * oc_use(OC) <= sum(TOC(OC,T), DELTAT(T) * poc(ST+1,OC,T,R));
    EMaxOpenContract(ST+1,OC,R) = sum(TOC(OC,T), DELTAT(T) * poc(ST+1,OC,T,R)) <= EMAXOC(OC) * oc_use(OC);
    
    PMaxOpenConstact(ST+1,TOC(OC,T),R) = poc(ST+1,T,R) <= pmaxoc(OC);

    PMinOpenContractLimit(ST+1,OC,T,R) = PMINOC(OC,T) * oc_use(OC) <= poc(ST+1,OC,T,R);
    PMaxOpenContractLimit(ST+1,OC,T,R) = poc(ST+1,OC,T,R) <= PMAXOC(OC,T) * oc_use(OC);

    //<--- Purchase
    
    //---> Storage
    
    MP_constraint
        EBeginngingStorage(ST,R,S),
        EStorage(ST,S,T,R),
        EEndStorage(ST,R,S),
        EMinMaxStorage0a(ST,S,T,R),
        EMinMaxStorage0b(ST,S,T,R),
        EMinMaxStorage1(ST,S,T,R),
        EMinMaxStorage3(ST,S,T,R),
        EMinMaxStorage4(ST,S,T,R),
        PMaxInj(ST,S,T,R),
        PMaxExt(ST,S,T,R),
        PMaxInjConstant(ST,S,T,R),
        PMaxExtConstant(ST,S,T,R);
    
    // !(S == Slack) - Slack oder SlackStorage??? SlackStorage, wie es in MOPS Studio steht, gibt es garnicht
    EBeginngingStorage(ST+1,R,S).such_that(!(S == Slack)) = es(ST+1,S,0,R) == emaxs(S) + DELTAT(0) * (pinj(ST+1,S,0,R) - pext(ST+1,S,0,R));
    EStorage(ST+1,S,T,R) = es(ST+1,S,T,R) == es(ST+1,S,T-1,R) + DELTAT(T) * (pinj(ST+1,S,T,R) - pext(ST+1,S,T,R));
    EEndStorage(ST+1,R,S).such_that(!(S == Slack)) = es(ST+1,S,numT,R) >= emaxs(S) * STCOND;
    
    EMinMaxStorage0a(ST+1,S,T,R) = (0*es(ST+1,S,T,R)) <= es(ST+1,S,T,R) ; // wieso geht "0 <= ..." nicht?
    EMinMaxStorage0b(ST+1,S,T,R) =  es(ST+1,S,T,R) <= EMAXST(S,T);
    EMinMaxStorage1(ST+1,S,T,R) = es(ST+1,S,T,R) <= emaxs(S);
    EMinMaxStorage3(ST+1,S,T,R).such_that(!(S == Slack)) = es(ST+1,S,T,R) <= EMAXST(S,T) * s_use(S);
    EMinMaxStorage4(ST+1,S,T,R).such_that(!(S == Slack)) = es(ST+1,S,T,R) >= EMINST(S,T) * s_use(S);
    PMaxInj(ST+1,S,T,R) = pinj(ST+1,S,T,R) <= pmaxinj(S);
    PMaxExt(ST+1,S,T,R) = pext(ST+1,S,T,R) <= pmaxext(S);
    PMaxInjConstant(ST+1,S,T,R) = pinj(ST+1,S,T,R) <= PMAXINJ(S,T);
    PMaxExtConstant(ST+1,S,T,R) = pext(ST+1,S,T,R) <= PMAXEXT(S,T);    
    
    //<--- Storage
    
    //---> Network
    
    MP_constraint
        PMinNetwork(ST,L,T,R),
        PMaxNetwork(ST,L,T,R),
        Flow(ST,N,T,R);
    
    PMinNetwork(ST+1,L,T,R) = pminl(L) <= pl(ST+1,L,T,R);
    PMaxNetwork(ST,L,T,R) = pl(ST+1,L,T,R) <= pmaxl(L);
    
    Flow(ST+1,N,T,R) =
           sum(BC.such_that(NBC(BC,N)) * M.such_that(TM(T,M) && MBC(BC,M)), pbcu(BC,T,R)) 
        +  sum(OC.such_that(NOC(OC,N) && TOC(OC,T)), poc(ST+1,OC,T,R))
        +  sum(S.such_that(NS(S,N)),  pext(ST+1,S,T,R) - pinj(ST+1,S,T,R))
        +  sum(L(N2,N), pl(ST+1,L,T,R)) - sum(L(N,N2), pl(ST+1,L,T,R))
        == sum(SC.such_that(NSC(SC,N)), LOAD(N,T,R));
    
    //<--- Network
    
    //---> Sale
    
    MP_constraint
        Returns(ST,SC,T,R),
        Sales(ST,R);
    
    Returns(ST+1,SC,T,R) = revenues(ST+1,S,T,R) == EPRICESC(S,T) * DELTAT(T) * LOAD(S,T,R);
    
    Sales(ST+1,R) = gain(ST+1,R) == sum(SC*T, EPRICESC(S,T) * DELTAT(T) * LOAD(S,T,R));
    
    //<--- Sale
    
#pragma endregion constraints

#pragma region optimization

    saphir.setObjective(
        sum(R, PR(R) * (gain(ST+1,R) - gasbezugskostengesamt(ST+1,R) - speicherkostengesamt(ST+1,R) - netzkostengesamt(ST+1,R)))
    );
    
    saphir.attach(saphir.Solver);    
    saphir.solve(MP_model::MAXIMIZE);
    
    //OsiStochasticBendersTwoStageSolverInterface benders(saphir.Solver,saphir.getSmi());
    //benders.initialSolve();

#pragma endregion optimization

    return 0;
}
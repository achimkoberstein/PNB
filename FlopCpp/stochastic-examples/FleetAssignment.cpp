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

    // Anzahl Stufen
    enum { numST=2};
    MP_stage ST(numST);
    ST.setName("ST");

    // Anzahl Szenarien
    enum { numScen=10};
    MP_scenario_set scen(numScen);
    scen.setName("scen");

    // Airports (4 mal nötig für Variablen und Constraints)
    enum { FRA, HAM, MUC, DRS, HAJ, STR, IAD, CHI, BOS, PHL, CLT, CVG, numA};
    MP_set A(numA);
    A.setName("A");
    MP_set A2(numA);
    A2.setName("A2");
    MP_set A3(numA);
    A3.setName("A3");
    MP_set A4(numA);
    A4.setName("A4");       

    // Flugzeugtypen
    enum { A320, A340, A380, numT};
    MP_set T(numT);
    T.setName("T");

    // Entfernungen
    double distance[numA][numA] =
    {
        {0,413,300,388,282,157,6545,6964,5884,6334,7052,6995},
        {413,0,600,378,132,553,6469,6827,5814,6262,6982,6886},
        {300,600,0,340,480,194,6838,7264,6177,6627,7345,7292},
        {388,378,340,0,317,425,6821,7196,6163,6612,7332,7248},
        {282,132,480,317,0,421,6504,6882,5847,6295,7016,6932},
        {157,553,194,425,421,0,6652,7087,5990,6440,7158,7110},
        {6545,6469,6838,6821,6504,6652,0,931,663,216,516,632},
        {6964,6827,7264,7196,6882,7087,931,0,1381,1075,949,410},
        {5884,5814,6177,6163,5847,5990,663,1381,0,450,1169,1207},
        {6334,6262,6627,6612,6295,6440,216,1075,450,0,720,813},
        {7052,6982,7345,7332,7016,7158,516,949,1169,720,0,539},
        {6995,6886,7292,7248,6932,7110,632,410,1207,813,539,0}
    };
    MP_data dist(&distance[0][0], A, A2);
    dist.setName("dist");

    // Verkaufspreis für einen Flug
    MP_data p(A, A2);
    forall(A*A2, p(A,A2) = dist(A,A2) * 0.08);
    p.setName("p");

    // discountfactor
    MP_data dcf;
    dcf() = 1.0;
    dcf.setName("dcf");

    // Rabatt für Passagiere die nicht non-stop fliegen
    MP_data pd;
    pd() = 0.1;
    pd.setName("pd");

    // maximale Kapazität je Flugzeugtyp
    double maxCapacity[numT] = { 150, 250, 500 };
    MP_data m(&maxCapacity[0], T);
    m.setName("m");

    // Kerosinverbrauch je Flugzeugtyp
    double kerosinConsumption[numT] = { 6.75, 10, 17.5 };
    MP_data cons(&kerosinConsumption[0], T);
    cons.setName("cons");

    // Kosten pro KM je Flugzeugtyp
    double costPerKM[numT] = { 10.125, 11.25, 18 };
    MP_data cpm(&costPerKM[0], T);
    cpm.setName("cpm");

    // maximale Reichweite je Flugzeugtyp
    double maxRange[numT] = { 5500, 14000, 14000 };
    MP_data r(&maxRange[0], T);
    r.setName("r");

    // maximale Flüge je Flugzeugtyp
    double maxFlights[numT] = { 1e10, 1e10, 1e10};
    MP_data ub(&maxFlights[0], T);
    ub.setName("ub");

    // Nachfrage
    double stochasticDemand[numST-1][numA][numA][numScen] =
    {
     {
      {
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {1638, 1787, 2027, 1906, 1771, 1702, 1809, 1678, 1605, 1526},
       {1344, 715, 1022, 738, 1278, 1272, 767, 1191, 1145, 1051},
       {416, 336, 501, 467, 459, 415, 351, 574, 488, 455},
       {163, 111, 187, 193, 189, 149, 162, 138, 168, 153},
       {102, 117, 83, 106, 111, 82, 98, 111, 143, 105},
       {614, 523, 628, 423, 574, 561, 550, 627, 486, 476},
       {444, 608, 596, 337, 630, 574, 626, 487, 618, 594},
       {375, 238, 393, 440, 344, 313, 371, 292, 399, 319},
       {204, 275, 254, 185, 315, 343, 306, 267, 318, 346},
       {139, 206, 195, 237, 240, 217, 237, 209, 135, 188},
       {42, 54, 51, 39, 46, 45, 42, 67, 40, 42}
      },
      {
       {1680, 1076, 1385, 1924, 1810, 1971, 1761, 1342, 2094, 1657},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {1708, 1809, 1727, 1345, 1788, 1788, 2752, 2618, 1990, 1777},
       {88, 79, 66, 86, 81, 107, 84, 90, 60, 69},
       {10, 11, 8, 8, 10, 11, 12, 12, 10, 12},
       {1806, 2066, 1966, 1062, 1362, 1666, 1765, 1668, 1441, 1540},
       {54, 42, 39, 36, 27, 50, 48, 41, 32, 30},
       {47, 44, 51, 37, 57, 61, 51, 59, 57, 63},
       {49, 40, 57, 62, 46, 42, 51, 54, 52, 46},
       {8, 11, 10, 7, 10, 10, 13, 10, 10, 9},
       {7, 9, 11, 12, 13, 10, 12, 12, 9, 11},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      },
      {
       {919, 1252, 1162, 797, 1316, 917, 1150, 1064, 974, 976},
       {2444, 1875, 1929, 2230, 1882, 1978, 1977, 2121, 1640, 1506},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {226, 311, 296, 264, 239, 271, 277, 254, 236, 186},
       {657, 562, 617, 707, 633, 670, 639, 589, 573, 683},
       {30, 53, 44, 31, 25, 44, 41, 40, 44, 31},
       {210, 288, 221, 179, 196, 265, 190, 237, 216, 165},
       {234, 200, 243, 279, 195, 191, 204, 292, 215, 156},
       {132, 145, 153, 168, 232, 259, 167, 216, 182, 147},
       {97, 71, 95, 78, 87, 90, 88, 86, 104, 103},
       {81, 99, 110, 102, 82, 104, 100, 86, 120, 85},
       {19, 28, 20, 23, 15, 22, 18, 17, 27, 21}
      },
      {
       {566, 461, 406, 421, 484, 560, 439, 455, 512, 504},
       {101, 89, 97, 76, 110, 96, 90, 102, 108, 107},
       {326, 246, 319, 294, 260, 252, 135, 285, 221, 243},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {242, 312, 293, 255, 314, 248, 401, 312, 284, 381},
       {10, 10, 10, 9, 9, 9, 8, 9, 9, 13},
       {8, 5, 10, 8, 7, 10, 14, 11, 9, 9},
       {10, 9, 10, 8, 7, 12, 11, 9, 12, 11},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      },
      {
       {237, 155, 140, 129, 159, 165, 129, 189, 178, 131},
       {8, 9, 11, 6, 13, 10, 9, 11, 9, 12},
       {688, 539, 768, 613, 545, 567, 544, 428, 589, 713},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {350, 347, 298, 364, 249, 264, 312, 339, 304, 342},
       {26, 19, 20, 24, 23, 16, 18, 21, 21, 18},
       {16, 21, 19, 18, 25, 19, 25, 18, 16, 14},
       {19, 21, 26, 15, 19, 27, 20, 16, 20, 28},
       {8, 12, 10, 8, 8, 8, 6, 10, 10, 11},
       {11, 9, 10, 10, 9, 10, 10, 11, 10, 11},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      },
      {
       {87, 88, 85, 80, 108, 99, 98, 94, 77, 87},
       {1859, 1427, 2079, 1467, 1736, 1681, 1317, 1475, 1953, 1445},
       {56, 39, 43, 39, 38, 37, 42, 39, 51, 44},
       {339, 229, 391, 342, 365, 310, 408, 318, 388, 338},
       {273, 357, 331, 257, 246, 407, 356, 328, 368, 335},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {39, 40, 39, 45, 49, 46, 39, 45, 41, 28},
       {38, 40, 36, 58, 21, 39, 47, 32, 34, 27},
       {42, 38, 46, 43, 38, 42, 49, 40, 37, 42},
       {7, 13, 9, 9, 10, 8, 12, 11, 12, 12},
       {12, 9, 11, 9, 9, 10, 8, 11, 6, 9},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      },
      {
       {657, 551, 548, 536, 565, 469, 547, 610, 504, 452},
       {42, 49, 48, 33, 29, 35, 32, 34, 44, 47},
       {223, 258, 184, 185, 174, 240, 175, 253, 230, 230},
       {11, 10, 8, 10, 7, 9, 7, 10, 8, 10},
       {23, 21, 26, 12, 18, 17, 18, 21, 23, 17},
       {45, 50, 37, 33, 38, 44, 40, 55, 27, 27},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {3880, 3354, 3629, 3946, 4133, 3550, 4845, 4952, 3305, 3533},
       {4676, 3599, 4146, 3126, 3065, 3574, 3729, 2236, 3090, 3713},
       {62, 68, 66, 58, 54, 60, 67, 84, 60, 68},
       {562, 599, 692, 835, 535, 588, 614, 625, 607, 820},
       {391, 296, 498, 391, 441, 466, 422, 293, 544, 372}
      },
      {
       {509, 458, 724, 631, 468, 777, 461, 597, 643, 474},
       {48, 63, 51, 52, 45, 55, 52, 41, 45, 59},
       {238, 198, 243, 145, 141, 228, 282, 250, 226, 268},
       {10, 11, 12, 11, 11, 8, 13, 12, 8, 10},
       {13, 16, 17, 27, 17, 16, 26, 16, 16, 23},
       {35, 40, 54, 46, 41, 40, 54, 39, 50, 56},
       {2504, 3640, 3387, 4088, 4148, 2662, 4021, 4480, 4313, 3376},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {2878, 3339, 3341, 3383, 2879, 3786, 2533, 2947, 2427, 3325},
       {3263, 3834, 3670, 3325, 2903, 2192, 2637, 2624, 2911, 2931},
       {1011, 1026, 1086, 941, 976, 1084, 1381, 998, 1140, 1204},
       {602, 663, 612, 761, 836, 713, 629, 596, 507, 784}
      },
      {
       {329, 389, 379, 361, 236, 488, 356, 345, 371, 211},
       {61, 45, 33, 53, 33, 40, 30, 34, 65, 66},
       {160, 214, 174, 180, 201, 168, 130, 147, 223, 157},
       {9, 11, 13, 10, 10, 10, 12, 8, 11, 7},
       {18, 24, 20, 20, 16, 24, 14, 23, 23, 20},
       {36, 31, 47, 47, 33, 49, 29, 46, 38, 40},
       {4215, 2959, 3499, 4150, 4701, 2979, 3667, 4638, 4952, 4329},
       {3012, 2899, 3170, 3047, 3007, 3125, 3702, 2682, 3449, 2138},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {684, 651, 835, 742, 560, 786, 857, 575, 475, 692},
       {1381, 1257, 911, 671, 1065, 812, 942, 1212, 1089, 1311},
       {424, 496, 438, 452, 488, 440, 421, 467, 464, 462}
      },
      {
       {287, 384, 355, 294, 284, 272, 330, 310, 269, 349},
       {10, 8, 8, 8, 9, 9, 8, 12, 10, 10},
       {84, 75, 71, 122, 94, 100, 104, 92, 101, 82},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {10, 12, 11, 9, 9, 8, 11, 10, 12, 8},
       {11, 10, 13, 14, 12, 8, 9, 10, 13, 8},
       {51, 59, 57, 54, 58, 68, 54, 61, 51, 75},
       {3183, 2105, 2288, 3816, 3568, 3065, 1832, 3073, 2561, 2987},
       {644, 585, 689, 726, 818, 790, 568, 690, 746, 610},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {505, 435, 373, 453, 387, 575, 503, 442, 500, 596},
       {393, 246, 407, 376, 288, 331, 423, 450, 446, 312}
      },
      {
       {205, 245, 244, 224, 155, 253, 252, 175, 200, 200},
       {11, 10, 8, 11, 9, 9, 9, 12, 11, 9},
       {91, 73, 102, 91, 114, 97, 96, 82, 72, 88},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {12, 10, 11, 10, 9, 11, 13, 8, 10, 8},
       {9, 10, 11, 11, 8, 14, 9, 7, 9, 12},
       {501, 562, 559, 741, 347, 646, 589, 494, 593, 582},
       {1076, 1007, 452, 1119, 1217, 933, 908, 1517, 1092, 1409},
       {1290, 1149, 1069, 1069, 844, 1034, 874, 927, 996, 1086},
       {616, 560, 502, 598, 452, 594, 429, 647, 486, 546},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {128, 133, 126, 125, 118, 146, 181, 108, 136, 148}
      },
      {
       {49, 43, 47, 49, 53, 50, 49, 53, 59, 65},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {14, 19, 20, 24, 20, 22, 22, 18, 12, 25},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {425, 272, 460, 330, 485, 500, 398, 294, 421, 417},
       {649, 674, 546, 724, 457, 729, 574, 861, 588, 725},
       {461, 414, 412, 426, 534, 476, 438, 379, 485, 488},
       {472, 358, 414, 346, 450, 379, 529, 396, 373, 536},
       {106, 126, 137, 125, 131, 168, 136, 153, 125, 123},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
      }
     }
    };
    MP_random_data d(&stochasticDemand[0][0][0][0], ST, A, A2);
    d.setName("d");

    // minimale Nachfrageerfüllung
    MP_data x_min;
    x_min() = 0.5;
    x_min.setName("x_min");

    // Kerosinpreise
    double kerosinPrice[numST-1][numScen] = { {0.055, 0.11, 0.165, 0.275, 0.4125, 0.55, 0.6875, 0.825, 1.045, 1.375} };
    MP_random_data k_price(&kerosinPrice[0][0], ST);
    k_price.setName("k_price");

    // forwared jet fuel price
    MP_data f_price;
    f_price() = 0.55;
    f_price.setName("f_price");

    // margin for forwarded jet fuel price
    MP_data f_price_margin;
    f_price_margin() = 10;
    f_price_margin.setName("f_price_margin");


    /////// VARIABLES

    MP_variable y(A,A2,T);
    y.integer();
    for (int i = 0; i < numA; i++) {
        for (int j = 0; j < numA; j++) {
            for (int k = 0; k < numT; k++) {
                if (i == j || dist(i,j) > r(k)) {
                    y.upperLimit(i,j,k) = 0;
                }
            }
        }
    }
    y.setName("y");

    MP_variable x0(ST,A,A2);
    MP_variable x1(ST,A,A3,A2);
    MP_variable x2(ST,A,A3,A4,A2);
    for (int i = 0; i < numA; i++) {
        for (int j = 0; j < numA; j++) {
            if (i == j) {
                x0.upperLimit(1,i,j) = 0;
            }
            for (int k = 0; k < numA; k++) {
                if (i == j || i == k || j == k) {
                    x1.upperLimit(1,i,k,j) = 0;
                }
                for (int l = 0; l < numA; l++) {
                    if (i == j || i == k || i == l || j == k || j == l || k == l) {
                        x2.upperLimit(1,i,k,l,j) = 0;
                    }
                }
            }
        }
    }
    x0.setName("x0");
    x1.setName("x1");
    x2.setName("x2");

    MP_variable buy_stoch(ST), buy_hedge, jet_fuel_consumption;
    buy_stoch.setName("buy_stoch");
    buy_hedge.setName("buy_hedge");
    jet_fuel_consumption.setName("jet_fuel_consumption");


    ////// CONSTRAINTS

    //demand
    MP_constraint demand(ST,A,A2);
    demand(ST+1,A,A2) = x0(ST+1,A,A2) + sum(A3, x1(ST+1,A,A3,A2)) + sum(A3*A4, x2(ST+1,A,A3,A4,A2)) <= d(ST+1,A,A2);
    demand.setName("demand");
    
    //subject to force_percentage_expected_demand_satisfaction 
    MP_constraint percentDemandSat(ST,A,A2);
    percentDemandSat(ST+1,A,A2) = x_min() * d(ST+1,A,A2) <= x0(ST+1,A,A2) + sum(A3, x1(ST+1,A,A3,A2)) + sum(A3*A4, x2(ST+1,A,A3,A4,A2));
    percentDemandSat.setName("percentDemandSat");

    //max_aircraft_capacity
    MP_constraint maxAircraftCapacity(ST,A,A2);
    maxAircraftCapacity(ST+1,A,A2) = x0(ST+1,A,A2) + sum(A3, x1(ST+1,A,A2,A3) + x1(ST+1,A3,A,A2) + sum(A4, x2(ST+1,A,A2,A3,A4) + x2(ST+1,A3,A,A2,A4) + x2(ST+1,A3,A4,A,A2))) <= sum(T, m(T) * y(A,A2,T));
    maxAircraftCapacity.setName("maxAircraftCapacity");
    
    //ub for airlcraft types{t in T}: sum{i in A, j in A}(y[i,j,t])<=ub[t];
    MP_constraint availableAircrafts(T);
    availableAircrafts(T) = sum(A*A2, y(A,A2,T)) <= ub(T);
    availableAircrafts.setName("availableAircrafts");

    //buy_jet_fuel: buy_stoch + buy_hedge = jet_fuel_consumption;
    MP_constraint buyJetFuel(ST);
    buyJetFuel(ST+1) = buy_stoch(ST+1) + buy_hedge() == jet_fuel_consumption();
    buyJetFuel.setName("buyJetFuel");
    
    //fuel consumption
    MP_constraint fuelConsumption;
    //fuelConsumption() = sum(A*A2*T, y(A,A2,T) * dist(A,A2) * cons(T)) == jet_fuel_consumption();
    fuelConsumption() = sum(A*A2, sum(T, y(A,A2,T) * dist(A,A2) * cons(T))) == jet_fuel_consumption();
    fuelConsumption.setName("fuelConsumption");


    /////// OBJECTIVE

    mod.setObjective(
          (buy_stoch(ST+1) * k_price(ST+1) + buy_hedge() * f_price() * (1 + (f_price_margin() / 100))) //fuel costs
        + (sum(A*A2, sum(T, y(A,A2,T) * dist(A,A2) * cpm(T)))) //operationcost
        - (sum(A*A2, p(A,A2) * dcf() * (x0(ST+1,A,A2) + (1 - pd()) * sum(A3, x1(ST+1,A,A3,A2)) + (1 - 2*pd()) * sum(A3*A4, x2(ST+1,A,A3,A4,A2))))) //revenue
    );

    mod.attach(mod.Solver);

    //mod.solve(MP_model::MINIMIZE);

    OsiStochasticBendersTwoStageSolverInterface benders(mod.Solver,mod.getSmi());
    benders.initialSolve();

    return 0;
}
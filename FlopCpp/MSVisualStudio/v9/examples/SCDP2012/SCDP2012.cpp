// $Id$
#include <iostream>
#include <fstream>
#include <ctime>
#include <limits>
#include <algorithm>
#include <cmath>

#include <boost/math/distributions.hpp>
#include <boost/math/constants/constants.hpp>

//#include "OsiClpSolverInterface.hpp"
#include "OsiCpxSolverInterface.hpp"

#include "CoinFloatEqual.hpp"
#include "CoinTime.hpp"

#include "flopc.hpp"
#include "SmiScnModel.hpp"

#include "OsiPnbStochasticSolverInterface.hpp"
#include "BendersNodeData.hpp"

using namespace flopc;

int main(int argc, char* argv[]) {
    MP_model model(new OsiCpxSolverInterface());

#pragma region sets

    // Stages
    enum {T2009, T2010, T2011, numT};
    MP_stage T(numT);
    T.setName("T");

	MP_set T2(numT);
    T2.setName("T2");

    // Lines
    enum {L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, L13, L14, L15, L16, L17, L18, L19, L20, numL};
    MP_set L(numL);
    L.setName("L");
    
    // Products
    enum {P1, P2, P3, numP};
    MP_set P(numP);
    P.setName("P");
    
    // Markets
    enum {M1, numM};
    MP_set M(numM);
    M.setName("M");
    
    // Currencies
    enum {C1, C2, numC};
    MP_set C(numC);
    C.setName("C");
    
    // Connects the Lines to Currencies
    MP_subset<2> LineLinks(L, C);
    LineLinks.insert(L1, C1);
    LineLinks.insert(L2, C1);
    LineLinks.insert(L3, C1);
    LineLinks.insert(L4, C1);
    LineLinks.insert(L5, C1);
    LineLinks.insert(L6, C1);
    LineLinks.insert(L7, C1);
    LineLinks.insert(L8, C1);
    LineLinks.insert(L9, C1);
    LineLinks.insert(L10, C1);
    LineLinks.insert(L11, C2);
    LineLinks.insert(L12, C2);
    LineLinks.insert(L13, C2);
    LineLinks.insert(L14, C2);
    LineLinks.insert(L15, C2);
    LineLinks.insert(L16, C2);
    LineLinks.insert(L17, C2);
    LineLinks.insert(L18, C2);
    LineLinks.insert(L19, C2);
    LineLinks.insert(L20, C2);
	LineLinks.setName("LineLinks");
    
    // Connects the Markets to the Currencies
    MP_subset<2> MarketLinks(M, C);
    MarketLinks.insert(M1, C1);
    MarketLinks.setName("MarketLinks");

#pragma endregion

#pragma region production

    // m{l in L}; #capacity of line l
    double capacity[numL] =
    {
      75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000
    };
    MP_data m(&capacity[0], L);
    m.setName("m");

	const double fc_level = 1.0;  // fixed cost level

	// c_build{l in L}; #costs für building line l
    double costs_building[numL] =
    {
      250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000
    };
    MP_data c_build(&costs_building[0], L);
    c_build.setName("c_build");

    // c_operate{l in L}; #costs für operating line l
    double costs_operate[numL] =
    {
        50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000
    };
    MP_data c_operate(&costs_operate[0], L);
    c_operate.setName("c_operate");

    // q_prod{l in L, p in P}; # production costs per unit of product p on line l
    double productionCosts[numL][numP] =
    {
        {7, 12.5, 17}, // L1
        {7, 12.5, 17}, // L2
        {7, 12.5, 17}, // L3
        {7, 12.5, 17}, // L4
        {7, 12.5, 17}, // L5
        {7, 12.5, 17}, // L6
        {7, 12.5, 17}, // L7
        {7, 12.5, 17}, // L8
        {7, 12.5, 17}, // L9
        {7, 12.5, 17}, // L10
        {5, 9, 13}, // L11
        {5, 9, 13}, // L12
        {5, 9, 13}, // L13
        {5, 9, 13}, // L14
        {5, 9, 13}, // L15
        {5, 9, 13}, // L16
        {5, 9, 13}, // L17
        {5, 9, 13}, // L18
        {5, 9, 13}, // L19
        {5, 9, 13}  // L20
	};
    MP_data q_prod(&productionCosts[0][0], L, P);
    q_prod.setName("q_prod");

    // d_basis{m in M,p in P,t in T}; # basis_demand, used to calculate the real demand
    double basisDemand[numM][numP][numT] =
    {
        { // M1
            { // P1
                0, 270000, 200000
            },
            { // P2
                0, 0, 120000
            },
            { // P3
                0, 160000, 180000
            }
        }
    };
    MP_data d_basis(&basisDemand[0][0][0], M, P, T);
    d_basis.setName("d_basis");

    // q_trans{l in L, p in P, m in M}; transportation costs for one unit of product p from line l to market m
    double transportationCosts[numL][numP][numM] =
    {
        { // L1
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L2
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L3
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L4
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L5
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L6
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L7
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L8
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L9
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L10
			{ { 0.2  } }, // P1 M1
			{ { 0.22 } }, // P2 M1
			{ { 0.25 } }  // P3 M1
		},
        { // L11
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L12
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L13
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L14
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L15
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L16
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L17
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L18
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L19
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		},
        { // L20
			{ { 0.8  } }, // P1 M1
			{ { 1.0  } }, // P2 M1
			{ { 1.2  } }  // P3 M1
		}
	};
    MP_data q_trans(&transportationCosts[0][0][0], L, P, M);
    q_trans.setName("q_trans");

    // r{p in P}; #capacity usage for producing one item of product p
    double capacityProduct[numP] = { 2.5, 3.0, 3.75};
    MP_data r(&capacityProduct[0], P);
    r.setName("r");

    // price{m in M, p in P}; #price of product p on market/currency
    double valPrice[numM][numP] = 
    {
        { // M1
          20, 35, 70
        }
    };
    MP_data price(&valPrice[0][0], M, P);
    price.setName("price");

    // df; factor for calculating the demand
	const double df = 1.28;  //sigma = 0.25

   // demand: up or down (50/50) with factor of df
    std::vector<double> probsD;
    std::vector<double> valuesD;
    probsD.push_back(0.5);
    probsD.push_back(0.5);
    valuesD.push_back(df);
    valuesD.push_back(1/df);

    RandomVariable* rVarDemand[numT-1] = { new EmpiricalRandomVariable(valuesD,probsD), new EmpiricalRandomVariable(valuesD,probsD)};
    MP_random_data d_factor(&rVarDemand[0], T);

    MP_random_data d(M,P,T); // We need a recursive formulation for that! (again..)
    d(M,P,T) = d_basis(M,P,T)* product(T2.such_that(T2<=T && T2 > 0),d_factor(T2));
    d.setName("d");

#pragma endregion

#pragma region financial

    // ir{c in C}; #interest rate of risk free investment in currency c
    double ir_data[numC] = { 0.02, 0.07};
    MP_data ir(&ir_data[0],C);

    // ir_ref; #interest rate of the reference currency
    const double ir_ref = 0.02;

	// ir_wacc; #interest used to calculate the npv (wacc - weighted average cost of capital) 
	const double ir_wacc = 0.08;

	// max_hedge_time; #maximal hedge period 	
    const int max_hedge_time = 1;

    // exr_basis{c in C}; #basis exchange rate - used to calculate the exchange rates for the scenarios
    double val_exr_basis[numC] = { 0, 1};
    MP_data exr_basis(&val_exr_basis[0], C);

    // exr_offset{c in C}; #with exr_basis=0 and exr_offset=1 a one currency can be defined as the reference currency - should be 0 at all other currencies;
    // C1 is the reference currency
    double val_exr_offset[numC] = { 1, 0};
    MP_data exr_offset(&val_exr_offset[0], C);

    // ef; #factor for calculating the exchange rate
    const double ef = 1.28;  //sigma = 0.25

    // probability for the exchange rate going up
//    const double p_up = (1-(1/ef))/(ef-(1/ef)); 
    const double p_up = (std::pow(boost::math::constants::e<double>(),ir_ref-ir(1))-(1/ef))/(ef-(1/ef));

	// let  p_up:=((exp(ir_ref-ir["C2"]))-(1/ef))/(ef-(1/ef)); # general case, valid for unequal interest rates

   // exchange rate
    std::vector<double> probsE;
    std::vector<double> valuesE;
    probsE.push_back(p_up);
    probsE.push_back(1 - p_up);
    valuesE.push_back(ef);
    valuesE.push_back(1/ef);

    RandomVariable* rVarExr[numT-1] = { new EmpiricalRandomVariable(valuesE,probsE),new EmpiricalRandomVariable(valuesE,probsE)};
    
    MP_random_data exr_factor(&rVarExr[0], T);
    exr_factor.setName("exr_factor");
    
    MP_random_data exr(T,C); // exchange rate
    exr.setName("exr");

    // Set Initial Values for first stage deviation
    MP_data exr_initial(T,C); // initial data for recursion, get's initialized with zeroes
    exr_initial.setName("exr_initial"); // 
    exr_initial(0,C) = exr_offset(C)+exr_basis(C); // This is correct, as offset is only 1 at reference currency, and 0 everywhere else.
    // Do Recursion
    exr(T,C) = mpif(T>0,exr(T-1,C)*mpif(exr_offset(C)==1,1,exr_factor(T)) ,exr_initial(0,C));//+exr_initial(T-1,C)*mpif(exr_offset(C)==1,1,exr_factor(T)); // This construction is necessary at the moment for this recursive formulation TODO
   
    // fexr; #exchange rate for a forward contract buying currency c and selling referency currency. contracted in period tbuy, operating in period teffect
    MP_random_data fexr(C,T2,T); // {c in C,s in Scen,tbuy in T, teffect in T:teffect>tbuy and teffect-tbuy<=max_hedge_time}:=exr[c,tbuy,s]*exp((ir_ref-ir[c])*(teffect-tbuy))
    fexr.setName("fexr");
    MP_index tbuy, teffect;

	// let {c in C,s in Scen,tbuy in T, teffect in T:teffect>tbuy and teffect-tbuy<=max_hedge_time} fexr[c,s,tbuy,teffect]:=exr[c,tbuy,s]*exp((ir_ref-ir[c])*(teffect-tbuy)) ; #exchange rate for a forward contract buying currency c and selling referency currency. contracted in period tbuy, operating in period teffect
	fexr(C,T2,T).such_that(T > T2 && T-T2 <= max_hedge_time) = mpif(T2 == 0,1.0,exr(T2,C)*power(boost::math::constants::e<double>(),(ir_ref-ir(C))*(T-T2))); 

	// rc; #recourse coefficient. how high are the recourse costs for not serving the demand (1.0 means that there are no. 1.1 means 10% additional loss of the price for a not delivered unit
    const double rc = 1;

#pragma endregion

#pragma region option price

    MP_data sigma(C, T, T2);
    MP_data o_d1(C, T, T2);
    MP_data o_d2(C, T, T2);
    MP_set o_inf(101); // 0..100
    MP_data phi_o_d1(C, T, T2);
    MP_data phi_o_minus_d1(C, T, T2);
    MP_data phi_o_d2(C, T, T2);
    MP_data phi_o_minus_d2(C, T, T2);
    MP_random_data p_put(C, T, T2);
    p_put.setName("p_put");
    MP_random_data p_call(C, T, T2);
    p_call.setName("p_call");
    //
    for (int tbuy = 1; tbuy < numT; tbuy++) {
        for (int texpire = tbuy + 1; texpire < min(numT, tbuy + max_hedge_time+1); texpire++) {// FORALL C.
            //forall(C, sigma(C, tbuy, texpire) = std::log(ef) / std::sqrt((double)(texpire-tbuy))); // power(texpire - tbuy,0.5);
            //forall(C, o_d1(C, tbuy, texpire) = ((ir_ref - ir(C) + (sigma(C, tbuy, texpire)*sigma(C, tbuy, texpire) ) / 2) * (texpire - tbuy)) / 
            //    (sigma(C, tbuy, texpire) * sqrt((double) texpire - tbuy)));
            //forall(C, o_d2(C, tbuy, texpire) = o_d1(C, tbuy, texpire) - sigma(C, tbuy, texpire) * sqrt((double) texpire - tbuy));
            
            MP_index o2;
            //// TODO: Normalverteilung is what we want!
            for (int current_c = 0; current_c < C.size(); current_c++){
                sigma(current_c, tbuy, texpire) = std::log(ef) / std::sqrt((double)(texpire-tbuy));
                o_d1(current_c, tbuy, texpire) = 0.5 * sigma(current_c, tbuy, texpire) * sqrt((double) texpire - tbuy);
                o_d2(current_c, tbuy, texpire) = o_d1(current_c, tbuy, texpire) - sigma(current_c, tbuy, texpire) * sqrt((double) texpire - tbuy);
                phi_o_d1(current_c, tbuy, texpire) = boost::math::cdf(boost::math::normal(0,1),o_d1(current_c,tbuy,texpire) );
                phi_o_d2(current_c, tbuy, texpire) = boost::math::cdf(boost::math::normal(0,1),o_d2(current_c,tbuy,texpire) );
                phi_o_minus_d1(current_c, tbuy, texpire) = boost::math::cdf(boost::math::normal(0,1),-o_d1(current_c,tbuy,texpire) );
                phi_o_minus_d2(current_c, tbuy, texpire) = boost::math::cdf(boost::math::normal(0,1),-o_d2(current_c,tbuy,texpire) );

            }
        }
    }
    p_put(C,T,T2).such_that(T2>T) = exr(T,C)* power(boost::math::constants::e<double>(),-ir(C)*(T2-T)) * phi_o_minus_d2(C,T,T2)
        - exr(T,C)*power(boost::math::constants::e<double>(),-ir(C) *(T2-T))*phi_o_minus_d1(C,T,T2);
//    p_put(C,T,T2).such_that(T2>T) = exr(T,C)* power(boost::math::constants::e<double>(),-ir_ref*(T2-T)) * phi_o_minus_d2(C,T,T2)
//        - exr(T,C)*power(boost::math::constants::e<double>(),-ir(C) *(T2-T))*phi_o_minus_d1(C,T,T2);
    
    p_call(C,T,T2).such_that(T2>T) = exr(T,C)* power(boost::math::constants::e<double>(),-ir(C)*(T2-T)) * phi_o_d1(C,T,T2)
        - exr(T,C)*power(boost::math::constants::e<double>(),-ir(C)*(T2-T)) *phi_o_d2(C,T,T2);

#pragma endregion

#pragma region accounting variables for output

	MP_variable sum_of_lines(C,P); sum_of_lines.setName("sum_of_lines");
	MP_variable weighted_npv; weighted_npv.setName("weighted_npv");
    MP_variable npv_min; npv_min.setName("npv_min"); // see risk management strategy 2
    MP_variable cvar95; cvar95.setName("cvar95"); 
    MP_variable y_build(L,P); y_build.setName("y_build"); // {l in L, p1 in P} binary; #1 if line l is built to produce product p, 0 otherwise
    y_build.binary();

	MP_variable npv; npv.setName("npv"); //{s in Scen}; #npv for each scenario
    MP_variable npv_stage(T); npv_stage.setName("npv_stage"); npv_stage.free();

#pragma endregion

#pragma region decision variables

    MP_variable x2(M,P,T); x2.setName("x2"); // {m1 in M,p in P,s in Scen} >=0; #not delivered amount of product p on market m from line l in period p in scenario s - needed for calculating the recourse costs
    MP_variable x(L,M,P,T); x.setName("x"); // Amount of product p produced on line l, delivered to market m in time period t
    MP_variable f_rev(C,T,T2); f_rev.setName("f_rev"); // {c in C, tbuy in T, teffect in T, s in Scen:teffect>tbuy and teffect-tbuy<=max_hedge_time}  =0; # forward amount (between currency c (buy) and reference currency (sell), bought in period tbuy, operates in period teffect), is limited to the production activities (i. e. 1 is the maxmium hedge rate - see restriction)
    f_rev.upperLimit(C,T,T2) = 0;
    f_rev.lowerLimit(C,T,T2) = -1e30; // -1e20 is infinity?
	MP_variable f_cost(C,T,T2); f_cost.setName("f_cost"); // {c in C, tbuy in T, teffect in T, s in Scen:teffect>tbuy and teffect-tbuy<=max_hedge_time} =0; # forward amount (between currency c (buy) and reference currency (sell), bought in period tbuy, operates in period teffect), is limited to the production activities (i. e. 1 is the maxmium hedge rate - see restriction)
    MP_variable call_b(C,T,T2); call_b.setName("call_b"); // {c in C, tbuy in T, texpire in T, s in Scen:texpire>tbuy and texpire-tbuy<=max_hedge_time} = 0; # amount of bought call options for currency c in period tbuy, expire in period texpire, strike price is the exr in period tbuy (european options)
    MP_variable put_b(C,T,T2); put_b.setName("put_b"); // {c in C, tbuy in T, texpire in T, s in Scen:texpire>tbuy and texpire-tbuy<=max_hedge_time} = 0; # amount of bought put options for currency c in period tbuy, expire in period texpire, strike price is the exr in period tbuy (european options)
    MP_variable call_e(C,T2,T); call_e.setName("call_e"); // {c in C, tbuy in T, texc in T, s in Scen:texc>tbuy and texc-tbuy<=max_hedge_time} = 0 ; # amount of excercised call options in period texc for currency c bought in period tbuy, strike price is the exr in period tbuy (european options)
    MP_variable put_e(C,T2,T); put_e.setName("put_e"); //{c in C, tbuy in T, texc in T, s in Scen:texc>tbuy and texc-tbuy<=max_hedge_time} = 0 ; # amount of excercised put options in period texc for currency c bought in period tbuy, strike price is the exr in period tbuy (european options)

    // Enable/Disable options

/*
	call_e.fix(0);
    call_b.fix(0);
    put_e.fix(0);
    put_b.fix(0);
	f_cost.fix(0);
    f_rev.fix(0);
*/

#pragma endregion

#pragma region accounting variables 2

    MP_variable cost_prod(L,T); cost_prod.setName("cost_prod"); // {l in L, t in T, s in Scen} >= 0; #production costs in currency c in period t
    MP_variable cost_trans(L,T); cost_trans.setName("cost_trans"); // {l in L, t in T, s in Scen} >= 0; #transportation costs in currency c in period t
    MP_variable cost_optionbuy(T); cost_optionbuy.setName("cost_optionbuy"); // {t in T, s in Scen} >=0; #costs for buying options in period p in scenario s
    MP_variable revenue(M,T); revenue.setName("revenue"); // {m1 in M, t in T, s in Scen} >= 0; #revenue in currency c in period t
    MP_variable cost_linebuild(C); cost_linebuild.setName("cost_linebuild"); // {c in C} >=0; #Line Building Costs in currency c (lines can only be built in the first time period)
    MP_variable cost_lineoperate(C); cost_lineoperate.setName("cost_lineoperate"); // {c in C} >=0; #line operating costs in currency c
    MP_variable cfp(C,T); cfp.setName("cfp"); // {c in C, t in T, s in Scen}; #cash flow of the production activities in currency c in periode t
    cfp.free();
    MP_variable cf(C,T); cf.setName("cf"); // {c in C, t in T, s in Scen}; #total cash flow in currency c in periode t (production + financial)
    cf.free();
    MP_variable cf_ref(T); cf_ref.setName("cf_ref"); // {t in T, s in Scen}; #over-all cash flow in the reference currency in period t
    cf_ref.free();

#pragma endregion

#pragma region fix binary variable

    //double valBuildY[numL][numP] =
    //{
    //    { // L3
    //        1, 1, 0
    //    },
    //    { // L8
    //        0, 0, 1
    //    }
    //};
    //MP_data buildY(&valBuildY[0][0], L, P);
    //MP_constraint fix_build_y(L,P);
    //fix_build_y.setName("fix_build_y");
    //fix_build_y(L,P) = y_build(L,P) == buildY(L,P);

    MP_variable y_build_loop(L,P,T);
    MP_constraint fix_y_build(L,P,T);
    MP_constraint loop_y_build(L,P,T);
    fix_y_build(L,P,T).such_that(T==0) = y_build_loop(L,P,T) == y_build(L,P);
    loop_y_build(L,P,T+1) = y_build_loop(L,P,T) == y_build_loop(L,P,T-1);
    

#pragma endregion

#pragma region production

    MP_constraint demand_fulfilment(M,T,P); // {m1 in M, t in T, p in P, s in Scen}: 	sum {l in L}(x[l,m1,p,t,s]) + sum{l in L}(x2[l,m1,p,t,s]) >= d[m1,p,t,s];
    demand_fulfilment.setName("demand_fulfilment");
    MP_constraint max_capacity(L,P,T); //{l in L, p1 in P, t in T, s in Scen}: sum {m1 in M}(x[l,m1,p1,t,s]*r[p1]) <= m[l,p1]*y_build[l,p1];
    max_capacity.setName("mac_capacity");
    MP_constraint prod_costs_for_line_l_in_period_t(L,T); // {l in L, t in T,s in Scen}: 	cost_prod[l,t,s] = sum{m1 in M, p1 in P}(x[l,m1,p1,t,s]*q_prod[l,p1,t]);
    prod_costs_for_line_l_in_period_t.setName("prod_cost");
    MP_constraint transportation_costs_from_line_l(L,T); // {l in L, t in T, s in Scen}: 	cost_trans[l,t,s] = sum{m1 in M, p1 in P}(x[l,m1,p1,t,s]*q_trans[l,p1,m1,t]);   #calculated in the currency of the line
    transportation_costs_from_line_l.setName("transportation_cost");
    MP_constraint revenue_in_market_m_in_periode_t(M,T); // {m1 in M, t in T, s in Scen}: 	revenue[m1,t,s] = sum{p1 in P}( (d[m1,p1,t,s]-rc*sum{l in L}x2[l,m1,p1,t,s])*price[m1,p1,t]);
    revenue_in_market_m_in_periode_t.setName("revenue_in_market");
    MP_constraint line_building_costs_in_currency_c(C); // {c in C}: 	cost_linebuild[c] = sum{(l,c)in LineLinks,p1 in P}(c_build[l,p1]*y_build[l,p1]);
    line_building_costs_in_currency_c.setName("line_building_cost");
    MP_constraint line_operating_costs_in_currency_c(C); // {c in C}: cost_lineoperate[c] = sum{(l,c)in LineLinks,p1 in P}(c_operate[l,p1]*y_build[l,p1]);
    line_operating_costs_in_currency_c.setName("line_operating_cost");

    demand_fulfilment(M,T,P) = sum(L, x(L,M,P,T)) + x2(M,P,T) == d(M,P,T);
    max_capacity(L,P,T) = sum(M, x(L,M,P,T) * r(P)) <= m(L) * y_build_loop(L,P,T); // Constraint that is rather odd from a stagewise constraint?!
    prod_costs_for_line_l_in_period_t(L,T) = cost_prod(L,T) == sum(M*P, x(L,M,P,T) * q_prod(L,P));
    transportation_costs_from_line_l(L,T) = cost_trans(L,T) == sum(M*P, x(L,M,P,T) * q_trans(L,P,M));
    revenue_in_market_m_in_periode_t(M,T) = revenue(M,T) == sum(P, (d(M,P,T) - rc * x2(M,P,T)) * price(M,P));
    line_building_costs_in_currency_c(C) = cost_linebuild(C) == sum(LineLinks(L,C)*P, fc_level * c_build(L) * y_build(L,P));
    line_operating_costs_in_currency_c(C) = cost_lineoperate(C) == sum(LineLinks(L,C)*P, fc_level * c_operate(L) * y_build(L,P));

    MP_variable cost_lineoperate_loop(C,T);
    cost_lineoperate_loop.free();
    MP_constraint loop_cost_lineoperate(C,T);
    MP_constraint fix_cost_lineoperate(C,T);
    loop_cost_lineoperate(C,T+1) = cost_lineoperate_loop(C,T) == cost_lineoperate_loop(C,T-1);
    fix_cost_lineoperate(C,T).such_that(T==0) = cost_lineoperate_loop(C,T) == cost_lineoperate(C);

#pragma endregion

#pragma region hedging

    MP_constraint production_cash_flow_in_period_p_in_currency_c(C,T); // {c in C, t in T, s in Scen}: 	cfp[c,t,s] = (sum{(m1,c)in MarketLinks}(revenue[m1,t,s])) - (sum{(l,c)in LineLinks}(cost_prod[l,t,s]+cost_trans[l,t,s])) - cost_lineoperate[c];
    production_cash_flow_in_period_p_in_currency_c.setName("production_cashflow");
    MP_constraint hedge_rate_lt_1_for_revenues(C,T); // {s in Scen,c in C, teffect in T}: sum{tbuy2 in T:tbuy2<teffect and teffect-tbuy2<=max_hedge_time}(f_rev[c,tbuy2,teffect,s]/fexr[c,s,tbuy2,teffect]-put_e[c,tbuy2,teffect,s]/exr[c,tbuy2,s] ) >= -(sum{(m1,c)in MarketLinks}(revenue[m1,teffect,s])); 
    hedge_rate_lt_1_for_revenues.setName("hege_rate_r");
    MP_constraint hedge_rate_lt_1_for_costs(C,T); //{c in C, teffect in T,s in Scen}: sum{tbuy2 in T:tbuy2<teffect and teffect-tbuy2<=max_hedge_time}(f_cost[c,tbuy2,teffect,s]/fexr[c,s,tbuy2,teffect]+call_e[c,tbuy2,teffect,s]/exr[c,tbuy2,s]) <= (sum{(l,c)in LineLinks}(cost_prod[l,teffect,s]+cost_trans[l,teffect,s])) + cost_lineoperate[c];
    hedge_rate_lt_1_for_costs.setName("hedge_rate_c");

	MP_constraint fix_f_cost0(C,T2);
	fix_f_cost0.setName("fix_fcost0");
	fix_f_cost0(C,T2) = f_cost(C,0,T2) == 0.0;

	MP_constraint fix_f_rev0(C,T2);
	fix_f_rev0.setName("fix_frev0");
	fix_f_rev0(C,T2) = f_rev(C,0,T2) == 0.0;

    production_cash_flow_in_period_p_in_currency_c(C,T) = cfp(C,T) == sum(MarketLinks(M,C), revenue(M,T)) - sum(LineLinks(L,C), cost_prod(L,T) + cost_trans(L,T)) - cost_lineoperate_loop(C,T);

//	hedge_rate_lt_1_for_revenues(C,T) = sum(T2.such_that(T < T2 && T2-T <= max_hedge_time), (f_rev(C,T,T2) - put_e(C,T,T2))) >= -sum(MarketLinks(M,C), revenue(M,T2));
//    hedge_rate_lt_1_for_costs(C,T) = sum(T2.such_that(T < T2 && T2-T <= max_hedge_time), (f_cost(C,T,T2) + call_e(C,T,T2))) <= sum(LineLinks(L,C), cost_prod(L,T2) + cost_trans(L,T2)) + cost_lineoperate(C);


	hedge_rate_lt_1_for_revenues(C,T+1) = sum(T2.such_that(T2 < T && T-T2 <= max_hedge_time), f_rev(C,T2,T) - put_e(C,T2,T) ) >= -sum(MarketLinks(M,C), revenue(M,T));
     // Non-staircase constraint!
     hedge_rate_lt_1_for_costs(C,T+1) = sum(T2.such_that(T2 < T && T-T2 <= max_hedge_time), (f_cost(C,T2,T) + call_e(C,T2,T))) <= sum(LineLinks(L,C), cost_prod(L,T) + cost_trans(L,T)) + cost_lineoperate_loop(C,T);



#pragma endregion

#pragma region options

    MP_constraint costs_for_buying_options(T); // {tbuy in T, s in Scen}: cost_optionbuy[tbuy,s] = sum{c in C,texp in T:tbuy<texp and texp-tbuy<=max_hedge_time}(call_b[c,tbuy,texp,s]*p_call[c,tbuy,texp,s]+put_b[c,tbuy,texp,s]*p_put[c,tbuy,texp,s]);
    costs_for_buying_options.setName("option_cost");
    MP_constraint use_not_more_than_the_bought_calls(C,T,T); // {c in C,t1 in T,t2 in T,s in Scen:t1<t2 and t2-t1<=max_hedge_time}: call_e[c,t1,t2,s] <= call_b[c,t1,t2,s];
    use_not_more_than_the_bought_calls.setName("call_limit");
    MP_constraint use_not_more_than_the_bought_puts(C,T,T); // {c in C,t1 in T,t2 in T,s in Scen:t1<t2 and t2-t1<=max_hedge_time}: put_e[c,t1,t2,s] <= put_b[c,t1,t2,s];
    use_not_more_than_the_bought_puts.setName("put_limit");

	MP_constraint fix_call_b0(C,T2);
	fix_call_b0.setName("fix_call_b0");
	fix_call_b0(C,T2) = call_b(C,0,T2) == 0.0;

	MP_constraint fix_put_b0(C,T2);
	fix_put_b0.setName("fix_put_b0");
	fix_put_b0(C,T2) = put_b(C,0,T2) == 0.0;

    MP_index texp;
    costs_for_buying_options(T) = cost_optionbuy(T) == sum(C*T2.such_that(T < T2 && T2-T <= max_hedge_time), call_b(C,T,T2) * p_call(C,T,T2) + put_b(C,T,T2) * p_put(C,T,T2));
    use_not_more_than_the_bought_calls(C,T,T2).such_that(T2 < T && T-T2 <= max_hedge_time) = call_e(C,T2,T) <= call_b(C,T2,T);
    use_not_more_than_the_bought_puts(C,T,T2).such_that(T2 < T && T-T2 <= max_hedge_time) = put_e(C,T2,T) <= put_b(C,T2,T);

#pragma endregion

#pragma region All-over-Cash-Flows

    MP_constraint cash_flow_in_period_p_in_currency_c_production_and_financial(C,T); // {s in Scen, c in C, t in T}: cf[c,t,s] = cfp[c,t,s] + ( sum{tbuy in T:tbuy<t and t-tbuy<=max_hedge_time}((f_cost[c,tbuy,t,s]+f_rev[c,tbuy,t,s])/fexr[c,s,tbuy,t]) ) + (sum{tbuy in T:tbuy<t and t-tbuy<=max_hedge_time}((call_e[c,tbuy,t,s]-put_e[c,tbuy,t,s])/exr[c,tbuy,s]));
    cash_flow_in_period_p_in_currency_c_production_and_financial.setName("cf_p_c");
    MP_constraint cash_flow_in_reference_currency_in_periode_t(T); // {t in T, s in Scen}: 	cf_ref[t,s] = ( sum{c in C}(cf[c,t,s]*exr[c,t,s]) ) - ( sum{c in C,tbuy in T:tbuy<t and t-tbuy<=max_hedge_time}(f_cost[c,tbuy,t,s]+f_rev[c,tbuy,t,s]) ) - (sum{c in C,tbuy in T:tbuy<t and t-tbuy<=max_hedge_time}(call_e[c,tbuy,t,s]-put_e[c,tbuy,t,s])) - cost_optionbuy[t,s];
    cash_flow_in_reference_currency_in_periode_t.setName("cf_ref_c");
    MP_constraint calculate_npvs_for_each_stage(T); // {s in Scen}: npv[s] =  (sum{t in T}(cf_ref[t,s]/((1 + ir_ref)^(t-2009)))) - sum{c in C}(cost_linebuild[c]*(exr_offset[c]+exr_basis[c]));
    calculate_npvs_for_each_stage.setName("calculate_npv");
    //MP_constraint calculate_weighted_npv(); // : weighted_npv = sum {s in Scen}(prob[s] * npv[s]);
    
    cash_flow_in_period_p_in_currency_c_production_and_financial(C,T) = cf(C,T) == cfp(C,T)
        + sum(T2.such_that(T2<T && T-T2 <= max_hedge_time), f_cost(C,T2,T) + f_rev(C,T2,T) 
            + call_e(C,T2,T) - put_e(C,T2,T));

    cash_flow_in_reference_currency_in_periode_t(T) = cf_ref(T) == sum(C, cf(C,T)* exr(T,C)) 
        - sum(C*T2.such_that(T2<T && T-T2 <= max_hedge_time), (f_cost(C,T2,T) + f_rev(C,T2,T) 
            + call_e(C,T2,T) - put_e(C,T2,T)) * fexr(C,T2,T)) 
        - cost_optionbuy(T);

    MP_data power_constant(T);
    power_constant(0) = 0;
    power_constant(1) = 1;
    power_constant(2) = 2;
    calculate_npvs_for_each_stage(T+1) = npv_stage(T) ==  cf_ref(T)/ power(1.0 + ir_wacc, power_constant(T)); 

	MP_variable y(T);
	y.setName("y");
	y.free();
	MP_variable y_s(T);
	y_s.setName("y_s");

	MP_data lambda;
	MP_data cvar_alpha;
	lambda() = 0.5;
	cvar_alpha() = 0.05;

	MP_constraint cvar_y(T);
	cvar_y.setName("cvar_y");
	cvar_y(T+1) = y(T) == y(T-1);

    // Loop through some variables
    MP_variable first_stage_costs(T);
    first_stage_costs.free();

    MP_constraint first_stage_fix; first_stage_fix.setName("first_stage_fix");
    MP_constraint first_stage_loop(T); first_stage_loop.setName("first_stage_loop");

    first_stage_fix() = first_stage_costs(0) == sum(C, cost_linebuild(C) * (exr_offset(C) + exr_basis(C)));
    first_stage_loop(T+1) = first_stage_costs(T) == first_stage_costs(T-1);

    MP_variable sum_npv_stage(T);
    sum_npv_stage.free();

    MP_constraint npv_stage_loop(T); npv_stage_loop.setName("npv_stage_loop");
    MP_constraint npv_stage_fix(T); npv_stage_fix.setName("npv_stage_fix");
    npv_stage_fix(T).such_that(T==1) = sum_npv_stage(1) == npv_stage(1);
    npv_stage_loop(T+2) = sum_npv_stage(T) == sum_npv_stage(T-1)+npv_stage(T);

	MP_constraint cvar_npv(T);
	cvar_npv.setName("cvar_npv");
	cvar_npv(T+2) = y(T)-y_s(T) <= (sum_npv_stage(T) - first_stage_costs(T))/1000.0; 
    // Evtl npv_stage durchschleifen. Genauso cost_linebuild!



#pragma endregion

#pragma region optimization

    //sigma.display("simga");
    //phi_o_d1.display("phi_o_d1");
    //phi_o_d2.display("phi_o_d2");
    //o_d1.display("o_d1");
    //o_d2.display("o_d2");
    //p_call.display("p_call");
    
    model.setObjective( lambda() * (-sum(T.such_that(T>0),npv_stage(T)) + sum(C, cost_linebuild(C) * (exr_offset(C) + exr_basis(C)) )) 
        - (1-lambda()) * 1000.0 * (y(0) - (1.0/cvar_alpha())*y_s(T.size()-1)) );
    //model.setObjective(sum(L*M*P*T, x(L,M,P,T)));

    model.attach(); // Load OsiSolverData??
    std::cout << "Writing SMPS" << std::endl;
    model.getSmi()->writeSmps("SCDP", false, false); // We always assume MINIMIZATION
    std::cout << "Solving model" << std::endl;
    model.solve(MP_model::MINIMIZE);
    std::cout << "Optimal: " << model->isProvenOptimal();
    std::cout << "Objective-Value " << model->getObjValue();
    model->writeMps("scdp-problem"); // We should take objective sense into account
    

 //   cost_linebuild.display("cost_linebuild");
	//cf_ref.display("cf_ref");
//	x.display("x");
//	y_build.display("y");
    //y_s.display("y_s");
	//y.display("y");
	fexr.display("fexr");
	p_put.display("p_put");
	p_call.display("p_call");
	//f_cost.display("f_cost");
	//f_rev.display("f_rev");

//	call_b.display("call_b");
//	call_e.display("call_e");
	//put_b.display("put_b");
	//put_e.display("put_e");
//    cf.display("cf");
//    cf_ref.display("cf_ref");
//    cost_optionbuy.display("cost_optionbuy");

    //revenue.display();

    //model.attach();
    

    //OsiStochasticParallelNestedBendersTwoStageSolverInterface nested_benders_solver(new OsiCpxSolverInterface(),model.getSmi());
    //nested_benders_solver.initialSolve(); // We only do minimization!!
    //std::cout << nested_benders_solver.getObjValue();

    //MP_model testModel2(OsiCpxSolverInterface);
    //MP_constraint testConstraint(C,T);
    //testConstraint(C,T) = 0 <= exr(T,C);
    //testModel2.setObjective(1);
    //testModel2.solve(MP_model::MAXIMIZE);
    //testModel2.writeLp("test2");
    
#pragma endregion

    return 0;
}

// y_build binary - selber Werte festlegen
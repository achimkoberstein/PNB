!   
!   
    SNDP_stochastic_MIP_general_instances\SNDP_stochastic_MIP_general_40_3_1_125.mpl
!   
!   Generated with the MPL Modeling System
!   Copyright (c) 1988-2016  Maximal Software
!   
!   Date:               October 16, 2018
!   Time:               18:08
!   
!   Constraints:        85
!   Variables:          620
!   Integers:           28
!   Nonzeros:           541
!   Density:            1.0 %
!   

TITLE SNDP_stochastic_general ;

DECISION VARIABLES

    OpenProduction01 OpenProduction03 OpenProduction05 
    OpenProduction08 OpenProduction09 OpenProduction10 
    OpenProduction11 OpenProduction12 OpenProduction13 
    OpenProduction15 OpenProduction18 OpenProduction19 
    OpenProduction20 OpenProduction21 OpenProduction22 
    OpenProduction23 OpenProduction25 OpenProduction26 
    OpenProduction27 OpenProduction28 OpenProduction29 
    OpenProduction30 OpenProduction31 OpenProduction34 
    OpenProduction35 OpenProduction36 OpenProduction37 
    OpenProduction39 Ship10203 Ship10205 Ship10208 Ship10209 Ship10210 
    Ship10211 Ship10213 Ship10220 Ship10221 Ship10225 Ship10227 
    Ship10228 Ship10231 Ship10236 Ship10237 Ship10239 Ship10401 
    Ship10403 Ship10405 Ship10409 Ship10411 Ship10415 Ship10418 
    Ship10419 Ship10421 Ship10422 Ship10425 Ship10429 Ship10431 
    Ship10434 Ship10437 Ship10505 Ship10508 Ship10509 Ship10510 
    Ship10511 Ship10512 Ship10518 Ship10519 Ship10525 Ship10528 
    Ship10529 Ship10531 Ship10536 Ship11108 Ship11111 Ship11113 
    Ship11115 Ship11118 Ship11120 Ship11122 Ship11123 Ship11128 
    Ship11130 Ship11131 Ship11135 Ship11418 Ship11422 Ship11429 
    Ship11431 Ship11437 Ship11439 Ship11512 Ship11515 Ship11536 
    Ship11603 Ship11605 Ship11609 Ship11611 Ship11612 Ship11618 
    Ship11620 Ship11621 Ship11622 Ship11625 Ship11626 Ship11628 
    Ship11629 Ship11631 Ship11634 Ship11636 Ship11637 Ship11639 
    Ship11709 Ship11710 Ship11712 Ship11713 Ship11715 Ship11718 
    Ship11720 Ship11723 Ship11728 Ship11729 Ship11730 Ship11731 
    Ship11734 Ship11739 Ship12303 Ship12305 Ship12308 Ship12309 
    Ship12312 Ship12315 Ship12320 Ship12323 Ship12325 Ship12326 
    Ship12327 Ship12328 Ship12329 Ship12330 Ship12335 Ship12339 
    Ship12605 Ship12608 Ship12610 Ship12612 Ship12625 Ship12626 
    Ship12628 Ship12630 Ship12635 Ship12639 Ship13211 Ship13212 
    Ship13222 Ship13223 Ship13226 Ship13230 Ship13231 Ship13235 
    Ship13236 Ship13237 Ship13801 Ship13805 Ship13808 Ship13811 
    Ship13815 Ship13819 Ship13821 Ship13822 Ship13823 Ship13826 
    Ship13829 Ship13831 Ship13835 Ship13837 Ship13839 Ship20101 
    Ship20103 Ship20105 Ship20108 Ship20111 Ship20113 Ship20119 
    Ship20122 Ship20123 Ship20125 Ship20126 Ship20128 Ship20136 
    Ship20139 Ship20203 Ship20205 Ship20208 Ship20209 Ship20210 
    Ship20211 Ship20213 Ship20220 Ship20221 Ship20225 Ship20227 
    Ship20228 Ship20231 Ship20236 Ship20237 Ship20239 Ship20601 
    Ship20605 Ship20613 Ship20615 Ship20619 Ship20621 Ship20625 
    Ship20627 Ship20635 Ship20703 Ship20705 Ship20708 Ship20709 
    Ship20711 Ship20715 Ship20719 Ship20720 Ship20721 Ship20722 
    Ship20723 Ship20725 Ship20726 Ship20729 Ship20730 Ship20731 
    Ship20734 Ship20735 Ship20736 Ship20737 Ship20739 Ship21303 
    Ship21305 Ship21308 Ship21309 Ship21310 Ship21312 Ship21313 
    Ship21315 Ship21318 Ship21320 Ship21321 Ship21322 Ship21326 
    Ship21327 Ship21328 Ship21330 Ship21331 Ship21334 Ship21335 
    Ship21336 Ship21337 Ship21339 Ship21418 Ship21422 Ship21429 
    Ship21431 Ship21437 Ship21439 Ship21603 Ship21605 Ship21609 
    Ship21611 Ship21612 Ship21618 Ship21620 Ship21621 Ship21622 
    Ship21625 Ship21626 Ship21628 Ship21629 Ship21631 Ship21634 
    Ship21636 Ship21637 Ship21639 Ship21709 Ship21710 Ship21712 
    Ship21713 Ship21715 Ship21718 Ship21720 Ship21723 Ship21728 
    Ship21729 Ship21730 Ship21731 Ship21734 Ship21739 Ship21803 
    Ship21808 Ship21809 Ship21810 Ship21812 Ship21818 Ship21819 
    Ship21820 Ship21823 Ship21826 Ship21829 Ship21830 Ship21834 
    Ship21835 Ship21836 Ship21837 Ship21911 Ship21915 Ship21919 
    Ship21921 Ship21923 Ship21925 Ship21930 Ship21931 Ship21935 
    Ship21937 Ship21939 Ship22103 Ship22105 Ship22108 Ship22111 
    Ship22112 Ship22118 Ship22120 Ship22121 Ship22122 Ship22127 
    Ship22128 Ship22134 Ship22136 Ship22137 Ship22139 Ship22203 
    Ship22205 Ship22208 Ship22209 Ship22210 Ship22212 Ship22215 
    Ship22218 Ship22219 Ship22220 Ship22222 Ship22223 Ship22225 
    Ship22226 Ship22227 Ship22228 Ship22229 Ship22230 Ship22231 
    Ship22234 Ship22235 Ship22236 Ship22237 Ship22239 Ship22401 
    Ship22409 Ship22410 Ship22411 Ship22412 Ship22413 Ship22415 
    Ship22418 Ship22420 Ship22421 Ship22422 Ship22423 Ship22425 
    Ship22426 Ship22427 Ship22428 Ship22429 Ship22430 Ship22431 
    Ship22434 Ship22435 Ship22436 Ship22437 Ship22439 Ship22727 
    Ship23001 Ship23008 Ship23009 Ship23010 Ship23020 Ship23021 
    Ship23028 Ship23029 Ship23030 Ship23034 Ship23036 Ship23037 
    Ship23039 Ship23115 Ship23118 Ship23120 Ship23125 Ship23131 
    Ship23135 Ship23305 Ship23313 Ship23325 Ship23327 Ship23337 
    Ship23339 Ship23401 Ship23434 Ship23801 Ship23805 Ship23808 
    Ship23811 Ship23815 Ship23819 Ship23821 Ship23822 Ship23823 
    Ship23826 Ship23829 Ship23831 Ship23835 Ship23837 Ship23839 
    Ship30103 Ship30105 Ship30108 Ship30111 Ship30113 Ship30119 
    Ship30122 Ship30123 Ship30125 Ship30126 Ship30128 Ship30136 
    Ship30139 Ship30140 Ship30340 Ship30508 Ship30509 Ship30510 
    Ship30511 Ship30512 Ship30518 Ship30519 Ship30525 Ship30528 
    Ship30529 Ship30531 Ship30536 Ship30540 Ship30840 Ship30940 
    Ship31040 Ship31108 Ship31113 Ship31115 Ship31118 Ship31120 
    Ship31122 Ship31123 Ship31128 Ship31130 Ship31131 Ship31135 
    Ship31140 Ship31240 Ship31303 Ship31305 Ship31308 Ship31309 
    Ship31310 Ship31312 Ship31315 Ship31318 Ship31320 Ship31321 
    Ship31322 Ship31326 Ship31327 Ship31328 Ship31330 Ship31331 
    Ship31334 Ship31335 Ship31336 Ship31337 Ship31339 Ship31340 
    Ship31512 Ship31536 Ship31540 Ship31803 Ship31808 Ship31809 
    Ship31810 Ship31812 Ship31819 Ship31820 Ship31823 Ship31826 
    Ship31829 Ship31830 Ship31834 Ship31835 Ship31836 Ship31837 
    Ship31840 Ship31911 Ship31915 Ship31921 Ship31923 Ship31925 
    Ship31930 Ship31931 Ship31935 Ship31937 Ship31939 Ship31940 
    Ship32040 Ship32103 Ship32105 Ship32108 Ship32111 Ship32112 
    Ship32118 Ship32120 Ship32122 Ship32127 Ship32128 Ship32134 
    Ship32136 Ship32137 Ship32139 Ship32140 Ship32203 Ship32205 
    Ship32208 Ship32209 Ship32210 Ship32212 Ship32215 Ship32218 
    Ship32219 Ship32220 Ship32223 Ship32225 Ship32226 Ship32227 
    Ship32228 Ship32229 Ship32230 Ship32231 Ship32234 Ship32235 
    Ship32236 Ship32237 Ship32239 Ship32240 Ship32303 Ship32305 
    Ship32308 Ship32309 Ship32312 Ship32315 Ship32320 Ship32325 
    Ship32326 Ship32327 Ship32328 Ship32329 Ship32330 Ship32335 
    Ship32339 Ship32340 Ship32540 Ship32605 Ship32608 Ship32610 
    Ship32612 Ship32625 Ship32628 Ship32630 Ship32635 Ship32639 
    Ship32640 Ship32740 Ship32840 Ship32940 Ship33001 Ship33008 
    Ship33009 Ship33010 Ship33020 Ship33021 Ship33028 Ship33029 
    Ship33034 Ship33036 Ship33037 Ship33039 Ship33040 Ship33115 
    Ship33118 Ship33120 Ship33125 Ship33135 Ship33140 Ship33401 
    Ship33440 Ship33540 Ship33640 Ship33740 Ship33940 ;

MODEL

  MAX  Profit = 
     - 2000 (OpenProduction01 + OpenProduction03 + OpenProduction05 + 
    OpenProduction08 + OpenProduction09 + OpenProduction10 + 
    OpenProduction11 + OpenProduction12 + OpenProduction13 + 
    OpenProduction15 + OpenProduction18 + OpenProduction19 + 
    OpenProduction20 + OpenProduction21 + OpenProduction22 + 
    OpenProduction23 + OpenProduction25 + OpenProduction26 + 
    OpenProduction27 + OpenProduction28 + OpenProduction29 + 
    OpenProduction30 + OpenProduction31 + OpenProduction34 + 
    OpenProduction35 + OpenProduction36 + OpenProduction37 + 
    OpenProduction39) - Ship10203 - 2 Ship10205 - 4 Ship10208
     - 3 Ship10209 - 5 Ship10210 - 4 Ship10211 - 15 Ship10213
     - 8 Ship10220 - Ship10221 - 6 Ship10225 - 14 Ship10227 - Ship10228
     - 13 Ship10231 - 3 Ship10236 - 10 Ship10237 - 4 Ship10239
     - 3 Ship10401 - 11 Ship10403 - 4 Ship10405 - Ship10409
     - 4 Ship10411 - 13 Ship10415 - 9 Ship10418 - Ship10419
     - 14 (Ship10421 + Ship10422) - 2 Ship10425 - 7 Ship10429
     - 4 Ship10431 - 7 Ship10434 - 5 Ship10437 - 3 Ship10508
     - Ship10509 - 14 Ship10510 - 15 Ship10511 - 6 Ship10512
     - 12 Ship10518 - 2 Ship10519 - 4 Ship10525 - 15 Ship10528
     - 13 Ship10529 - Ship10531 - 3 Ship10536 - 7 Ship11108
     - 8 Ship11113
     - 4 (Ship11115 + Ship11118) - 2 Ship11120 - 7 Ship11122
     - 14 (Ship11123 + Ship11128) - 5 Ship11130 - 9 Ship11131
     - 11 Ship11135 - 12 Ship11418 - 6 Ship11422 - 8 Ship11429
     - 13 Ship11431 - 6 Ship11437 - 5 Ship11439 - 13 Ship11512
     - 7 Ship11536 - 13 Ship11603 - 7 Ship11605 - Ship11609
     - 11 Ship11611 - 15 Ship11612 - 4 Ship11618 - 7 Ship11620
     - 8 Ship11621 - 2 Ship11622 - 9 Ship11625 - 5 Ship11626
     - 8 (Ship11628 + Ship11629) - 6 Ship11631 - Ship11634
     - 8 Ship11636 - Ship11637 - 15 Ship11639 - 5 Ship11709
     - 6 Ship11710 - 5 Ship11712 - 11 Ship11713 - 7 Ship11715
     - 10 Ship11718 - 3 Ship11720 - 12 Ship11723 - 6 Ship11728
     - 9 Ship11729 - 6 Ship11730 - 3 Ship11731 - 9 Ship11734
     - 10 Ship11739 - 11 Ship12303 - 15 Ship12305 - 4 Ship12308
     - 10 Ship12309 - 6 Ship12312 - 14 Ship12315 - 7 Ship12320
     - 11 Ship12325 - 15 Ship12326 - 6 Ship12327 - 12 Ship12328
     - 14 Ship12329 - 6 Ship12330 - 10 Ship12335 - 6 Ship12339
     - 15 Ship12605 - 14 Ship12608 - 3 Ship12610 - 5 Ship12612
     - 8 Ship12625 - 3 Ship12628 - 10 Ship12630
     - 2 (Ship12635 + Ship12639) - 11 Ship13211 - 2 Ship13212
     - 11 (Ship13222 + Ship13223) - 7 Ship13226 - 15 Ship13230
     - 14 Ship13231 - 4 Ship13235 - 13 Ship13236 - 15 Ship13237
     - 6 Ship13801 - 2 Ship13805 - 3 Ship13808 - 14 Ship13811
     - 7 Ship13815 - 10 Ship13819 - 13 Ship13821 - 2 Ship13822
     - 15 Ship13823 - 10 Ship13826 - 15 Ship13829 - 6 Ship13831
     - 9 Ship13835 - 11 Ship13837 - 3 Ship13839 - 14 Ship20103
     - 10 Ship20105 - 9 Ship20108 - 7 Ship20111
     - 10 (Ship20113 + Ship20119) - 9 Ship20122 - 8 Ship20123
     - 15 Ship20125 - 13 Ship20126
     - 12 (Ship20128 + Ship20136) - 14 Ship20139 - Ship20203
     - 2 Ship20205 - 4 Ship20208 - 3 Ship20209 - 5 Ship20210
     - 4 Ship20211 - 15 Ship20213 - 8 Ship20220 - Ship20221
     - 6 Ship20225 - 14 Ship20227 - Ship20228 - 13 Ship20231
     - 3 Ship20236 - 10 Ship20237 - 4 Ship20239 - 3 Ship20601
     - 9 Ship20605 - 5 Ship20613 - 13 Ship20615 - 15 Ship20619
     - 7 Ship20621 - 12 Ship20625 - 9 Ship20627 - 12 Ship20635
     - 10 Ship20703 - 11 Ship20705 - 4 Ship20708 - 3 Ship20709
     - 2 Ship20711 - 6 Ship20715 - 7 Ship20719 - Ship20720
     - 8 Ship20721 - 4 Ship20722 - 9 Ship20723 - 13 Ship20725
     - 2 (Ship20726 + Ship20729) - 9 Ship20730 - 5 Ship20731
     - 8 Ship20734 - 6 Ship20735 - 7 Ship20736 - 13 Ship20737
     - 4 Ship20739
     - 9 (Ship21303 + Ship21305) - 3 Ship21308 - 12 Ship21309
     - 5 Ship21310 - 14 Ship21312 - 15 Ship21315 - 5 Ship21318
     - 13 Ship21320 - 7 Ship21321 - 13 Ship21322 - 10 Ship21326
     - 7 Ship21327 - 4 Ship21328 - 12 Ship21330 - 8 Ship21331
     - 13 Ship21334 - 9 Ship21335 - 15 Ship21336 - 5 Ship21337
     - 11 Ship21339 - 12 Ship21418 - 6 Ship21422 - 8 Ship21429
     - 13 Ship21431 - 6 Ship21437 - 5 Ship21439 - 13 Ship21603
     - 7 Ship21605 - Ship21609 - 11 Ship21611 - 15 Ship21612
     - 4 Ship21618 - 7 Ship21620 - 8 Ship21621 - 2 Ship21622
     - 9 Ship21625 - 5 Ship21626
     - 8 (Ship21628 + Ship21629) - 6 Ship21631 - Ship21634
     - 8 Ship21636 - Ship21637 - 15 Ship21639 - 5 Ship21709
     - 6 Ship21710 - 5 Ship21712 - 11 Ship21713 - 7 Ship21715
     - 10 Ship21718 - 3 Ship21720 - 12 Ship21723 - 6 Ship21728
     - 9 Ship21729 - 6 Ship21730 - 3 Ship21731 - 9 Ship21734
     - 10 Ship21739 - 9 Ship21803 - 8 Ship21808 - 12 Ship21809
     - 11 Ship21810 - 9 Ship21812 - 2 Ship21819 - 14 Ship21820
     - 15 Ship21823 - 5 Ship21826 - 6 Ship21829 - 7 Ship21830
     - 10 Ship21834 - 3 Ship21835 - 11 Ship21836 - 15 Ship21837
     - 9 Ship21911 - 2 Ship21915 - 9 Ship21921 - 7 Ship21923
     - 9 Ship21925 - 12 Ship21930 - 15 Ship21931 - 9 Ship21935
     - 2 (Ship21937 + Ship21939) - 10 Ship22103 - Ship22105
     - 10 Ship22108 - 7 Ship22111 - 11 Ship22112 - 9 Ship22118
     - 12 Ship22120 - 15 Ship22122 - Ship22127 - Ship22128
     - 11 Ship22134
     - 5 (Ship22136 + Ship22137) - 12 Ship22139 - 5 Ship22203
     - 13 Ship22205 - Ship22208 - Ship22209 - 5 Ship22210 - Ship22212
     - 3 Ship22215 - 13 Ship22218 - 8 Ship22219 - 7 Ship22220
     - 5 Ship22223 - 10 Ship22225 - 2 Ship22226 - 3 Ship22227
     - 8 Ship22228 - Ship22229 - Ship22230 - 8 Ship22231 - 10 Ship22234
     - Ship22235 - 6 Ship22236 - 5 Ship22237 - 2 Ship22239
     - 15 Ship22401 - 2 Ship22409 - 11 Ship22410 - 3 Ship22411
     - 5 Ship22412 - 8 Ship22413 - 11 Ship22415 - 12 Ship22418
     - 6 Ship22420 - 14 Ship22421 - Ship22422 - Ship22423
     - 12 Ship22425
     - 10 (Ship22426 + Ship22427) - 13 Ship22428 - 5 Ship22429
     - 3 Ship22430 - 2 Ship22431 - 7 Ship22434 - 5 Ship22435
     - 12 Ship22436
     - 11 (Ship22437 + Ship22439) - 9 Ship23001 - 15 Ship23008
     - 4 Ship23009 - 7 Ship23010 - 4 Ship23020 - 12 Ship23021
     - 4 Ship23028 - 7 Ship23029 - 10 Ship23034 - 3 Ship23036
     - 2 Ship23037 - 14 Ship23039 - 6 Ship23115 - 13 Ship23118
     - 7 Ship23120 - 11 Ship23125 - 7 Ship23135 - 15 Ship23305
     - 8 Ship23313 - 5 Ship23325 - 11 Ship23327 - 14 Ship23337
     - 12 (Ship23339 + Ship23401) - 6 Ship23801 - 2 Ship23805
     - 3 Ship23808 - 14 Ship23811 - 7 Ship23815 - 10 Ship23819
     - 13 Ship23821 - 2 Ship23822 - 15 Ship23823 - 10 Ship23826
     - 15 Ship23829 - 6 Ship23831 - 9 Ship23835 - 11 Ship23837
     - 3 Ship23839 - 14 Ship30103 - 10 Ship30105 - 9 Ship30108
     - 7 Ship30111
     - 10 (Ship30113 + Ship30119) - 9 Ship30122 - 8 Ship30123
     - 15 Ship30125 - 13 Ship30126
     - 12 (Ship30128 + Ship30136) - 14 Ship30139 + 9 Ship30140
     + 7 Ship30340 - 3 Ship30508 - Ship30509 - 14 Ship30510
     - 15 Ship30511 - 6 Ship30512 - 12 Ship30518 - 2 Ship30519
     - 4 Ship30525 - 15 Ship30528 - 13 Ship30529 - Ship30531
     - 3 Ship30536 + 7 Ship30540 - Ship30840 + 5 Ship30940
     + 7 Ship31040 - 7 Ship31108 - 8 Ship31113
     - 4 (Ship31115 + Ship31118) - 2 Ship31120 - 7 Ship31122
     - 14 (Ship31123 + Ship31128) - 5 Ship31130 - 9 Ship31131
     - 11 Ship31135 + 12 Ship31140 + Ship31240
     - 9 (Ship31303 + Ship31305) - 3 Ship31308 - 12 Ship31309
     - 5 Ship31310 - 14 Ship31312 - 15 Ship31315 - 5 Ship31318
     - 13 Ship31320 - 7 Ship31321 - 13 Ship31322 - 10 Ship31326
     - 7 Ship31327 - 4 Ship31328 - 12 Ship31330 - 8 Ship31331
     - 13 Ship31334 - 9 Ship31335 - 15 Ship31336 - 5 Ship31337
     - 11 Ship31339 + 9 Ship31340 - 13 Ship31512 - 7 Ship31536
     + 6 Ship31540 - 9 Ship31803 - 8 Ship31808 - 12 Ship31809
     - 11 Ship31810 - 9 Ship31812 - 2 Ship31819 - 14 Ship31820
     - 15 Ship31823 - 5 Ship31826 - 6 Ship31829 - 7 Ship31830
     - 10 Ship31834 - 3 Ship31835 - 11 Ship31836 - 15 Ship31837
     + 10 Ship31840 - 9 Ship31911 - 2 Ship31915 - 9 Ship31921
     - 7 Ship31923 - 9 Ship31925 - 12 Ship31930 - 15 Ship31931
     - 9 Ship31935
     - 2 (Ship31937 + Ship31939 + Ship31940) + 12 Ship32040
     - 10 Ship32103 - Ship32105 - 10 Ship32108 - 7 Ship32111
     - 11 Ship32112 - 9 Ship32118 - 12 Ship32120 - 15 Ship32122
     - Ship32127 - Ship32128 - 11 Ship32134
     - 5 (Ship32136 + Ship32137) - 12 Ship32139 + 3 Ship32140
     - 5 Ship32203 - 13 Ship32205 - Ship32208 - Ship32209 - 5 Ship32210
     - Ship32212 - 3 Ship32215 - 13 Ship32218 - 8 Ship32219
     - 7 Ship32220 - 5 Ship32223 - 10 Ship32225 - 2 Ship32226
     - 3 Ship32227 - 8 Ship32228 - Ship32229 - Ship32230 - 8 Ship32231
     - 10 Ship32234 - Ship32235 - 6 Ship32236 - 5 Ship32237
     - 2 Ship32239 - 11 Ship32303 - 15 Ship32305 - 4 Ship32308
     - 10 Ship32309 - 6 Ship32312 - 14 Ship32315 - 7 Ship32320
     - 11 Ship32325 - 15 Ship32326 - 6 Ship32327 - 12 Ship32328
     - 14 Ship32329 - 6 Ship32330 - 10 Ship32335 - 6 Ship32339
     + Ship32340 + 9 Ship32540 - 15 Ship32605 - 14 Ship32608
     - 3 Ship32610 - 5 Ship32612 - 8 Ship32625 - 3 Ship32628
     - 10 Ship32630
     - 2 (Ship32635 + Ship32639) + 3 Ship32640 + 11 Ship32740
     + 9 Ship32840 + 3 Ship32940 - 9 Ship33001 - 15 Ship33008
     - 4 Ship33009 - 7 Ship33010 - 4 Ship33020 - 12 Ship33021
     - 4 Ship33028 - 7 Ship33029 - 10 Ship33034 - 3 Ship33036
     - 2 Ship33037 - 14 Ship33039 + 10 Ship33040 - 6 Ship33115
     - 13 Ship33118 - 7 Ship33120 - 11 Ship33125 - 7 Ship33135
     + 7 Ship33140 - 12 Ship33401 + 7 Ship33440 + 9 Ship33540
     + 12 Ship33640 + 6 Ship33740 + 2 Ship33940 ;

SUBJECT TO

  BOMConstr011: 2 Ship30140  =  Ship10401 + Ship13801 ;
  BOMConstr012: 2 Ship30140  =  Ship20101 + Ship20601 + Ship22401
     + Ship23001 + Ship23401 + Ship23801 ;
  BOMConstr031: 2 Ship30340  =  Ship10203 + Ship10403 + Ship11603
     + Ship12303 ;
  BOMConstr032: 2 Ship30340  =  Ship20103 + Ship20203 + Ship20703
     + Ship21303 + Ship21603 + Ship21803 + Ship22103 + Ship22203 ;
  BOMConstr051: 2 Ship30540  =  Ship10205 + Ship10405 + Ship10505
     + Ship11605 + Ship12305 + Ship12605 + Ship13805 ;
  BOMConstr052: 2 Ship30540  =  Ship20105 + Ship20205 + Ship20605
     + Ship20705 + Ship21305 + Ship21605 + Ship22105 + Ship22205
     + Ship23305 + Ship23805 ;
  BOMConstr081: 2 Ship30840  =  Ship10208 + Ship10508 + Ship11108
     + Ship12308 + Ship12608 + Ship13808 ;
  BOMConstr082: 2 Ship30840  =  Ship20108 + Ship20208 + Ship20708
     + Ship21308 + Ship21808 + Ship22108 + Ship22208 + Ship23008
     + Ship23808 ;
  BOMConstr091: 2 Ship30940  =  Ship10209 + Ship10409 + Ship10509
     + Ship11609 + Ship11709 + Ship12309 ;
  BOMConstr092: 2 Ship30940  =  Ship20209 + Ship20709 + Ship21309
     + Ship21609 + Ship21709 + Ship21809 + Ship22209 + Ship22409
     + Ship23009 ;
  BOMConstr101: 2 Ship31040  =  Ship10210 + Ship10510 + Ship11710
     + Ship12610 ;
  BOMConstr102: 2 Ship31040  =  Ship20210 + Ship21310 + Ship21710
     + Ship21810 + Ship22210 + Ship22410 + Ship23010 ;
  BOMConstr111: 2 Ship31140  =  Ship10211 + Ship10411 + Ship10511
     + Ship11111 + Ship11611 + Ship13211 + Ship13811 ;
  BOMConstr112: 2 Ship31140  =  Ship20111 + Ship20211 + Ship20711
     + Ship21611 + Ship21911 + Ship22111 + Ship22411 + Ship23811 ;
  BOMConstr121: 2 Ship31240  =  Ship10512 + Ship11512 + Ship11612
     + Ship11712 + Ship12312 + Ship12612 + Ship13212 ;
  BOMConstr122: 2 Ship31240  =  Ship21312 + Ship21612 + Ship21712
     + Ship21812 + Ship22112 + Ship22212 + Ship22412 ;
  BOMConstr131: 2 Ship31340  =  Ship10213 + Ship11113 + Ship11713 ;
  BOMConstr132: 2 Ship31340  =  Ship20113 + Ship20213 + Ship20613
     + Ship21313 + Ship21713 + Ship22413 + Ship23313 ;
  BOMConstr151: 2 Ship31540  =  Ship10415 + Ship11115 + Ship11515
     + Ship11715 + Ship12315 + Ship13815 ;
  BOMConstr152: 2 Ship31540  =  Ship20615 + Ship20715 + Ship21315
     + Ship21715 + Ship21915 + Ship22215 + Ship22415 + Ship23115
     + Ship23815 ;
  BOMConstr181: 2 Ship31840  =  Ship10418 + Ship10518 + Ship11118
     + Ship11418 + Ship11618 + Ship11718 ;
  BOMConstr182: 2 Ship31840  =  Ship21318 + Ship21418 + Ship21618
     + Ship21718 + Ship21818 + Ship22118 + Ship22218 + Ship22418
     + Ship23118 ;
  BOMConstr191: 2 Ship31940  =  Ship10419 + Ship10519 + Ship13819 ;
  BOMConstr192: 2 Ship31940  =  Ship20119 + Ship20619 + Ship20719
     + Ship21819 + Ship21919 + Ship22219 + Ship23819 ;
  BOMConstr201: 2 Ship32040  =  Ship10220 + Ship11120 + Ship11620
     + Ship11720 + Ship12320 ;
  BOMConstr202: 2 Ship32040  =  Ship20220 + Ship20720 + Ship21320
     + Ship21620 + Ship21720 + Ship21820 + Ship22120 + Ship22220
     + Ship22420 + Ship23020 + Ship23120 ;
  BOMConstr211: 2 Ship32140  =  Ship10221 + Ship10421 + Ship11621
     + Ship13821 ;
  BOMConstr212: 2 Ship32140  =  Ship20221 + Ship20621 + Ship20721
     + Ship21321 + Ship21621 + Ship21921 + Ship22121 + Ship22421
     + Ship23021 + Ship23821 ;
  BOMConstr221: 2 Ship32240  =  Ship10422 + Ship11122 + Ship11422
     + Ship11622 + Ship13222 + Ship13822 ;
  BOMConstr222: 2 Ship32240  =  Ship20122 + Ship20722 + Ship21322
     + Ship21422 + Ship21622 + Ship22122 + Ship22222 + Ship22422
     + Ship23822 ;
  BOMConstr231: 2 Ship32340  =  Ship11123 + Ship11723 + Ship12323
     + Ship13223 + Ship13823 ;
  BOMConstr232: 2 Ship32340  =  Ship20123 + Ship20723 + Ship21723
     + Ship21823 + Ship21923 + Ship22223 + Ship22423 + Ship23823 ;
  BOMConstr251: 2 Ship32540  =  Ship10225 + Ship10425 + Ship10525
     + Ship11625 + Ship12325 + Ship12625 ;
  BOMConstr252: 2 Ship32540  =  Ship20125 + Ship20225 + Ship20625
     + Ship20725 + Ship21625 + Ship21925 + Ship22225 + Ship22425
     + Ship23125 + Ship23325 ;
  BOMConstr261: 2 Ship32640  =  Ship11626 + Ship12326 + Ship12626
     + Ship13226 + Ship13826 ;
  BOMConstr262: 2 Ship32640  =  Ship20126 + Ship20726 + Ship21326
     + Ship21626 + Ship21826 + Ship22226 + Ship22426 + Ship23826 ;
  BOMConstr271: 2 Ship32740  =  Ship10227 + Ship12327 ;
  BOMConstr272: 2 Ship32740  =  Ship20227 + Ship20627 + Ship21327
     + Ship22127 + Ship22227 + Ship22427 + Ship22727 + Ship23327 ;
  BOMConstr281: 2 Ship32840  =  Ship10228 + Ship10528 + Ship11128
     + Ship11628 + Ship11728 + Ship12328 + Ship12628 ;
  BOMConstr282: 2 Ship32840  =  Ship20128 + Ship20228 + Ship21328
     + Ship21628 + Ship21728 + Ship22128 + Ship22228 + Ship22428
     + Ship23028 ;
  BOMConstr291: 2 Ship32940  =  Ship10429 + Ship10529 + Ship11429
     + Ship11629 + Ship11729 + Ship12329 + Ship13829 ;
  BOMConstr292: 2 Ship32940  =  Ship20729 + Ship21429 + Ship21629
     + Ship21729 + Ship21829 + Ship22229 + Ship22429 + Ship23029
     + Ship23829 ;
  BOMConstr301: 2 Ship33040  =  Ship11130 + Ship11730 + Ship12330
     + Ship12630 + Ship13230 ;
  BOMConstr302: 2 Ship33040  =  Ship20730 + Ship21330 + Ship21730
     + Ship21830 + Ship21930 + Ship22230 + Ship22430 + Ship23030 ;
  BOMConstr311: 2 Ship33140  =  Ship10231 + Ship10431 + Ship10531
     + Ship11131 + Ship11431 + Ship11631 + Ship11731 + Ship13231
     + Ship13831 ;
  BOMConstr312: 2 Ship33140  =  Ship20231 + Ship20731 + Ship21331
     + Ship21431 + Ship21631 + Ship21731 + Ship21931 + Ship22231
     + Ship22431 + Ship23131 + Ship23831 ;
  BOMConstr341: 2 Ship33440  =  Ship10434 + Ship11634 + Ship11734 ;
  BOMConstr342: 2 Ship33440  =  Ship20734 + Ship21334 + Ship21634
     + Ship21734 + Ship21834 + Ship22134 + Ship22234 + Ship22434
     + Ship23034 + Ship23434 ;
  BOMConstr351: 2 Ship33540  =  Ship11135 + Ship12335 + Ship12635
     + Ship13235 + Ship13835 ;
  BOMConstr352: 2 Ship33540  =  Ship20635 + Ship20735 + Ship21335
     + Ship21835 + Ship21935 + Ship22235 + Ship22435 + Ship23135
     + Ship23835 ;
  BOMConstr361: 2 Ship33640  =  Ship10236 + Ship10536 + Ship11536
     + Ship11636 + Ship13236 ;
  BOMConstr362: 2 Ship33640  =  Ship20136 + Ship20236 + Ship20736
     + Ship21336 + Ship21636 + Ship21836 + Ship22136 + Ship22236
     + Ship22436 + Ship23036 ;
  BOMConstr371: 2 Ship33740  =  Ship10237 + Ship10437 + Ship11437
     + Ship11637 + Ship13237 + Ship13837 ;
  BOMConstr372: 2 Ship33740  =  Ship20237 + Ship20737 + Ship21337
     + Ship21437 + Ship21637 + Ship21837 + Ship21937 + Ship22137
     + Ship22237 + Ship22437 + Ship23037 + Ship23337 + Ship23837 ;
  BOMConstr391: 2 Ship33940  =  Ship10239 + Ship11439 + Ship11639
     + Ship11739 + Ship12339 + Ship12639 + Ship13839 ;
  BOMConstr392: 2 Ship33940  =  Ship20139 + Ship20239 + Ship20739
     + Ship21339 + Ship21439 + Ship21639 + Ship21739 + Ship21939
     + Ship22139 + Ship22239 + Ship22439 + Ship23039 + Ship23339
     + Ship23839 ;
  DemConstr: Ship30140 + Ship30340 + Ship30540 + Ship30840 + Ship30940
     + Ship31040 + Ship31140 + Ship31240 + Ship31340 + Ship31540
     + Ship31840 + Ship31940 + Ship32040 + Ship32140 + Ship32240
     + Ship32340 + Ship32540 + Ship32640 + Ship32740 + Ship32840
     + Ship32940 + Ship33040 + Ship33140 + Ship33440 + Ship33540
     + Ship33640 + Ship33740 + Ship33940  <=  31720 ;
  PlantConstr01: Ship30140  <=  5000 OpenProduction01 ;
  PlantConstr03: Ship30340  <=  5000 OpenProduction03 ;
  PlantConstr05: Ship30540  <=  5000 OpenProduction05 ;
  PlantConstr08: Ship30840  <=  5000 OpenProduction08 ;
  PlantConstr09: Ship30940  <=  5000 OpenProduction09 ;
  PlantConstr10: Ship31040  <=  5000 OpenProduction10 ;
  PlantConstr11: Ship31140  <=  5000 OpenProduction11 ;
  PlantConstr12: Ship31240  <=  5000 OpenProduction12 ;
  PlantConstr13: Ship31340  <=  5000 OpenProduction13 ;
  PlantConstr15: Ship31540  <=  5000 OpenProduction15 ;
  PlantConstr18: Ship31840  <=  5000 OpenProduction18 ;
  PlantConstr19: Ship31940  <=  5000 OpenProduction19 ;
  PlantConstr20: Ship32040  <=  5000 OpenProduction20 ;
  PlantConstr21: Ship32140  <=  5000 OpenProduction21 ;
  PlantConstr22: Ship32240  <=  5000 OpenProduction22 ;
  PlantConstr23: Ship32340  <=  5000 OpenProduction23 ;
  PlantConstr25: Ship32540  <=  5000 OpenProduction25 ;
  PlantConstr26: Ship32640  <=  5000 OpenProduction26 ;
  PlantConstr27: Ship32740  <=  5000 OpenProduction27 ;
  PlantConstr28: Ship32840  <=  5000 OpenProduction28 ;
  PlantConstr29: Ship32940  <=  5000 OpenProduction29 ;
  PlantConstr30: Ship33040  <=  5000 OpenProduction30 ;
  PlantConstr31: Ship33140  <=  5000 OpenProduction31 ;
  PlantConstr34: Ship33440  <=  5000 OpenProduction34 ;
  PlantConstr35: Ship33540  <=  5000 OpenProduction35 ;
  PlantConstr36: Ship33640  <=  5000 OpenProduction36 ;
  PlantConstr37: Ship33740  <=  5000 OpenProduction37 ;
  PlantConstr39: Ship33940  <=  5000 OpenProduction39 ;

BINARY
    OpenProduction01
    OpenProduction03
    OpenProduction05
    OpenProduction08
    OpenProduction09
    OpenProduction10
    OpenProduction11
    OpenProduction12
    OpenProduction13
    OpenProduction15
    OpenProduction18
    OpenProduction19
    OpenProduction20
    OpenProduction21
    OpenProduction22
    OpenProduction23
    OpenProduction25
    OpenProduction26
    OpenProduction27
    OpenProduction28
    OpenProduction29
    OpenProduction30
    OpenProduction31
    OpenProduction34
    OpenProduction35
    OpenProduction36
    OpenProduction37
    OpenProduction39

END


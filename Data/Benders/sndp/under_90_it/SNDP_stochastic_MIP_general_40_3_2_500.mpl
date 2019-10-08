!   
!   
    SNDP_stochastic_MIP_general_instances\SNDP_stochastic_MIP_general_40_3_2_500.mpl
!   
!   Generated with the MPL Modeling System
!   Copyright (c) 1988-2016  Maximal Software
!   
!   Date:               October 16, 2018
!   Time:               18:09
!   
!   Constraints:        109
!   Variables:          715
!   Integers:           36
!   Nonzeros:           565
!   Density:            0.7 %
!   

TITLE SNDP_stochastic_general ;

DECISION VARIABLES

    OpenProduction01 OpenProduction02 OpenProduction03 
    OpenProduction04 OpenProduction05 OpenProduction06 
    OpenProduction07 OpenProduction08 OpenProduction09 
    OpenProduction10 OpenProduction11 OpenProduction12 
    OpenProduction13 OpenProduction14 OpenProduction15 
    OpenProduction16 OpenProduction18 OpenProduction19 
    OpenProduction20 OpenProduction21 OpenProduction22 
    OpenProduction23 OpenProduction24 OpenProduction25 
    OpenProduction26 OpenProduction27 OpenProduction29 
    OpenProduction30 OpenProduction31 OpenProduction32 
    OpenProduction33 OpenProduction34 OpenProduction36 
    OpenProduction37 OpenProduction38 OpenProduction39 Ship10301 
    Ship10302 Ship10303 Ship10304 Ship10305 Ship10306 Ship10307 
    Ship10308 Ship10309 Ship10310 Ship10311 Ship10312 Ship10313 
    Ship10315 Ship10316 Ship10318 Ship10319 Ship10320 Ship10321 
    Ship10322 Ship10326 Ship10327 Ship10329 Ship10330 Ship10331 
    Ship10332 Ship10333 Ship10336 Ship10337 Ship10338 Ship10339 
    Ship10906 Ship10907 Ship10909 Ship10924 Ship10930 Ship10931 
    Ship10937 Ship11502 Ship11505 Ship11506 Ship11507 Ship11509 
    Ship11510 Ship11511 Ship11512 Ship11513 Ship11514 Ship11515 
    Ship11516 Ship11518 Ship11519 Ship11520 Ship11521 Ship11522 
    Ship11523 Ship11524 Ship11525 Ship11526 Ship11529 Ship11530 
    Ship11531 Ship11532 Ship11533 Ship11534 Ship11536 Ship11537 
    Ship11538 Ship11539 Ship12801 Ship12802 Ship12803 Ship12806 
    Ship12807 Ship12810 Ship12811 Ship12813 Ship12814 Ship12816 
    Ship12818 Ship12819 Ship12821 Ship12822 Ship12823 Ship12824 
    Ship12826 Ship12827 Ship12829 Ship12833 Ship12834 Ship12836 
    Ship12837 Ship12838 Ship12839 Ship13201 Ship13202 Ship13205 
    Ship13206 Ship13207 Ship13209 Ship13210 Ship13211 Ship13212 
    Ship13213 Ship13214 Ship13216 Ship13219 Ship13220 Ship13221 
    Ship13223 Ship13224 Ship13225 Ship13227 Ship13230 Ship13231 
    Ship13232 Ship13233 Ship13236 Ship13237 Ship13238 Ship13401 
    Ship13407 Ship13425 Ship13434 Ship13807 Ship13830 Ship13838 
    Ship13901 Ship13902 Ship13904 Ship13905 Ship13906 Ship13907 
    Ship13909 Ship13910 Ship13911 Ship13912 Ship13913 Ship13914 
    Ship13916 Ship13919 Ship13920 Ship13921 Ship13923 Ship13924 
    Ship13925 Ship13927 Ship13929 Ship13930 Ship13931 Ship13932 
    Ship13933 Ship13934 Ship13936 Ship13937 Ship13938 Ship13939 
    Ship20301 Ship20302 Ship20303 Ship20304 Ship20305 Ship20306 
    Ship20307 Ship20308 Ship20309 Ship20310 Ship20311 Ship20312 
    Ship20313 Ship20315 Ship20316 Ship20318 Ship20319 Ship20320 
    Ship20321 Ship20322 Ship20326 Ship20327 Ship20329 Ship20330 
    Ship20331 Ship20332 Ship20333 Ship20336 Ship20337 Ship20338 
    Ship20339 Ship20801 Ship20802 Ship20804 Ship20805 Ship20807 
    Ship20808 Ship20809 Ship20810 Ship20811 Ship20812 Ship20813 
    Ship20814 Ship20815 Ship20816 Ship20818 Ship20819 Ship20820 
    Ship20821 Ship20822 Ship20823 Ship20824 Ship20825 Ship20826 
    Ship20827 Ship20829 Ship20831 Ship20832 Ship20833 Ship20834 
    Ship20836 Ship20837 Ship20838 Ship20839 Ship21701 Ship21702 
    Ship21703 Ship21704 Ship21705 Ship21706 Ship21707 Ship21708 
    Ship21709 Ship21710 Ship21711 Ship21712 Ship21713 Ship21714 
    Ship21716 Ship21718 Ship21721 Ship21722 Ship21723 Ship21724 
    Ship21725 Ship21726 Ship21727 Ship21729 Ship21730 Ship21731 
    Ship21732 Ship21733 Ship21734 Ship21736 Ship21737 Ship21739 
    Ship21801 Ship21802 Ship21804 Ship21805 Ship21806 Ship21807 
    Ship21809 Ship21810 Ship21811 Ship21812 Ship21813 Ship21814 
    Ship21816 Ship21818 Ship21819 Ship21820 Ship21821 Ship21822 
    Ship21823 Ship21824 Ship21825 Ship21826 Ship21827 Ship21829 
    Ship21830 Ship21831 Ship21832 Ship21833 Ship21834 Ship21836 
    Ship21837 Ship21838 Ship21839 Ship22001 Ship22002 Ship22005 
    Ship22009 Ship22010 Ship22012 Ship22019 Ship22020 Ship22022 
    Ship22023 Ship22025 Ship22026 Ship22027 Ship22202 Ship22205 
    Ship22206 Ship22207 Ship22210 Ship22211 Ship22212 Ship22213 
    Ship22214 Ship22216 Ship22219 Ship22221 Ship22222 Ship22223 
    Ship22224 Ship22225 Ship22227 Ship22229 Ship22230 Ship22231 
    Ship22232 Ship22233 Ship22234 Ship22237 Ship22238 Ship22239 
    Ship22403 Ship22405 Ship22413 Ship22419 Ship22424 Ship22434 
    Ship22437 Ship22601 Ship22604 Ship22605 Ship22606 Ship22607 
    Ship22609 Ship22610 Ship22616 Ship22621 Ship22622 Ship22623 
    Ship22626 Ship22627 Ship22629 Ship22630 Ship22631 Ship22632 
    Ship22636 Ship22638 Ship22639 Ship23001 Ship23010 Ship23012 
    Ship23024 Ship23025 Ship23030 Ship23031 Ship23036 Ship23501 
    Ship23503 Ship23504 Ship23506 Ship23507 Ship23508 Ship23509 
    Ship23511 Ship23513 Ship23514 Ship23518 Ship23519 Ship23520 
    Ship23521 Ship23522 Ship23523 Ship23524 Ship23526 Ship23527 
    Ship23529 Ship23530 Ship23531 Ship23536 Ship23537 Ship23539 
    Ship30140 Ship30240 Ship30301 Ship30302 Ship30304 Ship30305 
    Ship30306 Ship30307 Ship30308 Ship30309 Ship30310 Ship30311 
    Ship30312 Ship30313 Ship30315 Ship30316 Ship30318 Ship30319 
    Ship30320 Ship30321 Ship30322 Ship30326 Ship30327 Ship30329 
    Ship30330 Ship30331 Ship30332 Ship30333 Ship30336 Ship30337 
    Ship30338 Ship30339 Ship30340 Ship30440 Ship30540 Ship30640 
    Ship30740 Ship30801 Ship30802 Ship30804 Ship30805 Ship30807 
    Ship30809 Ship30810 Ship30811 Ship30812 Ship30813 Ship30814 
    Ship30815 Ship30816 Ship30818 Ship30819 Ship30820 Ship30821 
    Ship30822 Ship30823 Ship30824 Ship30825 Ship30826 Ship30827 
    Ship30829 Ship30831 Ship30832 Ship30833 Ship30834 Ship30836 
    Ship30837 Ship30838 Ship30839 Ship30840 Ship30906 Ship30907 
    Ship30924 Ship30930 Ship30931 Ship30937 Ship30940 Ship31040 
    Ship31140 Ship31240 Ship31340 Ship31440 Ship31502 Ship31505 
    Ship31506 Ship31507 Ship31509 Ship31510 Ship31511 Ship31512 
    Ship31513 Ship31514 Ship31516 Ship31518 Ship31519 Ship31520 
    Ship31521 Ship31522 Ship31523 Ship31524 Ship31525 Ship31526 
    Ship31529 Ship31530 Ship31531 Ship31532 Ship31533 Ship31534 
    Ship31536 Ship31537 Ship31538 Ship31539 Ship31540 Ship31640 
    Ship31801 Ship31802 Ship31804 Ship31805 Ship31806 Ship31807 
    Ship31809 Ship31810 Ship31811 Ship31812 Ship31813 Ship31814 
    Ship31816 Ship31819 Ship31820 Ship31821 Ship31822 Ship31823 
    Ship31824 Ship31825 Ship31826 Ship31827 Ship31829 Ship31830 
    Ship31831 Ship31832 Ship31833 Ship31834 Ship31836 Ship31837 
    Ship31838 Ship31839 Ship31840 Ship31940 Ship32001 Ship32002 
    Ship32005 Ship32009 Ship32010 Ship32012 Ship32019 Ship32022 
    Ship32023 Ship32025 Ship32026 Ship32027 Ship32040 Ship32140 
    Ship32202 Ship32205 Ship32206 Ship32207 Ship32210 Ship32211 
    Ship32212 Ship32213 Ship32214 Ship32216 Ship32219 Ship32221 
    Ship32223 Ship32224 Ship32225 Ship32227 Ship32229 Ship32230 
    Ship32231 Ship32232 Ship32233 Ship32234 Ship32237 Ship32238 
    Ship32239 Ship32240 Ship32340 Ship32403 Ship32405 Ship32413 
    Ship32419 Ship32434 Ship32437 Ship32440 Ship32540 Ship32601 
    Ship32604 Ship32605 Ship32606 Ship32607 Ship32609 Ship32610 
    Ship32616 Ship32621 Ship32622 Ship32623 Ship32627 Ship32629 
    Ship32630 Ship32631 Ship32632 Ship32636 Ship32638 Ship32639 
    Ship32640 Ship32740 Ship32940 Ship33001 Ship33010 Ship33012 
    Ship33024 Ship33025 Ship33031 Ship33036 Ship33040 Ship33140 
    Ship33201 Ship33202 Ship33205 Ship33206 Ship33207 Ship33209 
    Ship33210 Ship33211 Ship33212 Ship33213 Ship33214 Ship33216 
    Ship33219 Ship33220 Ship33221 Ship33223 Ship33224 Ship33225 
    Ship33227 Ship33230 Ship33231 Ship33233 Ship33236 Ship33237 
    Ship33238 Ship33240 Ship33340 Ship33401 Ship33407 Ship33425 
    Ship33440 Ship33640 Ship33740 Ship33807 Ship33830 Ship33840 
    Ship33901 Ship33902 Ship33904 Ship33905 Ship33906 Ship33907 
    Ship33909 Ship33910 Ship33911 Ship33912 Ship33913 Ship33914 
    Ship33916 Ship33919 Ship33920 Ship33921 Ship33923 Ship33924 
    Ship33925 Ship33927 Ship33929 Ship33930 Ship33931 Ship33932 
    Ship33933 Ship33934 Ship33936 Ship33937 Ship33938 Ship33940 ;

MODEL

  MAX  Profit = 
     - 2000 (OpenProduction01 + OpenProduction02 + OpenProduction03 + 
    OpenProduction04 + OpenProduction05 + OpenProduction06 + 
    OpenProduction07 + OpenProduction08 + OpenProduction09 + 
    OpenProduction10 + OpenProduction11 + OpenProduction12 + 
    OpenProduction13 + OpenProduction14 + OpenProduction15 + 
    OpenProduction16 + OpenProduction18 + OpenProduction19 + 
    OpenProduction20 + OpenProduction21 + OpenProduction22 + 
    OpenProduction23 + OpenProduction24 + OpenProduction25 + 
    OpenProduction26 + OpenProduction27 + OpenProduction29 + 
    OpenProduction30 + OpenProduction31 + OpenProduction32 + 
    OpenProduction33 + OpenProduction34 + OpenProduction36 + 
    OpenProduction37 + OpenProduction38 + OpenProduction39)
     - 14 Ship10301 - 5 Ship10302 - 4 Ship10304 - Ship10305
     - 7 Ship10306 - 8 Ship10307 - 4 Ship10308 - 12 Ship10309
     - 2 Ship10310 - 4 Ship10311 - Ship10312 - 11 Ship10313
     - 7 Ship10315 - 6 Ship10316 - 9 Ship10318 - 6 Ship10319
     - 5 Ship10320 - 8 Ship10321 - 7 Ship10322 - 5 Ship10326
     - 3 Ship10327 - 9 Ship10329
     - 8 (Ship10330 + Ship10331) - 14 Ship10332 - 7 Ship10333
     - 9 Ship10336 - 10 Ship10337
     - 11 (Ship10338 + Ship10339) - 14 Ship10906 - 15 Ship10907
     - 8 Ship10924
     - 11 (Ship10930 + Ship10931) - 3 Ship10937
     - 11 (Ship11502 + Ship11505) - 6 Ship11506 - 5 Ship11507
     - 14 Ship11509 - 3 Ship11510 - 13 Ship11511 - 11 Ship11512
     - 4 Ship11513 - 10 Ship11514 - 12 Ship11516 - 9 Ship11518
     - 12 Ship11519 - Ship11520 - 4 Ship11521 - Ship11522
     - 12 Ship11523 - 5 Ship11524 - 13 Ship11525 - 11 Ship11526
     - 6 Ship11529 - 8 Ship11530 - 4 Ship11531 - Ship11532
     - 10 Ship11533 - 8 Ship11534 - 12 Ship11536 - 15 Ship11537
     - 10 Ship11538 - 11 Ship11539 - 14 Ship12801 - 12 Ship12802
     - Ship12803 - 13 Ship12806 - 15 Ship12807 - 4 Ship12810
     - 12 Ship12811 - 4 Ship12813 - 15 Ship12814 - 4 Ship12816
     - 6 Ship12818 - 9 Ship12819 - 7 Ship12821 - 11 Ship12822
     - 7 Ship12823 - 14 Ship12824 - 4 Ship12826 - 6 Ship12827
     - 4 Ship12829 - 14 Ship12833
     - 10 (Ship12834 + Ship12836) - 5 Ship12837 - 13 Ship12838
     - 12 Ship12839 - 5 Ship13201 - 7 Ship13202 - Ship13205
     - 3 Ship13206 - 2 Ship13207 - 11 Ship13209
     - 4 (Ship13210 + Ship13211) - 13 Ship13212 - 14 Ship13213
     - 11 Ship13214 - 12 Ship13216 - 5 Ship13219 - Ship13220
     - 13 Ship13221 - 6 Ship13223 - 8 Ship13224 - 14 Ship13225
     - 8 Ship13227 - 13 Ship13230 - 12 Ship13231 - 6 Ship13233
     - 10 Ship13236 - 5 Ship13237 - 2 Ship13238 - 13 Ship13401
     - 9 Ship13407 - 12 Ship13425 - 2 Ship13807 - 4 Ship13830
     - 7 Ship13901 - 2 Ship13902 - 15 Ship13904 - 8 Ship13905
     - 4 Ship13906 - 11 Ship13907 - 2 Ship13909 - 15 Ship13910
     - 5 Ship13911 - 13 Ship13912 - 4 Ship13913 - 8 Ship13914
     - 10 Ship13916 - 2 Ship13919 - 8 Ship13920 - 15 Ship13921
     - 2 Ship13923 - 7 Ship13924 - 4 Ship13925 - 5 Ship13927
     - 15 Ship13929 - 12 Ship13930 - 9 Ship13931 - Ship13932
     - 7 Ship13933 - 15 Ship13934 - 10 Ship13936 - 12 Ship13937
     - 14 (Ship13938 + Ship20301) - 5 Ship20302 - 4 Ship20304
     - Ship20305 - 7 Ship20306 - 8 Ship20307 - 4 Ship20308
     - 12 Ship20309 - 2 Ship20310 - 4 Ship20311 - Ship20312
     - 11 Ship20313 - 7 Ship20315 - 6 Ship20316 - 9 Ship20318
     - 6 Ship20319 - 5 Ship20320 - 8 Ship20321 - 7 Ship20322
     - 5 Ship20326 - 3 Ship20327 - 9 Ship20329
     - 8 (Ship20330 + Ship20331) - 14 Ship20332 - 7 Ship20333
     - 9 Ship20336 - 10 Ship20337
     - 11 (Ship20338 + Ship20339) - 6 Ship20801 - 13 Ship20802
     - 15 Ship20804 - 13 Ship20805 - 6 Ship20807 - 3 Ship20809
     - Ship20810 - 6 Ship20811 - 12 Ship20812 - 10 Ship20813
     - 6 Ship20814 - 12 Ship20815 - 4 Ship20816 - 10 Ship20818
     - 8 Ship20819 - 10 Ship20820 - Ship20821 - 3 Ship20822
     - 14 Ship20823 - 2 Ship20824 - 10 Ship20825 - 13 Ship20826
     - 7 Ship20827 - 8 Ship20829 - 3 Ship20831 - 15 Ship20832
     - 7 Ship20833 - 5 Ship20834 - 11 Ship20836 - 8 Ship20837
     - 4 Ship20838
     - 3 (Ship20839 + Ship21701) - 4 Ship21702 - 2 Ship21703
     - 3 Ship21704 - 7 Ship21705 - 6 Ship21706 - Ship21707
     - 5 Ship21708 - 6 Ship21709 - 2 Ship21710
     - 12 (Ship21711 + Ship21712) - 11 Ship21713 - 14 Ship21714
     - 6 Ship21716 - 15 Ship21718 - Ship21721 - 4 Ship21722
     - 7 Ship21723 - 6 Ship21724 - 13 Ship21725 - 4 Ship21726
     - 14 Ship21727 - 8 Ship21729
     - 2 (Ship21730 + Ship21731) - 14 Ship21732 - 11 Ship21733
     - 7 Ship21734 - Ship21736 - 8 Ship21737 - 3 Ship21739
     - 10 Ship21801 - 4 Ship21802 - 7 Ship21804 - 11 Ship21805
     - 9 Ship21806 - 4 Ship21807 - 10 Ship21809 - 4 Ship21810
     - 7 Ship21811 - Ship21812 - 2 Ship21813 - 12 Ship21814
     - 3 Ship21816 - 14 Ship21819
     - 12 (Ship21820 + Ship21821) - 13 Ship21822 - Ship21823
     - 2 (Ship21824 + Ship21825) - 8 Ship21826 - 12 Ship21827
     - 15 Ship21829 - 9 Ship21830 - 11 Ship21831 - 9 Ship21832
     - 11 Ship21833 - 2 Ship21834 - 10 Ship21836
     - 13 (Ship21837 + Ship21838) - 5 Ship21839 - 6 Ship22001
     - 9 Ship22002 - 3 Ship22005 - 14 Ship22009 - 9 Ship22010
     - 2 Ship22012 - 12 Ship22019 - 11 Ship22022 - 12 Ship22023
     - Ship22025 - 12 Ship22026 - 15 Ship22027 - 3 Ship22202
     - 12 Ship22205 - 8 Ship22206 - 15 Ship22207 - 12 Ship22210
     - 13 Ship22211 - 4 Ship22212 - 9 Ship22213
     - 11 (Ship22214 + Ship22216) - 10 Ship22219 - 5 Ship22221
     - 9 Ship22223 - 7 Ship22224 - 3 Ship22225 - 5 Ship22227
     - 11 Ship22229 - 13 Ship22230 - 9 Ship22231 - 5 Ship22232
     - 8 Ship22233 - 14 Ship22234 - 4 Ship22237
     - 15 (Ship22238 + Ship22239) - 9 Ship22403 - 10 Ship22405
     - 2 Ship22413 - 11 Ship22419 - 14 Ship22434 - 12 Ship22437
     - 5 Ship22601 - 11 Ship22604 - 4 Ship22605 - 10 Ship22606
     - 2 Ship22607 - 13 Ship22609 - 7 Ship22610 - 11 Ship22616
     - 15 Ship22621 - 4 Ship22622 - 5 Ship22623 - 2 Ship22627
     - 13 Ship22629 - 11 Ship22630 - 8 Ship22631 - 10 Ship22632
     - 5 Ship22636 - 12 Ship22638 - 5 Ship22639 - Ship23001
     - 5 Ship23010 - 4 Ship23012 - 14 Ship23024 - 9 Ship23025
     - 13 Ship23031 - 9 Ship23036 - 12 Ship23501 - Ship23503
     - 8 Ship23504 - 13 Ship23506 - 10 Ship23507
     - 6 (Ship23508 + Ship23509) - 7 Ship23511 - Ship23513
     - 5 Ship23514 - 12 Ship23518 - 6 Ship23519 - 13 Ship23520
     - Ship23521 - 4 Ship23522 - 7 Ship23523 - 2 Ship23524
     - 15 Ship23526 - 5 Ship23527 - 12 Ship23529 - Ship23530
     - 12 Ship23531 - 4 Ship23536 - 14 Ship23537 - 4 Ship23539
     + 3 Ship30140 + 4 Ship30240 - 14 Ship30301 - 5 Ship30302
     - 4 Ship30304 - Ship30305 - 7 Ship30306 - 8 Ship30307
     - 4 Ship30308 - 12 Ship30309 - 2 Ship30310 - 4 Ship30311
     - Ship30312 - 11 Ship30313 - 7 Ship30315 - 6 Ship30316
     - 9 Ship30318 - 6 Ship30319 - 5 Ship30320 - 8 Ship30321
     - 7 Ship30322 - 5 Ship30326 - 3 Ship30327 - 9 Ship30329
     - 8 (Ship30330 + Ship30331) - 14 Ship30332 - 7 Ship30333
     - 9 Ship30336 - 10 Ship30337
     - 11 (Ship30338 + Ship30339) - 2 Ship30340 + 9 Ship30440
     - Ship30540 + 2 Ship30640 - 2 Ship30740 - 6 Ship30801
     - 13 Ship30802 - 15 Ship30804 - 13 Ship30805 - 6 Ship30807
     - 3 Ship30809 - Ship30810 - 6 Ship30811 - 12 Ship30812
     - 10 Ship30813 - 6 Ship30814 - 12 Ship30815 - 4 Ship30816
     - 10 Ship30818 - 8 Ship30819 - 10 Ship30820 - Ship30821
     - 3 Ship30822 - 14 Ship30823 - 2 Ship30824 - 10 Ship30825
     - 13 Ship30826 - 7 Ship30827 - 8 Ship30829 - 3 Ship30831
     - 15 Ship30832 - 7 Ship30833 - 5 Ship30834 - 11 Ship30836
     - 8 Ship30837 - 4 Ship30838 - 3 Ship30839 + 5 Ship30840
     - 14 Ship30906 - 15 Ship30907 - 8 Ship30924
     - 11 (Ship30930 + Ship30931) - 3 Ship30937 + 12 Ship30940
     + Ship31040 + 9 Ship31140 - Ship31240 + 12 Ship31340 + 4 Ship31440
     - 11 (Ship31502 + Ship31505) - 6 Ship31506 - 5 Ship31507
     - 14 Ship31509 - 3 Ship31510 - 13 Ship31511 - 11 Ship31512
     - 4 Ship31513 - 10 Ship31514 - 12 Ship31516 - 9 Ship31518
     - 12 Ship31519 - Ship31520 - 4 Ship31521 - Ship31522
     - 12 Ship31523 - 5 Ship31524 - 13 Ship31525 - 11 Ship31526
     - 6 Ship31529 - 8 Ship31530 - 4 Ship31531 - Ship31532
     - 10 Ship31533 - 8 Ship31534 - 12 Ship31536 - 15 Ship31537
     - 10 Ship31538 - 11 Ship31539 + 6 Ship31540 + 11 Ship31640
     - 10 Ship31801 - 4 Ship31802 - 7 Ship31804 - 11 Ship31805
     - 9 Ship31806 - 4 Ship31807 - 10 Ship31809 - 4 Ship31810
     - 7 Ship31811 - Ship31812 - 2 Ship31813 - 12 Ship31814
     - 3 Ship31816 - 14 Ship31819
     - 12 (Ship31820 + Ship31821) - 13 Ship31822 - Ship31823
     - 2 (Ship31824 + Ship31825) - 8 Ship31826 - 12 Ship31827
     - 15 Ship31829 - 9 Ship31830 - 11 Ship31831 - 9 Ship31832
     - 11 Ship31833 - 2 Ship31834 - 10 Ship31836
     - 13 (Ship31837 + Ship31838) - 5 Ship31839 + Ship31840
     + 10 Ship31940 - 6 Ship32001 - 9 Ship32002 - 3 Ship32005
     - 14 Ship32009 - 9 Ship32010 - 2 Ship32012 - 12 Ship32019
     - 11 Ship32022 - 12 Ship32023 - Ship32025 - 12 Ship32026
     - 15 Ship32027 - 2 Ship32040 + 5 Ship32140 - 3 Ship32202
     - 12 Ship32205 - 8 Ship32206 - 15 Ship32207 - 12 Ship32210
     - 13 Ship32211 - 4 Ship32212 - 9 Ship32213
     - 11 (Ship32214 + Ship32216) - 10 Ship32219 - 5 Ship32221
     - 9 Ship32223 - 7 Ship32224 - 3 Ship32225 - 5 Ship32227
     - 11 Ship32229 - 13 Ship32230 - 9 Ship32231 - 5 Ship32232
     - 8 Ship32233 - 14 Ship32234 - 4 Ship32237
     - 15 (Ship32238 + Ship32239) + 9 Ship32240 + 10 Ship32340
     - 9 Ship32403 - 10 Ship32405 - 2 Ship32413 - 11 Ship32419
     - 14 Ship32434 - 12 Ship32437 + 10 Ship32440 + 9 Ship32540
     - 5 Ship32601 - 11 Ship32604 - 4 Ship32605 - 10 Ship32606
     - 2 Ship32607 - 13 Ship32609 - 7 Ship32610 - 11 Ship32616
     - 15 Ship32621 - 4 Ship32622 - 5 Ship32623 - 2 Ship32627
     - 13 Ship32629 - 11 Ship32630 - 8 Ship32631 - 10 Ship32632
     - 5 Ship32636 - 12 Ship32638 - 5 Ship32639 + 7 Ship32640
     + 6 Ship32740 - 2 Ship32940 - Ship33001 - 5 Ship33010
     - 4 Ship33012 - 14 Ship33024 - 9 Ship33025 - 13 Ship33031
     - 9 Ship33036 + 9 Ship33040 + 12 Ship33140 - 5 Ship33201
     - 7 Ship33202 - Ship33205 - 3 Ship33206 - 2 Ship33207
     - 11 Ship33209
     - 4 (Ship33210 + Ship33211) - 13 Ship33212 - 14 Ship33213
     - 11 Ship33214 - 12 Ship33216 - 5 Ship33219 - Ship33220
     - 13 Ship33221 - 6 Ship33223 - 8 Ship33224 - 14 Ship33225
     - 8 Ship33227 - 13 Ship33230 - 12 Ship33231 - 6 Ship33233
     - 10 Ship33236 - 5 Ship33237 - 2 Ship33238 + 11 Ship33240
     + 12 Ship33340 - 13 Ship33401 - 9 Ship33407 - 12 Ship33425
     + 11 Ship33440 + 9 Ship33740 - 2 Ship33807 - 4 Ship33830
     + 6 Ship33840 - 7 Ship33901 - 2 Ship33902 - 15 Ship33904
     - 8 Ship33905 - 4 Ship33906 - 11 Ship33907 - 2 Ship33909
     - 15 Ship33910 - 5 Ship33911 - 13 Ship33912 - 4 Ship33913
     - 8 Ship33914 - 10 Ship33916 - 2 Ship33919 - 8 Ship33920
     - 15 Ship33921 - 2 Ship33923 - 7 Ship33924 - 4 Ship33925
     - 5 Ship33927 - 15 Ship33929 - 12 Ship33930 - 9 Ship33931
     - Ship33932 - 7 Ship33933 - 15 Ship33934 - 10 Ship33936
     - 12 Ship33937 - 14 Ship33938 + 11 Ship33940 ;

SUBJECT TO

  BOMConstr011: 2 Ship30140  =  Ship10301 + Ship12801 + Ship13201
     + Ship13401 + Ship13901 ;
  BOMConstr012: 2 Ship30140  =  Ship20301 + Ship20801 + Ship21701
     + Ship21801 + Ship22001 + Ship22601 + Ship23001 + Ship23501 ;
  BOMConstr021: 2 Ship30240  =  Ship10302 + Ship11502 + Ship12802
     + Ship13202 + Ship13902 ;
  BOMConstr022: 2 Ship30240  =  Ship20302 + Ship20802 + Ship21702
     + Ship21802 + Ship22002 + Ship22202 ;
  BOMConstr031: 2 Ship30340  =  Ship10303 + Ship12803 ;
  BOMConstr032: 2 Ship30340  =  Ship20303 + Ship21703 + Ship22403
     + Ship23503 ;
  BOMConstr041: 2 Ship30440  =  Ship10304 + Ship13904 ;
  BOMConstr042: 2 Ship30440  =  Ship20304 + Ship20804 + Ship21704
     + Ship21804 + Ship22604 + Ship23504 ;
  BOMConstr051: 2 Ship30540  =  Ship10305 + Ship11505 + Ship13205
     + Ship13905 ;
  BOMConstr052: 2 Ship30540  =  Ship20305 + Ship20805 + Ship21705
     + Ship21805 + Ship22005 + Ship22205 + Ship22405 + Ship22605 ;
  BOMConstr061: 2 Ship30640  =  Ship10306 + Ship10906 + Ship11506
     + Ship12806 + Ship13206 + Ship13906 ;
  BOMConstr062: 2 Ship30640  =  Ship20306 + Ship21706 + Ship21806
     + Ship22206 + Ship22606 + Ship23506 ;
  BOMConstr071: 2 Ship30740  =  Ship10307 + Ship10907 + Ship11507
     + Ship12807 + Ship13207 + Ship13407 + Ship13807 + Ship13907 ;
  BOMConstr072: 2 Ship30740  =  Ship20307 + Ship20807 + Ship21707
     + Ship21807 + Ship22207 + Ship22607 + Ship23507 ;
  BOMConstr081: 2 Ship30840  =  Ship10308 ;
  BOMConstr082: 2 Ship30840  =  Ship20308 + Ship20808 + Ship21708
     + Ship23508 ;
  BOMConstr091: 2 Ship30940  =  Ship10309 + Ship10909 + Ship11509
     + Ship13209 + Ship13909 ;
  BOMConstr092: 2 Ship30940  =  Ship20309 + Ship20809 + Ship21709
     + Ship21809 + Ship22009 + Ship22609 + Ship23509 ;
  BOMConstr101: 2 Ship31040  =  Ship10310 + Ship11510 + Ship12810
     + Ship13210 + Ship13910 ;
  BOMConstr102: 2 Ship31040  =  Ship20310 + Ship20810 + Ship21710
     + Ship21810 + Ship22010 + Ship22210 + Ship22610 + Ship23010 ;
  BOMConstr111: 2 Ship31140  =  Ship10311 + Ship11511 + Ship12811
     + Ship13211 + Ship13911 ;
  BOMConstr112: 2 Ship31140  =  Ship20311 + Ship20811 + Ship21711
     + Ship21811 + Ship22211 + Ship23511 ;
  BOMConstr121: 2 Ship31240  =  Ship10312 + Ship11512 + Ship13212
     + Ship13912 ;
  BOMConstr122: 2 Ship31240  =  Ship20312 + Ship20812 + Ship21712
     + Ship21812 + Ship22012 + Ship22212 + Ship23012 ;
  BOMConstr131: 2 Ship31340  =  Ship10313 + Ship11513 + Ship12813
     + Ship13213 + Ship13913 ;
  BOMConstr132: 2 Ship31340  =  Ship20313 + Ship20813 + Ship21713
     + Ship21813 + Ship22213 + Ship22413 + Ship23513 ;
  BOMConstr141: 2 Ship31440  =  Ship11514 + Ship12814 + Ship13214
     + Ship13914 ;
  BOMConstr142: 2 Ship31440  =  Ship20814 + Ship21714 + Ship21814
     + Ship22214 + Ship23514 ;
  BOMConstr151: 2 Ship31540  =  Ship10315 + Ship11515 ;
  BOMConstr152: 2 Ship31540  =  Ship20315 + Ship20815 ;
  BOMConstr161: 2 Ship31640  =  Ship10316 + Ship11516 + Ship12816
     + Ship13216 + Ship13916 ;
  BOMConstr162: 2 Ship31640  =  Ship20316 + Ship20816 + Ship21716
     + Ship21816 + Ship22216 + Ship22616 ;
  BOMConstr181: 2 Ship31840  =  Ship10318 + Ship11518 + Ship12818 ;
  BOMConstr182: 2 Ship31840  =  Ship20318 + Ship20818 + Ship21718
     + Ship21818 + Ship23518 ;
  BOMConstr191: 2 Ship31940  =  Ship10319 + Ship11519 + Ship12819
     + Ship13219 + Ship13919 ;
  BOMConstr192: 2 Ship31940  =  Ship20319 + Ship20819 + Ship21819
     + Ship22019 + Ship22219 + Ship22419 + Ship23519 ;
  BOMConstr201: 2 Ship32040  =  Ship10320 + Ship11520 + Ship13220
     + Ship13920 ;
  BOMConstr202: 2 Ship32040  =  Ship20320 + Ship20820 + Ship21820
     + Ship22020 + Ship23520 ;
  BOMConstr211: 2 Ship32140  =  Ship10321 + Ship11521 + Ship12821
     + Ship13221 + Ship13921 ;
  BOMConstr212: 2 Ship32140  =  Ship20321 + Ship20821 + Ship21721
     + Ship21821 + Ship22221 + Ship22621 + Ship23521 ;
  BOMConstr221: 2 Ship32240  =  Ship10322 + Ship11522 + Ship12822 ;
  BOMConstr222: 2 Ship32240  =  Ship20322 + Ship20822 + Ship21722
     + Ship21822 + Ship22022 + Ship22222 + Ship22622 + Ship23522 ;
  BOMConstr231: 2 Ship32340  =  Ship11523 + Ship12823 + Ship13223
     + Ship13923 ;
  BOMConstr232: 2 Ship32340  =  Ship20823 + Ship21723 + Ship21823
     + Ship22023 + Ship22223 + Ship22623 + Ship23523 ;
  BOMConstr241: 2 Ship32440  =  Ship10924 + Ship11524 + Ship12824
     + Ship13224 + Ship13924 ;
  BOMConstr242: 2 Ship32440  =  Ship20824 + Ship21724 + Ship21824
     + Ship22224 + Ship22424 + Ship23024 + Ship23524 ;
  BOMConstr251: 2 Ship32540  =  Ship11525 + Ship13225 + Ship13425
     + Ship13925 ;
  BOMConstr252: 2 Ship32540  =  Ship20825 + Ship21725 + Ship21825
     + Ship22025 + Ship22225 + Ship23025 ;
  BOMConstr261: 2 Ship32640  =  Ship10326 + Ship11526 + Ship12826 ;
  BOMConstr262: 2 Ship32640  =  Ship20326 + Ship20826 + Ship21726
     + Ship21826 + Ship22026 + Ship22626 + Ship23526 ;
  BOMConstr271: 2 Ship32740  =  Ship10327 + Ship12827 + Ship13227
     + Ship13927 ;
  BOMConstr272: 2 Ship32740  =  Ship20327 + Ship20827 + Ship21727
     + Ship21827 + Ship22027 + Ship22227 + Ship22627 + Ship23527 ;
  BOMConstr291: 2 Ship32940  =  Ship10329 + Ship11529 + Ship12829
     + Ship13929 ;
  BOMConstr292: 2 Ship32940  =  Ship20329 + Ship20829 + Ship21729
     + Ship21829 + Ship22229 + Ship22629 + Ship23529 ;
  BOMConstr301: 2 Ship33040  =  Ship10330 + Ship10930 + Ship11530
     + Ship13230 + Ship13830 + Ship13930 ;
  BOMConstr302: 2 Ship33040  =  Ship20330 + Ship21730 + Ship21830
     + Ship22230 + Ship22630 + Ship23030 + Ship23530 ;
  BOMConstr311: 2 Ship33140  =  Ship10331 + Ship10931 + Ship11531
     + Ship13231 + Ship13931 ;
  BOMConstr312: 2 Ship33140  =  Ship20331 + Ship20831 + Ship21731
     + Ship21831 + Ship22231 + Ship22631 + Ship23031 + Ship23531 ;
  BOMConstr321: 2 Ship33240  =  Ship10332 + Ship11532 + Ship13232
     + Ship13932 ;
  BOMConstr322: 2 Ship33240  =  Ship20332 + Ship20832 + Ship21732
     + Ship21832 + Ship22232 + Ship22632 ;
  BOMConstr331: 2 Ship33340  =  Ship10333 + Ship11533 + Ship12833
     + Ship13233 + Ship13933 ;
  BOMConstr332: 2 Ship33340  =  Ship20333 + Ship20833 + Ship21733
     + Ship21833 + Ship22233 ;
  BOMConstr341: 2 Ship33440  =  Ship11534 + Ship12834 + Ship13434
     + Ship13934 ;
  BOMConstr342: 2 Ship33440  =  Ship20834 + Ship21734 + Ship21834
     + Ship22234 + Ship22434 ;
  BOMConstr361: 2 Ship33640  =  Ship10336 + Ship11536 + Ship12836
     + Ship13236 + Ship13936 ;
  BOMConstr362: 2 Ship33640  =  Ship20336 + Ship20836 + Ship21736
     + Ship21836 + Ship22636 + Ship23036 + Ship23536 ;
  BOMConstr371: 2 Ship33740  =  Ship10337 + Ship10937 + Ship11537
     + Ship12837 + Ship13237 + Ship13937 ;
  BOMConstr372: 2 Ship33740  =  Ship20337 + Ship20837 + Ship21737
     + Ship21837 + Ship22237 + Ship22437 + Ship23537 ;
  BOMConstr381: 2 Ship33840  =  Ship10338 + Ship11538 + Ship12838
     + Ship13238 + Ship13838 + Ship13938 ;
  BOMConstr382: 2 Ship33840  =  Ship20338 + Ship20838 + Ship21838
     + Ship22238 + Ship22638 ;
  BOMConstr391: 2 Ship33940  =  Ship10339 + Ship11539 + Ship12839
     + Ship13939 ;
  BOMConstr392: 2 Ship33940  =  Ship20339 + Ship20839 + Ship21739
     + Ship21839 + Ship22239 + Ship22639 + Ship23539 ;
  DemConstr: Ship30140 + Ship30240 + Ship30340 + Ship30440 + Ship30540
     + Ship30640 + Ship30740 + Ship30840 + Ship30940 + Ship31040
     + Ship31140 + Ship31240 + Ship31340 + Ship31440 + Ship31540
     + Ship31640 + Ship31840 + Ship31940 + Ship32040 + Ship32140
     + Ship32240 + Ship32340 + Ship32440 + Ship32540 + Ship32640
     + Ship32740 + Ship32940 + Ship33040 + Ship33140 + Ship33240
     + Ship33340 + Ship33440 + Ship33640 + Ship33740 + Ship33840
     + Ship33940  <=  31131 ;
  PlantConstr01: Ship30140  <=  5000 OpenProduction01 ;
  PlantConstr02: Ship30240  <=  5000 OpenProduction02 ;
  PlantConstr03: Ship30340  <=  5000 OpenProduction03 ;
  PlantConstr04: Ship30440  <=  5000 OpenProduction04 ;
  PlantConstr05: Ship30540  <=  5000 OpenProduction05 ;
  PlantConstr06: Ship30640  <=  5000 OpenProduction06 ;
  PlantConstr07: Ship30740  <=  5000 OpenProduction07 ;
  PlantConstr08: Ship30840  <=  5000 OpenProduction08 ;
  PlantConstr09: Ship30940  <=  5000 OpenProduction09 ;
  PlantConstr10: Ship31040  <=  5000 OpenProduction10 ;
  PlantConstr11: Ship31140  <=  5000 OpenProduction11 ;
  PlantConstr12: Ship31240  <=  5000 OpenProduction12 ;
  PlantConstr13: Ship31340  <=  5000 OpenProduction13 ;
  PlantConstr14: Ship31440  <=  5000 OpenProduction14 ;
  PlantConstr15: Ship31540  <=  5000 OpenProduction15 ;
  PlantConstr16: Ship31640  <=  5000 OpenProduction16 ;
  PlantConstr18: Ship31840  <=  5000 OpenProduction18 ;
  PlantConstr19: Ship31940  <=  5000 OpenProduction19 ;
  PlantConstr20: Ship32040  <=  5000 OpenProduction20 ;
  PlantConstr21: Ship32140  <=  5000 OpenProduction21 ;
  PlantConstr22: Ship32240  <=  5000 OpenProduction22 ;
  PlantConstr23: Ship32340  <=  5000 OpenProduction23 ;
  PlantConstr24: Ship32440  <=  5000 OpenProduction24 ;
  PlantConstr25: Ship32540  <=  5000 OpenProduction25 ;
  PlantConstr26: Ship32640  <=  5000 OpenProduction26 ;
  PlantConstr27: Ship32740  <=  5000 OpenProduction27 ;
  PlantConstr29: Ship32940  <=  5000 OpenProduction29 ;
  PlantConstr30: Ship33040  <=  5000 OpenProduction30 ;
  PlantConstr31: Ship33140  <=  5000 OpenProduction31 ;
  PlantConstr32: Ship33240  <=  5000 OpenProduction32 ;
  PlantConstr33: Ship33340  <=  5000 OpenProduction33 ;
  PlantConstr34: Ship33440  <=  5000 OpenProduction34 ;
  PlantConstr36: Ship33640  <=  5000 OpenProduction36 ;
  PlantConstr37: Ship33740  <=  5000 OpenProduction37 ;
  PlantConstr38: Ship33840  <=  5000 OpenProduction38 ;
  PlantConstr39: Ship33940  <=  5000 OpenProduction39 ;

BINARY
    OpenProduction01
    OpenProduction02
    OpenProduction03
    OpenProduction04
    OpenProduction05
    OpenProduction06
    OpenProduction07
    OpenProduction08
    OpenProduction09
    OpenProduction10
    OpenProduction11
    OpenProduction12
    OpenProduction13
    OpenProduction14
    OpenProduction15
    OpenProduction16
    OpenProduction18
    OpenProduction19
    OpenProduction20
    OpenProduction21
    OpenProduction22
    OpenProduction23
    OpenProduction24
    OpenProduction25
    OpenProduction26
    OpenProduction27
    OpenProduction29
    OpenProduction30
    OpenProduction31
    OpenProduction32
    OpenProduction33
    OpenProduction34
    OpenProduction36
    OpenProduction37
    OpenProduction38
    OpenProduction39

END


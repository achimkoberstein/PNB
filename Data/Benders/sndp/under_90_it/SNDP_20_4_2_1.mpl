!   
!   
    SNDP_stochastic_MIP_general_instances\SNDP_stochastic_MIP_general_20_4_2_1.mpl
!   
!   Generated with the MPL Modeling System
!   Copyright (c) 1988-2016  Maximal Software
!   
!   Date:               October 16, 2018
!   Time:               18:08
!   
!   Constraints:        77
!   Variables:          175
!   Integers:           19
!   Nonzeros:           213
!   Density:            1.6 %
!   

TITLE SNDP_stochastic_general ;

DECISION VARIABLES

    OpenProduction01 OpenProduction02 OpenProduction03 
    OpenProduction04 OpenProduction05 OpenProduction06 
    OpenProduction07 OpenProduction08 OpenProduction09 
    OpenProduction10 OpenProduction11 OpenProduction12 
    OpenProduction13 OpenProduction14 OpenProduction15 
    OpenProduction16 OpenProduction17 OpenProduction18 
    OpenProduction19 Ship10501 Ship10502 Ship10503 Ship10505 Ship10507 
    Ship10512 Ship10513 Ship10514 Ship10515 Ship10518 Ship10707 
    Ship10713 Ship10714 Ship10715 Ship10717 Ship10718 Ship10719 
    Ship10801 Ship10803 Ship10808 Ship10816 Ship11001 Ship11004 
    Ship11005 Ship11006 Ship11007 Ship11009 Ship11010 Ship11018 
    Ship11111 Ship20202 Ship20207 Ship20217 Ship20501 Ship20502 
    Ship20503 Ship20505 Ship20507 Ship20512 Ship20513 Ship20514 
    Ship20515 Ship20518 Ship20801 Ship20803 Ship20808 Ship20816 
    Ship21001 Ship21004 Ship21005 Ship21006 Ship21007 Ship21009 
    Ship21010 Ship21018 Ship21111 Ship21901 Ship21902 Ship21905 
    Ship21906 Ship21909 Ship21912 Ship21917 Ship21918 Ship21919 
    Ship30501 Ship30502 Ship30503 Ship30505 Ship30507 Ship30512 
    Ship30513 Ship30514 Ship30515 Ship30518 Ship30707 Ship30713 
    Ship30714 Ship30715 Ship30717 Ship30718 Ship30719 Ship30801 
    Ship30803 Ship30808 Ship30816 Ship31001 Ship31004 Ship31005 
    Ship31006 Ship31007 Ship31009 Ship31010 Ship31018 Ship31111 
    Ship31203 Ship31209 Ship31212 Ship31217 Ship40120 Ship40207 
    Ship40217 Ship40220 Ship40320 Ship40420 Ship40501 Ship40502 
    Ship40503 Ship40507 Ship40512 Ship40513 Ship40514 Ship40515 
    Ship40518 Ship40520 Ship40620 Ship40713 Ship40714 Ship40715 
    Ship40717 Ship40718 Ship40719 Ship40720 Ship40801 Ship40803 
    Ship40816 Ship40820 Ship40920 Ship41001 Ship41004 Ship41005 
    Ship41006 Ship41007 Ship41009 Ship41018 Ship41020 Ship41120 
    Ship41203 Ship41209 Ship41217 Ship41220 Ship41320 Ship41420 
    Ship41520 Ship41620 Ship41720 Ship41820 Ship41901 Ship41902 
    Ship41905 Ship41906 Ship41909 Ship41912 Ship41917 Ship41918 
    Ship41920 ;

MODEL

  MAX  Profit = 
     - 2000 (OpenProduction01 + OpenProduction02 + OpenProduction03 + 
    OpenProduction04 + OpenProduction05 + OpenProduction06 + 
    OpenProduction07 + OpenProduction08 + OpenProduction09 + 
    OpenProduction10 + OpenProduction11 + OpenProduction12 + 
    OpenProduction13 + OpenProduction14 + OpenProduction15 + 
    OpenProduction16 + OpenProduction17 + OpenProduction18 + 
    OpenProduction19) - Ship10501 - 3 Ship10502 - 13 Ship10503
     - 10 Ship10507 - 8 Ship10512 - 3 Ship10513 - 10 Ship10514
     - 9 Ship10515 - 2 Ship10518 - 12 Ship10713 - 14 Ship10714
     - 13 Ship10715
     - 8 (Ship10717 + Ship10718) - 12 Ship10719 - 9 Ship10801
     - 2 Ship10803 - 13 Ship10816 - 12 Ship11001 - 14 Ship11004
     - 10 Ship11005 - 9 Ship11006 - 6 Ship11007 - 5 Ship11009
     - 3 Ship11018 - 6 Ship20207 - 8 Ship20217 - Ship20501
     - 3 Ship20502 - 13 Ship20503 - 10 Ship20507 - 8 Ship20512
     - 3 Ship20513 - 10 Ship20514 - 9 Ship20515 - 2 Ship20518
     - 9 Ship20801 - 2 Ship20803 - 13 Ship20816 - 12 Ship21001
     - 14 Ship21004 - 10 Ship21005 - 9 Ship21006 - 6 Ship21007
     - 5 Ship21009 - 3 Ship21018 - 6 Ship21901 - 13 Ship21902
     - 12 Ship21905 - 7 Ship21906 - 3 Ship21909 - 2 Ship21912
     - 15 Ship21917 - 6 Ship21918 - Ship30501 - 3 Ship30502
     - 13 Ship30503 - 10 Ship30507 - 8 Ship30512 - 3 Ship30513
     - 10 Ship30514 - 9 Ship30515 - 2 Ship30518 - 12 Ship30713
     - 14 Ship30714 - 13 Ship30715
     - 8 (Ship30717 + Ship30718) - 12 Ship30719 - 9 Ship30801
     - 2 Ship30803 - 13 Ship30816 - 12 Ship31001 - 14 Ship31004
     - 10 Ship31005 - 9 Ship31006 - 6 Ship31007 - 5 Ship31009
     - 3 Ship31018 - 5 Ship31203 - 6 Ship31209 - Ship31217
     - 2 Ship40120 - 6 Ship40207 - 8 Ship40217 + 5 Ship40220
     + 8 Ship40320 + 11 Ship40420 - Ship40501 - 3 Ship40502
     - 13 Ship40503 - 10 Ship40507 - 8 Ship40512 - 3 Ship40513
     - 10 Ship40514 - 9 Ship40515 - 2 Ship40518 + 2 Ship40520
     + 6 Ship40620 - 12 Ship40713 - 14 Ship40714 - 13 Ship40715
     - 8 (Ship40717 + Ship40718) - 12 Ship40719 + 9 Ship40720
     - 9 Ship40801 - 2 Ship40803 - 13 Ship40816 - Ship40820
     + 8 Ship40920 - 12 Ship41001 - 14 Ship41004 - 10 Ship41005
     - 9 Ship41006 - 6 Ship41007 - 5 Ship41009 - 3 Ship41018
     + 11 Ship41020 + 7 Ship41120 - 5 Ship41203 - 6 Ship41209
     - Ship41217 + 9 Ship41220 - Ship41320 + 4 Ship41420 + 7 Ship41520
     - Ship41620 + 9 Ship41820 - 6 Ship41901 - 13 Ship41902
     - 12 Ship41905 - 7 Ship41906 - 3 Ship41909 - 2 Ship41912
     - 15 Ship41917 - 6 Ship41918 + Ship41920 ;

SUBJECT TO

  BOMConstr011: 3 Ship40120  =  Ship10501 + Ship10801 + Ship11001 ;
  BOMConstr012: 3 Ship40120  =  Ship20501 + Ship20801 + Ship21001
     + Ship21901 ;
  BOMConstr013: 2 Ship40120  =  Ship30501 + Ship30801 + Ship31001 ;
  BOMConstr021: 3 Ship40220  =  Ship10502 ;
  BOMConstr022: 3 Ship40220  =  Ship20202 + Ship20502 + Ship21902 ;
  BOMConstr023: 2 Ship40220  =  Ship30502 ;
  BOMConstr031: 3 Ship40320  =  Ship10503 + Ship10803 ;
  BOMConstr032: 3 Ship40320  =  Ship20503 + Ship20803 ;
  BOMConstr033: 2 Ship40320  =  Ship30503 + Ship30803 + Ship31203 ;
  BOMConstr041: 3 Ship40420  =  Ship11004 ;
  BOMConstr042: 3 Ship40420  =  Ship21004 ;
  BOMConstr043: 2 Ship40420  =  Ship31004 ;
  BOMConstr051: 3 Ship40520  =  Ship10505 + Ship11005 ;
  BOMConstr052: 3 Ship40520  =  Ship20505 + Ship21005 + Ship21905 ;
  BOMConstr053: 2 Ship40520  =  Ship30505 + Ship31005 ;
  BOMConstr061: 3 Ship40620  =  Ship11006 ;
  BOMConstr062: 3 Ship40620  =  Ship21006 + Ship21906 ;
  BOMConstr063: 2 Ship40620  =  Ship31006 ;
  BOMConstr071: 3 Ship40720  =  Ship10507 + Ship10707 + Ship11007 ;
  BOMConstr072: 3 Ship40720  =  Ship20207 + Ship20507 + Ship21007 ;
  BOMConstr073: 2 Ship40720  =  Ship30507 + Ship30707 + Ship31007 ;
  BOMConstr081: 3 Ship40820  =  Ship10808 ;
  BOMConstr082: 3 Ship40820  =  Ship20808 ;
  BOMConstr083: 2 Ship40820  =  Ship30808 ;
  BOMConstr091: 3 Ship40920  =  Ship11009 ;
  BOMConstr092: 3 Ship40920  =  Ship21009 + Ship21909 ;
  BOMConstr093: 2 Ship40920  =  Ship31009 + Ship31209 ;
  BOMConstr101: 3 Ship41020  =  Ship11010 ;
  BOMConstr102: 3 Ship41020  =  Ship21010 ;
  BOMConstr103: 2 Ship41020  =  Ship31010 ;
  BOMConstr111: 3 Ship41120  =  Ship11111 ;
  BOMConstr112: 3 Ship41120  =  Ship21111 ;
  BOMConstr113: 2 Ship41120  =  Ship31111 ;
  BOMConstr121: 3 Ship41220  =  Ship10512 ;
  BOMConstr122: 3 Ship41220  =  Ship20512 + Ship21912 ;
  BOMConstr123: 2 Ship41220  =  Ship30512 + Ship31212 ;
  BOMConstr131: 3 Ship41320  =  Ship10513 + Ship10713 ;
  BOMConstr132: 3 Ship41320  =  Ship20513 ;
  BOMConstr133: 2 Ship41320  =  Ship30513 + Ship30713 ;
  BOMConstr141: 3 Ship41420  =  Ship10514 + Ship10714 ;
  BOMConstr142: 3 Ship41420  =  Ship20514 ;
  BOMConstr143: 2 Ship41420  =  Ship30514 + Ship30714 ;
  BOMConstr151: 3 Ship41520  =  Ship10515 + Ship10715 ;
  BOMConstr152: 3 Ship41520  =  Ship20515 ;
  BOMConstr153: 2 Ship41520  =  Ship30515 + Ship30715 ;
  BOMConstr161: 3 Ship41620  =  Ship10816 ;
  BOMConstr162: 3 Ship41620  =  Ship20816 ;
  BOMConstr163: 2 Ship41620  =  Ship30816 ;
  BOMConstr171: 3 Ship41720  =  Ship10717 ;
  BOMConstr172: 3 Ship41720  =  Ship20217 + Ship21917 ;
  BOMConstr173: 2 Ship41720  =  Ship30717 + Ship31217 ;
  BOMConstr181: 3 Ship41820  =  Ship10518 + Ship10718 + Ship11018 ;
  BOMConstr182: 3 Ship41820  =  Ship20518 + Ship21018 + Ship21918 ;
  BOMConstr183: 2 Ship41820  =  Ship30518 + Ship30718 + Ship31018 ;
  BOMConstr191: 3 Ship41920  =  Ship10719 ;
  BOMConstr192: 3 Ship41920  =  Ship21919 ;
  BOMConstr193: 2 Ship41920  =  Ship30719 ;
  DemConstr: Ship40120 + Ship40220 + Ship40320 + Ship40420 + Ship40520
     + Ship40620 + Ship40720 + Ship40820 + Ship40920 + Ship41020
     + Ship41120 + Ship41220 + Ship41320 + Ship41420 + Ship41520
     + Ship41620 + Ship41720 + Ship41820 + Ship41920  <=  11636 ;
  PlantConstr01: Ship40120  <=  5000 OpenProduction01 ;
  PlantConstr02: Ship40220  <=  5000 OpenProduction02 ;
  PlantConstr03: Ship40320  <=  5000 OpenProduction03 ;
  PlantConstr04: Ship40420  <=  5000 OpenProduction04 ;
  PlantConstr05: Ship40520  <=  5000 OpenProduction05 ;
  PlantConstr06: Ship40620  <=  5000 OpenProduction06 ;
  PlantConstr07: Ship40720  <=  5000 OpenProduction07 ;
  PlantConstr08: Ship40820  <=  5000 OpenProduction08 ;
  PlantConstr09: Ship40920  <=  5000 OpenProduction09 ;
  PlantConstr10: Ship41020  <=  5000 OpenProduction10 ;
  PlantConstr11: Ship41120  <=  5000 OpenProduction11 ;
  PlantConstr12: Ship41220  <=  5000 OpenProduction12 ;
  PlantConstr13: Ship41320  <=  5000 OpenProduction13 ;
  PlantConstr14: Ship41420  <=  5000 OpenProduction14 ;
  PlantConstr15: Ship41520  <=  5000 OpenProduction15 ;
  PlantConstr16: Ship41620  <=  5000 OpenProduction16 ;
  PlantConstr17: Ship41720  <=  5000 OpenProduction17 ;
  PlantConstr18: Ship41820  <=  5000 OpenProduction18 ;
  PlantConstr19: Ship41920  <=  5000 OpenProduction19 ;

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
    OpenProduction17
    OpenProduction18
    OpenProduction19

END


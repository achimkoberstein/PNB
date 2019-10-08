\   
\   
    SNDP_stochastic_MIP_general_instances\SNDP_stochastic_MIP_general_20_4_2_1.lp
\   
\   Generated with the MPL Modeling System
\   Copyright (c) 1988-2016  Maximal Software
\   
\   Date:               October 16, 2018
\   Time:               18:08
\   
\   Constraints:        77
\   Variables:          175
\   Integers:           19
\   Nonzeros:           213
\   Density:            1.6 %
\   


MAXIMIZE
  Profit:  - 2000 OpenProduction01 - 2000 OpenProduction02
     - 2000 OpenProduction03 - 2000 OpenProduction04
     - 2000 OpenProduction05 - 2000 OpenProduction06
     - 2000 OpenProduction07 - 2000 OpenProduction08
     - 2000 OpenProduction09 - 2000 OpenProduction10
     - 2000 OpenProduction11 - 2000 OpenProduction12
     - 2000 OpenProduction13 - 2000 OpenProduction14
     - 2000 OpenProduction15 - 2000 OpenProduction16
     - 2000 OpenProduction17 - 2000 OpenProduction18
     - 2000 OpenProduction19 - Ship10501 - 3 Ship10502 - 13 Ship10503
     - 10 Ship10507 - 8 Ship10512 - 3 Ship10513 - 10 Ship10514
     - 9 Ship10515 - 2 Ship10518 - 12 Ship10713 - 14 Ship10714
     - 13 Ship10715 - 8 Ship10717 - 8 Ship10718 - 12 Ship10719
     - 9 Ship10801 - 2 Ship10803 - 13 Ship10816 - 12 Ship11001
     - 14 Ship11004 - 10 Ship11005 - 9 Ship11006 - 6 Ship11007
     - 5 Ship11009 - 3 Ship11018 - 6 Ship20207 - 8 Ship20217
     - Ship20501 - 3 Ship20502 - 13 Ship20503 - 10 Ship20507
     - 8 Ship20512 - 3 Ship20513 - 10 Ship20514 - 9 Ship20515
     - 2 Ship20518 - 9 Ship20801 - 2 Ship20803 - 13 Ship20816
     - 12 Ship21001 - 14 Ship21004 - 10 Ship21005 - 9 Ship21006
     - 6 Ship21007 - 5 Ship21009 - 3 Ship21018 - 6 Ship21901
     - 13 Ship21902 - 12 Ship21905 - 7 Ship21906 - 3 Ship21909
     - 2 Ship21912 - 15 Ship21917 - 6 Ship21918 - Ship30501
     - 3 Ship30502 - 13 Ship30503 - 10 Ship30507 - 8 Ship30512
     - 3 Ship30513 - 10 Ship30514 - 9 Ship30515 - 2 Ship30518
     - 12 Ship30713 - 14 Ship30714 - 13 Ship30715 - 8 Ship30717
     - 8 Ship30718 - 12 Ship30719 - 9 Ship30801 - 2 Ship30803
     - 13 Ship30816 - 12 Ship31001 - 14 Ship31004 - 10 Ship31005
     - 9 Ship31006 - 6 Ship31007 - 5 Ship31009 - 3 Ship31018
     - 5 Ship31203 - 6 Ship31209 - Ship31217 - 2 Ship40120
     - 6 Ship40207 - 8 Ship40217 + 5 Ship40220 + 8 Ship40320
     + 11 Ship40420 - Ship40501 - 3 Ship40502 - 13 Ship40503
     - 10 Ship40507 - 8 Ship40512 - 3 Ship40513 - 10 Ship40514
     - 9 Ship40515 - 2 Ship40518 + 2 Ship40520 + 6 Ship40620
     - 12 Ship40713 - 14 Ship40714 - 13 Ship40715 - 8 Ship40717
     - 8 Ship40718 - 12 Ship40719 + 9 Ship40720 - 9 Ship40801
     - 2 Ship40803 - 13 Ship40816 - Ship40820 + 8 Ship40920
     - 12 Ship41001 - 14 Ship41004 - 10 Ship41005 - 9 Ship41006
     - 6 Ship41007 - 5 Ship41009 - 3 Ship41018 + 11 Ship41020
     + 7 Ship41120 - 5 Ship41203 - 6 Ship41209 - Ship41217
     + 9 Ship41220 - Ship41320 + 4 Ship41420 + 7 Ship41520 - Ship41620
     + 9 Ship41820 - 6 Ship41901 - 13 Ship41902 - 12 Ship41905
     - 7 Ship41906 - 3 Ship41909 - 2 Ship41912 - 15 Ship41917
     - 6 Ship41918 + Ship41920

SUBJECT TO
  BOMConstr011:   - Ship10501 - Ship10801 - Ship11001 + 3 Ship40120  = 
     0
  BOMConstr012:   - Ship20501 - Ship20801 - Ship21001 - Ship21901
     + 3 Ship40120  =  0
  BOMConstr013:   - Ship30501 - Ship30801 - Ship31001 + 2 Ship40120  = 
     0
  BOMConstr021:   - Ship10502 + 3 Ship40220  =  0
  BOMConstr022:   - Ship20202 - Ship20502 - Ship21902 + 3 Ship40220  = 
     0
  BOMConstr023:   - Ship30502 + 2 Ship40220  =  0
  BOMConstr031:   - Ship10503 - Ship10803 + 3 Ship40320  =  0
  BOMConstr032:   - Ship20503 - Ship20803 + 3 Ship40320  =  0
  BOMConstr033:   - Ship30503 - Ship30803 - Ship31203 + 2 Ship40320  = 
     0
  BOMConstr041:   - Ship11004 + 3 Ship40420  =  0
  BOMConstr042:   - Ship21004 + 3 Ship40420  =  0
  BOMConstr043:   - Ship31004 + 2 Ship40420  =  0
  BOMConstr051:   - Ship10505 - Ship11005 + 3 Ship40520  =  0
  BOMConstr052:   - Ship20505 - Ship21005 - Ship21905 + 3 Ship40520  = 
     0
  BOMConstr053:   - Ship30505 - Ship31005 + 2 Ship40520  =  0
  BOMConstr061:   - Ship11006 + 3 Ship40620  =  0
  BOMConstr062:   - Ship21006 - Ship21906 + 3 Ship40620  =  0
  BOMConstr063:   - Ship31006 + 2 Ship40620  =  0
  BOMConstr071:   - Ship10507 - Ship10707 - Ship11007 + 3 Ship40720  = 
     0
  BOMConstr072:   - Ship20207 - Ship20507 - Ship21007 + 3 Ship40720  = 
     0
  BOMConstr073:   - Ship30507 - Ship30707 - Ship31007 + 2 Ship40720  = 
     0
  BOMConstr081:   - Ship10808 + 3 Ship40820  =  0
  BOMConstr082:   - Ship20808 + 3 Ship40820  =  0
  BOMConstr083:   - Ship30808 + 2 Ship40820  =  0
  BOMConstr091:   - Ship11009 + 3 Ship40920  =  0
  BOMConstr092:   - Ship21009 - Ship21909 + 3 Ship40920  =  0
  BOMConstr093:   - Ship31009 - Ship31209 + 2 Ship40920  =  0
  BOMConstr101:   - Ship11010 + 3 Ship41020  =  0
  BOMConstr102:   - Ship21010 + 3 Ship41020  =  0
  BOMConstr103:   - Ship31010 + 2 Ship41020  =  0
  BOMConstr111:   - Ship11111 + 3 Ship41120  =  0
  BOMConstr112:   - Ship21111 + 3 Ship41120  =  0
  BOMConstr113:   - Ship31111 + 2 Ship41120  =  0
  BOMConstr121:   - Ship10512 + 3 Ship41220  =  0
  BOMConstr122:   - Ship20512 - Ship21912 + 3 Ship41220  =  0
  BOMConstr123:   - Ship30512 - Ship31212 + 2 Ship41220  =  0
  BOMConstr131:   - Ship10513 - Ship10713 + 3 Ship41320  =  0
  BOMConstr132:   - Ship20513 + 3 Ship41320  =  0
  BOMConstr133:   - Ship30513 - Ship30713 + 2 Ship41320  =  0
  BOMConstr141:   - Ship10514 - Ship10714 + 3 Ship41420  =  0
  BOMConstr142:   - Ship20514 + 3 Ship41420  =  0
  BOMConstr143:   - Ship30514 - Ship30714 + 2 Ship41420  =  0
  BOMConstr151:   - Ship10515 - Ship10715 + 3 Ship41520  =  0
  BOMConstr152:   - Ship20515 + 3 Ship41520  =  0
  BOMConstr153:   - Ship30515 - Ship30715 + 2 Ship41520  =  0
  BOMConstr161:   - Ship10816 + 3 Ship41620  =  0
  BOMConstr162:   - Ship20816 + 3 Ship41620  =  0
  BOMConstr163:   - Ship30816 + 2 Ship41620  =  0
  BOMConstr171:   - Ship10717 + 3 Ship41720  =  0
  BOMConstr172:   - Ship20217 - Ship21917 + 3 Ship41720  =  0
  BOMConstr173:   - Ship30717 - Ship31217 + 2 Ship41720  =  0
  BOMConstr181:   - Ship10518 - Ship10718 - Ship11018 + 3 Ship41820  = 
     0
  BOMConstr182:   - Ship20518 - Ship21018 - Ship21918 + 3 Ship41820  = 
     0
  BOMConstr183:   - Ship30518 - Ship30718 - Ship31018 + 2 Ship41820  = 
     0
  BOMConstr191:   - Ship10719 + 3 Ship41920  =  0
  BOMConstr192:   - Ship21919 + 3 Ship41920  =  0
  BOMConstr193:   - Ship30719 + 2 Ship41920  =  0
  DemConstr:   Ship40120 + Ship40220 + Ship40320 + Ship40420
     + Ship40520 + Ship40620 + Ship40720 + Ship40820 + Ship40920
     + Ship41020 + Ship41120 + Ship41220 + Ship41320 + Ship41420
     + Ship41520 + Ship41620 + Ship41720 + Ship41820 + Ship41920  <= 
     11636
  PlantConstr01:   - 5000 OpenProduction01 + Ship40120  <=  0
  PlantConstr02:   - 5000 OpenProduction02 + Ship40220  <=  0
  PlantConstr03:   - 5000 OpenProduction03 + Ship40320  <=  0
  PlantConstr04:   - 5000 OpenProduction04 + Ship40420  <=  0
  PlantConstr05:   - 5000 OpenProduction05 + Ship40520  <=  0
  PlantConstr06:   - 5000 OpenProduction06 + Ship40620  <=  0
  PlantConstr07:   - 5000 OpenProduction07 + Ship40720  <=  0
  PlantConstr08:   - 5000 OpenProduction08 + Ship40820  <=  0
  PlantConstr09:   - 5000 OpenProduction09 + Ship40920  <=  0
  PlantConstr10:   - 5000 OpenProduction10 + Ship41020  <=  0
  PlantConstr11:   - 5000 OpenProduction11 + Ship41120  <=  0
  PlantConstr12:   - 5000 OpenProduction12 + Ship41220  <=  0
  PlantConstr13:   - 5000 OpenProduction13 + Ship41320  <=  0
  PlantConstr14:   - 5000 OpenProduction14 + Ship41420  <=  0
  PlantConstr15:   - 5000 OpenProduction15 + Ship41520  <=  0
  PlantConstr16:   - 5000 OpenProduction16 + Ship41620  <=  0
  PlantConstr17:   - 5000 OpenProduction17 + Ship41720  <=  0
  PlantConstr18:   - 5000 OpenProduction18 + Ship41820  <=  0
  PlantConstr19:   - 5000 OpenProduction19 + Ship41920  <=  0
INTEGERS
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


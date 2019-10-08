*   
*   r01_7_0-6_64_cr.mps
*   
*   Generated with the MPL Modeling System
*   Copyright (c) 1988-2018  Maximal Software
*   
*   Date:               November 22, 2018
*   Time:               13:26
*   
*   Constraints:        135
*   Variables:          395
*   Integers:           35
*   Nonzeros:           1105
*   Density:            2 %
*   
NAME          MCFNDSD  (MIN)
ROWS
  N Total_Cost
  E NodeBalance0101
  E NodeBalance0102
  E NodeBalance0103
  E NodeBalance0104
  E NodeBalance0105
  E NodeBalance0106
  E NodeBalance0107
  E NodeBalance0108
  E NodeBalance0109
  E NodeBalance0110
  E NodeBalance0201
  E NodeBalance0202
  E NodeBalance0203
  E NodeBalance0204
  E NodeBalance0205
  E NodeBalance0206
  E NodeBalance0207
  E NodeBalance0208
  E NodeBalance0209
  E NodeBalance0210
  E NodeBalance0301
  E NodeBalance0302
  E NodeBalance0303
  E NodeBalance0304
  E NodeBalance0305
  E NodeBalance0306
  E NodeBalance0307
  E NodeBalance0308
  E NodeBalance0309
  E NodeBalance0310
  E NodeBalance0401
  E NodeBalance0402
  E NodeBalance0403
  E NodeBalance0404
  E NodeBalance0405
  E NodeBalance0406
  E NodeBalance0407
  E NodeBalance0408
  E NodeBalance0409
  E NodeBalance0410
  E NodeBalance0501
  E NodeBalance0502
  E NodeBalance0503
  E NodeBalance0504
  E NodeBalance0505
  E NodeBalance0506
  E NodeBalance0507
  E NodeBalance0508
  E NodeBalance0509
  E NodeBalance0510
  E NodeBalance0601
  E NodeBalance0602
  E NodeBalance0603
  E NodeBalance0604
  E NodeBalance0605
  E NodeBalance0606
  E NodeBalance0607
  E NodeBalance0608
  E NodeBalance0609
  E NodeBalance0610
  E NodeBalance0701
  E NodeBalance0702
  E NodeBalance0703
  E NodeBalance0704
  E NodeBalance0705
  E NodeBalance0706
  E NodeBalance0707
  E NodeBalance0708
  E NodeBalance0709
  E NodeBalance0710
  E NodeBalance0801
  E NodeBalance0802
  E NodeBalance0803
  E NodeBalance0804
  E NodeBalance0805
  E NodeBalance0806
  E NodeBalance0807
  E NodeBalance0808
  E NodeBalance0809
  E NodeBalance0810
  E NodeBalance0901
  E NodeBalance0902
  E NodeBalance0903
  E NodeBalance0904
  E NodeBalance0905
  E NodeBalance0906
  E NodeBalance0907
  E NodeBalance0908
  E NodeBalance0909
  E NodeBalance0910
  E NodeBalance1001
  E NodeBalance1002
  E NodeBalance1003
  E NodeBalance1004
  E NodeBalance1005
  E NodeBalance1006
  E NodeBalance1007
  E NodeBalance1008
  E NodeBalance1009
  E NodeBalance1010
  L Capacity0102
  L Capacity0107
  L Capacity0108
  L Capacity0110
  L Capacity0203
  L Capacity0204
  L Capacity0207
  L Capacity0208
  L Capacity0209
  L Capacity0301
  L Capacity0302
  L Capacity0304
  L Capacity0307
  L Capacity0310
  L Capacity0403
  L Capacity0405
  L Capacity0410
  L Capacity0502
  L Capacity0503
  L Capacity0506
  L Capacity0508
  L Capacity0607
  L Capacity0608
  L Capacity0610
  L Capacity0701
  L Capacity0708
  L Capacity0709
  L Capacity0802
  L Capacity0803
  L Capacity0807
  L Capacity0809
  L Capacity0906
  L Capacity0910
  L Capacity1001
  L Capacity1006
COLUMNS
    y0102     Total_Cost         595.0   Capacity0102        -214.0
    y0107     Total_Cost         369.0   Capacity0107         -28.0
    y0108     Total_Cost         375.0   Capacity0108         -19.0
    y0110     Total_Cost         244.0   Capacity0110         -11.0
    y0203     Total_Cost         595.0   Capacity0203        -214.0
    y0204     Total_Cost         441.0   Capacity0204         -25.0
    y0207     Total_Cost         340.0   Capacity0207         -15.0
    y0208     Total_Cost         363.0   Capacity0208         -25.0
    y0209     Total_Cost         179.0   Capacity0209         -27.0
    y0301     Total_Cost         143.0   Capacity0301         -17.0
    y0302     Total_Cost         554.0   Capacity0302         -34.0
    y0304     Total_Cost         595.0   Capacity0304        -214.0
    y0307     Total_Cost         238.0   Capacity0307         -18.0
    y0310     Total_Cost         518.0   Capacity0310         -17.0
    y0403     Total_Cost         108.0   Capacity0403         -29.0
    y0405     Total_Cost         595.0   Capacity0405        -214.0
    y0410     Total_Cost         322.0   Capacity0410         -23.0
    y0502     Total_Cost         125.0   Capacity0502         -10.0
    y0503     Total_Cost         524.0   Capacity0503         -35.0
    y0506     Total_Cost         595.0   Capacity0506        -214.0
    y0508     Total_Cost         536.0   Capacity0508         -29.0
    y0607     Total_Cost         595.0   Capacity0607        -214.0
    y0608     Total_Cost         393.0   Capacity0608         -22.0
    y0610     Total_Cost         304.0   Capacity0610         -13.0
    y0701     Total_Cost         286.0   Capacity0701         -25.0
    y0708     Total_Cost         595.0   Capacity0708        -214.0
    y0709     Total_Cost         572.0   Capacity0709         -28.0
    y0802     Total_Cost         256.0   Capacity0802         -26.0
    y0803     Total_Cost         340.0   Capacity0803         -17.0
    y0807     Total_Cost         357.0   Capacity0807         -17.0
    y0809     Total_Cost         595.0   Capacity0809        -214.0
    y0906     Total_Cost         298.0   Capacity0906         -30.0
    y0910     Total_Cost         595.0   Capacity0910        -214.0
    y1001     Total_Cost         595.0   Capacity1001        -214.0
    y1006     Total_Cost         375.0   Capacity1006         -20.0
    x010201   Total_Cost         100.0   NodeBalance0101           1.0
    x010201   NodeBalance0201          -1.0   Capacity0102           1.0
    x010202   Total_Cost         100.0   NodeBalance0102           1.0
    x010202   NodeBalance0202          -1.0   Capacity0102           1.0
    x010203   Total_Cost         100.0   NodeBalance0103           1.0
    x010203   NodeBalance0203          -1.0   Capacity0102           1.0
    x010204   Total_Cost         100.0   NodeBalance0104           1.0
    x010204   NodeBalance0204          -1.0   Capacity0102           1.0
    x010205   Total_Cost         100.0   NodeBalance0105           1.0
    x010205   NodeBalance0205          -1.0   Capacity0102           1.0
    x010206   Total_Cost         100.0   NodeBalance0106           1.0
    x010206   NodeBalance0206          -1.0   Capacity0102           1.0
    x010207   Total_Cost         100.0   NodeBalance0107           1.0
    x010207   NodeBalance0207          -1.0   Capacity0102           1.0
    x010208   Total_Cost         100.0   NodeBalance0108           1.0
    x010208   NodeBalance0208          -1.0   Capacity0102           1.0
    x010209   Total_Cost         100.0   NodeBalance0109           1.0
    x010209   NodeBalance0209          -1.0   Capacity0102           1.0
    x010210   Total_Cost         100.0   NodeBalance0110           1.0
    x010210   NodeBalance0210          -1.0   Capacity0102           1.0
    x010701   Total_Cost          18.0   NodeBalance0101           1.0
    x010701   NodeBalance0701          -1.0   Capacity0107           1.0
    x010702   Total_Cost          18.0   NodeBalance0102           1.0
    x010702   NodeBalance0702          -1.0   Capacity0107           1.0
    x010703   Total_Cost          18.0   NodeBalance0103           1.0
    x010703   NodeBalance0703          -1.0   Capacity0107           1.0
    x010704   Total_Cost          18.0   NodeBalance0104           1.0
    x010704   NodeBalance0704          -1.0   Capacity0107           1.0
    x010705   Total_Cost          18.0   NodeBalance0105           1.0
    x010705   NodeBalance0705          -1.0   Capacity0107           1.0
    x010706   Total_Cost          18.0   NodeBalance0106           1.0
    x010706   NodeBalance0706          -1.0   Capacity0107           1.0
    x010707   Total_Cost          18.0   NodeBalance0107           1.0
    x010707   NodeBalance0707          -1.0   Capacity0107           1.0
    x010708   Total_Cost          18.0   NodeBalance0108           1.0
    x010708   NodeBalance0708          -1.0   Capacity0107           1.0
    x010709   Total_Cost          18.0   NodeBalance0109           1.0
    x010709   NodeBalance0709          -1.0   Capacity0107           1.0
    x010710   Total_Cost          18.0   NodeBalance0110           1.0
    x010710   NodeBalance0710          -1.0   Capacity0107           1.0
    x010801   Total_Cost          45.0   NodeBalance0101           1.0
    x010801   NodeBalance0801          -1.0   Capacity0108           1.0
    x010802   Total_Cost          45.0   NodeBalance0102           1.0
    x010802   NodeBalance0802          -1.0   Capacity0108           1.0
    x010803   Total_Cost          45.0   NodeBalance0103           1.0
    x010803   NodeBalance0803          -1.0   Capacity0108           1.0
    x010804   Total_Cost          45.0   NodeBalance0104           1.0
    x010804   NodeBalance0804          -1.0   Capacity0108           1.0
    x010805   Total_Cost          45.0   NodeBalance0105           1.0
    x010805   NodeBalance0805          -1.0   Capacity0108           1.0
    x010806   Total_Cost          45.0   NodeBalance0106           1.0
    x010806   NodeBalance0806          -1.0   Capacity0108           1.0
    x010807   Total_Cost          45.0   NodeBalance0107           1.0
    x010807   NodeBalance0807          -1.0   Capacity0108           1.0
    x010808   Total_Cost          45.0   NodeBalance0108           1.0
    x010808   NodeBalance0808          -1.0   Capacity0108           1.0
    x010809   Total_Cost          45.0   NodeBalance0109           1.0
    x010809   NodeBalance0809          -1.0   Capacity0108           1.0
    x010810   Total_Cost          45.0   NodeBalance0110           1.0
    x010810   NodeBalance0810          -1.0   Capacity0108           1.0
    x011001   Total_Cost          54.0   NodeBalance0101           1.0
    x011001   NodeBalance1001          -1.0   Capacity0110           1.0
    x011002   Total_Cost          54.0   NodeBalance0102           1.0
    x011002   NodeBalance1002          -1.0   Capacity0110           1.0
    x011003   Total_Cost          54.0   NodeBalance0103           1.0
    x011003   NodeBalance1003          -1.0   Capacity0110           1.0
    x011004   Total_Cost          54.0   NodeBalance0104           1.0
    x011004   NodeBalance1004          -1.0   Capacity0110           1.0
    x011005   Total_Cost          54.0   NodeBalance0105           1.0
    x011005   NodeBalance1005          -1.0   Capacity0110           1.0
    x011006   Total_Cost          54.0   NodeBalance0106           1.0
    x011006   NodeBalance1006          -1.0   Capacity0110           1.0
    x011007   Total_Cost          54.0   NodeBalance0107           1.0
    x011007   NodeBalance1007          -1.0   Capacity0110           1.0
    x011008   Total_Cost          54.0   NodeBalance0108           1.0
    x011008   NodeBalance1008          -1.0   Capacity0110           1.0
    x011009   Total_Cost          54.0   NodeBalance0109           1.0
    x011009   NodeBalance1009          -1.0   Capacity0110           1.0
    x011010   Total_Cost          54.0   NodeBalance0110           1.0
    x011010   NodeBalance1010          -1.0   Capacity0110           1.0
    x020301   Total_Cost         100.0   NodeBalance0201           1.0
    x020301   NodeBalance0301          -1.0   Capacity0203           1.0
    x020302   Total_Cost         100.0   NodeBalance0202           1.0
    x020302   NodeBalance0302          -1.0   Capacity0203           1.0
    x020303   Total_Cost         100.0   NodeBalance0203           1.0
    x020303   NodeBalance0303          -1.0   Capacity0203           1.0
    x020304   Total_Cost         100.0   NodeBalance0204           1.0
    x020304   NodeBalance0304          -1.0   Capacity0203           1.0
    x020305   Total_Cost         100.0   NodeBalance0205           1.0
    x020305   NodeBalance0305          -1.0   Capacity0203           1.0
    x020306   Total_Cost         100.0   NodeBalance0206           1.0
    x020306   NodeBalance0306          -1.0   Capacity0203           1.0
    x020307   Total_Cost         100.0   NodeBalance0207           1.0
    x020307   NodeBalance0307          -1.0   Capacity0203           1.0
    x020308   Total_Cost         100.0   NodeBalance0208           1.0
    x020308   NodeBalance0308          -1.0   Capacity0203           1.0
    x020309   Total_Cost         100.0   NodeBalance0209           1.0
    x020309   NodeBalance0309          -1.0   Capacity0203           1.0
    x020310   Total_Cost         100.0   NodeBalance0210           1.0
    x020310   NodeBalance0310          -1.0   Capacity0203           1.0
    x020401   Total_Cost          13.0   NodeBalance0201           1.0
    x020401   NodeBalance0401          -1.0   Capacity0204           1.0
    x020402   Total_Cost          13.0   NodeBalance0202           1.0
    x020402   NodeBalance0402          -1.0   Capacity0204           1.0
    x020403   Total_Cost          13.0   NodeBalance0203           1.0
    x020403   NodeBalance0403          -1.0   Capacity0204           1.0
    x020404   Total_Cost          13.0   NodeBalance0204           1.0
    x020404   NodeBalance0404          -1.0   Capacity0204           1.0
    x020405   Total_Cost          13.0   NodeBalance0205           1.0
    x020405   NodeBalance0405          -1.0   Capacity0204           1.0
    x020406   Total_Cost          13.0   NodeBalance0206           1.0
    x020406   NodeBalance0406          -1.0   Capacity0204           1.0
    x020407   Total_Cost          13.0   NodeBalance0207           1.0
    x020407   NodeBalance0407          -1.0   Capacity0204           1.0
    x020408   Total_Cost          13.0   NodeBalance0208           1.0
    x020408   NodeBalance0408          -1.0   Capacity0204           1.0
    x020409   Total_Cost          13.0   NodeBalance0209           1.0
    x020409   NodeBalance0409          -1.0   Capacity0204           1.0
    x020410   Total_Cost          13.0   NodeBalance0210           1.0
    x020410   NodeBalance0410          -1.0   Capacity0204           1.0
    x020701   Total_Cost          92.0   NodeBalance0201           1.0
    x020701   NodeBalance0701          -1.0   Capacity0207           1.0
    x020702   Total_Cost          92.0   NodeBalance0202           1.0
    x020702   NodeBalance0702          -1.0   Capacity0207           1.0
    x020703   Total_Cost          92.0   NodeBalance0203           1.0
    x020703   NodeBalance0703          -1.0   Capacity0207           1.0
    x020704   Total_Cost          92.0   NodeBalance0204           1.0
    x020704   NodeBalance0704          -1.0   Capacity0207           1.0
    x020705   Total_Cost          92.0   NodeBalance0205           1.0
    x020705   NodeBalance0705          -1.0   Capacity0207           1.0
    x020706   Total_Cost          92.0   NodeBalance0206           1.0
    x020706   NodeBalance0706          -1.0   Capacity0207           1.0
    x020707   Total_Cost          92.0   NodeBalance0207           1.0
    x020707   NodeBalance0707          -1.0   Capacity0207           1.0
    x020708   Total_Cost          92.0   NodeBalance0208           1.0
    x020708   NodeBalance0708          -1.0   Capacity0207           1.0
    x020709   Total_Cost          92.0   NodeBalance0209           1.0
    x020709   NodeBalance0709          -1.0   Capacity0207           1.0
    x020710   Total_Cost          92.0   NodeBalance0210           1.0
    x020710   NodeBalance0710          -1.0   Capacity0207           1.0
    x020801   Total_Cost          93.0   NodeBalance0201           1.0
    x020801   NodeBalance0801          -1.0   Capacity0208           1.0
    x020802   Total_Cost          93.0   NodeBalance0202           1.0
    x020802   NodeBalance0802          -1.0   Capacity0208           1.0
    x020803   Total_Cost          93.0   NodeBalance0203           1.0
    x020803   NodeBalance0803          -1.0   Capacity0208           1.0
    x020804   Total_Cost          93.0   NodeBalance0204           1.0
    x020804   NodeBalance0804          -1.0   Capacity0208           1.0
    x020805   Total_Cost          93.0   NodeBalance0205           1.0
    x020805   NodeBalance0805          -1.0   Capacity0208           1.0
    x020806   Total_Cost          93.0   NodeBalance0206           1.0
    x020806   NodeBalance0806          -1.0   Capacity0208           1.0
    x020807   Total_Cost          93.0   NodeBalance0207           1.0
    x020807   NodeBalance0807          -1.0   Capacity0208           1.0
    x020808   Total_Cost          93.0   NodeBalance0208           1.0
    x020808   NodeBalance0808          -1.0   Capacity0208           1.0
    x020809   Total_Cost          93.0   NodeBalance0209           1.0
    x020809   NodeBalance0809          -1.0   Capacity0208           1.0
    x020810   Total_Cost          93.0   NodeBalance0210           1.0
    x020810   NodeBalance0810          -1.0   Capacity0208           1.0
    x020901   Total_Cost          13.0   NodeBalance0201           1.0
    x020901   NodeBalance0901          -1.0   Capacity0209           1.0
    x020902   Total_Cost          13.0   NodeBalance0202           1.0
    x020902   NodeBalance0902          -1.0   Capacity0209           1.0
    x020903   Total_Cost          13.0   NodeBalance0203           1.0
    x020903   NodeBalance0903          -1.0   Capacity0209           1.0
    x020904   Total_Cost          13.0   NodeBalance0204           1.0
    x020904   NodeBalance0904          -1.0   Capacity0209           1.0
    x020905   Total_Cost          13.0   NodeBalance0205           1.0
    x020905   NodeBalance0905          -1.0   Capacity0209           1.0
    x020906   Total_Cost          13.0   NodeBalance0206           1.0
    x020906   NodeBalance0906          -1.0   Capacity0209           1.0
    x020907   Total_Cost          13.0   NodeBalance0207           1.0
    x020907   NodeBalance0907          -1.0   Capacity0209           1.0
    x020908   Total_Cost          13.0   NodeBalance0208           1.0
    x020908   NodeBalance0908          -1.0   Capacity0209           1.0
    x020909   Total_Cost          13.0   NodeBalance0209           1.0
    x020909   NodeBalance0909          -1.0   Capacity0209           1.0
    x020910   Total_Cost          13.0   NodeBalance0210           1.0
    x020910   NodeBalance0910          -1.0   Capacity0209           1.0
    x030101   Total_Cost          76.0   NodeBalance0101          -1.0
    x030101   NodeBalance0301           1.0   Capacity0301           1.0
    x030102   Total_Cost          76.0   NodeBalance0102          -1.0
    x030102   NodeBalance0302           1.0   Capacity0301           1.0
    x030103   Total_Cost          76.0   NodeBalance0103          -1.0
    x030103   NodeBalance0303           1.0   Capacity0301           1.0
    x030104   Total_Cost          76.0   NodeBalance0104          -1.0
    x030104   NodeBalance0304           1.0   Capacity0301           1.0
    x030105   Total_Cost          76.0   NodeBalance0105          -1.0
    x030105   NodeBalance0305           1.0   Capacity0301           1.0
    x030106   Total_Cost          76.0   NodeBalance0106          -1.0
    x030106   NodeBalance0306           1.0   Capacity0301           1.0
    x030107   Total_Cost          76.0   NodeBalance0107          -1.0
    x030107   NodeBalance0307           1.0   Capacity0301           1.0
    x030108   Total_Cost          76.0   NodeBalance0108          -1.0
    x030108   NodeBalance0308           1.0   Capacity0301           1.0
    x030109   Total_Cost          76.0   NodeBalance0109          -1.0
    x030109   NodeBalance0309           1.0   Capacity0301           1.0
    x030110   Total_Cost          76.0   NodeBalance0110          -1.0
    x030110   NodeBalance0310           1.0   Capacity0301           1.0
    x030201   Total_Cost          27.0   NodeBalance0201          -1.0
    x030201   NodeBalance0301           1.0   Capacity0302           1.0
    x030202   Total_Cost          27.0   NodeBalance0202          -1.0
    x030202   NodeBalance0302           1.0   Capacity0302           1.0
    x030203   Total_Cost          27.0   NodeBalance0203          -1.0
    x030203   NodeBalance0303           1.0   Capacity0302           1.0
    x030204   Total_Cost          27.0   NodeBalance0204          -1.0
    x030204   NodeBalance0304           1.0   Capacity0302           1.0
    x030205   Total_Cost          27.0   NodeBalance0205          -1.0
    x030205   NodeBalance0305           1.0   Capacity0302           1.0
    x030206   Total_Cost          27.0   NodeBalance0206          -1.0
    x030206   NodeBalance0306           1.0   Capacity0302           1.0
    x030207   Total_Cost          27.0   NodeBalance0207          -1.0
    x030207   NodeBalance0307           1.0   Capacity0302           1.0
    x030208   Total_Cost          27.0   NodeBalance0208          -1.0
    x030208   NodeBalance0308           1.0   Capacity0302           1.0
    x030209   Total_Cost          27.0   NodeBalance0209          -1.0
    x030209   NodeBalance0309           1.0   Capacity0302           1.0
    x030210   Total_Cost          27.0   NodeBalance0210          -1.0
    x030210   NodeBalance0310           1.0   Capacity0302           1.0
    x030401   Total_Cost         100.0   NodeBalance0301           1.0
    x030401   NodeBalance0401          -1.0   Capacity0304           1.0
    x030402   Total_Cost         100.0   NodeBalance0302           1.0
    x030402   NodeBalance0402          -1.0   Capacity0304           1.0
    x030403   Total_Cost         100.0   NodeBalance0303           1.0
    x030403   NodeBalance0403          -1.0   Capacity0304           1.0
    x030404   Total_Cost         100.0   NodeBalance0304           1.0
    x030404   NodeBalance0404          -1.0   Capacity0304           1.0
    x030405   Total_Cost         100.0   NodeBalance0305           1.0
    x030405   NodeBalance0405          -1.0   Capacity0304           1.0
    x030406   Total_Cost         100.0   NodeBalance0306           1.0
    x030406   NodeBalance0406          -1.0   Capacity0304           1.0
    x030407   Total_Cost         100.0   NodeBalance0307           1.0
    x030407   NodeBalance0407          -1.0   Capacity0304           1.0
    x030408   Total_Cost         100.0   NodeBalance0308           1.0
    x030408   NodeBalance0408          -1.0   Capacity0304           1.0
    x030409   Total_Cost         100.0   NodeBalance0309           1.0
    x030409   NodeBalance0409          -1.0   Capacity0304           1.0
    x030410   Total_Cost         100.0   NodeBalance0310           1.0
    x030410   NodeBalance0410          -1.0   Capacity0304           1.0
    x030701   Total_Cost          11.0   NodeBalance0301           1.0
    x030701   NodeBalance0701          -1.0   Capacity0307           1.0
    x030702   Total_Cost          11.0   NodeBalance0302           1.0
    x030702   NodeBalance0702          -1.0   Capacity0307           1.0
    x030703   Total_Cost          11.0   NodeBalance0303           1.0
    x030703   NodeBalance0703          -1.0   Capacity0307           1.0
    x030704   Total_Cost          11.0   NodeBalance0304           1.0
    x030704   NodeBalance0704          -1.0   Capacity0307           1.0
    x030705   Total_Cost          11.0   NodeBalance0305           1.0
    x030705   NodeBalance0705          -1.0   Capacity0307           1.0
    x030706   Total_Cost          11.0   NodeBalance0306           1.0
    x030706   NodeBalance0706          -1.0   Capacity0307           1.0
    x030707   Total_Cost          11.0   NodeBalance0307           1.0
    x030707   NodeBalance0707          -1.0   Capacity0307           1.0
    x030708   Total_Cost          11.0   NodeBalance0308           1.0
    x030708   NodeBalance0708          -1.0   Capacity0307           1.0
    x030709   Total_Cost          11.0   NodeBalance0309           1.0
    x030709   NodeBalance0709          -1.0   Capacity0307           1.0
    x030710   Total_Cost          11.0   NodeBalance0310           1.0
    x030710   NodeBalance0710          -1.0   Capacity0307           1.0
    x031001   Total_Cost          25.0   NodeBalance0301           1.0
    x031001   NodeBalance1001          -1.0   Capacity0310           1.0
    x031002   Total_Cost          25.0   NodeBalance0302           1.0
    x031002   NodeBalance1002          -1.0   Capacity0310           1.0
    x031003   Total_Cost          25.0   NodeBalance0303           1.0
    x031003   NodeBalance1003          -1.0   Capacity0310           1.0
    x031004   Total_Cost          25.0   NodeBalance0304           1.0
    x031004   NodeBalance1004          -1.0   Capacity0310           1.0
    x031005   Total_Cost          25.0   NodeBalance0305           1.0
    x031005   NodeBalance1005          -1.0   Capacity0310           1.0
    x031006   Total_Cost          25.0   NodeBalance0306           1.0
    x031006   NodeBalance1006          -1.0   Capacity0310           1.0
    x031007   Total_Cost          25.0   NodeBalance0307           1.0
    x031007   NodeBalance1007          -1.0   Capacity0310           1.0
    x031008   Total_Cost          25.0   NodeBalance0308           1.0
    x031008   NodeBalance1008          -1.0   Capacity0310           1.0
    x031009   Total_Cost          25.0   NodeBalance0309           1.0
    x031009   NodeBalance1009          -1.0   Capacity0310           1.0
    x031010   Total_Cost          25.0   NodeBalance0310           1.0
    x031010   NodeBalance1010          -1.0   Capacity0310           1.0
    x040301   Total_Cost          95.0   NodeBalance0301          -1.0
    x040301   NodeBalance0401           1.0   Capacity0403           1.0
    x040302   Total_Cost          95.0   NodeBalance0302          -1.0
    x040302   NodeBalance0402           1.0   Capacity0403           1.0
    x040303   Total_Cost          95.0   NodeBalance0303          -1.0
    x040303   NodeBalance0403           1.0   Capacity0403           1.0
    x040304   Total_Cost          95.0   NodeBalance0304          -1.0
    x040304   NodeBalance0404           1.0   Capacity0403           1.0
    x040305   Total_Cost          95.0   NodeBalance0305          -1.0
    x040305   NodeBalance0405           1.0   Capacity0403           1.0
    x040306   Total_Cost          95.0   NodeBalance0306          -1.0
    x040306   NodeBalance0406           1.0   Capacity0403           1.0
    x040307   Total_Cost          95.0   NodeBalance0307          -1.0
    x040307   NodeBalance0407           1.0   Capacity0403           1.0
    x040308   Total_Cost          95.0   NodeBalance0308          -1.0
    x040308   NodeBalance0408           1.0   Capacity0403           1.0
    x040309   Total_Cost          95.0   NodeBalance0309          -1.0
    x040309   NodeBalance0409           1.0   Capacity0403           1.0
    x040310   Total_Cost          95.0   NodeBalance0310          -1.0
    x040310   NodeBalance0410           1.0   Capacity0403           1.0
    x040501   Total_Cost         100.0   NodeBalance0401           1.0
    x040501   NodeBalance0501          -1.0   Capacity0405           1.0
    x040502   Total_Cost         100.0   NodeBalance0402           1.0
    x040502   NodeBalance0502          -1.0   Capacity0405           1.0
    x040503   Total_Cost         100.0   NodeBalance0403           1.0
    x040503   NodeBalance0503          -1.0   Capacity0405           1.0
    x040504   Total_Cost         100.0   NodeBalance0404           1.0
    x040504   NodeBalance0504          -1.0   Capacity0405           1.0
    x040505   Total_Cost         100.0   NodeBalance0405           1.0
    x040505   NodeBalance0505          -1.0   Capacity0405           1.0
    x040506   Total_Cost         100.0   NodeBalance0406           1.0
    x040506   NodeBalance0506          -1.0   Capacity0405           1.0
    x040507   Total_Cost         100.0   NodeBalance0407           1.0
    x040507   NodeBalance0507          -1.0   Capacity0405           1.0
    x040508   Total_Cost         100.0   NodeBalance0408           1.0
    x040508   NodeBalance0508          -1.0   Capacity0405           1.0
    x040509   Total_Cost         100.0   NodeBalance0409           1.0
    x040509   NodeBalance0509          -1.0   Capacity0405           1.0
    x040510   Total_Cost         100.0   NodeBalance0410           1.0
    x040510   NodeBalance0510          -1.0   Capacity0405           1.0
    x041001   Total_Cost          50.0   NodeBalance0401           1.0
    x041001   NodeBalance1001          -1.0   Capacity0410           1.0
    x041002   Total_Cost          50.0   NodeBalance0402           1.0
    x041002   NodeBalance1002          -1.0   Capacity0410           1.0
    x041003   Total_Cost          50.0   NodeBalance0403           1.0
    x041003   NodeBalance1003          -1.0   Capacity0410           1.0
    x041004   Total_Cost          50.0   NodeBalance0404           1.0
    x041004   NodeBalance1004          -1.0   Capacity0410           1.0
    x041005   Total_Cost          50.0   NodeBalance0405           1.0
    x041005   NodeBalance1005          -1.0   Capacity0410           1.0
    x041006   Total_Cost          50.0   NodeBalance0406           1.0
    x041006   NodeBalance1006          -1.0   Capacity0410           1.0
    x041007   Total_Cost          50.0   NodeBalance0407           1.0
    x041007   NodeBalance1007          -1.0   Capacity0410           1.0
    x041008   Total_Cost          50.0   NodeBalance0408           1.0
    x041008   NodeBalance1008          -1.0   Capacity0410           1.0
    x041009   Total_Cost          50.0   NodeBalance0409           1.0
    x041009   NodeBalance1009          -1.0   Capacity0410           1.0
    x041010   Total_Cost          50.0   NodeBalance0410           1.0
    x041010   NodeBalance1010          -1.0   Capacity0410           1.0
    x050201   Total_Cost          34.0   NodeBalance0201          -1.0
    x050201   NodeBalance0501           1.0   Capacity0502           1.0
    x050202   Total_Cost          34.0   NodeBalance0202          -1.0
    x050202   NodeBalance0502           1.0   Capacity0502           1.0
    x050203   Total_Cost          34.0   NodeBalance0203          -1.0
    x050203   NodeBalance0503           1.0   Capacity0502           1.0
    x050204   Total_Cost          34.0   NodeBalance0204          -1.0
    x050204   NodeBalance0504           1.0   Capacity0502           1.0
    x050205   Total_Cost          34.0   NodeBalance0205          -1.0
    x050205   NodeBalance0505           1.0   Capacity0502           1.0
    x050206   Total_Cost          34.0   NodeBalance0206          -1.0
    x050206   NodeBalance0506           1.0   Capacity0502           1.0
    x050207   Total_Cost          34.0   NodeBalance0207          -1.0
    x050207   NodeBalance0507           1.0   Capacity0502           1.0
    x050208   Total_Cost          34.0   NodeBalance0208          -1.0
    x050208   NodeBalance0508           1.0   Capacity0502           1.0
    x050209   Total_Cost          34.0   NodeBalance0209          -1.0
    x050209   NodeBalance0509           1.0   Capacity0502           1.0
    x050210   Total_Cost          34.0   NodeBalance0210          -1.0
    x050210   NodeBalance0510           1.0   Capacity0502           1.0
    x050301   Total_Cost          59.0   NodeBalance0301          -1.0
    x050301   NodeBalance0501           1.0   Capacity0503           1.0
    x050302   Total_Cost          59.0   NodeBalance0302          -1.0
    x050302   NodeBalance0502           1.0   Capacity0503           1.0
    x050303   Total_Cost          59.0   NodeBalance0303          -1.0
    x050303   NodeBalance0503           1.0   Capacity0503           1.0
    x050304   Total_Cost          59.0   NodeBalance0304          -1.0
    x050304   NodeBalance0504           1.0   Capacity0503           1.0
    x050305   Total_Cost          59.0   NodeBalance0305          -1.0
    x050305   NodeBalance0505           1.0   Capacity0503           1.0
    x050306   Total_Cost          59.0   NodeBalance0306          -1.0
    x050306   NodeBalance0506           1.0   Capacity0503           1.0
    x050307   Total_Cost          59.0   NodeBalance0307          -1.0
    x050307   NodeBalance0507           1.0   Capacity0503           1.0
    x050308   Total_Cost          59.0   NodeBalance0308          -1.0
    x050308   NodeBalance0508           1.0   Capacity0503           1.0
    x050309   Total_Cost          59.0   NodeBalance0309          -1.0
    x050309   NodeBalance0509           1.0   Capacity0503           1.0
    x050310   Total_Cost          59.0   NodeBalance0310          -1.0
    x050310   NodeBalance0510           1.0   Capacity0503           1.0
    x050601   Total_Cost         100.0   NodeBalance0501           1.0
    x050601   NodeBalance0601          -1.0   Capacity0506           1.0
    x050602   Total_Cost         100.0   NodeBalance0502           1.0
    x050602   NodeBalance0602          -1.0   Capacity0506           1.0
    x050603   Total_Cost         100.0   NodeBalance0503           1.0
    x050603   NodeBalance0603          -1.0   Capacity0506           1.0
    x050604   Total_Cost         100.0   NodeBalance0504           1.0
    x050604   NodeBalance0604          -1.0   Capacity0506           1.0
    x050605   Total_Cost         100.0   NodeBalance0505           1.0
    x050605   NodeBalance0605          -1.0   Capacity0506           1.0
    x050606   Total_Cost         100.0   NodeBalance0506           1.0
    x050606   NodeBalance0606          -1.0   Capacity0506           1.0
    x050607   Total_Cost         100.0   NodeBalance0507           1.0
    x050607   NodeBalance0607          -1.0   Capacity0506           1.0
    x050608   Total_Cost         100.0   NodeBalance0508           1.0
    x050608   NodeBalance0608          -1.0   Capacity0506           1.0
    x050609   Total_Cost         100.0   NodeBalance0509           1.0
    x050609   NodeBalance0609          -1.0   Capacity0506           1.0
    x050610   Total_Cost         100.0   NodeBalance0510           1.0
    x050610   NodeBalance0610          -1.0   Capacity0506           1.0
    x050801   Total_Cost          72.0   NodeBalance0501           1.0
    x050801   NodeBalance0801          -1.0   Capacity0508           1.0
    x050802   Total_Cost          72.0   NodeBalance0502           1.0
    x050802   NodeBalance0802          -1.0   Capacity0508           1.0
    x050803   Total_Cost          72.0   NodeBalance0503           1.0
    x050803   NodeBalance0803          -1.0   Capacity0508           1.0
    x050804   Total_Cost          72.0   NodeBalance0504           1.0
    x050804   NodeBalance0804          -1.0   Capacity0508           1.0
    x050805   Total_Cost          72.0   NodeBalance0505           1.0
    x050805   NodeBalance0805          -1.0   Capacity0508           1.0
    x050806   Total_Cost          72.0   NodeBalance0506           1.0
    x050806   NodeBalance0806          -1.0   Capacity0508           1.0
    x050807   Total_Cost          72.0   NodeBalance0507           1.0
    x050807   NodeBalance0807          -1.0   Capacity0508           1.0
    x050808   Total_Cost          72.0   NodeBalance0508           1.0
    x050808   NodeBalance0808          -1.0   Capacity0508           1.0
    x050809   Total_Cost          72.0   NodeBalance0509           1.0
    x050809   NodeBalance0809          -1.0   Capacity0508           1.0
    x050810   Total_Cost          72.0   NodeBalance0510           1.0
    x050810   NodeBalance0810          -1.0   Capacity0508           1.0
    x060701   Total_Cost         100.0   NodeBalance0601           1.0
    x060701   NodeBalance0701          -1.0   Capacity0607           1.0
    x060702   Total_Cost         100.0   NodeBalance0602           1.0
    x060702   NodeBalance0702          -1.0   Capacity0607           1.0
    x060703   Total_Cost         100.0   NodeBalance0603           1.0
    x060703   NodeBalance0703          -1.0   Capacity0607           1.0
    x060704   Total_Cost         100.0   NodeBalance0604           1.0
    x060704   NodeBalance0704          -1.0   Capacity0607           1.0
    x060705   Total_Cost         100.0   NodeBalance0605           1.0
    x060705   NodeBalance0705          -1.0   Capacity0607           1.0
    x060706   Total_Cost         100.0   NodeBalance0606           1.0
    x060706   NodeBalance0706          -1.0   Capacity0607           1.0
    x060707   Total_Cost         100.0   NodeBalance0607           1.0
    x060707   NodeBalance0707          -1.0   Capacity0607           1.0
    x060708   Total_Cost         100.0   NodeBalance0608           1.0
    x060708   NodeBalance0708          -1.0   Capacity0607           1.0
    x060709   Total_Cost         100.0   NodeBalance0609           1.0
    x060709   NodeBalance0709          -1.0   Capacity0607           1.0
    x060710   Total_Cost         100.0   NodeBalance0610           1.0
    x060710   NodeBalance0710          -1.0   Capacity0607           1.0
    x060801   Total_Cost          73.0   NodeBalance0601           1.0
    x060801   NodeBalance0801          -1.0   Capacity0608           1.0
    x060802   Total_Cost          73.0   NodeBalance0602           1.0
    x060802   NodeBalance0802          -1.0   Capacity0608           1.0
    x060803   Total_Cost          73.0   NodeBalance0603           1.0
    x060803   NodeBalance0803          -1.0   Capacity0608           1.0
    x060804   Total_Cost          73.0   NodeBalance0604           1.0
    x060804   NodeBalance0804          -1.0   Capacity0608           1.0
    x060805   Total_Cost          73.0   NodeBalance0605           1.0
    x060805   NodeBalance0805          -1.0   Capacity0608           1.0
    x060806   Total_Cost          73.0   NodeBalance0606           1.0
    x060806   NodeBalance0806          -1.0   Capacity0608           1.0
    x060807   Total_Cost          73.0   NodeBalance0607           1.0
    x060807   NodeBalance0807          -1.0   Capacity0608           1.0
    x060808   Total_Cost          73.0   NodeBalance0608           1.0
    x060808   NodeBalance0808          -1.0   Capacity0608           1.0
    x060809   Total_Cost          73.0   NodeBalance0609           1.0
    x060809   NodeBalance0809          -1.0   Capacity0608           1.0
    x060810   Total_Cost          73.0   NodeBalance0610           1.0
    x060810   NodeBalance0810          -1.0   Capacity0608           1.0
    x061001   Total_Cost          41.0   NodeBalance0601           1.0
    x061001   NodeBalance1001          -1.0   Capacity0610           1.0
    x061002   Total_Cost          41.0   NodeBalance0602           1.0
    x061002   NodeBalance1002          -1.0   Capacity0610           1.0
    x061003   Total_Cost          41.0   NodeBalance0603           1.0
    x061003   NodeBalance1003          -1.0   Capacity0610           1.0
    x061004   Total_Cost          41.0   NodeBalance0604           1.0
    x061004   NodeBalance1004          -1.0   Capacity0610           1.0
    x061005   Total_Cost          41.0   NodeBalance0605           1.0
    x061005   NodeBalance1005          -1.0   Capacity0610           1.0
    x061006   Total_Cost          41.0   NodeBalance0606           1.0
    x061006   NodeBalance1006          -1.0   Capacity0610           1.0
    x061007   Total_Cost          41.0   NodeBalance0607           1.0
    x061007   NodeBalance1007          -1.0   Capacity0610           1.0
    x061008   Total_Cost          41.0   NodeBalance0608           1.0
    x061008   NodeBalance1008          -1.0   Capacity0610           1.0
    x061009   Total_Cost          41.0   NodeBalance0609           1.0
    x061009   NodeBalance1009          -1.0   Capacity0610           1.0
    x061010   Total_Cost          41.0   NodeBalance0610           1.0
    x061010   NodeBalance1010          -1.0   Capacity0610           1.0
    x070101   Total_Cost          96.0   NodeBalance0101          -1.0
    x070101   NodeBalance0701           1.0   Capacity0701           1.0
    x070102   Total_Cost          96.0   NodeBalance0102          -1.0
    x070102   NodeBalance0702           1.0   Capacity0701           1.0
    x070103   Total_Cost          96.0   NodeBalance0103          -1.0
    x070103   NodeBalance0703           1.0   Capacity0701           1.0
    x070104   Total_Cost          96.0   NodeBalance0104          -1.0
    x070104   NodeBalance0704           1.0   Capacity0701           1.0
    x070105   Total_Cost          96.0   NodeBalance0105          -1.0
    x070105   NodeBalance0705           1.0   Capacity0701           1.0
    x070106   Total_Cost          96.0   NodeBalance0106          -1.0
    x070106   NodeBalance0706           1.0   Capacity0701           1.0
    x070107   Total_Cost          96.0   NodeBalance0107          -1.0
    x070107   NodeBalance0707           1.0   Capacity0701           1.0
    x070108   Total_Cost          96.0   NodeBalance0108          -1.0
    x070108   NodeBalance0708           1.0   Capacity0701           1.0
    x070109   Total_Cost          96.0   NodeBalance0109          -1.0
    x070109   NodeBalance0709           1.0   Capacity0701           1.0
    x070110   Total_Cost          96.0   NodeBalance0110          -1.0
    x070110   NodeBalance0710           1.0   Capacity0701           1.0
    x070801   Total_Cost         100.0   NodeBalance0701           1.0
    x070801   NodeBalance0801          -1.0   Capacity0708           1.0
    x070802   Total_Cost         100.0   NodeBalance0702           1.0
    x070802   NodeBalance0802          -1.0   Capacity0708           1.0
    x070803   Total_Cost         100.0   NodeBalance0703           1.0
    x070803   NodeBalance0803          -1.0   Capacity0708           1.0
    x070804   Total_Cost         100.0   NodeBalance0704           1.0
    x070804   NodeBalance0804          -1.0   Capacity0708           1.0
    x070805   Total_Cost         100.0   NodeBalance0705           1.0
    x070805   NodeBalance0805          -1.0   Capacity0708           1.0
    x070806   Total_Cost         100.0   NodeBalance0706           1.0
    x070806   NodeBalance0806          -1.0   Capacity0708           1.0
    x070807   Total_Cost         100.0   NodeBalance0707           1.0
    x070807   NodeBalance0807          -1.0   Capacity0708           1.0
    x070808   Total_Cost         100.0   NodeBalance0708           1.0
    x070808   NodeBalance0808          -1.0   Capacity0708           1.0
    x070809   Total_Cost         100.0   NodeBalance0709           1.0
    x070809   NodeBalance0809          -1.0   Capacity0708           1.0
    x070810   Total_Cost         100.0   NodeBalance0710           1.0
    x070810   NodeBalance0810          -1.0   Capacity0708           1.0
    x070901   Total_Cost          22.0   NodeBalance0701           1.0
    x070901   NodeBalance0901          -1.0   Capacity0709           1.0
    x070902   Total_Cost          22.0   NodeBalance0702           1.0
    x070902   NodeBalance0902          -1.0   Capacity0709           1.0
    x070903   Total_Cost          22.0   NodeBalance0703           1.0
    x070903   NodeBalance0903          -1.0   Capacity0709           1.0
    x070904   Total_Cost          22.0   NodeBalance0704           1.0
    x070904   NodeBalance0904          -1.0   Capacity0709           1.0
    x070905   Total_Cost          22.0   NodeBalance0705           1.0
    x070905   NodeBalance0905          -1.0   Capacity0709           1.0
    x070906   Total_Cost          22.0   NodeBalance0706           1.0
    x070906   NodeBalance0906          -1.0   Capacity0709           1.0
    x070907   Total_Cost          22.0   NodeBalance0707           1.0
    x070907   NodeBalance0907          -1.0   Capacity0709           1.0
    x070908   Total_Cost          22.0   NodeBalance0708           1.0
    x070908   NodeBalance0908          -1.0   Capacity0709           1.0
    x070909   Total_Cost          22.0   NodeBalance0709           1.0
    x070909   NodeBalance0909          -1.0   Capacity0709           1.0
    x070910   Total_Cost          22.0   NodeBalance0710           1.0
    x070910   NodeBalance0910          -1.0   Capacity0709           1.0
    x080201   Total_Cost          30.0   NodeBalance0201          -1.0
    x080201   NodeBalance0801           1.0   Capacity0802           1.0
    x080202   Total_Cost          30.0   NodeBalance0202          -1.0
    x080202   NodeBalance0802           1.0   Capacity0802           1.0
    x080203   Total_Cost          30.0   NodeBalance0203          -1.0
    x080203   NodeBalance0803           1.0   Capacity0802           1.0
    x080204   Total_Cost          30.0   NodeBalance0204          -1.0
    x080204   NodeBalance0804           1.0   Capacity0802           1.0
    x080205   Total_Cost          30.0   NodeBalance0205          -1.0
    x080205   NodeBalance0805           1.0   Capacity0802           1.0
    x080206   Total_Cost          30.0   NodeBalance0206          -1.0
    x080206   NodeBalance0806           1.0   Capacity0802           1.0
    x080207   Total_Cost          30.0   NodeBalance0207          -1.0
    x080207   NodeBalance0807           1.0   Capacity0802           1.0
    x080208   Total_Cost          30.0   NodeBalance0208          -1.0
    x080208   NodeBalance0808           1.0   Capacity0802           1.0
    x080209   Total_Cost          30.0   NodeBalance0209          -1.0
    x080209   NodeBalance0809           1.0   Capacity0802           1.0
    x080210   Total_Cost          30.0   NodeBalance0210          -1.0
    x080210   NodeBalance0810           1.0   Capacity0802           1.0
    x080301   Total_Cost          41.0   NodeBalance0301          -1.0
    x080301   NodeBalance0801           1.0   Capacity0803           1.0
    x080302   Total_Cost          41.0   NodeBalance0302          -1.0
    x080302   NodeBalance0802           1.0   Capacity0803           1.0
    x080303   Total_Cost          41.0   NodeBalance0303          -1.0
    x080303   NodeBalance0803           1.0   Capacity0803           1.0
    x080304   Total_Cost          41.0   NodeBalance0304          -1.0
    x080304   NodeBalance0804           1.0   Capacity0803           1.0
    x080305   Total_Cost          41.0   NodeBalance0305          -1.0
    x080305   NodeBalance0805           1.0   Capacity0803           1.0
    x080306   Total_Cost          41.0   NodeBalance0306          -1.0
    x080306   NodeBalance0806           1.0   Capacity0803           1.0
    x080307   Total_Cost          41.0   NodeBalance0307          -1.0
    x080307   NodeBalance0807           1.0   Capacity0803           1.0
    x080308   Total_Cost          41.0   NodeBalance0308          -1.0
    x080308   NodeBalance0808           1.0   Capacity0803           1.0
    x080309   Total_Cost          41.0   NodeBalance0309          -1.0
    x080309   NodeBalance0809           1.0   Capacity0803           1.0
    x080310   Total_Cost          41.0   NodeBalance0310          -1.0
    x080310   NodeBalance0810           1.0   Capacity0803           1.0
    x080701   Total_Cost          78.0   NodeBalance0701          -1.0
    x080701   NodeBalance0801           1.0   Capacity0807           1.0
    x080702   Total_Cost          78.0   NodeBalance0702          -1.0
    x080702   NodeBalance0802           1.0   Capacity0807           1.0
    x080703   Total_Cost          78.0   NodeBalance0703          -1.0
    x080703   NodeBalance0803           1.0   Capacity0807           1.0
    x080704   Total_Cost          78.0   NodeBalance0704          -1.0
    x080704   NodeBalance0804           1.0   Capacity0807           1.0
    x080705   Total_Cost          78.0   NodeBalance0705          -1.0
    x080705   NodeBalance0805           1.0   Capacity0807           1.0
    x080706   Total_Cost          78.0   NodeBalance0706          -1.0
    x080706   NodeBalance0806           1.0   Capacity0807           1.0
    x080707   Total_Cost          78.0   NodeBalance0707          -1.0
    x080707   NodeBalance0807           1.0   Capacity0807           1.0
    x080708   Total_Cost          78.0   NodeBalance0708          -1.0
    x080708   NodeBalance0808           1.0   Capacity0807           1.0
    x080709   Total_Cost          78.0   NodeBalance0709          -1.0
    x080709   NodeBalance0809           1.0   Capacity0807           1.0
    x080710   Total_Cost          78.0   NodeBalance0710          -1.0
    x080710   NodeBalance0810           1.0   Capacity0807           1.0
    x080901   Total_Cost         100.0   NodeBalance0801           1.0
    x080901   NodeBalance0901          -1.0   Capacity0809           1.0
    x080902   Total_Cost         100.0   NodeBalance0802           1.0
    x080902   NodeBalance0902          -1.0   Capacity0809           1.0
    x080903   Total_Cost         100.0   NodeBalance0803           1.0
    x080903   NodeBalance0903          -1.0   Capacity0809           1.0
    x080904   Total_Cost         100.0   NodeBalance0804           1.0
    x080904   NodeBalance0904          -1.0   Capacity0809           1.0
    x080905   Total_Cost         100.0   NodeBalance0805           1.0
    x080905   NodeBalance0905          -1.0   Capacity0809           1.0
    x080906   Total_Cost         100.0   NodeBalance0806           1.0
    x080906   NodeBalance0906          -1.0   Capacity0809           1.0
    x080907   Total_Cost         100.0   NodeBalance0807           1.0
    x080907   NodeBalance0907          -1.0   Capacity0809           1.0
    x080908   Total_Cost         100.0   NodeBalance0808           1.0
    x080908   NodeBalance0908          -1.0   Capacity0809           1.0
    x080909   Total_Cost         100.0   NodeBalance0809           1.0
    x080909   NodeBalance0909          -1.0   Capacity0809           1.0
    x080910   Total_Cost         100.0   NodeBalance0810           1.0
    x080910   NodeBalance0910          -1.0   Capacity0809           1.0
    x090601   Total_Cost          50.0   NodeBalance0601          -1.0
    x090601   NodeBalance0901           1.0   Capacity0906           1.0
    x090602   Total_Cost          50.0   NodeBalance0602          -1.0
    x090602   NodeBalance0902           1.0   Capacity0906           1.0
    x090603   Total_Cost          50.0   NodeBalance0603          -1.0
    x090603   NodeBalance0903           1.0   Capacity0906           1.0
    x090604   Total_Cost          50.0   NodeBalance0604          -1.0
    x090604   NodeBalance0904           1.0   Capacity0906           1.0
    x090605   Total_Cost          50.0   NodeBalance0605          -1.0
    x090605   NodeBalance0905           1.0   Capacity0906           1.0
    x090606   Total_Cost          50.0   NodeBalance0606          -1.0
    x090606   NodeBalance0906           1.0   Capacity0906           1.0
    x090607   Total_Cost          50.0   NodeBalance0607          -1.0
    x090607   NodeBalance0907           1.0   Capacity0906           1.0
    x090608   Total_Cost          50.0   NodeBalance0608          -1.0
    x090608   NodeBalance0908           1.0   Capacity0906           1.0
    x090609   Total_Cost          50.0   NodeBalance0609          -1.0
    x090609   NodeBalance0909           1.0   Capacity0906           1.0
    x090610   Total_Cost          50.0   NodeBalance0610          -1.0
    x090610   NodeBalance0910           1.0   Capacity0906           1.0
    x091001   Total_Cost         100.0   NodeBalance0901           1.0
    x091001   NodeBalance1001          -1.0   Capacity0910           1.0
    x091002   Total_Cost         100.0   NodeBalance0902           1.0
    x091002   NodeBalance1002          -1.0   Capacity0910           1.0
    x091003   Total_Cost         100.0   NodeBalance0903           1.0
    x091003   NodeBalance1003          -1.0   Capacity0910           1.0
    x091004   Total_Cost         100.0   NodeBalance0904           1.0
    x091004   NodeBalance1004          -1.0   Capacity0910           1.0
    x091005   Total_Cost         100.0   NodeBalance0905           1.0
    x091005   NodeBalance1005          -1.0   Capacity0910           1.0
    x091006   Total_Cost         100.0   NodeBalance0906           1.0
    x091006   NodeBalance1006          -1.0   Capacity0910           1.0
    x091007   Total_Cost         100.0   NodeBalance0907           1.0
    x091007   NodeBalance1007          -1.0   Capacity0910           1.0
    x091008   Total_Cost         100.0   NodeBalance0908           1.0
    x091008   NodeBalance1008          -1.0   Capacity0910           1.0
    x091009   Total_Cost         100.0   NodeBalance0909           1.0
    x091009   NodeBalance1009          -1.0   Capacity0910           1.0
    x091010   Total_Cost         100.0   NodeBalance0910           1.0
    x091010   NodeBalance1010          -1.0   Capacity0910           1.0
    x100101   Total_Cost         100.0   NodeBalance0101          -1.0
    x100101   NodeBalance1001           1.0   Capacity1001           1.0
    x100102   Total_Cost         100.0   NodeBalance0102          -1.0
    x100102   NodeBalance1002           1.0   Capacity1001           1.0
    x100103   Total_Cost         100.0   NodeBalance0103          -1.0
    x100103   NodeBalance1003           1.0   Capacity1001           1.0
    x100104   Total_Cost         100.0   NodeBalance0104          -1.0
    x100104   NodeBalance1004           1.0   Capacity1001           1.0
    x100105   Total_Cost         100.0   NodeBalance0105          -1.0
    x100105   NodeBalance1005           1.0   Capacity1001           1.0
    x100106   Total_Cost         100.0   NodeBalance0106          -1.0
    x100106   NodeBalance1006           1.0   Capacity1001           1.0
    x100107   Total_Cost         100.0   NodeBalance0107          -1.0
    x100107   NodeBalance1007           1.0   Capacity1001           1.0
    x100108   Total_Cost         100.0   NodeBalance0108          -1.0
    x100108   NodeBalance1008           1.0   Capacity1001           1.0
    x100109   Total_Cost         100.0   NodeBalance0109          -1.0
    x100109   NodeBalance1009           1.0   Capacity1001           1.0
    x100110   Total_Cost         100.0   NodeBalance0110          -1.0
    x100110   NodeBalance1010           1.0   Capacity1001           1.0
    x100601   Total_Cost          48.0   NodeBalance0601          -1.0
    x100601   NodeBalance1001           1.0   Capacity1006           1.0
    x100602   Total_Cost          48.0   NodeBalance0602          -1.0
    x100602   NodeBalance1002           1.0   Capacity1006           1.0
    x100603   Total_Cost          48.0   NodeBalance0603          -1.0
    x100603   NodeBalance1003           1.0   Capacity1006           1.0
    x100604   Total_Cost          48.0   NodeBalance0604          -1.0
    x100604   NodeBalance1004           1.0   Capacity1006           1.0
    x100605   Total_Cost          48.0   NodeBalance0605          -1.0
    x100605   NodeBalance1005           1.0   Capacity1006           1.0
    x100606   Total_Cost          48.0   NodeBalance0606          -1.0
    x100606   NodeBalance1006           1.0   Capacity1006           1.0
    x100607   Total_Cost          48.0   NodeBalance0607          -1.0
    x100607   NodeBalance1007           1.0   Capacity1006           1.0
    x100608   Total_Cost          48.0   NodeBalance0608          -1.0
    x100608   NodeBalance1008           1.0   Capacity1006           1.0
    x100609   Total_Cost          48.0   NodeBalance0609          -1.0
    x100609   NodeBalance1009           1.0   Capacity1006           1.0
    x100610   Total_Cost          48.0   NodeBalance0610          -1.0
    x100610   NodeBalance1010           1.0   Capacity1006           1.0
    z01       Total_Cost      100000.0   NodeBalance0601          -1.0
    z01       NodeBalance0801           1.0
    z02       Total_Cost      100000.0   NodeBalance0602           1.0
    z02       NodeBalance1002          -1.0
    z03       Total_Cost      100000.0   NodeBalance0203          -1.0
    z03       NodeBalance0403           1.0
    z04       Total_Cost      100000.0   NodeBalance0404           1.0
    z04       NodeBalance0704          -1.0
    z05       Total_Cost      100000.0   NodeBalance0205           1.0
    z05       NodeBalance0805          -1.0
    z06       Total_Cost      100000.0   NodeBalance0406          -1.0
    z06       NodeBalance1006           1.0
    z07       Total_Cost      100000.0   NodeBalance0107          -1.0
    z07       NodeBalance0307           1.0
    z08       Total_Cost      100000.0   NodeBalance0108          -1.0
    z08       NodeBalance0808           1.0
    z09       Total_Cost      100000.0   NodeBalance0109          -1.0
    z09       NodeBalance0609           1.0
    z10       Total_Cost      100000.0   NodeBalance0810          -1.0
    z10       NodeBalance0910           1.0
RHS
    RHS1      NodeBalance0107      -32.7192   NodeBalance0108      -28.8356
    RHS1      NodeBalance0109       -35.012   NodeBalance0203      -80.2841
    RHS1      NodeBalance0205       34.7487   NodeBalance0307       32.7192
    RHS1      NodeBalance0403       80.2841   NodeBalance0404       37.8382
    RHS1      NodeBalance0406       -13.601   NodeBalance0601      -29.4625
    RHS1      NodeBalance0602       24.2297   NodeBalance0609        35.012
    RHS1      NodeBalance0704      -37.8382   NodeBalance0801       29.4625
    RHS1      NodeBalance0805      -34.7487   NodeBalance0808       28.8356
    RHS1      NodeBalance0810      -69.6096   NodeBalance0910       69.6096
    RHS1      NodeBalance1002      -24.2297   NodeBalance1006        13.601
BOUNDS
 BV BOUND1    y0102              1.0
 BV BOUND1    y0107              1.0
 BV BOUND1    y0108              1.0
 BV BOUND1    y0110              1.0
 BV BOUND1    y0203              1.0
 BV BOUND1    y0204              1.0
 BV BOUND1    y0207              1.0
 BV BOUND1    y0208              1.0
 BV BOUND1    y0209              1.0
 BV BOUND1    y0301              1.0
 BV BOUND1    y0302              1.0
 BV BOUND1    y0304              1.0
 BV BOUND1    y0307              1.0
 BV BOUND1    y0310              1.0
 BV BOUND1    y0403              1.0
 BV BOUND1    y0405              1.0
 BV BOUND1    y0410              1.0
 BV BOUND1    y0502              1.0
 BV BOUND1    y0503              1.0
 BV BOUND1    y0506              1.0
 BV BOUND1    y0508              1.0
 BV BOUND1    y0607              1.0
 BV BOUND1    y0608              1.0
 BV BOUND1    y0610              1.0
 BV BOUND1    y0701              1.0
 BV BOUND1    y0708              1.0
 BV BOUND1    y0709              1.0
 BV BOUND1    y0802              1.0
 BV BOUND1    y0803              1.0
 BV BOUND1    y0807              1.0
 BV BOUND1    y0809              1.0
 BV BOUND1    y0906              1.0
 BV BOUND1    y0910              1.0
 BV BOUND1    y1001              1.0
 BV BOUND1    y1006              1.0
*
TIME          MCFNDSD  (MIN)
PERIODS       IMPLICIT
    y0102     NodeBalance0101                 Stage1    
    x010201   NodeBalance0101                 Stage2    
*
STOCH         MCFNDSD  (MIN)
SCENARIOS
 SC Scen1     'ROOT'        0.015625   Stage1    
    RHS1      NodeBalance0107      -32.7192
    RHS1      NodeBalance0108      -28.8356
    RHS1      NodeBalance0109       -35.012
    RHS1      NodeBalance0203      -80.2841
    RHS1      NodeBalance0205       34.7487
    RHS1      NodeBalance0307       32.7192
    RHS1      NodeBalance0403       80.2841
    RHS1      NodeBalance0404       37.8382
    RHS1      NodeBalance0406       -13.601
    RHS1      NodeBalance0601      -29.4625
    RHS1      NodeBalance0602       24.2297
    RHS1      NodeBalance0609        35.012
    RHS1      NodeBalance0704      -37.8382
    RHS1      NodeBalance0801       29.4625
    RHS1      NodeBalance0805      -34.7487
    RHS1      NodeBalance0808       28.8356
    RHS1      NodeBalance0810      -69.6096
    RHS1      NodeBalance0910       69.6096
    RHS1      NodeBalance1002      -24.2297
    RHS1      NodeBalance1006        13.601
 SC Scen2     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -30.1962
    RHS1      NodeBalance0108      -52.4241
    RHS1      NodeBalance0109      -36.7791
    RHS1      NodeBalance0203      -31.8777
    RHS1      NodeBalance0205       23.7085
    RHS1      NodeBalance0307       30.1962
    RHS1      NodeBalance0403       31.8777
    RHS1      NodeBalance0404       19.2795
    RHS1      NodeBalance0406      -58.4562
    RHS1      NodeBalance0601      -51.9656
    RHS1      NodeBalance0602       14.3995
    RHS1      NodeBalance0609       36.7791
    RHS1      NodeBalance0704      -19.2795
    RHS1      NodeBalance0801       51.9656
    RHS1      NodeBalance0805      -23.7085
    RHS1      NodeBalance0808       52.4241
    RHS1      NodeBalance0810      -34.9333
    RHS1      NodeBalance0910       34.9333
    RHS1      NodeBalance1002      -14.3995
    RHS1      NodeBalance1006       58.4562
 SC Scen3     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -39.3741
    RHS1      NodeBalance0108      -42.9567
    RHS1      NodeBalance0109      -62.2533
    RHS1      NodeBalance0203      -71.0464
    RHS1      NodeBalance0205       11.9742
    RHS1      NodeBalance0307       39.3741
    RHS1      NodeBalance0403       71.0464
    RHS1      NodeBalance0404        46.201
    RHS1      NodeBalance0406      -63.7643
    RHS1      NodeBalance0601        -26.09
    RHS1      NodeBalance0602       37.1579
    RHS1      NodeBalance0609       62.2533
    RHS1      NodeBalance0704       -46.201
    RHS1      NodeBalance0801         26.09
    RHS1      NodeBalance0805      -11.9742
    RHS1      NodeBalance0808       42.9567
    RHS1      NodeBalance0810       -35.627
    RHS1      NodeBalance0910        35.627
    RHS1      NodeBalance1002      -37.1579
    RHS1      NodeBalance1006       63.7643
 SC Scen4     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -50.4759
    RHS1      NodeBalance0108      -53.5454
    RHS1      NodeBalance0109      -48.6446
    RHS1      NodeBalance0203      -89.3453
    RHS1      NodeBalance0205        25.628
    RHS1      NodeBalance0307       50.4759
    RHS1      NodeBalance0403       89.3453
    RHS1      NodeBalance0404       18.5244
    RHS1      NodeBalance0406      -73.0863
    RHS1      NodeBalance0601      -67.7709
    RHS1      NodeBalance0602       27.3177
    RHS1      NodeBalance0609       48.6446
    RHS1      NodeBalance0704      -18.5244
    RHS1      NodeBalance0801       67.7709
    RHS1      NodeBalance0805       -25.628
    RHS1      NodeBalance0808       53.5454
    RHS1      NodeBalance0810      -72.0594
    RHS1      NodeBalance0910       72.0594
    RHS1      NodeBalance1002      -27.3177
    RHS1      NodeBalance1006       73.0863
 SC Scen5     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -52.6124
    RHS1      NodeBalance0108      -71.7978
    RHS1      NodeBalance0109      -45.2101
    RHS1      NodeBalance0203      -89.1326
    RHS1      NodeBalance0205       29.0482
    RHS1      NodeBalance0307       52.6124
    RHS1      NodeBalance0403       89.1326
    RHS1      NodeBalance0404       46.4347
    RHS1      NodeBalance0406      -94.4062
    RHS1      NodeBalance0601      -66.7594
    RHS1      NodeBalance0602       38.0531
    RHS1      NodeBalance0609       45.2101
    RHS1      NodeBalance0704      -46.4347
    RHS1      NodeBalance0801       66.7594
    RHS1      NodeBalance0805      -29.0482
    RHS1      NodeBalance0808       71.7978
    RHS1      NodeBalance0810      -22.7997
    RHS1      NodeBalance0910       22.7997
    RHS1      NodeBalance1002      -38.0531
    RHS1      NodeBalance1006       94.4062
 SC Scen6     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -14.5078
    RHS1      NodeBalance0108      -19.1185
    RHS1      NodeBalance0109      -64.7719
    RHS1      NodeBalance0203      -79.7244
    RHS1      NodeBalance0205       45.7727
    RHS1      NodeBalance0307       14.5078
    RHS1      NodeBalance0403       79.7244
    RHS1      NodeBalance0404       17.5831
    RHS1      NodeBalance0406      -80.9511
    RHS1      NodeBalance0601      -87.2253
    RHS1      NodeBalance0602       18.6742
    RHS1      NodeBalance0609       64.7719
    RHS1      NodeBalance0704      -17.5831
    RHS1      NodeBalance0801       87.2253
    RHS1      NodeBalance0805      -45.7727
    RHS1      NodeBalance0808       19.1185
    RHS1      NodeBalance0810      -21.2164
    RHS1      NodeBalance0910       21.2164
    RHS1      NodeBalance1002      -18.6742
    RHS1      NodeBalance1006       80.9511
 SC Scen7     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -31.1846
    RHS1      NodeBalance0108      -63.5167
    RHS1      NodeBalance0109      -47.6615
    RHS1      NodeBalance0203      -58.7637
    RHS1      NodeBalance0205       23.2681
    RHS1      NodeBalance0307       31.1846
    RHS1      NodeBalance0403       58.7637
    RHS1      NodeBalance0404       14.1145
    RHS1      NodeBalance0406      -32.3716
    RHS1      NodeBalance0601      -41.4091
    RHS1      NodeBalance0602       3.86721
    RHS1      NodeBalance0609       47.6615
    RHS1      NodeBalance0704      -14.1145
    RHS1      NodeBalance0801       41.4091
    RHS1      NodeBalance0805      -23.2681
    RHS1      NodeBalance0808       63.5167
    RHS1      NodeBalance0810      -33.1358
    RHS1      NodeBalance0910       33.1358
    RHS1      NodeBalance1002      -3.86721
    RHS1      NodeBalance1006       32.3716
 SC Scen8     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -44.6232
    RHS1      NodeBalance0108       -48.912
    RHS1      NodeBalance0109      -30.2329
    RHS1      NodeBalance0203      -81.5184
    RHS1      NodeBalance0205       61.0872
    RHS1      NodeBalance0307       44.6232
    RHS1      NodeBalance0403       81.5184
    RHS1      NodeBalance0404       37.1176
    RHS1      NodeBalance0406      -83.4343
    RHS1      NodeBalance0601      -62.1984
    RHS1      NodeBalance0602       24.0689
    RHS1      NodeBalance0609       30.2329
    RHS1      NodeBalance0704      -37.1176
    RHS1      NodeBalance0801       62.1984
    RHS1      NodeBalance0805      -61.0872
    RHS1      NodeBalance0808        48.912
    RHS1      NodeBalance0810      -25.8145
    RHS1      NodeBalance0910       25.8145
    RHS1      NodeBalance1002      -24.0689
    RHS1      NodeBalance1006       83.4343
 SC Scen9     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -36.1879
    RHS1      NodeBalance0108      -77.2426
    RHS1      NodeBalance0109      -17.9206
    RHS1      NodeBalance0203      -29.1169
    RHS1      NodeBalance0205       60.3754
    RHS1      NodeBalance0307       36.1879
    RHS1      NodeBalance0403       29.1169
    RHS1      NodeBalance0404        11.882
    RHS1      NodeBalance0406      -70.2018
    RHS1      NodeBalance0601      -38.4104
    RHS1      NodeBalance0602       22.5388
    RHS1      NodeBalance0609       17.9206
    RHS1      NodeBalance0704       -11.882
    RHS1      NodeBalance0801       38.4104
    RHS1      NodeBalance0805      -60.3754
    RHS1      NodeBalance0808       77.2426
    RHS1      NodeBalance0810      -16.7781
    RHS1      NodeBalance0910       16.7781
    RHS1      NodeBalance1002      -22.5388
    RHS1      NodeBalance1006       70.2018
 SC Scen10    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -42.3533
    RHS1      NodeBalance0108      -44.6769
    RHS1      NodeBalance0109      -46.9033
    RHS1      NodeBalance0203      -55.7222
    RHS1      NodeBalance0205       25.5084
    RHS1      NodeBalance0307       42.3533
    RHS1      NodeBalance0403       55.7222
    RHS1      NodeBalance0404       32.9614
    RHS1      NodeBalance0406      -84.5796
    RHS1      NodeBalance0601      -44.1966
    RHS1      NodeBalance0602       20.4265
    RHS1      NodeBalance0609       46.9033
    RHS1      NodeBalance0704      -32.9614
    RHS1      NodeBalance0801       44.1966
    RHS1      NodeBalance0805      -25.5084
    RHS1      NodeBalance0808       44.6769
    RHS1      NodeBalance0810      -71.4754
    RHS1      NodeBalance0910       71.4754
    RHS1      NodeBalance1002      -20.4265
    RHS1      NodeBalance1006       84.5796
 SC Scen11    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -40.5458
    RHS1      NodeBalance0108      -16.4808
    RHS1      NodeBalance0109      -37.9296
    RHS1      NodeBalance0203      -72.6584
    RHS1      NodeBalance0205       28.6724
    RHS1      NodeBalance0307       40.5458
    RHS1      NodeBalance0403       72.6584
    RHS1      NodeBalance0404       48.4257
    RHS1      NodeBalance0406      -72.8315
    RHS1      NodeBalance0601        -28.39
    RHS1      NodeBalance0602       32.2551
    RHS1      NodeBalance0609       37.9296
    RHS1      NodeBalance0704      -48.4257
    RHS1      NodeBalance0801         28.39
    RHS1      NodeBalance0805      -28.6724
    RHS1      NodeBalance0808       16.4808
    RHS1      NodeBalance0810      -46.6417
    RHS1      NodeBalance0910       46.6417
    RHS1      NodeBalance1002      -32.2551
    RHS1      NodeBalance1006       72.8315
 SC Scen12    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -38.2074
    RHS1      NodeBalance0108      -33.5948
    RHS1      NodeBalance0109      -40.5114
    RHS1      NodeBalance0203      -49.9192
    RHS1      NodeBalance0205       45.5058
    RHS1      NodeBalance0307       38.2074
    RHS1      NodeBalance0403       49.9192
    RHS1      NodeBalance0404        1.7413
    RHS1      NodeBalance0406      -37.0559
    RHS1      NodeBalance0601      -64.2331
    RHS1      NodeBalance0602       4.75718
    RHS1      NodeBalance0609       40.5114
    RHS1      NodeBalance0704       -1.7413
    RHS1      NodeBalance0801       64.2331
    RHS1      NodeBalance0805      -45.5058
    RHS1      NodeBalance0808       33.5948
    RHS1      NodeBalance0810      -27.6619
    RHS1      NodeBalance0910       27.6619
    RHS1      NodeBalance1002      -4.75718
    RHS1      NodeBalance1006       37.0559
 SC Scen13    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -42.5878
    RHS1      NodeBalance0108      -34.3959
    RHS1      NodeBalance0109      -52.1552
    RHS1      NodeBalance0203       -31.211
    RHS1      NodeBalance0205       42.6618
    RHS1      NodeBalance0307       42.5878
    RHS1      NodeBalance0403        31.211
    RHS1      NodeBalance0404       27.6196
    RHS1      NodeBalance0406      -22.4708
    RHS1      NodeBalance0601       -65.497
    RHS1      NodeBalance0602       25.0435
    RHS1      NodeBalance0609       52.1552
    RHS1      NodeBalance0704      -27.6196
    RHS1      NodeBalance0801        65.497
    RHS1      NodeBalance0805      -42.6618
    RHS1      NodeBalance0808       34.3959
    RHS1      NodeBalance0810      -36.7761
    RHS1      NodeBalance0910       36.7761
    RHS1      NodeBalance1002      -25.0435
    RHS1      NodeBalance1006       22.4708
 SC Scen14    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -50.2775
    RHS1      NodeBalance0108      -75.4864
    RHS1      NodeBalance0109      -41.9219
    RHS1      NodeBalance0203      -69.1561
    RHS1      NodeBalance0205       19.2029
    RHS1      NodeBalance0307       50.2775
    RHS1      NodeBalance0403       69.1561
    RHS1      NodeBalance0404       41.6006
    RHS1      NodeBalance0406      -45.7986
    RHS1      NodeBalance0601      -36.4016
    RHS1      NodeBalance0602       24.6033
    RHS1      NodeBalance0609       41.9219
    RHS1      NodeBalance0704      -41.6006
    RHS1      NodeBalance0801       36.4016
    RHS1      NodeBalance0805      -19.2029
    RHS1      NodeBalance0808       75.4864
    RHS1      NodeBalance0810      -77.6922
    RHS1      NodeBalance0910       77.6922
    RHS1      NodeBalance1002      -24.6033
    RHS1      NodeBalance1006       45.7986
 SC Scen15    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -8.66093
    RHS1      NodeBalance0108      -54.5358
    RHS1      NodeBalance0109      -55.1664
    RHS1      NodeBalance0203      -87.0893
    RHS1      NodeBalance0205       42.9789
    RHS1      NodeBalance0307       8.66093
    RHS1      NodeBalance0403       87.0893
    RHS1      NodeBalance0404       39.5803
    RHS1      NodeBalance0406      -58.0777
    RHS1      NodeBalance0601        -69.81
    RHS1      NodeBalance0602       28.2454
    RHS1      NodeBalance0609       55.1664
    RHS1      NodeBalance0704      -39.5803
    RHS1      NodeBalance0801         69.81
    RHS1      NodeBalance0805      -42.9789
    RHS1      NodeBalance0808       54.5358
    RHS1      NodeBalance0810      -65.0565
    RHS1      NodeBalance0910       65.0565
    RHS1      NodeBalance1002      -28.2454
    RHS1      NodeBalance1006       58.0777
 SC Scen16    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -51.9597
    RHS1      NodeBalance0108      -12.1617
    RHS1      NodeBalance0109      -12.5468
    RHS1      NodeBalance0203      -53.9358
    RHS1      NodeBalance0205       44.8877
    RHS1      NodeBalance0307       51.9597
    RHS1      NodeBalance0403       53.9358
    RHS1      NodeBalance0404       24.9023
    RHS1      NodeBalance0406      -61.9476
    RHS1      NodeBalance0601      -33.3654
    RHS1      NodeBalance0602        11.734
    RHS1      NodeBalance0609       12.5468
    RHS1      NodeBalance0704      -24.9023
    RHS1      NodeBalance0801       33.3654
    RHS1      NodeBalance0805      -44.8877
    RHS1      NodeBalance0808       12.1617
    RHS1      NodeBalance0810      -57.5709
    RHS1      NodeBalance0910       57.5709
    RHS1      NodeBalance1002       -11.734
    RHS1      NodeBalance1006       61.9476
 SC Scen17    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -60.2228
    RHS1      NodeBalance0108      -60.7349
    RHS1      NodeBalance0109      -44.8517
    RHS1      NodeBalance0203      -89.5876
    RHS1      NodeBalance0205       10.9277
    RHS1      NodeBalance0307       60.2228
    RHS1      NodeBalance0403       89.5876
    RHS1      NodeBalance0404       34.3448
    RHS1      NodeBalance0406      -60.7103
    RHS1      NodeBalance0601      -64.9918
    RHS1      NodeBalance0602       16.9631
    RHS1      NodeBalance0609       44.8517
    RHS1      NodeBalance0704      -34.3448
    RHS1      NodeBalance0801       64.9918
    RHS1      NodeBalance0805      -10.9277
    RHS1      NodeBalance0808       60.7349
    RHS1      NodeBalance0810      -84.9627
    RHS1      NodeBalance0910       84.9627
    RHS1      NodeBalance1002      -16.9631
    RHS1      NodeBalance1006       60.7103
 SC Scen18    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -66.2416
    RHS1      NodeBalance0108      -78.0557
    RHS1      NodeBalance0109      -53.5933
    RHS1      NodeBalance0203      -78.8252
    RHS1      NodeBalance0205       42.4838
    RHS1      NodeBalance0307       66.2416
    RHS1      NodeBalance0403       78.8252
    RHS1      NodeBalance0404       19.0973
    RHS1      NodeBalance0406      -58.8872
    RHS1      NodeBalance0601      -34.2634
    RHS1      NodeBalance0602       34.7183
    RHS1      NodeBalance0609       53.5933
    RHS1      NodeBalance0704      -19.0973
    RHS1      NodeBalance0801       34.2634
    RHS1      NodeBalance0805      -42.4838
    RHS1      NodeBalance0808       78.0557
    RHS1      NodeBalance0810      -84.2405
    RHS1      NodeBalance0910       84.2405
    RHS1      NodeBalance1002      -34.7183
    RHS1      NodeBalance1006       58.8872
 SC Scen19    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -44.545
    RHS1      NodeBalance0108      -71.0241
    RHS1      NodeBalance0109      -40.3909
    RHS1      NodeBalance0203      -81.9284
    RHS1      NodeBalance0205       49.8237
    RHS1      NodeBalance0307        44.545
    RHS1      NodeBalance0403       81.9284
    RHS1      NodeBalance0404       30.9376
    RHS1      NodeBalance0406         -41.8
    RHS1      NodeBalance0601      -70.5844
    RHS1      NodeBalance0602       26.8539
    RHS1      NodeBalance0609       40.3909
    RHS1      NodeBalance0704      -30.9376
    RHS1      NodeBalance0801       70.5844
    RHS1      NodeBalance0805      -49.8237
    RHS1      NodeBalance0808       71.0241
    RHS1      NodeBalance0810      -43.7634
    RHS1      NodeBalance0910       43.7634
    RHS1      NodeBalance1002      -26.8539
    RHS1      NodeBalance1006          41.8
 SC Scen20    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -35.8807
    RHS1      NodeBalance0108      -65.6001
    RHS1      NodeBalance0109      -48.1632
    RHS1      NodeBalance0203      -67.5338
    RHS1      NodeBalance0205       56.5811
    RHS1      NodeBalance0307       35.8807
    RHS1      NodeBalance0403       67.5338
    RHS1      NodeBalance0404       31.0786
    RHS1      NodeBalance0406      -57.9744
    RHS1      NodeBalance0601       -71.638
    RHS1      NodeBalance0602       8.92028
    RHS1      NodeBalance0609       48.1632
    RHS1      NodeBalance0704      -31.0786
    RHS1      NodeBalance0801        71.638
    RHS1      NodeBalance0805      -56.5811
    RHS1      NodeBalance0808       65.6001
    RHS1      NodeBalance0810      -41.2789
    RHS1      NodeBalance0910       41.2789
    RHS1      NodeBalance1002      -8.92028
    RHS1      NodeBalance1006       57.9744
 SC Scen21    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -43.6494
    RHS1      NodeBalance0108      -56.6805
    RHS1      NodeBalance0109      -62.4453
    RHS1      NodeBalance0203       -66.986
    RHS1      NodeBalance0205       60.9613
    RHS1      NodeBalance0307       43.6494
    RHS1      NodeBalance0403        66.986
    RHS1      NodeBalance0404       14.6108
    RHS1      NodeBalance0406      -92.2495
    RHS1      NodeBalance0601      -43.1729
    RHS1      NodeBalance0602       16.3471
    RHS1      NodeBalance0609       62.4453
    RHS1      NodeBalance0704      -14.6108
    RHS1      NodeBalance0801       43.1729
    RHS1      NodeBalance0805      -60.9613
    RHS1      NodeBalance0808       56.6805
    RHS1      NodeBalance0810      -19.3131
    RHS1      NodeBalance0910       19.3131
    RHS1      NodeBalance1002      -16.3471
    RHS1      NodeBalance1006       92.2495
 SC Scen22    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -67.6973
    RHS1      NodeBalance0108      -67.9796
    RHS1      NodeBalance0109      -40.9483
    RHS1      NodeBalance0203      -83.6148
    RHS1      NodeBalance0205       20.6164
    RHS1      NodeBalance0307       67.6973
    RHS1      NodeBalance0403       83.6148
    RHS1      NodeBalance0404       24.7895
    RHS1      NodeBalance0406      -61.4243
    RHS1      NodeBalance0601        -76.57
    RHS1      NodeBalance0602       27.5344
    RHS1      NodeBalance0609       40.9483
    RHS1      NodeBalance0704      -24.7895
    RHS1      NodeBalance0801         76.57
    RHS1      NodeBalance0805      -20.6164
    RHS1      NodeBalance0808       67.9796
    RHS1      NodeBalance0810       -38.416
    RHS1      NodeBalance0910        38.416
    RHS1      NodeBalance1002      -27.5344
    RHS1      NodeBalance1006       61.4243
 SC Scen23    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -26.3242
    RHS1      NodeBalance0108      -38.9607
    RHS1      NodeBalance0109        -55.62
    RHS1      NodeBalance0203      -78.3777
    RHS1      NodeBalance0205       36.2475
    RHS1      NodeBalance0307       26.3242
    RHS1      NodeBalance0403       78.3777
    RHS1      NodeBalance0404       47.2902
    RHS1      NodeBalance0406      -46.3225
    RHS1      NodeBalance0601      -61.1718
    RHS1      NodeBalance0602       37.7893
    RHS1      NodeBalance0609         55.62
    RHS1      NodeBalance0704      -47.2902
    RHS1      NodeBalance0801       61.1718
    RHS1      NodeBalance0805      -36.2475
    RHS1      NodeBalance0808       38.9607
    RHS1      NodeBalance0810      -35.8805
    RHS1      NodeBalance0910       35.8805
    RHS1      NodeBalance1002      -37.7893
    RHS1      NodeBalance1006       46.3225
 SC Scen24    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -38.196
    RHS1      NodeBalance0108      -76.4689
    RHS1      NodeBalance0109      -47.7332
    RHS1      NodeBalance0203       -63.247
    RHS1      NodeBalance0205       24.4018
    RHS1      NodeBalance0307        38.196
    RHS1      NodeBalance0403        63.247
    RHS1      NodeBalance0404       30.8876
    RHS1      NodeBalance0406      -40.8365
    RHS1      NodeBalance0601      -37.6208
    RHS1      NodeBalance0602       30.5282
    RHS1      NodeBalance0609       47.7332
    RHS1      NodeBalance0704      -30.8876
    RHS1      NodeBalance0801       37.6208
    RHS1      NodeBalance0805      -24.4018
    RHS1      NodeBalance0808       76.4689
    RHS1      NodeBalance0810      -86.2626
    RHS1      NodeBalance0910       86.2626
    RHS1      NodeBalance1002      -30.5282
    RHS1      NodeBalance1006       40.8365
 SC Scen25    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -44.8427
    RHS1      NodeBalance0108      -54.5894
    RHS1      NodeBalance0109      -43.1261
    RHS1      NodeBalance0203      -67.4019
    RHS1      NodeBalance0205       46.3653
    RHS1      NodeBalance0307       44.8427
    RHS1      NodeBalance0403       67.4019
    RHS1      NodeBalance0404       25.6056
    RHS1      NodeBalance0406      -80.2291
    RHS1      NodeBalance0601      -19.7467
    RHS1      NodeBalance0602       6.06785
    RHS1      NodeBalance0609       43.1261
    RHS1      NodeBalance0704      -25.6056
    RHS1      NodeBalance0801       19.7467
    RHS1      NodeBalance0805      -46.3653
    RHS1      NodeBalance0808       54.5894
    RHS1      NodeBalance0810      -68.8145
    RHS1      NodeBalance0910       68.8145
    RHS1      NodeBalance1002      -6.06785
    RHS1      NodeBalance1006       80.2291
 SC Scen26    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -59.6518
    RHS1      NodeBalance0108      -73.1778
    RHS1      NodeBalance0109      -43.9283
    RHS1      NodeBalance0203      -93.2068
    RHS1      NodeBalance0205       29.4242
    RHS1      NodeBalance0307       59.6518
    RHS1      NodeBalance0403       93.2068
    RHS1      NodeBalance0404       39.3823
    RHS1      NodeBalance0406      -70.1634
    RHS1      NodeBalance0601      -79.7099
    RHS1      NodeBalance0602       34.8075
    RHS1      NodeBalance0609       43.9283
    RHS1      NodeBalance0704      -39.3823
    RHS1      NodeBalance0801       79.7099
    RHS1      NodeBalance0805      -29.4242
    RHS1      NodeBalance0808       73.1778
    RHS1      NodeBalance0810       -28.922
    RHS1      NodeBalance0910        28.922
    RHS1      NodeBalance1002      -34.8075
    RHS1      NodeBalance1006       70.1634
 SC Scen27    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -59.6212
    RHS1      NodeBalance0108      -67.3772
    RHS1      NodeBalance0109      -41.0703
    RHS1      NodeBalance0203      -83.9045
    RHS1      NodeBalance0205       60.0566
    RHS1      NodeBalance0307       59.6212
    RHS1      NodeBalance0403       83.9045
    RHS1      NodeBalance0404       34.8464
    RHS1      NodeBalance0406      -35.7537
    RHS1      NodeBalance0601      -23.5227
    RHS1      NodeBalance0602       21.1043
    RHS1      NodeBalance0609       41.0703
    RHS1      NodeBalance0704      -34.8464
    RHS1      NodeBalance0801       23.5227
    RHS1      NodeBalance0805      -60.0566
    RHS1      NodeBalance0808       67.3772
    RHS1      NodeBalance0810      -42.4144
    RHS1      NodeBalance0910       42.4144
    RHS1      NodeBalance1002      -21.1043
    RHS1      NodeBalance1006       35.7537
 SC Scen28    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -12.1686
    RHS1      NodeBalance0108      -44.1296
    RHS1      NodeBalance0109      -18.6573
    RHS1      NodeBalance0203      -35.7775
    RHS1      NodeBalance0205       13.2068
    RHS1      NodeBalance0307       12.1686
    RHS1      NodeBalance0403       35.7775
    RHS1      NodeBalance0404       36.1307
    RHS1      NodeBalance0406      -55.0935
    RHS1      NodeBalance0601      -31.4828
    RHS1      NodeBalance0602       24.8579
    RHS1      NodeBalance0609       18.6573
    RHS1      NodeBalance0704      -36.1307
    RHS1      NodeBalance0801       31.4828
    RHS1      NodeBalance0805      -13.2068
    RHS1      NodeBalance0808       44.1296
    RHS1      NodeBalance0810      -47.6233
    RHS1      NodeBalance0910       47.6233
    RHS1      NodeBalance1002      -24.8579
    RHS1      NodeBalance1006       55.0935
 SC Scen29    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -39.297
    RHS1      NodeBalance0108      -22.6489
    RHS1      NodeBalance0109      -53.3098
    RHS1      NodeBalance0203      -61.8523
    RHS1      NodeBalance0205       48.7706
    RHS1      NodeBalance0307        39.297
    RHS1      NodeBalance0403       61.8523
    RHS1      NodeBalance0404       29.0797
    RHS1      NodeBalance0406      -49.5522
    RHS1      NodeBalance0601      -48.9981
    RHS1      NodeBalance0602       23.8013
    RHS1      NodeBalance0609       53.3098
    RHS1      NodeBalance0704      -29.0797
    RHS1      NodeBalance0801       48.9981
    RHS1      NodeBalance0805      -48.7706
    RHS1      NodeBalance0808       22.6489
    RHS1      NodeBalance0810      -37.3857
    RHS1      NodeBalance0910       37.3857
    RHS1      NodeBalance1002      -23.8013
    RHS1      NodeBalance1006       49.5522
 SC Scen30    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -61.8858
    RHS1      NodeBalance0108      -59.2523
    RHS1      NodeBalance0109      -64.5966
    RHS1      NodeBalance0203      -77.3327
    RHS1      NodeBalance0205       53.9917
    RHS1      NodeBalance0307       61.8858
    RHS1      NodeBalance0403       77.3327
    RHS1      NodeBalance0404       37.8224
    RHS1      NodeBalance0406      -93.7271
    RHS1      NodeBalance0601      -80.1449
    RHS1      NodeBalance0602       26.5764
    RHS1      NodeBalance0609       64.5966
    RHS1      NodeBalance0704      -37.8224
    RHS1      NodeBalance0801       80.1449
    RHS1      NodeBalance0805      -53.9917
    RHS1      NodeBalance0808       59.2523
    RHS1      NodeBalance0810      -59.8109
    RHS1      NodeBalance0910       59.8109
    RHS1      NodeBalance1002      -26.5764
    RHS1      NodeBalance1006       93.7271
 SC Scen31    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -29.8998
    RHS1      NodeBalance0108      -57.4383
    RHS1      NodeBalance0109      -60.1675
    RHS1      NodeBalance0203      -54.0896
    RHS1      NodeBalance0205       25.1001
    RHS1      NodeBalance0307       29.8998
    RHS1      NodeBalance0403       54.0896
    RHS1      NodeBalance0404       21.1974
    RHS1      NodeBalance0406      -23.8128
    RHS1      NodeBalance0601      -47.7635
    RHS1      NodeBalance0602       25.3024
    RHS1      NodeBalance0609       60.1675
    RHS1      NodeBalance0704      -21.1974
    RHS1      NodeBalance0801       47.7635
    RHS1      NodeBalance0805      -25.1001
    RHS1      NodeBalance0808       57.4383
    RHS1      NodeBalance0810       -75.495
    RHS1      NodeBalance0910        75.495
    RHS1      NodeBalance1002      -25.3024
    RHS1      NodeBalance1006       23.8128
 SC Scen32    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -69.8012
    RHS1      NodeBalance0108      -85.7457
    RHS1      NodeBalance0109      -49.5537
    RHS1      NodeBalance0203      -37.2468
    RHS1      NodeBalance0205       38.1681
    RHS1      NodeBalance0307       69.8012
    RHS1      NodeBalance0403       37.2468
    RHS1      NodeBalance0404       28.4181
    RHS1      NodeBalance0406      -59.9233
    RHS1      NodeBalance0601      -32.9061
    RHS1      NodeBalance0602       22.1266
    RHS1      NodeBalance0609       49.5537
    RHS1      NodeBalance0704      -28.4181
    RHS1      NodeBalance0801       32.9061
    RHS1      NodeBalance0805      -38.1681
    RHS1      NodeBalance0808       85.7457
    RHS1      NodeBalance0810      -56.2068
    RHS1      NodeBalance0910       56.2068
    RHS1      NodeBalance1002      -22.1266
    RHS1      NodeBalance1006       59.9233
 SC Scen33    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -9.26239
    RHS1      NodeBalance0108      -24.0809
    RHS1      NodeBalance0109      -54.8939
    RHS1      NodeBalance0203      -92.0549
    RHS1      NodeBalance0205       25.9861
    RHS1      NodeBalance0307       9.26239
    RHS1      NodeBalance0403       92.0549
    RHS1      NodeBalance0404       44.6922
    RHS1      NodeBalance0406      -81.2154
    RHS1      NodeBalance0601      -22.8269
    RHS1      NodeBalance0602       28.7597
    RHS1      NodeBalance0609       54.8939
    RHS1      NodeBalance0704      -44.6922
    RHS1      NodeBalance0801       22.8269
    RHS1      NodeBalance0805      -25.9861
    RHS1      NodeBalance0808       24.0809
    RHS1      NodeBalance0810      -61.1472
    RHS1      NodeBalance0910       61.1472
    RHS1      NodeBalance1002      -28.7597
    RHS1      NodeBalance1006       81.2154
 SC Scen34    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -46.0397
    RHS1      NodeBalance0108        -40.72
    RHS1      NodeBalance0109       -10.663
    RHS1      NodeBalance0203      -56.5442
    RHS1      NodeBalance0205       35.7561
    RHS1      NodeBalance0307       46.0397
    RHS1      NodeBalance0403       56.5442
    RHS1      NodeBalance0404       32.0586
    RHS1      NodeBalance0406      -72.0618
    RHS1      NodeBalance0601       -10.543
    RHS1      NodeBalance0602       32.2059
    RHS1      NodeBalance0609        10.663
    RHS1      NodeBalance0704      -32.0586
    RHS1      NodeBalance0801        10.543
    RHS1      NodeBalance0805      -35.7561
    RHS1      NodeBalance0808         40.72
    RHS1      NodeBalance0810      -59.7408
    RHS1      NodeBalance0910       59.7408
    RHS1      NodeBalance1002      -32.2059
    RHS1      NodeBalance1006       72.0618
 SC Scen35    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -57.6482
    RHS1      NodeBalance0108      -74.0945
    RHS1      NodeBalance0109      -46.0388
    RHS1      NodeBalance0203      -56.6659
    RHS1      NodeBalance0205       43.4696
    RHS1      NodeBalance0307       57.6482
    RHS1      NodeBalance0403       56.6659
    RHS1      NodeBalance0404        6.7977
    RHS1      NodeBalance0406      -44.1183
    RHS1      NodeBalance0601      -35.9851
    RHS1      NodeBalance0602       15.0674
    RHS1      NodeBalance0609       46.0388
    RHS1      NodeBalance0704       -6.7977
    RHS1      NodeBalance0801       35.9851
    RHS1      NodeBalance0805      -43.4696
    RHS1      NodeBalance0808       74.0945
    RHS1      NodeBalance0810      -82.3309
    RHS1      NodeBalance0910       82.3309
    RHS1      NodeBalance1002      -15.0674
    RHS1      NodeBalance1006       44.1183
 SC Scen36    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -34.0652
    RHS1      NodeBalance0108      -43.4826
    RHS1      NodeBalance0109      -43.0955
    RHS1      NodeBalance0203      -67.0339
    RHS1      NodeBalance0205       43.5922
    RHS1      NodeBalance0307       34.0652
    RHS1      NodeBalance0403       67.0339
    RHS1      NodeBalance0404       32.9295
    RHS1      NodeBalance0406      -77.0244
    RHS1      NodeBalance0601      -46.8171
    RHS1      NodeBalance0602       31.7597
    RHS1      NodeBalance0609       43.0955
    RHS1      NodeBalance0704      -32.9295
    RHS1      NodeBalance0801       46.8171
    RHS1      NodeBalance0805      -43.5922
    RHS1      NodeBalance0808       43.4826
    RHS1      NodeBalance0810       -79.735
    RHS1      NodeBalance0910        79.735
    RHS1      NodeBalance1002      -31.7597
    RHS1      NodeBalance1006       77.0244
 SC Scen37    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -58.4967
    RHS1      NodeBalance0108       -57.943
    RHS1      NodeBalance0109      -46.7832
    RHS1      NodeBalance0203      -11.7568
    RHS1      NodeBalance0205        46.897
    RHS1      NodeBalance0307       58.4967
    RHS1      NodeBalance0403       11.7568
    RHS1      NodeBalance0404       45.3542
    RHS1      NodeBalance0406      -34.5292
    RHS1      NodeBalance0601      -43.8581
    RHS1      NodeBalance0602       25.1768
    RHS1      NodeBalance0609       46.7832
    RHS1      NodeBalance0704      -45.3542
    RHS1      NodeBalance0801       43.8581
    RHS1      NodeBalance0805       -46.897
    RHS1      NodeBalance0808        57.943
    RHS1      NodeBalance0810      -24.6256
    RHS1      NodeBalance0910       24.6256
    RHS1      NodeBalance1002      -25.1768
    RHS1      NodeBalance1006       34.5292
 SC Scen38    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -15.1271
    RHS1      NodeBalance0108      -42.8836
    RHS1      NodeBalance0109       -51.975
    RHS1      NodeBalance0203      -53.4785
    RHS1      NodeBalance0205       38.5982
    RHS1      NodeBalance0307       15.1271
    RHS1      NodeBalance0403       53.4785
    RHS1      NodeBalance0404       45.5301
    RHS1      NodeBalance0406      -74.0073
    RHS1      NodeBalance0601      -51.0917
    RHS1      NodeBalance0602       35.0066
    RHS1      NodeBalance0609        51.975
    RHS1      NodeBalance0704      -45.5301
    RHS1      NodeBalance0801       51.0917
    RHS1      NodeBalance0805      -38.5982
    RHS1      NodeBalance0808       42.8836
    RHS1      NodeBalance0810      -58.9371
    RHS1      NodeBalance0910       58.9371
    RHS1      NodeBalance1002      -35.0066
    RHS1      NodeBalance1006       74.0073
 SC Scen39    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -63.8469
    RHS1      NodeBalance0108       -74.271
    RHS1      NodeBalance0109      -29.2873
    RHS1      NodeBalance0203      -77.5955
    RHS1      NodeBalance0205       59.1225
    RHS1      NodeBalance0307       63.8469
    RHS1      NodeBalance0403       77.5955
    RHS1      NodeBalance0404       29.9113
    RHS1      NodeBalance0406      -72.6522
    RHS1      NodeBalance0601      -48.3845
    RHS1      NodeBalance0602       21.5179
    RHS1      NodeBalance0609       29.2873
    RHS1      NodeBalance0704      -29.9113
    RHS1      NodeBalance0801       48.3845
    RHS1      NodeBalance0805      -59.1225
    RHS1      NodeBalance0808        74.271
    RHS1      NodeBalance0810      -74.8602
    RHS1      NodeBalance0910       74.8602
    RHS1      NodeBalance1002      -21.5179
    RHS1      NodeBalance1006       72.6522
 SC Scen40    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -56.989
    RHS1      NodeBalance0108      -82.5292
    RHS1      NodeBalance0109      -60.3314
    RHS1      NodeBalance0203      -28.4408
    RHS1      NodeBalance0205       43.8982
    RHS1      NodeBalance0307        56.989
    RHS1      NodeBalance0403       28.4408
    RHS1      NodeBalance0404       30.2048
    RHS1      NodeBalance0406      -29.7646
    RHS1      NodeBalance0601      -75.8708
    RHS1      NodeBalance0602       9.30046
    RHS1      NodeBalance0609       60.3314
    RHS1      NodeBalance0704      -30.2048
    RHS1      NodeBalance0801       75.8708
    RHS1      NodeBalance0805      -43.8982
    RHS1      NodeBalance0808       82.5292
    RHS1      NodeBalance0810      -71.7401
    RHS1      NodeBalance0910       71.7401
    RHS1      NodeBalance1002      -9.30046
    RHS1      NodeBalance1006       29.7646
 SC Scen41    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -24.928
    RHS1      NodeBalance0108      -47.3201
    RHS1      NodeBalance0109      -35.7361
    RHS1      NodeBalance0203      -45.4729
    RHS1      NodeBalance0205       52.5965
    RHS1      NodeBalance0307        24.928
    RHS1      NodeBalance0403       45.4729
    RHS1      NodeBalance0404       27.0086
    RHS1      NodeBalance0406      -55.9161
    RHS1      NodeBalance0601      -6.16428
    RHS1      NodeBalance0602       27.2937
    RHS1      NodeBalance0609       35.7361
    RHS1      NodeBalance0704      -27.0086
    RHS1      NodeBalance0801       6.16428
    RHS1      NodeBalance0805      -52.5965
    RHS1      NodeBalance0808       47.3201
    RHS1      NodeBalance0810      -51.3964
    RHS1      NodeBalance0910       51.3964
    RHS1      NodeBalance1002      -27.2937
    RHS1      NodeBalance1006       55.9161
 SC Scen42    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -64.5145
    RHS1      NodeBalance0108      -84.6424
    RHS1      NodeBalance0109      -50.3271
    RHS1      NodeBalance0203      -58.0568
    RHS1      NodeBalance0205       32.4124
    RHS1      NodeBalance0307       64.5145
    RHS1      NodeBalance0403       58.0568
    RHS1      NodeBalance0404       20.1137
    RHS1      NodeBalance0406      -23.4016
    RHS1      NodeBalance0601      -82.8372
    RHS1      NodeBalance0602       7.24202
    RHS1      NodeBalance0609       50.3271
    RHS1      NodeBalance0704      -20.1137
    RHS1      NodeBalance0801       82.8372
    RHS1      NodeBalance0805      -32.4124
    RHS1      NodeBalance0808       84.6424
    RHS1      NodeBalance0810      -65.4125
    RHS1      NodeBalance0910       65.4125
    RHS1      NodeBalance1002      -7.24202
    RHS1      NodeBalance1006       23.4016
 SC Scen43    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -58.9584
    RHS1      NodeBalance0108       -72.827
    RHS1      NodeBalance0109      -40.5814
    RHS1      NodeBalance0203      -87.4124
    RHS1      NodeBalance0205       44.2251
    RHS1      NodeBalance0307       58.9584
    RHS1      NodeBalance0403       87.4124
    RHS1      NodeBalance0404       15.3483
    RHS1      NodeBalance0406      -40.5875
    RHS1      NodeBalance0601      -50.7974
    RHS1      NodeBalance0602        26.776
    RHS1      NodeBalance0609       40.5814
    RHS1      NodeBalance0704      -15.3483
    RHS1      NodeBalance0801       50.7974
    RHS1      NodeBalance0805      -44.2251
    RHS1      NodeBalance0808        72.827
    RHS1      NodeBalance0810      -42.8793
    RHS1      NodeBalance0910       42.8793
    RHS1      NodeBalance1002       -26.776
    RHS1      NodeBalance1006       40.5875
 SC Scen44    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -37.6456
    RHS1      NodeBalance0108      -36.1909
    RHS1      NodeBalance0109      -52.7514
    RHS1      NodeBalance0203      -46.6922
    RHS1      NodeBalance0205       7.86502
    RHS1      NodeBalance0307       37.6456
    RHS1      NodeBalance0403       46.6922
    RHS1      NodeBalance0404       12.7879
    RHS1      NodeBalance0406      -85.0343
    RHS1      NodeBalance0601      -74.5548
    RHS1      NodeBalance0602       22.2441
    RHS1      NodeBalance0609       52.7514
    RHS1      NodeBalance0704      -12.7879
    RHS1      NodeBalance0801       74.5548
    RHS1      NodeBalance0805      -7.86502
    RHS1      NodeBalance0808       36.1909
    RHS1      NodeBalance0810      -61.5547
    RHS1      NodeBalance0910       61.5547
    RHS1      NodeBalance1002      -22.2441
    RHS1      NodeBalance1006       85.0343
 SC Scen45    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -30.5544
    RHS1      NodeBalance0108      -39.3244
    RHS1      NodeBalance0109      -59.4541
    RHS1      NodeBalance0203      -76.4311
    RHS1      NodeBalance0205        48.036
    RHS1      NodeBalance0307       30.5544
    RHS1      NodeBalance0403       76.4311
    RHS1      NodeBalance0404       21.2635
    RHS1      NodeBalance0406      -33.7556
    RHS1      NodeBalance0601       -44.866
    RHS1      NodeBalance0602       20.2084
    RHS1      NodeBalance0609       59.4541
    RHS1      NodeBalance0704      -21.2635
    RHS1      NodeBalance0801        44.866
    RHS1      NodeBalance0805       -48.036
    RHS1      NodeBalance0808       39.3244
    RHS1      NodeBalance0810      -62.7602
    RHS1      NodeBalance0910       62.7602
    RHS1      NodeBalance1002      -20.2084
    RHS1      NodeBalance1006       33.7556
 SC Scen46    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.7092
    RHS1      NodeBalance0108      -61.4753
    RHS1      NodeBalance0109      -39.3202
    RHS1      NodeBalance0203      -43.2496
    RHS1      NodeBalance0205       33.4404
    RHS1      NodeBalance0307       49.7092
    RHS1      NodeBalance0403       43.2496
    RHS1      NodeBalance0404       39.8541
    RHS1      NodeBalance0406      -68.1103
    RHS1      NodeBalance0601      -30.4918
    RHS1      NodeBalance0602       33.2076
    RHS1      NodeBalance0609       39.3202
    RHS1      NodeBalance0704      -39.8541
    RHS1      NodeBalance0801       30.4918
    RHS1      NodeBalance0805      -33.4404
    RHS1      NodeBalance0808       61.4753
    RHS1      NodeBalance0810      -87.6354
    RHS1      NodeBalance0910       87.6354
    RHS1      NodeBalance1002      -33.2076
    RHS1      NodeBalance1006       68.1103
 SC Scen47    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -56.086
    RHS1      NodeBalance0108      -72.2333
    RHS1      NodeBalance0109       -41.795
    RHS1      NodeBalance0203      -51.6564
    RHS1      NodeBalance0205        51.846
    RHS1      NodeBalance0307        56.086
    RHS1      NodeBalance0403       51.6564
    RHS1      NodeBalance0404       32.5289
    RHS1      NodeBalance0406      -20.0364
    RHS1      NodeBalance0601      -81.4911
    RHS1      NodeBalance0602       24.4635
    RHS1      NodeBalance0609        41.795
    RHS1      NodeBalance0704      -32.5289
    RHS1      NodeBalance0801       81.4911
    RHS1      NodeBalance0805       -51.846
    RHS1      NodeBalance0808       72.2333
    RHS1      NodeBalance0810      -51.0427
    RHS1      NodeBalance0910       51.0427
    RHS1      NodeBalance1002      -24.4635
    RHS1      NodeBalance1006       20.0364
 SC Scen48    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -55.7571
    RHS1      NodeBalance0108      -62.0482
    RHS1      NodeBalance0109      -38.9026
    RHS1      NodeBalance0203       -73.788
    RHS1      NodeBalance0205       45.9091
    RHS1      NodeBalance0307       55.7571
    RHS1      NodeBalance0403        73.788
    RHS1      NodeBalance0404       33.3034
    RHS1      NodeBalance0406      -84.3817
    RHS1      NodeBalance0601      -30.9918
    RHS1      NodeBalance0602       32.5608
    RHS1      NodeBalance0609       38.9026
    RHS1      NodeBalance0704      -33.3034
    RHS1      NodeBalance0801       30.9918
    RHS1      NodeBalance0805      -45.9091
    RHS1      NodeBalance0808       62.0482
    RHS1      NodeBalance0810      -52.5082
    RHS1      NodeBalance0910       52.5082
    RHS1      NodeBalance1002      -32.5608
    RHS1      NodeBalance1006       84.3817
 SC Scen49    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -24.681
    RHS1      NodeBalance0108      -65.9032
    RHS1      NodeBalance0109      -54.7615
    RHS1      NodeBalance0203      -82.5361
    RHS1      NodeBalance0205       27.3548
    RHS1      NodeBalance0307        24.681
    RHS1      NodeBalance0403       82.5361
    RHS1      NodeBalance0404       16.9994
    RHS1      NodeBalance0406      -55.4942
    RHS1      NodeBalance0601      -68.7868
    RHS1      NodeBalance0602       10.9194
    RHS1      NodeBalance0609       54.7615
    RHS1      NodeBalance0704      -16.9994
    RHS1      NodeBalance0801       68.7868
    RHS1      NodeBalance0805      -27.3548
    RHS1      NodeBalance0808       65.9032
    RHS1      NodeBalance0810      -70.5487
    RHS1      NodeBalance0910       70.5487
    RHS1      NodeBalance1002      -10.9194
    RHS1      NodeBalance1006       55.4942
 SC Scen50    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -51.9782
    RHS1      NodeBalance0108      -67.6581
    RHS1      NodeBalance0109       -57.284
    RHS1      NodeBalance0203      -90.6127
    RHS1      NodeBalance0205        52.102
    RHS1      NodeBalance0307       51.9782
    RHS1      NodeBalance0403       90.6127
    RHS1      NodeBalance0404       8.48641
    RHS1      NodeBalance0406      -95.5393
    RHS1      NodeBalance0601      -49.6045
    RHS1      NodeBalance0602       1.43412
    RHS1      NodeBalance0609        57.284
    RHS1      NodeBalance0704      -8.48641
    RHS1      NodeBalance0801       49.6045
    RHS1      NodeBalance0805       -52.102
    RHS1      NodeBalance0808       67.6581
    RHS1      NodeBalance0810      -37.5911
    RHS1      NodeBalance0910       37.5911
    RHS1      NodeBalance1002      -1.43412
    RHS1      NodeBalance1006       95.5393
 SC Scen51    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -41.4133
    RHS1      NodeBalance0108      -70.4395
    RHS1      NodeBalance0109      -58.3597
    RHS1      NodeBalance0203      -62.4734
    RHS1      NodeBalance0205       54.3471
    RHS1      NodeBalance0307       41.4133
    RHS1      NodeBalance0403       62.4734
    RHS1      NodeBalance0404       31.7085
    RHS1      NodeBalance0406      -86.4693
    RHS1      NodeBalance0601       -59.877
    RHS1      NodeBalance0602       27.1056
    RHS1      NodeBalance0609       58.3597
    RHS1      NodeBalance0704      -31.7085
    RHS1      NodeBalance0801        59.877
    RHS1      NodeBalance0805      -54.3471
    RHS1      NodeBalance0808       70.4395
    RHS1      NodeBalance0810      -63.6631
    RHS1      NodeBalance0910       63.6631
    RHS1      NodeBalance1002      -27.1056
    RHS1      NodeBalance1006       86.4693
 SC Scen52    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -38.3045
    RHS1      NodeBalance0108      -54.2929
    RHS1      NodeBalance0109      -20.3371
    RHS1      NodeBalance0203      -90.4734
    RHS1      NodeBalance0205       55.7552
    RHS1      NodeBalance0307       38.3045
    RHS1      NodeBalance0403       90.4734
    RHS1      NodeBalance0404       40.9348
    RHS1      NodeBalance0406      -64.8753
    RHS1      NodeBalance0601      -54.7912
    RHS1      NodeBalance0602       19.6807
    RHS1      NodeBalance0609       20.3371
    RHS1      NodeBalance0704      -40.9348
    RHS1      NodeBalance0801       54.7912
    RHS1      NodeBalance0805      -55.7552
    RHS1      NodeBalance0808       54.2929
    RHS1      NodeBalance0810      -81.5753
    RHS1      NodeBalance0910       81.5753
    RHS1      NodeBalance1002      -19.6807
    RHS1      NodeBalance1006       64.8753
 SC Scen53    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -28.9314
    RHS1      NodeBalance0108      -37.8235
    RHS1      NodeBalance0109      -36.4303
    RHS1      NodeBalance0203      -83.1198
    RHS1      NodeBalance0205       37.6199
    RHS1      NodeBalance0307       28.9314
    RHS1      NodeBalance0403       83.1198
    RHS1      NodeBalance0404       37.0194
    RHS1      NodeBalance0406      -42.0506
    RHS1      NodeBalance0601      -54.2359
    RHS1      NodeBalance0602       30.9598
    RHS1      NodeBalance0609       36.4303
    RHS1      NodeBalance0704      -37.0194
    RHS1      NodeBalance0801       54.2359
    RHS1      NodeBalance0805      -37.6199
    RHS1      NodeBalance0808       37.8235
    RHS1      NodeBalance0810      -69.9219
    RHS1      NodeBalance0910       69.9219
    RHS1      NodeBalance1002      -30.9598
    RHS1      NodeBalance1006       42.0506
 SC Scen54    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -24.8973
    RHS1      NodeBalance0108      -65.1811
    RHS1      NodeBalance0109      -57.5489
    RHS1      NodeBalance0203      -74.5276
    RHS1      NodeBalance0205       54.4909
    RHS1      NodeBalance0307       24.8973
    RHS1      NodeBalance0403       74.5276
    RHS1      NodeBalance0404       43.0566
    RHS1      NodeBalance0406       -79.073
    RHS1      NodeBalance0601      -67.5178
    RHS1      NodeBalance0602        26.998
    RHS1      NodeBalance0609       57.5489
    RHS1      NodeBalance0704      -43.0566
    RHS1      NodeBalance0801       67.5178
    RHS1      NodeBalance0805      -54.4909
    RHS1      NodeBalance0808       65.1811
    RHS1      NodeBalance0810      -49.3621
    RHS1      NodeBalance0910       49.3621
    RHS1      NodeBalance1002       -26.998
    RHS1      NodeBalance1006        79.073
 SC Scen55    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -41.9902
    RHS1      NodeBalance0108      -32.7347
    RHS1      NodeBalance0109      -20.6461
    RHS1      NodeBalance0203      -65.0202
    RHS1      NodeBalance0205       50.1898
    RHS1      NodeBalance0307       41.9902
    RHS1      NodeBalance0403       65.0202
    RHS1      NodeBalance0404       41.9446
    RHS1      NodeBalance0406      -58.3114
    RHS1      NodeBalance0601      -34.7027
    RHS1      NodeBalance0602       38.7783
    RHS1      NodeBalance0609       20.6461
    RHS1      NodeBalance0704      -41.9446
    RHS1      NodeBalance0801       34.7027
    RHS1      NodeBalance0805      -50.1898
    RHS1      NodeBalance0808       32.7347
    RHS1      NodeBalance0810      -57.0136
    RHS1      NodeBalance0910       57.0136
    RHS1      NodeBalance1002      -38.7783
    RHS1      NodeBalance1006       58.3114
 SC Scen56    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -41.9809
    RHS1      NodeBalance0108      -37.8035
    RHS1      NodeBalance0109      -59.3098
    RHS1      NodeBalance0203      -27.4862
    RHS1      NodeBalance0205       31.5188
    RHS1      NodeBalance0307       41.9809
    RHS1      NodeBalance0403       27.4862
    RHS1      NodeBalance0404       22.6955
    RHS1      NodeBalance0406      -51.8779
    RHS1      NodeBalance0601      -73.9329
    RHS1      NodeBalance0602       28.5964
    RHS1      NodeBalance0609       59.3098
    RHS1      NodeBalance0704      -22.6955
    RHS1      NodeBalance0801       73.9329
    RHS1      NodeBalance0805      -31.5188
    RHS1      NodeBalance0808       37.8035
    RHS1      NodeBalance0810      -74.6769
    RHS1      NodeBalance0910       74.6769
    RHS1      NodeBalance1002      -28.5964
    RHS1      NodeBalance1006       51.8779
 SC Scen57    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -10.369
    RHS1      NodeBalance0108      -60.4013
    RHS1      NodeBalance0109      -44.5751
    RHS1      NodeBalance0203      -49.1646
    RHS1      NodeBalance0205       58.5537
    RHS1      NodeBalance0307        10.369
    RHS1      NodeBalance0403       49.1646
    RHS1      NodeBalance0404       32.2048
    RHS1      NodeBalance0406      -63.9272
    RHS1      NodeBalance0601      -50.2042
    RHS1      NodeBalance0602       23.3141
    RHS1      NodeBalance0609       44.5751
    RHS1      NodeBalance0704      -32.2048
    RHS1      NodeBalance0801       50.2042
    RHS1      NodeBalance0805      -58.5537
    RHS1      NodeBalance0808       60.4013
    RHS1      NodeBalance0810       -66.999
    RHS1      NodeBalance0910        66.999
    RHS1      NodeBalance1002      -23.3141
    RHS1      NodeBalance1006       63.9272
 SC Scen58    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -3.07957
    RHS1      NodeBalance0108      -4.66753
    RHS1      NodeBalance0109      -31.3328
    RHS1      NodeBalance0203      -46.1348
    RHS1      NodeBalance0205       47.4828
    RHS1      NodeBalance0307       3.07957
    RHS1      NodeBalance0403       46.1348
    RHS1      NodeBalance0404       4.66467
    RHS1      NodeBalance0406      -51.4143
    RHS1      NodeBalance0601      -39.9395
    RHS1      NodeBalance0602       11.1783
    RHS1      NodeBalance0609       31.3328
    RHS1      NodeBalance0704      -4.66467
    RHS1      NodeBalance0801       39.9395
    RHS1      NodeBalance0805      -47.4828
    RHS1      NodeBalance0808       4.66753
    RHS1      NodeBalance0810      -45.2278
    RHS1      NodeBalance0910       45.2278
    RHS1      NodeBalance1002      -11.1783
    RHS1      NodeBalance1006       51.4143
 SC Scen59    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -47.7032
    RHS1      NodeBalance0108      -70.5811
    RHS1      NodeBalance0109      -30.1797
    RHS1      NodeBalance0203      -52.6068
    RHS1      NodeBalance0205       62.3235
    RHS1      NodeBalance0307       47.7032
    RHS1      NodeBalance0403       52.6068
    RHS1      NodeBalance0404        26.217
    RHS1      NodeBalance0406      -91.8718
    RHS1      NodeBalance0601       -52.254
    RHS1      NodeBalance0602       16.7682
    RHS1      NodeBalance0609       30.1797
    RHS1      NodeBalance0704       -26.217
    RHS1      NodeBalance0801        52.254
    RHS1      NodeBalance0805      -62.3235
    RHS1      NodeBalance0808       70.5811
    RHS1      NodeBalance0810      -80.9637
    RHS1      NodeBalance0910       80.9637
    RHS1      NodeBalance1002      -16.7682
    RHS1      NodeBalance1006       91.8718
 SC Scen60    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -64.0208
    RHS1      NodeBalance0108      -83.3992
    RHS1      NodeBalance0109      -57.3379
    RHS1      NodeBalance0203      -53.0687
    RHS1      NodeBalance0205       55.4353
    RHS1      NodeBalance0307       64.0208
    RHS1      NodeBalance0403       53.0687
    RHS1      NodeBalance0404       28.8297
    RHS1      NodeBalance0406      -33.5042
    RHS1      NodeBalance0601      -40.3122
    RHS1      NodeBalance0602       12.0094
    RHS1      NodeBalance0609       57.3379
    RHS1      NodeBalance0704      -28.8297
    RHS1      NodeBalance0801       40.3122
    RHS1      NodeBalance0805      -55.4353
    RHS1      NodeBalance0808       83.3992
    RHS1      NodeBalance0810      -57.4063
    RHS1      NodeBalance0910       57.4063
    RHS1      NodeBalance1002      -12.0094
    RHS1      NodeBalance1006       33.5042
 SC Scen61    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -66.8058
    RHS1      NodeBalance0108      -81.9565
    RHS1      NodeBalance0109       -34.941
    RHS1      NodeBalance0203      -26.5649
    RHS1      NodeBalance0205       35.4997
    RHS1      NodeBalance0307       66.8058
    RHS1      NodeBalance0403       26.5649
    RHS1      NodeBalance0404       27.3693
    RHS1      NodeBalance0406      -53.7736
    RHS1      NodeBalance0601      -59.6163
    RHS1      NodeBalance0602       13.2793
    RHS1      NodeBalance0609        34.941
    RHS1      NodeBalance0704      -27.3693
    RHS1      NodeBalance0801       59.6163
    RHS1      NodeBalance0805      -35.4997
    RHS1      NodeBalance0808       81.9565
    RHS1      NodeBalance0810      -15.4004
    RHS1      NodeBalance0910       15.4004
    RHS1      NodeBalance1002      -13.2793
    RHS1      NodeBalance1006       53.7736
 SC Scen62    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -63.6961
    RHS1      NodeBalance0108      -81.5732
    RHS1      NodeBalance0109      -37.1677
    RHS1      NodeBalance0203      -50.5624
    RHS1      NodeBalance0205       51.9902
    RHS1      NodeBalance0307       63.6961
    RHS1      NodeBalance0403       50.5624
    RHS1      NodeBalance0404       42.8052
    RHS1      NodeBalance0406      -92.9464
    RHS1      NodeBalance0601      -57.7719
    RHS1      NodeBalance0602       37.5517
    RHS1      NodeBalance0609       37.1677
    RHS1      NodeBalance0704      -42.8052
    RHS1      NodeBalance0801       57.7719
    RHS1      NodeBalance0805      -51.9902
    RHS1      NodeBalance0808       81.5732
    RHS1      NodeBalance0810      -24.5119
    RHS1      NodeBalance0910       24.5119
    RHS1      NodeBalance1002      -37.5517
    RHS1      NodeBalance1006       92.9464
 SC Scen63    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -23.5187
    RHS1      NodeBalance0108      -60.9594
    RHS1      NodeBalance0109      -2.72666
    RHS1      NodeBalance0203      -66.0866
    RHS1      NodeBalance0205       58.7231
    RHS1      NodeBalance0307       23.5187
    RHS1      NodeBalance0403       66.0866
    RHS1      NodeBalance0404       35.7505
    RHS1      NodeBalance0406      -82.9982
    RHS1      NodeBalance0601      -17.9652
    RHS1      NodeBalance0602       30.8847
    RHS1      NodeBalance0609       2.72666
    RHS1      NodeBalance0704      -35.7505
    RHS1      NodeBalance0801       17.9652
    RHS1      NodeBalance0805      -58.7231
    RHS1      NodeBalance0808       60.9594
    RHS1      NodeBalance0810      -23.7303
    RHS1      NodeBalance0910       23.7303
    RHS1      NodeBalance1002      -30.8847
    RHS1      NodeBalance1006       82.9982
 SC Scen64    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -55.0519
    RHS1      NodeBalance0108      -58.3198
    RHS1      NodeBalance0109      -31.0878
    RHS1      NodeBalance0203      -41.9852
    RHS1      NodeBalance0205       49.6798
    RHS1      NodeBalance0307       55.0519
    RHS1      NodeBalance0403       41.9852
    RHS1      NodeBalance0404       21.7273
    RHS1      NodeBalance0406      -69.9918
    RHS1      NodeBalance0601      -47.1346
    RHS1      NodeBalance0602       16.5806
    RHS1      NodeBalance0609       31.0878
    RHS1      NodeBalance0704      -21.7273
    RHS1      NodeBalance0801       47.1346
    RHS1      NodeBalance0805      -49.6798
    RHS1      NodeBalance0808       58.3198
    RHS1      NodeBalance0810      -50.2011
    RHS1      NodeBalance0910       50.2011
    RHS1      NodeBalance1002      -16.5806
    RHS1      NodeBalance1006       69.9918
ENDATA



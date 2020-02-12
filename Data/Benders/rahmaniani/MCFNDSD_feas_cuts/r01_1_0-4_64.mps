*   
*   r01_1_0-4_64.mps
*   
*   Generated with the MPL Modeling System
*   Copyright (c) 1988-2018  Maximal Software
*   
*   Date:               November 1, 2018
*   Time:               15:39
*   
*   Constraints:        135
*   Variables:          385
*   Integers:           35
*   Nonzeros:           1085
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
    y0102     Total_Cost         595.0   Capacity0102        -613.0
    y0107     Total_Cost         369.0   Capacity0107        -223.0
    y0108     Total_Cost         375.0   Capacity0108        -151.0
    y0110     Total_Cost         244.0   Capacity0110         -81.0
    y0203     Total_Cost         595.0   Capacity0203        -613.0
    y0204     Total_Cost         441.0   Capacity0204        -198.0
    y0207     Total_Cost         340.0   Capacity0207        -115.0
    y0208     Total_Cost         363.0   Capacity0208        -193.0
    y0209     Total_Cost         179.0   Capacity0209        -215.0
    y0301     Total_Cost         143.0   Capacity0301        -129.0
    y0302     Total_Cost         554.0   Capacity0302        -265.0
    y0304     Total_Cost         595.0   Capacity0304        -613.0
    y0307     Total_Cost         238.0   Capacity0307        -143.0
    y0310     Total_Cost         518.0   Capacity0310        -129.0
    y0403     Total_Cost         108.0   Capacity0403        -232.0
    y0405     Total_Cost         595.0   Capacity0405        -613.0
    y0410     Total_Cost         322.0   Capacity0410        -179.0
    y0502     Total_Cost         125.0   Capacity0502         -73.0
    y0503     Total_Cost         524.0   Capacity0503        -276.0
    y0506     Total_Cost         595.0   Capacity0506        -613.0
    y0508     Total_Cost         536.0   Capacity0508        -232.0
    y0607     Total_Cost         595.0   Capacity0607        -613.0
    y0608     Total_Cost         393.0   Capacity0608        -173.0
    y0610     Total_Cost         304.0   Capacity0610        -101.0
    y0701     Total_Cost         286.0   Capacity0701        -198.0
    y0708     Total_Cost         595.0   Capacity0708        -613.0
    y0709     Total_Cost         572.0   Capacity0709        -221.0
    y0802     Total_Cost         256.0   Capacity0802        -201.0
    y0803     Total_Cost         340.0   Capacity0803        -134.0
    y0807     Total_Cost         357.0   Capacity0807        -134.0
    y0809     Total_Cost         595.0   Capacity0809        -613.0
    y0906     Total_Cost         298.0   Capacity0906        -240.0
    y0910     Total_Cost         595.0   Capacity0910        -613.0
    y1001     Total_Cost         595.0   Capacity1001        -613.0
    y1006     Total_Cost         375.0   Capacity1006        -156.0
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
RHS
    RHS1      NodeBalance0107      -29.3894   NodeBalance0108      -28.7523
    RHS1      NodeBalance0109      -32.7523   NodeBalance0203      -82.1742
    RHS1      NodeBalance0205       44.5441   NodeBalance0307       29.3894
    RHS1      NodeBalance0403       82.1742   NodeBalance0404       22.0898
    RHS1      NodeBalance0406      -97.1842   NodeBalance0601      -41.0469
    RHS1      NodeBalance0602       32.2104   NodeBalance0609       32.7523
    RHS1      NodeBalance0704      -22.0898   NodeBalance0801       41.0469
    RHS1      NodeBalance0805      -44.5441   NodeBalance0808       28.7523
    RHS1      NodeBalance0810      -40.5682   NodeBalance0910       40.5682
    RHS1      NodeBalance1002      -32.2104   NodeBalance1006       97.1842
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
    RHS1      NodeBalance0107      -29.3894
    RHS1      NodeBalance0108      -28.7523
    RHS1      NodeBalance0109      -32.7523
    RHS1      NodeBalance0203      -82.1742
    RHS1      NodeBalance0205       44.5441
    RHS1      NodeBalance0307       29.3894
    RHS1      NodeBalance0403       82.1742
    RHS1      NodeBalance0404       22.0898
    RHS1      NodeBalance0406      -97.1842
    RHS1      NodeBalance0601      -41.0469
    RHS1      NodeBalance0602       32.2104
    RHS1      NodeBalance0609       32.7523
    RHS1      NodeBalance0704      -22.0898
    RHS1      NodeBalance0801       41.0469
    RHS1      NodeBalance0805      -44.5441
    RHS1      NodeBalance0808       28.7523
    RHS1      NodeBalance0810      -40.5682
    RHS1      NodeBalance0910       40.5682
    RHS1      NodeBalance1002      -32.2104
    RHS1      NodeBalance1006       97.1842
 SC Scen2     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -28.1352
    RHS1      NodeBalance0108      -43.8047
    RHS1      NodeBalance0109      -47.1362
    RHS1      NodeBalance0203      -9.30184
    RHS1      NodeBalance0205        30.971
    RHS1      NodeBalance0307       28.1352
    RHS1      NodeBalance0403       9.30184
    RHS1      NodeBalance0404        25.495
    RHS1      NodeBalance0406      -86.3574
    RHS1      NodeBalance0601      -61.1718
    RHS1      NodeBalance0602       18.7065
    RHS1      NodeBalance0609       47.1362
    RHS1      NodeBalance0704       -25.495
    RHS1      NodeBalance0801       61.1718
    RHS1      NodeBalance0805       -30.971
    RHS1      NodeBalance0808       43.8047
    RHS1      NodeBalance0810      -6.60742
    RHS1      NodeBalance0910       6.60742
    RHS1      NodeBalance1002      -18.7065
    RHS1      NodeBalance1006       86.3574
 SC Scen3     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -55.8208
    RHS1      NodeBalance0108      -30.4044
    RHS1      NodeBalance0109      -47.1441
    RHS1      NodeBalance0203       -84.929
    RHS1      NodeBalance0205       42.2239
    RHS1      NodeBalance0307       55.8208
    RHS1      NodeBalance0403        84.929
    RHS1      NodeBalance0404       13.4007
    RHS1      NodeBalance0406      -69.3187
    RHS1      NodeBalance0601      -41.4091
    RHS1      NodeBalance0602       23.5413
    RHS1      NodeBalance0609       47.1441
    RHS1      NodeBalance0704      -13.4007
    RHS1      NodeBalance0801       41.4091
    RHS1      NodeBalance0805      -42.2239
    RHS1      NodeBalance0808       30.4044
    RHS1      NodeBalance0810      -53.7793
    RHS1      NodeBalance0910       53.7793
    RHS1      NodeBalance1002      -23.5413
    RHS1      NodeBalance1006       69.3187
 SC Scen4     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -63.0967
    RHS1      NodeBalance0108      -31.5428
    RHS1      NodeBalance0109      -34.1969
    RHS1      NodeBalance0203      -62.6833
    RHS1      NodeBalance0205        10.599
    RHS1      NodeBalance0307       63.0967
    RHS1      NodeBalance0403       62.6833
    RHS1      NodeBalance0404        42.099
    RHS1      NodeBalance0406      -28.0341
    RHS1      NodeBalance0601      -14.8049
    RHS1      NodeBalance0602       37.0466
    RHS1      NodeBalance0609       34.1969
    RHS1      NodeBalance0704       -42.099
    RHS1      NodeBalance0801       14.8049
    RHS1      NodeBalance0805       -10.599
    RHS1      NodeBalance0808       31.5428
    RHS1      NodeBalance0810      -79.8614
    RHS1      NodeBalance0910       79.8614
    RHS1      NodeBalance1002      -37.0466
    RHS1      NodeBalance1006       28.0341
 SC Scen5     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -29.7037
    RHS1      NodeBalance0108      -56.6589
    RHS1      NodeBalance0109      -31.5407
    RHS1      NodeBalance0203      -71.5346
    RHS1      NodeBalance0205       48.3174
    RHS1      NodeBalance0307       29.7037
    RHS1      NodeBalance0403       71.5346
    RHS1      NodeBalance0404       35.4829
    RHS1      NodeBalance0406       -76.493
    RHS1      NodeBalance0601      -70.0673
    RHS1      NodeBalance0602       30.5014
    RHS1      NodeBalance0609       31.5407
    RHS1      NodeBalance0704      -35.4829
    RHS1      NodeBalance0801       70.0673
    RHS1      NodeBalance0805      -48.3174
    RHS1      NodeBalance0808       56.6589
    RHS1      NodeBalance0810      -76.6438
    RHS1      NodeBalance0910       76.6438
    RHS1      NodeBalance1002      -30.5014
    RHS1      NodeBalance1006        76.493
 SC Scen6     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -60.4781
    RHS1      NodeBalance0108      -19.0902
    RHS1      NodeBalance0109      -36.6799
    RHS1      NodeBalance0203      -71.6125
    RHS1      NodeBalance0205       60.9244
    RHS1      NodeBalance0307       60.4781
    RHS1      NodeBalance0403       71.6125
    RHS1      NodeBalance0404       29.6773
    RHS1      NodeBalance0406      -70.4058
    RHS1      NodeBalance0601      -48.6922
    RHS1      NodeBalance0602       9.56244
    RHS1      NodeBalance0609       36.6799
    RHS1      NodeBalance0704      -29.6773
    RHS1      NodeBalance0801       48.6922
    RHS1      NodeBalance0805      -60.9244
    RHS1      NodeBalance0808       19.0902
    RHS1      NodeBalance0810      -45.6653
    RHS1      NodeBalance0910       45.6653
    RHS1      NodeBalance1002      -9.56244
    RHS1      NodeBalance1006       70.4058
 SC Scen7     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.2514
    RHS1      NodeBalance0108      -3.67285
    RHS1      NodeBalance0109      -25.9085
    RHS1      NodeBalance0203      -95.6781
    RHS1      NodeBalance0205       29.3829
    RHS1      NodeBalance0307       49.2514
    RHS1      NodeBalance0403       95.6781
    RHS1      NodeBalance0404       27.4097
    RHS1      NodeBalance0406      -67.7516
    RHS1      NodeBalance0601      -48.0749
    RHS1      NodeBalance0602       4.79691
    RHS1      NodeBalance0609       25.9085
    RHS1      NodeBalance0704      -27.4097
    RHS1      NodeBalance0801       48.0749
    RHS1      NodeBalance0805      -29.3829
    RHS1      NodeBalance0808       3.67285
    RHS1      NodeBalance0810      -89.3581
    RHS1      NodeBalance0910       89.3581
    RHS1      NodeBalance1002      -4.79691
    RHS1      NodeBalance1006       67.7516
 SC Scen8     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -52.9166
    RHS1      NodeBalance0108      -28.0661
    RHS1      NodeBalance0109      -18.5872
    RHS1      NodeBalance0203      -36.3027
    RHS1      NodeBalance0205       53.5062
    RHS1      NodeBalance0307       52.9166
    RHS1      NodeBalance0403       36.3027
    RHS1      NodeBalance0404       39.6804
    RHS1      NodeBalance0406      -52.8704
    RHS1      NodeBalance0601      -31.9652
    RHS1      NodeBalance0602       13.9976
    RHS1      NodeBalance0609       18.5872
    RHS1      NodeBalance0704      -39.6804
    RHS1      NodeBalance0801       31.9652
    RHS1      NodeBalance0805      -53.5062
    RHS1      NodeBalance0808       28.0661
    RHS1      NodeBalance0810      -63.3052
    RHS1      NodeBalance0910       63.3052
    RHS1      NodeBalance1002      -13.9976
    RHS1      NodeBalance1006       52.8704
 SC Scen9     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -57.0362
    RHS1      NodeBalance0108      -75.9784
    RHS1      NodeBalance0109      -42.0186
    RHS1      NodeBalance0203      -66.6232
    RHS1      NodeBalance0205       40.3813
    RHS1      NodeBalance0307       57.0362
    RHS1      NodeBalance0403       66.6232
    RHS1      NodeBalance0404       20.5904
    RHS1      NodeBalance0406      -16.6591
    RHS1      NodeBalance0601      -70.5844
    RHS1      NodeBalance0602       17.4278
    RHS1      NodeBalance0609       42.0186
    RHS1      NodeBalance0704      -20.5904
    RHS1      NodeBalance0801       70.5844
    RHS1      NodeBalance0805      -40.3813
    RHS1      NodeBalance0808       75.9784
    RHS1      NodeBalance0810       -56.512
    RHS1      NodeBalance0910        56.512
    RHS1      NodeBalance1002      -17.4278
    RHS1      NodeBalance1006       16.6591
 SC Scen10    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -50.8921
    RHS1      NodeBalance0108      -79.4795
    RHS1      NodeBalance0109      -65.6919
    RHS1      NodeBalance0203      -70.8813
    RHS1      NodeBalance0205       27.8036
    RHS1      NodeBalance0307       50.8921
    RHS1      NodeBalance0403       70.8813
    RHS1      NodeBalance0404       34.6252
    RHS1      NodeBalance0406       -77.409
    RHS1      NodeBalance0601      -45.5255
    RHS1      NodeBalance0602       12.8744
    RHS1      NodeBalance0609       65.6919
    RHS1      NodeBalance0704      -34.6252
    RHS1      NodeBalance0801       45.5255
    RHS1      NodeBalance0805      -27.8036
    RHS1      NodeBalance0808       79.4795
    RHS1      NodeBalance0810      -36.0855
    RHS1      NodeBalance0910       36.0855
    RHS1      NodeBalance1002      -12.8744
    RHS1      NodeBalance1006        77.409
 SC Scen11    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.2703
    RHS1      NodeBalance0108      -31.9559
    RHS1      NodeBalance0109      -55.7659
    RHS1      NodeBalance0203      -44.4164
    RHS1      NodeBalance0205       62.8483
    RHS1      NodeBalance0307       49.2703
    RHS1      NodeBalance0403       44.4164
    RHS1      NodeBalance0404       35.3122
    RHS1      NodeBalance0406      -36.3837
    RHS1      NodeBalance0601      -74.2411
    RHS1      NodeBalance0602       34.9048
    RHS1      NodeBalance0609       55.7659
    RHS1      NodeBalance0704      -35.3122
    RHS1      NodeBalance0801       74.2411
    RHS1      NodeBalance0805      -62.8483
    RHS1      NodeBalance0808       31.9559
    RHS1      NodeBalance0810      -85.7904
    RHS1      NodeBalance0910       85.7904
    RHS1      NodeBalance1002      -34.9048
    RHS1      NodeBalance1006       36.3837
 SC Scen12    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -58.2391
    RHS1      NodeBalance0108      -44.9884
    RHS1      NodeBalance0109      -46.3059
    RHS1      NodeBalance0203      -75.3095
    RHS1      NodeBalance0205       55.2805
    RHS1      NodeBalance0307       58.2391
    RHS1      NodeBalance0403       75.3095
    RHS1      NodeBalance0404       42.3659
    RHS1      NodeBalance0406      -33.3381
    RHS1      NodeBalance0601      -73.9329
    RHS1      NodeBalance0602        36.462
    RHS1      NodeBalance0609       46.3059
    RHS1      NodeBalance0704      -42.3659
    RHS1      NodeBalance0801       73.9329
    RHS1      NodeBalance0805      -55.2805
    RHS1      NodeBalance0808       44.9884
    RHS1      NodeBalance0810      -43.4948
    RHS1      NodeBalance0910       43.4948
    RHS1      NodeBalance1002       -36.462
    RHS1      NodeBalance1006       33.3381
 SC Scen13    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -30.0674
    RHS1      NodeBalance0108       -33.971
    RHS1      NodeBalance0109      -54.8922
    RHS1      NodeBalance0203      -38.4782
    RHS1      NodeBalance0205       47.3529
    RHS1      NodeBalance0307       30.0674
    RHS1      NodeBalance0403       38.4782
    RHS1      NodeBalance0404       47.7493
    RHS1      NodeBalance0406      -51.3799
    RHS1      NodeBalance0601      -68.2781
    RHS1      NodeBalance0602       37.0914
    RHS1      NodeBalance0609       54.8922
    RHS1      NodeBalance0704      -47.7493
    RHS1      NodeBalance0801       68.2781
    RHS1      NodeBalance0805      -47.3529
    RHS1      NodeBalance0808        33.971
    RHS1      NodeBalance0810      -71.6673
    RHS1      NodeBalance0910       71.6673
    RHS1      NodeBalance1002      -37.0914
    RHS1      NodeBalance1006       51.3799
 SC Scen14    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -33.3139
    RHS1      NodeBalance0108      -49.5278
    RHS1      NodeBalance0109      -32.2238
    RHS1      NodeBalance0203      -57.6635
    RHS1      NodeBalance0205        45.235
    RHS1      NodeBalance0307       33.3139
    RHS1      NodeBalance0403       57.6635
    RHS1      NodeBalance0404       30.7597
    RHS1      NodeBalance0406       -39.648
    RHS1      NodeBalance0601      -64.7391
    RHS1      NodeBalance0602       38.7356
    RHS1      NodeBalance0609       32.2238
    RHS1      NodeBalance0704      -30.7597
    RHS1      NodeBalance0801       64.7391
    RHS1      NodeBalance0805       -45.235
    RHS1      NodeBalance0808       49.5278
    RHS1      NodeBalance0810       -56.861
    RHS1      NodeBalance0910        56.861
    RHS1      NodeBalance1002      -38.7356
    RHS1      NodeBalance1006        39.648
 SC Scen15    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -37.7741
    RHS1      NodeBalance0108      -70.9662
    RHS1      NodeBalance0109      -46.0784
    RHS1      NodeBalance0203       -29.373
    RHS1      NodeBalance0205       48.7083
    RHS1      NodeBalance0307       37.7741
    RHS1      NodeBalance0403        29.373
    RHS1      NodeBalance0404       32.8358
    RHS1      NodeBalance0406      -90.1307
    RHS1      NodeBalance0601      -53.3942
    RHS1      NodeBalance0602       29.6946
    RHS1      NodeBalance0609       46.0784
    RHS1      NodeBalance0704      -32.8358
    RHS1      NodeBalance0801       53.3942
    RHS1      NodeBalance0805      -48.7083
    RHS1      NodeBalance0808       70.9662
    RHS1      NodeBalance0810      -12.3715
    RHS1      NodeBalance0910       12.3715
    RHS1      NodeBalance1002      -29.6946
    RHS1      NodeBalance1006       90.1307
 SC Scen16    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -62.1348
    RHS1      NodeBalance0108      -67.7113
    RHS1      NodeBalance0109      -25.5254
    RHS1      NodeBalance0203      -39.4116
    RHS1      NodeBalance0205       27.1476
    RHS1      NodeBalance0307       62.1348
    RHS1      NodeBalance0403       39.4116
    RHS1      NodeBalance0404       24.4993
    RHS1      NodeBalance0406      -69.7655
    RHS1      NodeBalance0601      -56.1601
    RHS1      NodeBalance0602       12.2641
    RHS1      NodeBalance0609       25.5254
    RHS1      NodeBalance0704      -24.4993
    RHS1      NodeBalance0801       56.1601
    RHS1      NodeBalance0805      -27.1476
    RHS1      NodeBalance0808       67.7113
    RHS1      NodeBalance0810      -41.0898
    RHS1      NodeBalance0910       41.0898
    RHS1      NodeBalance1002      -12.2641
    RHS1      NodeBalance1006       69.7655
 SC Scen17    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -57.2758
    RHS1      NodeBalance0108      -83.1512
    RHS1      NodeBalance0109      -54.9096
    RHS1      NodeBalance0203      -92.1426
    RHS1      NodeBalance0205       40.7588
    RHS1      NodeBalance0307       57.2758
    RHS1      NodeBalance0403       92.1426
    RHS1      NodeBalance0404       39.6257
    RHS1      NodeBalance0406      -62.1937
    RHS1      NodeBalance0601      -65.7495
    RHS1      NodeBalance0602        30.706
    RHS1      NodeBalance0609       54.9096
    RHS1      NodeBalance0704      -39.6257
    RHS1      NodeBalance0801       65.7495
    RHS1      NodeBalance0805      -40.7588
    RHS1      NodeBalance0808       83.1512
    RHS1      NodeBalance0810      -50.1394
    RHS1      NodeBalance0910       50.1394
    RHS1      NodeBalance1002       -30.706
    RHS1      NodeBalance1006       62.1937
 SC Scen18    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -50.8993
    RHS1      NodeBalance0108      -61.7778
    RHS1      NodeBalance0109      -35.1934
    RHS1      NodeBalance0203      -70.5339
    RHS1      NodeBalance0205       41.5013
    RHS1      NodeBalance0307       50.8993
    RHS1      NodeBalance0403       70.5339
    RHS1      NodeBalance0404       21.2041
    RHS1      NodeBalance0406      -83.1805
    RHS1      NodeBalance0601      -58.3026
    RHS1      NodeBalance0602       19.5738
    RHS1      NodeBalance0609       35.1934
    RHS1      NodeBalance0704      -21.2041
    RHS1      NodeBalance0801       58.3026
    RHS1      NodeBalance0805      -41.5013
    RHS1      NodeBalance0808       61.7778
    RHS1      NodeBalance0810      -86.0826
    RHS1      NodeBalance0910       86.0826
    RHS1      NodeBalance1002      -19.5738
    RHS1      NodeBalance1006       83.1805
 SC Scen19    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -2.88608
    RHS1      NodeBalance0108      -50.3947
    RHS1      NodeBalance0109      -63.3166
    RHS1      NodeBalance0203      -47.7215
    RHS1      NodeBalance0205       50.5835
    RHS1      NodeBalance0307       2.88608
    RHS1      NodeBalance0403       47.7215
    RHS1      NodeBalance0404       44.1038
    RHS1      NodeBalance0406      -33.8705
    RHS1      NodeBalance0601      -87.2253
    RHS1      NodeBalance0602       26.6225
    RHS1      NodeBalance0609       63.3166
    RHS1      NodeBalance0704      -44.1038
    RHS1      NodeBalance0801       87.2253
    RHS1      NodeBalance0805      -50.5835
    RHS1      NodeBalance0808       50.3947
    RHS1      NodeBalance0810      -65.3071
    RHS1      NodeBalance0910       65.3071
    RHS1      NodeBalance1002      -26.6225
    RHS1      NodeBalance1006       33.8705
 SC Scen20    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -61.8272
    RHS1      NodeBalance0108      -70.1415
    RHS1      NodeBalance0109      -39.7446
    RHS1      NodeBalance0203      -54.0449
    RHS1      NodeBalance0205       29.1258
    RHS1      NodeBalance0307       61.8272
    RHS1      NodeBalance0403       54.0449
    RHS1      NodeBalance0404       20.4968
    RHS1      NodeBalance0406      -19.1168
    RHS1      NodeBalance0601      -58.8302
    RHS1      NodeBalance0602       26.1625
    RHS1      NodeBalance0609       39.7446
    RHS1      NodeBalance0704      -20.4968
    RHS1      NodeBalance0801       58.8302
    RHS1      NodeBalance0805      -29.1258
    RHS1      NodeBalance0808       70.1415
    RHS1      NodeBalance0810       -70.103
    RHS1      NodeBalance0910        70.103
    RHS1      NodeBalance1002      -26.1625
    RHS1      NodeBalance1006       19.1168
 SC Scen21    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -69.771
    RHS1      NodeBalance0108      -88.7424
    RHS1      NodeBalance0109      -36.3903
    RHS1      NodeBalance0203      -7.57209
    RHS1      NodeBalance0205       29.3143
    RHS1      NodeBalance0307        69.771
    RHS1      NodeBalance0403       7.57209
    RHS1      NodeBalance0404       30.3001
    RHS1      NodeBalance0406      -32.8728
    RHS1      NodeBalance0601      -35.5633
    RHS1      NodeBalance0602       8.27294
    RHS1      NodeBalance0609       36.3903
    RHS1      NodeBalance0704      -30.3001
    RHS1      NodeBalance0801       35.5633
    RHS1      NodeBalance0805      -29.3143
    RHS1      NodeBalance0808       88.7424
    RHS1      NodeBalance0810      -74.5848
    RHS1      NodeBalance0910       74.5848
    RHS1      NodeBalance1002      -8.27294
    RHS1      NodeBalance1006       32.8728
 SC Scen22    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -17.8193
    RHS1      NodeBalance0108      -52.6748
    RHS1      NodeBalance0109      -46.7818
    RHS1      NodeBalance0203      -78.2157
    RHS1      NodeBalance0205       57.9926
    RHS1      NodeBalance0307       17.8193
    RHS1      NodeBalance0403       78.2157
    RHS1      NodeBalance0404       25.1418
    RHS1      NodeBalance0406      -32.3454
    RHS1      NodeBalance0601       -44.866
    RHS1      NodeBalance0602       17.7951
    RHS1      NodeBalance0609       46.7818
    RHS1      NodeBalance0704      -25.1418
    RHS1      NodeBalance0801        44.866
    RHS1      NodeBalance0805      -57.9926
    RHS1      NodeBalance0808       52.6748
    RHS1      NodeBalance0810       -51.634
    RHS1      NodeBalance0910        51.634
    RHS1      NodeBalance1002      -17.7951
    RHS1      NodeBalance1006       32.3454
 SC Scen23    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -31.9389
    RHS1      NodeBalance0108      -57.3406
    RHS1      NodeBalance0109       -16.806
    RHS1      NodeBalance0203      -66.2435
    RHS1      NodeBalance0205       33.4822
    RHS1      NodeBalance0307       31.9389
    RHS1      NodeBalance0403       66.2435
    RHS1      NodeBalance0404       40.9394
    RHS1      NodeBalance0406      -74.3085
    RHS1      NodeBalance0601      -58.0376
    RHS1      NodeBalance0602       27.2235
    RHS1      NodeBalance0609        16.806
    RHS1      NodeBalance0704      -40.9394
    RHS1      NodeBalance0801       58.0376
    RHS1      NodeBalance0805      -33.4822
    RHS1      NodeBalance0808       57.3406
    RHS1      NodeBalance0810      -55.4261
    RHS1      NodeBalance0910       55.4261
    RHS1      NodeBalance1002      -27.2235
    RHS1      NodeBalance1006       74.3085
 SC Scen24    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -19.4161
    RHS1      NodeBalance0108      -10.3172
    RHS1      NodeBalance0109      -54.4032
    RHS1      NodeBalance0203      -42.0735
    RHS1      NodeBalance0205       26.3046
    RHS1      NodeBalance0307       19.4161
    RHS1      NodeBalance0403       42.0735
    RHS1      NodeBalance0404       24.3847
    RHS1      NodeBalance0406      -42.6404
    RHS1      NodeBalance0601      -65.2445
    RHS1      NodeBalance0602       17.5853
    RHS1      NodeBalance0609       54.4032
    RHS1      NodeBalance0704      -24.3847
    RHS1      NodeBalance0801       65.2445
    RHS1      NodeBalance0805      -26.3046
    RHS1      NodeBalance0808       10.3172
    RHS1      NodeBalance0810      -85.2578
    RHS1      NodeBalance0910       85.2578
    RHS1      NodeBalance1002      -17.5853
    RHS1      NodeBalance1006       42.6404
 SC Scen25    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -7.59851
    RHS1      NodeBalance0108      -78.1459
    RHS1      NodeBalance0109       -19.131
    RHS1      NodeBalance0203       -52.551
    RHS1      NodeBalance0205       25.7541
    RHS1      NodeBalance0307       7.59851
    RHS1      NodeBalance0403        52.551
    RHS1      NodeBalance0404       10.5709
    RHS1      NodeBalance0406      -76.1447
    RHS1      NodeBalance0601       -62.709
    RHS1      NodeBalance0602       7.24338
    RHS1      NodeBalance0609        19.131
    RHS1      NodeBalance0704      -10.5709
    RHS1      NodeBalance0801        62.709
    RHS1      NodeBalance0805      -25.7541
    RHS1      NodeBalance0808       78.1459
    RHS1      NodeBalance0810      -75.0764
    RHS1      NodeBalance0910       75.0764
    RHS1      NodeBalance1002      -7.24338
    RHS1      NodeBalance1006       76.1447
 SC Scen26    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -48.8136
    RHS1      NodeBalance0108      -75.1914
    RHS1      NodeBalance0109      -31.7458
    RHS1      NodeBalance0203      -58.9653
    RHS1      NodeBalance0205       36.6013
    RHS1      NodeBalance0307       48.8136
    RHS1      NodeBalance0403       58.9653
    RHS1      NodeBalance0404        39.811
    RHS1      NodeBalance0406      -26.1482
    RHS1      NodeBalance0601      -3.17502
    RHS1      NodeBalance0602       34.2827
    RHS1      NodeBalance0609       31.7458
    RHS1      NodeBalance0704       -39.811
    RHS1      NodeBalance0801       3.17502
    RHS1      NodeBalance0805      -36.6013
    RHS1      NodeBalance0808       75.1914
    RHS1      NodeBalance0810      -67.0353
    RHS1      NodeBalance0910       67.0353
    RHS1      NodeBalance1002      -34.2827
    RHS1      NodeBalance1006       26.1482
 SC Scen27    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -50.4536
    RHS1      NodeBalance0108      -65.0561
    RHS1      NodeBalance0109      -24.2394
    RHS1      NodeBalance0203      -45.7392
    RHS1      NodeBalance0205       63.1464
    RHS1      NodeBalance0307       50.4536
    RHS1      NodeBalance0403       45.7392
    RHS1      NodeBalance0404       38.2239
    RHS1      NodeBalance0406      -49.5595
    RHS1      NodeBalance0601      -18.8812
    RHS1      NodeBalance0602       36.2463
    RHS1      NodeBalance0609       24.2394
    RHS1      NodeBalance0704      -38.2239
    RHS1      NodeBalance0801       18.8812
    RHS1      NodeBalance0805      -63.1464
    RHS1      NodeBalance0808       65.0561
    RHS1      NodeBalance0810      -82.9538
    RHS1      NodeBalance0910       82.9538
    RHS1      NodeBalance1002      -36.2463
    RHS1      NodeBalance1006       49.5595
 SC Scen28    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -42.6232
    RHS1      NodeBalance0108      -59.7713
    RHS1      NodeBalance0109      -43.8199
    RHS1      NodeBalance0203      -71.0898
    RHS1      NodeBalance0205       50.7355
    RHS1      NodeBalance0307       42.6232
    RHS1      NodeBalance0403       71.0898
    RHS1      NodeBalance0404       9.22207
    RHS1      NodeBalance0406      -41.4083
    RHS1      NodeBalance0601      -76.2167
    RHS1      NodeBalance0602       25.8587
    RHS1      NodeBalance0609       43.8199
    RHS1      NodeBalance0704      -9.22207
    RHS1      NodeBalance0801       76.2167
    RHS1      NodeBalance0805      -50.7355
    RHS1      NodeBalance0808       59.7713
    RHS1      NodeBalance0810      -78.2982
    RHS1      NodeBalance0910       78.2982
    RHS1      NodeBalance1002      -25.8587
    RHS1      NodeBalance1006       41.4083
 SC Scen29    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -13.1206
    RHS1      NodeBalance0108      -13.9583
    RHS1      NodeBalance0109      -52.3377
    RHS1      NodeBalance0203       -32.394
    RHS1      NodeBalance0205       45.4427
    RHS1      NodeBalance0307       13.1206
    RHS1      NodeBalance0403        32.394
    RHS1      NodeBalance0404       31.7891
    RHS1      NodeBalance0406      -38.6636
    RHS1      NodeBalance0601      -39.9395
    RHS1      NodeBalance0602       12.5375
    RHS1      NodeBalance0609       52.3377
    RHS1      NodeBalance0704      -31.7891
    RHS1      NodeBalance0801       39.9395
    RHS1      NodeBalance0805      -45.4427
    RHS1      NodeBalance0808       13.9583
    RHS1      NodeBalance0810      -59.2456
    RHS1      NodeBalance0910       59.2456
    RHS1      NodeBalance1002      -12.5375
    RHS1      NodeBalance1006       38.6636
 SC Scen30    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -62.1032
    RHS1      NodeBalance0108       -75.568
    RHS1      NodeBalance0109      -36.6991
    RHS1      NodeBalance0203      -74.0828
    RHS1      NodeBalance0205        26.421
    RHS1      NodeBalance0307       62.1032
    RHS1      NodeBalance0403       74.0828
    RHS1      NodeBalance0404       18.9024
    RHS1      NodeBalance0406      -27.9234
    RHS1      NodeBalance0601      -38.4104
    RHS1      NodeBalance0602       13.3623
    RHS1      NodeBalance0609       36.6991
    RHS1      NodeBalance0704      -18.9024
    RHS1      NodeBalance0801       38.4104
    RHS1      NodeBalance0805       -26.421
    RHS1      NodeBalance0808        75.568
    RHS1      NodeBalance0810      -75.4636
    RHS1      NodeBalance0910       75.4636
    RHS1      NodeBalance1002      -13.3623
    RHS1      NodeBalance1006       27.9234
 SC Scen31    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -46.3701
    RHS1      NodeBalance0108      -44.2519
    RHS1      NodeBalance0109      -57.2988
    RHS1      NodeBalance0203      -69.9595
    RHS1      NodeBalance0205       30.7544
    RHS1      NodeBalance0307       46.3701
    RHS1      NodeBalance0403       69.9595
    RHS1      NodeBalance0404       36.2349
    RHS1      NodeBalance0406      -91.1441
    RHS1      NodeBalance0601      -46.4975
    RHS1      NodeBalance0602       33.1822
    RHS1      NodeBalance0609       57.2988
    RHS1      NodeBalance0704      -36.2349
    RHS1      NodeBalance0801       46.4975
    RHS1      NodeBalance0805      -30.7544
    RHS1      NodeBalance0808       44.2519
    RHS1      NodeBalance0810      -59.9997
    RHS1      NodeBalance0910       59.9997
    RHS1      NodeBalance1002      -33.1822
    RHS1      NodeBalance1006       91.1441
 SC Scen32    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -37.404
    RHS1      NodeBalance0108      -46.0775
    RHS1      NodeBalance0109      -48.4381
    RHS1      NodeBalance0203      -68.2817
    RHS1      NodeBalance0205        35.478
    RHS1      NodeBalance0307        37.404
    RHS1      NodeBalance0403       68.2817
    RHS1      NodeBalance0404       22.0297
    RHS1      NodeBalance0406      -46.0823
    RHS1      NodeBalance0601       -61.686
    RHS1      NodeBalance0602       35.8014
    RHS1      NodeBalance0609       48.4381
    RHS1      NodeBalance0704      -22.0297
    RHS1      NodeBalance0801        61.686
    RHS1      NodeBalance0805       -35.478
    RHS1      NodeBalance0808       46.0775
    RHS1      NodeBalance0810      -66.6072
    RHS1      NodeBalance0910       66.6072
    RHS1      NodeBalance1002      -35.8014
    RHS1      NodeBalance1006       46.0823
 SC Scen33    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -32.1351
    RHS1      NodeBalance0108      -69.5052
    RHS1      NodeBalance0109      -47.0581
    RHS1      NodeBalance0203      -31.8197
    RHS1      NodeBalance0205       46.3552
    RHS1      NodeBalance0307       32.1351
    RHS1      NodeBalance0403       31.8197
    RHS1      NodeBalance0404       23.6467
    RHS1      NodeBalance0406       -67.255
    RHS1      NodeBalance0601      -66.5068
    RHS1      NodeBalance0602       34.3087
    RHS1      NodeBalance0609       47.0581
    RHS1      NodeBalance0704      -23.6467
    RHS1      NodeBalance0801       66.5068
    RHS1      NodeBalance0805      -46.3552
    RHS1      NodeBalance0808       69.5052
    RHS1      NodeBalance0810      -46.8888
    RHS1      NodeBalance0910       46.8888
    RHS1      NodeBalance1002      -34.3087
    RHS1      NodeBalance1006        67.255
 SC Scen34    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -39.3609
    RHS1      NodeBalance0108      -21.1614
    RHS1      NodeBalance0109      -43.2017
    RHS1      NodeBalance0203      -59.2067
    RHS1      NodeBalance0205       22.2921
    RHS1      NodeBalance0307       39.3609
    RHS1      NodeBalance0403       59.2067
    RHS1      NodeBalance0404       42.8412
    RHS1      NodeBalance0406      -81.4159
    RHS1      NodeBalance0601      -69.5533
    RHS1      NodeBalance0602       11.8138
    RHS1      NodeBalance0609       43.2017
    RHS1      NodeBalance0704      -42.8412
    RHS1      NodeBalance0801       69.5533
    RHS1      NodeBalance0805      -22.2921
    RHS1      NodeBalance0808       21.1614
    RHS1      NodeBalance0810      -58.7476
    RHS1      NodeBalance0910       58.7476
    RHS1      NodeBalance1002      -11.8138
    RHS1      NodeBalance1006       81.4159
 SC Scen35    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -38.2918
    RHS1      NodeBalance0108      -69.4213
    RHS1      NodeBalance0109      -47.5701
    RHS1      NodeBalance0203      -89.8865
    RHS1      NodeBalance0205       28.6622
    RHS1      NodeBalance0307       38.2918
    RHS1      NodeBalance0403       89.8865
    RHS1      NodeBalance0404       37.2803
    RHS1      NodeBalance0406      -74.8161
    RHS1      NodeBalance0601      -68.7868
    RHS1      NodeBalance0602       28.3342
    RHS1      NodeBalance0609       47.5701
    RHS1      NodeBalance0704      -37.2803
    RHS1      NodeBalance0801       68.7868
    RHS1      NodeBalance0805      -28.6622
    RHS1      NodeBalance0808       69.4213
    RHS1      NodeBalance0810      -44.4977
    RHS1      NodeBalance0910       44.4977
    RHS1      NodeBalance1002      -28.3342
    RHS1      NodeBalance1006       74.8161
 SC Scen36    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -60.1035
    RHS1      NodeBalance0108      -85.5295
    RHS1      NodeBalance0109      -29.5027
    RHS1      NodeBalance0203      -53.5119
    RHS1      NodeBalance0205       34.5371
    RHS1      NodeBalance0307       60.1035
    RHS1      NodeBalance0403       53.5119
    RHS1      NodeBalance0404       28.1679
    RHS1      NodeBalance0406      -57.2669
    RHS1      NodeBalance0601      -63.7262
    RHS1      NodeBalance0602       31.2314
    RHS1      NodeBalance0609       29.5027
    RHS1      NodeBalance0704      -28.1679
    RHS1      NodeBalance0801       63.7262
    RHS1      NodeBalance0805      -34.5371
    RHS1      NodeBalance0808       85.5295
    RHS1      NodeBalance0810      -65.5294
    RHS1      NodeBalance0910       65.5294
    RHS1      NodeBalance1002      -31.2314
    RHS1      NodeBalance1006       57.2669
 SC Scen37    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -53.6574
    RHS1      NodeBalance0108      -58.1249
    RHS1      NodeBalance0109      -51.9152
    RHS1      NodeBalance0203      -62.1303
    RHS1      NodeBalance0205       47.8276
    RHS1      NodeBalance0307       53.6574
    RHS1      NodeBalance0403       62.1303
    RHS1      NodeBalance0404        41.471
    RHS1      NodeBalance0406      -75.1186
    RHS1      NodeBalance0601      -82.8372
    RHS1      NodeBalance0602       15.6449
    RHS1      NodeBalance0609       51.9152
    RHS1      NodeBalance0704       -41.471
    RHS1      NodeBalance0801       82.8372
    RHS1      NodeBalance0805      -47.8276
    RHS1      NodeBalance0808       58.1249
    RHS1      NodeBalance0810      -79.4112
    RHS1      NodeBalance0910       79.4112
    RHS1      NodeBalance1002      -15.6449
    RHS1      NodeBalance1006       75.1186
 SC Scen38    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -48.0348
    RHS1      NodeBalance0108      -60.4489
    RHS1      NodeBalance0109      -68.7458
    RHS1      NodeBalance0203      -50.3506
    RHS1      NodeBalance0205       34.9325
    RHS1      NodeBalance0307       48.0348
    RHS1      NodeBalance0403       50.3506
    RHS1      NodeBalance0404       49.2312
    RHS1      NodeBalance0406      -69.7483
    RHS1      NodeBalance0601      -82.3849
    RHS1      NodeBalance0602       33.3278
    RHS1      NodeBalance0609       68.7458
    RHS1      NodeBalance0704      -49.2312
    RHS1      NodeBalance0801       82.3849
    RHS1      NodeBalance0805      -34.9325
    RHS1      NodeBalance0808       60.4489
    RHS1      NodeBalance0810      -82.3592
    RHS1      NodeBalance0910       82.3592
    RHS1      NodeBalance1002      -33.3278
    RHS1      NodeBalance1006       69.7483
 SC Scen39    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -21.0906
    RHS1      NodeBalance0108      -25.0403
    RHS1      NodeBalance0109      -64.1371
    RHS1      NodeBalance0203      -55.6535
    RHS1      NodeBalance0205       37.0693
    RHS1      NodeBalance0307       21.0906
    RHS1      NodeBalance0403       55.6535
    RHS1      NodeBalance0404        28.279
    RHS1      NodeBalance0406      -87.1016
    RHS1      NodeBalance0601      -81.4911
    RHS1      NodeBalance0602       22.2252
    RHS1      NodeBalance0609       64.1371
    RHS1      NodeBalance0704       -28.279
    RHS1      NodeBalance0801       81.4911
    RHS1      NodeBalance0805      -37.0693
    RHS1      NodeBalance0808       25.0403
    RHS1      NodeBalance0810      -62.4725
    RHS1      NodeBalance0910       62.4725
    RHS1      NodeBalance1002      -22.2252
    RHS1      NodeBalance1006       87.1016
 SC Scen40    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -55.0544
    RHS1      NodeBalance0108      -52.0547
    RHS1      NodeBalance0109       -62.677
    RHS1      NodeBalance0203      -65.7768
    RHS1      NodeBalance0205       39.5922
    RHS1      NodeBalance0307       55.0544
    RHS1      NodeBalance0403       65.7768
    RHS1      NodeBalance0404       24.8222
    RHS1      NodeBalance0406      -55.5705
    RHS1      NodeBalance0601       -52.254
    RHS1      NodeBalance0602       15.9509
    RHS1      NodeBalance0609        62.677
    RHS1      NodeBalance0704      -24.8222
    RHS1      NodeBalance0801        52.254
    RHS1      NodeBalance0805      -39.5922
    RHS1      NodeBalance0808       52.0547
    RHS1      NodeBalance0810      -19.2795
    RHS1      NodeBalance0910       19.2795
    RHS1      NodeBalance1002      -15.9509
    RHS1      NodeBalance1006       55.5705
 SC Scen41    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -60.7726
    RHS1      NodeBalance0108      -45.0644
    RHS1      NodeBalance0109      -53.7576
    RHS1      NodeBalance0203      -40.3561
    RHS1      NodeBalance0205       55.2287
    RHS1      NodeBalance0307       60.7726
    RHS1      NodeBalance0403       40.3561
    RHS1      NodeBalance0404       2.94625
    RHS1      NodeBalance0406      -25.8467
    RHS1      NodeBalance0601      -68.0244
    RHS1      NodeBalance0602       1.38294
    RHS1      NodeBalance0609       53.7576
    RHS1      NodeBalance0704      -2.94625
    RHS1      NodeBalance0801       68.0244
    RHS1      NodeBalance0805      -55.2287
    RHS1      NodeBalance0808       45.0644
    RHS1      NodeBalance0810      -81.7787
    RHS1      NodeBalance0910       81.7787
    RHS1      NodeBalance1002      -1.38294
    RHS1      NodeBalance1006       25.8467
 SC Scen42    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -22.8822
    RHS1      NodeBalance0108      -17.8067
    RHS1      NodeBalance0109       -59.159
    RHS1      NodeBalance0203      -44.9209
    RHS1      NodeBalance0205       54.0006
    RHS1      NodeBalance0307       22.8822
    RHS1      NodeBalance0403       44.9209
    RHS1      NodeBalance0404       34.4635
    RHS1      NodeBalance0406      -75.0321
    RHS1      NodeBalance0601      -22.8269
    RHS1      NodeBalance0602       24.3608
    RHS1      NodeBalance0609        59.159
    RHS1      NodeBalance0704      -34.4635
    RHS1      NodeBalance0801       22.8269
    RHS1      NodeBalance0805      -54.0006
    RHS1      NodeBalance0808       17.8067
    RHS1      NodeBalance0810       -46.256
    RHS1      NodeBalance0910        46.256
    RHS1      NodeBalance1002      -24.3608
    RHS1      NodeBalance1006       75.0321
 SC Scen43    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -23.4722
    RHS1      NodeBalance0108      -40.8767
    RHS1      NodeBalance0109      -50.1039
    RHS1      NodeBalance0203      -56.2925
    RHS1      NodeBalance0205       58.4028
    RHS1      NodeBalance0307       23.4722
    RHS1      NodeBalance0403       56.2925
    RHS1      NodeBalance0404       31.9391
    RHS1      NodeBalance0406       -87.949
    RHS1      NodeBalance0601      -79.7099
    RHS1      NodeBalance0602       36.6352
    RHS1      NodeBalance0609       50.1039
    RHS1      NodeBalance0704      -31.9391
    RHS1      NodeBalance0801       79.7099
    RHS1      NodeBalance0805      -58.4028
    RHS1      NodeBalance0808       40.8767
    RHS1      NodeBalance0810      -69.4115
    RHS1      NodeBalance0910       69.4115
    RHS1      NodeBalance1002      -36.6352
    RHS1      NodeBalance1006        87.949
 SC Scen44    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -57.4722
    RHS1      NodeBalance0108       -72.761
    RHS1      NodeBalance0109      -23.7296
    RHS1      NodeBalance0203       -67.713
    RHS1      NodeBalance0205       41.1791
    RHS1      NodeBalance0307       57.4722
    RHS1      NodeBalance0403        67.713
    RHS1      NodeBalance0404       23.9617
    RHS1      NodeBalance0406      -70.1169
    RHS1      NodeBalance0601      -13.5545
    RHS1      NodeBalance0602       11.7303
    RHS1      NodeBalance0609       23.7296
    RHS1      NodeBalance0704      -23.9617
    RHS1      NodeBalance0801       13.5545
    RHS1      NodeBalance0805      -41.1791
    RHS1      NodeBalance0808        72.761
    RHS1      NodeBalance0810      -63.3322
    RHS1      NodeBalance0910       63.3322
    RHS1      NodeBalance1002      -11.7303
    RHS1      NodeBalance1006       70.1169
 SC Scen45    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -20.8293
    RHS1      NodeBalance0108      -68.5536
    RHS1      NodeBalance0109      -15.7506
    RHS1      NodeBalance0203      -48.8524
    RHS1      NodeBalance0205       50.0358
    RHS1      NodeBalance0307       20.8293
    RHS1      NodeBalance0403       48.8524
    RHS1      NodeBalance0404       22.8031
    RHS1      NodeBalance0406      -23.4967
    RHS1      NodeBalance0601      -23.5227
    RHS1      NodeBalance0602       19.2574
    RHS1      NodeBalance0609       15.7506
    RHS1      NodeBalance0704      -22.8031
    RHS1      NodeBalance0801       23.5227
    RHS1      NodeBalance0805      -50.0358
    RHS1      NodeBalance0808       68.5536
    RHS1      NodeBalance0810      -49.4172
    RHS1      NodeBalance0910       49.4172
    RHS1      NodeBalance1002      -19.2574
    RHS1      NodeBalance1006       23.4967
 SC Scen46    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.1607
    RHS1      NodeBalance0108      -72.9367
    RHS1      NodeBalance0109      -40.4359
    RHS1      NodeBalance0203      -35.6817
    RHS1      NodeBalance0205       20.7371
    RHS1      NodeBalance0307       49.1607
    RHS1      NodeBalance0403       35.6817
    RHS1      NodeBalance0404       16.7996
    RHS1      NodeBalance0406      -82.8649
    RHS1      NodeBalance0601      -78.4623
    RHS1      NodeBalance0602       28.5462
    RHS1      NodeBalance0609       40.4359
    RHS1      NodeBalance0704      -16.7996
    RHS1      NodeBalance0801       78.4623
    RHS1      NodeBalance0805      -20.7371
    RHS1      NodeBalance0808       72.9367
    RHS1      NodeBalance0810      -38.1495
    RHS1      NodeBalance0910       38.1495
    RHS1      NodeBalance1002      -28.5462
    RHS1      NodeBalance1006       82.8649
 SC Scen47    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -47.2092
    RHS1      NodeBalance0108      -49.2793
    RHS1      NodeBalance0109      -46.1515
    RHS1      NodeBalance0203      -24.3488
    RHS1      NodeBalance0205        46.058
    RHS1      NodeBalance0307       47.2092
    RHS1      NodeBalance0403       24.3488
    RHS1      NodeBalance0404       37.4695
    RHS1      NodeBalance0406      -58.5404
    RHS1      NodeBalance0601      -49.3022
    RHS1      NodeBalance0602       27.9772
    RHS1      NodeBalance0609       46.1515
    RHS1      NodeBalance0704      -37.4695
    RHS1      NodeBalance0801       49.3022
    RHS1      NodeBalance0805       -46.058
    RHS1      NodeBalance0808       49.2793
    RHS1      NodeBalance0810      -68.4128
    RHS1      NodeBalance0910       68.4128
    RHS1      NodeBalance1002      -27.9772
    RHS1      NodeBalance1006       58.5404
 SC Scen48    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -46.4038
    RHS1      NodeBalance0108      -56.5861
    RHS1      NodeBalance0109      -31.5728
    RHS1      NodeBalance0203      -63.9317
    RHS1      NodeBalance0205       28.8998
    RHS1      NodeBalance0307       46.4038
    RHS1      NodeBalance0403       63.9317
    RHS1      NodeBalance0404       43.3356
    RHS1      NodeBalance0406      -22.1312
    RHS1      NodeBalance0601       -56.431
    RHS1      NodeBalance0602       29.0525
    RHS1      NodeBalance0609       31.5728
    RHS1      NodeBalance0704      -43.3356
    RHS1      NodeBalance0801        56.431
    RHS1      NodeBalance0805      -28.8998
    RHS1      NodeBalance0808       56.5861
    RHS1      NodeBalance0810      -57.1772
    RHS1      NodeBalance0910       57.1772
    RHS1      NodeBalance1002      -29.0525
    RHS1      NodeBalance1006       22.1312
 SC Scen49    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -26.6621
    RHS1      NodeBalance0108      -66.4668
    RHS1      NodeBalance0109      -57.1061
    RHS1      NodeBalance0203      -74.7986
    RHS1      NodeBalance0205       35.9919
    RHS1      NodeBalance0307       26.6621
    RHS1      NodeBalance0403       74.7986
    RHS1      NodeBalance0404       39.0861
    RHS1      NodeBalance0406      -61.5078
    RHS1      NodeBalance0601      -55.0671
    RHS1      NodeBalance0602       33.2417
    RHS1      NodeBalance0609       57.1061
    RHS1      NodeBalance0704      -39.0861
    RHS1      NodeBalance0801       55.0671
    RHS1      NodeBalance0805      -35.9919
    RHS1      NodeBalance0808       66.4668
    RHS1      NodeBalance0810      -67.6512
    RHS1      NodeBalance0910       67.6512
    RHS1      NodeBalance1002      -33.2417
    RHS1      NodeBalance1006       61.5078
 SC Scen50    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -39.805
    RHS1      NodeBalance0108       -54.159
    RHS1      NodeBalance0109       -41.886
    RHS1      NodeBalance0203       -54.529
    RHS1      NodeBalance0205        6.1039
    RHS1      NodeBalance0307        39.805
    RHS1      NodeBalance0403        54.529
    RHS1      NodeBalance0404       30.4338
    RHS1      NodeBalance0406      -73.9199
    RHS1      NodeBalance0601      -63.2183
    RHS1      NodeBalance0602       6.56376
    RHS1      NodeBalance0609        41.886
    RHS1      NodeBalance0704      -30.4338
    RHS1      NodeBalance0801       63.2183
    RHS1      NodeBalance0805       -6.1039
    RHS1      NodeBalance0808        54.159
    RHS1      NodeBalance0810      -48.4633
    RHS1      NodeBalance0910       48.4633
    RHS1      NodeBalance1002      -6.56376
    RHS1      NodeBalance1006       73.9199
 SC Scen51    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -16.9273
    RHS1      NodeBalance0108      -42.6573
    RHS1      NodeBalance0109      -23.4323
    RHS1      NodeBalance0203      -81.2449
    RHS1      NodeBalance0205       20.0169
    RHS1      NodeBalance0307       16.9273
    RHS1      NodeBalance0403       81.2449
    RHS1      NodeBalance0404       30.2169
    RHS1      NodeBalance0406      -44.8592
    RHS1      NodeBalance0601      -58.5667
    RHS1      NodeBalance0602       20.8048
    RHS1      NodeBalance0609       23.4323
    RHS1      NodeBalance0704      -30.2169
    RHS1      NodeBalance0801       58.5667
    RHS1      NodeBalance0805      -20.0169
    RHS1      NodeBalance0808       42.6573
    RHS1      NodeBalance0810      -39.2181
    RHS1      NodeBalance0910       39.2181
    RHS1      NodeBalance1002      -20.8048
    RHS1      NodeBalance1006       44.8592
 SC Scen52    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -59.6455
    RHS1      NodeBalance0108      -66.2949
    RHS1      NodeBalance0109      -28.2606
    RHS1      NodeBalance0203      -34.9825
    RHS1      NodeBalance0205       52.6844
    RHS1      NodeBalance0307       59.6455
    RHS1      NodeBalance0403       34.9825
    RHS1      NodeBalance0404       32.4567
    RHS1      NodeBalance0406      -45.6273
    RHS1      NodeBalance0601      -49.9052
    RHS1      NodeBalance0602        29.989
    RHS1      NodeBalance0609       28.2606
    RHS1      NodeBalance0704      -32.4567
    RHS1      NodeBalance0801       49.9052
    RHS1      NodeBalance0805      -52.6844
    RHS1      NodeBalance0808       66.2949
    RHS1      NodeBalance0810       -36.655
    RHS1      NodeBalance0910        36.655
    RHS1      NodeBalance1002       -29.989
    RHS1      NodeBalance1006       45.6273
 SC Scen53    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -67.0222
    RHS1      NodeBalance0108      -87.0749
    RHS1      NodeBalance0109      -59.4716
    RHS1      NodeBalance0203      -34.3135
    RHS1      NodeBalance0205       56.1449
    RHS1      NodeBalance0307       67.0222
    RHS1      NodeBalance0403       34.3135
    RHS1      NodeBalance0404        17.917
    RHS1      NodeBalance0406      -89.9748
    RHS1      NodeBalance0601      -50.2042
    RHS1      NodeBalance0602       18.6141
    RHS1      NodeBalance0609       59.4716
    RHS1      NodeBalance0704       -17.917
    RHS1      NodeBalance0801       50.2042
    RHS1      NodeBalance0805      -56.1449
    RHS1      NodeBalance0808       87.0749
    RHS1      NodeBalance0810      -67.4289
    RHS1      NodeBalance0910       67.4289
    RHS1      NodeBalance1002      -18.6141
    RHS1      NodeBalance1006       89.9748
 SC Scen54    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -49.803
    RHS1      NodeBalance0108      -65.3051
    RHS1      NodeBalance0109      -35.8708
    RHS1      NodeBalance0203      -77.6125
    RHS1      NodeBalance0205       55.3711
    RHS1      NodeBalance0307        49.803
    RHS1      NodeBalance0403       77.6125
    RHS1      NodeBalance0404       25.3482
    RHS1      NodeBalance0406      -31.6437
    RHS1      NodeBalance0601      -57.2381
    RHS1      NodeBalance0602       28.1604
    RHS1      NodeBalance0609       35.8708
    RHS1      NodeBalance0704      -25.3482
    RHS1      NodeBalance0801       57.2381
    RHS1      NodeBalance0805      -55.3711
    RHS1      NodeBalance0808       65.3051
    RHS1      NodeBalance0810      -68.9217
    RHS1      NodeBalance0910       68.9217
    RHS1      NodeBalance1002      -28.1604
    RHS1      NodeBalance1006       31.6437
 SC Scen55    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -18.8926
    RHS1      NodeBalance0108      -53.9534
    RHS1      NodeBalance0109      -39.0017
    RHS1      NodeBalance0203      -27.2008
    RHS1      NodeBalance0205       54.7878
    RHS1      NodeBalance0307       18.8926
    RHS1      NodeBalance0403       27.2008
    RHS1      NodeBalance0404       29.6195
    RHS1      NodeBalance0406      -70.6737
    RHS1      NodeBalance0601      -69.2972
    RHS1      NodeBalance0602       11.1773
    RHS1      NodeBalance0609       39.0017
    RHS1      NodeBalance0704      -29.6195
    RHS1      NodeBalance0801       69.2972
    RHS1      NodeBalance0805      -54.7878
    RHS1      NodeBalance0808       53.9534
    RHS1      NodeBalance0810      -35.5324
    RHS1      NodeBalance0910       35.5324
    RHS1      NodeBalance1002      -11.1773
    RHS1      NodeBalance1006       70.6737
 SC Scen56    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -63.7109
    RHS1      NodeBalance0108      -82.2322
    RHS1      NodeBalance0109      -37.7491
    RHS1      NodeBalance0203      -48.6386
    RHS1      NodeBalance0205       35.8325
    RHS1      NodeBalance0307       63.7109
    RHS1      NodeBalance0403       48.6386
    RHS1      NodeBalance0404       37.7616
    RHS1      NodeBalance0406      -6.56791
    RHS1      NodeBalance0601      -34.7027
    RHS1      NodeBalance0602       35.6363
    RHS1      NodeBalance0609       37.7491
    RHS1      NodeBalance0704      -37.7616
    RHS1      NodeBalance0801       34.7027
    RHS1      NodeBalance0805      -35.8325
    RHS1      NodeBalance0808       82.2322
    RHS1      NodeBalance0810      -64.1629
    RHS1      NodeBalance0910       64.1629
    RHS1      NodeBalance1002      -35.6363
    RHS1      NodeBalance1006       6.56791
 SC Scen57    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -48.0588
    RHS1      NodeBalance0108      -47.1989
    RHS1      NodeBalance0109      -34.3936
    RHS1      NodeBalance0203      -37.8693
    RHS1      NodeBalance0205       13.4081
    RHS1      NodeBalance0307       48.0588
    RHS1      NodeBalance0403       37.8693
    RHS1      NodeBalance0404       40.9166
    RHS1      NodeBalance0406      -75.8905
    RHS1      NodeBalance0601      -33.8178
    RHS1      NodeBalance0602       34.1556
    RHS1      NodeBalance0609       34.3936
    RHS1      NodeBalance0704      -40.9166
    RHS1      NodeBalance0801       33.8178
    RHS1      NodeBalance0805      -13.4081
    RHS1      NodeBalance0808       47.1989
    RHS1      NodeBalance0810      -25.4409
    RHS1      NodeBalance0910       25.4409
    RHS1      NodeBalance1002      -34.1556
    RHS1      NodeBalance1006       75.8905
 SC Scen58    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -51.2304
    RHS1      NodeBalance0108      -48.9807
    RHS1      NodeBalance0109      -50.3746
    RHS1      NodeBalance0203      -85.4868
    RHS1      NodeBalance0205       18.9784
    RHS1      NodeBalance0307       51.2304
    RHS1      NodeBalance0403       85.4868
    RHS1      NodeBalance0404       28.4572
    RHS1      NodeBalance0406      -50.4025
    RHS1      NodeBalance0601      -51.0917
    RHS1      NodeBalance0602       14.4331
    RHS1      NodeBalance0609       50.3746
    RHS1      NodeBalance0704      -28.4572
    RHS1      NodeBalance0801       51.0917
    RHS1      NodeBalance0805      -18.9784
    RHS1      NodeBalance0808       48.9807
    RHS1      NodeBalance0810      -87.3394
    RHS1      NodeBalance0910       87.3394
    RHS1      NodeBalance1002      -14.4331
    RHS1      NodeBalance1006       50.4025
 SC Scen59    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -59.0427
    RHS1      NodeBalance0108      -55.2012
    RHS1      NodeBalance0109      -63.7894
    RHS1      NodeBalance0203      -78.6974
    RHS1      NodeBalance0205       58.7661
    RHS1      NodeBalance0307       59.0427
    RHS1      NodeBalance0403       78.6974
    RHS1      NodeBalance0404       31.7322
    RHS1      NodeBalance0406       -24.212
    RHS1      NodeBalance0601      -80.1449
    RHS1      NodeBalance0602       20.9295
    RHS1      NodeBalance0609       63.7894
    RHS1      NodeBalance0704      -31.7322
    RHS1      NodeBalance0801       80.1449
    RHS1      NodeBalance0805      -58.7661
    RHS1      NodeBalance0808       55.2012
    RHS1      NodeBalance0810      -50.2338
    RHS1      NodeBalance0910       50.2338
    RHS1      NodeBalance1002      -20.9295
    RHS1      NodeBalance1006        24.212
 SC Scen60    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -59.4676
    RHS1      NodeBalance0108      -24.2116
    RHS1      NodeBalance0109      -61.2913
    RHS1      NodeBalance0203      -50.9598
    RHS1      NodeBalance0205       53.3135
    RHS1      NodeBalance0307       59.4676
    RHS1      NodeBalance0403       50.9598
    RHS1      NodeBalance0404       35.9993
    RHS1      NodeBalance0406      -43.6926
    RHS1      NodeBalance0601      -85.9988
    RHS1      NodeBalance0602       24.5056
    RHS1      NodeBalance0609       61.2913
    RHS1      NodeBalance0704      -35.9993
    RHS1      NodeBalance0801       85.9988
    RHS1      NodeBalance0805      -53.3135
    RHS1      NodeBalance0808       24.2116
    RHS1      NodeBalance0810      -66.6934
    RHS1      NodeBalance0910       66.6934
    RHS1      NodeBalance1002      -24.5056
    RHS1      NodeBalance1006       43.6926
 SC Scen61    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -32.4195
    RHS1      NodeBalance0108      -51.5426
    RHS1      NodeBalance0109      -55.4373
    RHS1      NodeBalance0203      -58.1356
    RHS1      NodeBalance0205       29.9031
    RHS1      NodeBalance0307       32.4195
    RHS1      NodeBalance0403       58.1356
    RHS1      NodeBalance0404       46.7538
    RHS1      NodeBalance0406      -40.6571
    RHS1      NodeBalance0601      -61.4292
    RHS1      NodeBalance0602       32.8622
    RHS1      NodeBalance0609       55.4373
    RHS1      NodeBalance0704      -46.7538
    RHS1      NodeBalance0801       61.4292
    RHS1      NodeBalance0805      -29.9031
    RHS1      NodeBalance0808       51.5426
    RHS1      NodeBalance0810      -72.1489
    RHS1      NodeBalance0910       72.1489
    RHS1      NodeBalance1002      -32.8622
    RHS1      NodeBalance1006       40.6571
 SC Scen62    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -11.2865
    RHS1      NodeBalance0108       -11.587
    RHS1      NodeBalance0109      -21.3761
    RHS1      NodeBalance0203       -81.045
    RHS1      NodeBalance0205       56.4653
    RHS1      NodeBalance0307       11.2865
    RHS1      NodeBalance0403        81.045
    RHS1      NodeBalance0404        24.332
    RHS1      NodeBalance0406      -46.8812
    RHS1      NodeBalance0601      -32.4395
    RHS1      NodeBalance0602       35.2991
    RHS1      NodeBalance0609       21.3761
    RHS1      NodeBalance0704       -24.332
    RHS1      NodeBalance0801       32.4395
    RHS1      NodeBalance0805      -56.4653
    RHS1      NodeBalance0808        11.587
    RHS1      NodeBalance0810      -82.1144
    RHS1      NodeBalance0910       82.1144
    RHS1      NodeBalance1002      -35.2991
    RHS1      NodeBalance1006       46.8812
 SC Scen63    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -47.6404
    RHS1      NodeBalance0108      -60.6779
    RHS1      NodeBalance0109      -45.4726
    RHS1      NodeBalance0203      -82.2565
    RHS1      NodeBalance0205       47.7045
    RHS1      NodeBalance0307       47.6404
    RHS1      NodeBalance0403       82.2565
    RHS1      NodeBalance0404       10.2469
    RHS1      NodeBalance0406      -66.7963
    RHS1      NodeBalance0601      -77.3007
    RHS1      NodeBalance0602       19.8263
    RHS1      NodeBalance0609       45.4726
    RHS1      NodeBalance0704      -10.2469
    RHS1      NodeBalance0801       77.3007
    RHS1      NodeBalance0805      -47.7045
    RHS1      NodeBalance0808       60.6779
    RHS1      NodeBalance0810      -15.7758
    RHS1      NodeBalance0910       15.7758
    RHS1      NodeBalance1002      -19.8263
    RHS1      NodeBalance1006       66.7963
 SC Scen64    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -42.7841
    RHS1      NodeBalance0108      -33.0627
    RHS1      NodeBalance0109      -43.1764
    RHS1      NodeBalance0203       -76.924
    RHS1      NodeBalance0205       33.9555
    RHS1      NodeBalance0307       42.7841
    RHS1      NodeBalance0403        76.924
    RHS1      NodeBalance0404       25.7928
    RHS1      NodeBalance0406      -29.4029
    RHS1      NodeBalance0601      -51.9656
    RHS1      NodeBalance0602       15.3619
    RHS1      NodeBalance0609       43.1764
    RHS1      NodeBalance0704      -25.7928
    RHS1      NodeBalance0801       51.9656
    RHS1      NodeBalance0805      -33.9555
    RHS1      NodeBalance0808       33.0627
    RHS1      NodeBalance0810      -31.4767
    RHS1      NodeBalance0910       31.4767
    RHS1      NodeBalance1002      -15.3619
    RHS1      NodeBalance1006       29.4029
ENDATA



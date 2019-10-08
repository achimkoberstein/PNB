*   
*   r01_5_0-8_64_cr.mps
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
    y0102     Total_Cost        2975.0   Capacity0102        -613.0
    y0107     Total_Cost        1845.0   Capacity0107        -112.0
    y0108     Total_Cost        1874.0   Capacity0108         -76.0
    y0110     Total_Cost        1220.0   Capacity0110         -41.0
    y0203     Total_Cost        2975.0   Capacity0203        -613.0
    y0204     Total_Cost        2202.0   Capacity0204         -99.0
    y0207     Total_Cost        1696.0   Capacity0207         -58.0
    y0208     Total_Cost        1815.0   Capacity0208         -97.0
    y0209     Total_Cost         893.0   Capacity0209        -108.0
    y0301     Total_Cost         714.0   Capacity0301         -65.0
    y0302     Total_Cost        2767.0   Capacity0302        -133.0
    y0304     Total_Cost        2975.0   Capacity0304        -613.0
    y0307     Total_Cost        1190.0   Capacity0307         -72.0
    y0310     Total_Cost        2588.0   Capacity0310         -65.0
    y0403     Total_Cost         536.0   Capacity0403        -116.0
    y0405     Total_Cost        2975.0   Capacity0405        -613.0
    y0410     Total_Cost        1607.0   Capacity0410         -90.0
    y0502     Total_Cost         625.0   Capacity0502         -37.0
    y0503     Total_Cost        2618.0   Capacity0503        -138.0
    y0506     Total_Cost        2975.0   Capacity0506        -613.0
    y0508     Total_Cost        2678.0   Capacity0508        -116.0
    y0607     Total_Cost        2975.0   Capacity0607        -613.0
    y0608     Total_Cost        1964.0   Capacity0608         -87.0
    y0610     Total_Cost        1517.0   Capacity0610         -51.0
    y0701     Total_Cost        1428.0   Capacity0701         -99.0
    y0708     Total_Cost        2975.0   Capacity0708        -613.0
    y0709     Total_Cost        2856.0   Capacity0709        -111.0
    y0802     Total_Cost        1280.0   Capacity0802        -101.0
    y0803     Total_Cost        1696.0   Capacity0803         -67.0
    y0807     Total_Cost        1785.0   Capacity0807         -67.0
    y0809     Total_Cost        2975.0   Capacity0809        -613.0
    y0906     Total_Cost        1488.0   Capacity0906        -120.0
    y0910     Total_Cost        2975.0   Capacity0910        -613.0
    y1001     Total_Cost        2975.0   Capacity1001        -613.0
    y1006     Total_Cost        1874.0   Capacity1006         -78.0
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
    RHS1      NodeBalance0107      -59.7072   NodeBalance0108      -69.3135
    RHS1      NodeBalance0109      -29.2639   NodeBalance0203      -71.7421
    RHS1      NodeBalance0205       15.5103   NodeBalance0307       59.7072
    RHS1      NodeBalance0403       71.7421   NodeBalance0404       49.0308
    RHS1      NodeBalance0406      -62.6441   NodeBalance0601      -25.4547
    RHS1      NodeBalance0602       36.6536   NodeBalance0609       29.2639
    RHS1      NodeBalance0704      -49.0308   NodeBalance0801       25.4547
    RHS1      NodeBalance0805      -15.5103   NodeBalance0808       69.3135
    RHS1      NodeBalance0810      -78.0837   NodeBalance0910       78.0837
    RHS1      NodeBalance1002      -36.6536   NodeBalance1006       62.6441
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
    RHS1      NodeBalance0107      -59.7072
    RHS1      NodeBalance0108      -69.3135
    RHS1      NodeBalance0109      -29.2639
    RHS1      NodeBalance0203      -71.7421
    RHS1      NodeBalance0205       15.5103
    RHS1      NodeBalance0307       59.7072
    RHS1      NodeBalance0403       71.7421
    RHS1      NodeBalance0404       49.0308
    RHS1      NodeBalance0406      -62.6441
    RHS1      NodeBalance0601      -25.4547
    RHS1      NodeBalance0602       36.6536
    RHS1      NodeBalance0609       29.2639
    RHS1      NodeBalance0704      -49.0308
    RHS1      NodeBalance0801       25.4547
    RHS1      NodeBalance0805      -15.5103
    RHS1      NodeBalance0808       69.3135
    RHS1      NodeBalance0810      -78.0837
    RHS1      NodeBalance0910       78.0837
    RHS1      NodeBalance1002      -36.6536
    RHS1      NodeBalance1006       62.6441
 SC Scen2     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -19.8499
    RHS1      NodeBalance0108      -31.4049
    RHS1      NodeBalance0109      -9.68914
    RHS1      NodeBalance0203       -45.726
    RHS1      NodeBalance0205       48.8555
    RHS1      NodeBalance0307       19.8499
    RHS1      NodeBalance0403        45.726
    RHS1      NodeBalance0404       25.3557
    RHS1      NodeBalance0406      -68.4913
    RHS1      NodeBalance0601      -12.1453
    RHS1      NodeBalance0602       31.8888
    RHS1      NodeBalance0609       9.68914
    RHS1      NodeBalance0704      -25.3557
    RHS1      NodeBalance0801       12.1453
    RHS1      NodeBalance0805      -48.8555
    RHS1      NodeBalance0808       31.4049
    RHS1      NodeBalance0810      -28.2045
    RHS1      NodeBalance0910       28.2045
    RHS1      NodeBalance1002      -31.8888
    RHS1      NodeBalance1006       68.4913
 SC Scen3     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -45.2571
    RHS1      NodeBalance0108      -66.4583
    RHS1      NodeBalance0109       -39.781
    RHS1      NodeBalance0203      -62.6176
    RHS1      NodeBalance0205       61.5472
    RHS1      NodeBalance0307       45.2571
    RHS1      NodeBalance0403       62.6176
    RHS1      NodeBalance0404       45.0384
    RHS1      NodeBalance0406      -56.8125
    RHS1      NodeBalance0601      -52.5585
    RHS1      NodeBalance0602       28.4928
    RHS1      NodeBalance0609        39.781
    RHS1      NodeBalance0704      -45.0384
    RHS1      NodeBalance0801       52.5585
    RHS1      NodeBalance0805      -61.5472
    RHS1      NodeBalance0808       66.4583
    RHS1      NodeBalance0810      -63.8392
    RHS1      NodeBalance0910       63.8392
    RHS1      NodeBalance1002      -28.4928
    RHS1      NodeBalance1006       56.8125
 SC Scen4     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -37.4323
    RHS1      NodeBalance0108       -33.487
    RHS1      NodeBalance0109      -26.4737
    RHS1      NodeBalance0203      -52.4672
    RHS1      NodeBalance0205       61.0352
    RHS1      NodeBalance0307       37.4323
    RHS1      NodeBalance0403       52.4672
    RHS1      NodeBalance0404       41.3377
    RHS1      NodeBalance0406      -50.1418
    RHS1      NodeBalance0601      -24.1721
    RHS1      NodeBalance0602       27.2787
    RHS1      NodeBalance0609       26.4737
    RHS1      NodeBalance0704      -41.3377
    RHS1      NodeBalance0801       24.1721
    RHS1      NodeBalance0805      -61.0352
    RHS1      NodeBalance0808        33.487
    RHS1      NodeBalance0810      -2.92277
    RHS1      NodeBalance0910       2.92277
    RHS1      NodeBalance1002      -27.2787
    RHS1      NodeBalance1006       50.1418
 SC Scen5     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -5.00672
    RHS1      NodeBalance0108      -5.30667
    RHS1      NodeBalance0109      -59.8693
    RHS1      NodeBalance0203      -33.2355
    RHS1      NodeBalance0205        33.495
    RHS1      NodeBalance0307       5.00672
    RHS1      NodeBalance0403       33.2355
    RHS1      NodeBalance0404       31.5944
    RHS1      NodeBalance0406      -76.1695
    RHS1      NodeBalance0601      -78.4273
    RHS1      NodeBalance0602       25.8352
    RHS1      NodeBalance0609       59.8693
    RHS1      NodeBalance0704      -31.5944
    RHS1      NodeBalance0801       78.4273
    RHS1      NodeBalance0805       -33.495
    RHS1      NodeBalance0808       5.30667
    RHS1      NodeBalance0810      -19.3397
    RHS1      NodeBalance0910       19.3397
    RHS1      NodeBalance1002      -25.8352
    RHS1      NodeBalance1006       76.1695
 SC Scen6     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -67.6516
    RHS1      NodeBalance0108       -86.496
    RHS1      NodeBalance0109      -65.8462
    RHS1      NodeBalance0203      -81.1487
    RHS1      NodeBalance0205       38.9622
    RHS1      NodeBalance0307       67.6516
    RHS1      NodeBalance0403       81.1487
    RHS1      NodeBalance0404       33.3461
    RHS1      NodeBalance0406      -68.4547
    RHS1      NodeBalance0601      -83.2925
    RHS1      NodeBalance0602       24.0051
    RHS1      NodeBalance0609       65.8462
    RHS1      NodeBalance0704      -33.3461
    RHS1      NodeBalance0801       83.2925
    RHS1      NodeBalance0805      -38.9622
    RHS1      NodeBalance0808        86.496
    RHS1      NodeBalance0810      -81.9233
    RHS1      NodeBalance0910       81.9233
    RHS1      NodeBalance1002      -24.0051
    RHS1      NodeBalance1006       68.4547
 SC Scen7     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -61.2072
    RHS1      NodeBalance0108      -77.8401
    RHS1      NodeBalance0109      -31.9328
    RHS1      NodeBalance0203      -29.0446
    RHS1      NodeBalance0205       49.0636
    RHS1      NodeBalance0307       61.2072
    RHS1      NodeBalance0403       29.0446
    RHS1      NodeBalance0404       24.0614
    RHS1      NodeBalance0406      -49.9856
    RHS1      NodeBalance0601      -53.9755
    RHS1      NodeBalance0602       34.6709
    RHS1      NodeBalance0609       31.9328
    RHS1      NodeBalance0704      -24.0614
    RHS1      NodeBalance0801       53.9755
    RHS1      NodeBalance0805      -49.0636
    RHS1      NodeBalance0808       77.8401
    RHS1      NodeBalance0810      -81.2285
    RHS1      NodeBalance0910       81.2285
    RHS1      NodeBalance1002      -34.6709
    RHS1      NodeBalance1006       49.9856
 SC Scen8     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -37.7448
    RHS1      NodeBalance0108      -27.5958
    RHS1      NodeBalance0109      -61.4343
    RHS1      NodeBalance0203         -55.8
    RHS1      NodeBalance0205       55.6323
    RHS1      NodeBalance0307       37.7448
    RHS1      NodeBalance0403          55.8
    RHS1      NodeBalance0404       22.4397
    RHS1      NodeBalance0406      -22.2134
    RHS1      NodeBalance0601      -82.8019
    RHS1      NodeBalance0602       25.7481
    RHS1      NodeBalance0609       61.4343
    RHS1      NodeBalance0704      -22.4397
    RHS1      NodeBalance0801       82.8019
    RHS1      NodeBalance0805      -55.6323
    RHS1      NodeBalance0808       27.5958
    RHS1      NodeBalance0810      -16.3635
    RHS1      NodeBalance0910       16.3635
    RHS1      NodeBalance1002      -25.7481
    RHS1      NodeBalance1006       22.2134
 SC Scen9     Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -46.5102
    RHS1      NodeBalance0108      -69.6316
    RHS1      NodeBalance0109      -50.4057
    RHS1      NodeBalance0203      -57.4739
    RHS1      NodeBalance0205       51.3241
    RHS1      NodeBalance0307       46.5102
    RHS1      NodeBalance0403       57.4739
    RHS1      NodeBalance0404       48.4896
    RHS1      NodeBalance0406      -50.4573
    RHS1      NodeBalance0601      -66.5227
    RHS1      NodeBalance0602        35.091
    RHS1      NodeBalance0609       50.4057
    RHS1      NodeBalance0704      -48.4896
    RHS1      NodeBalance0801       66.5227
    RHS1      NodeBalance0805      -51.3241
    RHS1      NodeBalance0808       69.6316
    RHS1      NodeBalance0810      -41.4301
    RHS1      NodeBalance0910       41.4301
    RHS1      NodeBalance1002       -35.091
    RHS1      NodeBalance1006       50.4573
 SC Scen10    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -19.1551
    RHS1      NodeBalance0108      -49.6801
    RHS1      NodeBalance0109      -49.7177
    RHS1      NodeBalance0203      -72.6296
    RHS1      NodeBalance0205       48.4989
    RHS1      NodeBalance0307       19.1551
    RHS1      NodeBalance0403       72.6296
    RHS1      NodeBalance0404       24.4092
    RHS1      NodeBalance0406       -94.567
    RHS1      NodeBalance0601      -67.7838
    RHS1      NodeBalance0602        27.085
    RHS1      NodeBalance0609       49.7177
    RHS1      NodeBalance0704      -24.4092
    RHS1      NodeBalance0801       67.7838
    RHS1      NodeBalance0805      -48.4989
    RHS1      NodeBalance0808       49.6801
    RHS1      NodeBalance0810      -60.3668
    RHS1      NodeBalance0910       60.3668
    RHS1      NodeBalance1002       -27.085
    RHS1      NodeBalance1006        94.567
 SC Scen11    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -52.2658
    RHS1      NodeBalance0108       -51.442
    RHS1      NodeBalance0109      -23.5636
    RHS1      NodeBalance0203       -66.444
    RHS1      NodeBalance0205       46.5881
    RHS1      NodeBalance0307       52.2658
    RHS1      NodeBalance0403        66.444
    RHS1      NodeBalance0404       23.6111
    RHS1      NodeBalance0406      -90.0524
    RHS1      NodeBalance0601      -51.4005
    RHS1      NodeBalance0602       18.3795
    RHS1      NodeBalance0609       23.5636
    RHS1      NodeBalance0704      -23.6111
    RHS1      NodeBalance0801       51.4005
    RHS1      NodeBalance0805      -46.5881
    RHS1      NodeBalance0808        51.442
    RHS1      NodeBalance0810      -29.0047
    RHS1      NodeBalance0910       29.0047
    RHS1      NodeBalance1002      -18.3795
    RHS1      NodeBalance1006       90.0524
 SC Scen12    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -63.1003
    RHS1      NodeBalance0108      -84.1793
    RHS1      NodeBalance0109      -64.5191
    RHS1      NodeBalance0203      -20.1619
    RHS1      NodeBalance0205       62.4519
    RHS1      NodeBalance0307       63.1003
    RHS1      NodeBalance0403       20.1619
    RHS1      NodeBalance0404        25.882
    RHS1      NodeBalance0406      -64.9768
    RHS1      NodeBalance0601      -85.1256
    RHS1      NodeBalance0602       23.1415
    RHS1      NodeBalance0609       64.5191
    RHS1      NodeBalance0704       -25.882
    RHS1      NodeBalance0801       85.1256
    RHS1      NodeBalance0805      -62.4519
    RHS1      NodeBalance0808       84.1793
    RHS1      NodeBalance0810      -55.9503
    RHS1      NodeBalance0910       55.9503
    RHS1      NodeBalance1002      -23.1415
    RHS1      NodeBalance1006       64.9768
 SC Scen13    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -55.4787
    RHS1      NodeBalance0108      -61.0533
    RHS1      NodeBalance0109       -47.703
    RHS1      NodeBalance0203      -75.5637
    RHS1      NodeBalance0205       38.5139
    RHS1      NodeBalance0307       55.4787
    RHS1      NodeBalance0403       75.5637
    RHS1      NodeBalance0404       22.8166
    RHS1      NodeBalance0406      -32.7095
    RHS1      NodeBalance0601      -44.5383
    RHS1      NodeBalance0602       22.3478
    RHS1      NodeBalance0609        47.703
    RHS1      NodeBalance0704      -22.8166
    RHS1      NodeBalance0801       44.5383
    RHS1      NodeBalance0805      -38.5139
    RHS1      NodeBalance0808       61.0533
    RHS1      NodeBalance0810      -73.4407
    RHS1      NodeBalance0910       73.4407
    RHS1      NodeBalance1002      -22.3478
    RHS1      NodeBalance1006       32.7095
 SC Scen14    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -17.3443
    RHS1      NodeBalance0108      -26.6803
    RHS1      NodeBalance0109      -60.2898
    RHS1      NodeBalance0203      -67.5831
    RHS1      NodeBalance0205       38.5237
    RHS1      NodeBalance0307       17.3443
    RHS1      NodeBalance0403       67.5831
    RHS1      NodeBalance0404       42.5587
    RHS1      NodeBalance0406      -47.8411
    RHS1      NodeBalance0601      -85.9788
    RHS1      NodeBalance0602        39.363
    RHS1      NodeBalance0609       60.2898
    RHS1      NodeBalance0704      -42.5587
    RHS1      NodeBalance0801       85.9788
    RHS1      NodeBalance0805      -38.5237
    RHS1      NodeBalance0808       26.6803
    RHS1      NodeBalance0810      -61.8775
    RHS1      NodeBalance0910       61.8775
    RHS1      NodeBalance1002       -39.363
    RHS1      NodeBalance1006       47.8411
 SC Scen15    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -31.8433
    RHS1      NodeBalance0108      -28.9756
    RHS1      NodeBalance0109      -58.7729
    RHS1      NodeBalance0203      -55.9943
    RHS1      NodeBalance0205       50.7487
    RHS1      NodeBalance0307       31.8433
    RHS1      NodeBalance0403       55.9943
    RHS1      NodeBalance0404       4.39962
    RHS1      NodeBalance0406      -77.8023
    RHS1      NodeBalance0601      -71.6361
    RHS1      NodeBalance0602       4.89294
    RHS1      NodeBalance0609       58.7729
    RHS1      NodeBalance0704      -4.39962
    RHS1      NodeBalance0801       71.6361
    RHS1      NodeBalance0805      -50.7487
    RHS1      NodeBalance0808       28.9756
    RHS1      NodeBalance0810      -67.4416
    RHS1      NodeBalance0910       67.4416
    RHS1      NodeBalance1002      -4.89294
    RHS1      NodeBalance1006       77.8023
 SC Scen16    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -31.4773
    RHS1      NodeBalance0108      -53.6817
    RHS1      NodeBalance0109      -52.7661
    RHS1      NodeBalance0203      -43.2325
    RHS1      NodeBalance0205       64.4714
    RHS1      NodeBalance0307       31.4773
    RHS1      NodeBalance0403       43.2325
    RHS1      NodeBalance0404       46.4967
    RHS1      NodeBalance0406      -19.8827
    RHS1      NodeBalance0601      -76.9036
    RHS1      NodeBalance0602       30.8573
    RHS1      NodeBalance0609       52.7661
    RHS1      NodeBalance0704      -46.4967
    RHS1      NodeBalance0801       76.9036
    RHS1      NodeBalance0805      -64.4714
    RHS1      NodeBalance0808       53.6817
    RHS1      NodeBalance0810      -71.8512
    RHS1      NodeBalance0910       71.8512
    RHS1      NodeBalance1002      -30.8573
    RHS1      NodeBalance1006       19.8827
 SC Scen17    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107        -57.34
    RHS1      NodeBalance0108      -68.4511
    RHS1      NodeBalance0109      -16.1637
    RHS1      NodeBalance0203      -35.5581
    RHS1      NodeBalance0205       30.5342
    RHS1      NodeBalance0307         57.34
    RHS1      NodeBalance0403       35.5581
    RHS1      NodeBalance0404       41.9002
    RHS1      NodeBalance0406      -81.0766
    RHS1      NodeBalance0601      -41.4095
    RHS1      NodeBalance0602       30.1909
    RHS1      NodeBalance0609       16.1637
    RHS1      NodeBalance0704      -41.9002
    RHS1      NodeBalance0801       41.4095
    RHS1      NodeBalance0805      -30.5342
    RHS1      NodeBalance0808       68.4511
    RHS1      NodeBalance0810      -76.5685
    RHS1      NodeBalance0910       76.5685
    RHS1      NodeBalance1002      -30.1909
    RHS1      NodeBalance1006       81.0766
 SC Scen18    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -52.0871
    RHS1      NodeBalance0108        -76.59
    RHS1      NodeBalance0109      -30.2586
    RHS1      NodeBalance0203      -62.0871
    RHS1      NodeBalance0205        13.153
    RHS1      NodeBalance0307       52.0871
    RHS1      NodeBalance0403       62.0871
    RHS1      NodeBalance0404       34.0428
    RHS1      NodeBalance0406      -76.8565
    RHS1      NodeBalance0601      -48.0863
    RHS1      NodeBalance0602       34.0993
    RHS1      NodeBalance0609       30.2586
    RHS1      NodeBalance0704      -34.0428
    RHS1      NodeBalance0801       48.0863
    RHS1      NodeBalance0805       -13.153
    RHS1      NodeBalance0808         76.59
    RHS1      NodeBalance0810      -39.4891
    RHS1      NodeBalance0910       39.4891
    RHS1      NodeBalance1002      -34.0993
    RHS1      NodeBalance1006       76.8565
 SC Scen19    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -56.2157
    RHS1      NodeBalance0108      -80.4508
    RHS1      NodeBalance0109      -43.5812
    RHS1      NodeBalance0203      -36.6116
    RHS1      NodeBalance0205       53.2368
    RHS1      NodeBalance0307       56.2157
    RHS1      NodeBalance0403       36.6116
    RHS1      NodeBalance0404       29.7559
    RHS1      NodeBalance0406      -49.5975
    RHS1      NodeBalance0601      -69.8161
    RHS1      NodeBalance0602       26.6613
    RHS1      NodeBalance0609       43.5812
    RHS1      NodeBalance0704      -29.7559
    RHS1      NodeBalance0801       69.8161
    RHS1      NodeBalance0805      -53.2368
    RHS1      NodeBalance0808       80.4508
    RHS1      NodeBalance0810      -22.4352
    RHS1      NodeBalance0910       22.4352
    RHS1      NodeBalance1002      -26.6613
    RHS1      NodeBalance1006       49.5975
 SC Scen20    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -65.5092
    RHS1      NodeBalance0108      -78.5073
    RHS1      NodeBalance0109      -45.0041
    RHS1      NodeBalance0203        -80.22
    RHS1      NodeBalance0205       21.4187
    RHS1      NodeBalance0307       65.5092
    RHS1      NodeBalance0403         80.22
    RHS1      NodeBalance0404       31.1938
    RHS1      NodeBalance0406      -93.0745
    RHS1      NodeBalance0601      -33.3525
    RHS1      NodeBalance0602       22.4621
    RHS1      NodeBalance0609       45.0041
    RHS1      NodeBalance0704      -31.1938
    RHS1      NodeBalance0801       33.3525
    RHS1      NodeBalance0805      -21.4187
    RHS1      NodeBalance0808       78.5073
    RHS1      NodeBalance0810      -60.8572
    RHS1      NodeBalance0910       60.8572
    RHS1      NodeBalance1002      -22.4621
    RHS1      NodeBalance1006       93.0745
 SC Scen21    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.4818
    RHS1      NodeBalance0108      -58.6126
    RHS1      NodeBalance0109      -52.1423
    RHS1      NodeBalance0203       -42.968
    RHS1      NodeBalance0205       37.5107
    RHS1      NodeBalance0307       49.4818
    RHS1      NodeBalance0403        42.968
    RHS1      NodeBalance0404       41.3398
    RHS1      NodeBalance0406       -41.252
    RHS1      NodeBalance0601      -67.0268
    RHS1      NodeBalance0602       36.3284
    RHS1      NodeBalance0609       52.1423
    RHS1      NodeBalance0704      -41.3398
    RHS1      NodeBalance0801       67.0268
    RHS1      NodeBalance0805      -37.5107
    RHS1      NodeBalance0808       58.6126
    RHS1      NodeBalance0810      -70.2382
    RHS1      NodeBalance0910       70.2382
    RHS1      NodeBalance1002      -36.3284
    RHS1      NodeBalance1006        41.252
 SC Scen22    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.8829
    RHS1      NodeBalance0108         -56.3
    RHS1      NodeBalance0109      -59.6699
    RHS1      NodeBalance0203      -19.5827
    RHS1      NodeBalance0205       44.4372
    RHS1      NodeBalance0307       49.8829
    RHS1      NodeBalance0403       19.5827
    RHS1      NodeBalance0404       37.2358
    RHS1      NodeBalance0406      -91.5116
    RHS1      NodeBalance0601      -62.9848
    RHS1      NodeBalance0602       27.6774
    RHS1      NodeBalance0609       59.6699
    RHS1      NodeBalance0704      -37.2358
    RHS1      NodeBalance0801       62.9848
    RHS1      NodeBalance0805      -44.4372
    RHS1      NodeBalance0808          56.3
    RHS1      NodeBalance0810      -70.4513
    RHS1      NodeBalance0910       70.4513
    RHS1      NodeBalance1002      -27.6774
    RHS1      NodeBalance1006       91.5116
 SC Scen23    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -31.124
    RHS1      NodeBalance0108      -46.8747
    RHS1      NodeBalance0109      -38.3134
    RHS1      NodeBalance0203      -64.9085
    RHS1      NodeBalance0205         54.84
    RHS1      NodeBalance0307        31.124
    RHS1      NodeBalance0403       64.9085
    RHS1      NodeBalance0404       43.3471
    RHS1      NodeBalance0406      -52.0343
    RHS1      NodeBalance0601      -50.8127
    RHS1      NodeBalance0602       32.0632
    RHS1      NodeBalance0609       38.3134
    RHS1      NodeBalance0704      -43.3471
    RHS1      NodeBalance0801       50.8127
    RHS1      NodeBalance0805        -54.84
    RHS1      NodeBalance0808       46.8747
    RHS1      NodeBalance0810      -61.3314
    RHS1      NodeBalance0910       61.3314
    RHS1      NodeBalance1002      -32.0632
    RHS1      NodeBalance1006       52.0343
 SC Scen24    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -46.0639
    RHS1      NodeBalance0108      -66.8942
    RHS1      NodeBalance0109      -36.5688
    RHS1      NodeBalance0203      -82.2847
    RHS1      NodeBalance0205       15.9918
    RHS1      NodeBalance0307       46.0639
    RHS1      NodeBalance0403       82.2847
    RHS1      NodeBalance0404       27.7992
    RHS1      NodeBalance0406      -66.2525
    RHS1      NodeBalance0601      -62.7303
    RHS1      NodeBalance0602       21.3912
    RHS1      NodeBalance0609       36.5688
    RHS1      NodeBalance0704      -27.7992
    RHS1      NodeBalance0801       62.7303
    RHS1      NodeBalance0805      -15.9918
    RHS1      NodeBalance0808       66.8942
    RHS1      NodeBalance0810      -38.7417
    RHS1      NodeBalance0910       38.7417
    RHS1      NodeBalance1002      -21.3912
    RHS1      NodeBalance1006       66.2525
 SC Scen25    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -57.8369
    RHS1      NodeBalance0108      -38.6704
    RHS1      NodeBalance0109      -26.1997
    RHS1      NodeBalance0203      -25.4731
    RHS1      NodeBalance0205       41.9204
    RHS1      NodeBalance0307       57.8369
    RHS1      NodeBalance0403       25.4731
    RHS1      NodeBalance0404        33.904
    RHS1      NodeBalance0406      -74.2801
    RHS1      NodeBalance0601      -34.2518
    RHS1      NodeBalance0602       27.4236
    RHS1      NodeBalance0609       26.1997
    RHS1      NodeBalance0704       -33.904
    RHS1      NodeBalance0801       34.2518
    RHS1      NodeBalance0805      -41.9204
    RHS1      NodeBalance0808       38.6704
    RHS1      NodeBalance0810      -31.2709
    RHS1      NodeBalance0910       31.2709
    RHS1      NodeBalance1002      -27.4236
    RHS1      NodeBalance1006       74.2801
 SC Scen26    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -35.8486
    RHS1      NodeBalance0108      -22.6798
    RHS1      NodeBalance0109      -48.6595
    RHS1      NodeBalance0203       -82.932
    RHS1      NodeBalance0205       8.74341
    RHS1      NodeBalance0307       35.8486
    RHS1      NodeBalance0403        82.932
    RHS1      NodeBalance0404       25.1008
    RHS1      NodeBalance0406      -94.0038
    RHS1      NodeBalance0601      -70.0725
    RHS1      NodeBalance0602       18.6663
    RHS1      NodeBalance0609       48.6595
    RHS1      NodeBalance0704      -25.1008
    RHS1      NodeBalance0801       70.0725
    RHS1      NodeBalance0805      -8.74341
    RHS1      NodeBalance0808       22.6798
    RHS1      NodeBalance0810        -46.52
    RHS1      NodeBalance0910         46.52
    RHS1      NodeBalance1002      -18.6663
    RHS1      NodeBalance1006       94.0038
 SC Scen27    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.3472
    RHS1      NodeBalance0108      -42.4644
    RHS1      NodeBalance0109      -62.2681
    RHS1      NodeBalance0203       -77.647
    RHS1      NodeBalance0205       56.9603
    RHS1      NodeBalance0307       49.3472
    RHS1      NodeBalance0403        77.647
    RHS1      NodeBalance0404       13.2591
    RHS1      NodeBalance0406      -40.2342
    RHS1      NodeBalance0601      -74.2267
    RHS1      NodeBalance0602       14.7471
    RHS1      NodeBalance0609       62.2681
    RHS1      NodeBalance0704      -13.2591
    RHS1      NodeBalance0801       74.2267
    RHS1      NodeBalance0805      -56.9603
    RHS1      NodeBalance0808       42.4644
    RHS1      NodeBalance0810      -11.1947
    RHS1      NodeBalance0910       11.1947
    RHS1      NodeBalance1002      -14.7471
    RHS1      NodeBalance1006       40.2342
 SC Scen28    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -28.3883
    RHS1      NodeBalance0108      -35.8368
    RHS1      NodeBalance0109      -11.7544
    RHS1      NodeBalance0203      -78.5015
    RHS1      NodeBalance0205       51.6994
    RHS1      NodeBalance0307       28.3883
    RHS1      NodeBalance0403       78.5015
    RHS1      NodeBalance0404       3.19236
    RHS1      NodeBalance0406      -44.0087
    RHS1      NodeBalance0601      -48.3963
    RHS1      NodeBalance0602       3.81637
    RHS1      NodeBalance0609       11.7544
    RHS1      NodeBalance0704      -3.19236
    RHS1      NodeBalance0801       48.3963
    RHS1      NodeBalance0805      -51.6994
    RHS1      NodeBalance0808       35.8368
    RHS1      NodeBalance0810      -35.7752
    RHS1      NodeBalance0910       35.7752
    RHS1      NodeBalance1002      -3.81637
    RHS1      NodeBalance1006       44.0087
 SC Scen29    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -64.5548
    RHS1      NodeBalance0108      -55.9389
    RHS1      NodeBalance0109       -58.333
    RHS1      NodeBalance0203      -89.2392
    RHS1      NodeBalance0205       51.3868
    RHS1      NodeBalance0307       64.5548
    RHS1      NodeBalance0403       89.2392
    RHS1      NodeBalance0404        33.743
    RHS1      NodeBalance0406      -67.5538
    RHS1      NodeBalance0601      -79.6697
    RHS1      NodeBalance0602       17.4375
    RHS1      NodeBalance0609        58.333
    RHS1      NodeBalance0704       -33.743
    RHS1      NodeBalance0801       79.6697
    RHS1      NodeBalance0805      -51.3868
    RHS1      NodeBalance0808       55.9389
    RHS1      NodeBalance0810      -68.4419
    RHS1      NodeBalance0910       68.4419
    RHS1      NodeBalance1002      -17.4375
    RHS1      NodeBalance1006       67.5538
 SC Scen30    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -42.6635
    RHS1      NodeBalance0108       -52.738
    RHS1      NodeBalance0109      -54.7952
    RHS1      NodeBalance0203      -29.4843
    RHS1      NodeBalance0205       32.2929
    RHS1      NodeBalance0307       42.6635
    RHS1      NodeBalance0403       29.4843
    RHS1      NodeBalance0404       35.1659
    RHS1      NodeBalance0406      -67.4992
    RHS1      NodeBalance0601      -73.0303
    RHS1      NodeBalance0602       30.6226
    RHS1      NodeBalance0609       54.7952
    RHS1      NodeBalance0704      -35.1659
    RHS1      NodeBalance0801       73.0303
    RHS1      NodeBalance0805      -32.2929
    RHS1      NodeBalance0808        52.738
    RHS1      NodeBalance0810      -46.0702
    RHS1      NodeBalance0910       46.0702
    RHS1      NodeBalance1002      -30.6226
    RHS1      NodeBalance1006       67.4992
 SC Scen31    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -56.094
    RHS1      NodeBalance0108      -48.5968
    RHS1      NodeBalance0109      -61.1728
    RHS1      NodeBalance0203      -31.9497
    RHS1      NodeBalance0205        38.132
    RHS1      NodeBalance0307        56.094
    RHS1      NodeBalance0403       31.9497
    RHS1      NodeBalance0404       30.1192
    RHS1      NodeBalance0406      -42.9735
    RHS1      NodeBalance0601      -75.5113
    RHS1      NodeBalance0602       22.2987
    RHS1      NodeBalance0609       61.1728
    RHS1      NodeBalance0704      -30.1192
    RHS1      NodeBalance0801       75.5113
    RHS1      NodeBalance0805       -38.132
    RHS1      NodeBalance0808       48.5968
    RHS1      NodeBalance0810      -79.3404
    RHS1      NodeBalance0910       79.3404
    RHS1      NodeBalance1002      -22.2987
    RHS1      NodeBalance1006       42.9735
 SC Scen32    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -34.9865
    RHS1      NodeBalance0108      -65.6159
    RHS1      NodeBalance0109       -6.9121
    RHS1      NodeBalance0203      -79.3451
    RHS1      NodeBalance0205       39.5489
    RHS1      NodeBalance0307       34.9865
    RHS1      NodeBalance0403       79.3451
    RHS1      NodeBalance0404       19.0819
    RHS1      NodeBalance0406      -19.7619
    RHS1      NodeBalance0601      -21.3301
    RHS1      NodeBalance0602       10.9769
    RHS1      NodeBalance0609        6.9121
    RHS1      NodeBalance0704      -19.0819
    RHS1      NodeBalance0801       21.3301
    RHS1      NodeBalance0805      -39.5489
    RHS1      NodeBalance0808       65.6159
    RHS1      NodeBalance0810      -72.9263
    RHS1      NodeBalance0910       72.9263
    RHS1      NodeBalance1002      -10.9769
    RHS1      NodeBalance1006       19.7619
 SC Scen33    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -56.6754
    RHS1      NodeBalance0108      -71.0435
    RHS1      NodeBalance0109      -24.5913
    RHS1      NodeBalance0203        -15.39
    RHS1      NodeBalance0205       59.6578
    RHS1      NodeBalance0307       56.6754
    RHS1      NodeBalance0403         15.39
    RHS1      NodeBalance0404       45.3699
    RHS1      NodeBalance0406      -44.1889
    RHS1      NodeBalance0601      -28.9131
    RHS1      NodeBalance0602       32.1193
    RHS1      NodeBalance0609       24.5913
    RHS1      NodeBalance0704      -45.3699
    RHS1      NodeBalance0801       28.9131
    RHS1      NodeBalance0805      -59.6578
    RHS1      NodeBalance0808       71.0435
    RHS1      NodeBalance0810      -31.7504
    RHS1      NodeBalance0910       31.7504
    RHS1      NodeBalance1002      -32.1193
    RHS1      NodeBalance1006       44.1889
 SC Scen34    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -58.413
    RHS1      NodeBalance0108      -48.8462
    RHS1      NodeBalance0109       -58.971
    RHS1      NodeBalance0203      -17.3894
    RHS1      NodeBalance0205       25.6949
    RHS1      NodeBalance0307        58.413
    RHS1      NodeBalance0403       17.3894
    RHS1      NodeBalance0404       21.7745
    RHS1      NodeBalance0406      -30.4758
    RHS1      NodeBalance0601      -68.2895
    RHS1      NodeBalance0602        24.103
    RHS1      NodeBalance0609        58.971
    RHS1      NodeBalance0704      -21.7745
    RHS1      NodeBalance0801       68.2895
    RHS1      NodeBalance0805      -25.6949
    RHS1      NodeBalance0808       48.8462
    RHS1      NodeBalance0810      -58.7424
    RHS1      NodeBalance0910       58.7424
    RHS1      NodeBalance1002       -24.103
    RHS1      NodeBalance1006       30.4758
 SC Scen35    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -53.0736
    RHS1      NodeBalance0108      -73.2767
    RHS1      NodeBalance0109        -25.42
    RHS1      NodeBalance0203      -65.1458
    RHS1      NodeBalance0205       46.6281
    RHS1      NodeBalance0307       53.0736
    RHS1      NodeBalance0403       65.1458
    RHS1      NodeBalance0404       32.7147
    RHS1      NodeBalance0406      -97.0948
    RHS1      NodeBalance0601      -55.0871
    RHS1      NodeBalance0602       35.1866
    RHS1      NodeBalance0609         25.42
    RHS1      NodeBalance0704      -32.7147
    RHS1      NodeBalance0801       55.0871
    RHS1      NodeBalance0805      -46.6281
    RHS1      NodeBalance0808       73.2767
    RHS1      NodeBalance0810      -63.0826
    RHS1      NodeBalance0910       63.0826
    RHS1      NodeBalance1002      -35.1866
    RHS1      NodeBalance1006       97.0948
 SC Scen36    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -40.0557
    RHS1      NodeBalance0108      -57.5197
    RHS1      NodeBalance0109      -43.2943
    RHS1      NodeBalance0203      -83.6439
    RHS1      NodeBalance0205       11.9453
    RHS1      NodeBalance0307       40.0557
    RHS1      NodeBalance0403       83.6439
    RHS1      NodeBalance0404       36.4641
    RHS1      NodeBalance0406      -57.5827
    RHS1      NodeBalance0601      -46.5064
    RHS1      NodeBalance0602       21.0727
    RHS1      NodeBalance0609       43.2943
    RHS1      NodeBalance0704      -36.4641
    RHS1      NodeBalance0801       46.5064
    RHS1      NodeBalance0805      -11.9453
    RHS1      NodeBalance0808       57.5197
    RHS1      NodeBalance0810      -70.0421
    RHS1      NodeBalance0910       70.0421
    RHS1      NodeBalance1002      -21.0727
    RHS1      NodeBalance1006       57.5827
 SC Scen37    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -66.3582
    RHS1      NodeBalance0108      -78.0331
    RHS1      NodeBalance0109      -14.0366
    RHS1      NodeBalance0203      -68.5955
    RHS1      NodeBalance0205       53.8428
    RHS1      NodeBalance0307       66.3582
    RHS1      NodeBalance0403       68.5955
    RHS1      NodeBalance0404       37.1278
    RHS1      NodeBalance0406      -69.4568
    RHS1      NodeBalance0601      -8.62173
    RHS1      NodeBalance0602       29.6146
    RHS1      NodeBalance0609       14.0366
    RHS1      NodeBalance0704      -37.1278
    RHS1      NodeBalance0801       8.62173
    RHS1      NodeBalance0805      -53.8428
    RHS1      NodeBalance0808       78.0331
    RHS1      NodeBalance0810      -68.8766
    RHS1      NodeBalance0910       68.8766
    RHS1      NodeBalance1002      -29.6146
    RHS1      NodeBalance1006       69.4568
 SC Scen38    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -62.6468
    RHS1      NodeBalance0108      -82.3328
    RHS1      NodeBalance0109      -41.7796
    RHS1      NodeBalance0203      -76.7272
    RHS1      NodeBalance0205       58.1158
    RHS1      NodeBalance0307       62.6468
    RHS1      NodeBalance0403       76.7272
    RHS1      NodeBalance0404       14.8785
    RHS1      NodeBalance0406      -47.3245
    RHS1      NodeBalance0601      -50.2187
    RHS1      NodeBalance0602       22.1178
    RHS1      NodeBalance0609       41.7796
    RHS1      NodeBalance0704      -14.8785
    RHS1      NodeBalance0801       50.2187
    RHS1      NodeBalance0805      -58.1158
    RHS1      NodeBalance0808       82.3328
    RHS1      NodeBalance0810      -41.9249
    RHS1      NodeBalance0910       41.9249
    RHS1      NodeBalance1002      -22.1178
    RHS1      NodeBalance1006       47.3245
 SC Scen39    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -44.7013
    RHS1      NodeBalance0108      -51.4962
    RHS1      NodeBalance0109      -46.0938
    RHS1      NodeBalance0203      -38.1448
    RHS1      NodeBalance0205       29.2958
    RHS1      NodeBalance0307       44.7013
    RHS1      NodeBalance0403       38.1448
    RHS1      NodeBalance0404       13.5791
    RHS1      NodeBalance0406      -80.6171
    RHS1      NodeBalance0601      -39.1795
    RHS1      NodeBalance0602       11.4083
    RHS1      NodeBalance0609       46.0938
    RHS1      NodeBalance0704      -13.5791
    RHS1      NodeBalance0801       39.1795
    RHS1      NodeBalance0805      -29.2958
    RHS1      NodeBalance0808       51.4962
    RHS1      NodeBalance0810       -53.999
    RHS1      NodeBalance0910        53.999
    RHS1      NodeBalance1002      -11.4083
    RHS1      NodeBalance1006       80.6171
 SC Scen40    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -22.1927
    RHS1      NodeBalance0108      -60.4477
    RHS1      NodeBalance0109      -68.4696
    RHS1      NodeBalance0203       -67.265
    RHS1      NodeBalance0205       25.9573
    RHS1      NodeBalance0307       22.1927
    RHS1      NodeBalance0403        67.265
    RHS1      NodeBalance0404        42.837
    RHS1      NodeBalance0406      -29.8417
    RHS1      NodeBalance0601      -87.2116
    RHS1      NodeBalance0602       38.4317
    RHS1      NodeBalance0609       68.4696
    RHS1      NodeBalance0704       -42.837
    RHS1      NodeBalance0801       87.2116
    RHS1      NodeBalance0805      -25.9573
    RHS1      NodeBalance0808       60.4477
    RHS1      NodeBalance0810      -45.6836
    RHS1      NodeBalance0910       45.6836
    RHS1      NodeBalance1002      -38.4317
    RHS1      NodeBalance1006       29.8417
 SC Scen41    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -25.3114
    RHS1      NodeBalance0108      -47.7953
    RHS1      NodeBalance0109      -55.5661
    RHS1      NodeBalance0203      -24.4863
    RHS1      NodeBalance0205       28.7361
    RHS1      NodeBalance0307       25.3114
    RHS1      NodeBalance0403       24.4863
    RHS1      NodeBalance0404       41.2235
    RHS1      NodeBalance0406      -59.1412
    RHS1      NodeBalance0601      -65.5149
    RHS1      NodeBalance0602       31.0648
    RHS1      NodeBalance0609       55.5661
    RHS1      NodeBalance0704      -41.2235
    RHS1      NodeBalance0801       65.5149
    RHS1      NodeBalance0805      -28.7361
    RHS1      NodeBalance0808       47.7953
    RHS1      NodeBalance0810      -85.2958
    RHS1      NodeBalance0910       85.2958
    RHS1      NodeBalance1002      -31.0648
    RHS1      NodeBalance1006       59.1412
 SC Scen42    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -21.3117
    RHS1      NodeBalance0108      -37.8303
    RHS1      NodeBalance0109      -19.8578
    RHS1      NodeBalance0203      -64.6043
    RHS1      NodeBalance0205       60.5171
    RHS1      NodeBalance0307       21.3117
    RHS1      NodeBalance0403       64.6043
    RHS1      NodeBalance0404       27.0537
    RHS1      NodeBalance0406      -66.1798
    RHS1      NodeBalance0601      -38.4057
    RHS1      NodeBalance0602       10.1298
    RHS1      NodeBalance0609       19.8578
    RHS1      NodeBalance0704      -27.0537
    RHS1      NodeBalance0801       38.4057
    RHS1      NodeBalance0805      -60.5171
    RHS1      NodeBalance0808       37.8303
    RHS1      NodeBalance0810      -61.4462
    RHS1      NodeBalance0910       61.4462
    RHS1      NodeBalance1002      -10.1298
    RHS1      NodeBalance1006       66.1798
 SC Scen43    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -51.4968
    RHS1      NodeBalance0108      -67.5073
    RHS1      NodeBalance0109      -49.9579
    RHS1      NodeBalance0203      -44.9676
    RHS1      NodeBalance0205       53.1002
    RHS1      NodeBalance0307       51.4968
    RHS1      NodeBalance0403       44.9676
    RHS1      NodeBalance0404       44.0085
    RHS1      NodeBalance0406      -30.7226
    RHS1      NodeBalance0601      -54.5336
    RHS1      NodeBalance0602       33.5577
    RHS1      NodeBalance0609       49.9579
    RHS1      NodeBalance0704      -44.0085
    RHS1      NodeBalance0801       54.5336
    RHS1      NodeBalance0805      -53.1002
    RHS1      NodeBalance0808       67.5073
    RHS1      NodeBalance0810      -45.2383
    RHS1      NodeBalance0910       45.2383
    RHS1      NodeBalance1002      -33.5577
    RHS1      NodeBalance1006       30.7226
 SC Scen44    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -67.9057
    RHS1      NodeBalance0108      -81.4046
    RHS1      NodeBalance0109      -41.5119
    RHS1      NodeBalance0203      -68.7077
    RHS1      NodeBalance0205       53.9813
    RHS1      NodeBalance0307       67.9057
    RHS1      NodeBalance0403       68.7077
    RHS1      NodeBalance0404       35.2325
    RHS1      NodeBalance0406      -53.1305
    RHS1      NodeBalance0601      -50.5165
    RHS1      NodeBalance0602       25.5373
    RHS1      NodeBalance0609       41.5119
    RHS1      NodeBalance0704      -35.2325
    RHS1      NodeBalance0801       50.5165
    RHS1      NodeBalance0805      -53.9813
    RHS1      NodeBalance0808       81.4046
    RHS1      NodeBalance0810      -57.8716
    RHS1      NodeBalance0910       57.8716
    RHS1      NodeBalance1002      -25.5373
    RHS1      NodeBalance1006       53.1305
 SC Scen45    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -32.4849
    RHS1      NodeBalance0108      -44.0827
    RHS1      NodeBalance0109      -17.2735
    RHS1      NodeBalance0203      -35.0996
    RHS1      NodeBalance0205       29.8244
    RHS1      NodeBalance0307       32.4849
    RHS1      NodeBalance0403       35.0996
    RHS1      NodeBalance0404       35.0767
    RHS1      NodeBalance0406      -61.3086
    RHS1      NodeBalance0601      -31.9502
    RHS1      NodeBalance0602         27.19
    RHS1      NodeBalance0609       17.2735
    RHS1      NodeBalance0704      -35.0767
    RHS1      NodeBalance0801       31.9502
    RHS1      NodeBalance0805      -29.8244
    RHS1      NodeBalance0808       44.0827
    RHS1      NodeBalance0810      -73.6103
    RHS1      NodeBalance0910       73.6103
    RHS1      NodeBalance1002        -27.19
    RHS1      NodeBalance1006       61.3086
 SC Scen46    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -49.2233
    RHS1      NodeBalance0108      -23.9191
    RHS1      NodeBalance0109      -66.3895
    RHS1      NodeBalance0203      -39.5263
    RHS1      NodeBalance0205        30.087
    RHS1      NodeBalance0307       49.2233
    RHS1      NodeBalance0403       39.5263
    RHS1      NodeBalance0404       17.4258
    RHS1      NodeBalance0406      -65.8471
    RHS1      NodeBalance0601      -72.1815
    RHS1      NodeBalance0602       14.5048
    RHS1      NodeBalance0609       66.3895
    RHS1      NodeBalance0704      -17.4258
    RHS1      NodeBalance0801       72.1815
    RHS1      NodeBalance0805       -30.087
    RHS1      NodeBalance0808       23.9191
    RHS1      NodeBalance0810      -72.2806
    RHS1      NodeBalance0910       72.2806
    RHS1      NodeBalance1002      -14.5048
    RHS1      NodeBalance1006       65.8471
 SC Scen47    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -60.4106
    RHS1      NodeBalance0108      -77.0099
    RHS1      NodeBalance0109      -43.2844
    RHS1      NodeBalance0203      -16.5572
    RHS1      NodeBalance0205       34.4609
    RHS1      NodeBalance0307       60.4106
    RHS1      NodeBalance0403       16.5572
    RHS1      NodeBalance0404       16.5848
    RHS1      NodeBalance0406      -98.3206
    RHS1      NodeBalance0601       -42.829
    RHS1      NodeBalance0602       15.9237
    RHS1      NodeBalance0609       43.2844
    RHS1      NodeBalance0704      -16.5848
    RHS1      NodeBalance0801        42.829
    RHS1      NodeBalance0805      -34.4609
    RHS1      NodeBalance0808       77.0099
    RHS1      NodeBalance0810      -5.95729
    RHS1      NodeBalance0910       5.95729
    RHS1      NodeBalance1002      -15.9237
    RHS1      NodeBalance1006       98.3206
 SC Scen48    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -31.9714
    RHS1      NodeBalance0108      -37.0987
    RHS1      NodeBalance0109      -59.8526
    RHS1      NodeBalance0203      -74.9095
    RHS1      NodeBalance0205       43.1543
    RHS1      NodeBalance0307       31.9714
    RHS1      NodeBalance0403       74.9095
    RHS1      NodeBalance0404       23.3529
    RHS1      NodeBalance0406      -83.9529
    RHS1      NodeBalance0601      -84.4311
    RHS1      NodeBalance0602       14.9342
    RHS1      NodeBalance0609       59.8526
    RHS1      NodeBalance0704      -23.3529
    RHS1      NodeBalance0801       84.4311
    RHS1      NodeBalance0805      -43.1543
    RHS1      NodeBalance0808       37.0987
    RHS1      NodeBalance0810      -45.5802
    RHS1      NodeBalance0910       45.5802
    RHS1      NodeBalance1002      -14.9342
    RHS1      NodeBalance1006       83.9529
 SC Scen49    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107        -57.06
    RHS1      NodeBalance0108      -79.7105
    RHS1      NodeBalance0109      -18.8826
    RHS1      NodeBalance0203      -62.9695
    RHS1      NodeBalance0205       34.5742
    RHS1      NodeBalance0307         57.06
    RHS1      NodeBalance0403       62.9695
    RHS1      NodeBalance0404       38.0813
    RHS1      NodeBalance0406      -34.5672
    RHS1      NodeBalance0601      -32.4252
    RHS1      NodeBalance0602       28.6693
    RHS1      NodeBalance0609       18.8826
    RHS1      NodeBalance0704      -38.0813
    RHS1      NodeBalance0801       32.4252
    RHS1      NodeBalance0805      -34.5742
    RHS1      NodeBalance0808       79.7105
    RHS1      NodeBalance0810       -57.149
    RHS1      NodeBalance0910        57.149
    RHS1      NodeBalance1002      -28.6693
    RHS1      NodeBalance1006       34.5672
 SC Scen50    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -51.9496
    RHS1      NodeBalance0108      -76.2768
    RHS1      NodeBalance0109      -53.1077
    RHS1      NodeBalance0203       -70.765
    RHS1      NodeBalance0205       36.5793
    RHS1      NodeBalance0307       51.9496
    RHS1      NodeBalance0403        70.765
    RHS1      NodeBalance0404       47.9533
    RHS1      NodeBalance0406      -78.5831
    RHS1      NodeBalance0601      -53.4126
    RHS1      NodeBalance0602       37.5423
    RHS1      NodeBalance0609       53.1077
    RHS1      NodeBalance0704      -47.9533
    RHS1      NodeBalance0801       53.4126
    RHS1      NodeBalance0805      -36.5793
    RHS1      NodeBalance0808       76.2768
    RHS1      NodeBalance0810      -65.2388
    RHS1      NodeBalance0910       65.2388
    RHS1      NodeBalance1002      -37.5423
    RHS1      NodeBalance1006       78.5831
 SC Scen51    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -65.3723
    RHS1      NodeBalance0108        -79.49
    RHS1      NodeBalance0109      -57.3575
    RHS1      NodeBalance0203      -76.0728
    RHS1      NodeBalance0205        51.424
    RHS1      NodeBalance0307       65.3723
    RHS1      NodeBalance0403       76.0728
    RHS1      NodeBalance0404       13.9812
    RHS1      NodeBalance0406      -54.6888
    RHS1      NodeBalance0601      -76.5442
    RHS1      NodeBalance0602        8.9829
    RHS1      NodeBalance0609       57.3575
    RHS1      NodeBalance0704      -13.9812
    RHS1      NodeBalance0801       76.5442
    RHS1      NodeBalance0805       -51.424
    RHS1      NodeBalance0808         79.49
    RHS1      NodeBalance0810      -13.9292
    RHS1      NodeBalance0910       13.9292
    RHS1      NodeBalance1002       -8.9829
    RHS1      NodeBalance1006       54.6888
 SC Scen52    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -18.9317
    RHS1      NodeBalance0108      -28.0903
    RHS1      NodeBalance0109      -20.3466
    RHS1      NodeBalance0203      -87.0908
    RHS1      NodeBalance0205       44.0077
    RHS1      NodeBalance0307       18.9317
    RHS1      NodeBalance0403       87.0908
    RHS1      NodeBalance0404       18.2003
    RHS1      NodeBalance0406      -45.4694
    RHS1      NodeBalance0601      -45.5329
    RHS1      NodeBalance0602       24.1625
    RHS1      NodeBalance0609       20.3466
    RHS1      NodeBalance0704      -18.2003
    RHS1      NodeBalance0801       45.5329
    RHS1      NodeBalance0805      -44.0077
    RHS1      NodeBalance0808       28.0903
    RHS1      NodeBalance0810      -64.1396
    RHS1      NodeBalance0910       64.1396
    RHS1      NodeBalance1002      -24.1625
    RHS1      NodeBalance1006       45.4694
 SC Scen53    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -61.1362
    RHS1      NodeBalance0108       -80.362
    RHS1      NodeBalance0109      -35.6037
    RHS1      NodeBalance0203      -47.6866
    RHS1      NodeBalance0205       42.8646
    RHS1      NodeBalance0307       61.1362
    RHS1      NodeBalance0403       47.6866
    RHS1      NodeBalance0404       23.7841
    RHS1      NodeBalance0406      -15.3819
    RHS1      NodeBalance0601      -26.0684
    RHS1      NodeBalance0602       26.4659
    RHS1      NodeBalance0609       35.6037
    RHS1      NodeBalance0704      -23.7841
    RHS1      NodeBalance0801       26.0684
    RHS1      NodeBalance0805      -42.8646
    RHS1      NodeBalance0808        80.362
    RHS1      NodeBalance0810      -51.7145
    RHS1      NodeBalance0910       51.7145
    RHS1      NodeBalance1002      -26.4659
    RHS1      NodeBalance1006       15.3819
 SC Scen54    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -20.5255
    RHS1      NodeBalance0108      -19.8098
    RHS1      NodeBalance0109      -62.0927
    RHS1      NodeBalance0203      -65.2646
    RHS1      NodeBalance0205        19.855
    RHS1      NodeBalance0307       20.5255
    RHS1      NodeBalance0403       65.2646
    RHS1      NodeBalance0404       43.1447
    RHS1      NodeBalance0406      -69.9414
    RHS1      NodeBalance0601      -81.9001
    RHS1      NodeBalance0602       36.1894
    RHS1      NodeBalance0609       62.0927
    RHS1      NodeBalance0704      -43.1447
    RHS1      NodeBalance0801       81.9001
    RHS1      NodeBalance0805       -19.855
    RHS1      NodeBalance0808       19.8098
    RHS1      NodeBalance0810      -67.8197
    RHS1      NodeBalance0910       67.8197
    RHS1      NodeBalance1002      -36.1894
    RHS1      NodeBalance1006       69.9414
 SC Scen55    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107       -15.042
    RHS1      NodeBalance0108      -15.7927
    RHS1      NodeBalance0109      -2.36847
    RHS1      NodeBalance0203      -78.9052
    RHS1      NodeBalance0205       48.9292
    RHS1      NodeBalance0307        15.042
    RHS1      NodeBalance0403       78.9052
    RHS1      NodeBalance0404       43.5683
    RHS1      NodeBalance0406      -44.8232
    RHS1      NodeBalance0601      -6.18549
    RHS1      NodeBalance0602       30.8636
    RHS1      NodeBalance0609       2.36847
    RHS1      NodeBalance0704      -43.5683
    RHS1      NodeBalance0801       6.18549
    RHS1      NodeBalance0805      -48.9292
    RHS1      NodeBalance0808       15.7927
    RHS1      NodeBalance0810      -52.9603
    RHS1      NodeBalance0910       52.9603
    RHS1      NodeBalance1002      -30.8636
    RHS1      NodeBalance1006       44.8232
 SC Scen56    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -42.6383
    RHS1      NodeBalance0108      -49.2332
    RHS1      NodeBalance0109      -50.9206
    RHS1      NodeBalance0203      -74.2722
    RHS1      NodeBalance0205       41.6515
    RHS1      NodeBalance0307       42.6383
    RHS1      NodeBalance0403       74.2722
    RHS1      NodeBalance0404       37.0002
    RHS1      NodeBalance0406      -35.8315
    RHS1      NodeBalance0601      -46.8265
    RHS1      NodeBalance0602       31.4924
    RHS1      NodeBalance0609       50.9206
    RHS1      NodeBalance0704      -37.0002
    RHS1      NodeBalance0801       46.8265
    RHS1      NodeBalance0805      -41.6515
    RHS1      NodeBalance0808       49.2332
    RHS1      NodeBalance0810      -9.69734
    RHS1      NodeBalance0910       9.69734
    RHS1      NodeBalance1002      -31.4924
    RHS1      NodeBalance1006       35.8315
 SC Scen57    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -60.9777
    RHS1      NodeBalance0108      -75.2468
    RHS1      NodeBalance0109      -36.0811
    RHS1      NodeBalance0203      -35.9177
    RHS1      NodeBalance0205        34.926
    RHS1      NodeBalance0307       60.9777
    RHS1      NodeBalance0403       35.9177
    RHS1      NodeBalance0404       28.0809
    RHS1      NodeBalance0406      -22.9161
    RHS1      NodeBalance0601      -67.5313
    RHS1      NodeBalance0602       22.8807
    RHS1      NodeBalance0609       36.0811
    RHS1      NodeBalance0704      -28.0809
    RHS1      NodeBalance0801       67.5313
    RHS1      NodeBalance0805       -34.926
    RHS1      NodeBalance0808       75.2468
    RHS1      NodeBalance0810      -60.5204
    RHS1      NodeBalance0910       60.5204
    RHS1      NodeBalance1002      -22.8807
    RHS1      NodeBalance1006       22.9161
 SC Scen58    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107        -41.01
    RHS1      NodeBalance0108      -42.9333
    RHS1      NodeBalance0109      -51.4661
    RHS1      NodeBalance0203      -60.7407
    RHS1      NodeBalance0205       30.3722
    RHS1      NodeBalance0307         41.01
    RHS1      NodeBalance0403       60.7407
    RHS1      NodeBalance0404       9.93705
    RHS1      NodeBalance0406       -51.135
    RHS1      NodeBalance0601      -69.0507
    RHS1      NodeBalance0602       20.0131
    RHS1      NodeBalance0609       51.4661
    RHS1      NodeBalance0704      -9.93705
    RHS1      NodeBalance0801       69.0507
    RHS1      NodeBalance0805      -30.3722
    RHS1      NodeBalance0808       42.9333
    RHS1      NodeBalance0810      -47.6369
    RHS1      NodeBalance0910       47.6369
    RHS1      NodeBalance1002      -20.0131
    RHS1      NodeBalance1006        51.135
 SC Scen59    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -11.3256
    RHS1      NodeBalance0108      -30.4203
    RHS1      NodeBalance0109      -65.6962
    RHS1      NodeBalance0203       -21.166
    RHS1      NodeBalance0205       58.7333
    RHS1      NodeBalance0307       11.3256
    RHS1      NodeBalance0403        21.166
    RHS1      NodeBalance0404       43.3366
    RHS1      NodeBalance0406      -86.6393
    RHS1      NodeBalance0601       -71.369
    RHS1      NodeBalance0602       31.1696
    RHS1      NodeBalance0609       65.6962
    RHS1      NodeBalance0704      -43.3366
    RHS1      NodeBalance0801        71.369
    RHS1      NodeBalance0805      -58.7333
    RHS1      NodeBalance0808       30.4203
    RHS1      NodeBalance0810       -50.155
    RHS1      NodeBalance0910        50.155
    RHS1      NodeBalance1002      -31.1696
    RHS1      NodeBalance1006       86.6393
 SC Scen60    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -59.5393
    RHS1      NodeBalance0108      -74.0366
    RHS1      NodeBalance0109      -54.2095
    RHS1      NodeBalance0203      -84.7247
    RHS1      NodeBalance0205        26.213
    RHS1      NodeBalance0307       59.5393
    RHS1      NodeBalance0403       84.7247
    RHS1      NodeBalance0404       11.9346
    RHS1      NodeBalance0406      -80.3434
    RHS1      NodeBalance0601      -76.1926
    RHS1      NodeBalance0602       13.4644
    RHS1      NodeBalance0609       54.2095
    RHS1      NodeBalance0704      -11.9346
    RHS1      NodeBalance0801       76.1926
    RHS1      NodeBalance0805       -26.213
    RHS1      NodeBalance0808       74.0366
    RHS1      NodeBalance0810      -58.4546
    RHS1      NodeBalance0910       58.4546
    RHS1      NodeBalance1002      -13.4644
    RHS1      NodeBalance1006       80.3434
 SC Scen61    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -44.9073
    RHS1      NodeBalance0108      -63.5333
    RHS1      NodeBalance0109      -41.9182
    RHS1      NodeBalance0203      -65.5237
    RHS1      NodeBalance0205       7.06173
    RHS1      NodeBalance0307       44.9073
    RHS1      NodeBalance0403       65.5237
    RHS1      NodeBalance0404       39.9527
    RHS1      NodeBalance0406      -38.8608
    RHS1      NodeBalance0601      -68.0365
    RHS1      NodeBalance0602       24.5731
    RHS1      NodeBalance0609       41.9182
    RHS1      NodeBalance0704      -39.9527
    RHS1      NodeBalance0801       68.0365
    RHS1      NodeBalance0805      -7.06173
    RHS1      NodeBalance0808       63.5333
    RHS1      NodeBalance0810      -38.2882
    RHS1      NodeBalance0910       38.2882
    RHS1      NodeBalance1002      -24.5731
    RHS1      NodeBalance1006       38.8608
 SC Scen62    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -11.1379
    RHS1      NodeBalance0108      -21.5443
    RHS1      NodeBalance0109      -53.5367
    RHS1      NodeBalance0203      -51.2234
    RHS1      NodeBalance0205       63.4322
    RHS1      NodeBalance0307       11.1379
    RHS1      NodeBalance0403       51.2234
    RHS1      NodeBalance0404       38.0146
    RHS1      NodeBalance0406      -33.5325
    RHS1      NodeBalance0601      -60.1594
    RHS1      NodeBalance0602       20.8575
    RHS1      NodeBalance0609       53.5367
    RHS1      NodeBalance0704      -38.0146
    RHS1      NodeBalance0801       60.1594
    RHS1      NodeBalance0805      -63.4322
    RHS1      NodeBalance0808       21.5443
    RHS1      NodeBalance0810      -60.7295
    RHS1      NodeBalance0910       60.7295
    RHS1      NodeBalance1002      -20.8575
    RHS1      NodeBalance1006       33.5325
 SC Scen63    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -66.1288
    RHS1      NodeBalance0108      -81.8938
    RHS1      NodeBalance0109       -45.285
    RHS1      NodeBalance0203      -88.3919
    RHS1      NodeBalance0205       27.9418
    RHS1      NodeBalance0307       66.1288
    RHS1      NodeBalance0403       88.3919
    RHS1      NodeBalance0404       20.5135
    RHS1      NodeBalance0406      -61.4926
    RHS1      NodeBalance0601      -68.5429
    RHS1      NodeBalance0602        11.714
    RHS1      NodeBalance0609        45.285
    RHS1      NodeBalance0704      -20.5135
    RHS1      NodeBalance0801       68.5429
    RHS1      NodeBalance0805      -27.9418
    RHS1      NodeBalance0808       81.8938
    RHS1      NodeBalance0810      -21.6747
    RHS1      NodeBalance0910       21.6747
    RHS1      NodeBalance1002       -11.714
    RHS1      NodeBalance1006       61.4926
 SC Scen64    Scen1         0.015625   Stage2    
    RHS1      NodeBalance0107      -29.5768
    RHS1      NodeBalance0108      -21.1218
    RHS1      NodeBalance0109      -17.0113
    RHS1      NodeBalance0203      -77.0385
    RHS1      NodeBalance0205       55.9153
    RHS1      NodeBalance0307       29.5768
    RHS1      NodeBalance0403       77.0385
    RHS1      NodeBalance0404       34.2075
    RHS1      NodeBalance0406       -83.554
    RHS1      NodeBalance0601      -37.2125
    RHS1      NodeBalance0602       24.7882
    RHS1      NodeBalance0609       17.0113
    RHS1      NodeBalance0704      -34.2075
    RHS1      NodeBalance0801       37.2125
    RHS1      NodeBalance0805      -55.9153
    RHS1      NodeBalance0808       21.1218
    RHS1      NodeBalance0810      -37.8651
    RHS1      NodeBalance0910       37.8651
    RHS1      NodeBalance1002      -24.7882
    RHS1      NodeBalance1006        83.554
ENDATA



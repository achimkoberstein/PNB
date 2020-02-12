*   
*   example_example.scen
*   
*   Generated with the MPL Modeling System
*   Copyright (c) 1988-2018  Maximal Software
*   
*   Date:               November 1, 2018
*   Time:               13:28
*   
*   Constraints:        9
*   Variables:          10
*   Integers:           5
*   Nonzeros:           20
*   Density:            22 %
*   
NAME          MCFNDSD  (MIN)
ROWS
  N Total_Cost
  E NodeBalance11
  E NodeBalance21
  E NodeBalance31
  E NodeBalance41
  L Capacity12
  L Capacity13
  L Capacity23
  L Capacity24
  L Capacity34
COLUMNS
    y12       Total_Cost         101.0   Capacity12         -10.0
    y13       Total_Cost         102.0   Capacity13         -10.0
    y23       Total_Cost         103.0   Capacity23         -10.0
    y24       Total_Cost         104.0   Capacity24         -10.0
    y34       Total_Cost         105.0   Capacity34         -10.0
    x121      Total_Cost          11.0   NodeBalance11           1.0
    x121      NodeBalance21          -1.0   Capacity12           1.0
    x131      Total_Cost          12.0   NodeBalance11           1.0
    x131      NodeBalance31          -1.0   Capacity13           1.0
    x231      Total_Cost          13.0   NodeBalance21           1.0
    x231      NodeBalance31          -1.0   Capacity23           1.0
    x241      Total_Cost          14.0   NodeBalance21           1.0
    x241      NodeBalance41          -1.0   Capacity24           1.0
    x341      Total_Cost          15.0   NodeBalance31           1.0
    x341      NodeBalance41          -1.0   Capacity34           1.0
RHS
    RHS1      NodeBalance11          15.0   NodeBalance41         -15.0
BOUNDS
 BV BOUND1    y12                1.0
 BV BOUND1    y13                1.0
 BV BOUND1    y23                1.0
 BV BOUND1    y24                1.0
 BV BOUND1    y34                1.0
*
TIME          MCFNDSD  (MIN)
PERIODS       IMPLICIT
    y12       NodeBalance11                 Stage1    
    x121      NodeBalance11                 Stage2    
*
STOCH         MCFNDSD  (MIN)
SCENARIOS
 SC Scen1     'ROOT'         0.33333   Stage1    
    RHS1      NodeBalance11          15.0
    RHS1      NodeBalance41         -15.0
 SC Scen2     Scen1          0.33333   Stage2    
    RHS1      NodeBalance11          10.0
    RHS1      NodeBalance41         -10.0
 SC Scen3     Scen1          0.33333   Stage2    
    RHS1      NodeBalance11          20.0
    RHS1      NodeBalance41         -20.0
ENDATA



\Problem name: 

Minimize
obj: 100 x0 + 100 x1 + 10 x2 + 10 x3 + x4 + x5 + 500 x6 + 100 x7 + 500 x8
Subject To
cons0:  x0 + x4 = 70
cons1:  x1 + x5 = 0
cons2:  - x0 + x2 = 0
cons3:  - x1 + x3 = 30
cons4:  - x2 - x4 = -70
cons5:  - x3 - x5 = -30
cons6:  x0 -60 x6 <= 0
cons7:  x1 -60 x6 <= 0
cons8:  x2 -100 x7 <= 0
cons9:  x3 -100 x7 <= 0
cons10:  x4 -10 x8 <= 0
cons11:  x5 -10 x8 <= 0
Bounds
 0 <= x6 <= 1
 0 <= x7 <= 1
 0 <= x8 <= 1
Integers
x6 x7 x8 
End

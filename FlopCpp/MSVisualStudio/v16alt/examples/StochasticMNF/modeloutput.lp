\ENCODING=ISO-8859-1
\Problem name: OsiDefaultName

Minimize
 obj1: 595 x1 + 595 x2 + 554 x3 + 143 x4 + 100 x5 + 100 x6 + 100 x7 + 100 x8
       + 27 x9 + 27 x10 + 76 x11 + 76 x12
Subject To
 c1:  x5 - x11  = -70.332
 c2:  x6 - x12  = -12.7952
 c3:  - x5 + x7 - x9  = 0
 c4:  - x6 + x8 - x10  = 12.7952
 c5:  - x7 + x9 + x11  = 70.332
 c6:  - x8 + x10 + x12  = 0
 c7:  - 613 x1 + x5 + x6 <= 0
 c8:  - 613 x2 + x7 + x8 <= 0
 c9:  - 265 x3 + x9 + x10 <= 0
 c10: - 129 x4 + x11 + x12 <= 0
Bounds
 0 <= x1 <= 1
 0 <= x2 <= 1
 0 <= x3 <= 1
 0 <= x4 <= 1
Binaries
 x1  x2  x3  x4 
End

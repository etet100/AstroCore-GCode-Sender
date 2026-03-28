(1001)
(T1 D=3.175 CR=0 - ZMIN=-1 - flat end mill)
G90 G94
G17
G21
(-Attention- Property Safe Retracts is set to Clearance Height.)
(Ensure the clearance height will clear the part and or fixtures.)
(Raise the Z-axis to a safe height before starting the program.)
(When using Fusion for Personal Use, the feedrate of rapid)
(moves is reduced to match the feedrate of cutting moves,)
(which can increase machining time. Unrestricted rapid moves)
(are available with a Fusion Subscription.)

(2D Contour1)
T1
S5000 M3
G17 G90 G94
G54
M8
G0 X5.589 Y3.108
Z15
G1 Z5 F1000
Z0.635 F333.3
Z-0.683
X5.592 Y3.116 Z-0.753 F1000
X5.6 Y3.138 Z-0.82
X5.613 Y3.173 Z-0.88
X5.63 Y3.22 Z-0.931
X5.651 Y3.277 Z-0.969
X5.674 Y3.34 Z-0.992
X5.699 Y3.406 Z-1
X5.809 Y3.704
G3 X5.622 Y4.112 I-0.298 J0.11
G2 X4.586 Y5.6 I0.551 J1.489
G1 Y14.72
X4.581 Y15.226
X1.733 Y15.197
X1.711 Y15.084
X1.67 Y14.756
X1.647 Y14.389
X1.635 Y13.993
Y5.6
G2 X0.048 Y4.013 I-1.587 J0
G1 X-2.408
G2 X-3.995 Y5.6 I0 J1.587
G1 Y14.985
X-4.016 Y15.554
X-4.037 Y15.762
X-4.067 Y15.934
X-4.11 Y16.084
X-4.179 Y16.258
X-4.284 Y16.478
X-4.294 Y16.498
Z15
M9
M5
M30

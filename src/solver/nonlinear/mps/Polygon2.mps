NAME          Polygon
ROWS
 N  __OBJ__
 E  OBJEQ
 G  T2T1
 G  T3T2
 G  T4T3
 L  V1V2
 L  V1V3
 L  V1V4
 L  V2V3
 L  V2V4
 L  V3V4
COLUMNS
    OBJX      __OBJ__              1   OBJEQ               -1 
    THETA1    T2T1                -1 
    THETA2    T3T2                -1   T2T1                 1 
    THETA3    T4T3                -1   T3T2                 1 
    THETA4    T4T3                 1 
    =         OBJEQ     = ( RHO2 * RHO1 * SIN ( THETA2 - THETA1 ) + RHO3 * RHO2 * SIN ( THETA3 - THETA2 ) + RHO4 * RHO3 * SIN ( THETA4 - THETA3 ) ) *  0.5
    =         V1V2      = RHO1 ^  2 + RHO2 ^  2 -  2 * RHO1 * RHO2 * COS ( THETA2 - THETA1 )
    =         V1V3      = RHO1 ^  2 + RHO3 ^  2 -  2 * RHO1 * RHO3 * COS ( THETA3 - THETA1 )
    =         V1V4      = RHO1 ^  2 + RHO4 ^  2 -  2 * RHO1 * RHO4 * COS ( THETA4 - THETA1 )
    =         V2V3      = RHO2 ^  2 + RHO3 ^  2 -  2 * RHO2 * RHO3 * COS ( THETA3 - THETA2 )
    =         V2V4      = RHO2 ^  2 + RHO4 ^  2 -  2 * RHO2 * RHO4 * COS ( THETA4 - THETA2 )
    =         V3V4      = RHO3 ^  2 + RHO4 ^  2 -  2 * RHO3 * RHO4 * COS ( THETA4 - THETA3 )
RHS
    RHS00001  T2T1             0.001   T3T2             0.001 
    RHS00001  T4T3             0.001   V1V2                 1 
    RHS00001  V1V3                 1   V1V4                 1 
    RHS00001  V2V3                 1   V2V4                 1 
    RHS00001  V3V4                 1 
BOUNDS
 FR BND00001  OBJX    
 UP BND00001  THETA4       3.1415926
 FX BND00001  =                    1
 LO BND00001  RHO1              0.01
 UP BND00001  RHO1                 1
 LO BND00001  RHO2              0.01
 UP BND00001  RHO2                 1
 LO BND00001  RHO3              0.01
 UP BND00001  RHO3                 1
 LO BND00001  RHO4              0.01
 UP BND00001  RHO4                 1
SLPDATA
 IV IVSET1    THETA1        0.628318
 IV IVSET1    THETA2        1.256636
 IV IVSET1    THETA3        1.884954
 IV IVSET1    THETA4        2.513272
 IV IVSET1    =                    1
 IV IVSET1    RHO1                 1
 IV IVSET1    RHO2                 1
 IV IVSET1    RHO3                 1
 IV IVSET1    RHO4                 1
ENDATA
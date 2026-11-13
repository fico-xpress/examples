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
    RHO1      OBJEQ     = RHO2 * SIN ( THETA2 - THETA1 ) * 0.5
    RHO1      V1V2      = RHO1 -  2 * RHO2 * COS ( THETA2 - THETA1 )
    RHO1      V1V3      = RHO1 -  2 * RHO3 * COS ( THETA3 - THETA1 )
    RHO1      V1V4      = RHO1 -  2 * RHO4 * COS ( THETA4 - THETA1 )
    RHO2      OBJEQ     = RHO3 * SIN ( THETA3 - THETA2 ) * 0.5
    RHO2      V1V2      = RHO2
    RHO2      V2V3      = RHO2 -  2 * RHO3 * COS ( THETA3 - THETA2 )
    RHO2      V2V4      = RHO2 -  2 * RHO4 * COS ( THETA4 - THETA2 )
    RHO3      OBJEQ     = RHO4 * SIN ( THETA4 - THETA3 ) * 0.5
    RHO3      V1V3      = RHO3
    RHO3      V2V3      = RHO3
    RHO3      V3V4      = RHO3 -  2 * RHO4 * COS ( THETA4 - THETA3 )
    RHO4      V1V4      = RHO4
    RHO4      V2V4      = RHO4
    RHO4      V3V4      = RHO4
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
ENDATA

/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file folioqp.c
  ``````````````
  Loading a small QP problem via XPRSloadqp.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. June 2010
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"

int main(int argc, char **argv)
{
 XPRSprob prob;
 int s, status;
 double objval, *sol;

 /* Problem parameters */
 int ncol = 10;
 int nrow = 3;
 int nqt  = 43;

 /* Row data */
 char rowtype[] = {'G','E','G'};
 double rhs[]   = {0.5, 1,  9};

 /* Column data */
 double obj[] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
 double lb[]  = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
 double ub[]  = {0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3};

 /* Matrix coefficient data */
 int colbeg[]    = {0,    3,     6,     9,     12,  14,  16,  18,  20,   22,  24};
 int rowidx[]    = {0,1,2,0,1, 2,0,1, 2,0,1, 2, 1,2, 1,2, 1,2, 1,2, 1, 2, 1,2};
 double matval[] = {1,1,5,1,1,17,1,1,26,1,1,12, 1,8, 1,9, 1,7, 1,6, 1,31, 1,21};

 /* QP problem data */
 int qcol1[]   = {0,
                    1,1,1,1,1,1,1,1,1,
                      2,2,2,2,2,  2,2,
                        3,  3,3,  3,3,
                          4,4,4,4,4,4,
                            5,5,5,5,5,
                              6,6,6,6,
                                7,7,7,
                                  8,8,
                                    9};
 int qcol2[]   = {0,
                    1,2,3,4,5,6,7,8,9,
                      2,3,4,5,6,  8,9,
                        3,  5,6,  8,9,
                          4,5,6,7,8,9,
                            5,6,7,8,9,
                              6,7,8,9,
                                7,8,9,
                                  8,9,
                                    9};
 double qval[] = {0.1,
                      19,-2, 4,1,   1, 1,0.5, 10,  5,
                         28, 1,2,   1, 1,     -2, -1,
                            22,     1, 2,      3,  4,
                               4,-1.5,-2, -1,  1,  1,
                                  3.5, 2,0.5,  1,1.5,
                                       5,0.5,  1,2.5,
                                           1,0.5,0.5,
                                              25,  8,
                                                  16};
 for(s=0;s<nqt;s++) qval[s]*=2;

 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   printf("Failed to initialize Xpress.\n");
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */

                                         /* Load the problem matrix */
 XPRSloadqp(prob, "FolioQP", ncol, nrow, rowtype, rhs, NULL,
            obj, colbeg, NULL, rowidx, matval, lb, ub, 
            nqt, qcol1, qcol2, qval);

 XPRSchgobjsense(prob, XPRS_OBJ_MINIMIZE);  /* Set sense to maximization */
 XPRSlpoptimize(prob, "");               /* Solve the problem */
 
 XPRSgetintattrib(prob, XPRS_LPSTATUS, &status);   /* Get solution status */

 if(status == XPRS_LP_OPTIMAL)
 {
  XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &objval);  /* Get objective value */
  printf("Minimum variance: %g\n", objval);
 
  sol = (double *)malloc(ncol*sizeof(double));
  XPRSgetsolution(prob, NULL, sol, 0, ncol-1);     /* Get primal solution */
  for(s=0;s<ncol;s++) printf("%d: %g%%\n", s, sol[s]*100);   
 }

 XPRSdestroyprob(prob);                  /* Delete the problem */
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file foliomip.c
  ```````````````
  Loading a small MIP problem via XPRSloadmip.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. Sep. 2022
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
 int nmip = 10;

 /* Row data */
 char rowtype[] = {  'L','G','E'};
 double rhs[]   = {1.0/3,0.5, 1};

 /* Column data */
 double obj[] = {  5, 17, 26, 12,  8,  9,  7,  6, 31, 21};
 double lb[]  = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
 double ub[]  = {0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3};

 /* Matrix coefficient data */
 int colbeg[]    = {0,  2,    5,    8,    11,12,13,14,15,  17,  19};
 int rowidx[]    = {1,2,0,1,2,0,1,2,0,1,2, 2, 2, 2, 2, 0,2, 0,2};
 double matval[] = {1,1,1,1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1,1, 1,1};

 /* MIP problem data */
 char miptype[] = {'S','S','S','S','S','S','S','S','S','S'};
 int mipcol[]   = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9};
 double sclim[] = {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};


 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   printf("Failed to initialize Xpress.\n");
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */

                                         /* Load the problem matrix */
 XPRSloadmip(prob, "FolioSC", ncol, nrow, rowtype, rhs, NULL,
             obj, colbeg, NULL, rowidx, matval, lb, ub, 
             nmip, 0, miptype, mipcol, sclim, NULL, NULL, NULL, NULL);

 XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE);  /* Set sense to maximization */
 XPRSmipoptimize(prob, "");              /* Solve the problem */
 
 XPRSgetintattrib(prob, XPRS_MIPSTATUS, &status);  /* Get MIP sol. status */

 if((status == XPRS_MIP_OPTIMAL) || (status == XPRS_MIP_SOLUTION))
 {
  XPRSgetdblattrib(prob, XPRS_MIPOBJVAL, &objval); /* Get objective value */
  printf("Total return: %g\n", objval);
 
  sol = (double *)malloc(ncol*sizeof(double));
  XPRSgetsolution(prob, NULL, sol, 0, ncol-1);     /* Get primal solution */
  for(s=0;s<ncol;s++) printf("%d: %g%%\n", s, sol[s]*100);   
 }

 XPRSdestroyprob(prob);                  /* Delete the problem */
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file foliomip1.c
  ````````````````
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
 int ncol = 20;
 int nrow = 14;
 int nmip = 10;

 /* Row data */
 char rowtype[] = {  'L','G','E','L','L','L','L','L','L','L','L','L','L','L'};
 double rhs[]   = {1.0/3,0.5,  1,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

 /* Column data */
 double obj[] = {  5, 17, 26, 12,  8,  9,  7,  6, 31, 21,0,0,0,0,0,0,0,0,0,0};
 double lb[]  = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0,0,0,0,0,0,0,0,0,0};
 double ub[]  = {0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,1,1,1,1,1,1,1,1,1,1};

 /* Matrix coefficient data */
 int colbeg[] = {0,3,7,11,15,17,19,21,23,26,29,31,33,35,37,39,41,43,45,47,49};
 int rowidx[] = {1,2,4,0,1,2,5,0,1,2,6,0,1,2,7,2,8,2,9,2,10,2,11,0,2,12,0,2,13,3,4,3,5,3,6,3,7,3,8,3,9,3,10,3,11,3,12,3,13};
 double matval[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1};

 /* MIP problem data */
 char miptype[] = {'B','B','B','B','B','B','B','B','B','B'};
 int mipcol[]   = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};


 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   printf("Failed to initialize Xpress.\n");
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */

                                         /* Load the problem matrix */
 XPRSloadmip(prob, "FolioMIP1", ncol, nrow, rowtype, rhs, NULL,
             obj, colbeg, NULL, rowidx, matval, lb, ub, 
             nmip, 0, miptype, mipcol, NULL, NULL, NULL, NULL, NULL);

 XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE);  /* Set sense to maximization */
 XPRSmipoptimize(prob, "");              /* Solve the problem */
 
 XPRSgetintattrib(prob, XPRS_MIPSTATUS, &status);  /* Get MIP sol. status */

 if((status == XPRS_MIP_OPTIMAL) || (status == XPRS_MIP_SOLUTION))
 {
  XPRSgetdblattrib(prob, XPRS_MIPOBJVAL, &objval); /* Get objective value */
  printf("Total return: %g\n", objval);
 
  sol = (double *)malloc(ncol*sizeof(double));
  XPRSgetsolution(prob, NULL, sol, 0, ncol-1);     /* Get primal solution */
  for(s=0;s<ncol/2;s++) 
   printf("%d: %g%% (%g)\n", s, sol[s]*100, sol[ncol/2+s]);   
 }

 XPRSdestroyprob(prob);                  /* Delete the problem */
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

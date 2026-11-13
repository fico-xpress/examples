/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file foliolp.c
  ``````````````
  Loading a small LP problem via XPRSloadlp.

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

 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   printf("Failed to initialize Xpress.\n");
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */

 XPRSsetlogfile(prob, "logfile.txt");    /* Set optimizer output log file */

                                         /* Load the problem matrix */
 XPRSloadlp(prob, "FolioLP", ncol, nrow, rowtype, rhs, NULL,
            obj, colbeg, NULL, rowidx, matval, lb, ub);

 XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE);  /* Set sense to maximization */
 XPRSlpoptimize(prob, "");               /* Solve the problem */
 
 XPRSgetintattrib(prob, XPRS_LPSTATUS, &status);  /* Get LP sol. status */

 if(status == XPRS_LP_OPTIMAL)
 {
  XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &objval); /* Get objective value */
  printf("Total return: %g\n", objval);
 
  sol = (double *)malloc(ncol*sizeof(double));
  XPRSgetsolution(prob, NULL, sol, 0, ncol-1);    /* Get primal solution */
  for(s=0;s<ncol;s++) printf("%d: %g%%\n", s+1, sol[s]*100);   
 }

 XPRSdestroyprob(prob);                  /* Delete the problem */
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

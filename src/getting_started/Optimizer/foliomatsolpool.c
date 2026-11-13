/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file foliomatsolpool.c
  ``````````````````````
  Using the MIP solution pool with a MIP problem
  input from a matrix file.

  (c) 2009 Fair Isaac Corporation
      author: D.Nielsen, S.Heipcke, July 2009, rev. June 2010
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"

int main(int argc, char **argv)
{
 XPRSprob prob;
 XPRSmipsolpool msp;
 int s, nSols, solID, nCols, solStatus;
 double objval, sol;

 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   printf("Failed to initialize Xpress.\n");
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */
                                
 XPRSreadprob(prob, "folio10_7.lp", "");    /* Read the problem matrix */

 /* Create a mip solution pool to store the solutions */
 XPRS_msp_create(&msp);

 /* Attach the mip solution pool to the problem so it will 
    automatically receive the solutions as they are found */
 XPRS_msp_probattach(msp, prob);

 /* Solve the problem */
 XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE);  /* Set sense to maximization */
 XPRSmipoptimize(prob, "");                 /* Solve the problem */

 /* Get the number of solutions found */
 XPRS_msp_getintattrib(msp, XPRS_MSP_SOLUTIONS, &nSols);

 if(nSols)
 {
  printf("Number of solutions: %d\n", nSols);

  /* Get the best objective value of the solutions found */
  XPRS_msp_getdblattribprobextreme(msp, prob, 1, &solID,
                                   XPRS_MSP_SOLPRB_OBJ, &objval);

  printf("Optimal solution ID: %d\n", solID);
  printf("Optimal objective  : %g\n", objval);

  /* Get the number of columns in the solutions */
  XPRS_msp_getintattribsol(msp, solID, &solStatus,
                           XPRS_MSP_SOL_COLS, &nCols);

  /* Print out the solution values of the best solution */
  for(s=0; s<nCols; s++) {
   XPRS_msp_getsol(msp, solID, &solStatus, &sol, s, s, NULL);
   if(sol!=0) printf("%3d: %g\n", s+1, sol);
  }
 }

 XPRSdestroyprob(prob);                  /* Delete the problem */
 XPRS_msp_destroy(msp);
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

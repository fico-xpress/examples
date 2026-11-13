/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file foliomatenumsol.c
  ``````````````````````
  Using the MIP solution enumerator with a MIP problem
  input from a matrix file.

  (c) 2009 Fair Isaac Corporation
      author: D.Nielsen, S.Heipcke, July 2009, rev. Jan. 2023
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"
#include "xprs_mse_defaulthandler.h"

int main(int argc, char **argv)
{
 XPRSprob prob;
 XPRSmipsolpool msp;
 XPRSmipsolenum mse;
 int i, s, nSols, solID, nCols, solStatus, maxSols;
 double objval, sol;

 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   printf("Failed to initialize Xpress.\n");
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */
                                
 XPRSreadprob(prob, "folio10_7.lp", "");    /* Read the problem matrix */

 /* Get the number of columns in the problem */
 XPRSgetintattrib(prob, XPRS_INPUTCOLS, &nCols);

 /* Create a mip solution pool to store the solutions */
 XPRS_msp_create(&msp);

 /* Create a mip solution enumerator to run the search */
 XPRS_mse_create(&mse);

 /* Disable heuristics to avoid duplicate solutions being found and stored */
 XPRSsetintcontrol(prob, XPRS_HEUREMPHASIS, 0);

 /* Disable presolve operations that attempt to improve the efficiency by 
    cutting off MIP solutions from the feasible region that are either:-
    1) Degenerate (i.e., that have equivalent representations with 
       different feasible values of the variables) or
    2) Dominated (i.e., that can be deduced to be worse than solutions 
       contained in the remaining feasible region). 
 */
 XPRSsetintcontrol(prob, XPRS_MIPDUALREDUCTIONS, 0);

 /* Run the enumeration */
 maxSols = 10;
 XPRS_mse_maxim(mse, prob, msp, XPRS_mse_defaulthandler, NULL, &maxSols);

 /* Get the number of solutions found */
 XPRS_mse_getintattrib(mse, XPRS_MSE_SOLUTIONS, &nSols);

 /* Print out the solutions found */
 for(i=1; i<=nSols; i++)
 {
  XPRS_mse_getsollist(mse, XPRS_MSE_METRIC_MIPOBJECT, i, i, &solID, 
                      NULL, NULL);
  XPRS_mse_getsolmetric(mse, solID, &solStatus, XPRS_MSE_METRIC_MIPOBJECT,
                        &objval);
  printf("--------\nSolution %d:  Objective: %g\n", i, objval);
  for(s=0; s<nCols ;s++) {
   XPRS_msp_getsol(msp, solID, &solStatus, &sol, s, s, NULL);
   if(sol!=0) printf("%3d: %g\n", s+1, sol);
  }
 }

 XPRS_mse_destroy(mse);
 XPRS_msp_destroy(msp);

 XPRSdestroyprob(prob);                  /* Delete the problem */ 
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

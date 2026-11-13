/********************************************************
  Xpress Optimizer Example Problems
  =================================

  file folioinput.c
  `````````````````
  Loading an LP problem via matrix input.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. Feb. 2023
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"

int main(int argc, char **argv)
{
 XPRSprob prob;
 int s, status, ncol;
 double objval, *sol;

 /* Initialize Xpress */
 if (XPRSinit(NULL)) {
   char message[512];
   XPRSgetlicerrmsg(message,512);
   printf("%s\n", message);
   return -1;
 }

 XPRScreateprob(&prob);                  /* Create a new problem */
                                
 XPRSreadprob(prob, "Folio","");         /* Read the problem matrix */

 XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE);  /* Set sense to maximization */
 XPRSlpoptimize(prob, "");               /* Solve the problem */

 XPRSwriteprtsol(prob, "Folio.prt", ""); /* Write results to file `Folio.prt' */
 
/* 
 XPRSgetintattrib(prob, XPRS_LPSTATUS, &status);  / * Get LP sol. status * /

 if(status == XPRS_LP_OPTIMAL)
 {
  XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &objval); / * Get objective value * /
  printf("Total return: %g\n", objval);
 
  XPRSgetintattrib(prob, XPRS_INPUTCOLS, &ncol);  / * Get total no. of rows * /
  sol = (double *)malloc(ncol*sizeof(double));
  XPRSgetsolution(prob, NULL, sol, 0, ncol-1);       / * Get primal solution * /
  for(s=0;s<ncol;s++) printf("%d: %g%%\n", s+1, sol[s]*100);   
 }
*/

 XPRSdestroyprob(prob);                  /* Delete the problem */
 XPRSfree();                             /* Terminate Xpress */
  
 return 0;
} 

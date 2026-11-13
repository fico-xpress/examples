/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugxprs2.c
   ``````````````
   Passing from Mosel to solving with Xpress Optimizer.
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprm.h"
#include "xprs.h"

#ifdef _WIN64
#define strtoadr _strtoui64
#else
#define strtoadr strtoul
#endif

int main()
{
 XPRMmodel mod;
 XPRMdsolib dso;
 XPRMalltypes rvalue;
 XPRSprob prob;
 int result, ncol, len, i;
 double *sol, val;
 char *names, *onecol;

 if(XPRMinit())                         /* Initialize Mosel */
  return 1;

 if(XPRMexecmod(NULL,"burglar4.mos",NULL,&result,&mod))
  return 2;                             /* Execute a model file */

  /* Retrieve the pointer to the problem loaded in the Xpress Optimizer */
 if((dso=XPRMfinddso("mmxprs"))==NULL)
  return 4;             
 if(XPRMgetdsoparam(mod, dso, "xprs_problem", &result, &rvalue))
  return 5;
 prob=(XPRSprob)strtoadr(rvalue.ref,NULL,0);
  
 XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE);  /* Set sense to maximization */
 if(XPRSmipoptimize(prob, ""))              /* Solve the problem */
  return 6;

 if(XPRSgetintattrib(prob, XPRS_MIPSTATUS, &result))
  return 7;                             /* Test whether a solution is found */

 if((result==XPRS_MIP_SOLUTION) || (result==XPRS_MIP_OPTIMAL))
 {
  if(XPRSgetdblattrib(prob, XPRS_MIPOBJVAL, &val))
   return 8;  
  printf("Objective value: %g\n", val); /* Print the objective function value */
  if(XPRSgetintattrib(prob, XPRS_INPUTCOLS, &ncol))
   return 9; 
  if((sol = (double *)malloc(ncol * sizeof(double)))==NULL)
   return 10;
  if(XPRSgetsolution(prob, NULL, sol, 0, ncol-1))
   return 11;                           /* Get the primal solution values */
  if(XPRSgetnamelist(prob, 2, NULL, 0, &len, 0, ncol-1))
   return 11;                           /* Get the name array length */
  if((names = (char *)malloc(len*sizeof(char)))==NULL)
   return 12;
  if(XPRSgetnamelist(prob, 2, names, len, NULL, 0, ncol-1))
   return 13;                           /* Get the variable names */
  onecol = names;
  for(i=0; i<ncol; i++) {               /* Print out the solution */
    printf("%s: %g\n", onecol, sol[i]);
    onecol = onecol+strlen(onecol)+1;
  }   
  free(names);
  free(sol); 
 }
 
 return 0;
}


/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioobj.c
  ```````````````
  Accessing model results.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main(int argc, char *argv[])
{
  XPRMmodel model;
  XPRMalltypes rvalue, itemname;
  XPRMarray varr;
  XPRMmpvar x;
  XPRMset set;
  int result, indices[1], type;
  
  XPRMinit();                          /* Initialize Mosel */
                                       /* Execute = compile/load/run a model */
  XPRMexecmod(NULL, "foliodata.mos", NULL, &result, &model);

                                       /* Test whether a solution is found 
				          and print the objective value */
  if((XPRMgetprobstat(model)&XPRM_PBRES)==XPRM_PBOPT)
    printf("Objective value: %g\n", XPRMgetobjval(model));


                                       /* Retrieve the decision variables */
  type=XPRMfindident(model,"frac",&rvalue);   /* Get model object 'frac' */
  if((XPRM_TYP(type)!=XPRM_TYP_MPVAR)||       /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))           /* it must be an `mpvar' array */
    return 1;
  varr = rvalue.array;

                                       /* Retrieve the index names */
  type = XPRMfindident(model,"SHARES",&rvalue); /* Get model object 'SHARES'*/
  if((XPRM_TYP(type)!=XPRM_TYP_STRING)||      /* Check the type: */
     (XPRM_STR(type)!=XPRM_STR_SET))          /* it must be a set of strings */
    return 2;
  set = rvalue.set;

  XPRMgetfirstarrentry(varr, indices); /* Get the first entry of array varr
                                          (we know that the array is dense 
                                          and has a single dimension) */
  do                                   /* Print out the solution */
  {
   XPRMgetarrval(varr,indices,&x);            /* Get a variable from varr */
   printf("%s:\t%g%%\n", XPRMgetelsetval(set, indices[0], &itemname)->string,
          XPRMgetvsol(model,x)*100);          /* Print the solution value */
  } while(!XPRMgetnextarrentry(varr, indices));  /* Get the next index */

  return 0;
}

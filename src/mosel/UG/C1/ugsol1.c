/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugsol1.c
   `````````````
   Accessing modeling objects and solution information.
   Running the BIM file.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2001, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue, itemname;
 XPRMarray varr, darr;
 XPRMmpvar x;
 XPRMset set;
 int indices[1], result, type;
 double val;

 if(XPRMinit())                         /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("burglar3.bim",NULL))==NULL)  /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))       /* Run the model (includes 
                                           optimization) */
  return 3;

 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 4;                             /* Test whether a solution is found */

 printf("Objective value: %g\n", XPRMgetobjval(mod));
                                        /* Print the objective function value */

 type=XPRMfindident(mod,"take",&rvalue);     /* Get the model object 'take' */
 if((XPRM_TYP(type)!=XPRM_TYP_MPVAR)||       /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))          /* it must be an `mpvar' array */
  return 5;
 varr = rvalue.array;

 type=XPRMfindident(mod,"VALUE",&rvalue);    /* Get the model object 'VALUE' */
 if((XPRM_TYP(type)!=XPRM_TYP_REAL)||        /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))          /* it must be an array of reals */
  return 6;
 darr = rvalue.array;

 type = XPRMfindident(mod,"ITEMS",&rvalue);  /* Get the model object 'ITEMS' */
 if((XPRM_TYP(type)!=XPRM_TYP_STRING)||      /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_SET))          /* it must be a set of strings */
  return 7;
 set = rvalue.set;

 XPRMgetfirstarrentry(varr, indices);     /* Get the first entry of array varr
                                             (we know that the array is dense 
                                             and has a single dimension) */
 do
 {
  XPRMgetarrval(varr,indices,&x);            /* Get a variable from varr */
  XPRMgetarrval(darr,indices,&val);          /* Get the corresponding value */
  printf("take(%s): %g\t (item value: %g)\n", XPRMgetelsetval(set, indices[0], 
         &itemname)->string, XPRMgetvsol(mod,x), val); 
                                             /* Print the solution value */
 } while(!XPRMgetnextarrentry(varr, indices));  /* Get the next index */

 XPRMresetmod(mod);
 
 return 0;
}


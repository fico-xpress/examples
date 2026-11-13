/********************************************************
   Mosel Example Problems
   ====================== 

   file dataafterexec.c
   ````````````````````
   Retrieving data from a model after its execution.
   - Sparse data format -
   
   (c) 2008 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue; 
 XPRMarray dataout;
 int result, type, indices[2];
 double val;

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

 if(XPRMexecmod(NULL, "data_out.mos", NULL, &result, &mod))
  return 2;                       /* Execute a model file */

 type=XPRMfindident(mod, "A", &rvalue);  /* Get the model object 'A' */
 if((XPRM_TYP(type)!=XPRM_TYP_REAL)||    /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))      /* it must be an array of reals */
  return 3;
 dataout = rvalue.array;
                                  /* Print out the values */
 XPRMgetfirstarrtruentry(dataout, indices);  /* Get the first array entry */
 do
 {
  XPRMgetarrval(dataout, indices, &val);     /* Get the corresponding value */
  printf(" A(%d,%d): %g\n", indices[0], indices[1], val); 
 } while(!XPRMgetnextarrtruentry(dataout, indices));  /* Get the next index */
 
 XPRMresetmod(mod);
 
 return 0;
}

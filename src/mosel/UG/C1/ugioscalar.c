/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugioscalar.c
   `````````````````
   Exchanging data between model and host application.
   - Scalars -
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, Mar. 2008, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int wmax=100;
int numitem;
double objval;

int main()
{
 XPRMmodel mod;
 int result;
 char wmax_name[40];              /* File name of input data 'wmax' */
 char num_name[40];               /* File name of output data 'num' */
 char solution_name[40];          /* File name of solution value */
 char params[160];                /* Parameter string for model execution */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */

 sprintf(wmax_name, "mem:%p/%d", &wmax, (int)sizeof(wmax));
 sprintf(num_name, "mem:%p/%d", &numitem, (int)sizeof(numitem));
 sprintf(solution_name, "mem:%p/%d", &objval, (int)sizeof(objval));

                           /* Pass file names as execution param.s */
 sprintf(params, "WMAX='%s',NUM='%s',SOLVAL='%s'", wmax_name, num_name,
         solution_name);

 if(XPRMexecmod(NULL, "burglar12.mos", params, &result, &mod))
  return 2;                       /* Execute a model file */
 
 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                       /* Test whether a solution is found */

/* Display solution values obtained from the model */
 printf("Objective value: %g\n", objval);
 printf("Total number of items: %d\n", numitem);

 XPRMresetmod(mod);
 
 return 0;
}


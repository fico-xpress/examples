/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugiodense.c
   ````````````````
   Exchanging data between model and host application.
   - Dense data -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

double vdata[8]={15,100,90,60,40,15,10, 1};   /* Input data: VALUE */
double wdata[8]={ 2, 20,20,30,40,30,60,10};   /* Input data: WEIGHT */
double solution[8];               /* Array for solution values */

int main()
{
 XPRMmodel mod;
 int i,result;
 char vdata_name[40];             /* File name of input data 'vdata' */
 char wdata_name[40];             /* File name of input data 'wdata' */
 char solution_name[40];          /* File name of solution values */
 char params[144];                /* Parameter string for model execution */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */
/* Options for 'raw':                                                       */
/* 'noindex':   only array values are expected - no indices requested       */

 sprintf(vdata_name, "noindex,mem:%p/%d", vdata, (int)sizeof(vdata));
 sprintf(wdata_name, "noindex,mem:%p/%d", wdata, (int)sizeof(wdata));
 sprintf(solution_name, "noindex,mem:%p/%d", solution, (int)sizeof(solution));

                                  /* Pass file names as execution param.s */
 sprintf(params, "VDATA='%s',WDATA='%s',SOL='%s'", vdata_name, wdata_name,
         solution_name);

 if(XPRMexecmod(NULL, "burglar6.mos", params, &result, &mod))
  return 2;                       /* Execute a model file */
 
 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                       /* Test whether a solution is found */

/* Display solution values obtained from the model */
 printf("Objective value: %g\n", XPRMgetobjval(mod));
 for(i=0;i<8;i++)
  printf(" take(%d): %g\n", i+1, solution[i]);

 XPRMresetmod(mod);               /* Reset the model */
 
 return 0;
}


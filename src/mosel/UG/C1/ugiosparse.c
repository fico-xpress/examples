/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugiosparse.c
   `````````````````
   Exchanging data between model and host application.
   - Sparse data (string indices) -
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

const struct
{                                 /* Initial values for array 'data': */
 const char *ind;                 /*   index name */
 double val,wght;                 /*   value and weight data entries */
} data[]={{"camera",15,2}, {"necklace",100,20}, {"vase",90,20}, 
          {"picture",60,30}, {"tv",40,40}, {"video",15,30}, 
          {"chest",10,60}, {"brick",1,10}};

struct
{                                 /* Array to receive solution values: */
 const char *ind;                 /*   index name */
 double val;                      /*   solution value */
} solution[8];

int main()
{
 XPRMmodel mod;
 int i,result;
 char data_name[40];              /* File name of input data 'data' */
 char solution_name[40];          /* File name of solution values */
 char params[96];                 /* Parameter string for model execution */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */
/* Options for 'raw':                                                       */
/* 'slength=0': strings are represented by pointers to null terminated      */
/*              arrays of characters (C-string) instead of fixed size arrays*/

 sprintf(data_name, "slength=0,mem:%p/%d", data, (int)sizeof(data));
 sprintf(solution_name, "slength=0,mem:%p/%d", solution, (int)sizeof(solution));

                                  /* Pass file names as execution param.s */
 sprintf(params, "DATA='%s',SOL='%s'", data_name, solution_name);

 if(XPRMexecmod(NULL, "burglar7.mos", params, &result, &mod))
  return 2;                       /* Execute a model file */
 
 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                       /* Test whether a solution is found */

/* Display solution values obtained from the model */
 printf("Objective value: %g\n", XPRMgetobjval(mod));
 for(i=0;i<8;i++)
  printf(" take(%s): %g\n", solution[i].ind, solution[i].val);

 XPRMresetmod(mod);
 
 return 0;
}


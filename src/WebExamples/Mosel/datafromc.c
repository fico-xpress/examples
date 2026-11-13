/********************************************************
   Mosel Example Problems
   ======================

   file datafromc.c
   ````````````````
   Exchanging data between model and host application
   using IO drivers.
   - Sparse data format -
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S.Heipcke, Mar. 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

const struct
{                                 /* Initial values for array 'data': */
 int index_i,index_j;             /*   index names */
 double value;                    /*   data value */
} data[]={{1,1,12.5}, {2,3,5.6}, {10,9,-7.1}, {3,2,1}};

int main()
{
 XPRMmodel mod;
 int result;
 char data_name[40];              /* File name of input data 'data' */
 char params[96];                 /* Parameter string for model execution */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */

 sprintf(data_name, "mem:%p/%d", data, (int)sizeof(data));
                                  /* Pass file name as execution param. */
 sprintf(params, "DATA='%s'", data_name);

 if(XPRMexecmod(NULL, "datainputc.mos", params, &result, &mod))
  return 2;                       /* Execute a model file */
 
 XPRMresetmod(mod);
 
 return 0;
}

/********************************************************
   Mosel Example Problems
   ======================

   file datatoc.c
   ``````````````
   Exchanging data between model and host application
   using IO drivers.
   - Sparse data format -
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, Mar. 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

struct
{                                 /* Array to receive output values: */
 int index1,index2;               /*   index names */
 double value;                    /*   data value */
} dataout[9];

int main()
{
 XPRMmodel mod;
 int i,result;
 char dout_name[40];              /* File name of output data */
 char params[96];                 /* Parameter string for model execution */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */

 sprintf(dout_name, "mem:%p/%d", dataout, (int)sizeof(dataout));
                                  /* Pass file name as execution param. */
 sprintf(params, "DOUT='%s'", dout_name);

 if(XPRMexecmod(NULL, "dataoutputc.mos", params, &result, &mod))
  return 2;                       /* Execute a model file */

 for(i=0;i<9;i++)                 /* Print out the values we have read */
  printf(" A(%d,%d): %g\n", dataout[i].index1, dataout[i].index2,
         dataout[i].value);
 
 XPRMresetmod(mod);
 
 return 0;
}

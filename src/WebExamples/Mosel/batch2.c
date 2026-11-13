/*******************************************************
   Mosel Example Problems 
   ======================

   file batch2.c
   `````````````
   Looping over program executions coupled with 
   parameterized data in- and output.
       
   (c) 2008 Fair Isaac Corporation
       author: S.Heipcke, Oct. 2002, rev. July 2026
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"

int main()
{
 int i, result;
 char params[60];
 XPRMmodel mod;

 XPRMinit();                            /* Initialize Mosel */

 XPRMcompmod(NULL,"batch2.mos",NULL,"");/* Compile the model batch2.mos */
 mod=XPRMloadmod("batch2.bim",NULL);    /* Load the BIM file */
 XPRMrunmod(mod,&result,NULL);          /* Run the model */

 for(i=1;i<=2;i++)
 {
  sprintf(params,"INFILE=batchdata%d.dat, OUTFILE=batchdata%d.dat, LIM=%d",
    i,i+1, 100+i*10);
  XPRMrunmod(mod,&result,params);      /* Re-run the model with other data */
 }

 return 0;
}

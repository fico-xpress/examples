/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugdefstream.c
   ``````````````````
   Redirecting model output.
   
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Apr. 2013, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"


int main()
{
 int result;

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;
  
/* Uncomment one of the following: */
                                  /* Disable model output */
/* XPRMsetdefstream(NULL, XPRM_F_WRITE, "null:"); */

                                  /* Double model output (file+stdout) */
 XPRMsetdefstream(NULL, XPRM_F_WRITE, "tee:burgres.txt&");

                                  /* Execute = compile/load/run a model */
 if(XPRMexecmod(NULL, "burglar2.mos", NULL, &result, NULL))
  return 2;

 printf("`burglar2' returned: %d\n", result);

 return 0;
}


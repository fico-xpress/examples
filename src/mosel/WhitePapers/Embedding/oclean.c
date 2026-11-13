/*********************************************************
   Mosel Example Problems
   ====================== 

   file oclean.c
   `````````````
   Run the Office cleaning problem with different 
   algorithm settings.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
*********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 int result;

 if(XPRMinit())                         /* Initialize Mosel */
  return 1;
                                        /* Execute a model file */
 if(XPRMexecmod(NULL, "cleana.mos", NULL, &result, &mod))
  return 2;

 if(XPRMexecmod(NULL, "cleana.mos", "ALG=1", &result, &mod))
  return 3;

 if(XPRMexecmod(NULL, "cleana.mos", "ALG=2", &result, &mod))
  return 4;

 if(XPRMexecmod(NULL, "cleana.mos", "ALG=3", &result, &mod))
  return 5;

 if(XPRMexecmod(NULL, "cleana.mos", "ALG=4", &result, &mod))
  return 6;

 if(XPRMexecmod(NULL, "cleana.mos", "ALG=5", &result, &mod))
  return 7;

 if(XPRMexecmod(NULL, "cleana.mos", "ALG=6", &result, &mod))
  return 8;

 return 0;
}

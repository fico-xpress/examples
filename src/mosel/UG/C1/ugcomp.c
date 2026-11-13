/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcomp.c
   `````````````
   Compiling a model into a BIM file.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2001, rev. July 2026
********************************************************/

#include <stdlib.h>
#include "xprm.h"

int main()
{
 if(XPRMinit())                       /* Initialize Mosel */
  return 1;

 if(XPRMcompmod(NULL, "burglar2.mos", NULL, "Knapsack example"))
  return 2;                           /* Compile the model burglar2.mos,
                                         output the file burglar2.bim */
 return 0;
}

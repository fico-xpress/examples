/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcomptmp.c
   ````````````````
   Compiling a model into a BIM file saved in Mosel's
   temporary directory, then load and run it.
   
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Apr. 2013, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 int result;

 if(XPRMinit())                       /* Initialize Mosel */
  return 1;

 if(XPRMcompmod(NULL, "burglar2.mos", "tmp:burglar2.bim", "Knapsack example"))
  return 2;                           /* Compile the model burglar2.mos,
                                         output the file burglar2.bim 
					 in Mosel's temporary directory */

 if((mod=XPRMloadmod("tmp:burglar2.bim",NULL))==NULL)  /* Load the BIM file */
  return 3;
 
 if(XPRMrunmod(mod,&result,NULL))     /* Run the model */
  return 4;

 return 0;
}


/*******************************************************
   Mosel Example Problems 
   ======================

   file exitvalue.c
   ````````````````
   Example of the use of `exit' for error handling in C.
       
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, Feb. 2002, rev. July 2026
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"

int main()
{
 int result;
 XPRMmodel mod;

 XPRMinit();

 XPRMcompmod(NULL,"exitvalue.mos",NULL,""); /* Compile the model */
 mod=XPRMloadmod("exitvalue.bim",NULL);     /* Load the BIM file */
 XPRMrunmod(mod,&result,NULL);              /* Run the model */

 switch(result)
 {
  case 0: printf("Integer solution: %g\n", XPRMgetobjval(mod)); break;
  case 1: printf("LP infeasible\n"); break;
  case 2: printf("No integer solution\n"); break;
 }

 return 0;
}

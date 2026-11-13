/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugrun.c
   ````````````
   Executing a BIM file.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2001, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 int result;

 if(XPRMinit())                       /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("burglar2.bim",NULL))==NULL)  /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))     /* Run the model */
  return 3;

 return 0;
}


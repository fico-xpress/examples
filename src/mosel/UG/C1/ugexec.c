/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugexec.c
   `````````````
   Execute (compile/load/run) a model.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 int result;

 if(XPRMinit())                       /* Initialize Mosel */
  return 1;
                                      /* Execute = compile/load/run a model */
 if(XPRMexecmod(NULL,"burglar2.mos",NULL,&result,NULL))
  return 2;

 return 0;
}


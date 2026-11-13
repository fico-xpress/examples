/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugparam2.c
   ```````````````
   Passing parameters to a Mosel program.
   Executing a model file.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue, setitem;
 XPRMset set;
 int result, type, i, size, first, last;
 int LIM=500;
 char params[128];

 if(XPRMinit())                         /* Initialize Mosel */
  return 1;

 sprintf(params, "LIMIT=%d", LIM);

 if(XPRMexecmod(NULL,"prime2.mos",params,&result,&mod))
  return 2;                             /* Execute the model */

 type=XPRMfindident(mod,"SPrime",&rvalue);   /* Get the object 'SPrime' */
 if((XPRM_TYP(type)!=XPRM_TYP_INT)||    /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_SET))     /* it must be a set of integers */
  return 3;
 set = rvalue.set;

 size = XPRMgetsetsize(set);            /* Get the size of the set */
 if(size>0)
 {
  first = XPRMgetfirstsetndx(set);      /* Get the number of the first index */
  last = XPRMgetlastsetndx(set);        /* Get the number of the last index */
  printf("Prime numbers from 2 to %d:\n", LIM);
  for(i=first;i<=last;i++)              /* Print all set elements */
   printf(" %d,",XPRMgetelsetval(set,i,&setitem)->integer);
  printf("\n");  
 }

 return 0;
}


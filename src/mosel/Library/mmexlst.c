/********************************************************
   Mosel Library Examples
   ====================== 

   file mmexlst.c
   ``````````````
   Accessing modeling objects 
   (enumerating the elements of a list).
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2007, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMalltypes value;
 mm_list lst;
 int i,result,type,ct;
 void *ref;

 i=XPRMinit();
 if((i!=0)&&(i!=32))       /* Initialize Mosel */
  return 1;
                           /* Disable output from Mosel (make model silent) */
 XPRMsetdefstream(NULL, XPRM_F_WRITE, "null:");  
                           /* Execute = compile/load/run a model */
 if(XPRMexecmod(NULL, "Models/euler.mos", NULL, &result, &mod))
  return 2;

                           /* Get the model object named 'TOUR' */
 type=XPRMfindident(mod, "TOUR", &value);      
                           /* Check the type: it must be a list of integers */
 if((XPRM_STR(type)!=XPRM_STR_LIST)||(XPRM_TYP(type)!=XPRM_TYP_INT))
   return 3; 

 lst=value.list;           /* Retrieve the list */
 ref=NULL; ct=0;
 printf("Tour: ");         /* Print out all list elements */
 while((ref=XPRMgetnextlistelt(lst, ref, &type, &value))!=NULL)
  if (++ct<XPRMgetlistsize(lst))  
   printf("%d -> ", value.integer);
  else
   printf("%d\n", value.integer); 

 return 0;
}


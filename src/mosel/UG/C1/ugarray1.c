/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugarray1.c
   ```````````````
   Accessing modeling objects (sparse arrays).
   Running the BIM file.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2001, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue;
 XPRMarray varr;
 XPRMset *sets;
 int *indices, dim, result, type, i;

 if(XPRMinit())                          /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("transport.bim",NULL))==NULL)  /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))        /* Run the model */
  return 3;

 type=XPRMfindident(mod,"flow",&rvalue); /* Get the model object named 'flow' */
 if((XPRM_TYP(type)!=XPRM_TYP_MPVAR)||   /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))      /* it must be an array of unknowns */
  return 4;
 varr=rvalue.array;

 dim = XPRMgetarrdim(varr);              /* Get the number of dimensions of 
                                            the array */
 indices = (int *)malloc(dim*sizeof(int));
 sets = (XPRMset *)malloc(dim*sizeof(XPRMset)); 

 XPRMgetarrsets(varr,sets);              /* Get the indexing sets  */
 XPRMgetfirstarrtruentry(varr,indices);  /* Get the first true index tuple */
 do
 {
  printf("flow(");
  for(i=0;i<dim-1;i++)
   printf("%s,",XPRMgetelsetval(sets[i],indices[i],&rvalue)->string);
  printf("%s), ",XPRMgetelsetval(sets[dim-1],indices[dim-1],&rvalue)->string);
 } while(!XPRMgetnextarrtruentry(varr,indices));  /* Get next true index tuple*/
 printf("\n");

 free(sets);
 free(indices);
 XPRMresetmod(mod);
 
 return 0;
}


/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexas.c                                       */
/*  `````````````                                       */
/*  Example for the use of the Mosel libraries          */
/*  (using arrays with index sets: different ways       */
/*   of enumerating arrays)                             */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2001, rev. July 2026        */
/********************************************************/

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

 i=XPRMinit();
 if((i!=0)&&(i!=32))                      /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("Models/trans.bim",NULL))==NULL)  /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))         /* Run the model */
  return 3;

 type=XPRMfindident(mod,"x",&rvalue);     /* Get the model object named 'x' */
 if((XPRM_TYP(type)!=XPRM_TYP_MPVAR)||      /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))         /* it must be an array of unknowns */
  return 4;
 varr=rvalue.array;
 
 dim = XPRMgetarrdim(varr);               /* Get the number of dimensions of 
                                           the array */
 indices = (int *)malloc(dim*sizeof(int));
 sets = (XPRMset *)malloc(dim*sizeof(XPRMset)); 

 XPRMgetarrsets(varr,sets);               /* Get the indexing sets  */
   
 printf("\n1. Logic entries:\n");
 XPRMgetfirstarrentry(varr,indices);         /* Get the first index tuple */
 do
 {
  printf("x(");
  for(i=0;i<dim-1;i++)
   printf("%s,", XPRMgetelsetval(sets[i],indices[i],&rvalue)->string);
  printf("%s), ",XPRMgetelsetval(sets[dim-1],indices[dim-1],&rvalue)->string);
 } while(!XPRMgetnextarrentry(varr,indices));  /* Get the next index tuple */ 

 type = XPRMgetarrtype(varr);                   /* Get the type of the array */

 if(XPRM_ARR_IS_SPARSE(type))           /* There would be no difference to
                                           the first way of enumerating 
                                           in the case of a dense array */
 { 
  printf("\n\n2. True entries:\n");
  XPRMgetfirstarrtruentry(varr,indices);   /* Get the first true index tuple */
  do
  {
   printf("x(");
   for(i=0;i<dim-1;i++)
    printf("%s,",XPRMgetelsetval(sets[i],indices[i],&rvalue)->string);
   printf("%s), ",XPRMgetelsetval(sets[dim-1],indices[dim-1],&rvalue)->string);
  } while(!XPRMgetnextarrtruentry(varr,indices));/* Get next true index tuple*/
 } 
 printf("\n");

 free(sets);
 free(indices);
 XPRMresetmod(mod);
 
 return 0;
}

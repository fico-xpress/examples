/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexarr.c                                      */
/*  ``````````````                                      */
/*  Example for the use of the Mosel libraries          */
/*  (accessing arrays in Mosel)                         */
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
 XPRMmpvar lv;
 int *indices, dim, size, result, type, i;
 int ind1[] = {-1,0,6}, ind2[] = {-1,2,-5};

 i=XPRMinit();
 if((i!=0)&&(i!=32))                      /* Initialize Mosel */
  return 1;

 /**** Compare two given index tuples independent of any model ****/ 
 if(XPRMcmpindices(3, ind1, ind2)<0)      /* Compare two index tuples */
  printf("The tuple (%d,%d,%d) comes before (%d,%d,%d).\n", 
         ind1[0], ind1[1], ind1[2], ind2[0], ind2[1], ind2[2]);

 /**** Load and run a model ****/
 if((mod=XPRMloadmod("Models/trans.bim",NULL))==NULL)  /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))         /* Run the model */
  return 3;

 type=XPRMfindident(mod,"x",&rvalue);     /* Get the model object named 'x' */
 if((XPRM_TYP(type)!=XPRM_TYP_MPVAR)||    /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))       /* it must be an array of unknowns */
  return 4;
 varr=rvalue.array;
 
 size = XPRMgetarrsize(varr);             /* Get the array size */

 dim = XPRMgetarrdim(varr);               /* Get the number of dimensions of 
                                           the array */
 printf("\nArray size: %d true elements, %d dimensions\n", size, dim);
 
 indices = (int *)malloc(dim*sizeof(int));

 XPRMgetfirstarrentry(varr,indices);      /* Get the first index tuple */
 printf("First array entry: x(%d", indices[0]);
 for(i=1;i<dim;i++) printf(",%d", indices[i]);

 XPRMgetlastarrentry(varr,indices);       /* Get the last index tuple */
 printf(");  last array entry: x(%d", indices[0]);
 for(i=1;i<dim;i++) printf(",%d", indices[i]);
 printf(")\n");

 for(i=0;i<dim;i++) indices[i]=2;
 if(XPRMchkarrind(varr,indices)==0)     /* Check whether a tuple lies within */
 {                                      /* the range defined for the array   */
  printf("The tuple (%d", indices[0]);
  for(i=1;i<dim;i++) printf(",%d", indices[i]);
  printf(") lies within the range of the array");
  XPRMgetarrval(varr, indices, &lv);      /* Get the array entry corresponding
                                           to the index tuple: it may be NULL
                                           in the case of a sparse array! */
  if(lv==NULL) 
   printf("\nbut the array entry is not defined (sparse array!).\n");
  else printf(".\n");
 }

 free(indices);
 XPRMresetmod(mod);
 
 return 0;
}

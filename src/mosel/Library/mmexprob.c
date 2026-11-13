/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexprob.c                                     */
/*  ```````````````                                     */
/*  Example for the use of the Mosel libraries          */
/*  (accessing problems and solution information)       */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2001, rev. July 2026        */
/********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue;
 XPRMarray varr, darr;
 XPRMmpvar x;
 XPRMlinctr lgrade;
 double cost;
 int result, type;
 int indices[1],i;

 i=XPRMinit();
 if((i!=0)&&(i!=32))                                   /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("Models/blend2.bim",NULL))==NULL) /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))         /* Run the model (it includes 
                                             optimization) */
  return 3;

 XPRMexportprob(mod,"p","blend",NULL);    /* Export problem to a file in LP 
                                             format (maximization) */

 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 5;                             /* Test whether a solution is found */

 printf("Objective value: %g\n", XPRMgetobjval(mod));
                                        /* Print the objective function value */

 type=XPRMfindident(mod,"x",&rvalue);     /* Get the model object named 'x' */
 if((XPRM_TYP(type)!=XPRM_TYP_MPVAR)||    /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))       /* it must be an array of unknowns */
  return 5;
 varr=rvalue.array;

 type=XPRMfindident(mod,"COST",&rvalue);  /* Get the model object 'COST' */
 if((XPRM_TYP(type)!=XPRM_TYP_REAL)||     /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_ARR))       /* it must be an array of reals */
  return 6;
 darr=rvalue.array;
   
 XPRMgetfirstarrentry(varr, indices);  /* Get the first entry of the array varr
                                          (we know that the array is dense 
                                          and has a single dimension) */
 do
 {
  XPRMgetarrval(varr,indices,&x);         /* Get the a variable from varr */
  XPRMgetarrval(darr,indices,&cost);      /* Get the corresponding cost value */
  printf("x(%d)=%g (COST: %g, rcost=%g, col=%d)\n",indices[0],
   XPRMgetvsol(mod,x), cost, XPRMgetrcost(mod,x),
   XPRMgetvarnum(mod,x));                 /* Print the solution value */
 } while(!XPRMgetnextarrentry(varr, indices));  /* Get the next index */
 
 type=XPRMfindident(mod,"LoGrade",&rvalue);/* Get the model object 'LoGrade' */
 if((XPRM_TYP(type)!=XPRM_TYP_LINCTR)||    /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_REF))        /* it must be a reference to a linear
                                              constraint */
  return 7;
 lgrade=rvalue.linctr;

 printf("LoGrade: activity=%g, dual=%g, value=%g, slack=%g, row=%d\n",
   XPRMgetact(mod,lgrade), XPRMgetdual(mod,lgrade),
   XPRMgetcsol(mod,lgrade), XPRMgetslack(mod,lgrade),
   XPRMgetctrnum(mod,lgrade));
                                      /* Print the activity and dual values */

 XPRMresetmod(mod);
 XPRMremovetmpdir();
 return 0;
}

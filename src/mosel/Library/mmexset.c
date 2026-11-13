/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexset.c                                      */
/*  ``````````````                                      */
/*  Example for the use of the Mosel libraries          */
/*  (accessing sets in Mosel)                           */
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
 XPRMset set;
 int result,type,first,last,i,size;
 char *setname;
 XPRMalltypes itemname;

 i=XPRMinit();
 if((i!=0)&&(i!=32))                      /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("Models/burglari.bim",NULL))==NULL)  /* Load a BIM file */
  return 2;
 
 if(XPRMrunmod(mod,&result,NULL))         /* Run the model */
  return 3;

 setname="ITEMS";
 type = XPRMfindident(mod,setname,&rvalue);  /* Get the model object 'ITEMS' */
 if((XPRM_TYP(type)!=XPRM_TYP_STRING)||     /* Check the type: */
    (XPRM_STR(type)!=XPRM_STR_SET))         /* it must be a set of strings */
  return 4;

 set = rvalue.set;
  
 size = XPRMgetsetsize(set);             /* Get the size of the set */
 if(size>0)
 {
  first = XPRMgetfirstsetndx(set);       /* Get the number of the first index */
  last = XPRMgetlastsetndx(set);         /* Get the number of the last index */
  printf("Elements of set %s:\n",setname);
  for(i=first;i<=last;i++)              /* Print names of all set elements */
   printf(" %s,",XPRMgetelsetval(set,i,&itemname)->string);
  printf("\n");  
 }

 itemname.string="CD player";
 if(XPRMgetelsetndx(mod, set, &itemname)<0)
  printf("'%s' is not contained in '%s'.\n", itemname.string, setname); 
 
 return 0;
}


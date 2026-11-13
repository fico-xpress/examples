/********************************************************
   Mosel Example Problems
   ======================

   file paperio.c
   ``````````````
   Initializing a Mosel array from data stored in C
   using the I/O drivers, and retrieving data from the
   model.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"

/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue;
 XPRMmemblk *memblk;
 int result;
 char params[200];
 static int tabdemand[] = {150, 96,   48, 108,  227};
 static double tabwidth[] ={17, 21, 22.5,  24, 29.5};
 static double *tabsol;
 int i, type, sizesol;
 int nwidths = 5;

 if(XPRMinit()) return 1;

 /* Parameters: the addresses of the data tables and their sizes */
 sprintf(params, 
      "DDATA='noindex,mem:%p/%d',WDATA='noindex,mem:%p/%d',NWIDTHS=%d",
      tabdemand, (int)sizeof(tabdemand), tabwidth, 
      (int)sizeof(tabwidth), nwidths);

 /* Execute the model */
 if(XPRMexecmod("", "paperio.mos", params, &result, &mod)) return 2;

 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                             /* Test whether a solution is found */
  
 /* Retrieve solution data */
 type=XPRMfindident(mod,"soltab",&rvalue);     /* Get model object 'soltab' */
 if(XPRM_STR(type)!=XPRM_STR_MEM)              /* Check the type */
  return 4;
  
 memblk=rvalue.memblk;
 tabsol=(double *)(memblk->ref);
 sizesol=(int)(memblk->size/sizeof(double));

 /* Print out the solution */ 
 printf("Best integer solution: %g rolls\n", XPRMgetobjval(mod));
 printf("  Rolls per pattern: ");
 for(i=0;i<sizesol;i++) printf("%g, ", tabsol[i]);
 printf("\n");
 
 return result;
}

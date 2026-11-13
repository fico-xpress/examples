/********************************************************
   Mosel Example Problems
   ======================

   file paperio.c
   ``````````````
   Declaring a "static" module and initializing
   a Mosel array from data stored in C using the
   I/O drivers.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"
#include "xprm_ni.h"

/* Initialization function of the module 'cutstkdata' */
static int cutstkdata_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf);

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

 /* Register 'cutstkdata' as a static module (=stored in the program) */
 if(XPRMregstatdso("cutstkdata", cutstkdata_init)) return 2;

 /* Parameters: the addresses of the data tables and their sizes */
 sprintf(params, 
      "DDATA='noindex,mem:%p/%d',WDATA='noindex,mem:%p/%d',NWIDTHS=%d",
      tabdemand, (int)sizeof(tabdemand), tabwidth, 
      (int)sizeof(tabwidth), nwidths);

 /* Execute the model */
 if(XPRMexecmod("", "paperiom.mos", params, &result, &mod)) return 3;

 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 4;                             /* Test whether a solution is found */
  
 /* Retrieve solution data */
 type=XPRMfindident(mod,"soltab",&rvalue);     /* Get model object 'soltab' */
 if(XPRM_STR(type)!=XPRM_STR_MEM)              /* Check the type */
  return 5;
  
 memblk=rvalue.memblk;
 tabsol=(double *)(memblk->ref);
 sizesol=(int)(memblk->size/sizeof(double));
 
 printf("Best integer solution: %g rolls\n", XPRMgetobjval(mod));
 printf("  Rolls per pattern: ");
 for(i=0;i<sizesol;i++) printf("%g, ", tabsol[i]);
 printf("\n");
 
 return result;
}

/********************** Body of the module 'cutstkdata' ******************/

static int printpattern(XPRMcontext ctx,void *libctx);

static XPRMdsofct tabfct[]=
        {
         {"printpat",1002,XPRM_TYP_NOT,3,"rAI.iAI.r",printpattern}
        };

static XPRMdsointer dsointer=
        {
         0,NULL,
         sizeof(tabfct)/sizeof(XPRMdsofct),tabfct,
         0,NULL,
         0,NULL
        };

static XPRMnifct mm;             /* To store the mosel function table */

/*****************************************/
/* Initialization function of the module */
/*****************************************/
static int cutstkdata_init(XPRMnifct nifct, int *interver, int *libver,
		XPRMdsointer **interf)
/* The following header is required to compile 'cutstkdata' as a DSO file:
DSO_INIT cutstkdata_init(XPRMnifct nifct, int *interver, int *libver,
		XPRMdsointer **interf)
*/
{
 mm=nifct;                      /* Save the table of functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/******************************************************************/
/* Print the new pattern found:                                   */
/*  printpat(real, array(range) of integer, array(range) of real) */
/******************************************************************/
static int printpattern(XPRMcontext ctx,void *libctx)
{
 XPRMarray varr,warr;
 int v,index[1];
 double dj,w,tw=0;

 dj=XPRM_POP_REAL(ctx); 
 varr=XPRM_POP_REF(ctx);                /* The value array */
 warr=XPRM_POP_REF(ctx);                /* The widths array */

 mm->printf(ctx,"new pattern found with marginal cost %g\n", dj);
 mm->printf(ctx,"   Widths distribution: ");
 mm->getfirstarrentry(varr,index);      /* Get the first index tuple */
 do
 {
  mm->getarrval(varr,index,&v);
  mm->getarrval(warr,index,&w);
  mm->printf(ctx,"%g:%d  ",w,v);
  tw+=v*w;
 } while(!mm->getnextarrentry(varr,index));
 mm->printf(ctx,"Total width: %g\n", tw);
  
 return XPRM_RT_OK;
}


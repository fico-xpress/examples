/********************************************************
   Mosel Example Problems
   ======================

   file paper.c
   ````````````
   Declaring a "static" module and initializing
   a Mosel array from data stored in C.
   
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
 int result;
 char params[80];
 static int tabdemand[] = {150, 96,   48, 108,  227};
 static double tabwidth[] ={17, 21, 22.5,  24, 29.5};
 int nwidths = 5;

 if(XPRMinit()) return 1;

 /* Register 'cutstkdata' as a static module (=stored in the program) */
 if(XPRMregstatdso("cutstkdata", cutstkdata_init)) return 2;

 /* Parameters: the addresses of the data tables and their sizes */
 sprintf(params, "DDATA='%p',DSIZE=%d,WDATA='%p',WSIZE=%d,NWIDTHS=%d",
         tabdemand, (int)(sizeof(tabdemand)/sizeof(int)), tabwidth, 
         (int)(sizeof(tabwidth)/sizeof(double)), nwidths);

 /* Execute the model */
 if(XPRMexecmod("", "paperm.mos", params, &result, &mod)) return 3;
 
 return result;
}

/********************** Body of the module 'cutstkdata' ******************/

static int getcutstkdata_int(XPRMcontext ctx,void *libctx);
static int getcutstkdata_dbl(XPRMcontext ctx,void *libctx);
static int printpattern(XPRMcontext ctx,void *libctx);

static XPRMdsofct tabfct[]=
        {
         {"getcutstkdata",1000,XPRM_TYP_NOT,3,"AI.isi",getcutstkdata_int},
         {"getcutstkdata",1001,XPRM_TYP_NOT,3,"AI.rsi",getcutstkdata_dbl},
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

/************************************************************/
/* Initialize an array of integer with data held in C:      */
/*  getcutstkdata(array(range) of integer, string, integer) */
/************************************************************/
static int getcutstkdata_int(XPRMcontext ctx,void *libctx)
{
 XPRMarray arr;
 XPRMstring adr_s;
 XPRMset ndxset;
 int *adr,siz,index[1],last,i;

 arr=XPRM_POP_REF(ctx);             /* The array */
 adr_s=XPRM_POP_STRING(ctx);        /* Data location (as a string) */
 siz=XPRM_POP_INT(ctx);             /* Data size */
 sscanf(adr_s,"%p",&adr);           /* Get the address from the string */

 mm->getarrsets(arr,&ndxset);
 index[0]=mm->getfirstsetndx(ndxset);
 last=mm->getlastsetndx(ndxset);
 for(i=0;(i<siz) && (index[0]<=last);i++,index[0]++)
  mm->setarrvalint(ctx,arr,index,adr[i]);
 return XPRM_RT_OK;
}

/**********************************************************/
/* Initialize an array of double with data held in C:     */
/*  getcutstkdata(array(range) of real, string, integer)  */
/**********************************************************/
static int getcutstkdata_dbl(XPRMcontext ctx,void *libctx)
{
 XPRMarray arr;
 XPRMstring adr_s;
 XPRMset ndxset;
 int siz,index[1],last,i;
 double *adr;

 arr=XPRM_POP_REF(ctx);             /* The array */
 adr_s=XPRM_POP_STRING(ctx);        /* Data location (as a string) */
 siz=XPRM_POP_INT(ctx);             /* Data size */
 sscanf(adr_s,"%p",&adr);           /* Get the address from the string */

 mm->getarrsets(arr,&ndxset);
 index[0]=mm->getfirstsetndx(ndxset);
 last=mm->getlastsetndx(ndxset);
 for(i=0;(i<siz) && (index[0]<=last);i++,index[0]++)
  mm->setarrvalreal(ctx,arr,index,adr[i]);
 return XPRM_RT_OK;
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


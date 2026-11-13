/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmstatdso.c                                    */
/*  ````````````````                                    */
/*  Example for the use of the Mosel libraries          */
/*  (declaring a "static" module and initializing       */
/*   a Mosel array from data stored in C)               */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2002, rev. July 2026      */
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"
#include "xprm_ni.h"

/* Initialisation function of the module 'meminit' */
static int meminit_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf);

/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 int result;
 char params[80];
 static int tabinit[]= {23,78,45,90,234,111,900,68,110};


 if(XPRMinit())
  return 1;

 /* Register 'meminit' as a static module (=stored in the program) */
 if(XPRMregstatdso("meminit",meminit_init))
  return 2;

 /* Compile the model source */
 if(XPRMcompmod("","meminit.mos",NULL,NULL))
  return 3;
 
 /* Load the BIM file */
 if((mod=XPRMloadmod("meminit.bim",NULL))==NULL)
  return 4;

 /* parameters: the address of the data table and its size */
 sprintf(params,"MEMDAT='%p',MEMSIZ=%d",tabinit,(int)(sizeof(tabinit)/sizeof(int)));

 /* Run the model */
 if(XPRMrunmod(mod,&result,params))
  return 5;

 return result;
}

/***************************** Body of the module 'meminit' ******************/

static int mi_meminit(XPRMcontext ctx,void *libctx);
static XPRMdsofct tabfct[]=
        {
         {"meminit",1000,XPRM_TYP_NOT,3,"AI.isi",mi_meminit}
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
static int meminit_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf)
/* The following header is required to compile 'meminit' as a DSO file:
DSO_INIT meminit_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf)
*/
{
 mm=nifct;                      /* Save the table of functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/****************************************************/
/* Initialize an array with data in C:              */
/*  meminit(array(range) of integer,string,integer) */
/****************************************************/
static int mi_meminit(XPRMcontext ctx,void *libctx)
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


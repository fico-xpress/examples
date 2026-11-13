/********************************************************
   Mosel Example Problems
   ====================== 

   file datainout.c
   ````````````````
   Exchanging data between model and host application
   through a static module.
   - Sparse data format -
   
   (c) 2008 Fair Isaac Corporation
       author: S.Heipcke, Mar. 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprm.h"
#include "xprm_ni.h"

typedef struct
{                                 /* Initial values for array 'data': */
 int index_i,index_j;             /*   index names */
 double value;                    /*   data value */
} s_mydata;

static s_mydata data[]={{1,1,12.5}, {2,3,5.6}, {10,9,-7.1}, {3,2,1}};

s_mydata dataout[9];


/* Initialization function of the module 'datainout' */
static int datainout_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf);

/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 int result;
 char params[120];

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

 /* Register 'datainout' as a static module (=stored in the program) */
 if(XPRMregstatdso("datainout",datainout_init))
  return 2;

 /* Parameters: the address of the data tables and their size */
 sprintf(params,"INDATA='%p',INSIZE=%d,OUTDATA='%p',OUTSIZE=%d", 
         data,(int)(sizeof(data)/sizeof(s_mydata)), 
	 dataout,(int)(sizeof(dataout)/sizeof(s_mydata)));

 if(XPRMexecmod(NULL, "datainout.mos", params, &result, &mod))
  return 3;                       /* Execute the model file */

 return result;
}

/************************* Body of the module 'datainout' ******************/

static int dt_readdata(XPRMcontext ctx,void *libctx);
static int dt_writedata(XPRMcontext ctx,void *libctx);

static XPRMdsofct tabfct[]=
        {
         {"readdata",1000,XPRM_TYP_NOT,3,"AII.rsi",dt_readdata},
         {"writedata",1001,XPRM_TYP_NOT,3,"AII.rsi",dt_writedata}
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
static int datainout_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf)
/* The following header is required to compile 'datainout' as a DSO file:
DSO_INIT datainout_init(XPRMnifct nifct, int *interver,int *libver,
		XPRMdsointer **interf)
*/
{
 mm=nifct;                      /* Save the table of functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/*********************************************************/
/* Initialize an array with data in C:                   */
/*  readdata(array(range,range) of real,string,integer)  */
/*********************************************************/
static int dt_readdata(XPRMcontext ctx,void *libctx)
{
 XPRMarray arr;
 XPRMstring adr_s;
 s_mydata *adr;
 int indices[2],siz,i;

 arr=XPRM_POP_REF(ctx);             /* The array */
 adr_s=XPRM_POP_STRING(ctx);        /* Data location (as a string) */
 siz=XPRM_POP_INT(ctx);             /* Data size */
 sscanf(adr_s, "%p", &adr);         /* Get the address from the string */

 for(i=0;i<siz;i++)
 {
  indices[0]=adr[i].index_i;
  indices[1]=adr[i].index_j;
  mm->setarrvalreal(ctx, arr, indices, adr[i].value);
 }
 return XPRM_RT_OK;
}

/**********************************************************/
/* Initialize a C array with data from the model:         */
/*  writedata(array(range,range) of real,string,integer)  */
/**********************************************************/
static int dt_writedata(XPRMcontext ctx,void *libctx)
{
 XPRMarray arr;
 XPRMstring adr_s;
 s_mydata *adr;
 int indices[2],i,siz;

 arr=XPRM_POP_REF(ctx);             /* The array */
 adr_s=XPRM_POP_STRING(ctx);        /* Data location (as a string) */
 siz=XPRM_POP_INT(ctx);             /* Data size */
 sscanf(adr_s, "%p", &adr);         /* Get the address from the string */

 i=0;                               /* Print out values read from the model */
 if(!mm->getfirstarrtruentry(arr,indices))   /* Get first index tuple */
  do
  {                                 /* Get the array value */
   mm->getarrval(arr, indices, &(adr[i].value));
   adr[i].index_i=indices[0];       /* Get the indices */
   adr[i].index_j=indices[1];
   printf(" A(%d,%d): %g\n", adr[i].index_i, adr[i].index_j,
         adr[i].value);
   i++;	 
  } while(!mm->getnextarrtruentry(arr,indices) && i<siz);  /* Get next index */
 return XPRM_RT_OK;
}

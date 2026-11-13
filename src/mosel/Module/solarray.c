/******************************************
  Mosel NI Examples
  =================

  File solarray.c
  ```````````````
  Example module providing the procedure
    solarray(array of mpvar, array of real)
  for getting solutions into an array.

  (c) 2008 Fair Isaac Corporation
      author: S. Heipcke, 2002
*******************************************/

#include <stdlib.h>
#include "xprm_ni.h"

#define MAXDIM 20            /* Max. number of array dimensions */

/**** Function prototypes ****/
static int ar_getsol(mm_context ctx,void *libctx);

/**** Structures for passing info to Mosel ****/
/* Subroutines */
static mm_dsofct tabfct[]=
    {
     {"solarray",1000,XPRM_TYP_NOT,2,"A.vA.r",ar_getsol}
    };
	
/* Interface structure */
static XPRMdsointer dsointer= 
    { 
     0,NULL, sizeof(tabfct)/sizeof(XPRMdsofct),tabfct, 0,NULL, 0,NULL
    };

/**** Structures used by this module ****/
static XPRMnifct mm;         /* For storing Mosel NI function table */


/*******************************************************/
/* Initialize the module library just after loading it */
/*******************************************************/
DSO_INIT solarray_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                  /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;     /* Mosel NI version */
 *libver=XPRM_MKVER(0,0,1); /* Module version */
 *interf=&dsointer;         /* Pass info about module contents to Mosel */

 return 0;
}

/***************************/
/* Solution array function */
/***************************/
static int ar_getsol(XPRMcontext ctx,void *libctx)
{
 XPRMarray varr, solarr;
 XPRMset sets1[MAXDIM],sets2[MAXDIM];
 XPRMmpvar var;
 int dim, indices[MAXDIM], i;

/* Get variable and solution arrays from stack in the order that they are
   used as parameters for `getsol' */
 varr=XPRM_POP_REF(ctx);
 solarr=XPRM_POP_REF(ctx);

/* Compare sizes */
 dim = mm->getarrdim(varr);
 if(dim!=mm->getarrdim(solarr))
 {
  mm->dispmsg(ctx,"Solarray: Arrays have different number of dimensions.\n");
  return XPRM_RT_ERROR;
 }

 if(dim>MAXDIM)
 {
  mm->dispmsg(ctx,"Solarray: Maximum number of dimensions exceeded.\n");
  return XPRM_RT_ERROR;
 } 

/* Get the index sets */
 mm->getarrsets(varr,sets1);
 mm->getarrsets(solarr,sets2);
 for(i=0;i<dim;i++)
  if(sets1[i]!=sets2[i])
  {
   mm->dispmsg(ctx,"Solarray: Arrays have different index sets.\n");
   return XPRM_RT_ERROR;
  }
 
/* Get solution for all variables: since this uses the ...truentry functions,
   there is no need for any further testing */
 if(!mm->getfirstarrtruentry(varr,indices))
  do
  {
   mm->getarrval(varr,indices,&var);
   mm->setarrvalreal(ctx,solarr,indices,mm->getvsol(ctx,var));
  } while(!mm->getnextarrtruentry(varr,indices));

 return XPRM_RT_OK;
}

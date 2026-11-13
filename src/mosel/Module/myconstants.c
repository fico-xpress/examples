/******************************************
  Mosel NI Examples
  =================

  File myconstants.c
  ``````````````````
  Example module defining 
    constants
  of different types.

  (c) 2008 Fair Isaac Corporation
      author: S. Heipcke, 2002
*******************************************/

#include <stdlib.h>
#include "xprm_ni.h"

/**** Structures for passing info to Mosel ****/
/* Constants */
static const double tol=0.00001;

static XPRMdsoconst tabconst[]=
    {
     XPRM_CST_INT("MYCST_BIGM",10000),    /* A large integer value */
     XPRM_CST_REAL("MYCST_TOL",tol),      /* A tolerance value */
     XPRM_CST_STRING("MYCST_LINE",        /* String constant */
     "----------------------------------------------------------------"),
     XPRM_CST_BOOL("MYCST_FLAG",XPRM_TRUE),     /* Constant with value true */
     XPRM_CST_BOOL("MYCST_NOFLAG",XPRM_FALSE)   /* Constant with value false */
    };

/* Interface structure */
static XPRMdsointer dsointer= 
    { 
     sizeof(tabconst)/sizeof(XPRMdsoconst),tabconst, 0,NULL, 0,NULL, 0,NULL
    };

/*******************************************************/
/* Initialize the module library just after loading it */
/*******************************************************/
DSO_INIT myconstants_init(XPRMnifct nifct, int *interver,int *libver, 
  XPRMdsointer **interf)
{
 *interver=XPRM_NIVERS;     /* Mosel NI version */
 *libver=XPRM_MKVER(0,0,1); /* Module version */
 *interf=&dsointer;         /* Pass info about module contents to Mosel */

 return 0;
}

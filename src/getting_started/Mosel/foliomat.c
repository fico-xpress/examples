/********************************************************
  Mosel Library Example Problems
  ==============================

  file foliomat.c
  ```````````````
  Exporting a matrix.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main(int argc, char *argv[])
{
  int result, type;
  XPRMmodel model;
  XPRMalltypes rvalue;
  XPRMlinctr obj;
  
  XPRMinit();                        /* Initialize Mosel */
                                     /* Execute = compile/load/run a model */
  XPRMexecmod(NULL, "foliodata.mos", NULL, &result, &model);

                                     /* Retrieve a model object by its name */
  type = XPRMfindident(model, "Return", &rvalue);
  if((XPRM_TYP(type)!=XPRM_TYP_LINCTR)||  /* Check the type: */
     (XPRM_STR(type)!=XPRM_STR_REF))      /* it must be a reference to a linear
                                             constraint */
   return 1;
  obj = rvalue.linctr;               /* Store the objective function reference */

 /* Output the LP/MIP problem (or the portion of a problem that is specified 
  * via mpvar+linctr only, ignoring solver-specific extensions such as 
  * indicators or general constraints) */
  XPRMexportprob(model, "p", "folio", obj);

  return 0;
}

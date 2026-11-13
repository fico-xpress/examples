/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioparam.c
  `````````````````
  Parameterized model execution.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main(int argc, char *argv[])
{
  XPRMmodel model;
  int result;
  char params[128];
  
  XPRMinit();                        /* Initialize Mosel */

  sprintf(params, "OUTFILE=result2.dat,MAXRISK=0.4,MAXVAL=0.25");

                                     /* Execute = compile/load/run a model */  
  XPRMexecmod(NULL, "foliodata.mos", params, &result, &model);

  XPRMunloadmod(model);

  return 0;
}

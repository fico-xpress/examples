/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioexec.c
  ````````````````
  Executing a Mosel model.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main(int argc, char *argv[])
{
  int result;
  
  XPRMinit();                        /* Initialize Mosel */
                                     /* Execute = compile/load/run a model */
  XPRMexecmod(NULL, "foliodata.mos", NULL, &result, NULL);

  return 0;
}

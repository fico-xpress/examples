/********************************************************
  Mosel Library Example Problems
  ==============================

  file foliorun.c
  ```````````````
  Loading and running a BIM file.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Aug. 2003, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main(int argc, char *argv[])
{
  XPRMmodel model;
  int result;
  
  XPRMinit();                                  /* Initialize Mosel */
  model = XPRMloadmod("foliodata.bim", NULL);  /* Load compiled model */
  XPRMrunmod(model, &result, NULL);
  XPRMunloadmod(model);

  return 0;
}

/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexcomp.c                                     */
/*  ````````````'``                                     */
/*  Example for the use of the Mosel libraries          */
/*  (compile a model into a BIM file)                   */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2001, rev. July 2026        */
/********************************************************/

#include <stdlib.h>
#include "xprm.h"

int main()
{
 int i;

 i=XPRMinit();
 if((i!=0)&&(i!=32))                    /* Initialize Mosel */
  return 1;

 if(XPRMcompmod(NULL,"Models/burglari.mos",NULL,"Knapsack example"))
  return 2;                             /* Compile the model burglari.mos,
                                           output the file burglari.bim */
 return 0;
}

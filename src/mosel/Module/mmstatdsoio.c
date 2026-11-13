/********************************************************
   Mosel Library Examples
   ======================

   file mmstatdsoio.c
   ``````````````````
   Example for the use of the Mosel libraries
   (using I/O drivers instead of a static module for
    initializing a Mosel array from data stored in C)

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2005, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"


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

 /* Compile the model source */
 if(XPRMcompmod("", "meminitio.mos", NULL, NULL))
  return 2;
 
 /* Load the BIM file */
 if((mod=XPRMloadmod("meminitio.bim", NULL))==NULL)
  return 3;

 /* Parameters: the address of the data table and its size */
 sprintf(params, "MEMDAT='noindex,mem:%p/%u'", tabinit,
         sizeof(tabinit));

 /* Run the model */
 if(XPRMrunmod(mod, &result, params))
  return 4;

 return result;
}


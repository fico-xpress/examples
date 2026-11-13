/********************************************************
   Mosel Example Problems
   ======================

   file iodrvmem2.c
   ````````````````
   Using the IO driver `mem'.
   - Model source in separate .mos file -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2004, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"


/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 int result,i;
 char bimfile[2000];              /* Buffer to store BIM file */
 size_t bimfile_size;             /* Buffer to store actual size of BIM file */
 char bimfile_name[64];           /* File name of BIM file */

 i=XPRMinit();                    /* Initialize Mosel */
 if((i!=0)&&(i!=32))
  return 1;

/* Prepare file names for compilation using 'mem' driver: */
/*   "mem:base address/size[/actual size pointer]"        */
 bimfile_size=0;
 sprintf(bimfile_name, "mem:%p/%d/%p",
         bimfile, (int)sizeof(bimfile), &bimfile_size);

                                  /* Compile model file to memory */
 if(XPRMcompmod(NULL, "burglar2.mos", bimfile_name, "Knapsack example")) 
    return 2; 
 printf("BIM file uses %d bytes of memory.\n", (int)bimfile_size);

                                  /* Load a BIM file from memory */
 if((mod=XPRMloadmod(bimfile_name, NULL))==NULL)
  return 3;

 if(XPRMrunmod(mod, &result, NULL))  /* Run the model */
  return 4;

 return 0;
}


/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugcompmem.c
   ````````````````
   Compilation to memory.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 int result;
 char bimfile[2000];              /* Buffer to store BIM file */
 size_t bimfile_size;             /* Buffer to store actual size of BIM file */
 char bimfile_name[64];           /* File name of BIM file */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file name for compilation using 'mem' driver: */
/*   "mem:base_address/size[/actual_size_of_pointer]"    */
 bimfile_size=0;
 sprintf(bimfile_name, "mem:%p/%d/%p",
         bimfile, (int)sizeof(bimfile), &bimfile_size);

                                  /* Compile model file to memory */
 if(XPRMcompmod(NULL, "burglar2.mos", bimfile_name, "Knapsack example")) 
    return 2;
 printf("BIM file uses %lu bytes of memory.\n", (unsigned long)bimfile_size);

                                  /* Load a BIM file from memory */
 if((mod=XPRMloadmod(bimfile_name, NULL))==NULL)
  return 3;

 if(XPRMrunmod(mod, &result, NULL))  /* Run the model */
  return 4;

 XPRMresetmod(mod);
 
 return 0;
}


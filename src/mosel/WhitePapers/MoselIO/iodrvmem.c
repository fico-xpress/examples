/********************************************************
   Mosel Example Problems
   ======================

   file iodrvmem.c
   ```````````````
   Using the IO driver `mem'.
   - Model source included in application program -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2004, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

/*****************************************************/
/* The source of the model as an array of characters */
/*****************************************************/
const char source_of_model[]=
"model Burglar\n"
"uses 'mmxprs'\n"
 
"declarations\n"
" WTMAX = 102                    ! Maximum weight allowed\n"
" ITEMS = {'camera', 'necklace', 'vase', 'picture', 'tv', 'video', \n"
"          'chest', 'brick'}     ! Index set for items\n"
" VALUE: array(ITEMS) of real    ! Value of items\n"
" WEIGHT: array(ITEMS) of real   ! Weight of items\n"
" take: array(ITEMS) of mpvar    ! 1 if we take item i; 0 otherwise\n"
"end-declarations\n"

"VALUE::(['camera', 'necklace', 'vase', 'picture', 'tv', 'video',\n"
"         'chest', 'brick'])[15,100,90,60,40,15,10,1]\n"
"WEIGHT::(['camera', 'necklace', 'vase', 'picture', 'tv', 'video',\n"
"          'chest', 'brick'])[2,20,20,30,40,30,60,10]\n"

"! Objective: maximize total value\n"
"MaxVal:= sum(i in ITEMS) VALUE(i)*take(i)\n"

"! Weight restriction\n"
"sum(i in ITEMS) WEIGHT(i)*take(i) <= WTMAX\n"
"! All variables are 0/1\n"
"forall(i in ITEMS) take(i) is_binary\n"

"maximize(MaxVal)                ! Solve the problem\n"

"! Print out the solution\n"
"writeln(\"Solution:\\n Objective: \", getobjval)\n"
"forall(i in ITEMS)  writeln(' take(', i, '): ', getsol(take(i)))\n"

"end-model";


/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 int result,i;
 char bimfile[2000];              /* Buffer to store BIM file */
 size_t bimfile_size;             /* Buffer to store actual size of BIM file */
 char mosfile_name[40];           /* File name of MOS file */
 char bimfile_name[64];           /* File name of BIM file */

 i=XPRMinit();                    /* Initialize Mosel */
 if((i!=0)&&(i!=32))
  return 1;

/* Prepare file names for compilation using 'mem' driver: */
/*   "mem:base address/size[/actual size pointer]"        */
 bimfile_size=0;
 sprintf(mosfile_name, "mem:%p/%d",
        source_of_model, (int)sizeof(source_of_model));
 sprintf(bimfile_name, "mem:%p/%d/%p",
        bimfile, (int)sizeof(bimfile), &bimfile_size);

                                  /* Compile model from memory to memory */
 if(XPRMcompmod(NULL, mosfile_name, bimfile_name, "Knapsack example"))
  return 2;
 printf("BIM file uses %d bytes of memory.\n", (int)bimfile_size);

                                  /* Load a BIM file from memory */
 if((mod=XPRMloadmod(bimfile_name, NULL))==NULL)
  return 3;

 if(XPRMrunmod(mod, &result, NULL))  /* Run the model */
  return 4;

 return 0;
}


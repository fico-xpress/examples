/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugcompfrmem.c
   ``````````````````
   Compilation from memory.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

/**** The source of the model as an array of characters ****/
const char source_of_model[]=
 "model Burglar\n"
 "uses 'mmxprs'\n"
 
 "declarations\n"
 " WTMAX = 102                    ! Maximum weight allowed\n"
 " ITEMS = 1..8                   ! Index range for items\n"
 " VALUE: array(ITEMS) of real    ! Value of items\n"
 " WEIGHT: array(ITEMS) of real   ! Weight of items\n"
 " take: array(ITEMS) of mpvar    ! 1 if we take item i; 0 otherwise\n"
 "end-declarations\n"

 "VALUE :: [15, 100, 90, 60, 40, 15, 10,  1]\n"
 "WEIGHT:: [ 2,  20, 20, 30, 40, 30, 60, 10]\n"

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


/**** Main function ****/
int main()
{
 XPRMmodel mod;
 int result;
 char mosfile_name[40];           /* File name of MOS file */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file name for compilation using 'mem' driver: */
/*   "mem:base_address/size[/actual_size_of_pointer]"    */
 sprintf(mosfile_name, "mem:%p/%d",  
  source_of_model, (int)sizeof(source_of_model));

                                  /* Compile model file from memory */
 if(XPRMcompmod(NULL, mosfile_name, "burglar.bim", "Knapsack example")) 
  return 2;
                                  /* Load BIM file */
 if((mod=XPRMloadmod("burglar.bim", NULL))==NULL)
  return 3;

 if(XPRMrunmod(mod, &result, NULL))  /* Run the model */
  return 4;

/* Alternative to compile/load/run sequence: execute the model */
/* if(XPRMexecmod(NULL, mosfile_name, NULL, &result, &mod)) return 2; */

 XPRMresetmod(mod);
 
 return 0;
}


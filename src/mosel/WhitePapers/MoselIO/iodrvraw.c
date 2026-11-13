/********************************************************
   Mosel Example Problems
   ======================

   file iodrvraw.c
   ```````````````
   Using the IO drivers `raw' and `mem'.
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
"model Burglar2r\n"
"uses 'mmxprs'\n"

"parameters\n"
" DATA=''\n"
" SOL=''\n"
" WTMAX = 102                    ! Maximum weight allowed\n"
"end-parameters\n"

"declarations\n"
" public ITEMS: set of string    ! Index set for items\n"
" VALUE: array(ITEMS) of real    ! Value of items\n"
" WEIGHT: array(ITEMS) of real   ! Weight of items\n"
" SOLTAKE: array(ITEMS) of real  ! Solution values\n"
"end-declarations\n"

"initialisations from 'raw:'\n"
" [VALUE,WEIGHT] as DATA\n"
"end-initialisations\n"

"declarations\n"
" take: array(ITEMS) of mpvar    ! 1 if we take item i; 0 otherwise\n"
"end-declarations\n"

"! Objective: maximize total value\n"
"MaxVal:= sum(i in ITEMS) VALUE(i)*take(i)\n"

"! Weight restriction\n"
"sum(i in ITEMS) WEIGHT(i)*take(i) <= WTMAX\n"

"! All variables are 0/1\n"
"forall(i in ITEMS) take(i) is_binary\n"

"maximize(MaxVal)                ! Solve the problem\n"

"! Solution output\n"
"forall(i in ITEMS) SOLTAKE(i):= getsol(take(i))\n"

"writeln(\"Solution:\\n Objective: \", getobjval)\n"
"writeln(SOLTAKE)\n"

"initialisations to 'raw:'\n"
" SOLTAKE as SOL\n"
"end-initialisations\n"

"end-model";


const struct
{                                 /* Initial values for array 'data': */
 const char *ind;                 /*   index name */
 double val,wght;                 /*   value and weight data entries */
} data[]={{"camera",15,2}, {"necklace",100,20}, {"vase",90,20}, 
          {"picture",60,30}, {"tv",40,40}, {"video",15,30}, 
          {"chest",10,60}, {"brick",1,10}};

double solution[8];               /* Array for solution values */

/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 XPRMalltypes rvalue, itemname;
 XPRMset set;
 int result,i;
 char bimfile[2000];              /* Buffer to store BIM file */
 size_t bimfile_size;             /* Buffer to store actual size of BIM file */
 char mosfile_name[40];           /* File name of MOS file */
 char bimfile_name[64];           /* File name of BIM file */
 char data_name[40];              /* File name of initial values for 'data' */
 char solution_name[40];          /* File name of solution values */
 char params[96];                 /* Parameter string for model execution */

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

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */
/* Options for 'raw':                                                       */
/* 'slength=0': strings are represented by pointers to null terminated      */
/*              arrays of characters (C-string) instead of fixed size arrays*/
/* 'noindex':   only array values are expected - no indices requested       */

#ifdef _WIN32
 sprintf(data_name, "slength=0,mem:%#Ix/%u", (size_t)data, 
         (unsigned int)sizeof(data));
 sprintf(solution_name, "noindex,mem:%#Ix/%u", (size_t)solution, 
         (unsigned int)sizeof(solution));
#else
 sprintf(data_name, "slength=0,mem:%#lx/%u", (unsigned long)data, 
         (unsigned int)sizeof(data));
 sprintf(solution_name, "noindex,mem:%#lx/%u", (unsigned long)solution, 
         (unsigned int)sizeof(solution));
#endif
                                   /* Pass file names as execution param.s */
 sprintf(params, "DATA='%s',SOL='%s'", data_name, solution_name);

 if(XPRMrunmod(mod, &result, params))      /* Run the model */
  return 4;

/* Display solutions values obtained from the model */
 printf("Objective: %g\n", XPRMgetobjval(mod));
 XPRMfindident(mod, "ITEMS", &rvalue);     /* Get the model object 'ITEMS' */
 set = rvalue.set;
 for(i=0;i<8;i++)
  printf(" take(%s): %g\n", XPRMgetelsetval(set, i+1, &itemname)->string,
         solution[i]);

 return 0;
}


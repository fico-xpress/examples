/********************************************************
   Mosel Example Problems
   ======================

   file iodrvraw2.c
   ````````````````
   Using the IO drivers `raw' and `mem'.
   - Model source in separate .mos file -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, Feb. 2007, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

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
 char data_name[40];              /* File name of initial values for 'data' */
 char solution_name[40];          /* File name of solution values */
 char params[96];                 /* Parameter string for model execution */

 i=XPRMinit();                    /* Initialize Mosel */
 if((i!=0)&&(i!=32))
  return 1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */
/* Options for 'raw':                                                       */
/* 'slength=0': strings are represented by pointers to null terminated      */
/*              arrays of characters (C-string) instead of fixed size arrays*/
/* 'noindex':   only array values are expected - no indices requested       */

 sprintf(data_name, "slength=0,mem:%p/%d", data, (int)sizeof(data));
 sprintf(solution_name, "noindex,mem:%p/%d", solution, (int)sizeof(solution));

                                   /* Pass file names as execution param.s */
 sprintf(params, "DATA='%s',SOL='%s'", data_name, solution_name);

 if(XPRMexecmod(NULL, "burglar2r.mos", params, &result, &mod))
  return 2;                        /* Execute a model file */

/* Display solutions values obtained from the model */
 printf("Objective: %g\n", XPRMgetobjval(mod));
 XPRMfindident(mod, "ITEMS", &rvalue);     /* Get the model object 'ITEMS' */
 set = rvalue.set;
 for(i=0;i<8;i++)
  printf(" take(%s): %g\n", XPRMgetelsetval(set, i+1, &itemname)->string,
         solution[i]);

 return 0;
}


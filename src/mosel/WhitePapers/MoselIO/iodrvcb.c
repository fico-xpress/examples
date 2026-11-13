/********************************************************
   Mosel Example Problems
   ======================

   file iodrvcb.c
   ``````````````
   Using the IO drivers `cb' and `sysfd'.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2004, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

/**************************************/
/* Callback function to handle output */
/**************************************/
long XPRM_RTC cbmsg(XPRMmodel model, void *info, char *buf, unsigned long size)
{
/* Note: 'model' is NULL if the stream is used outside of an execution */
 printf("Mosel: %.*s", (int)size, buf);
 return 0;
}

/*****************/
/* Main function */
/*****************/
int main()
{
 XPRMmodel mod;
 int result, i;
 char outfile_name[40];           /* File name of output stream */

 i=XPRMinit();                    /* Initialize Mosel */
 if((i!=0)&&(i!=32))
  return 1;
                                  /* Using 'sysfd' driver:                   */
                                  /* Redirect error stream to default output */
 XPRMsetdefstream(NULL, XPRM_F_ERROR, "sysfd:1");

                                  /* Prepare file name for output stream   */
                                  /* using 'cb' driver:                    */
                                  /* "cb:function pointer[/callback data]" */
 sprintf(outfile_name, "cb:%p", cbmsg);

                                  /* Set default output stream to callback */
 XPRMsetdefstream(NULL, XPRM_F_WRITE, outfile_name);

                                  /* Execute = compile/load/run model file
                                               `burglar2.mos'
                                     Generates the BIM file `burglar2.bim' */
/* if(XPRMexecmod(NULL,"burglar2.mos",NULL,&result,NULL))
  return 2; */

 if(XPRMcompmod(NULL, "burglar2.mos", NULL, "Knapsack example"))
  return 2;                       /* Compile the model `burglar2.mos',
                                     output the file `burglar2.bim' */

 if((mod=XPRMloadmod("burglar2.bim", NULL))==NULL)  /* Load a BIM file */
  return 3;
 
 if(XPRMrunmod(mod, &result, NULL))  /* Run the model */
  return 4;

 return 0;
}


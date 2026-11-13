/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugcb.c
   ```````````
   Retrieve model output via a callback.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006, rev. July 2026
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

int main()
{
 int result;
 char outfile_name[40];           /* File name of output stream */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;
                                  /* Using 'sysfd' driver:                   */
                                  /* Redirect error stream to default output */
 XPRMsetdefstream(NULL, XPRM_F_ERROR, "sysfd:1");

                                  /* Prepare file name for output stream   */
                                  /* using 'cb' driver:                    */
                                  /* "cb:function_pointer[/callback_data]" */
 sprintf(outfile_name, "cb:%p", cbmsg);

                                  /* Set default output stream to callback */
 XPRMsetdefstream(NULL, XPRM_F_WRITE, outfile_name);

                                  /* Execute = compile/load/run a model */
 if(XPRMexecmod(NULL, "burglar2.mos", NULL, &result, NULL))
  return 2;

 return 0;
}


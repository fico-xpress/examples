/*******************************************************
   Mosel Example Problems 
   ======================

   file runrtdistr.c
   `````````````````
   Running a model on a remote machine,
   passing runtime parameters to the submodel.

   Before running this model, you need to set up the 
   NODENAME with a machine name/address of your local network.
   The node that is used needs to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on this machine.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Jan 2012
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprd.h"

int main(int argv,char *args[]) 
{
  XPRDcontext xprd;
  XPRDmosel mosInst;
  XPRDmodel modRP;
  char params[200];

       /* Use the name or IP address of a machine in
        * your local network, or "" for current node */
  char *NODENAME = "";

  xprd=XPRDinit();             /* Create an XPRD context */
                               /* Open connection to a remote node */
  mosInst=XPRDconnect(xprd, NODENAME, NULL, NULL, NULL, 0);
                               /* Compile the model file */
  XPRDcompmod(mosInst, "", "rmt:rtparams.mos", "tmp:rp.bim", "");
                               /* Load the bim file into the remote instance */
  modRP=XPRDloadmod(mosInst, "tmp:rp.bim"); 
                               /* Run-time parameters */
  sprintf(params, "PARAM1=%d,PARAM2=%g,PARAM3='a string',PARAM4=true", 2, 3.4);
  XPRDrunmod(modRP, params);   /* Run the model */
  XPRDwaitevent(xprd,-1);      /* Wait for model termination */
  XPRDdropevent(xprd);         /* Ignore termination event message */

  printf("`rtparams' returned: %d\n", XPRDgetexitcode(modRP));

  XPRDunloadmod(modRP);        /* Unload the model */
  XPRDdisconnect(mosInst);     /* Disconnect remote instance */
  XPRDfinish(xprd);            /* Terminate XPRD */
  return 0;
} 

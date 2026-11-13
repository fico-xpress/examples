/*******************************************************
   Mosel Example Problems 
   ======================

   file runrtpardistr.c
   ````````````````````
   Running several instances of a model remotely from
   a host program.
   - Parallel submodels in distributed architecture -

   Before running this model, you need to set up the 
   NODENAME with a machine name/address of your local network.
   The node that is used needs to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on this machine.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Apr 2012, rev. Jan. 2013
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprd.h"

#define A 10 
#define B 5

int main(int argv,char *args[]) 
{
  XPRDcontext xprd;
  XPRDmosel mosInst[B];
  XPRDmodel modRP[A];
  char params[200];
  char *NODENAME[B];
  int i,j;
  char bufd[200],bufn[200];

       /* Use the name or IP address of a machine in
        * your local network, or "" for current node */
  for(j=0;j<B;j++) NODENAME[j] = "localhost";

  xprd=XPRDinit();             /* Create an XPRD context */
                               /* Open connection to a remote node */
  for(j=0;j<B;j++)
  {
    mosInst[j]=XPRDconnect(xprd, NODENAME[j], NULL, NULL, NULL, 0);
    if(mosInst[j]==NULL)
    {
     printf("Failed to connect to %s - aborting.\n",NODENAME[j]);
     exit(1);
    }
  }

  for(j=0;j<B;j++)
  {
    XPRDsysinfo(mosInst[j], XPRD_SYS_NODE, bufd, sizeof(bufd));
    XPRDsysinfo(mosInst[j], XPRD_SYS_NAME, bufn, sizeof(bufn));
    printf("Submodel node: %s on %s\n", bufd, bufn);
  }

                               /* Compile the model file */
  XPRDcompmod(mosInst[0], "", "rmt:rtparams3.mos", "rmt:rp3.bim", "");

  for(i=0;i<A;i++)
  {                             /* Load the bim file into the remote instance */
    modRP[i]=XPRDloadmod(mosInst[i%B], "rmt:rp3.bim"); 
                               /* Run-time parameters */
    sprintf(params, "PARAM1=%d,PARAM2=%g,PARAM3='a string',PARAM4=%s", i,
            0.1*i, (i%2==0)?"true":"false");
    XPRDrunmod(modRP[i], params);   /* Run the model */
  }

  for(i=0;i<A;i++)
  {
    XPRDwaitevent(xprd,-1);    /* Wait for model termination */
    XPRDdropevent(xprd);       /* Ignore termination event message */
  }

  for(i=0;i<A;i++)
    XPRDunloadmod(modRP[i]);   /* Unload the models */

  for(j=0;j<B;j++)
    XPRDdisconnect(mosInst[j]);   /* Disconnect remote instance */
  XPRDfinish(xprd);            /* Terminate XPRD */
  
  remove("rp3.bim");          /* Cleaning up */
  return 0;
} 

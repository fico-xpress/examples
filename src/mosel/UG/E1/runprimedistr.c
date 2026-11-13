/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file runprimedistr.c
   ````````````````````
   Distributed computing: 
   Running a model on a remote Mosel instance.

   Before running this model, the server 
   "xprmsrv" must have been started on this machine.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Apr 2012
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprd.h"

int main(int argv,char *args[]) 
{
  XPRDcontext xprd;
  XPRDmosel moselInst;
  XPRDmodel modPrime, evsender;
  double evvalue;
  int evclass;

  xprd=XPRDinit();              /* Create an XPRD context */
                                /* Open connection to a remote node:
			           "" means the node running this program */
  moselInst=XPRDconnect(xprd, "", NULL, NULL, NULL, 0);
                                /* Compile the model file */
  XPRDcompmod(moselInst, "", "rmt:prime.mos", "rmt:prime.bim", "");
                                /* Load the bim file into the remote instance */
  modPrime=XPRDloadmod(moselInst, "rmt:prime.bim"); 

  XPRDrunmod(modPrime, "LIMIT=50000");  /* Start execution and */
  XPRDwaitevent(xprd,2);        /* wait 2 seconds for an event */

  if (XPRDqueueempty(xprd)==1)  /* No event has been sent... */
  {
   printf("Model too slow: stopping it!\n");
   XPRDstoprunmod(modPrime);    /* ... stop the model, then wait */
   XPRDwaitevent(xprd,-1);
  }

  XPRDgetevent(xprd, &evsender, &evclass, &evvalue);    /* Get the event */
  printf("Event value: %g\n", evvalue);
/*printf("Event value: %g sent by model %d\n", evvalue, XPRDgetnum(evsender));*/
  printf("Exit status: %d\n", XPRDgetstatus(modPrime));
  printf("Exit code  : %d\n", XPRDgetexitcode(modPrime));

  XPRDunloadmod(modPrime);       /* Unload the model */
  XPRDdisconnect(moselInst);     /* Disconnect remote instance */
  XPRDfinish(xprd);              /* Terminate XPRD */

  remove("prime.bim");           /* Clean up temporary files */

  return 0;
} 

/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexctc.c                                      */
/*  ``````````````                                      */
/*  Example for the use of the Mosel libraries          */
/*  (Interrupting a running model)                      */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2001, rev. July 2026      */
/********************************************************/

#include <stdio.h>
#include <signal.h>
#include "xprm.h"

XPRMmodel mod;

void stop_mod(int sig)
{
 if(XPRMisrunmod(mod))            /* If the model `mod' is currently running */
  XPRMstoprunmod(mod);            /* Stop it */
}

int main()
{
 int rts_run,result,i;

 i=XPRMinit();
 if((i!=0)&&(i!=32))              /* Initialize Mosel */
  return 1;

 if((mod=XPRMloadmod("Models/toolong.bim",NULL))==NULL)   /* Load a BIM file */
  return 2;

 signal(SIGINT,stop_mod);         /* Redirect the Ctrl-C signal */

 printf("Running a long loop... Press Ctrl-C to stop it!\n");
 rts_run=XPRMrunmod(mod,&result,NULL);    /* Run the model */

 if(rts_run==0)
  printf("Normal termination.\n");
 else
  if(rts_run&XPRM_RT_STOP)
   printf("Interrupted.\n");
  else
   printf("Error when running.\n");
 
 return 0;
}


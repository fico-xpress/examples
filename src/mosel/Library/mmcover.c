/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmcover.c                                      */
/*  ``````````````                                      */
/*  Example for the use of the Mosel libraries          */
/*  (using debugger interface)                          */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2004, rev. July 2026      */
/********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xprm.h"

int MM_RTC dbgcb(void *dctx,int vmstat, int lindex);
void print_report(XPRMmodel mod);

int *count;
int nbstat;

int main(int argc,const char *argv[])
{
 XPRMmodel mod;
 char filename[256];
 int rts;

 if(argc!=2)
 {
  printf("Usage: %s modname\n",argv[0]);
  return 0;
 }
 else
 {
  rts=XPRMinit();
  if((rts!=0)&&(rts!=32))      /* Initialize Mosel */
   return 1;

  sprintf(filename,"%s.mos",argv[1]);
  if(XPRMcompmod("G",filename,NULL,NULL))
   return 2;                   /* Compile the model with debug info */

  sprintf(filename,"%s.bim",argv[1]);
  if((mod=XPRMloadmod(filename,NULL))==NULL)
   return 3;
                               /* Create a table to store the number of times */
                               /* each statement is executed */
  nbstat=XPRMdbg_getnblndx(mod);
  count=(int *)malloc(nbstat*sizeof(int));
  memset(count,0,nbstat*sizeof(int));

                               /* Disable model output */
  XPRMsetdefstream(mod,XPRM_F_OUTPUT,"null:");
  XPRMdbg_setbrkp(mod,0);      /* Put a breakpoint before the first statement */

  printf("Running model...");
  XPRMdbg_runmod(mod,&rts,NULL,dbgcb,NULL);
  printf(" done.\n");

  print_report(mod);
  XPRMresetmod(mod);
  return 0;
 }
}

/*********************/
/* Debugger callback */
/*********************/
int MM_RTC dbgcb(void *dctx,int vmstat, int lindex)
{
 count[lindex]++;             /* Increase counter for current statement */
 return XPRM_DBG_STEP;        /* Continue until next one */
}

/***********************************/
/* Display results after execution */
/***********************************/
void print_report(XPRMmodel mod)
{
 int l,line,type;
 const char *name,*symb;
 int never_done;
 void *ref;
 XPRMproc proc;
 XPRMalltypes value;

                           /* Count statements that have never been executed */
 never_done=0;
 for(l=0;l<nbstat;l++)
  if(count[l]==0) never_done++;

 XPRMgetmodprop(mod,XPRM_PROP_NAME,(XPRMalltypes *)&name);
 printf("%.2f%% (%d) of the %d statements of model `%s' have been executed.\n",
                        100.0-(double)never_done*100/nbstat,nbstat-never_done,
                        nbstat,name!=NULL?name:"*");

                           /* Look for all subroutines... */
 ref=NULL;
 while((symb=XPRMgetnextident(mod,&ref))!=NULL)
 {
  type=XPRMfindident(mod,symb,&value);
  if(XPRM_STR(type)==XPRM_STR_PROC)
  {
   proc=value.proc;
   do
   {                      /* Display location */
    l=XPRMdbg_findproclndx(mod,proc);
    XPRMdbg_getlocation(mod,l,&line,&name);
    printf("%s `%s' at (%s:%d)",
                            XPRM_TYP(type)==XPRM_TYP_NOT?"procedure":"function",
                            symb,name,line);

                          /* Display number of times it has been called */
    if(count[l]>1)
     printf(" has been called %d times\n",count[l]);
    else if(count[l]==1)
     printf(" has been called once\n");
    else
     printf(" has never been called\n");

                          /* Do the same for all overloaded routine */
   }while((proc=XPRMgetnextproc(proc))!=NULL);
  }
 }
}

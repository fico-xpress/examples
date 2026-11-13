/********************************************************
   Mosel Example Problems
   ======================

   file runfoliocbio.c
   ```````````````````
   Running a Mosel model from a C application
   with data exchange between model and host application
   during the optimization run.
   (Passing data via callback)

   *** The model started by this program cannot be run with
       a Community Licence for the provided data instance ***

   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, May 2012, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprm.h"


          /* Array to receive solution values */
struct MySolArray
{
 const char *ind;                 /*   index name */
 double val;                      /*   solution value */
};

struct MySolArray *solfrac;
struct MySolArray *solbuy;
int ifprint;

/******************************************/
/* Callback for getting model output data */
/******************************************/
int XPRM_RTC cbinit_to(XPRMcbinit cb, void *info, const char *label,
                       int type, XPRMalltypes *ref)
{
 static const char *typename[]={"none","integer","real","string","boolean"};
 const char *tn,*sn;
 char *index;
 XPRMarray solarr;
 XPRMset sets[1];             /* We know all sol. arrays have 1 dimension */
 int indices[1];
 XPRMalltypes rvalue;
 int asize,ct,i;
 double objval;
 int numshares, solcount;
 

 if(strcmp(label,"FRAC")==0)
 {
  solarr=ref->array;

  asize=XPRMgetarrsize(solarr);
  solfrac = (struct MySolArray *)malloc(asize * sizeof(struct MySolArray));

  XPRMgetarrsets(solarr,sets);    /* Get the indexing sets 
                                     (we know array has 1 dimension) */ 
  ct=0;
  XPRMgetfirstarrtruentry(solarr,indices); /* Get the first true index tuple */
  do
  {
   solfrac[ct].ind=XPRMgetelsetval(sets[0],indices[0],&rvalue)->string;
   XPRMgetarrval(solarr,indices,&rvalue);
   solfrac[ct].val=rvalue.real;
   ct++;
  } while(!XPRMgetnextarrtruentry(solarr,indices));
  if (ifprint==0) 
  { ifprint=1; }
  else
  {
   ifprint=0;
   for(i=0;i<asize;i++)
    printf(" %s: %g%% (%g)\n", solfrac[i].ind, solfrac[i].val*100, 
     solbuy[i].val);
   free(solbuy);
   free(solfrac);  
  }
 }
 else if(strcmp(label,"BUY")==0)
 {
  solarr=ref->array;

  asize=XPRMgetarrsize(solarr);
  solbuy = (struct MySolArray *)malloc(asize * sizeof(struct MySolArray));

  XPRMgetarrsets(solarr,sets);    /* Get the indexing sets 
                                     (we know array has 1 dimension) */ 
  ct=0;
  XPRMgetfirstarrtruentry(solarr,indices); /* Get the first true index tuple */
  do
  {
   solbuy[ct].ind=XPRMgetelsetval(sets[0],indices[0],&rvalue)->string;
   XPRMgetarrval(solarr,indices,&rvalue);
   solbuy[ct].val=rvalue.real;
   ct++;
  } while(!XPRMgetnextarrtruentry(solarr,indices));
  if (ifprint==0) 
  { ifprint=1; }
  else
  {
   ifprint=0;
   for(i=0;i<asize;i++)
    printf(" %s: %g%% (%g)\n", solfrac[i].ind, solfrac[i].val*100, 
     solbuy[i].val);
   free(solbuy);
   free(solfrac);  
  }
 }
 else if(strcmp(label,"RETSOL")==0)
 {
  objval=ref->real;
  printf("Total return: %g\n", objval);
 }
 else if(strcmp(label,"NUMSHARES")==0)
 {
  numshares=ref->integer;
  printf("Number of shares: %d\n", numshares);
 }
 else if(strcmp(label,"SOLCOUNT")==0)
 {
  solcount=ref->integer;
  printf("Solution number: %d\n", solcount);
 }
 else
 {
  if(XPRM_TYP(type)<=4) tn=typename[XPRM_TYP(type)];
  else tn="external";
  switch(XPRM_STR(type))
  {
   case XPRM_STR_CONST:
   case XPRM_STR_REF: sn="ref";break;
   case XPRM_STR_ARR: sn="array";break;
   case XPRM_STR_SET: sn="set";break;
   case XPRM_STR_LIST: sn="list";break;
   default: sn="unknown";
  }
  printf("Unknown output data item: %s %s %s %p\n",label,sn,tn,ref);
 }
 return 0;
}

/********************************************************/

int main()
{
 XPRMmodel mod;
 char solution_name[40];          /* File name of solution callback */
 char params[1000];               /* Parameter string for model execution */
 int i,result;
 double maxrisk = 1.0/3;          /* Model parameter settings */
 double minreg = 0.2;
 double maxreg = 0.5;
 double maxsec = 0.25;
 double maxval = 0.2;
 double minval = 0.1;
 int maxnum = 15;
 ifprint = 0;

/* Prepare file names for 'initializations' using the 'cb' driver */
 sprintf(solution_name, "cb:%p", cbinit_to);

                                  /* Pass file names as execution param.s */
 sprintf(params, "MAXRISK=%g,MINREG=%g,MAXREG=%g,MAXSEC=%g,MAXVAL=%g,MINVAL=%g,MAXNUM=%d,DATAFILE='folio250.dat',OUTPUTFILE='%s'",
   maxrisk, minreg, maxreg, maxsec, maxval, minval, maxnum, solution_name); 


 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

 if(XPRMexecmod(NULL, "foliocbio.mos", params, &result, &mod))
  return 2;                       /* Execute the model file (only during  
                                     development phase, the deployed 
				     application would only use BIM) */

 if(result != XPRM_RT_OK){
   printf("Error during model execution: %d\n", result);
   return 3;
 }
 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT) {
   printf("Problem not optimal");           /* No solution available */
   return 4;
 }

 XPRMresetmod(mod);               /* Reset the model */
 
 return 0;
}

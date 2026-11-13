/********************************************************
   Mosel Example Problems
   ======================

   file runfoliod.c
   ````````````````
   Running a Mosel model from a C application
   with data exchange between model and host application.
   (Passing data via callback)

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Oct. 2009, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprm.h"

#define maxnum 15

char* riskset[] = {"hardware1", "hardware2", "hardware3", "hardware4", 
   "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", 
   "hardware10", "theater1", "theater2", "theater3", "theater4", "theater5", 
   "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1",
   "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", 
   "telecom8", "telecom9", "telecom10", "software1", "software2", "software3", 
   "software4", "software5", "software6", "software7", "software8", 
   "software9", "software10", "electronics1", "electronics2", "electronics3", 
   "electronics4", "electronics5", "electronics6", "electronics7", 
   "electronics8", "electronics9", "electronics10"};
int risksize=50;

char* locdataind1[] = {"EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC"};
char* locdataind2[] = {"treasury1", "treasury10", "hardware7", "hardware9", "theater8", "theater10", "telecom3", "telecom9", "telecom10", "brewery1", "brewery4", "brewery6", "brewery7", "brewery8", "highways2", "highways5", "highways7", "highways10", "cars4", "cars7", "cars8", "cars9", "cars10", "bank1", "bank3", "bank5", "bank6", "bank7", "bank10", "software3", "software4", "software5", "software6", "electronics2", "treasury2", "treasury6", "treasury7", "treasury8", "hardware4", "hardware6", "theater1", "theater3", "theater7", "theater9", "telecom2", "telecom4", "telecom8", "highways3", "highways4", "cars5", "cars6", "bank2", "bank4", "bank8", "bank9", "software1", "software2", "software9", "electronics3", "electronics5", "electronics6", "electronics8", "electronics9", "electronics10", "treasury4", "treasury5", "treasury9", "hardware1", "hardware2", "hardware5", "hardware8", "hardware10", "theater2", "theater4", "telecom1", "telecom5", "telecom6", "telecom7", "brewery9", "brewery10", "highways8", "cars1", "cars2", "software7", "software8", "software10", "electronics1", "electronics4", "electronics7"};
int locdata[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int locsize=89;

char* secdataind1[] = {"bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "food", "food", "food", "food", "food", "food", "food", "food", "food", "food", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "finance", "finance", "finance", "finance", "finance", "finance", "finance", "finance", "finance", "finance"};
char* secdataind2[] = {"treasury1", "treasury2", "treasury3", "treasury4", "treasury5", "treasury6", "treasury7", "treasury8", "treasury9", "treasury10", "hardware1", "hardware2", "hardware3", "hardware4", "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", "hardware10", "software1", "software2", "software3", "software4", "software5", "software6", "software7", "software8", "software9", "software10", "electronics1", "electronics2", "electronics3", "electronics4", "electronics5", "electronics6", "electronics7", "electronics8", "electronics9", "electronics10", "theater1", "theater2", "theater3", "theater4", "theater5", "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1", "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", "telecom8", "telecom9", "telecom10", "brewery1", "brewery2", "brewery3", "brewery4", "brewery5", "brewery6", "brewery7", "brewery8", "brewery9", "brewery10", "highways1", "highways2", "highways3", "highways4", "highways5", "highways6", "highways7", "highways8", "highways9", "highways10", "cars1", "cars2", "cars3", "cars4", "cars5", "cars6", "cars7", "cars8", "cars9", "cars10", "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9", "bank10"};
int secdata[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int secsize=100;

char* retdataind[] = {"treasury1", "treasury2", "treasury3", "treasury4", "treasury5", "treasury6", "treasury7", "treasury8", "treasury9", "treasury10", "hardware1", "hardware2", "hardware3", "hardware4", "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", "hardware10", "theater1", "theater2", "theater3", "theater4", "theater5", "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1", "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", "telecom8", "telecom9", "telecom10", "brewery1", "brewery2", "brewery3", "brewery4", "brewery5", "brewery6", "brewery7", "brewery8", "brewery9", "brewery10", "highways1", "highways2", "highways3", "highways4", "highways5", "highways6", "highways7", "highways8", "highways9", "highways10", "cars1", "cars2", "cars3", "cars4", "cars5", "cars6", "cars7", "cars8", "cars9", "cars10", "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9", "bank10", "software1", "software2", "software3", "software4", "software5", "software6", "software7", "software8", "software9", "software10", "electronics1", "electronics2", "electronics3", "electronics4", "electronics5", "electronics6", "electronics7", "electronics8", "electronics9", "electronics10"};
double retdata[] = {5.29, 2.8, 3.59, 6.97, 3.3, 3.02, 2.98, 3.41, 3.93, 3.86, 11.36, 17.23, 16.31, 21.1, 13.76, 9.31, 16.99, 24.85, 18.52, 12.79, 36.1, 19.4, 23.5, 34.61, 16.91, 27.04, 25.82, 24.99, 36.89, 31.71, 10.89, 17.98, 12.31, 6.53, 6.11, 15.89, 12.46, 11.11, 16.6, 13.93, 8.79, 9.35, 8.88, 11.63, 6.21, 9.26, 4.77, 5.98, 10.92, 5.26, 5.39, 4.85, 8.34, 8.48, 6.42, 11.04, 13.15, 10.01, 7.71, 11.07, 5.74, 7.13, 8.75, 5.08, 6.23, 8.19, 8.03, 5.96, 4.17, 8.11, 6.79, 3.01, 4.92, 4.14, 8.98, 8.81, 4.7, 8.06, 6.22, 4.47, 34.59, 44.94, 43.05, 15.58, 42.05, 20.66, 20.76, 19.85, 20.05, 45.38, 23.07, 19.1, 23.83, 16.54, 28.33, 25.88, 22.14, 22.65, 12.59, 28.1};
int retsize=100;

          /* Array to receive solution values */
struct MySolArray
{
 const char *ind;                 /*   index name */
 double val;                      /*   solution value */
};

struct MySolArray *solfrac;
struct MySolArray *solbuy;
double objval;
int numshares, status;


/********************************************/
/* Callback for generating Model input data */
/********************************************/
int XPRM_RTC cbinit_from(XPRMcbinit cb, void *info, const char *label, 
                         int type, void *ref)
{
 int i;

 if(strcmp(label,"RISK")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=0;i<risksize;i++)
  {
   XPRMcb_sendstring(cb,riskset[i],-1,0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else
 if(strcmp(label,"RET")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=0;i<retsize;i++)
  {
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENNDX,0);
   XPRMcb_sendstring(cb,retdataind[i],-1,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSENDX,0);
   XPRMcb_sendreal(cb,retdata[i],0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else
 if(strcmp(label,"LOCTAB")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=0;i<locsize;i++)
  {
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENNDX,0);
   XPRMcb_sendstring(cb,locdataind1[i],-1,0);
   XPRMcb_sendstring(cb,locdataind2[i],-1,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSENDX,0);
   XPRMcb_sendint(cb,locdata[i],0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else
 if(strcmp(label,"SECTAB")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=0;i<secsize;i++)
  {
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENNDX,0);
   XPRMcb_sendstring(cb,secdataind1[i],-1,0);
   XPRMcb_sendstring(cb,secdataind2[i],-1,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSENDX,0);
   XPRMcb_sendint(cb,secdata[i],0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else
 {
  fprintf(stderr,"Label `%s' not found.\n",label);
  return 1;
 }
}

/******************************************/
/* Callback for getting model output data */
/******************************************/
int XPRM_RTC cbinit_to(XPRMcbinit cb, void *info, const char *label,
                       int type, XPRMalltypes *ref)
{
 static const char *typename[]={"none","integer","real","string","boolean"};
 const char *tn,*sn;
 XPRMarray solarr;
 XPRMset sets[1];             /* We know all sol. arrays have 1 dimension */
 int indices[1];
 XPRMalltypes rvalue;
 int asize, ct;

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
 }
 else
 if(strcmp(label,"BUY")==0)
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
 }
 else
 if(strcmp(label,"RETSOL")==0)
 {
  objval=ref->real;
 }
 else
 if(strcmp(label,"NUMSHARES")==0)
 {
  numshares=ref->integer;
 }
 else
 if(strcmp(label,"SOLSTATUS")==0)
 {
  status=ref->integer;
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
 char datafile_name[40], outfile_name[40];   /* File names input / output */
 char params[1000];               /* Parameter string for model execution */
 int i,result;
 double maxrisk = 1.0/3;          /* Model parameter settings */
 double minreg = 0.2;
 double maxreg = 0.5;
 double maxsec = 0.25;
 double maxval = 0.2;
 double minval = 0.1;

 sprintf(datafile_name, "cb:%p", cbinit_from);
 sprintf(outfile_name, "cb:%p", cbinit_to);

                                  /* Pass file names as execution param.s */
 sprintf(params, "MAXRISK=%g,MINREG=%g,MAXREG=%g,MAXSEC=%g,MAXVAL=%g,MINVAL=%g,MAXNUM=%d,DATAFILE='%s',OUTPUTFILE='%s'",
         maxrisk, minreg, maxreg, maxsec, maxval, minval, maxnum,
         datafile_name, outfile_name); 


 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

 if(XPRMexecmod(NULL, "foliomemio.mos", params, &result, &mod))
  return 2;                       /* Execute the model file (only during  
                                     development phase, the deployed 
				     application would only use BIM) */

 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                       /* Stop if no solution available */

                    /* Display solution values obtained from the model */
 printf("Total return: %g\n", objval);

 for(i=0;i<numshares;i++)
  printf("%s: %g%% (%g)\n", solfrac[i].ind, solfrac[i].val*100, solbuy[i].val);

 XPRMresetmod(mod);               /* Reset the model */
 
 return 0;
}

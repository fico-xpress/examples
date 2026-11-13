/********************************************************
   Mosel User Guide Example Problems
   =================================

   file ugiocb.c
   `````````````
   Exchanging data between model and host application.
   - Callbacks for exchanging data (sparse data, string indices) -
   Executing a model file.

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Nov. 2009, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprm.h"

                                  /* Input values for data: */
char *ind[]={"camera", "necklace", "vase", "picture", "tv", "video", 
             "chest", "brick"};              /* Index names */
double vdata[]={15,100,90,60,40,15,10, 1};   /* Input data: VALUE */
double wdata[]={ 2, 20,20,30,40,30,60,10};   /* Input data: WEIGHT */
int datasize=8;

struct SolArray
{                                 /* Array to receive solution values: */
 const char *ind;                 /*   index name */
 double val;                      /*   solution value */
};

struct SolArray *solution;
int solsize;

/********************************************/
/* Callback for generating Model input data */
/********************************************/
int XPRM_RTC cbinit_from(XPRMcbinit cb, void *info, const char *label,
                         int type, void *ref)
{
 int i;

 if(strcmp(label,"DATA")==0)
 {
  XPRMcb_sendctrl(cb, XPRM_CBC_OPENLST, 0);
  for(i=0;i<datasize;i++)
  {
   XPRMcb_sendctrl(cb, XPRM_CBC_OPENNDX, 0);
    XPRMcb_sendstring(cb, ind[i], -1, 0);
   XPRMcb_sendctrl(cb, XPRM_CBC_CLOSENDX, 0);
   XPRMcb_sendctrl(cb, XPRM_CBC_OPENLST, 0);
    XPRMcb_sendreal(cb, vdata[i], 0);
    XPRMcb_sendreal(cb, wdata[i], 0);
   XPRMcb_sendctrl(cb, XPRM_CBC_CLOSELST, 0);
  }
  XPRMcb_sendctrl(cb, XPRM_CBC_CLOSELST, 0);
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
 XPRMarray solarr;
 XPRMset sets[1];
 int indices[1];
 XPRMalltypes rvalue;
 int ct;

 if(strcmp(label,"SOL")==0)
 {
  solarr=ref->array;

  solsize=XPRMgetarrsize(solarr);
  solution = (struct SolArray *)malloc(solsize * sizeof(struct SolArray));

  XPRMgetarrsets(solarr,sets);    /* Get the indexing sets 
                                     (we know array has 1 dimension) */ 
  ct=0;
  XPRMgetfirstarrtruentry(solarr,indices); /* Get the first true index tuple */
  do
  {
   solution[ct].ind=XPRMgetelsetval(sets[0],indices[0],&rvalue)->string;
   XPRMgetarrval(solarr,indices,&rvalue);
   solution[ct].val=rvalue.real;
   ct++;
  } while(!XPRMgetnextarrtruentry(solarr,indices));
 }
 else
 {
  printf("Unknown output data item: %s %p\n", label, ref);
 }
 return 0;
}

/********************************************************/

int main()
{
 XPRMmodel mod;
 int i,result;
 char data_name[40];              /* File name of input data 'data' */
 char solution_name[40];          /* File name of solution values */
 char params[128];                /* Parameter string for model execution */

 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

/* Prepare file names for 'initializations' using the 'cb' driver */
 sprintf(data_name, "cb:%p", cbinit_from);
 sprintf(solution_name, "cb:%p", cbinit_to);

                                  /* Pass file names as execution param.s */
 sprintf(params, "DATAFILE='%s',SOLFILE='%s'", data_name, solution_name);

 if(XPRMexecmod(NULL, "burglar13.mos", params, &result, &mod))
  return 2;                       /* Execute a model file */
 
 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                       /* Test whether a solution is found */

/* Display solution values obtained from the model */
 printf("Objective value: %g\n", XPRMgetobjval(mod));
 for(i=0;i<solsize;i++)
  printf(" take(%s): %g\n", solution[i].ind, solution[i].val);

 XPRMresetmod(mod);
 
 return 0;
}


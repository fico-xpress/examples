/********************************************************
   Mosel Example Problems
   ====================== 

   file scenar.c
   `````````````
   Executing different scenarios.
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002, rev. July 2026
********************************************************/

#include <stdio.h>
#include <string.h>
#include "xprm.h"

int execscenar(const char *FILENAME, double WEIGHT);

#define NSUP 6

double obj, capuse[NSUP], CAP[NSUP];
char SUPNAME[NSUP][64];

int main()
{
 int NSCENAR = 5;
 const char *DEMDATA[] = {"dem1.dat", "dem2.dat", "dem3.dat", "dem4.dat", 
                          "dem5.dat"};
 double WEIGHT[] = {0.4, 0.15, 0.1, 0.25, 0.1};
 int i,s;
 double fact = 0;

 if(XPRMinit())                         /* Initialize Mosel */
  return 1;
                                        /* Execute all scenarios */
 for(i=0;i<NSCENAR;i++) if(execscenar(DEMDATA[i], WEIGHT[i])) fact+=WEIGHT[i];
 fact= 1/fact;
                                        /* Print out aggregate results */
 printf("Weighted average objective: %g\n", obj*fact);
 for(s=0;s<NSUP;s++) 
  printf("%s: %g (%g%% of %g)\n", SUPNAME[s], capuse[s]*fact, 
         capuse[s]*fact/CAP[s]*100, CAP[s]);

 return 0;
}

/**** Run a scenario and retrieve its results ****/
int execscenar(const char *FILENAME, double WEIGHT)
{
 XPRMmodel mod;
 XPRMalltypes rvalue;
 XPRMarray darr, varr;
 XPRMset set[1];
 XPRMmpvar x;
 int indices[2], result;
 char params[128];

 sprintf(params, "DEMFILE=Data/%s", FILENAME);

                                          /* Execute model file 'scenar.mos' */
 if(XPRMexecmod(NULL,"scenar.mos",params,&result,&mod))
 { printf("Problem %s not executed.\n", FILENAME); return 0; }
 
                                          /* Test whether a solution is found */
 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
 { printf("Demand scenario %s is infeasible.\n", FILENAME); return 0; }

/* Get the objective function value */
 obj += WEIGHT*XPRMgetobjval(mod);                         

/* At first execution: get 'CAP' data and the 'Suppliers' index set for
   pretty solution printout */
 if(!strcmp(FILENAME, "dem1.dat"))
 {
  XPRMfindident(mod, "CAP", &rvalue);     /* Get the model object 'CAP' */
  darr = rvalue.array;
  XPRMgetarrsets(darr, set);              /* Get the index set of 'CAP' */
  XPRMgetfirstarrentry(darr, indices);    /* Get the first index tuple */
  do
  {
   strcpy(SUPNAME[indices[0]-1], 
     XPRMgetelsetval(set[0], indices[0], &rvalue)->string);
   XPRMgetarrval(darr, indices, &(CAP[indices[0]-1]));
  } while(!XPRMgetnextarrentry(darr, indices));  /* Get the next index tuple */
 }

/* At every execution: get the solution for variables 'x' */
 XPRMfindident(mod, "x", &rvalue);        /* Get the model object 'x' */
 varr = rvalue.array;
 XPRMgetfirstarrtruentry(varr, indices); /* Get the first entry of array varr
                                             (we know that the array is sparse 
                                             and has two dimensions) */
 do
 {
  XPRMgetarrval(varr, indices, &x);       /* Get the corresponding variable */
  capuse[indices[0]-1] += WEIGHT*XPRMgetvsol(mod,x);   /* Save the sol. value */
/*  printf("<%d,%d:%g>\n", indices[0], indices[1], XPRMgetvsol(mod,x)); */
 } while(!XPRMgetnextarrtruentry(varr, indices));  
                                          /* Get the next index tuple */
 return 1;
}

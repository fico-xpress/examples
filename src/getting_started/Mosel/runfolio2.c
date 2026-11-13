/********************************************************
   Mosel Example Problems
   ======================

   file runfolio2.c
   ````````````````
   Running a Mosel model from a C application
   with data exchange between model and host application.
   (Sparse data format using string indices)
   -- Grouping arrays with identical index sets --

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Feb. 2009, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

#define maxnum 15

          /* Arrays to store initial values for data */
struct MyArrayTwo
{
 const char *ind1,*ind2;          /*   index names */
 int val;                         /*   data value  */
};
struct MyArrayOne
{
 const char *ind;                 /*   index name */
 double val;                      /*   data value  */
};

          /* Array to receive solution values */
struct MySolArray
{
 const char *ind;                 /*   index name */
 double val1,val2;                /*   solution values */
};

struct MySolArray sol[maxnum];

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
struct MyArrayTwo locdata[] = {{"EU","treasury1",1},
   {"EU","treasury10",1}, {"EU","hardware7",1},
   {"EU","hardware9",1}, {"EU","theater8",1}, 
   {"EU","theater10",1}, {"EU","telecom3",1}, 
   {"EU","telecom9",1}, {"EU","telecom10",1}, 
   {"EU","brewery1",1}, {"EU","brewery4",1}, 
   {"EU","brewery6",1}, {"EU","brewery7",1}, 
   {"EU","brewery8",1}, {"EU","highways2",1}, 
   {"EU","highways5",1}, {"EU","highways7",1}, 
   {"EU","highways10",1}, {"EU","cars4",1}, 
   {"EU","cars7",1}, {"EU","cars8",1}, {"EU","cars9",1}, {"EU","cars10",1}, 
   {"EU","bank1",1}, {"EU","bank3",1}, {"EU","bank5",1}, {"EU","bank6",1}, 
   {"EU","bank7",1}, {"EU","bank10",1},
   {"EU","software3",1}, {"EU","software4",1}, 
   {"EU","software5",1}, {"EU","software6",1},
   {"EU","electronics2",1}, {"NA","treasury2",1}, 
   {"NA","treasury6",1}, {"NA","treasury7",1},
   {"NA","treasury8",1}, {"NA","hardware4",1}, 
   {"NA","hardware6",1}, {"NA","theater1",1},
   {"NA","theater3",1}, {"NA","theater7",1}, 
   {"NA","theater9",1}, {"NA","telecom2",1},
   {"NA","telecom4",1}, {"NA","telecom8",1}, 
   {"NA","highways3",1}, {"NA","highways4",1},
   {"NA","cars5",1}, {"NA","cars6",1},
   {"NA","bank2",1}, {"NA","bank4",1},{"NA","bank8",1}, {"NA","bank9",1}, 
   {"NA","software1",1}, {"NA","software2",1}, 
   {"NA","software9",1}, {"NA","electronics3",1}, 
   {"NA","electronics5",1}, {"NA","electronics6",1},
   {"NA","electronics8",1}, {"NA","electronics9",1},
   {"NA","electronics10",1}, {"APAC","treasury4",1},
   {"APAC","treasury5",1}, {"APAC","treasury9",1}, 
   {"APAC","hardware1",1}, {"APAC","hardware2",1}, 
   {"APAC","hardware5",1}, {"APAC","hardware8",1},  
   {"APAC","hardware10",1}, {"APAC","theater2",1}, 
   {"APAC","theater4",1}, {"APAC","telecom1",1},
   {"APAC","telecom5",1}, {"APAC","telecom6",1}, 
   {"APAC","telecom7",1}, {"APAC","brewery9",1}, 
   {"APAC","brewery10",1}, {"APAC","highways8",1}, 
   {"APAC","cars1",1}, {"APAC","cars2",1}, 
   {"APAC","software7",1}, {"APAC","software8",1}, 
   {"APAC","software10",1},{"APAC","electronics1",1}, 
   {"APAC","electronics4",1}, {"APAC","electronics7",1}};
struct MyArrayTwo secdata[] = {{"bonds","treasury1",1}, 
   {"bonds","treasury10",1}, {"bonds","treasury2",1}, 
   {"bonds","treasury6",1}, {"bonds","treasury7",1},
   {"bonds","treasury8",1}, {"bonds","treasury4",1},
   {"bonds","treasury5",1}, {"bonds","treasury9",1}, {"bonds","treasury3",1}, 
   {"technology","hardware7",1}, {"technology","hardware9",1}, 
   {"technology","software3",1}, {"technology","software4",1}, 
   {"technology","software5",1}, {"technology","software6",1}, 
   {"technology","electronics2",1}, {"technology","hardware4",1}, 
   {"technology","hardware6",1}, {"technology","software1",1}, 
   {"technology","software2",1}, {"technology","software9",1}, 
   {"technology","electronics3",1}, {"technology","electronics5",1}, 
   {"technology","electronics6",1}, {"technology","electronics8",1}, 
   {"technology","electronics9",1}, {"technology","electronics10",1}, 
   {"technology","hardware1",1}, {"technology","hardware2",1}, 
   {"technology","hardware5",1}, {"technology","hardware8",1}, 
   {"technology","hardware10",1}, {"technology","software7",1}, 
   {"technology","software8",1}, {"technology","software10",1}, 
   {"technology","electronics1",1}, {"technology","electronics4",1}, 
   {"technology","electronics7",1}, {"technology","hardware3",1},
   {"entertainment","theater8",1}, {"entertainment","theater10",1}, 
   {"entertainment","theater1",1}, {"entertainment","theater3",1}, 
   {"entertainment","theater7",1}, {"entertainment","theater9",1}, 
   {"entertainment","theater2",1}, {"entertainment","theater4",1}, 
   {"entertainment","theater5",1}, {"entertainment","theater6",1},
   {"telecom","telecom3",1}, {"telecom","telecom9",1}, 
   {"telecom","telecom10",1}, {"telecom","telecom2",1}, 
   {"telecom","telecom4",1}, {"telecom","telecom8",1},
   {"telecom","telecom1",1}, {"telecom","telecom5",1}, 
   {"telecom","telecom6",1}, {"telecom","telecom7",1}, 
   {"food","brewery1",1}, {"food","brewery4",1}, 
   {"food","brewery6",1}, {"food","brewery7",1}, 
   {"food","brewery8",1}, {"food","brewery9",1},
   {"food","brewery10",1}, {"food","brewery2",1}, 
   {"food","brewery3",1}, {"food","brewery5",1}, 
   {"construction","highways2",1}, {"construction","highways5",1},
   {"construction","highways7",1}, {"construction","highways10",1}, 
   {"construction","highways3",1}, {"construction","highways4",1}, 
   {"construction","highways8",1}, {"construction","highways1",1}, 
   {"construction","highways6",1}, {"construction","highways9",1}, 
   {"manufacturing","cars4",1}, {"manufacturing","cars7",1}, 
   {"manufacturing","cars8",1}, {"manufacturing","cars9",1}, 
   {"manufacturing","cars10",1}, {"manufacturing","cars5",1}, 
   {"manufacturing","cars6",1}, {"manufacturing","cars1",1}, 
   {"manufacturing","cars2",1}, {"manufacturing","cars3",1}, 
   {"finance","bank1",1}, {"finance","bank3",1},
   {"finance","bank5",1}, {"finance","bank6",1}, 
   {"finance","bank7",1}, {"finance","bank10",1}, 
   {"finance","bank2",1}, {"finance","bank4",1}, 
   {"finance","bank8",1}, {"finance","bank9",1}};   
struct MyArrayOne retdata[] = {{"treasury1",5.29},
   {"treasury2",2.8}, {"treasury3",3.59}, 
   {"treasury4",6.97}, {"treasury5",3.3}, 
   {"treasury6",3.02}, {"treasury7",2.98}, 
   {"treasury8",3.41}, {"treasury9",3.93}, 
   {"treasury10",3.86}, {"hardware1",11.36}, 
   {"hardware2",17.23}, {"hardware3",16.31}, 
   {"hardware4",21.1}, {"hardware5",13.76}, 
   {"hardware6",9.31}, {"hardware7",16.99}, 
   {"hardware8",24.85}, {"hardware9",18.52}, 
   {"hardware10",12.79}, {"theater1",36.1}, 
   {"theater2",19.4}, {"theater3",23.5}, 
   {"theater4",34.61}, {"theater5",16.91}, 
   {"theater6",27.04}, {"theater7",25.82}, 
   {"theater8",24.99}, {"theater9",36.89}, 
   {"theater10",31.71}, {"telecom1",10.89}, 
   {"telecom2",17.98}, {"telecom3",12.31}, 
   {"telecom4",6.53}, {"telecom5",6.11}, 
   {"telecom6",15.89}, {"telecom7",12.46}, 
   {"telecom8",11.11}, {"telecom9",16.6}, 
   {"telecom10",13.93}, {"brewery1",8.79}, 
   {"brewery2",9.35}, {"brewery3",8.88}, 
   {"brewery4",11.63}, {"brewery5",6.21}, 
   {"brewery6",9.26}, {"brewery7",4.77},
   {"brewery8",5.98}, {"brewery9",10.92}, 
   {"brewery10",5.26}, {"highways1",5.39}, 
   {"highways2",4.85}, {"highways3",8.34}, 
   {"highways4",8.48}, {"highways5",6.42}, 
   {"highways6",11.04}, {"highways7",13.15}, 
   {"highways8",10.01}, {"highways9",7.71}, 
   {"highways10",11.07}, {"cars1",5.74}, 
   {"cars2",7.13}, {"cars3",8.75}, {"cars4",5.08}, {"cars5",6.23}, 
   {"cars6",8.19}, {"cars7",8.03}, {"cars8",5.96}, {"cars9",4.17}, 
   {"cars10",8.11}, {"bank1",6.79}, {"bank2",3.01}, {"bank3",4.92}, 
   {"bank4",4.14}, {"bank5",8.98}, {"bank6",8.81}, {"bank7",4.7}, 
   {"bank8",8.06}, {"bank9",6.22}, {"bank10",4.47}, {"software1",34.59}, 
   {"software2",44.94}, {"software3",43.05}, 
   {"software4",15.58}, {"software5",42.05}, 
   {"software6",20.66}, {"software7",20.76}, 
   {"software8",19.85}, {"software9",20.05}, 
   {"software10",45.38}, {"electronics1",23.07}, 
   {"electronics2",19.1}, {"electronics3",23.83}, 
   {"electronics4",16.54}, {"electronics5",28.33}, 
   {"electronics6",25.88}, {"electronics7",22.14}, 
   {"electronics8",22.65}, {"electronics9",12.59}, {"electronics10",28.1}};

/********************************************************/

int main()
{
 XPRMmodel mod;
 char riskdata_name[40], locdata_name[40], secdata_name[40], retdata_name[40]; 
                                  /* File names of input data */
 char solfracbuy_name[40];        /* File names of solution arrays */
 char objval_name[40], num_name[40], stat_name[40];
 char params[1000];               /* Parameter string for model execution */
 int i,result,numshares,status;
 double objval;
 double maxrisk = 1.0/3;          /* Model parameter settings */
 double minreg = 0.2;
 double maxreg = 0.5;
 double maxsec = 0.25;
 double maxval = 0.2;
 double minval = 0.1;

/* Prepare file names for 'initializations' using the 'raw' driver:         */
/*   "rawoption[,...],filename"                                             */
/*   (Here, 'filename' uses the 'mem' driver, data is stored in memory)     */
/* Options for 'raw':                                                       */
/* 'slength=0': strings are represented by pointers to null terminated      */
/*              arrays of characters (C-string) instead of fixed size arrays*/

 sprintf(riskdata_name, "slength=0,mem:%p/%d", riskset, (int)sizeof(riskset));
 sprintf(locdata_name, "slength=0,mem:%p/%d", locdata, (int)sizeof(locdata));
 sprintf(secdata_name, "slength=0,mem:%p/%d", secdata, (int)sizeof(secdata));
 sprintf(retdata_name, "slength=0,mem:%p/%d", retdata, (int)sizeof(retdata));
 sprintf(solfracbuy_name, "slength=0,mem:%p/%d", sol, (int)sizeof(sol));
 sprintf(objval_name, "mem:%p/%d", &objval, (int)sizeof(objval));
 sprintf(num_name, "mem:%p/%d", &numshares, (int)sizeof(numshares));
 sprintf(stat_name, "mem:%p/%d", &status, (int)sizeof(status));

                                  /* Pass file names as execution param.s */
 sprintf(params, "MAXRISK=%g,MINREG=%g,MAXREG=%g,MAXSEC=%g,MAXVAL=%g,MINVAL=%g,MAXNUM=%d,DATAFILE='raw:',OUTPUTFILE='raw:',RISKDATA='%s',LOCDATA='%s',RETDATA='%s',SECDATA='%s',FRACBUYSOL='%s',RETSOL='%s',NUMSHARES='%s',SOLSTATUS='%s'",
         maxrisk, minreg, maxreg, maxsec, maxval, minval, maxnum,
         riskdata_name, locdata_name, retdata_name, secdata_name,
         solfracbuy_name, objval_name, num_name, stat_name); 


 if(XPRMinit())                   /* Initialize Mosel */
  return 1;

 if(XPRMexecmod(NULL, "foliomemio2.mos", params, &result, &mod))
  return 2;                       /* Execute the model file (only during  
                                     development phase, the deployed 
				     application would only use BIM) */

 if((XPRMgetprobstat(mod)&XPRM_PBRES)!=XPRM_PBOPT)
  return 3;                       /* Stop if no solution available */

                    /* Display solution values obtained from the model */
 printf("Total return: %g\n", objval);
/* Alternatively: 
 printf("Total return: %g\n", XPRMgetobjval(mod));
*/
 for(i=0;i<numshares;i++)
  printf("%s: %g%% (%g)\n", sol[i].ind, sol[i].val1*100, sol[i].val2);

 XPRMresetmod(mod);               /* Reset the model */
 
 return 0;
}

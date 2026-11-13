/*******************************************************
   Mosel Example Problems 
   ======================

   file distfolio.c
   ````````````````
   Running a model on a remote machine,
   passing runtime parameters to the submodel.

   Before running this model, you need to set up the 
   NODENAME with a machine name/address of your local network.
   The node that is used needs to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on this machine.

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Oct 2012
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprd.h"
#include "bindrv.h"

/***************** Reading result data ******************/
struct MySolArray
{
 const char *ind;                 /*   index name */
 double val;                      /*   solution value */
};

struct MySolArray *solfrac;
struct MySolArray *solbuy;

 int getbdintval(s_bindrvctx bdrv)
 {
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;
   bindrv_getint(bdrv,&(val.i));
   return val.i;
 }
 
 double getbddblval(s_bindrvctx bdrv)
 {
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;
   bindrv_getreal(bdrv,&(val.r));
   return val.r;
 }

 /**** Reading a one-dim. array 'ar' of fixed max. size ****/
 static int assignFixedTableVal(s_bindrvctx bdrv, struct MySolArray *ar) 
 {
   int ct,ctrl;
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;

   ct=0;
   bindrv_getctrl(bdrv,&(val.i));
   if (val.i==BINDRV_CTRL_OPENLST)
   {
     while(bindrv_nexttoken(bdrv)>=0) {
     bindrv_getctrl(bdrv,&(val.i));
     ctrl=val.i;
     if(ctrl==BINDRV_CTRL_CLOSELST) break;
     else
       if(ctrl==BINDRV_CTRL_OPENNDX)
       {
         bindrv_getstring(bdrv,&(val.str));
         ar[ct].ind=val.str;
	 bindrv_getctrl(bdrv,&(val.i));
         if(val.i==BINDRV_CTRL_CLOSENDX)
         {
           bindrv_getreal(bdrv,&(val.r));
           ar[ct].val=val.r;
	   ct++;
         }
         else
         {
           printf("Wrong file format. ')' expected.\n");
	   return 1;
         }
       }
       else
       {
         printf("Wrong file format. '(' expected.\n");
	 return 2;
       }
     }
   }
   else
   {
     printf("Wrong file format. '[' expected.\n");
     return 3;
   }  
   return 0;  
 }
 
 static int readSol(char *filename) 
 {
   s_bindrvctx bdrv;
   FILE *f;
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;
   int res=0,num=0,i;

   f=fopen(filename,"r");                        /* Use Mosel bin reader */
   bdrv=bindrv_newreader((size_t (*)(void *,size_t,size_t,void*))fread,f);

   while(bindrv_nexttoken(bdrv)>=0 && res==0) {
     bindrv_getctrl(bdrv,&(val.i));
     if(val.i==BINDRV_CTRL_LABEL)
     {
       bindrv_getstring(bdrv,&(val.str));
       if(strcmp(val.str,"RETSOL")==0)
        printf("Total return: %g\n", getbddblval(bdrv));
       else if(strcmp(val.str,"NUMSHARES")==0)
       {
        num = getbdintval(bdrv);
        printf("Number of shares: %d\n", num);
       }	
       else if(strcmp(val.str,"SOLCOUNT")==0)
        printf("Solution number: %d\n", getbdintval(bdrv));
       else if(strcmp(val.str,"SOLSTATUS")==0)
        printf("Solution status: %d\n", getbdintval(bdrv));
       else if (strcmp(val.str,"BUY")==0)
         res=assignFixedTableVal(bdrv, solbuy);
       else if (strcmp(val.str,"FRAC")==0)
         res=assignFixedTableVal(bdrv, solfrac);
       else printf("Unexpected label: %s", val.str);
     }
     else
     {
       printf("Wrong file format\n");
       res=4;
     }  
   }

   bindrv_delete(bdrv);
   fclose(f);

   for(i=0;i<num;i++)
    printf("%s: %g%% (%g)\n", 
           solfrac[i].ind, solfrac[i].val*100, solbuy[i].val); 

   return res;
 }
/***************** Main ******************/

int main(int argv,char *args[]) 
{
  XPRDcontext xprd;
  XPRDmosel mosel;
  XPRDmodel mod;
  char params[1000];
  double maxrisk = 1.0/3;          /* Model parameter settings */
  double minreg = 0.2;
  double maxreg = 0.5;
  double maxsec = 0.25;
  double maxval = 0.2;
  double minval = 0.1;
  int maxnum = 9;
       /* Use the name or IP address of a machine in
        * your local network, or "" for current node */
  char *NODENAME = "";

  solfrac = (struct MySolArray *)malloc(maxnum * sizeof(struct MySolArray));
  solbuy = (struct MySolArray *)malloc(maxnum * sizeof(struct MySolArray));

  xprd=XPRDinit();             /* Create an XPRD context */
                               /* Open connection to a remote node */
  mosel=XPRDconnect(xprd, NODENAME, NULL, NULL, NULL, 0);
                               /* Compile the model file */
  XPRDcompmod(mosel, "", "rmt:foliomemio.mos", "rmt:foliomemio.bim", "");
                               /* Load the bim file into the remote instance */
  mod=XPRDloadmod(mosel, "rmt:foliomemio.bim"); 
  remove("foliomemio.bim");    /* Cleaning up */
                               /* Run-time parameters */
  sprintf(params, "MAXRISK=%.16f,MINREG=%g,MAXREG=%g,MAXSEC=%g,MAXVAL=%g,MINVAL=%g,MAXNUM=%d,DATAFILE='rmt:folio250.dat',OUTPUTFILE='bin:rmt:solfile.dat'",
         maxrisk, minreg, maxreg, maxsec, maxval, minval, maxnum ); 
  XPRDrunmod(mod, params);     /* Run the model */
  XPRDwaitevent(xprd,-1);      /* Wait for model termination */
  XPRDdropevent(xprd);         /* Ignore termination event message */

  printf("Model returned: %d\n", XPRDgetexitcode(mod));
  readSol("solfile.dat");      /* Display the solution */
  remove("solfile.dat");

  XPRDunloadmod(mod);          /* Unload the model */
  XPRDdisconnect(mosel);       /* Disconnect remote instance */
  XPRDfinish(xprd);            /* Terminate XPRD */
  return 0;
} 

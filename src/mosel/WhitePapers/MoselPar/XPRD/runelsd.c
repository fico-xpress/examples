/*******************************************************
   Mosel Example Problems 
   ======================

   file runelsd.c
   ``````````````
   Run several instances of the model elsd.mos in
   parallel and coordinate the solution update.

   Before running this program, you need to set up the array
   NodeNames with machine names/addresses of your local network.
   All nodes that are used need to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on these machines.
   
   All files are local to the root node, no write access is
   required at remote nodes.

  *** The model started by this program cannot be run with 
      a Community Licence for the provided data instance ***

   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Apr 2012, rev. Jul. 2014
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "xprd.h"
#include "bindrv.h"

#define DATAFILE "els.dat"
#define T 15                   /* Time periods */
#define P 4                    /* No. of products */
#define I 2                    /* No. of (remote) Mosel instances */
#define M 6                    /* No. of Mosel models */

#define NEWSOL 2               /* Identifier for "sol. found" event */
#define M1 1
#define M2 3

/* Setting up remote connections:
 * Use machine names within your local network, IP addresses, or 
 * empty string for the current node running this model */
static char *NodeNames[] = {"", "localhost"};

double **solprod;              /* Sol. values for var.s produce */
double **solsetup;             /* Sol. values for var.s setup */
double **DEMAND;               /* Demand per period */

static void *dummy_malloc(size_t s,void *ctx);

/***************** Reading result data ******************/
/**** Read a table of the form 'double ar[][]' ****/
/*
 static int assignTableVal(s_bindrvctx bdrv, double **ar) 
 {
   int i,j,ctrl;
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;

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
         bindrv_getint(bdrv,&(val.i));
         i=val.i;
         bindrv_getint(bdrv,&(val.i));
         j=val.i;
	 bindrv_getctrl(bdrv,&(val.i));
         if(val.i==BINDRV_CTRL_CLOSENDX)
         {
           bindrv_getreal(bdrv,&(val.r));
           ar[i-1][j-1]=val.r;
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
*/

/**** Fixed size version for reading 'double ar[][]' ****/
 static int assignFixedTableVal(s_bindrvctx bdrv, double **ar) 
 {
   int i,j,ctrl,j2,c;
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;

   c=bindrv_getctrl(bdrv,&(val.i));
   if (val.i==BINDRV_CTRL_OPENLST)
   {
     while(bindrv_nexttoken(bdrv)>=0) {
     c=bindrv_getctrl(bdrv,&(val.i));
     ctrl=val.i;
     if(ctrl==BINDRV_CTRL_CLOSELST) break;
     else
       if(ctrl==BINDRV_CTRL_OPENNDX)
       {
         bindrv_getint(bdrv,&(val.i));
         i=val.i;
         bindrv_getint(bdrv,&(val.i));
         j=val.i;
	 bindrv_getctrl(bdrv,&(val.i));
         if(val.i==BINDRV_CTRL_CLOSENDX)
         {
           for(j2=j;j2<=T;j2++) 
           {
             bindrv_getreal(bdrv,&(val.r));
             ar[i-1][j2-1]=val.r;
           }
         }
         else
         {
           printf("Wrong file format. ')' expected, found %d/%d.\n",c,val.i);
	   return 1;
         }
       }
       else
       {
         printf("Wrong file format. '(' expected, found %d/%d.\n",c,val.i);
	 return 2;
       }
     }
   }
   else
   {
     printf("Wrong file format. '[' expected, found %d/%d.\n",c,val.i);
     return 3;
   }  
   return 0;  
 }
 
/**** Identify the label to read ****/
 static int readSol(char *filename) 
 {
   s_bindrvctx bdrv;
   FILE *f;
   union { int i; double r; char b; char *str; BINDRV_LONG l;} val;
   int res=0;

   f=fopen(filename,"r");                        /* Use Mosel bin reader */
   bdrv=bindrv_newreader((size_t (*)(void *,size_t,size_t,void*))fread,f);

 /* By default the library will allocate strings (returned by 'getstring') */
 /* using 'malloc'. The following routine allows to replace 'malloc' by some */
 /* user-defined memory allocation routine (that gets a context as an */
 /* extra parameter) */
   bindrv_setalloc(bdrv,dummy_malloc,NULL);

   while(bindrv_nexttoken(bdrv)>=0 && res==0) {
     bindrv_getctrl(bdrv,&(val.i));
     if(val.i==BINDRV_CTRL_LABEL)
     {
       bindrv_getstring(bdrv,&(val.str));
       printf("Reading %s\n", val.str);
       if (strcmp(val.str,"solprod")==0)
         res=assignFixedTableVal(bdrv, solprod);
       else if (strcmp(val.str,"solsetup")==0)
         res=assignFixedTableVal(bdrv, solsetup);
       else if (strcmp(val.str,"DEMAND")==0)
         res=assignFixedTableVal(bdrv, DEMAND);
       /* Unknow labels are simply ignored */
     }
     else
     {
       printf("Wrong file format\n");
       res=4;
     }  
   }

   bindrv_delete(bdrv);
   fclose(f);
   
   return res;
 }

/**** A dummy malloc function using a static region ****/
static void *dummy_malloc(size_t s,void *ctx)
{
 static char buf[1024];

 return (s>1024)?NULL:buf;
}

/****** Run a submodel to read demand data for pretty solution printing ******/
 static int readDem(XPRDcontext xprd,XPRDmosel moselInst)
 {
   int e;
   char params[200];
   XPRDmodel modDem = NULL;                /* Model */

                                           /* Compile the model */
   if((e=XPRDcompmod(moselInst, "", "rmt:readelsdem.mos", 
                                    "rmt:readelsdem.bim", ""))!=0)
   {
     printf("Compilation failed: %d\n", e);
     return 2;
   }
                                          /* Load model */
   modDem = XPRDloadmod(moselInst, "rmt:readelsdem.bim");
   if(modDem==NULL)
   {
     printf("Loading failed\n");
     return 3;
   }  

   sprintf(params, "DATAFILE=%s,T=%d,P=%d", DATAFILE, T, P); 
   if(XPRDrunmod(modDem, params)!=0)       /* Run the model */
   {
     printf("Failed to run model\n");
     return 4;
   }
   XPRDwaitevent(xprd,-1);                 /* Wait for termination event */

   readSol("Demand");                      /* Read the demand data */

   remove("readelsdem.bim");               /* Cleaning up temporary files */
   remove("Demand");

   return 0;
 }

/***************** Main ******************/

 int main(int argv,char *args[])
 {
  XPRDcontext xprd;
  XPRDmosel moselInst[I];                  /* Mosel instances */
  XPRDmodel modELS[M],evsender;            /* Models */
  char msg[256];
  char params[200];
  double objval,evvalue,sp;
  int algsol,algopt,p,e,i,m,t,evclass,senderid;

  DEMAND = (double **)malloc(P * sizeof(double *));
  for(p=0;p<P;p++) DEMAND[p] = (double *)malloc(T * sizeof(double));
  solprod = (double **)malloc(P * sizeof(double *));
  for(p=0;p<P;p++) solprod[p] = (double *)malloc(T * sizeof(double));
  solsetup = (double **)malloc(P * sizeof(double *));
  for(p=0;p<P;p++) solsetup[p] = (double *)malloc(T * sizeof(double));

  xprd=XPRDinit();                        /* Create an XPRD context */

  for(i=0;i<I;i++)                        /* Establish remote connections */
  {
    moselInst[i]=XPRDconnect(xprd, NodeNames[i], NULL, NULL, msg,sizeof(msg));       if(moselInst[i]==NULL)
    {
      printf("Connection failed: %s\n", msg);
      return 1;
    }  
  }
                                          /* Compile the model */
  if((e=XPRDcompmod(moselInst[0], "", "rmt:elsd.mos", "rmt:elsd.bim", ""))!=0)
  {
    printf("Compilation failed: %d\n", e);
    return 2;
  }
      
                                         /* Load models */
  modELS[M1-1] = XPRDloadmod(moselInst[0], "rmt:elsd.bim");
  modELS[M2-1] = XPRDloadmod(moselInst[1], "rmt:elsd.bim");
  if((modELS[M1-1]==NULL)||(modELS[M2-1]==NULL))
  {
    printf("Loading failed\n");
    return 3;
  }  

                                         /* Run the models */
  sprintf(params, "ALG=%d,DATAFILE=%s,T=%d,P=%d", M1, DATAFILE, T, P); 
  if(XPRDrunmod(modELS[M1-1], params)!=0)
  {
    printf("Failed to run model %d\n", M1);
    return 4;
  }
  sprintf(params, "ALG=%d,DATAFILE=%s,T=%d,P=%d", M2, DATAFILE, T, P); 
  if(XPRDrunmod(modELS[M2-1], params)!=0)
  {
    printf("Failed to run model %d\n", M2);
    return 4;
  }

  objval = 1.0e+20;  
  algsol = -1; algopt = -1;

  do
  {
    XPRDwaitevent(xprd,-1);        /* Wait for the next event */
    XPRDgetevent(xprd, &evsender, &evclass, &evvalue);    /* Get the event */
    senderid = (evsender==modELS[M1-1]?M1:M2);
    if (evclass==NEWSOL)           /* Test the event class */
    {
      if (evvalue<objval)          /* Value of the event (= obj. value) */
      {                            /* ID of model sending the event */
        algsol = senderid;
	objval = evvalue;
        printf("Improved solution %g found by model %d\n", objval, algsol);
	for(m=0;m<M;m++)
          if (m!=algsol && (m==M1-1 || m==M2-1))
            XPRDsendevent(modELS[m], NEWSOL, objval);
      }
      else
        printf("Solution %g found by model %d\n", evvalue, senderid);
    }
  } while (evclass!=XPRD_EVENT_END);        /* No model has finished */
                           
  algopt = senderid;               /* Retrieve ID of terminated model */
  for(m=0;m<M;m++)
    if (m==M1-1 || m==M2-1)
      XPRDstoprunmod(modELS[m]);   /* Stop all running models */

  while((XPRDgetstatus(modELS[M1-1])==XPRD_RT_RUNNING)||
        (XPRDgetstatus(modELS[M2-1])==XPRD_RT_RUNNING)||
        !XPRDqueueempty(xprd))
  {
    XPRDwaitevent(xprd,-1);        /* Wait for the next event */
    XPRDdropevent(xprd);           /* Ignore termination event */
  }

  if (algsol==-1)
  {
    printf("No solution available");
    return 1;
  }
  else                     
  {                  /* Retrieve the best solution from shared memory */
    sprintf(params, "sol_%d", algsol);
    if((e=readSol(params))!=0) 
    {
      printf("%d Could not read file %d\n", e, algsol);
      return 5;
    }
                     /* Read the demand data by running another Mosel model */
    if((e=readDem(xprd,moselInst[0]))!=0) 
    {
      printf("Could not read demand data - %d\n", e);
      return 6;
    }
  
  /* Solution printing */
    printf("Best solution found by model %d\n", algsol); 
    printf("Optimality proven by model %d\n", algopt); 
    printf("Objective value: %g\n", objval); 
    printf("Period  setup\n");
    for(p=0;p<P;p++) printf("      %d", (p+1));
    for(t=0;t<T;t++) 
    {	  
      sp=0;
      for(p=0;p<P;p++) sp+=solsetup[p][t];
      printf("\n %2d%8.0f    ", (t+1), sp);
      for(p=0;p<P;p++)
        printf("%3.0f (%1.0f)", solprod[p][t], DEMAND[p][t]);
    }
    printf("\n");
  }


/* Cleaning up temporary files */
  for(m=0;m<M;m++) 
    if (m==M1 || m==M2)
    {
      sprintf(params, "sol_%d", m);
      remove(params);
    }
  remove("elsd.bim");

  for(i=0;i<I;i++)                /* Terminate remote connections */
    XPRDdisconnect(moselInst[i]); 
  XPRDfinish(xprd);               /* Terminate XPRD */
  
  return 0;
}

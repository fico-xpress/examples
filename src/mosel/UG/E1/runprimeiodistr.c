/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file runprimeiodistr.c
   ``````````````````````
   Distributed computing: 
   Running a model on a remote Mosel instance
   and retrieving output data.

   - Data saved on the remote machine running the Mosel model -
     (Submodel writing to "bin:shmem:" or "bin:tmp:")
       
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Jan. 2013
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprd.h"
#include "bindrv.h"

#define STOPMOD 2            /* Identifier for "Stop submodel" user event */
#define MODREADY 3           /* Identifier for "Submodel ready" user event */

/*********************************/
/* Wrapper function for 'bindrv' */
/*********************************/
static size_t my_read(void *buf, size_t size, size_t nmemb, void *ctx)
{
 size_t s,a;
 long t;
 
 s=size*nmemb;
 a=0;
 while(s>0)
 {
  t=XPRDfread(ctx,(char*)buf+a,s);
  if(t<=0) break;
  else
  {
   a+=t;
   s-=t;
  }
 }
 return a/size;
}

/********************************************************/
/* Using bindrv 
   Decode the binary file and display its content       */
/********************************************************/
void show_solution(size_t (*doread)(void *,size_t,size_t,void*), void *rctx)
{
 s_bindrvctx bdrv;
 int *solarr;
 int size,i,n;
 char *str;

 bdrv=bindrv_newreader(doread,rctx);  /* Initialize binreader */

 i=size=0;
 solarr=NULL;
 while(bindrv_nexttoken(bdrv)>=0)
 {
  bindrv_getctrl(bdrv,&n);         /* 'label'  (marker) */
  bindrv_getstring(bdrv,&str);        /* Read a string */
  if(strcmp(str,"NumP")==0)
  {
   free(str);
   bindrv_getint(bdrv,&size);         /* Read an integer */
   printf("( %d prime numbers)\n", size);
   if(size>0)                         /* Prepare array to receive values */
    solarr=malloc(sizeof(int)*size);
   else
    break;
  } 
  else
  if(strcmp(str,"SPrime")==0)
  {
   free(str);
   bindrv_getctrl(bdrv,&n);       /* [  (start marker) */
   while(bindrv_nexttoken(bdrv)==BINDRV_TYP_INT)
   {                                  /* Read integers */
    bindrv_getint(bdrv,&(solarr[i++]));
   }
   bindrv_getctrl(bdrv,&n);        /* ]  (end marker) */
  }
  else
  {
   printf("Unexpected label: %s\n", str);
   free(str);
   exit(1);
  }
 }

 bindrv_delete(bdrv);              /* Release bin reader */

/* Print the set of prime numbers */
 printf("Prime numbers={");
 for(i=0;i<size;i++)
  printf(" %d",solarr[i]);
 printf(" }\n");
 
 free(solarr);                     /* Clean up */
}

/********************************************************/
int main(int argv,char *args[]) 
{
  XPRDcontext xprd;
  XPRDmosel moselInst;
  XPRDmodel modPrime, evsender;
  double evvalue;
  int evclass;
  XPRDfile f;

  xprd=XPRDinit();              /* Create an XPRD context */
                                /* Open connection to a remote node:
			           "" means the node running this program */
  moselInst=XPRDconnect(xprd, "", NULL, NULL, NULL, 0);
                                /* Compile the model file */
  XPRDcompmod(moselInst, "", "rmt:primeio.mos", "tmp:primeio.bim", "");
                                /* Load the bim file into the remote instance */
  modPrime=XPRDloadmod(moselInst, "tmp:primeio.bim"); 

                                /* Disable submodel output */
  XPRDsetdefstream(moselInst, modPrime, XPRD_F_WRITE, "null:");

                                /* Start execution */
  XPRDrunmod(modPrime, "LIMIT=50000,OUTPUTFILE=bin:shmem:resdata"); 
  XPRDwaitevent(xprd,0);        /* Wait for an event */
  XPRDgetevent(xprd, &evsender, &evclass, &evvalue);    /* Get the event */
  if (evclass != MODREADY)      /* Check the event class */
  {
    printf("Problem with submodel run");
    return 1;
  } 

  XPRDwaitevent(xprd,2);        /* Wait 2 seconds for an event */

  if (XPRDqueueempty(xprd)==1)  /* No event has been sent */
  {
   printf("Model too slow: stopping it!\n");
   XPRDsendevent(modPrime, STOPMOD, 0);    /* Stop the model, then  */
   XPRDwaitevent(xprd,-1);      /* wait for its termination */
  } 

  /* Open the output file, retrieve and display the solution data */
  f=XPRDfopen(moselInst, "shmem:resdata", XPRD_F_BINARY|XPRD_F_INPUT, NULL,0);
  show_solution(my_read,f);
  XPRDfclose(f);

  XPRDunloadmod(modPrime);      /* Unload the model */
  XPRDdisconnect(moselInst);    /* Disconnect remote instance */
  XPRDfinish(xprd);             /* Terminate XPRD */

  return 0;
} 

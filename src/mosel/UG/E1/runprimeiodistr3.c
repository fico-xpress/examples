/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file runprimeiodistr3.c
   ```````````````````````
   Distributed computing: 
   Running a model on a remote Mosel instance
   and retrieving output data.
   
   - Data saved to a file on the local machine running XPRD -
     (Submodel writing to "bin:rmt:")
       
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


/********************************************************/
/* Using bindrv 
   Decode the binary file and display its content       */
/********************************************************/
void show_solution(size_t (*doread)(void *,size_t,size_t,void*),void *rctx)
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
   bindrv_getctrl(bdrv,&n);        /* [  (start marker) */
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
 printf("primes={");
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
  FILE *f;

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
  XPRDrunmod(modPrime, "LIMIT=50000,OUTPUTFILE=bin:rmt:resdata"); 
  XPRDwaitevent(xprd,0);        /* Wait for an event */
  XPRDgetevent(xprd, &evsender, &evclass, &evvalue);    /* Get the event */
  if (evclass != MODREADY) 
  {
    printf("Problem with submodel run");
    return 1;
  } 

  XPRDwaitevent(xprd,2);        /* Wait 2 seconds for an event */

  if (XPRDqueueempty(xprd)==1)  /* No event has been sent... */
  {
   printf("Model too slow: stopping it!\n");
   XPRDsendevent(modPrime, STOPMOD, 0);    /* ... stop the model, then wait */
   XPRDwaitevent(xprd,-1);
  }

  /* Open the output file, retrieve and display the solution data */
  f=fopen("resdata","rb");      /* Open file for reading in binary format */
  show_solution((size_t (*)(void *, size_t,  size_t,  void *))fread,f);
  fclose(f);

  XPRDunloadmod(modPrime);      /* Unload the model */
  XPRDdisconnect(moselInst);    /* Disconnect remote instance */
  XPRDfinish(xprd);             /* Terminate XPRD */

  remove("resdata");            /* Clean up temporary files */

  return 0;
} 

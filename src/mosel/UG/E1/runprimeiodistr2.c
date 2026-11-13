/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file runprimeiodistr2.c
   ```````````````````````
   Distributed computing: 
   Running a model on a remote Mosel instance
   and retrieving output data.
   
   - Data saved in memory on the local machine running XPRD -
   - Implementation of a file manager for data exchange in memory -
     (Submodel writing to "bin:rmt:")
       
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke & Y. Colombani, Jan. 2013
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprd.h"
#include "bindrv.h"

#define STOPMOD 2            /* Identifier for "Stop submodel" user event */
#define MODREADY 3           /* Identifier for "Submodel ready" user event */

static char datafile[20*1024];	 /* Memory block to store the output data */
                                 /* (using static size for simplicity's sake) */
static int datafilesize;

typedef struct
	{
	 char *buffer;
	 int mode;
	 int bufsize;
	 int curpos;
	} s_memfile;

/********************************************************
   Implementation of a file manager 
   This file manager accesses directly file 'resdata'
   in local memory block 
   Warning: concurrency not handled here!!!
 ********************************************************/

void* XPRD_CC my_open(void *ctx, char *filename, int, XPRDfct_data* fct_data, 
       XPRDfct_close* fct_close, XPRDfct_skip *fct_skip, char *msg, int msglen);
int XPRD_CC my_close(void *data);
int XPRD_CC my_read(void *data, char *buf, int size);
int XPRD_CC my_write(void *data, char *buf, int size);

void* XPRD_CC my_open(void *ctx, char *filename, int mode, 
                      XPRDfct_data* fct_data, XPRDfct_close* fct_close, 
                      XPRDfct_skip *fct_skip, char *msg, int msglen)
{
 s_memfile *memfile;

 if(strcmp(filename,"resdata")==0)
 {
  if((memfile=malloc(sizeof(s_memfile)))==NULL)
  {
   strncpy(msg,"memory error",msglen);
   return XPRD_FMGR_ERR;
  }
  else
  {
   memfile->buffer=datafile;
   memfile->mode=mode&(XPRD_F_READ|XPRD_F_WRITE);      
   memfile->bufsize=(memfile->mode==XPRD_F_READ)?datafilesize:sizeof(datafile);
   memfile->curpos=0;
   *fct_data=(memfile->mode==XPRD_F_READ)?my_read:my_write;
   *fct_close=my_close;
   *fct_skip=NULL;
   return memfile;
  }
 }
 else
  return NULL;               /*  Use default behaviour (open local file) */
}

/**** Release resources used by a memory file ****/
int XPRD_CC my_close(void *data)
{
 s_memfile *memfile;

 memfile=data;
 /* save size of datafile */
 if((memfile->mode==XPRD_F_WRITE)&&(memfile->buffer==datafile))
  datafilesize=memfile->curpos;
 free(data);
 return 0;
}

/**** Send data from a block of memory ****/
int XPRD_CC my_read(void *data,char *buf,int size)
{
 s_memfile *memfile;
 int l;

 memfile=data;
 if(memfile->curpos>=memfile->bufsize)           /* end of file */
  return 0;
 else
 if((l=memfile->bufsize-memfile->curpos)<=size)  /* last block */
 {
  memcpy(buf,memfile->buffer+memfile->curpos,l);
  memfile->curpos=memfile->bufsize;
  return l;
 }
 else
 {
  memcpy(buf,memfile->buffer+memfile->curpos,size);
  memfile->curpos+=size;
  return size;
 }
}

/**** Reading function with signature as required by the bin driver ****/
size_t my_fread(void *buf,size_t l,size_t nm,void *fd)
{
 return (my_read(fd,buf,nm*l)==l*nm)?nm:0;
}

/**** Store data in a block of memory ****/
int XPRD_CC my_write(void *data,char *buf,int size)
{
 s_memfile *memfile;
 memfile=data;
 if(memfile->curpos+size>=memfile->bufsize)     /* buffer overflow */
  return -1;
 else
 {
  memcpy(memfile->buffer+memfile->curpos,buf,size);
  memfile->curpos+=size;
  return size;
 }
}

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
  s_memfile *f;
  XPRDfct_data fct_data; 
  XPRDfct_close fct_close;
  XPRDfct_skip fct_skip;

  xprd=XPRDinit();              /* Create an XPRD context */
                                /* Open connection to a remote node:
			           "" means the node running this program */
  moselInst=XPRDconnect(xprd, "", my_open, NULL, NULL, 0);
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
  f=my_open(NULL,"resdata",XPRD_F_READ,&fct_data,&fct_close,&fct_skip,NULL,0);
  show_solution(my_fread,f);
  my_close(f);

  XPRDunloadmod(modPrime);       /* Unload the model */
  XPRDdisconnect(moselInst);     /* Disconnect remote instance */
  XPRDfinish(xprd);              /* Terminate XPRD */

  return 0;
} 

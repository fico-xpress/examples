/*******************************************************
   Mosel Example Problems
   ====================== 

   file distfoliocbioev.c
   ``````````````````````
   Running a Mosel model from a C application
   with data exchange between model and host application.
   (Use Mosel running remotely controlled via XPRD)
   - Using the bindrv utility -

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2011 Fair Isaac Corporation
       author: S. Heipcke, Oct. 2011, rev. Apr 2012
********************************************************/

/*
cc -I$MOSEL/include -L$MOSEL/lib distfoliocbioev.c -o distfoliocbioev -lpthread -lxprd -lbindrv -lxprnls
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprd.h"
#include "bindrv.h"

#define NEWSOL 2
#define SOLREAD 3

static char datafile[10*1024];	 /* Memory block to store the output data */
                                 /* (using static size for simplicity's sake) */
static int datafilesize;

typedef struct
	{
	 char *buffer;
	 int mode;
	 int bufsize;
	 int curpos;
	} s_memfile;

          /* Array to receive solution values */
struct MySolArray
{
 char *ind;                       /*   index name */
 double val;                      /*   solution value */
 struct MySolArray *next;
};

struct MySolArray *solfrac;
struct MySolArray *solbuy;


/********************************************************
   Implementation of a file manager 
   This file manager accesses directly file 'solfile'
   in local memory block 
   Warning: concurrency not handled here!!!
 ********************************************************/

void* XPRD_CC my_open(void *ctx, char *filename, int, XPRDfct_data* fct_data, 
       XPRDfct_close* fct_close, XPRDfct_skip *fct_skip, char *msg, int msglen);
int XPRD_CC my_close(void *data);
int XPRD_CC my_read(void *data, char *buf, int size);
int XPRD_CC my_write(void *data, char *buf, int size);
int XPRD_CC outremote(void *data, char *buf, int size);

void* XPRD_CC my_open(void *ctx, char *filename, int mode, 
                      XPRDfct_data* fct_data, XPRDfct_close* fct_close, 
                      XPRDfct_skip *fct_skip, char *msg, int msglen)
{
 s_memfile *memfile;

 if(strcmp(filename,"outremote")==0)
 {
  if((mode&(XPRD_F_READ|XPRD_F_WRITE))!=XPRD_F_WRITE)
  {
   strncpy(msg, "'outremote' is write only.", msglen);
   return XPRD_FMGR_ERR;
  }
  else
  {
   *fct_data=outremote;
   *fct_close=NULL;
   return (void*)1;
  }
 }
 if(strcmp(filename,"solfile")==0)
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
 if(memfile->curpos>=memfile->bufsize)        /* end of file */
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
 if(memfile->curpos+size>=memfile->bufsize)   /* buffer overflow */
  return -1;
 else
 {
  memcpy(memfile->buffer+memfile->curpos,buf,size);
  memfile->curpos+=size;
  return size;
 }
}

/**** Display received message with a prefix ****/
int XPRD_CC outremote(void *data,char *buf,int size)
{
 printf("REMOTE: %.*s",size,buf);
 return size;
}


/********************************************************/
/* Using bindrv 
   Decode the binary file and display its content       */
/********************************************************/
void show_solution()
{
 s_memfile *f;
 s_bindrvctx bdrv;
 union { int i; double r; char b; char *str; BINDRV_LONG l;} val;
 char *index;
 struct MySolArray *solfracf,*solfracl,*solfracn;
 struct MySolArray *solbuyf,*solbuyl,*solbuyn;
 XPRDfct_data fct_data; 
 XPRDfct_close fct_close;
 XPRDfct_skip fct_skip;


 f=my_open(NULL,"solfile",XPRD_F_READ,&fct_data,&fct_close,&fct_skip,NULL,0);
 bdrv=bindrv_newreader(my_fread,f);


  while(bindrv_nexttoken(bdrv)>=0)
  {
   bindrv_getctrl(bdrv,&(val.i));         /* 'label' */
   bindrv_getstring(bdrv,&(val.str));
   if(strcmp(val.str,"RETSOL")==0)
   {
    free(val.str);
    bindrv_getreal(bdrv,&(val.r));
    printf("Total return: %g\n", val.r);
   } 
   else
   if(strcmp(val.str,"NUMSHARES")==0)
   {
    free(val.str);
    bindrv_getint(bdrv,&(val.i));
    printf("Number of shares: %d\n", val.i);
   } 
   else
   if(strcmp(val.str,"SOLCOUNT")==0)
   {
    free(val.str);
    bindrv_getint(bdrv,&(val.i));
    printf("Solution number: %d\n", val.i);
   } 
   else
   if(strcmp(val.str,"BUY")==0)
   {
    free(val.str);
    bindrv_getctrl(bdrv,&(val.i));    /* [ */
    bindrv_getctrl(bdrv,&(val.i));    /* ( or ] at end of list */
    solbuyf=NULL;
    while(val.i==BINDRV_CTRL_OPENNDX)
    {
     bindrv_getstring(bdrv,&index);
     bindrv_getctrl(bdrv,&(val.i));   /* ) */
     bindrv_getreal(bdrv,&(val.r));
     solbuyn = (struct MySolArray *)malloc(sizeof(struct MySolArray));
     solbuyn->next=NULL;
     if(solbuyf==NULL)
      solbuyf=solbuyl=solbuyn;
     else
     {
      solbuyl->next=solbuyn;
      solbuyl=solbuyn;
     }
     solbuyn->ind=index;
     solbuyn->val=val.r;
     bindrv_getctrl(bdrv,&(val.i));   /* ( or ] at end of list */
    }
    solbuy=solbuyf;
   }
   else
   if(strcmp(val.str,"FRAC")==0)
   {
    free(val.str);
    bindrv_getctrl(bdrv,&(val.i));    /* [ */
    bindrv_getctrl(bdrv,&(val.i));    /* ( or ] at end of list */
    solfracf=NULL;
    while(val.i==BINDRV_CTRL_OPENNDX)
    {
     bindrv_getstring(bdrv,&index);
     bindrv_getctrl(bdrv,&(val.i));   /* ) */
     bindrv_getreal(bdrv,&(val.r));
     solfracn = (struct MySolArray *)malloc(sizeof(struct MySolArray));
     solfracn->next=NULL;
     if(solfracf==NULL)
      solfracf=solfracl=solfracn;
     else
     {
      solfracl->next=solfracn;
      solfracl=solfracn;
     }
     solfracn->ind=index;
     solfracn->val=val.r;
     bindrv_getctrl(bdrv,&(val.i));   /* ( or ] at end of list */
    }
    solfrac=solfracf;
   }
   else
   {
    printf("Unexpected label: %s\n", val.str);
    free(val.str);
    exit(1);
   }
  }

 bindrv_delete(bdrv);                 /* Release bin reader */
 my_close(f);

 /* Print the solution values */
  solfracn=solfrac;
  solbuyn=solbuy;
  while(solfracn!=NULL && solbuyn!=NULL)
  {           /* This assumes that array indices are in the same order */
   printf(" %s: %g%% (%g)\n",  solfracn->ind, solfracn->val*100, solbuyn->val);
   solfracn=solfracn->next;
   solbuyn=solbuyn->next;
  }

  /* Clean up */
  solfracn=solfrac;
  while (solfracn!=NULL)
  {
   solfracn=solfrac->next;
   solfrac->next=NULL;
   free(solfrac->ind);
   free(solfrac);
   solfrac=solfracn;
  }
  solbuyn=solbuy;
  while (solbuyn!=NULL)
  {
   solbuyn=solbuy->next;
   solbuy->next=NULL;
   free(solbuy->ind);
   free(solbuy);
   solbuy=solbuyn;
  }

}

/********************************************************/

int main()
{
 char msg[256],params[1000];
 XPRDcontext xprd;
 XPRDmosel mosel;
 XPRDmodel mod,evmod;
 double maxrisk = 1.0/3;          /* Model parameter settings */
 double minreg = 0.2;
 double maxreg = 0.5;
 double maxsec = 0.25;
 double maxval = 0.2;
 double minval = 0.1;
 int maxnum = 15;
 double val;
 int i,evclass;

 sprintf(params, "MAXRISK=%g,MINREG=%g,MAXREG=%g,MAXSEC=%g,MAXVAL=%g,MINVAL=%g,MAXNUM=%d,DATAFILE='rmt:folio250.dat',OUTPUTFILE='bin:rmt:solfile'",
         maxrisk, minreg, maxreg, maxsec, maxval, minval, maxnum); 


 xprd=XPRDinit();                 /* Initialize XPRD */

          /* Create a new Mosel instance with a user-defined file manager */
 mosel=XPRDconnect(xprd, "", my_open, NULL, msg, sizeof(msg));
 if(mosel==NULL)
 {
   printf("Connection failed: %s\n",msg);
   return 1;
 }
 printf("Connected to:\n%s\n", XPRDbanner(mosel));
 printf("Host: %s\n", XPRDsysinfo(mosel,XPRD_SYS_NODE,msg,sizeof(msg)));

          /* Set error stream of the remote instance to a local file */
 if(XPRDsetdefstream(mosel, NULL, XPRD_F_ERROR, "rmt:err.txt")!=0)
  printf("Failed to change error stream.\n");

                                  /* Compile the model on the remote instance */
 if((i=XPRDcompmod(mosel,"","rmt:foliocbioev.mos","tmp:foliocbioev.bim",""))!=0)
 {
  printf("Compilation failed: Error %d\n",i);
  return 2;
 }
                                  /* Load the model */
 mod=XPRDloadmod(mosel, "tmp:foliocbioev.bim");
 if(mod==NULL)
 {
  printf("Failed to load model\n");
  return 3;
 }

          /* Redirect output stream of the model to the 'outremote' callback */
 if(XPRDsetdefstream(NULL,mod,XPRD_F_OUTPUT|XPRD_F_LINBUF, "rmt:outremote")!=0)
  printf("Failed to change output stream\n");

 XPRDrunmod(mod,params);          /* Run the model */
 do
 {
  XPRDwaitevent(xprd, 0);         /* Wait for an event */
  XPRDgetevent(xprd,&evmod,&evclass,&val);
   if (evclass==NEWSOL)
   {
     show_solution();             /* Decode & display solution data */
     XPRDsendevent(mod, SOLREAD, 0);
   }
 } while(evclass==NEWSOL);        /* Only other event sent by model is "END" */

  if(XPRDgetstatus(mod) != XPRD_RT_OK){
   printf("Error during model execution: %d\n", XPRDgetstatus(mod));
   return 4;
  }
  if(XPRDgetexitcode(mod) != 0){
   printf("Problem not optimal: %d\n", XPRDgetexitcode(mod));
   return 5;
  }                               /* Stop if no solution available */

 XPRDdisconnect(mosel);           /* Disconnect remote instance */
 
 XPRDfinish(xprd);                /* Release context */

 return 0;
}

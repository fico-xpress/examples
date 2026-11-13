/********************************************************/
/*  XPRD library example                                */
/*  ====================                                */
/*                                                      */
/*  file fmgr.c                                         */
/*  ```````````                                         */
/* Example of use of XPRD:                              */
/*                    implementation of a file manager  */
/*                                                      */
/*  (c) 2011 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2011                      */
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprd.h"

static char *srcbuf=
	"model a\n"
	"uses 'mmjobs';\n"
	"send(9,-1)\n"
	"wait\n"
	"writeln('Got:',getnextevent)\n"
	"fopen('tmp:file.txt',F_INPUT)\n"
	"s:='';readln(s); fclose(F_INPUT)\n"
	"writeln('Read:',s)\n"
	"writeln('my ID:',getparam('jobid'))\n"
	"send(10,12.5)\n"
	"fopen('tmp:file.txt',F_OUTPUT)\n"
	"writeln('Some text from remote')\n"
	"fclose(F_OUTPUT)\n"
	"writeln('End of model')\n"
	"end-model";

typedef struct
	{
	 char *buffer;
	 int mode;
	 int bufsize;
	 int curpos;
	} s_memfile;

static char bimfile[10*1024];	/* memory block to store the bimfile */
static int bimfilesize;

void* XPRD_CC my_open(void *ctx,char *filename,int mode,XPRDfct_data* fct_data,XPRDfct_close* fct_close,XPRDfct_skip* fct_skip,char *msg,int msglen);
int XPRD_CC my_close(void *data);
int XPRD_CC my_read(void *data,char *buf,int size);
int XPRD_CC my_skip(void *data,int size);
int XPRD_CC my_write(void *data,char *buf,int size);
int XPRD_CC outremote(void *data,char *buf,int size);

int main()
{
 XPRDcontext xprd;
 XPRDmosel mosel;
 XPRDmodel model,sender;
 XPRDfile f;
 char msg[256];
 int e;
 int cls;
 double val;

 /* Create context */
 xprd=XPRDinit();
 if(xprd==NULL)
 {
  printf("Could not create XPRD context.\n");
  return 1;
 }

 /* Create an instance with a user-defined file manager */
                                                         /* separate process */
 mosel=XPRDconnect(xprd,"",my_open,NULL,msg,sizeof(msg));
                                                         /* xprmsrv server */
 /* mosel=XPRDconnect(xprd,"localhost",my_open,NULL,msg,sizeof(msg)); */
 if(mosel==NULL)
 {
  printf("Connection failed: %s\n",msg);
  return 2;
 }

 printf("Connected to:\n%s\n",XPRDbanner(mosel));
 printf("Host: %s\n",XPRDsysinfo(mosel,XPRD_SYS_NODE,msg,sizeof(msg)));

 /* Set error stream to a local file */
 if(XPRDsetdefstream(mosel,NULL,XPRD_F_ERROR,"rmt:err.txt")!=0)
  printf("Failed to change error stream\n");

 /* Now compile the source from a local buffer to a local buffer(!) */
 if((e=XPRDcompmod(mosel,"","rmt:a.mos","rmt:bimfile",""))!=0)
 {
  printf("Compilation failed - error %d\n",e);
  return 3;
 }

 /* Save something in the temp directory of the remote instance */
 f=XPRDfopen(mosel,"tmp:file.txt",XPRD_F_WRITE,msg,sizeof(msg));
 if(f==NULL)
 {
  printf("Open file failed: %s\n",msg);
  return 4;
 }
 XPRDfwrite(f,"Hello_from_a_file\n",18);
 XPRDfclose(f);

 /* use the local buffer to load a model */
 model=XPRDloadmod(mosel,"rmt:bimfile");
 if(model==NULL)
 {
  printf("Failed to load model\n");
  return 5;
 }

 /* Redirect output stream of the model to the 'outremote' callback */
 if(XPRDsetdefstream(NULL,model,XPRD_F_OUTPUT|XPRD_F_LINBUF,"rmt:outremote")!=0)
  printf("Failed to change error stream\n");

 printf("run\n");
 XPRDrunmod(model,"");
 XPRDwaitevent(xprd,-1);
 XPRDgetevent(xprd,&sender,&cls,&val);
 printf("Received event: %d %g\n",cls,val);
 XPRDsendevent(model,5,3.33);
 do
 {
  XPRDwaitevent(xprd,-1);
  XPRDgetevent(xprd,&sender,&cls,&val);
  printf("Received event: %d %g\n",cls,val);
 }while(XPRDgetstatus(model)==XPRD_RT_RUNNING);
 
 /* Read something from the temp directory of the remote instance */
 f=XPRDfopen(mosel,"tmp:file.txt",XPRD_F_READ,msg,sizeof(msg));
 if(f==NULL)
 {
  printf("Open file failed: %s\n",msg);
  return 4;
 }
 e=XPRDfread(f,msg,sizeof(msg));
 printf("Read from remote file: %.*s\n",e,msg);
 XPRDfclose(f);

 /* Unload model */
 XPRDunloadmod(model);

 /* Disconnect instance */
 XPRDdisconnect(mosel);

 /* Release context */
 XPRDfinish(xprd);

 return 0;
}

/*************************************************************/
/* This file manager will direct accesses to files 'bimfile' */
/* and 'a.mos' to local memory blocks.                       */
/* Warning: concurrency not handled here!!!                  */
/*************************************************************/
void* XPRD_CC my_open(void *ctx,char *filename,int mode,XPRDfct_data* fct_data,XPRDfct_close* fct_close,XPRDfct_skip* fct_skip,char *msg,int msglen)
{
 s_memfile *memfile;

 if(strcmp(filename,"outremote")==0)
 {
  if((mode&(XPRD_F_READ|XPRD_F_WRITE))!=XPRD_F_WRITE)
  {
   strncpy(msg,"'outremote' is write only!",msglen);
   return XPRD_FMGR_ERR;
  }
  else
  {
   *fct_data=outremote;
   *fct_close=NULL;
   return (void*)1;
  }
 }
 else
 if(strcmp(filename,"bimfile")==0)
 {
  if((memfile=malloc(sizeof(s_memfile)))==NULL)
  {
   strncpy(msg,"memory error",msglen);
   return XPRD_FMGR_ERR;
  }
  else
  {
   memfile->buffer=bimfile;
   memfile->curpos=0;
   memfile->mode=mode&(XPRD_F_READ|XPRD_F_WRITE);
   memfile->bufsize=(memfile->mode==XPRD_F_READ)?bimfilesize:sizeof(bimfile);
   *fct_data=(memfile->mode==XPRD_F_READ)?my_read:my_write;
   *fct_close=my_close;
   *fct_skip=my_skip;
   return memfile;
  }
 }
 else
 if(strcmp(filename,"a.mos")==0)
 {
  if((mode&(XPRD_F_READ|XPRD_F_WRITE))!=XPRD_F_READ)
  {
   strncpy(msg,"'a.mos' is read only!",msglen);
   return XPRD_FMGR_ERR;
  }
  else
  if((memfile=malloc(sizeof(s_memfile)))==NULL)
  {
   strncpy(msg,"memory error",msglen);
   return XPRD_FMGR_ERR;
  }
  else
  {
   memfile->buffer=srcbuf;
   memfile->bufsize=strlen(srcbuf);
   memfile->curpos=0;
   memfile->mode=XPRD_F_READ;
   *fct_data=my_read;
   *fct_close=my_close;
   *fct_skip=my_skip;
   return memfile;
  }
 }
 else
  return NULL;                    /* for default processing */
}

/*******************************************/
/* Release resources used by a memory file */
/*******************************************/
int XPRD_CC my_close(void *data)
{
 s_memfile *memfile;

 memfile=data;
 /* save size of bimfile */
 if((memfile->mode==XPRD_F_WRITE)&&(memfile->buffer==bimfile))
  bimfilesize=memfile->curpos;
 free(data);
 return 0;
}

/************************************/
/* Send data from a block of memory */
/************************************/
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

/**************************/
/* Skip a block of memory */
/**************************/
int XPRD_CC my_skip(void *data,int size)
{
 s_memfile *memfile;

 memfile=data;
 if(memfile->curpos<memfile->bufsize)
 {
  if((memfile->bufsize-memfile->curpos)<=size)
   memfile->curpos=memfile->bufsize;
  else
   memfile->curpos+=size;
 }
 return 0;
}

/***********************************/
/* Store data in a block of memory */
/***********************************/
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

/******************************************/
/* Display received message with a prefix */
/******************************************/
int XPRD_CC outremote(void *data,char *buf,int size)
{
 printf("REMOTE: %.*s",size,buf);
 return size;
}


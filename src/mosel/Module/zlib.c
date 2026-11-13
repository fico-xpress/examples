/******************************************
  Mosel NI Examples
  =================
    
  File zlib.c
  ```````````
  Example module defining an IO driver
  for (un)compression using zlib library
  which can be found at http://www.zlib.org

  (c) 2008 Fair Isaac Corporation
      author: Y. Colombani, 2003
  zlib:
  (c) 1995-2002 Jean-Loup Gailly and Mark Adler
*******************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#define XPRM_NICOMPAT 3004002           /* Compatibility level: Mosel 3.4.2 */
#include "xprm_ni.h"

#ifdef _WIN32
#define snprintf _snprintf
#define _WINDOWS
#define ZLIB_DLL
   /* for precompiled zlib 1.2.1 */
#define ZLIB_WINAPI
#endif
#include "zlib.h"

#define BUFSIZE 1024                    /* buffer size for data transfer */

/**************************************************/
/* gzip driver: to handle files using GZIP format */
/* Example: "zlib.gzip:mydata.gz"                 */
/**************************************************/
                                        /* Functions of 'gzip' driver */
static void *gzip_open(XPRMcontext ctx,int *mode,const char *fname);
static int gzip_close(XPRMcontext ctx,gzFile gzf,int mode);
static long gzip_read(XPRMcontext ctx,gzFile gzf,void *buffer,
                                                        unsigned long size);
static long gzip_write(XPRMcontext ctx,gzFile gzf,void *buffer,
                                                        unsigned long size);
static void  setgzerror(XPRMcontext ctx,gzFile gzf);
static XPRMiofcttab iodrv_gzip[]=
        {
         {XPRM_IOCTRL_OPEN,gzip_open},
         {XPRM_IOCTRL_CLOSE,gzip_close},
         {XPRM_IOCTRL_READ,gzip_read},
         {XPRM_IOCTRL_WRITE,gzip_write},
         {XPRM_IOCTRL_INFO,"filename"},
         {0,NULL}
        };

/*************************************************/
/* compress driver: to handle compressed streams */
/* As opposed to 'gzip' driver this driver does  */
/* not require a physical file                   */
/* Note also that files produced by this driver  */
/* are not compatible with gzip                  */
/* Example: "zlib.compress:mem:0x1234/256"       */
/**************************************************/
                                        /* Functions of 'compress' driver */
static void *comp_open(XPRMcontext ctx,int *mode,const char *fname);
static int comp_close(XPRMcontext ctx,z_streamp zstr,int mode);
static long comp_read(XPRMcontext ctx,z_streamp zstr,void *buffer,
                                                        unsigned long size);
static long comp_write(XPRMcontext ctx,z_streamp zstr,void *buffer,
                                                        unsigned long size);
static XPRMiofcttab iodrv_compress[]=
        {
         {XPRM_IOCTRL_OPEN,comp_open},
         {XPRM_IOCTRL_CLOSE,comp_close},
         {XPRM_IOCTRL_READ,comp_read},
         {XPRM_IOCTRL_WRITE,comp_write},
         {XPRM_IOCTRL_INFO,"extended_filename"},
         {0,NULL}
        };

                                     /* Drivers of zlib module: gzip,compress */
static XPRMiodrvtab iodrv_zlib[]=
        {
         {"gzip",iodrv_gzip},
         {"compress",iodrv_compress},
         {NULL,NULL}
        };

static int chkres(int);
                                     /* Table of services: only IO drivers */
static XPRMdsoserv tabserv[]=
        {
         {XPRM_SRV_IODRVS,iodrv_zlib},
         {XPRM_SRV_CHKRES,(void*)chkres}
        };

                                     /* DSO interface: only services */
static XPRMdsointer dsointer= 
        { 
         0,NULL,
         0,NULL,
         0,NULL,
         sizeof(tabserv)/sizeof(mm_dsoserv),tabserv
        };

static XPRMnifct mm;             /* For storing Mosel NI function table */

/************************************************/
/* Initialize the library just after loading it */
/************************************************/
DSO_INIT zlib_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 const char *zlv;

 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */

                                /* We check ZLIB library version here */
 zlv=zlibVersion();
 if((zlv==NULL)||(zlv[0]!=ZLIB_VERSION[0]))
 {
  mm->dispmsg(NULL,"ZLIB: wrong version (expect %s got %s).\n",ZLIB_VERSION,
                                                          (zlv!=NULL)?zlv:"0");
  return 1;
 }
 else
 {
  *libver=XPRM_MKVER(0,0,2);     /* The version of the module: 0.0.2 */
  *interf=&dsointer;             /* Our interface */
  return 0;
 }
}

/****************************************************************/
/* Check restrictions (this module implements all restrictions) */
/****************************************************************/
static int chkres(int r)
{
 return 0;
}

/****************************************/
/* Open a gzip-file for (de)compression */
/****************************************/
static void *gzip_open(XPRMcontext ctx,int *mode,const char *fname)
{
 char cfname[MM_MAXPATHLEN];
 char cmode[16];
 int cml;
 void *gzf;

 if((fname==NULL)||
    (mm->pathcheck(ctx,fname,cfname,MM_MAXPATHLEN,
    ((*mode)&MM_F_WRITE)?MM_RCHK_WRITE:MM_RCHK_READ)!=0))
 {
  mm->setioerrmsg(ctx,strerror(EACCES),1);
  return NULL;
 }
 else
 {
  cml=2;
  cmode[0]=((*mode)&MM_F_WRITE)?'w':'r';
  cmode[1]=((*mode)&MM_F_BINARY)?'b':'t';
  if((*mode)&MM_F_APPEND) cmode[cml++]='a';
  cmode[cml]='\0';
  gzf=gzopen(cfname,cmode);
  if(gzf==NULL)
   mm->setioerrmsg(ctx,strerror(errno),2);
  return gzf;
 }
}

/***********************/
/* Close the gzip-file */
/***********************/
static int gzip_close(XPRMcontext ctx,gzFile gzf,int mode)
{ return gzclose(gzf); }

/******************************/
/* Uncompress a block of data */
/******************************/
static long gzip_read(XPRMcontext ctx,gzFile gzf,void *buffer,
                                                        unsigned long size)
{
 long l;

 l=gzread(gzf,buffer,size);
 if(l<0) setgzerror(ctx,gzf);
 return l;
}

/****************************/
/* Compress a block of data */
/****************************/
static long gzip_write(XPRMcontext ctx,gzFile gzf,void *buffer,
                                                        unsigned long size)
{
 long l;
 
 l=gzwrite(gzf,buffer,size);
 if(l<=0) setgzerror(ctx,gzf);
 return l;
}

/*********************************/
/* Set message of the last error */
/*********************************/
static void  setgzerror(XPRMcontext ctx,gzFile gzf)
{
 int errnum;
 const char *emsg;

 if((((emsg=gzerror(gzf,&errnum))!=NULL)&&(errnum==Z_ERRNO)))
 {
  errnum=errno;
  emsg=strerror(errno);
 }
 if(errnum!=0)
  mm->setioerrmsg(ctx,emsg,errnum);
}

/*************************************/
/* Open a stream for (de)compression */
/*************************************/
static void *comp_open(XPRMcontext ctx,int *mode,const char *fname)
{
 z_streamp zstr;
 int rts;

                                /* 1st: open actual file */
 if(mm->fopen(ctx,((*mode)&XPRM_F_WRITE)|XPRM_F_BINARY,fname)<0)
 {
  *mode|=XPRM_F_SILENT;                 /* Error message already displayed */
                                        /* switch so silent mode */
  return NULL;
 }
 else
 {                                 /* If OK, prepare data structures for zlib */
                /* Allocate memory for zstream and a buffer */
  if((zstr=(z_streamp)malloc(sizeof(z_stream)+BUFSIZE))==NULL)
  {
   mm->fclose(ctx,(*mode)&(XPRM_F_READ|XPRM_F_WRITE));
   mm->setioerrmsg(ctx,"Not enough memory",1);
   return NULL;
  }
  memset(zstr,0,sizeof(z_stream));
  zstr->zalloc=Z_NULL;
  zstr->zfree=Z_NULL;
  
  if(((*mode)&(XPRM_F_READ|XPRM_F_WRITE))==XPRM_F_READ)
  {                             /* For reading: 'inflate' algorithm */
   rts=inflateInit(zstr);
   zstr->next_in=(void *)(zstr+1);      /* Buffer is after zstream */
  }
  else
  {                             /* For writing: 'deflate' algorithm */
   rts=deflateInit(zstr,Z_DEFAULT_COMPRESSION);
   zstr->next_out=(void *)(zstr+1);     /* Buffer is after zstream */
   zstr->avail_out=BUFSIZE;
  }

  if(rts!=Z_OK)
  {
   mm->setioerrmsg(ctx,zstr->msg,rts);
   free(zstr);
   mm->fclose(ctx,(*mode)&(XPRM_F_READ|XPRM_F_WRITE));
   return NULL;
  }
  else
   return zstr;
 }
}

/********************/
/* Close the stream */
/********************/
static int comp_close(XPRMcontext ctx,z_stream *zstr,int mode)
{
 int rts1,rts2;
 Bytef dummy;

 if((mode&(XPRM_F_READ|XPRM_F_WRITE))==XPRM_F_READ)
 {                      /* If reading: just release zstream */
  rts1=inflateEnd(zstr);
 }
 else
 {                      /* If writing: buffers have to be flushed first */
  zstr->next_in=&dummy;
  zstr->avail_in=0;
  if(zstr->avail_out==0)
  {
   mm->fwrite(ctx,zstr+1,BUFSIZE);
   zstr->next_out=(void *)(zstr+1);
   zstr->avail_out=BUFSIZE;
  }
  do
  {
   rts1=deflate(zstr,Z_FINISH);
   if(BUFSIZE-zstr->avail_out>0)
    mm->fwrite(ctx,zstr+1,BUFSIZE-zstr->avail_out);
   zstr->next_out=(void *)(zstr+1);
   zstr->avail_out=BUFSIZE;
  } while(rts1==Z_OK);
  
  rts1=deflateEnd(zstr);
 }
                        /* Finally close actual file and release memory */
 rts2=mm->fclose(ctx,mode);
 if(rts1!=Z_OK)
  mm->setioerrmsg(ctx,zstr->msg,rts1);
 free(zstr);
 return (rts1!=Z_OK)||(rts2!=0);
}

/******************************/
/* Uncompress a block of data */
/******************************/
static long comp_read(XPRMcontext ctx,z_stream *zstr,void *buffer,
                                                        unsigned long size)
{
 int rts,finish;

 zstr->next_out=buffer;
 zstr->avail_out=size;
 finish=0;
 do
 {
  if(zstr->avail_in==0)
  {
   zstr->avail_in=mm->fread(ctx,zstr+1,BUFSIZE);
   zstr->next_in=(void *)(zstr+1);
   finish=(zstr->avail_in==0);
  }
  rts=inflate(zstr,Z_NO_FLUSH);
 } while((rts==Z_OK)&&(zstr->avail_out>0) && !finish);
 if(finish && (zstr->avail_out>0))
  rts=inflate(zstr,Z_SYNC_FLUSH);
 if((rts==Z_OK)||(rts==Z_STREAM_END))
  return (long)(size-zstr->avail_out);
 else
 {
  mm->setioerrmsg(ctx,zstr->msg,rts);
  return -1;
 }
}

/****************************/
/* Compress a block of data */
/****************************/
static long comp_write(XPRMcontext ctx,z_stream *zstr,void *buffer,
                                                        unsigned long size)
{
 int rts;

 zstr->next_in=buffer;
 zstr->avail_in=size;
 do
 {
  if(zstr->avail_out==0)
  {
   mm->fwrite(ctx,zstr+1,BUFSIZE);
   zstr->next_out=(void *)(zstr+1);
   zstr->avail_out=BUFSIZE;
  }
  rts=deflate(zstr,Z_NO_FLUSH);
 } while((rts==Z_OK)&&(zstr->avail_in>0));
 if(rts==Z_OK)
  return 1;
 else
 {
  mm->setioerrmsg(ctx,zstr->msg,rts);
  return 0;
 }
}


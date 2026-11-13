/**************************************************
  Mosel NI Examples
  =================
    
  File export.c
  `````````````
  Example module defining an IO driver
  that generates a C-file embedding a
  model

  (c) 2008 Fair Isaac Corporation
      author: Y. Colombani, 2003, rev. July 2026
**************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xprm_ni.h"

static char prg_part1[]=
"#include <stdio.h>\n"
"#include \"xprm.h\"\n\n"

"static unsigned int bimfile[]={";

static char prg_part2[]=
"0};\n\n"

"int main(int argc,char *argv[])\n"
"{\n"
" char modname[40];\n"
" XPRMmodel mod;\n"
" int rts;\n\n"

" rts=XPRMinit();\n"
" if((rts!=0)&&(rts!=32))\n"
" {\n"
"  char msg[512];\n\n"
  
"  XPRMgetlicerrmsg(msg,512);\n"
"  fprintf(stderr,\"%s\",msg);\n"
"  return 1;\n"
" }\n\n"

#ifdef _WIN32
" sprintf(modname,\"mem:%#Ix/%u\",\n"
"        (size_t)bimfile,";
#else
" sprintf(modname,\"mem:%#lx/%u\",\n"
"        (unsigned long)bimfile,";
#endif

static char prg_part3[]=
                                ");\n"
" if((mod=XPRMloadmod(modname,NULL))==NULL)\n"
"  return 2;\n"
" if(XPRMrunmod(mod,&rts,NULL))\n"
"  return 3;\n"
" else\n"
"  return rts;\n"
"}\n";

typedef struct
        {
         unsigned int total;      /* Total number of bytes written so far */
         union
         {
          char c[4];
          unsigned int i;
         } map;                   /* Mapping between 4 chars and an integer */
         int nb_remain;           /* Number of bytes not yet written */
         int nb_printed;          /* Number of numbers written on the line */
        } s_tocdata;

/*******************************************************/
/* toC driver: to generate a C program from a bim file */
/* Example: "export.toC:mymodel.c"                     */
/* Using it from Mosel console:                        */
/* compile mymodel '' export.toC:mymodel.c             */
/*******************************************************/
                                        /* Functions of 'toC' driver */
static void *toc_open(XPRMcontext ctx,int *mode,const char *fname);
static int toc_close(XPRMcontext ctx,s_tocdata *td,int mode);
static long toc_write(XPRMcontext ctx,s_tocdata *td,char *buffer,
                                                        unsigned long size);
static XPRMiofcttab iodrv_toc[]=
        {
         {XPRM_IOCTRL_OPEN,(void *)toc_open},
         {XPRM_IOCTRL_CLOSE,(void *)toc_close},
         {XPRM_IOCTRL_WRITE,(void *)toc_write},
         {XPRM_IOCTRL_INFO,"extended_filename"},
         {0,NULL}
        };

                                        /* Drivers of export module: toC */
static XPRMiodrvtab iodrv_export[]=
        {
         {"toC",iodrv_toc},
         {NULL,NULL}
        };

                                        /* Table of services: only IO drivers */
static XPRMdsoserv tabserv[]=
        {
         {XPRM_SRV_IODRVS,iodrv_export}
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
DSO_INIT export_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */
 return 0;
}

/*****************************/
/* Open the C-file to create */
/*****************************/
static void *toc_open(XPRMcontext ctx,int *mode,const char *fname)
{
 s_tocdata *td;

                                /* 1st: open actual file */
 if(mm->fopen(ctx,*mode,fname)<0)
 {
  *mode|=XPRM_F_SILENT;                /* Error message already displayed */
                                       /* switch so silent mode */
  return NULL;
 }
 else
 {
  td=(s_tocdata *)malloc(sizeof(s_tocdata));
  td->total=0;
  td->nb_remain=0;
  td->nb_printed=0;
  mm->printf(ctx,"%s\n",prg_part1);    /* Display the beginning of program */
  return td;
 }
}

/********************/
/* Close the C-file */
/********************/
static int toc_close(XPRMcontext ctx,s_tocdata *td,int mode)
{
 if(td->nb_remain>0)                    /* Send bytes to be written */
 {
  td->total+=td->nb_remain;
  for(;td->nb_remain<4;td->nb_remain++)
   td->map.c[td->nb_remain]=0;
  mm->printf(ctx,"%#x,",td->map.i);
 }                                      /* and complete program */
 mm->printf(ctx,"%s%u%s",prg_part2,td->total,prg_part3);
 return mm->fclose(ctx,mode);
}

/**************************/
/* Write a list of values */
/**************************/
static long toc_write(XPRMcontext ctx,s_tocdata *td,char *buffer,
                                                        unsigned long size)
{
 int i;

 if(size+td->nb_remain<4)              /* Not enough for an integer... */
 {
  for(i=0;i<size;i++)
    td->map.c[td->nb_remain+i]=buffer[i];
  td->nb_remain+=size;                 /* store them for next call */
 }
 else
 {
  if(td->nb_remain>0)                  /* Bytes from previous call first... */
  {
   for(i=0;i<4-td->nb_remain;i++)
    td->map.c[td->nb_remain+i]=buffer[i];
   mm->printf(ctx,"%#x,",td->map.i);
   td->total+=4;
   td->nb_printed++;
   size-=4-td->nb_remain;
   buffer-=4-td->nb_remain;
  }
  while(size>=4)                       /* then the buffer we receive */
  {
   td->map.c[0]=buffer[0];
   td->map.c[1]=buffer[1];
   td->map.c[2]=buffer[2];
   td->map.c[3]=buffer[3];
   mm->printf(ctx,"%#x,",td->map.i);
   size-=4;
   buffer+=4;
   td->total+=4;
   if(++td->nb_printed>=6)
   {
    td->nb_printed=0;
    mm->printf(ctx,"\n");
   }
  }
  td->nb_remain=size;                  /* Keep track of remaining bytes */
  memcpy(td->map.c,buffer,size);
 }
 return 1;
}


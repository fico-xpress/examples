/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexdrvs.c                                     */
/*  ```````````````                                     */
/*  Example for the use of the Mosel libraries          */
/*  (using IOdrivers for data exchange)                 */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2003, rev. July 2026      */
/********************************************************/

#include <stdio.h>
#include "xprm.h"

                /*****************************************************/
                /* The source of the model as an array of characters */
                /*****************************************************/
const char source_of_model[]=
"model drivers\n"

"parameters\n"
" DATA=''\n"
" SOL=''\n"
"end-parameters\n"

"declarations\n"
" S:set of string\n"
" R:range\n"
" data:array(S,R) of real\n"
" sol:array(1..10) of real\n"
"end-declarations\n"

"initialisations from 'raw:'\n"
" data as DATA\n"
"end-initialisations\n"

"writeln('set   S=',S)\n"
"writeln('range R=',R)\n"
"writeln('array data=',data)\n"

"forall(i in 1..10) sol(i):=i^2\n"

"initialisations to 'raw:'\n"
" sol as SOL\n"
"end-initialisations\n"

"end-model";
                /*******************************************************/
                /* Structure to store initial values for array 'data': */
                /*******************************************************/
const struct
{
 const char *s;      /* text for the first index */
 int r;              /* integer value for the second index */
 double v;           /* corresponding value: data(s,r) */
} data[]={{"one",2,12.5},{"two",1,15},{"three",16,9},{"hundred",2,17}};

                /************************************/
                /* Array to receive solution values */
                /************************************/
double solution[10];

                /**********************************************/
                /* Callback function to handle default output */
                /**********************************************/
long XPRM_RTC cbmsg(XPRMmodel model,void *info,char *buf,unsigned long size)
{
/* Note: 'model' is NULL if the stream is used outside of an execution */
 printf("Mosel: %.*s",(int)size,buf);
 return 0;
}

                /*****************/
                /* Main function */
                /*****************/
int main()
{
 XPRMmodel mod;
 int result,i;
 char bimfile[1024];           /* Buffer to store BIM file */
 unsigned long bimfile_size;   /* Buffer to store actual size of BIM file */
 char outfile_name[40];        /* File name of output stream */
 char mosfile_name[40];        /* File name of MOS file */
 char bimfile_name[64];        /* File name of BIM file */
 char data_name[40];           /* File name of initial values for 'data' */
 char solution_name[40];       /* File name of solution values */
 char params[96];              /* Parameter string for model execution */

/*                                                        Initialize Mosel */
 i=XPRMinit();
 if((i!=0)&&(i!=32))
  return 1;
                        /****************************************/
                        /* Prepare file name for output stream  */
                        /* using 'cb' driver:                   */
                        /* "cb:function pointer[/callback data]"*/
                        /****************************************/
#ifdef _WIN32
 sprintf(outfile_name,"cb:%#Ix",(size_t)cbmsg);
#else
 sprintf(outfile_name,"cb:%#lx",(unsigned long)cbmsg);
#endif

/*                                   Set default output stream to callback */
 XPRMsetdefstream(NULL,XPRM_F_WRITE,outfile_name);

                        /*************************************************/
                        /* Prepare file names for compilation            */
                        /* using 'mem' driver:                           */
                        /* "mem:base address/size[/actual size pointer]" */
                        /*************************************************/
#ifdef _WIN32
 sprintf(mosfile_name,"mem:%#Ix/%u",
        (size_t)source_of_model,(unsigned int)sizeof(source_of_model));
#else
 sprintf(mosfile_name,"mem:%#lx/%u",
        (unsigned long)source_of_model,(unsigned int)sizeof(source_of_model));
#endif
 bimfile_size=0;
#ifdef _WIN32
 sprintf(bimfile_name,"mem:%#Ix/%u/%#Ix",
         (size_t)bimfile,(unsigned int)sizeof(bimfile),(size_t)&bimfile_size);
#else
 sprintf(bimfile_name,"mem:%#lx/%u/%#lx",
         (unsigned long)bimfile,(unsigned int)sizeof(bimfile),(unsigned long)&bimfile_size);
#endif

/*                                     Compile model from memory to memory */
 if(XPRMcompmod(NULL,mosfile_name,bimfile_name,NULL))
  return 2;
 printf("BIM file uses %lu bytes of memory.\n",bimfile_size);

/*                                             Load a BIM file from memory */
 if((mod=XPRMloadmod(bimfile_name,NULL))==NULL)
  return 3;

                        /*******************************************/
                        /* Prepare file names for 'initialisations'*/
                        /* using 'raw' driver:                     */
                        /* "rawoption[,...],filename"              */
                        /* Here, 'filename' uses the 'mem' driver, */
                        /* data is stored in memory                */
                        /*******************************************/

        /* Option for 'raw': 'slength=0' means that strings are represented */
        /*               by pointers to null terminated arrays of           */
        /*               characters (C-string) instead of fixed size arrays */
#ifdef _WIN32
 sprintf(data_name,"slength=0,mem:%#Ix/%u",
                                (size_t)data,(unsigned int)sizeof(data));
#else
 sprintf(data_name,"slength=0,mem:%#lx/%u",
                                (unsigned long)data,(unsigned int)sizeof(data));
#endif

        /* Option for 'raw': 'noindex' means that only array values are */
        /*                   expected - no indices requested            */
#ifdef _WIN32
 sprintf(solution_name,"noindex,mem:%#Ix/%u",
                                 (size_t)solution,(unsigned int)sizeof(solution));
#else
 sprintf(solution_name,"noindex,mem:%#lx/%u",
                                 (unsigned long)solution,(unsigned int)sizeof(solution));
#endif

                        /* file names are passed through execution parameters */
 sprintf(params,"DATA='%s',SOL='%s'",data_name,solution_name);

/*                                                           Run the model */
 if(XPRMrunmod(mod,&result,params))
  return 4;

/*                        Display solutions values obtained from the model */
 printf("Solution values:");
 for(i=0;i<10;i++)
  printf(" %g",solution[i]);
 printf("\n");

 return 0;
}


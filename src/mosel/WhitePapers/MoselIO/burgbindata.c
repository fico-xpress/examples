/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file burgbindata.c
   ``````````````````
   Example of use of BinDrv for reading and writing files.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013, rev. July 2026
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "bindrv.h"
#include "xprm.h"

static void writeburgbin(void);
static void readburgbin(void);

   /* Input data */ 
static double vdata[]={15,100,90,60,40,15,10, 1};   /* VALUE */
static double wdata[]={ 2, 20,20,30,40,30,60,10};   /* WEIGHT */
static char *ind[]={"camera", "necklace", "vase", "picture", "tv", 
                   "video", "chest", "brick" };     /* Index names */
int datasize=8;
 
int main()
{
 int result;

 writeburgbin();

 if(XPRMinit())                       /* Initialize Mosel */
  return 1;
                                      /* Execute = compile/load/run a model */
 if(XPRMexecmod(NULL,"burglar2m.mos",NULL,&result,NULL))
  return 2;

 readburgbin();
 return 0;
}

/*****************************/
/* Create a BinDrv data file */
/*****************************/
static void writeburgbin(void)
{
 FILE *f;
 s_bindrvctx bdrv;
 int i;

 f=fopen("burgdatabin","w");
 bdrv=bindrv_newwriter((size_t (*)(const void *,size_t,size_t,void*))fwrite,f);

 bindrv_putctrl(bdrv,BINDRV_CTRL_LABEL); bindrv_putstring(bdrv,"BurgData");
 bindrv_putctrl(bdrv,BINDRV_CTRL_OPENLST);         /* [       */
 for(i=0;i<datasize;i++)
 {
  bindrv_putctrl(bdrv,BINDRV_CTRL_OPENNDX);        /*  (      */
   bindrv_putstring(bdrv,ind[i]);                  /*   index */
  bindrv_putctrl(bdrv,BINDRV_CTRL_CLOSENDX);       /*  )      */
  bindrv_putctrl(bdrv,BINDRV_CTRL_OPENLST);        /*   [     */
   bindrv_putreal(bdrv,vdata[i]);                  /*    val1 */
   bindrv_putreal(bdrv,wdata[i]);                  /*    val2 */
  bindrv_putctrl(bdrv,BINDRV_CTRL_CLOSELST);       /*   ]     */
 }
 bindrv_putctrl(bdrv,BINDRV_CTRL_CLOSELST);        /* ]       */

 bindrv_delete(bdrv);
 fclose(f);
}

/***************************************/
/* Read and display a BinDrv data file */
/***************************************/
static void readburgbin(void)
{
 FILE *f;
 s_bindrvctx bdrv;
 union { int i; double r; char b; char *str; BINDRV_LONG l;} val;

 f=fopen("resdatabin","r");
 bdrv=bindrv_newreader((size_t (*)(void *,size_t,size_t,void*))fread,f);

 bindrv_getctrl(bdrv,&(val.i));
 if(val.i==BINDRV_CTRL_LABEL)
 {
  while(bindrv_nexttoken(bdrv)>=0)
  {
   bindrv_getstring(bdrv,&(val.str));                      /* label:  */
   if(strcmp(val.str,"SolTake")==0)
   {
    free(val.str);
    printf("Solution values:\n");
    bindrv_getctrl(bdrv,&(val.i));                         /* [       */
    while(bindrv_nexttoken(bdrv)>=0)
    {
     bindrv_getctrl(bdrv,&(val.i));
     if(val.i== BINDRV_CTRL_LABEL)  break;
     switch(val.i)
     {
      case BINDRV_CTRL_OPENNDX:                             /*  (      */
       printf(" take(");
       bindrv_getstring(bdrv,&(val.str));                   /*   index */
       printf("%s",val.str);
       bindrv_getctrl(bdrv,&(val.i));                       /*  )      */
       printf(")=");
       bindrv_getreal(bdrv,&(val.r));                       /*  value  */
       printf(" %g\n",val.r);
       break;
      case BINDRV_CTRL_CLOSELST:                            /* ]       */
       printf("\n");  
       break;
      default:
       printf("Unexpected token %d\n", val.i); exit(1); 
     }
    }
   }
   else if(strcmp(val.str,"Objective")==0)
   {
    free(val.str);
    bindrv_getreal(bdrv,&(val.r));
    printf("Objective value = %g\n", val.r);
   }
   else
   {
    printf("Unexpected label '%s'\n", val.str);
    free(val.str);
    exit(1);
   }
  }
 }
 else
 {
  printf("Unexpected token '%d'\n", val.i);
  exit(1);
 } 

 bindrv_delete(bdrv);
 fclose(f);
}


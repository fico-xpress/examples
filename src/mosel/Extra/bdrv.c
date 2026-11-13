/********************************************************/
/*  BinDrv library example                              */
/*  ======================                              */
/*                                                      */
/*  file bdrv.c                                         */
/*  ```````````                                         */
/* Example of use of BinDrv                             */
/*                                                      */
/*  (c) 2011 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2011                      */
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "bindrv.h"

static void writeit(void);
static void readit(void);
static void *dummy_malloc(size_t s,void *ctx);

int main()
{
 writeit();
 readit();
 return 0;
}

/*****************************/
/* Create a BinDrv data file */
/*****************************/
static void writeit(void)
{
 FILE *f;
 s_bindrvctx bdrv;
 unsigned char data[]={1,2,3,4,5,6,7,8};

 f=fopen("bindata","w");
 bdrv=bindrv_newwriter((size_t (*)(const void *,size_t,size_t,void*))fwrite,f);
 bindrv_putctrl(bdrv,BINDRV_CTRL_LABEL); bindrv_putstring(bdrv,"A");
 bindrv_putctrl(bdrv,BINDRV_CTRL_OPENLST);
 bindrv_putctrl(bdrv,BINDRV_CTRL_OPENNDX); bindrv_putint(bdrv,1);
 bindrv_putctrl(bdrv,BINDRV_CTRL_CLOSENDX);
 bindrv_putreal(bdrv,0.45);bindrv_putreal(bdrv,-31.02); bindrv_putint(bdrv,-28);
 bindrv_putctrl(bdrv,BINDRV_CTRL_CLOSELST);
 bindrv_putctrl(bdrv,BINDRV_CTRL_LABEL); bindrv_putstring(bdrv,"B");
 bindrv_putbool(bdrv,1);
 bindrv_putctrl(bdrv,BINDRV_CTRL_LABEL); bindrv_putstring(bdrv,"C");
 bindrv_putstring(bdrv,"sometext");
 bindrv_putctrl(bdrv,BINDRV_CTRL_LABEL); bindrv_putstring(bdrv,"D");
 bindrv_putlong(bdrv,(BINDRV_LONG)INT_MAX*11);
 bindrv_putctrl(bdrv,BINDRV_CTRL_LABEL); bindrv_putstring(bdrv,"E");
 bindrv_putdata(bdrv,data,sizeof(data));
 bindrv_delete(bdrv);
 fclose(f);
}

/***************************************/
/* Read and display a BinDrv data file */
/***************************************/
static void readit(void)
{
 FILE *f;
 s_bindrvctx bdrv;
 union { int i; double r; char b; char *str; void *data; BINDRV_LONG l;} val;
 size_t size,i;

 f=fopen("bindata","r");
 bdrv=bindrv_newreader((size_t (*)(void *,size_t,size_t,void*))fread,f);
 /* By default the library will allocate strings (returned by 'getstring') */
 /* using 'malloc'. The following routine allows to replace 'malloc' by some */
 /* user-defined memory allocation routine (that gets a context as an */
 /* extra parameter) */
 bindrv_setalloc(bdrv,dummy_malloc,NULL);

 while(bindrv_nexttoken(bdrv)>=0)
 {
  switch(bindrv_nexttoken(bdrv))
  {
   case BINDRV_TYP_INT:
	bindrv_getint(bdrv,&(val.i));
	printf(" %d",val.i);
	break;
   case BINDRV_TYP_DATA:
	bindrv_getdata(bdrv,&(val.data),&size);
	printf(" ");
	for(i=0;i<size;i++)
	 printf("%02x",val.str[i]);
	break;
   case BINDRV_TYP_REAL:
	bindrv_getreal(bdrv,&(val.r));
	printf(" %g",val.r);
	break;
   case BINDRV_TYP_STR:
	bindrv_getstring(bdrv,&(val.str));
	printf(" %s",val.str);
	break;
   case BINDRV_TYP_BOOL:
	bindrv_getbool(bdrv,&(val.b));
	printf(" %s",val.b?"true":"false");
	break;
   case BINDRV_TYP_CTRL:
	bindrv_getctrl(bdrv,&(val.i));
	switch(val.i)
	{
	 case BINDRV_CTRL_SKIP: printf(" *"); break;
	 case BINDRV_CTRL_LABEL:
		bindrv_getstring(bdrv,&(val.str));
		printf("\n%s:",val.str);
		break;
	 case BINDRV_CTRL_OPENLST: printf("["); break;
	 case BINDRV_CTRL_CLOSELST: printf("]"); break;
	 case BINDRV_CTRL_OPENNDX: printf("("); break;
	 case BINDRV_CTRL_CLOSENDX: printf(")"); break;
	 case BINDRV_CTRL_NIL: printf(" ?"); break;
	 default:
	 	fprintf(stderr,"Unexpected control\n");
		exit(1);
	}
	break;
   case BINDRV_TYP_LONG:
	bindrv_getlong(bdrv,&(val.l));
	printf(" %lld",val.l);
	break;
   default:
	fprintf(stderr,"Unexpected token\n");
	exit(1);
  }
 }
 bindrv_delete(bdrv);
 fclose(f);
 printf("\n");
}

/* A dummy malloc function using a static region */
static void *dummy_malloc(size_t s,void *ctx)
{
 static char buf[1024];

 return (s>1024)?NULL:buf;
}

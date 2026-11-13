/******************************************
  Mosel NI Examples
  =================
    
  File complex.c
  ``````````````
  Example module defining a new type
    complex
  with the corresponding arithmetic operators.

  (c) 2022 Fair Isaac Corporation
      author: Y. Colombani, 2002
*******************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define XPRM_NICOMPAT 6000000   /* Compatibility level: Mosel 6.0.0 */
#include "xprm_ni.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

/**** Function prototypes ****/
static int xritrinit(XPRMcontext ctx,void *libctx);
static int xritrnext(XPRMcontext ctx,void *libctx);
static int cx_new0(XPRMcontext ctx,void *libctx);
static int cx_new1r(XPRMcontext ctx,void *libctx);
static int cx_new1i(XPRMcontext ctx,void *libctx);
static int cx_new2(XPRMcontext ctx,void *libctx);
static int cx_zero(XPRMcontext ctx,void *libctx);
static int cx_one(XPRMcontext ctx,void *libctx);
static int cx_imin(XPRMcontext ctx,void *libctx);
static int cx_imax(XPRMcontext ctx,void *libctx);
static int cx_asgn(XPRMcontext ctx,void *libctx);
static int cx_asgn_r(XPRMcontext ctx,void *libctx);
static int cx_pls(XPRMcontext ctx,void *libctx);
static int cx_pls_r(XPRMcontext ctx,void *libctx);
static int cx_neg(XPRMcontext ctx,void *libctx);
static int cx_mul(XPRMcontext ctx,void *libctx);
static int cx_mul_r(XPRMcontext ctx,void *libctx);
static int cx_div(XPRMcontext ctx,void *libctx);
static int cx_div_r1(XPRMcontext ctx,void *libctx);
static int cx_div_r2(XPRMcontext ctx,void *libctx);
static int cx_eql_r(XPRMcontext ctx,void *libctx);
static int cx_getim(XPRMcontext ctx,void *libctx);
static int cx_getre(XPRMcontext ctx,void *libctx);
static int cx_setim(XPRMcontext ctx,void *libctx);
static int cx_setre(XPRMcontext ctx,void *libctx);
static int cx_tostr(XPRMcontext ctx,void *,void *,char *,int,int);
static int cx_fromstr(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int,const char **endptr);
static int cx_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum);
static int cx_compare(XPRMcontext ctx,void *libctx,void *c1,void *c2,int typnum);
static void *cx_create(XPRMcontext ctx,void *,void *,int);
static void cx_delete(XPRMcontext ctx,void *,void *,int);
static void *cx_reset(XPRMcontext ctx,void *libctx,int version);
static size_t cx_memuse(XPRMcontext ctx,void *libctx,void *ref,int code);
static int cx_getarrind(XPRMcontext ctx,void*libctx,void *itr,int cde,XPRMarray arr,int *indices,int op);

static void *reo_create(XPRMcontext ctx,void *,void *,int);
static void reo_delete(XPRMcontext ctx,void *,void *,int);
static int reo_reset(XPRMcontext ctx,void *libctx,void *toreset,void *src,int ust);

/**** Structures for passing info to Mosel ****/
/* Subroutines */
static XPRMdsofct tabfct[]=
        {
         {"@&",1000,XPRM_TYP_EXTN,1,"complex:|complex|",cx_new0},
         {"@&I",1001,XPRM_TYP_EXTN,1,"complex:r",cx_new1r},
         {"@&I",1002,XPRM_TYP_EXTN,1,"complex:i",cx_new1i},
         {"@&",1003,XPRM_TYP_EXTN,2,"complex:rr",cx_new2},
         {"@0",1004,XPRM_TYP_EXTN,0,"complex:",cx_zero},
         {"@1",1005,XPRM_TYP_EXTN,0,"complex:",cx_one},
         {"@2",1006,XPRM_TYP_EXTN,0,"complex:",cx_imin},
         {"@3",1007,XPRM_TYP_EXTN,0,"complex:",cx_imax},
         {"@:",1008,XPRM_TYP_NOT,2,"|complex||complex|",cx_asgn},
         {"@:",1009,XPRM_TYP_NOT,2,"|complex|r",cx_asgn_r},
         {"@+",1010,XPRM_TYP_EXTN,2,"complex:|complex||complex|",cx_pls},
         {"@+",1011,XPRM_TYP_EXTN,2,"complex:|complex|r",cx_pls_r},
         {"@*",1012,XPRM_TYP_EXTN,2,"complex:|complex||complex|",cx_mul},
         {"@*",1013,XPRM_TYP_EXTN,2,"complex:|complex|r",cx_mul_r},
         {"@-",1014,XPRM_TYP_EXTN,1,"complex:|complex|",cx_neg},
         {"@/",1015,XPRM_TYP_EXTN,2,"complex:|complex||complex|",cx_div},
         {"@/",1016,XPRM_TYP_EXTN,2,"complex:|complex|r",cx_div_r1},
         {"@/",1017,XPRM_TYP_EXTN,2,"complex:r|complex|",cx_div_r2},
         {"@=",1018,XPRM_TYP_BOOL,2,"|complex|r",cx_eql_r},

         {"getim",1050,XPRM_TYP_REAL,1,"|complex|",cx_getim},
         {"getre",1051,XPRM_TYP_REAL,1,"|complex|",cx_getre},
         {"setim",1052,XPRM_TYP_NOT,2,"|complex|r",cx_setim},
         {"setre",1053,XPRM_TYP_NOT,2,"|complex|r",cx_setre},

         {"initenum",1100,XPRM_TYP_NOT,2,"|realonly|a",xritrinit},
         {"nextreal",1103,XPRM_TYP_BOOL,1,"|realonly|",xritrnext}
        };

/* Types */
static XPRMdsotyp tabtyp[]=
        {
         {"complex",1,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT|XPRM_DTYP_APPND|XPRM_DTYP_SHARE|XPRM_DTYP_TFBIN|XPRM_DTYP_ORD|XPRM_DTYP_CONST,cx_create,cx_delete,cx_tostr,cx_fromstr,cx_copy,cx_compare},
         {"realonly",2,XPRM_DTYP_RFCNT|XPRM_DTYP_ANDX|XPRM_DTYP_ORSET,reo_create,reo_delete,NULL,NULL,reo_reset}
        };

/* Services */
static XPRMdsoserv tabserv[]=
        {
         {XPRM_SRV_RESET,(void *)cx_reset},
         {XPRM_SRV_MEMUSE,(void *)cx_memuse},
         {XPRM_SRV_ARRIND,(void*)cx_getarrind}
        };

/* Interface structure */
static XPRMdsointer dsointer= 
        { 
         0,NULL,
         sizeof(tabfct)/sizeof(XPRMdsofct),tabfct,
         sizeof(tabtyp)/sizeof(XPRMdsotyp),tabtyp,
         sizeof(tabserv)/sizeof(XPRMdsoserv),tabserv
        };

/**** Structures used by this module ****/
static XPRMnifct mm;             /* For storing Mosel NI function table */

#define CX_SHARED (1<<29)       /* Marker for a shared complex number */
#define CX_CONST  (1<<30)       /* Marker for a constant complex number */
typedef struct                  /* Where we store a complex number */
        {
         unsigned int refcnt;   /* For reference count and shared flag */
         double re,im;
        } s_complex;

#define MAXITNDX 5              /* Maximum number of indices for an iterator */

#define ITR_READY  1            /* Stopped on a valid cell */
#define ITR_END    2            /* Enumeration finished */
typedef struct
        {
         unsigned int refcnt;
         unsigned short nbdim;
         unsigned short status;
         XPRMarray reftab;
         int indices[MAXITNDX];
        } s_realonly;


/************************************************/
/* Initialize the library just after loading it */
/************************************************/
DSO_INIT complex_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/******** Functions implementing the operators ********/

/*****************************/
/* Initialise an enumeration */
/*****************************/
static int xritrinit(XPRMcontext ctx,void *libctx)
{
 XPRMarray tab;
 s_realonly *itr;
 int nbdim;

 itr=XPRM_POP_REF(ctx);
 tab=XPRM_POP_REF(ctx);
 if(itr==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized realonly.\n");
  return XPRM_RT_ERROR;
 }
 else
 if(tab==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized array.\n");
  return XPRM_RT_ERROR;
 }
 else
 if((nbdim=mm->getarrdim(tab))>MAXITNDX)
 {
  mm->dispmsg(ctx,"Complex: Too many array dimensions for a realonly.\n");
  return XPRM_RT_ERROR;
 }
 else
 {
  if(itr->reftab!=tab)
  {
   reo_reset(ctx,libctx,itr,NULL,0);
   itr->reftab=mm->newref(ctx,XPRM_STR_ARR,tab);
   itr->nbdim=nbdim;
  }
  itr->status=0;
  return RT_OK;
 }
}

/****************************************************/
/* Advance to the next cell containing a real value */
/****************************************************/
static int xritrnext(XPRMcontext ctx,void *libctx)
{
 s_realonly *itr;
 s_complex *cx;
 int cont;

 itr=XPRM_TOP_ST(ctx)->ref;
 if(itr==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized realonly.\n");
  return XPRM_RT_ERROR;
 }
 else
 if(itr->reftab==NULL)
 {
  mm->dispmsg(ctx,"Complex: Realonly not associated to any array.\n");
  return XPRM_RT_ERROR;
 }
 else
 {
  if(itr->status==ITR_END)
   XPRM_TOP_ST(ctx)->integer=0;
  else
  {
   if(itr->status==0)
   {
    itr->status=ITR_READY;
    cont=!mm->getfirstarrtruentry(itr->reftab,itr->indices);
   }
   else
    cont=!mm->getnextarrtruentry(itr->reftab,itr->indices);

   while(cont)
   {
    mm->getarrval(itr->reftab,itr->indices,&cx);
    if(cx->im==0)
     break;
    else
     cont=!mm->getnextarrtruentry(itr->reftab,itr->indices);
   }
   if(!cont)
   {
    itr->status=ITR_END;
    XPRM_TOP_ST(ctx)->integer=0;
   }
   else
    XPRM_TOP_ST(ctx)->integer=1;
  }
  return XPRM_RT_OK;
 }
}

/*******************/
/* Clone a complex */
/*******************/
static int cx_new0(XPRMcontext ctx,void *libctx)
{
 s_complex *complex,*new_complex;

 complex=XPRM_POP_REF(ctx);
 if(complex!=NULL)
 {
  new_complex=cx_create(ctx,libctx,NULL,0);
  new_complex->im=complex->im;
  new_complex->re=complex->re;
  XPRM_PUSH_REF(ctx,new_complex);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

/********************************/
/* Create a complex from a real */
/********************************/
static int cx_new1r(XPRMcontext ctx,void *libctx)
{
 s_complex *complex;

 complex=cx_create(ctx,libctx,NULL,0);
 complex->re=XPRM_POP_REAL(ctx);
 XPRM_PUSH_REF(ctx,complex);
 return XPRM_RT_OK;
}

/************************************/
/* Create a complex from an integer */
/************************************/
static int cx_new1i(XPRMcontext ctx,void *libctx)
{
 s_complex *complex;

 complex=cx_create(ctx,libctx,NULL,0);
 complex->re=XPRM_POP_INT(ctx);
 XPRM_PUSH_REF(ctx,complex);
 return XPRM_RT_OK;
}

/***********************************/
/* Create a complex from two reals */
/***********************************/
static int cx_new2(XPRMcontext ctx,void *libctx)
{
 s_complex *complex;

 complex=cx_create(ctx,libctx,NULL,0);
 complex->re=XPRM_POP_REAL(ctx);
 complex->im=XPRM_POP_REAL(ctx);
 XPRM_PUSH_REF(ctx,complex);
 return XPRM_RT_OK;
}

/**********************************************/
/* Zero in complex (used to initialise `sum') */
/**********************************************/
static int cx_zero(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_REF(ctx,cx_create(ctx,libctx,NULL,0));
 return XPRM_RT_OK;
}

/**********************************************/
/* One in complex (used to initialise `prod') */
/**********************************************/
static int cx_one(XPRMcontext ctx,void *libctx)
{
 s_complex *complex;

 complex=cx_create(ctx,libctx,NULL,0);
 complex->re=1;
 XPRM_PUSH_REF(ctx,complex);
 return XPRM_RT_OK;
}

/******************************************/
/* Initial value for the 'min' aggregator */
/******************************************/
static int cx_imin(XPRMcontext ctx,void *libctx)
{
 s_complex *complex;

 complex=cx_create(ctx,libctx,NULL,0);
 complex->re=complex->im=INFINITY;
 XPRM_PUSH_REF(ctx,complex);
 return XPRM_RT_OK;
}

/******************************************/
/* Initial value for the 'max' aggregator */
/******************************************/
static int cx_imax(XPRMcontext ctx,void *libctx)
{
 s_complex *complex;

 complex=cx_create(ctx,libctx,NULL,0);
 complex->re=complex->im=-INFINITY;
 XPRM_PUSH_REF(ctx,complex);
 return XPRM_RT_OK;
}

/*******************************/
/* Assignment complex:=complex */
/*******************************/
static int cx_asgn(XPRMcontext ctx,void *libctx)
{
 s_complex *c1,*c2;

 c1=XPRM_POP_REF(ctx);
 c2=XPRM_POP_REF(ctx);
 if(c1==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized complex.\n");
  return RT_ERROR;
 }
 else
 if(c1->refcnt&CX_CONST)
 {
  mm->dispmsg(ctx,"Complex: Trying to modify a constant.\n");
  return RT_ERROR;
 }
 else
 if(c2==NULL)
 {
  c1->im=c1->re=0;
  return RT_OK;
 }
 else
 {
  c1->im=c2->im;
  c1->re=c2->re;
  cx_delete(ctx,libctx,c2,0);
  return XPRM_RT_OK;
 }
}

/****************************/
/* Assignment complex:=real */
/****************************/
static int cx_asgn_r(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;

 c1=XPRM_POP_REF(ctx);
 if(c1==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized complex.\n");
  return RT_ERROR;
 }
 else
 if(c1->refcnt&CX_CONST)
 {
  mm->dispmsg(ctx,"Complex: Trying to modify a constant.\n");
  return RT_ERROR;
 }
 else
 {
  c1->re=XPRM_POP_REAL(ctx);
  c1->im=0;
  return XPRM_RT_OK;
 }
}

/***************************************/
/* Addition complex+complex -> complex */
/***************************************/
static int cx_pls(XPRMcontext ctx,void *libctx)
{
 s_complex *c1,*c2;

 c1=XPRM_POP_REF(ctx);
 c2=XPRM_POP_REF(ctx);
 if(c1!=NULL)
 {
  if(c2!=NULL)
  {
   c1->re+=c2->re;
   c1->im+=c2->im;
   cx_delete(ctx,libctx,c2,0);
  }
  XPRM_PUSH_REF(ctx,c1);
 }
 else
  XPRM_PUSH_REF(ctx,c2);
 return XPRM_RT_OK;
}

/************************************/
/* Addition complex+real -> complex */
/************************************/
static int cx_pls_r(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;

 c1=XPRM_POP_REF(ctx);
 if(c1!=NULL)
 {
  c1->re+=XPRM_POP_REAL(ctx);
  XPRM_PUSH_REF(ctx,c1);
  return XPRM_RT_OK;
 }
 else
  return cx_new1r(ctx,libctx);
}

/**************************************/
/* Change of sign complex -> -complex */
/**************************************/
static int cx_neg(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;

 c1=XPRM_POP_REF(ctx);
 if(c1!=NULL)
 {
  c1->re=-c1->re;
  c1->im=-c1->im;
 }
 XPRM_PUSH_REF(ctx,c1);
 return XPRM_RT_OK;
}

/**************************************/
/* Product complex*complex -> complex */
/**************************************/
static int cx_mul(XPRMcontext ctx,void *libctx)
{
 s_complex *c1,*c2;
 double re,im;

 c1=XPRM_POP_REF(ctx);
 c2=XPRM_POP_REF(ctx);
 if(c1!=NULL)
 {
  if(c2!=NULL)
  {
   re=c1->re*c2->re-c1->im*c2->im;
   im=c1->re*c2->im+c1->im*c2->re;
   c1->re=re;
   c1->im=im;
  }
  else
   c1->re=c2->im=0;
 }
 cx_delete(ctx,libctx,c2,0);
 XPRM_PUSH_REF(ctx,c1);
 return XPRM_RT_OK;
}

/***********************************/
/* Product complex*real -> complex */
/***********************************/
static int cx_mul_r(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;
 double r;

 c1=XPRM_POP_REF(ctx);
 r=XPRM_POP_REAL(ctx);
 if(c1!=NULL)
 {
  c1->re*=r;
  c1->im*=r;
 }
 XPRM_PUSH_REF(ctx,c1);
 return XPRM_RT_OK;
}

/***************************************/
/* Division complex/complex -> complex */
/***************************************/
static int cx_div(XPRMcontext ctx,void *libctx)
{
 s_complex *c1,*c2;
 double re,im;

 c1=XPRM_POP_REF(ctx);
 c2=XPRM_POP_REF(ctx);
 if((c2==NULL)||((c2->re==0)&&(c2->im==0)))
 {
  mm->dispmsg(ctx,"Complex: Division by 0.");
  return XPRM_RT_ERROR;
 }
 else
 {
  if(c1!=NULL)
  {                             /* Compute 1/c2 then c1* 1/c2 */
   re=c2->re/(c2->re*c2->re+c2->im*c2->im);
   im=-c2->im/(c2->re*c2->re+c2->im*c2->im);
   c2->re=re;
   c2->im=im;
   XPRM_PUSH_REF(ctx,c2);
   XPRM_PUSH_REF(ctx,c1);
   return cx_mul(ctx,libctx);
  }
  else
  {
   cx_delete(ctx,libctx,c2,0);
   XPRM_PUSH_REF(ctx,c1);
   return XPRM_RT_OK;
  }
 }
}

/************************************/
/* Division complex/real -> complex */
/************************************/
static int cx_div_r1(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;
 double r;

 c1=XPRM_POP_REF(ctx);
 r=XPRM_POP_REAL(ctx);
 if(r==0)
 {
  mm->dispmsg(ctx,"Complex: Division by 0.");
  return XPRM_RT_ERROR;
 }
 else
 {
  if(c1!=NULL)
  {
   c1->re/=r;
   c1->im/=r;
  }
  XPRM_PUSH_REF(ctx,c1);
  return XPRM_RT_OK;
 }
}

/************************************/
/* Division real/complex -> complex */
/************************************/
static int cx_div_r2(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;
 double r,re,im;

 r=XPRM_POP_REAL(ctx);
 c1=XPRM_POP_REF(ctx);
 if((c1==NULL)||((c1->re==0)&&(c1->im==0)))
 {
  mm->dispmsg(ctx,"Complex: Division by 0.");
  return XPRM_RT_ERROR;
 }
 else
 {
  re=(c1->re*r)/(c1->re*c1->re+c1->im*c1->im);
  im=-(c1->im*r)/(c1->re*c1->re+c1->im*c1->im);
  c1->re=re;
  c1->im=im;
  XPRM_PUSH_REF(ctx,c1);
  return XPRM_RT_OK;
 }
}

/***************************/
/* Comparison complex=real */
/***************************/
static int cx_eql_r(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;
 double r;
 int b;

 c1=XPRM_POP_REF(ctx);
 r=XPRM_POP_REAL(ctx);
 if(c1!=NULL)
  b=(c1->im==0)&&(c1->re==r);
 else
  b=(r==0);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/***********************************/
/* Get imaginary part of a complex */
/***********************************/
static int cx_getim(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;

 c1=XPRM_POP_REF(ctx);
 if(c1!=NULL)
  XPRM_PUSH_REAL(ctx,c1->im);
 else
  XPRM_PUSH_REAL(ctx,0);
 return RT_OK;
}

/******************************/
/* Get real part of a complex */
/******************************/
static int cx_getre(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;

 c1=XPRM_POP_REF(ctx);
 if(c1!=NULL)
  XPRM_PUSH_REAL(ctx,c1->re);
 else
  XPRM_PUSH_REAL(ctx,0);
 return RT_OK;
}

/***********************************/
/* Set imaginary part of a complex */
/***********************************/
static int cx_setim(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;
 double im;

 c1=XPRM_POP_REF(ctx);
 im=XPRM_POP_REAL(ctx);
 if(c1==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized complex.\n");
  return RT_ERROR;
 }
 else
 {
  c1->im=im;
  return RT_OK;
 }
}

/******************************/
/* Set real part of a complex */
/******************************/
static int cx_setre(XPRMcontext ctx,void *libctx)
{
 s_complex *c1;
 double re;

 c1=XPRM_POP_REF(ctx);
 re=XPRM_POP_REAL(ctx);
 if(c1==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized complex.\n");
  return RT_ERROR;
 }
 else
 {
  c1->re=re;
  return RT_OK;
 }
}

/**************** Type-related functions ****************/

/*****************************/
/* Allocate a complex number */
/*****************************/
static void *cx_create(XPRMcontext ctx,void *libctx,void *todup,int typnum)
{
 s_complex *complex;

 if((todup!=NULL)&&(XPRM_CREATE(typnum)==XPRM_CREATE_NEW))
 {
  /* Do not update the reference count if the object is shared */
  /* the global entity will be cleared when the model is reset */
  /* Note that this example is not complete: the implementation should */
  /* guarantee that concurrent access to a given shared object does not */
  /* corrupt the datastructure using e.g. critical sections */
  if((((s_complex *)todup)->refcnt&CX_SHARED)==0)
   ((s_complex *)todup)->refcnt++;
  return todup;
 }
 else
 {
  complex=mm->memalloc(ctx,sizeof(s_complex),0);
  if(XPRM_CREATE(typnum)==XPRM_CREATE_CST)
  {
   complex->re=((s_complex *)todup)->re;
   complex->im=((s_complex *)todup)->im;
   complex->refcnt=1|CX_CONST;
  }
  else
  {
   complex->re=complex->im=0;
   complex->refcnt=1;
   /* Tag a shared complex number to disable reference counting */
   if(XPRM_CREATE(typnum)==XPRM_CREATE_SHR)
    complex->refcnt|=CX_SHARED;
  }
  return complex;
 }
}

/*******************************/
/* Deallocate a complex number */
/*******************************/
static void cx_delete(XPRMcontext ctx,void *libctx,void *todel,int typnum)
{
 if((todel!=NULL)&&((((s_complex *)todel)->refcnt&CX_SHARED)==0)&&
    (((--((s_complex *)todel)->refcnt)&~CX_CONST)<1))
 {
  mm->memfree(ctx,todel,sizeof(s_complex));
 }
}

/*********************/
/* Complex -> String */
/*********************/
static int cx_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,int len,int typnum)
{
 s_complex *c;

 if(typnum&XPRM_TFSTR_BIN)
 {
  if(len>=2*sizeof(double))
  {
   c=toprt;
   if(toprt==NULL)
    memset(str,0,2*sizeof(double));
   else
   {
    /* We assume that all supported platforms are little endian */
    memcpy(str,&(c->re),sizeof(double));
    memcpy(str+sizeof(double),&(c->im),sizeof(double));
   }
  }
  return 2*sizeof(double);
 }
 else
 if(toprt==NULL)
 {
  strncpy(str,"0+0i",len);
  return 4;
 }
 else
 {
  c=toprt;
  return snprintf(str,len,"%g%+gi",c->re,c->im);
 }
}

/*********************/
/* String -> Complex */
/*********************/
static int cx_fromstr(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int typnum,const char **endptr)
{
 double re,im;
 s_complex *c;
 int len;
 struct Info
 {
  char dummy[4];
  s_complex *c;
 } *ref;

 c=toinit;
 if(c->refcnt&CX_CONST)
 {
  mm->dispmsg(ctx,"Complex: Trying to modify a constant.\n");
  return XPRM_RT_ERROR;
 }
 else
 if(typnum&XPRM_TFSTR_BIN)
 {
  if(*endptr-str!=2*sizeof(double))
   return XPRM_RT_ERROR;
  else
  {
   /* We assume that all supported platforms are little endian */
   memcpy(&(c->re),str,sizeof(double));
   memcpy(&(c->im),str+sizeof(double),sizeof(double));
   return RT_OK;
  }
 }
 else
 if((str[0]=='r') && (str[1]=='a') && (str[2]=='w') && (str[3]=='\0'))
 {
  if(endptr!=NULL) *endptr=NULL;
  ref=(struct Info *)str;
  if(ref->c==NULL)
   c->re=c->im=0;
  else
  {
   c->re=ref->c->re;
   c->im=ref->c->im;
  }
  return XPRM_RT_OK;
 }
 else
  if(sscanf(str,"%lf%lf%ni%n",&re,&im,&len,&len)<2)
  {
   if(endptr!=NULL) *endptr=str;
   return XPRM_RT_ERROR;
  }
  else
  {
   if(endptr!=NULL) *endptr=str+len;
   c->re=re;
   c->im=im;
   return XPRM_RT_OK;
  }
}

/******************/
/* Copy a complex */
/******************/
static int cx_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum)
{
 s_complex *c_dst;

 c_dst=(s_complex *)toinit;
 switch(XPRM_CPY(typnum))
 {
  case XPRM_CPY_COPY:
  case XPRM_CPY_RESET:
      if(c_dst->refcnt&CX_CONST)
       return 1;
      else
      if(src!=NULL)
      {
       c_dst->re=((s_complex *)src)->re;
       c_dst->im=((s_complex *)src)->im;
       return 0;
      }
      else
      {
       c_dst->re=0;
       c_dst->im=0;
       return 0;
      }
  case XPRM_CPY_APPEND:
      if(src!=NULL)
      {
       c_dst->re+=((s_complex *)src)->re;
       c_dst->im+=((s_complex *)src)->im;
      }
      return 0;
  case XPRM_CPY_HASH:
      {
       unsigned int hv;

       if(src==NULL)
       {
        double z=0;

        hv=mm->hashmix(ctx,0,&z,sizeof(double));
        *(unsigned int *)toinit=mm->hashmix(ctx,hv,&z,sizeof(double));
       }
       else
       {
        hv=mm->hashmix(ctx,0,&(((s_complex*)src)->re),sizeof(double));
        *(unsigned int *)toinit=mm->hashmix(ctx,hv,&(((s_complex*)src)->im),sizeof(double));
       }
       return 0;
      }
  default:
      return 1;
 }
}

/****************************/
/* Compare 2 complex values */
/****************************/
static int cx_compare(XPRMcontext ctx,void *libctx,void *r1,void *r2,int typnum)
{
 static const s_complex cstzero;
 const s_complex *c1,*c2;

 c1=(r1==NULL)?&cstzero:r1;
 c2=(r2==NULL)?&cstzero:r2;
 
 switch(XPRM_COMPARE(typnum))
 {
  case XPRM_COMPARE_EQ:
    return (c1->re==c2->re)&&(c1->im==c2->im);
  case XPRM_COMPARE_NEQ:
    return (c1->re!=c2->re)||(c1->im!=c2->im);
  case XPRM_COMPARE_LTH:
    return (c1->re<c2->re)||((c1->re==c2->re)&&(c1->im<c2->im));
  case XPRM_COMPARE_LEQ:
    return (c1->re<c2->re)||((c1->re==c2->re)&&(c1->im<=c2->im));
  case XPRM_COMPARE_GEQ:
    return (c1->re>c2->re)||((c1->re==c2->re)&&(c1->im>=c2->im));
  case XPRM_COMPARE_GTH:
    return (c1->re>c2->re)||((c1->re==c2->re)&&(c1->im>c2->im));
  case XPRM_COMPARE_CMP:
    if(c1->re==c2->re)
    {
     if(c1->im==c2->im)
      return 0;
     else
      return (c1->im<c2->im)?-1:1;
    }
    else
     return (c1->re<c2->re)?-1:1;
  default:
    return XPRM_COMPARE_ERROR;
 }
}

/**************** Iterator 'realonly' ***************/

/***********************/
/* Allocate a realonly */
/***********************/
static void *reo_create(XPRMcontext ctx,void *libctx,void *todup,int typnum)
{
 if(todup!=NULL)
 {
  ((s_realonly *)todup)->refcnt++;
  return todup;
 }
 else
 {
  s_realonly *itr;

  itr=mm->memalloc(ctx,sizeof(s_realonly),1);
  itr->refcnt=1;
  return itr;
 }
}

/**********************/
/* Release a realonly */
/**********************/
static void reo_delete(XPRMcontext ctx,void *libctx,void *todel,int typnum)
{
 if((todel!=NULL)&&((--((s_realonly *)todel)->refcnt)<1))
 {
  reo_reset(ctx,libctx,todel,NULL,0);
  mm->memfree(ctx,todel,sizeof(s_realonly));
 }
}

/********************/
/* Reset a realonly */
/********************/
static int reo_reset(XPRMcontext ctx,void *libctx,void *toreset,void *src,int ust)
{
 s_realonly *itr=toreset;

 if(itr!=NULL)
 {
  if(itr->reftab!=NULL)
  {
   mm->delref(ctx,XPRM_STR_ARR,itr->reftab);
   itr->reftab=NULL;
  }
  memset(itr->indices,0,sizeof(int)*MAXITNDX);
  itr->nbdim=0;
  itr->status=0;
 }
 return 0;
}

/******************** Services ********************/

/**************************************/
/* Reset the Complex module for a run */
/**************************************/
/* Dummy context to enable 'memoryuse' */
static void *cx_reset(XPRMcontext ctx,void *libctx,int version)
{
 if(libctx==NULL)               /* libctx==NULL => initialisation */
  return (void*)1l;
 else
  return NULL;
}

/*****************************/
/* Memory used by the module */
/*****************************/
static size_t cx_memuse(XPRMcontext ctx,void *libctx,void *ref,int code)
{
 switch(code)
 {
  case 0:
    return 0;
  case 1:
    return sizeof(s_complex);
  default:
    return -1;
 }
}

/*******************************************/
/* Extract the index tuple from a realonly */
/*******************************************/
static int cx_getarrind(XPRMcontext ctx,void*libctx,void *x,int cde,XPRMarray arr,int *indices,int op)
{
 s_realonly *itr;

 itr=x;
 if(itr==NULL)
 {
  mm->dispmsg(ctx,"Complex: Trying to access an uninitialized realonly.\n");
  return -1;
 }
 else
 if(itr->reftab==NULL)
 {
  mm->dispmsg(ctx,"Complex: Realonly not associated to any array.\n");
  return -1;
 }
 else
 if(arr!=itr->reftab)
 {
  mm->dispmsg(ctx,"Complex: Invalid array reference for a realonly.\n");
  return -1;
 }
 else
 if(itr->status!=ITR_READY)
 {
  if(op<=XPRM_OPNDX_DEL)
   return 1;
  else
  {
   mm->dispmsg(ctx,"Complex: Realonly in an invalid state.\n");
   return -1;
  }
 }
 else
 {
  memcpy(indices,itr->indices,itr->nbdim*sizeof(int));
  return 0;
 }
}

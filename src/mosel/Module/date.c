/******************************************
  Mosel NI Examples
  =================

  File date.c
  ```````````
  Example module defining a new type
    date
  with functions for accessing it and a
  control parameter for the style.

  (c) 2008 Fair Isaac Corporation
      author: Y. Colombani, 2002
*******************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "xprm_ni.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

#define NDTL 20               /* Number of dates to allocate at once */

/**** Function prototypes ****/
static int dt_getpar(XPRMcontext ctx,void *libctx);
static int dt_setpar(XPRMcontext ctx,void *libctx);
static int dt_new0(XPRMcontext ctx,void *libctx);
static int dt_asgn(XPRMcontext ctx,void *libctx);
static int dt_eql(XPRMcontext ctx,void *libctx);
static int dt_less(XPRMcontext ctx,void *libctx);
static int dt_leq(XPRMcontext ctx,void *libctx);
static int dt_gettoday(XPRMcontext ctx,void *libctx);
static int dt_getyear(XPRMcontext ctx,void *libctx);
static int dt_getmonth(XPRMcontext ctx,void *libctx);
static int dt_getday(XPRMcontext ctx,void *libctx);
static int dt_gethour(XPRMcontext ctx,void *libctx);
static int dt_getminute(XPRMcontext ctx,void *libctx);
static int dt_getsecond(XPRMcontext ctx,void *libctx);
static void *dt_create(XPRMcontext ctx,void *,void *,int);
static void dt_delete(XPRMcontext ctx,void *,void *,int);
static int dt_tostr(XPRMcontext ctx,void *,void *,char *,int,int);
static int dt_fromstr(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int,const char **endp);
static int dt_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum);
static void *dt_reset(XPRMcontext ctx,void *libctx,int version);
static int dt_findparm(const char *name,int *type);
static void *dt_nextparm(void *ref,const char **name,const char **desc,
                                                                    int *type);
/**** Structures for passing info to Mosel ****/
/* Constants */
static XPRMdsoconst tabconst[]=
        {
         XPRM_CST_BOOL("DATE_EU",XPRM_TRUE),XPRM_CST_BOOL("DATE_US",XPRM_FALSE)
        };

/* Subroutines */
static XPRMdsofct tabfct[]=
        { /* Implementing Mosel subroutines get/setparam for this module: */
         {"",XPRM_FCT_GETPAR,XPRM_TYP_NOT,0,NULL,dt_getpar},
         {"",XPRM_FCT_SETPAR,XPRM_TYP_NOT,0,NULL,dt_setpar},
  /* Operators: */
         {"@&",1000,XPRM_TYP_EXTN,1,"date:|date|",dt_new0},
         {"@:",1001,XPRM_TYP_NOT,2,"|date||date|",dt_asgn},
         {"@=",1002,XPRM_TYP_BOOL,2,"|date||date|",dt_eql},
         {"@<",1003,XPRM_TYP_BOOL,2,"|date||date|",dt_less},
         {"@l",1004,XPRM_TYP_BOOL,2,"|date||date|",dt_leq},
  /* Functions for accessing date info: */
         {"getdatenow",1005,XPRM_TYP_EXTN,0,"date:",dt_gettoday},
         {"getyear",1006,XPRM_TYP_INT,1,"|date|",dt_getyear},
         {"getmonth",1007,XPRM_TYP_INT,1,"|date|",dt_getmonth},
         {"getday",1008,XPRM_TYP_INT,1,"|date|",dt_getday},
         {"gethour",1009,XPRM_TYP_INT,1,"|date|",dt_gethour},
         {"getminute",1010,XPRM_TYP_INT,1,"|date|",dt_getminute},
         {"getsecond",1011,XPRM_TYP_INT,1,"|date|",dt_getsecond}
        };

/* Types */
static XPRMdsotyp tabtyp[]=
        {
         {"date",1,XPRM_DTYP_PNCTX,dt_create,dt_delete,dt_tostr,dt_fromstr,dt_copy}
        };

/* Services */
static XPRMdsoserv tabserv[]=
        {
         {XPRM_SRV_RESET,(void *)dt_reset},
         {XPRM_SRV_PARAM,(void *)dt_findparm},
         {XPRM_SRV_PARLST,(void *)dt_nextparm}
        };

/* Interface structure */
static XPRMdsointer dsointer= 
        { 
         sizeof(tabconst)/sizeof(XPRMdsoconst),tabconst,
         sizeof(tabfct)/sizeof(XPRMdsofct),tabfct,
         sizeof(tabtyp)/sizeof(XPRMdsotyp),tabtyp,
         sizeof(tabserv)/sizeof(XPRMdsoserv),tabserv
        };

/**** Structures used by this module ****/
static XPRMnifct mm;             /* To store the Mosel NI function table */

typedef struct tm s_date;       /* A date */

typedef union Freelist          /* List of allocated but not used dates */
        {
         s_date dt;
         union Freelist *next;
        } u_freelist;
        
typedef struct Nmlist           /* A block of memory */
        {
         s_date list[NDTL];
         int nextfree;
         struct Nmlist *next;
        } s_nmlist;

typedef struct                  /* A context for this module */
        {
         u_freelist *freelist;
         s_nmlist *nmlist;
         int in_eu;             /* Date style (EU/US) */
        } s_dtctx;


/************************************************/
/* Initialize the library just after loading it */
/************************************************/
DSO_INIT date_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The Mosel NI version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/******** Functions implementing subroutines and operators ********/

/****************************/
/* Read a control parameter */
/****************************/
static int dt_getpar(XPRMcontext ctx,void *libctx)
{
 s_dtctx *dtctx;
 int n;

 n=XPRM_POP_INT(ctx);
 if(n!=0) return XPRM_RT_ERROR;
 else
 {
  dtctx=libctx;
  XPRM_PUSH_INT(ctx,dtctx->in_eu);
  return XPRM_RT_OK;
 }
}

/***************************/
/* Set a control parameter */
/***************************/
static int dt_setpar(XPRMcontext ctx,void *libctx)
{
 s_dtctx *dtctx;
 int n;

 n=XPRM_POP_INT(ctx);
 if(n!=0) return XPRM_RT_ERROR;
 else
 {
  dtctx=libctx;
  dtctx->in_eu=XPRM_POP_INT(ctx);
  return XPRM_RT_OK;
 }
}

/****************/
/* Clone a date */
/****************/
static int dt_new0(XPRMcontext ctx,void *libctx)
{
 s_date *date,*new_date;

 date=XPRM_POP_REF(ctx);
 if(date!=NULL)
 {
  new_date=dt_create(ctx,libctx,NULL,0);
  *new_date=*date;
  XPRM_PUSH_REF(ctx,new_date);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

/*************************/
/* Assignment date:=date */
/*************************/
static int dt_asgn(XPRMcontext ctx,void *libctx)
{
 s_date *d1,*d2;

 d1=XPRM_POP_REF(ctx);
 d2=XPRM_POP_REF(ctx);
 *d1=*d2;
 dt_delete(ctx,libctx,d2,0);
 return XPRM_RT_OK;
}

/************************/
/* Comparison date=date */
/************************/
static int dt_eql(XPRMcontext ctx,void *libctx)
{
 s_date *d1,*d2;
 int b;

 d1=XPRM_POP_REF(ctx);
 d2=XPRM_POP_REF(ctx);
 if(d1!=NULL)
 {
  if(d2!=NULL)
   b=memcmp(d1,d2,sizeof(s_date))==0;
  else
   b=0;
 }
 else
  b=(d2==NULL);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/************************/
/* Comparison date<date */
/************************/
static int dt_less(XPRMcontext ctx,void *libctx)
{
 s_date *d1,*d2;
 int b;

 d1=XPRM_POP_REF(ctx);
 d2=XPRM_POP_REF(ctx);
 if(d1!=NULL)
 {
  if(d2!=NULL)
   b=mktime(d1)<mktime(d2);
  else
   b=0;
 }
 else
  b=(d2==NULL);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/*************************/
/* Comparison date<=date */
/*************************/
static int dt_leq(XPRMcontext ctx,void *libctx)
{
 s_date *d1,*d2;
 int b;

 d1=XPRM_POP_REF(ctx);
 d2=XPRM_POP_REF(ctx);
 if(d1!=NULL)
 {
  if(d2!=NULL)
   b=mktime(d1)<=mktime(d2);
  else
   b=0;
 }
 else
  b=(d2==NULL);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/************************/
/* Get the current date */
/************************/
static int dt_gettoday(XPRMcontext ctx,void *libctx)
{
 s_date *d,*ds;
 time_t t;

 d=dt_create(ctx,libctx,NULL,0);
 t=time(NULL);
 ds=localtime(&t);
 *d=*ds;
 XPRM_PUSH_REF(ctx,d);
 return XPRM_RT_OK;
}

/****************************/
/* Get the year from a date */
/****************************/
static int dt_getyear(XPRMcontext ctx,void *libctx)
{
 s_date *d;

 d=XPRM_POP_REF(ctx);
 if(d!=NULL)
  XPRM_PUSH_INT(ctx,d->tm_year+1900);
 else
  XPRM_PUSH_INT(ctx,1900);
 return XPRM_RT_OK;
}

/*****************************/
/* Get the month from a date */
/*****************************/
static int dt_getmonth(XPRMcontext ctx,void *libctx)
{
 s_date *d;

 d=XPRM_POP_REF(ctx);
 if(d!=NULL)
  XPRM_PUSH_INT(ctx,d->tm_mon+1);
 else
  XPRM_PUSH_INT(ctx,1);
 return XPRM_RT_OK;
}

/***************************/
/* Get the day from a date */
/***************************/
static int dt_getday(XPRMcontext ctx,void *libctx)
{
 s_date *d;

 d=XPRM_POP_REF(ctx);
 if(d!=NULL)
  XPRM_PUSH_INT(ctx,d->tm_mday);
 else
  XPRM_PUSH_INT(ctx,1);
 return XPRM_RT_OK;
}

/****************************/
/* Get the hour from a date */
/****************************/
static int dt_gethour(XPRMcontext ctx,void *libctx)
{
 s_date *d;

 d=XPRM_POP_REF(ctx);
 if(d!=NULL)
  XPRM_PUSH_INT(ctx,d->tm_hour);
 else
  XPRM_PUSH_INT(ctx,0);
 return XPRM_RT_OK;
}

/******************************/
/* Get the minute from a date */
/******************************/
static int dt_getminute(XPRMcontext ctx,void *libctx)
{
 s_date *d;

 d=XPRM_POP_REF(ctx);
 if(d!=NULL)
  XPRM_PUSH_INT(ctx,d->tm_min);
 else
  XPRM_PUSH_INT(ctx,0);
 return XPRM_RT_OK;
}

/******************************/
/* Get the second from a date */
/******************************/
static int dt_getsecond(XPRMcontext ctx,void *libctx)
{
 s_date *d;

 d=XPRM_POP_REF(ctx);
 if(d!=NULL)
  XPRM_PUSH_INT(ctx,d->tm_sec);
 else
  XPRM_PUSH_INT(ctx,0);
 return XPRM_RT_OK;
}

/***************/
/* Copy a date */
/***************/
static int dt_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum)
{
 s_date *dst;

 dst=(s_date *)toinit;
 if(src!=NULL)
  *dst=*(s_date *)src;
 else
  memset(dst,0,sizeof(s_date));
 return 0;
}

/**************** Type-related functions ****************/

/*******************/
/* Allocate a date */
/*******************/
static void *dt_create(XPRMcontext ctx,void *libctx,void *todup,int typnum)
{
 s_dtctx *dtctx;
 s_date *date;
 s_nmlist *nmlist;

 dtctx=libctx;
 if(dtctx->freelist!=NULL)              /* We have got some free date */
 {
  date=&(dtctx->freelist->dt);
  dtctx->freelist=dtctx->freelist->next;
 }
 else                                   /* We must allocate a new block */
  if((dtctx->nmlist==NULL)||(dtctx->nmlist->nextfree>=NDTL))
  {
   nmlist=malloc(sizeof(s_nmlist));
   nmlist->next=dtctx->nmlist;
   dtctx->nmlist=nmlist;
   nmlist->nextfree=1;
   date=nmlist->list;
  }
  else                                  /* We can take one from the block */
   date=&(dtctx->nmlist->list[dtctx->nmlist->nextfree++]);
 memset(date,0,sizeof(s_date));
 return date;
}

/*********************/
/* Deallocate a date */
/*********************/
static void dt_delete(XPRMcontext ctx,void *libctx,void *todel,int typnum)
{
 s_dtctx *dtctx;
 u_freelist *freelist;

 if(todel!=NULL)
 {
  dtctx=libctx;
  freelist=todel;
  freelist->next=dtctx->freelist;
  dtctx->freelist=freelist;
 }
}

/******************/
/* Date -> String */
/******************/
static int dt_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,int len,int typnum)
{
 s_dtctx *dtctx;
 s_date *d;

 if(toprt==NULL)
 {
  strncpy(str,"1/1/1900 00:00:00",len);
  return 17;
 }
 else
 {
  dtctx=libctx;
  d=toprt;
  if(dtctx->in_eu)
   return snprintf(str,len,"%d/%d/%d %02d:%02d:%02d",d->tm_mday,d->tm_mon+1,
                 d->tm_year+1900,d->tm_hour,d->tm_min,d->tm_sec);
  else
   return snprintf(str,len,"%d/%d/%d %02d:%02d:%02d",d->tm_mon+1,d->tm_mday,
                 d->tm_year+1900,d->tm_hour,d->tm_min,d->tm_sec);
 }
}

/******************/
/* String -> Date */
/******************/
static int dt_fromstr(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int typnum,const char **endp)
{
 s_dtctx *dtctx;
 int day,month,year,cnt;
 int hour,minute,second;
 s_date *d;
 struct Info
 {
  char dummy[4];
  s_date *d;
 } *ref;

 hour=minute=second=0;
 dtctx=libctx;
 d=toinit;

 if((str[0]=='r') && (str[1]=='a') && (str[2]=='w') && (str[3]=='\0'))
 {
  if(endp!=NULL) *endp=NULL;
  ref=(struct Info *)str;
  if(ref->d==NULL)
   memset(d,0,sizeof(s_date));
  else
   *d=*ref->d;
  return XPRM_RT_OK;
 }
 else
  if(sscanf(str,"%d/%d/%d%n %d%n:%d%n:%d%n",&day,&month,&year,&cnt,
  				&hour,&cnt,&minute,&cnt,&second,&cnt)<3)
  {
   if(endp!=NULL) *endp=str;
   return XPRM_RT_ERROR;
  }
  else
  {
   if(dtctx->in_eu)
   {
    d->tm_mday=day;
    d->tm_mon=month-1;
   }
   else
   {
    d->tm_mday=month;
    d->tm_mon=day-1;
   }
   d->tm_year=year-1900;
   d->tm_hour=hour;
   d->tm_min=minute;
   d->tm_sec=second;
   if(endp!=NULL) *endp=str+cnt;
   return XPRM_RT_OK;
  }
}

/******************** Services ********************/

/**************************************/
/* Reset the Complex module for a run */
/**************************************/
static void *dt_reset(XPRMcontext ctx,void *libctx,int version)
{
 s_dtctx *dtctx;
 s_nmlist *nmlist;

 if(libctx==NULL)               /* libctx==NULL => initialisation */
 {
  dtctx=malloc(sizeof(s_dtctx));
  memset(dtctx,0,sizeof(s_dtctx));
  dtctx->in_eu=XPRM_TRUE;       /* By default we are in Europe ! */
  return dtctx;
 }
 else                           /* otherwise release the resources we use */
 {
  dtctx=libctx;
  while(dtctx->nmlist!=NULL)
  {
   nmlist=dtctx->nmlist;
   dtctx->nmlist=nmlist->next;
   free(nmlist);
  }
  free(dtctx);
  return NULL;
 }
}

/****************************/
/* Find a control parameter */
/****************************/
static int dt_findparm(const char *name,int *type)
{
 if(strcmp(name,"date_style")==0)
 {
  *type=XPRM_TYP_BOOL|XPRM_CPAR_READ|XPRM_CPAR_WRITE;
  return 0;
 }
 else
  return -1;
}

/**********************************/
/* Return the next parameter name */
/**********************************/
static void *dt_nextparm(void *ref,const char **name,const char **desc,
                                                                     int *type)
{
 if(ref!=NULL)
  return NULL;
 else
 {
  *name="date_style";
  *type=XPRM_TYP_BOOL|XPRM_CPAR_READ|XPRM_CPAR_WRITE;
  *desc="Selects the date style (US/EU)";
 
  return (void *)(1);
 }
}


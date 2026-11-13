/******************************************
  Mosel NI Examples
  =================
    
  File euro.c
  ```````````
  Example module defining three new types
    FrFranc
    DMark
    Euro
  with the corresponding operators.

  (c) 2008 Fair Isaac Corporation
      author: S. Heipcke, 2002
*******************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xprm_ni.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

#define NML 20            /* Number of money structures to allocate at once */

#define CURR_EUR 0
#define CURR_FR 1
#define CURR_DM 2
#define NCURR 3

static const double CONV[]={1, 6.55957, 1.95583};

/* Conversion rates:
   1 Euro =  40.3399   Belgian Franc
             1.95583   German Mark
             340.750   Greek Drachme
             166.386   Spanish Peseta 
             6.55957   French Franc 
             0.787564  Irish Pound
             1936.27   Italian Lira
             40.3399   Luxemburg Franc
             2.20371   Dutch Guilder
             13.7603   Austrian Schilling
             200.482   Portuguese Escudo
             5.94573   Finnish Mark
*/
                                      
/**** Function prototypes ****/
static int euro_clone(XPRMcontext ctx,void *libctx);
static int euro_newE(XPRMcontext ctx,void *libctx);
static int euro_newF(XPRMcontext ctx,void *libctx);
static int euro_newD(XPRMcontext ctx,void *libctx);
static int euro_zeroE(XPRMcontext ctx,void *libctx);
static int euro_zeroF(XPRMcontext ctx,void *libctx);
static int euro_zeroD(XPRMcontext ctx,void *libctx);
static int euro_oneE(XPRMcontext ctx,void *libctx);
static int euro_oneF(XPRMcontext ctx,void *libctx);
static int euro_oneD(XPRMcontext ctx,void *libctx);
static int euro_asgn(XPRMcontext ctx,void *libctx);
static int euro_asgn_convE(XPRMcontext ctx,void *libctx);
static int euro_asgn_convF(XPRMcontext ctx,void *libctx);
static int euro_asgn_convD(XPRMcontext ctx,void *libctx);
static int euro_pls(XPRMcontext ctx,void *libctx);
static int euro_pls_conv(XPRMcontext ctx,void *libctx);
static int euro_mul(XPRMcontext ctx,void *libctx);
static int euro_neg(XPRMcontext ctx,void *libctx);
static int euro_eql(XPRMcontext ctx,void *libctx);
static int euro_eql_conv(XPRMcontext ctx,void *libctx);
static int euro_convE(XPRMcontext ctx,void *libctx);
static int euro_convF(XPRMcontext ctx,void *libctx);
static int euro_convD(XPRMcontext ctx,void *libctx);
static void *euro_create(XPRMcontext ctx,void *libctx,void *,int);
static void euro_delete(XPRMcontext ctx,void *libctx,void *todel,int);
static int euro_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,int len,int typnum);
static int euro_fromstrE(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int, const char **endp);
static int euro_fromstrF(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int, const char **endp);
static int euro_fromstrD(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int, const char **endp);
static int euro_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum);
static void *euro_reset(XPRMcontext ctx,void *libctx,int version);

/**** Structures for passing info to Mosel ****/
/* Subroutines */
static mm_dsofct tabfct[]=
        {
         {"@&",1000,XPRM_TYP_EXTN,1,"Euro:|Euro|",euro_clone},
         {"@&",1002,XPRM_TYP_EXTN,1,"FrFranc:|FrFranc|",euro_clone},
         {"@&",1003,XPRM_TYP_EXTN,1,"DMark:|DMark|",euro_clone},
         {"@&",1004,XPRM_TYP_EXTN,1,"Euro:r",euro_newE},
         {"@&",1005,XPRM_TYP_EXTN,1,"FrFranc:r",euro_newF},
         {"@&",1006,XPRM_TYP_EXTN,1,"DMark:r",euro_newD},
         {"@0",1011,XPRM_TYP_EXTN,0,"Euro:",euro_zeroE},
         {"@0",1012,XPRM_TYP_EXTN,0,"FrFranc:",euro_zeroF},
         {"@0",1013,XPRM_TYP_EXTN,0,"DMark:",euro_zeroD},
         {"@:",1021,XPRM_TYP_NOT,2,"|Euro||Euro|",euro_asgn},
         {"@:",1022,XPRM_TYP_NOT,2,"|FrFranc||FrFranc|",euro_asgn},
         {"@:",1023,XPRM_TYP_NOT,2,"|DMark||DMark|",euro_asgn},
         {"@:",1024,XPRM_TYP_NOT,2,"|Euro||FrFranc|",euro_asgn_convE},
         {"@:",1025,XPRM_TYP_NOT,2,"|Euro||DMark|",euro_asgn_convE},
         {"@:",1026,XPRM_TYP_NOT,2,"|FrFranc||Euro|",euro_asgn_convF},
         {"@:",1027,XPRM_TYP_NOT,2,"|FrFranc||DMark|",euro_asgn_convF},
         {"@:",1028,XPRM_TYP_NOT,2,"|DMark||Euro|",euro_asgn_convD},
         {"@:",1029,XPRM_TYP_NOT,2,"|DMark||FrFranc|",euro_asgn_convD},
         {"@+",1031,XPRM_TYP_EXTN,2,"Euro:|Euro||Euro|",euro_pls},
         {"@+",1032,XPRM_TYP_EXTN,2,"Euro:|Euro||DMark|",euro_pls_conv},
         {"@+",1033,XPRM_TYP_EXTN,2,"Euro:|Euro||FrFranc|",euro_pls_conv},
         {"@+",1034,XPRM_TYP_EXTN,2,"Euro:|DMark||FrFranc|",euro_pls_conv},
         {"@+",1035,XPRM_TYP_EXTN,2,"FrFranc:|FrFranc||FrFranc|",euro_pls},
         {"@+",1036,XPRM_TYP_EXTN,2,"DMark:|DMark||DMark|",euro_pls},
         {"@*",1041,XPRM_TYP_EXTN,2,"Euro:r|Euro|",euro_mul},
         {"@*",1042,XPRM_TYP_EXTN,2,"FrFranc:r|FrFranc|",euro_mul},
         {"@*",1043,XPRM_TYP_EXTN,2,"DMark:r|DMark|",euro_mul},
         {"@-",1051,XPRM_TYP_EXTN,1,"Euro:|Euro|",euro_neg},
         {"@-",1052,XPRM_TYP_EXTN,1,"FrFranc:|FrFranc|",euro_neg},
         {"@-",1053,XPRM_TYP_EXTN,1,"DMark:|DMark|",euro_neg},
         {"@=",1061,XPRM_TYP_BOOL,2,"|Euro||Euro|",euro_eql},
         {"@=",1062,XPRM_TYP_BOOL,2,"|Euro||FrFranc|",euro_eql_conv},
         {"@=",1063,XPRM_TYP_BOOL,2,"|Euro||DMark|",euro_eql_conv},
         {"@=",1064,XPRM_TYP_BOOL,2,"|FrFranc||Euro|",euro_eql_conv},
         {"@=",1065,XPRM_TYP_BOOL,2,"|FrFranc||FrFranc|",euro_eql},
         {"@=",1066,XPRM_TYP_BOOL,2,"|FrFranc||DMark|",euro_eql_conv},
         {"@=",1067,XPRM_TYP_BOOL,2,"|DMark||Euro|",euro_eql_conv},
         {"@=",1068,XPRM_TYP_BOOL,2,"|DMark||FrFranc|",euro_eql_conv},
         {"@=",1069,XPRM_TYP_BOOL,2,"|DMark||DMark|",euro_eql},
         {"@&",1071,XPRM_TYP_EXTN,1,"Euro:|FrFranc|",euro_convE},
         {"@&",1072,XPRM_TYP_EXTN,1,"Euro:|DMark|",euro_convE},
         {"@&",1073,XPRM_TYP_EXTN,1,"FrFranc:|Euro|",euro_convF},
         {"@&",1074,XPRM_TYP_EXTN,1,"FrFranc:|DMark|",euro_convF},
         {"@&",1075,XPRM_TYP_EXTN,1,"DMark:|Euro|",euro_convD},
         {"@&",1076,XPRM_TYP_EXTN,1,"DMark:|FrFranc|",euro_convD},
         {"EUR",1081,XPRM_TYP_EXTN,0,"Euro:",euro_oneE},
         {"FF",1082,XPRM_TYP_EXTN,0,"FrFranc:",euro_oneF},
         {"DM",1083,XPRM_TYP_EXTN,0,"DMark:",euro_oneD}
        };

/* Types */
static mm_dsotyp tabtyp[]=
        {
         {"Euro",1,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT,euro_create,euro_delete,euro_tostr,euro_fromstrE,euro_copy},
         {"DMark",2,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT,euro_create,euro_delete,euro_tostr,euro_fromstrD,euro_copy},
         {"FrFranc",3,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT,euro_create,euro_delete,euro_tostr,euro_fromstrF,euro_copy},
        };

/* Services */
static mm_dsoserv tabserv[]=
        {
         {XPRM_SRV_RESET,(void *)euro_reset}
        };

/* Interface structure */
static mm_dsointer dsointer= 
        { 
         0,NULL,
         sizeof(tabfct)/sizeof(mm_dsofct),tabfct,
         sizeof(tabtyp)/sizeof(mm_dsotyp),tabtyp,
         sizeof(tabserv)/sizeof(mm_dsoserv),tabserv
        };

/**** Structures used by this module ****/
static mm_nifct mm;           /* For storing Mosel NI function table */

typedef struct                /* A money amount */
        {
	 int refcnt;          /* For reference count */
         int curr;
         double val;
        } s_money;

typedef union Freelist        /* List of allocated but not used money amounts */
        {
         s_money mon;
         union Freelist *next;
        } u_freelist;
        
typedef struct Moneylist      /* A block of memory */
        {
         s_money list[NML];
         int nextfree;
         struct Moneylist *next;
        } s_mlist;

typedef struct                /* Module context */
        {
         u_freelist *freelist;
         s_mlist *mlist;
        } s_euroctx;

/*************************/
/* Initialize the module */
/*************************/
DSO_INIT euro_init(mm_nifct nifct, int *interver,int *libver, mm_dsointer **interf)
{
 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/******** Functions implementing the operators ********/
/*******************/
/* Clone a complex */
/*******************/
static int euro_clone(XPRMcontext ctx,void *libctx)
{
 s_money *old_mon,*new_mon;

 old_mon=XPRM_POP_REF(ctx);
 if(old_mon!=NULL)
 {
  new_mon=euro_create(ctx,libctx,NULL,0);
  *new_mon=*old_mon;
  XPRM_PUSH_REF(ctx,new_mon);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

/*************************************/
/* Create a money amount from a real */
/*************************************/
static int euro_newE(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->val=XPRM_POP_REAL(ctx);
 mon->curr=CURR_EUR;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

static int euro_newF(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->val=XPRM_POP_REAL(ctx);
 mon->curr=CURR_FR;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

static int euro_newD(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->val=XPRM_POP_REAL(ctx);
 mon->curr=CURR_DM;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

/*****************/
/* Zero elements */
/*****************/
static int euro_zeroE(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->curr=CURR_EUR;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

static int euro_zeroF(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->curr=CURR_FR;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

static int euro_zeroD(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->curr=CURR_DM;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

/****************/
/* One elements */
/****************/
static int euro_oneE(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->curr=CURR_EUR;
 mon->val=1;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

static int euro_oneF(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->curr=CURR_FR;
 mon->val=1;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

static int euro_oneD(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=euro_create(ctx,libctx,NULL,0);
 mon->curr=CURR_DM;
 mon->val=1;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

/*************************/
/* Assignment same types */
/*************************/
static int euro_asgn(XPRMcontext ctx,void *libctx)
{
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 m1->curr=m2->curr;
 m1->val=m2->val;
 euro_delete(ctx,libctx,m2,0);
 return XPRM_RT_OK;
}

/******************************/
/* Assignment with conversion */
/******************************/
static int euro_asgn_convE(XPRMcontext ctx,void *libctx)
{
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 m1->curr=CURR_EUR;
 m1->val=m2->val*CONV[m1->curr]/CONV[m2->curr];
 euro_delete(ctx,libctx,m2,0);
 return XPRM_RT_OK;
}

static int euro_asgn_convF(XPRMcontext ctx,void *libctx)
{
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 m1->curr=CURR_FR;
 m1->val=m2->val*CONV[m1->curr]/CONV[m2->curr];
 euro_delete(ctx,libctx,m2,0);
 return XPRM_RT_OK;
}

static int euro_asgn_convD(XPRMcontext ctx,void *libctx)
{
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 m1->curr=CURR_DM;
 m1->val=m2->val*CONV[m1->curr]/CONV[m2->curr];
 euro_delete(ctx,libctx,m2,0);
 return XPRM_RT_OK;
}

/***********************/
/* Addition same types */
/***********************/
static int euro_pls(XPRMcontext ctx,void *libctx)
{
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 if(m1!=NULL)
 {
  if(m2!=NULL)
  {
   m1->val+=m2->val;
   euro_delete(ctx,libctx,m2,0);
  }
  XPRM_PUSH_REF(ctx,m1);
 }
 else
  XPRM_PUSH_REF(ctx,m2);
 return XPRM_RT_OK;
}

/****************************/
/* Addition with conversion */
/****************************/
static int euro_pls_conv(XPRMcontext ctx,void *libctx)
{
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 if(m1!=NULL)
 {
  m1->val=(m1->val)/CONV[m1->curr]; 
  m1->curr=CURR_EUR;
  if(m2!=NULL)
  {
   m1->val+=(m2->val)/CONV[m2->curr];
   euro_delete(ctx,libctx,m2,0);
  }
  XPRM_PUSH_REF(ctx,m1);
 }
 else
 {
  if(m2!=NULL)
   m2->val=(m2->val)/CONV[m2->curr];
  XPRM_PUSH_REF(ctx,m2);
 } 
 return XPRM_RT_OK;
}

/*******************************/
/* Product real*money -> money */
/*******************************/
static int euro_mul(XPRMcontext ctx,void *libctx)
{
 s_money *mon;
 double r;

 r=XPRM_POP_REAL(ctx);
 mon=XPRM_POP_REF(ctx);
 if(mon!=NULL)
  mon->val*=r;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

/**********************************/
/* Change of sign money -> -money */
/**********************************/
static int euro_neg(XPRMcontext ctx,void *libctx)
{
 s_money *mon;

 mon=XPRM_POP_REF(ctx);
 if(mon!=NULL)
  mon->val=-mon->val;
 XPRM_PUSH_REF(ctx,mon);
 return XPRM_RT_OK;
}

/******************************/
/* Comparison (same currency) */
/******************************/
static int euro_eql(XPRMcontext ctx,void *libctx)
{
 int b;
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 if(m1!=NULL)
 {
  if(m2!=NULL)
   b=(m1->val==m2->val);
  else
   b=0;
 }
 else
  b=(m2==NULL);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/***************************/
/* Comparison (conversion) */
/***************************/
static int euro_eql_conv(XPRMcontext ctx,void *libctx)
{
 int b;
 s_money *m1,*m2;

 m1=XPRM_POP_REF(ctx);
 m2=XPRM_POP_REF(ctx);
 if(m1!=NULL)
 {
  if(m2!=NULL)
   b=(m1->val/CONV[m1->curr]==m2->val/CONV[m2->curr]);
  else
   b=0;
 }
 else 
  b=(m2==NULL);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/*******************/
/* Type conversion */
/*******************/
static int euro_convE(XPRMcontext ctx,void *libctx)
{
 s_money *old_mon,*new_mon;

 old_mon=XPRM_POP_REF(ctx);
 if(old_mon!=NULL)
 {
  new_mon=euro_create(ctx,libctx,NULL,0);
  new_mon->curr=CURR_EUR;
  new_mon->val=(old_mon->val)/CONV[old_mon->curr];
  XPRM_PUSH_REF(ctx,new_mon);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

static int euro_convF(XPRMcontext ctx,void *libctx)
{
 s_money *old_mon,*new_mon;

 old_mon=XPRM_POP_REF(ctx);
 if(old_mon!=NULL)
 {
  new_mon=euro_create(ctx,libctx,NULL,0);
  new_mon->curr=CURR_FR;
  new_mon->val=(old_mon->val)*CONV[CURR_FR]/CONV[old_mon->curr];
  XPRM_PUSH_REF(ctx,new_mon);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

static int euro_convD(XPRMcontext ctx,void *libctx)
{
 s_money *old_mon,*new_mon;

 old_mon=XPRM_POP_REF(ctx);
 if(old_mon!=NULL)
 {
  new_mon=euro_create(ctx,libctx,NULL,0);
  new_mon->curr=CURR_DM;
  new_mon->val=(old_mon->val)*CONV[CURR_DM]/CONV[old_mon->curr];
  XPRM_PUSH_REF(ctx,new_mon);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

/**************** Type-related functions ****************/

/***************************/
/* Allocate a money amount */
/***************************/
static void *euro_create(XPRMcontext ctx,void *libctx,void *todup,int typnum)
{
 s_euroctx *ectx;
 s_money *mon;
 s_mlist *mlist;

 if(todup!=NULL)
 {
  ((s_money *)todup)->refcnt++;
  return todup;
 }
 else
 {
  ectx=libctx;
  if(ectx->freelist!=NULL)                /* We have got some free space */
  {
   mon=&(ectx->freelist->mon);
   ectx->freelist=ectx->freelist->next;
  }
  else                                   /* We must allocate a new block */
   if((ectx->mlist==NULL)||(ectx->mlist->nextfree>=NML))
   {
    mlist=malloc(sizeof(s_mlist));
    mlist->next=ectx->mlist;
    ectx->mlist=mlist;
    mlist->nextfree=1;
    mon=mlist->list;
   }
   else                                  /* We can take one from the block */
    mon=&(ectx->mlist->list[ectx->mlist->nextfree++]);
  mon->val=0;
  mon->curr=0;
  return mon;
 }
}


/*****************************/
/* Deallocate a money amount */
/*****************************/
static void euro_delete(XPRMcontext ctx,void *libctx,void *todel,int typnum)
{
 s_euroctx *ectx;
 u_freelist *freelist;

 if((todel!=NULL)&&((--((s_money *)todel)->refcnt)<1))
 {
  ectx=libctx;
  freelist=todel;
  freelist->next=ectx->freelist;
  ectx->freelist=freelist;
 }
}

/**************************/
/* Money amount -> String */
/**************************/
static int euro_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,int len,int typnum)
{
 s_money *mon;

 if(toprt==NULL)
 {
  strncpy(str,"0",len);
  return 1;
 }
 else
 {
  mon=toprt;
  switch(mon->curr)
  {
   case CURR_FR: return snprintf(str,len,"%2.2fFF", mon->val); break;
   case CURR_DM: return snprintf(str,len,"%2.2fDM", mon->val); break;
   default: return snprintf(str,len,"%2.2fEUR", mon->val); break;
  } 
 }
}

/**************************/
/* String -> Money amount */
/**************************/
static int euro_fromstrE(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int typnum, const char **endp)
{
 int cnt;
 double val;
 s_money *mon;
 struct Info
 {
  char dummy[4];
  s_money *m;
 } *ref;

 mon=toinit;
 if((str[0]=='r') && (str[1]=='a') && (str[2]=='w') && (str[3]=='\0'))
 {
  if(endp!=NULL) *endp=NULL;
  ref=(struct Info *)str;
  if(ref->m==NULL)
  {
   mon->val=0;
   mon->curr=CURR_EUR;
  }
  else
  {
   mon->curr=ref->m->curr;
   mon->val=ref->m->val;
  }
  return XPRM_RT_OK;
 }
 else
  if(sscanf(str,"%lf%n",&val,&cnt)<1)
  {
   if(endp!=NULL) *endp=str;
   return XPRM_RT_ERROR;
  }
  else
  {
   mon->val=val;
   mon->curr=CURR_EUR;
   if(endp!=NULL) *endp=str+cnt;
   return XPRM_RT_OK;
  }
}

static int euro_fromstrF(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int typnum, const char **endp)
{
 int cnt;
 double val;
 s_money *mon;
 struct Info
 {
  char dummy[4];
  s_money *m;
 } *ref;

 mon=toinit;
 if((str[0]=='r') && (str[1]=='a') && (str[2]=='w') && (str[3]=='\0'))
 {
  if(endp!=NULL) *endp=NULL;
  ref=(struct Info *)str;
  if(ref->m==NULL)
  {
   mon->val=0;
   mon->curr=CURR_FR;
  }
  else
  {
   mon->curr=ref->m->curr;
   mon->val=ref->m->val;
  }
  return XPRM_RT_OK;
 }
 else
  if(sscanf(str,"%lf%n",&val,&cnt)<1)
  {
   if(endp!=NULL) *endp=str;
   return XPRM_RT_ERROR;
  }
  else
  {
   mon->val=val;
   mon->curr=CURR_FR;
   if(endp!=NULL) *endp=str+cnt;
   return XPRM_RT_OK;
  }
}

static int euro_fromstrD(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int typnum, const char **endp)
{
 int cnt;
 double val;
 s_money *mon;
 struct Info
 {
  char dummy[4];
  s_money *m;
 } *ref;

 mon=toinit;
 if((str[0]=='r') && (str[1]=='a') && (str[2]=='w') && (str[3]=='\0'))
 {
  if(endp!=NULL) *endp=NULL;
  ref=(struct Info *)str;
  if(ref->m==NULL)
  {
   mon->val=0;
   mon->curr=CURR_DM;
  }
  else
  {
   mon->curr=ref->m->curr;
   mon->val=ref->m->val;
  }
  return XPRM_RT_OK;
 }
 else
  if(sscanf(str,"%lf%n",&val,&cnt)<1)
  {
   if(endp!=NULL) *endp=str;
   return XPRM_RT_ERROR;
  }
  else
  {
   mon->val=val;
   mon->curr=CURR_DM;
   if(endp!=NULL) *endp=str+cnt;
   return XPRM_RT_OK;
  }
}

/***************/
/* Copy a euro */
/***************/
static int euro_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum)
{
 s_money *mon_dst;

 mon_dst=(s_money *)toinit;
 if(src!=NULL)
 {
  mon_dst->curr=((s_money *)src)->curr;
  mon_dst->val=((s_money *)src)->val;
 }
 else
  mon_dst->val=0;
 return 0;
}

/******************** Services ********************/

/***********************************/
/* Reset the Euro module for a run */
/***********************************/
static void *euro_reset(XPRMcontext ctx,void *libctx,int version)
{
 s_euroctx *ectx;
 s_mlist *mlist;

 if(libctx==NULL)               /* libctx==NULL => initialisation */
 {
  ectx=malloc(sizeof(s_euroctx));
  memset(ectx,0,sizeof(s_euroctx));
  return ectx;
 }
 else                           /* otherwise release the resources we use */
 {
  ectx=libctx;
  while(ectx->mlist!=NULL)
  {
   mlist=ectx->mlist;
   ectx->mlist=mlist->next;
   free(mlist);
  }
  free(ectx);
  return NULL;
 }
}


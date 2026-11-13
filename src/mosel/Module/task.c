/******************************************
  Mosel NI Examples
  =================
 
  File task.c
  ```````````
  Example module defining a new type
    task
  regrouping information related to tasks
  in scheduling problems.
  (Short example for User Guide: only
   rudimentary memory management)

  (c) 2008 Fair Isaac Corporation
      author: S. Heipcke, 2002
*******************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xprm_ni.h"

#ifdef _WIN32
#define alloca(a) _alloca(a)
#define snprintf _snprintf
#else
#include <alloca.h>
#endif

#define TASK_MAXNAME 128
#define TASK_MAXTIME 100
#define TASK_MEM 20
#define TASK_NUMPARAM 2

/**** Function prototypes ****/
DSO_INIT task_init(XPRMnifct nifct, int *interver,int *libver, 
    XPRMdsointer **interf);
static int task_getpar(XPRMcontext ctx,void *libctx);
static int task_setpar(XPRMcontext ctx,void *libctx);
static int task_getname(XPRMcontext ctx,void *libctx);
static int task_getdur(XPRMcontext ctx,void *libctx);
static int task_getaflag(XPRMcontext ctx,void *libctx);
static int task_getdue(XPRMcontext ctx,void *libctx);
static int task_setname(XPRMcontext ctx,void *libctx);
static int task_setdur(XPRMcontext ctx,void *libctx);
static int task_setaflag(XPRMcontext ctx,void *libctx);
static int task_setdue(XPRMcontext ctx,void *libctx);
static int task_clone(XPRMcontext ctx,void *libctx);
static int task_new1(XPRMcontext ctx,void *libctx);
static int task_new2(XPRMcontext ctx,void *libctx);
static int task_new3(XPRMcontext ctx,void *libctx);
static int task_new4(XPRMcontext ctx,void *libctx);
static int task_new5(XPRMcontext ctx,void *libctx);
static int task_assign(XPRMcontext ctx,void *libctx);
static int task_eql(XPRMcontext ctx,void *libctx);
static void *task_create(XPRMcontext ctx,void *libctx,void *,int);
static void task_delete(XPRMcontext ctx,void *libctx,void *todel,int);
static int task_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,
    int len,int typnum);
static int task_fromstr(XPRMcontext ctx,void *libctx,void *toinit,
    const char *str,int typnum, const char **endp);
static int task_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum);
static int task_findparam(const char *name,int *type);
static void *task_nextparam(void *ref,const char **name,const char **desc,
    int *type);
static void *task_reset(XPRMcontext ctx,void *libctx,int version);

/**** Structures for passing info to Mosel ****/
/* Constants */
 static XPRMdsoconst tabconst[]=
    {
     XPRM_CST_BOOL("TASK_FLAG",1),         /* Constant with value true */
     XPRM_CST_BOOL("TASK_NOFLAG",0)        /* Constant with value false */
    };

/* Subroutines */
static XPRMdsofct tabfct[]=
    { /* Implementing Mosel subroutines get/setparam for this module: */
     {"",XPRM_FCT_GETPAR,XPRM_TYP_NOT,0,NULL,task_getpar},
     {"",XPRM_FCT_SETPAR,XPRM_TYP_NOT,0,NULL,task_setpar},
  /* Accessing task info: */
     {"getname",1000,XPRM_TYP_STRING,1,"|task|",task_getname},
     {"getduration",1002,XPRM_TYP_REAL,1,"|task|",task_getdur},
     {"getaflag",1003,XPRM_TYP_BOOL,1,"|task|",task_getaflag},
     {"getduedate",1004,XPRM_TYP_INT,1,"|task|",task_getdue},
     {"setname",1005,XPRM_TYP_NOT,2,"|task|s",task_setname},
     {"setduration",1006,XPRM_TYP_NOT,2,"|task|r",task_setdur},
     {"setaflag",1007,XPRM_TYP_NOT,2,"|task|b",task_setaflag},
     {"setduedate",1008,XPRM_TYP_NOT,2,"|task|i",task_setdue},
  /* Constructors for tasks: */
     {"@&",1011,XPRM_TYP_EXTN,1,"task:|task|",task_clone},
     {"@&",1012,XPRM_TYP_EXTN,1,"task:s",task_new1},
     {"@&",1013,XPRM_TYP_EXTN,1,"task:r",task_new2},
     {"@&",1014,XPRM_TYP_EXTN,2,"task:sr",task_new3},
     {"@&",1015,XPRM_TYP_EXTN,4,"task:srbi",task_new4},
     {"@&",1016,XPRM_TYP_EXTN,3,"task:rbi",task_new5},
  /* Operators for assignment and comparison: */
     {"@:",1020,XPRM_TYP_NOT,2,"|task||task|",task_assign},
     {"@=",1021,XPRM_TYP_BOOL,2,"|task||task|",task_eql}
    };

/* Types */
 static XPRMdsotyp tabtyp[]=
    {
     {"task",1,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT,task_create,task_delete,task_tostr,task_fromstr,task_copy}
    };

/* Services */
 static XPRMdsoserv tabserv[]=
    {
     {XPRM_SRV_PARAM,(void *)task_findparam},  /* Req. by parameters*/
     {XPRM_SRV_PARLST,(void *)task_nextparam}, /* Opt. with parameters*/
     {XPRM_SRV_RESET,(void *)task_reset}       /* Req. by types */
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
static XPRMnifct mm;         /* For storing Mosel NI function table */

typedef struct Task         /* A task */
     {
      int refcnt;
      const char *name;
      int aflag,duedate;
      double duration;
      struct Task *next;
     } s_task;

typedef struct             /* Context of this module */
    {
     s_task *firsttask;
     int maxname;
     double maxtime;
    } s_taskctx;

static struct              /* Parameters published by this module */
    {
     char *name;
     int type;
     char *desc;
    } taskparams[]={
         {"taskmaxtime",XPRM_TYP_REAL|XPRM_CPAR_READ|XPRM_CPAR_WRITE, 
          "a time limit value"},
         {"tasknamelength",XPRM_TYP_INT|XPRM_CPAR_READ|XPRM_CPAR_WRITE, 
          "maximum length of task names"}
        };


/*******************************************************/
/* Initialize the module library just after loading it */
/*******************************************************/
DSO_INIT task_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                   /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;        /* Mosel NI version */
 *libver=XPRM_MKVER(0,0,1);    /* Module version */
 *interf=&dsointer;          /* Pass info about module contents to Mosel */

 return 0;
}

/******** Functions implementing subroutines and operators ********/

/*******************************/
/* Getting a control parameter */
/*******************************/
static int task_getpar(XPRMcontext ctx,void *libctx)
{
 s_taskctx *taskctx;
 int n;

 taskctx=libctx;
 n=XPRM_POP_INT(ctx);
 switch(n)
 {
  case 0: XPRM_PUSH_INT(ctx,taskctx->maxname); break;
  case 1: XPRM_PUSH_REAL(ctx,taskctx->maxtime); break;
  default:
	mm->dispmsg(ctx,"Task: Wrong control parameter number.\n");
	return XPRM_RT_ERROR;
 }
 return XPRM_RT_OK;
}

/*******************************/
/* Setting a control parameter */
/*******************************/
static int task_setpar(XPRMcontext ctx,void *libctx)
{
 s_taskctx *taskctx;
 int n;

 taskctx=libctx;
 n=XPRM_POP_INT(ctx);
 switch(n)
 {
  case 0: taskctx->maxname=XPRM_POP_INT(ctx); break;
  case 1: taskctx->maxtime=XPRM_POP_REAL(ctx); break;
  default:
	mm->dispmsg(ctx,"Task: Wrong control parameter number.\n");
	return XPRM_RT_ERROR;
 }

/* Test to be added if there are any read-only parameters:
 if(!(taskparams[n].type&XPRM_CPAR_WRITE))
 {
  mm->dispmsg(ctx,"Task: Control parameter is read-only.\n"));
  return XPRM_RT_ERROR;
 }
 else
*/
  return XPRM_RT_OK;
}

/*************************/
/* Getting the task name */
/*************************/
static int task_getname(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 
 task=XPRM_POP_REF(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 XPRM_PUSH_STRING(ctx,task->name);
 return XPRM_RT_OK;
}

/*****************************/
/* Getting the task duration */
/*****************************/
static int task_getdur(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 
 task=XPRM_POP_REF(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 XPRM_PUSH_REAL(ctx,task->duration);
 return XPRM_RT_OK;
}

/*************************/
/* Getting the task flag */
/*************************/
static int task_getaflag(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 
 task=XPRM_POP_REF(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 XPRM_PUSH_INT(ctx,task->aflag);
 return XPRM_RT_OK;
}

/*****************************/
/* Getting the task due date */
/*****************************/
static int task_getdue(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 
 task=XPRM_POP_REF(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 XPRM_PUSH_INT(ctx,task->duedate);
 return XPRM_RT_OK;
}

/*************************/
/* Setting the task name */
/*************************/
static int task_setname(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 const char *name; 
 
 task=XPRM_POP_REF(ctx);
 name=XPRM_POP_STRING(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 task->name=name;
 return XPRM_RT_OK;
}

/*****************************/
/* Setting the task duration */
/*****************************/
static int task_setdur(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 double dur;
 
 task=XPRM_POP_REF(ctx);
 dur=XPRM_POP_REAL(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 task->duration=dur;
 return XPRM_RT_OK;
}

/*************************/
/* Setting the task flag */
/*************************/
static int task_setaflag(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 int af;
 
 task=XPRM_POP_REF(ctx);
 af=XPRM_POP_INT(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 task->aflag=af;
 return XPRM_RT_OK;
}

/*****************************/
/* Setting the task due date */
/*****************************/
static int task_setdue(XPRMcontext ctx,void *libctx)
{
 s_task *task;
 int due;
 
 task=XPRM_POP_REF(ctx);
 due=XPRM_POP_INT(ctx);
 if(task==NULL)
 {
  mm->dispmsg(ctx,"Task: Accessing undefined task.\n");
  return XPRM_RT_ERROR;
 }
 task->duedate=due;
 return XPRM_RT_OK;
}

/****************/
/* Clone a task */
/****************/
static int task_clone(XPRMcontext ctx,void *libctx)
{
 s_task *task,*new_task;

 task=XPRM_POP_REF(ctx);
 if(task!=NULL)
 {
  new_task=task_create(ctx,libctx,NULL,0);
  new_task->name=task->name;
  new_task->aflag=task->aflag;
  new_task->duedate=task->duedate;
  new_task->duration=task->duration;
  XPRM_PUSH_REF(ctx,new_task);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

/************************************************/
/* Create a task with                           */
/* 1 - name given                               */
/* 2 - duration given                           */
/* 3 - name and duration given                  */
/* 4 - name, duration, aflag, and duedate given */
/* 5 - duration, aflag, and duedate given       */
/************************************************/
static int task_new1(XPRMcontext ctx,void *libctx)
{
 s_task *task;

 task=task_create(ctx,libctx,NULL,0);
 task->name=XPRM_POP_STRING(ctx);
 XPRM_PUSH_REF(ctx,task);
 return XPRM_RT_OK;
}

static int task_new2(XPRMcontext ctx,void *libctx)
{
 s_task *task;

 task=task_create(ctx,libctx,NULL,0);
 task->duration=XPRM_POP_REAL(ctx);
 XPRM_PUSH_REF(ctx,task);
 return XPRM_RT_OK;
}

static int task_new3(XPRMcontext ctx,void *libctx)
{
 s_task *task;

 task=task_create(ctx,libctx,NULL,0);
 task->name=XPRM_POP_STRING(ctx);
 task->duration=XPRM_POP_REAL(ctx);
 XPRM_PUSH_REF(ctx,task);
 return XPRM_RT_OK;
}

static int task_new4(XPRMcontext ctx,void *libctx)
{
 s_task *task;

 task=task_create(ctx,libctx,NULL,0);
 task->name=XPRM_POP_STRING(ctx);
 task->duration=XPRM_POP_REAL(ctx);
 task->aflag=XPRM_POP_INT(ctx);
 task->duedate=XPRM_POP_INT(ctx);
 XPRM_PUSH_REF(ctx,task);
 return XPRM_RT_OK;
}

static int task_new5(XPRMcontext ctx,void *libctx)
{
 s_task *task;

 task=task_create(ctx,libctx,NULL,0);
 task->duration=XPRM_POP_REAL(ctx);
 task->aflag=XPRM_POP_INT(ctx);
 task->duedate=XPRM_POP_INT(ctx);
 XPRM_PUSH_REF(ctx,task);
 return XPRM_RT_OK;
}

/*************************/
/* Assignment task:=task */
/*************************/
static int task_assign(XPRMcontext ctx,void *libctx)
{
 s_task *task1,*task2;

 task1=XPRM_POP_REF(ctx);
 task2=XPRM_POP_REF(ctx);
 task1->name=task2->name;
 task1->aflag=task2->aflag;
 task1->duedate=task2->duedate;
 task1->duration=task2->duration;
 task_delete(ctx,libctx,task2,0);
 return XPRM_RT_OK;
}

/***************************/
/* Comparison of two tasks */
/***************************/
static int task_eql(XPRMcontext ctx,void *libctx)
{
 s_task *task1,*task2;
 int b;

 task1=XPRM_POP_REF(ctx);
 task2=XPRM_POP_REF(ctx);
 if(task1!=NULL)
 {
  if(task2!=NULL)
   b=((task1->name==task2->name)    /* This comparison is correct since we
                                       are using Mosel's dictionary */
   &&(task1->duration==task2->duration) && (task1->aflag==task2->aflag)
   &&(task1->duedate==task2->duedate));
  else
   b=0;
 }
 else
  b=(task2==NULL);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/**************** Type-related functions ****************/

/*****************************/
/* Allocate space for a task */
/*****************************/
static void *task_create(XPRMcontext ctx,void *libctx,void *todup,int typnum)
{
 s_taskctx *taskctx;
 s_task *task;

 if(todup!=NULL)
 {
  ((s_task *)todup)->refcnt++;
  return todup;
 }
 else
 {
  taskctx=libctx;
  task=(s_task *)malloc(sizeof(s_task));
  task->next=taskctx->firsttask;
  taskctx->firsttask=task;
  task->refcnt=1;

  task->name=NULL;                    /* Initialize the task */
  task->duration=0;
  task->aflag=task->duedate=0;
  return task;
 }
}

/*********************/
/* Deallocate a task */
/*********************/
static void task_delete(XPRMcontext ctx,void *libctx,void *todel,int typnum)
{
 s_taskctx *taskctx;
 s_task *task,*prev;

 if((todel!=NULL)&&((--((s_task *)todel)->refcnt)<1))
 {
  taskctx=libctx;
  task=todel;
  if(taskctx->firsttask==task) taskctx->firsttask=task->next;
  else
  {
   prev=taskctx->firsttask;
   while((prev->next!=NULL) && (prev->next!=task))
    prev=prev->next;
   if(prev->next==NULL) mm->dispmsg(ctx,"Task: task not found.\n");
   else prev->next=task->next;
  }
  free(task);
 }
}

/******************/
/* Task -> String */
/******************/
static int task_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,int len,int typnum)
{
 s_task *task;

 if(toprt==NULL)
  return 0;
 else
 {
  task=toprt;
  return snprintf(str,len,"%s %g %d %d",(task->name!=NULL)?task->name:"anon",
         task->duration,task->aflag,task->duedate);
 }
}

/******************/
/* String -> Task */
/******************/
static int task_fromstr(XPRMcontext ctx,void *libctx,void *toinit,
  const char *str,int typnum, const char **endp)
{
 double dur;
 int af,due,res,cnt;
 char *name;
 s_taskctx *taskctx;
 s_task *task;
 struct Info
 {
  char dummy[4];
  s_task *t;
 } *ref;

 taskctx=libctx;
 task=toinit;
 if((str[0]=='r') && (str[1]=='a') && (str[2]=='w') && (str[3]=='\0'))
 {
  if(endp!=NULL) *endp=NULL;
  ref=(struct Info *)str;
  if(ref->t==NULL)
   return RT_ERROR;
  else
  {
   task->name=ref->t->name;
   task->aflag=ref->t->aflag;
   task->duedate=ref->t->duedate;
   task->duration=ref->t->duration;
  }
  return XPRM_RT_OK;
 }
 else
 {
  name=(char *)alloca((taskctx->maxname)*sizeof(char));
  af=due=cnt=0;
  res=sscanf(str,"%s %lf %d%n %d%n",name,&dur,&af,&cnt,&due,&cnt);
  if(res<3)
  {
   if(endp!=NULL) *endp=str;
   return XPRM_RT_ERROR;
  }
  else 
  {
   task->name=mm->regstring(ctx,name);
   task->duration=dur;
   task->aflag=af;
   task->duedate=due;
   if(endp!=NULL) *endp=str+cnt;
   return XPRM_RT_OK;
  }
 }
}

/***************/
/* Copy a task */
/***************/
static int task_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum)
{
 s_task *task1,*task2;

 task1=(s_task *)toinit;
 if(src==NULL)
 {
  task1->name=NULL;
  task1->duration=0;
  task1->aflag=task1->duedate=0;
 }
 else
 {
  task2=(s_task *)src;
  task1->name=task2->name;
  task1->aflag=task2->aflag;
  task1->duedate=task2->duedate;
  task1->duration=task2->duration;
 }
 return 0;
}

/******************** Services ********************/

/****************************/
/* Find a control parameter */
/****************************/
static int task_findparam(const char *name,int *type)
{
 int n;
 int notfound;

 n=0;
 do
 {
  if((notfound=strcmp(name,taskparams[n].name))==0) break;
  n++;
 } while(taskparams[n].name!=NULL);

 if(!notfound)
 {
  *type=taskparams[n].type;
  return n;
 }
 else
  return -1;
}

/*********************************************/
/* Return the next parameter for enumeration */
/*********************************************/
static void *task_nextparam(void *ref,const char **name,const char **desc,
								int *type)
{
 long cst;

 cst=(long)ref;
 if((cst<0)||(cst>=TASK_NUMPARAM))
  return NULL;
 else
 {
  *name=taskparams[cst].name;
  *type=taskparams[cst].type;
  *desc=taskparams[cst].desc; 
  return (void *)(cst+1);
 }
}

/*************************/
/* Reset the Task module */
/*************************/
static void *task_reset(XPRMcontext ctx,void *libctx,int version)
{
 s_taskctx *taskctx;
 s_task *task;

 if(libctx==NULL)          /* At start: create the context, allocate space */
 {
  taskctx=malloc(sizeof(s_taskctx));
  memset(taskctx,0,sizeof(s_taskctx));
  taskctx->maxname=TASK_MAXNAME;
  taskctx->maxtime=TASK_MAXTIME;
  return taskctx;
 }
 else                      /* At the end: delete everything */
 {
  taskctx=libctx;
  while(taskctx->firsttask!=NULL)
  {
   task=taskctx->firsttask;
   taskctx->firsttask=task->next;
   free(task);
  }
  free(taskctx);
  return NULL;
 }
}


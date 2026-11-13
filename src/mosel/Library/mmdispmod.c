/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmdispmod.c                                    */
/*  ````````````````                                    */
/*  Example for the use of the Mosel libraries          */
/*  (display the contents of a model)                   */
/*                                                      */
/*  (c) 2021 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2006, rev. July 2026      */
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprm.h"

/* To decode version number */
#define VMAJ(r) (int)(r/1000000)
#define VMIN(r) (int)((r%1000000)/1000)
#define VREL(r) (int)((r%1000000)%1000)

/* Maximum number of annotations */
#define MAXANN 64

void dispfulltype(XPRMmodel mod,int t);
void dispval(int type,XPRMalltypes *value,int rts);
void dispprocfct(XPRMmodel mod,const char *name,const char *parms, int type);
void disptyp(XPRMmodel mod,const char **c);
void dispannot(const char *symb,const char *ann[],int n);

int main(int argc,const char *argv[])
{
 XPRMmodel mod;
 XPRMproc proc;
 XPRMalltypes value;
 void *ref,*data;
 const char *symb,*parms;
 int type,nbpar;
 int i,n;
 const char *ann[MAXANN*2];

 if(argc!=2)
 {
  printf("Usage: %s modname.bim\n",argv[0]);
  return 0;
 }
 else
 {
  i=XPRMinit();
  if((i!=0)&&(i!=32))                    /* Initialize Mosel */
   return 1;
 
  if((mod=XPRMloadmod(argv[1],NULL))!=NULL)
  {
   XPRMgetmodprop(mod,XPRM_PROP_NAME,(XPRMalltypes *)&symb);
   XPRMgetmodprop(mod,XPRM_PROP_VERSION,(XPRMalltypes *)&i);
   XPRMgetmodprop(mod,XPRM_PROP_SYSCOM,(XPRMalltypes *)&parms);
   printf("%s `%s' version %d.%d.%d\n",
                            (strncmp("PKG",parms,3)==0)?"Package":"Model",
                            symb,VMAJ(i),VMIN(i),VREL(i));

   printf("Parameters:\n");              /* List of parameters */
   ref=NULL;
   while((symb=XPRMgetnextparam(mod,&ref))!=NULL)
    printf(" %s\n",symb);
   printf("\n");
 
   printf("Requirements:\n");            /* List of requirements */
   ref=NULL;
   while((ref=XPRMgetnextreq(mod,ref,&symb,&type,&data))!=NULL)
    switch(XPRM_STR(type))
    {
     case XPRM_STR_REF:                    /* Reference: display type */
     case XPRM_STR_SET:                    /* Set: display type */
     case XPRM_STR_LIST:                   /* List: display type */
        printf(" %s: ",symb);
        dispfulltype(mod,type);
        printf("\n");
        break;
     case XPRM_STR_ARR:                    /* Array: display type */
        printf(" %s: array(%s) of ",symb,(char *)data);
        dispfulltype(mod,XPRM_TYP(type));
        printf("\n");
        break;
     case XPRM_STR_PROC:                   /* Subroutine */
        XPRMgetprocinfo((XPRMproc)data,&parms,&nbpar,&type);
        dispprocfct(mod,symb,parms,type);/* Display prototype */
        break;
     default:
        printf(" %s: ?\n",symb);
    }
   printf("\n");
 
   printf("Symbols:\n");
   ref=NULL;                             /* List of symbols */
   while((symb=XPRMgetnextident(mod,&ref))!=NULL)
   {
    type=XPRMfindident(mod,symb,&value);   /* Get information for the symbol */
    switch(XPRM_STR(type))
    {
     case XPRM_STR_CONST:                  /* Constant: display value */
        printf(" %s=",symb);
        dispval(type,&value,1);
        break;
     case XPRM_STR_REF:                    /* Reference: display type */
     case XPRM_STR_ARR:                    /* Array: display type */
     case XPRM_STR_SET:                    /* Set: display type */
     case XPRM_STR_LIST:                   /* List: display type */
        printf(" %s: ",symb);
        dispfulltype(mod,type);
        printf("\n");
        break;
     case XPRM_STR_PROC:                   /* Subroutine */
        proc=value.proc;
        do                              /* Look for all overloading proc/func */
        {
         XPRMgetprocinfo(proc,&parms,&nbpar,&type);
         dispprocfct(mod,symb,parms,type); /* Display prototype */
        }while((proc=XPRMgetnextproc(proc))!=NULL);
        break;
     case XPRM_STR_UTYP:                   /* User type */
        if(symb[0]=='&') break;
        printf(" %s: ",symb);
        if(XPRM_STR(value.integer)==XPRM_STR_REC)
        {
         XPRMgettypeprop(mod,type,XPRM_TPROP_NAME,(mm_alltypes *)&parms);
         if(!strcmp(parms,symb))
         {
          void *ref2;

          ref2=XPRMgetnextfield(mod,NULL,type,&parms,&i,&nbpar);
          if(ref2!=NULL)
          {
           printf("a record type publishing:\n");
           while(ref2!=NULL)
           {
            printf("    %s:",parms);
            dispfulltype(mod,XPRM_TYP(i));
            printf("\n");
            ref2=XPRMgetnextfield(mod,ref2,type,&parms,&i,&nbpar);
           }
          }
          else
           printf("a record type\n");
         }
         else
          printf("a user defined type (=%s)\n",parms);
        }
        else
         if(XPRM_STR(value.integer)==XPRM_STR_UNION)
         {
          printf("a union type (=");
          dispfulltype(mod,XPRM_TYP(type));
          printf(")\n");
         }
        else
         if(XPRM_STR(value.integer)==XPRM_STR_PROB)
         {
          printf("a problem type (=");
          dispfulltype(mod,XPRM_TYP(type));
          printf(")\n");
         }
        else
         if(XPRM_STR(value.integer)==XPRM_STR_PROC)
         {
          printf("a subroutine type (=");
          dispfulltype(mod,XPRM_TYP(type));
          printf(")\n");
         }
         else
         {
          printf("a user defined type (=");
          dispfulltype(mod,value.integer);
          printf(")\n");
         }
        break;
     case XPRM_STR_NTYP:                   /* Native type */
        break;
     default:
        printf(" %s: ?\n",symb);
    }
   }
   printf("\n");
 
   printf("Dependencies:\n");            /* List of required pkgs/modules */
   ref=NULL;
   while((ref=XPRMgetnextdep(mod,ref,&symb,&i,&type))!=NULL)
   {
    printf(" %s %s (%d.%d.%d)\n",type?"package":"module",symb,
                                 VMAJ(i),VMIN(i),VREL(i));
   }
   printf("\n");

   printf("Annotations:\n");            /* List of annotations */
   n=XPRMgetannotations(mod,NULL,NULL,ann,MAXANN*2);
   if(n>0) dispannot("[global]",ann,n);
   ref=NULL;                             /* List of symbols */
   while((symb=XPRMgetnextanident(mod,&ref))!=NULL)
   {
    n=XPRMgetannotations(mod,symb,NULL,ann,MAXANN*2);
    dispannot(symb,ann,n);
   }
  }
 }
 return 0;
}

/***********************/
/* Display a type name */
/***********************/
void dispfulltype(XPRMmodel mod,int t)
{
 switch(XPRM_STR(t))
 {
  case XPRM_STR_CONST:
  case XPRM_STR_REF:
    switch(XPRM_TYP(t))
    {
      case XPRM_TYP_INT: printf("integer"); break;
      case XPRM_TYP_REAL: printf("real"); break;
      case XPRM_TYP_STRING: printf("string"); break;
      case XPRM_TYP_BOOL: printf("boolean"); break;
      case XPRM_TYP_MPVAR: printf("mpvar"); break;
      case XPRM_TYP_LINCTR: printf("linctr"); break;
      default:
        {
         XPRMalltypes expn;
         int pbid,tid,firstdone;
         void *ref;

         if(XPRMgettypeprop(mod,XPRM_TYP(t),XPRM_TPROP_EXP,&expn)||
            (XPRM_STR(expn.integer)==XPRM_STR_REC)||
            (XPRM_STR(expn.integer)==XPRM_STR_PROB)||
            (XPRM_STR(expn.integer)==XPRM_STR_UNION)||
            (XPRM_STR(expn.integer)==XPRM_STR_PROC))
         {
          XPRMgettypeprop(mod,t,XPRM_TPROP_PBID,(mm_alltypes *)&pbid);
          if(pbid>=0)                   /* this is a problem type */
          {
           ref=NULL;
           firstdone=0;
           while((ref=XPRMgetnextpbcomp(mod,ref,t,&tid))!=NULL)
            if(tid==0)
            {
             printf("mpproblem");
             firstdone=1;               /* this one is always the first */
            }
            else
            {
             if(firstdone)
              printf("+");
             else
              firstdone=1;
             XPRMgettypeprop(mod,XPRM_TYP(tid),XPRM_TPROP_NAME,&expn);
             printf("%s",expn.string);
            }
          }
          else
          if(XPRM_STR(expn.integer)==XPRM_STR_UNION)
          {
           ref=XPRMgetnextuncomptype(mod,NULL,t,&tid);
           dispfulltype(mod,XPRM_TYP(tid));
           while((ref=XPRMgetnextuncomptype(mod,ref,t,&tid))!=NULL)
           {
            printf(" or ");
            if(tid==0)
             printf("any");
            else
             dispfulltype(mod,XPRM_TYP(tid));
           }
          }
          else
          if(XPRM_STR(expn.integer)==XPRM_STR_PROC)
          {
           XPRMalltypes p,rt;

           XPRMgettypeprop(mod,XPRM_TYP(t),XPRM_TPROP_SIGN,&p);
           XPRMgettypeprop(mod,XPRM_TYP(t),XPRM_TPROP_EXP,&rt);
           dispprocfct(mod,NULL,p.string,XPRM_TYP(rt.integer));
          }
          else
          {
           XPRMgettypeprop(mod,XPRM_TYP(t),XPRM_TPROP_NAME,&expn);
           printf("%s",expn.string);
          }
         }
         else
          dispfulltype(mod,expn.integer);
        }
    }
    break;
  case XPRM_STR_ARR:
    printf("array of ");
    dispfulltype(mod,XPRM_TYP(t));
    break;
  case XPRM_STR_SET:
    printf("set of ");
    dispfulltype(mod,XPRM_TYP(t));
    break;
  case XPRM_STR_LIST:
    printf("list of ");
    dispfulltype(mod,XPRM_TYP(t));
    break;
  case XPRM_STR_CSREF:
    printf("constant ");
    dispfulltype(mod,XPRM_TYP(t));
    break;
  default:
    printf("?");
 }
}

/*******************/
/* Display a value */
/*******************/
void dispval(int type,XPRMalltypes *value,int rts)
{
 switch(XPRM_TYP(type))
 {
  case XPRM_TYP_INT: printf("%d",value->integer);break;
  case XPRM_TYP_REAL: printf("%g",value->real);break;
  case XPRM_TYP_STRING: printf("`%s'",value->string!=NULL?value->string:"");
                        break;
  case XPRM_TYP_BOOL: printf("%s",value->boolean==0?"false":"true");break;
  default: printf("?");
 }
 if(rts) printf("\n");
}

/***************************************/
/* Diplay a prototype from a signature */
/***************************************/
void dispprocfct(XPRMmodel mod,const char *name,const char *parms, int type)
{
 const char *c;

 if(name!=NULL)
 {
  if(XPRM_TYP(type)!=XPRM_TYP_NOT)
   printf(" function %s",name);
  else
   printf(" procedure %s",name);
 }
 else
  printf("%s",(XPRM_TYP(type)!=XPRM_TYP_NOT)?"function":"procedure");

 if((parms!=NULL)&&(parms[0]!='\0'))
 {
  printf("(");
  c=parms;
  while(*c!='\0')
  {
   if(c>parms) printf(",");
   disptyp(mod,&c);
   c++;
  }
  printf(")");
 }

 if(XPRM_TYP(type)!=XPRM_TYP_NOT)
 {
  printf(":");
  dispfulltype(mod,XPRM_TYP(type));
 }
 if(name!=NULL) printf("\n");
}

/****************************************/
/* Display a type name from a signature */
/****************************************/
void disptyp(XPRMmodel mod,const char **c)
{
 const char *s;

 switch(**c)
 {
  case 'i': printf("integer");break;
  case 'r': printf("real");break;
  case 's': printf("string");break;
  case 'b': printf("boolean");break;
  case 'v': printf("mpvar");break;
  case 'c': printf("linctr");break;
  case 'I': printf("range");break;
  case 'a': printf("array");break;
  case 'e': printf("set");break;
  case 'l': printf("list");break;
  case 'u': printf("any");break;
  case '%':
        {
         char tn[4];

         (*c)++;
         tn[0]=(*(*c)++);
         tn[1]=(*(*c)++);
         tn[2]=(*(*c));
         tn[3]='\0';
         dispfulltype(mod,(int)strtol(tn,NULL,16));
         break;
        }
  case '|':
        (*c)++;
        do
        {
         printf("%c",**c);
         (*c)++;
        } while(**c!='|');
        break;
  case '!':
        (*c)++;
        do
        {
         printf("%c",**c);
         (*c)++;
        } while(**c!='!');
        break;
  case 'A':
        printf("array (");
        s=++(*c);
        while(**c!='.')
        {
         if(s!=*c) printf(",");
         disptyp(mod,c);
         (*c)++;
        }
        printf(") of ");
        (*c)++;
        disptyp(mod,c);
        break;
  case 'E':
        printf("set of ");
        (*c)++;
        disptyp(mod,c);
        break;
  case 'L':
        printf("list of ");
        (*c)++;
        disptyp(mod,c);
        break;
  case '*':
        printf("...");
        break;
  default: printf("?");
 }
}

/*********************************/
/* Display a list of annotations */
/*********************************/
void dispannot(const char *symb,const char *ann[],int n)
{
 int i;

 printf(" %s->\n",symb);
 for(i=0;i<n && i<MAXANN;i+=2)
  printf("   %s:%s\n",ann[i],(ann[i+1]!=NULL)?ann[i+1]:"");
}

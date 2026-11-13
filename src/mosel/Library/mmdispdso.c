/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmdispdso.c                                    */
/*  ````````````````                                    */
/*  Example for the use of the Mosel libraries          */
/*  (display the contents of a module)                  */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2001, rev. July 2026      */
/********************************************************/

#include <stdio.h>
#include <string.h>
#include "xprm.h"

/* To decode version number */
#define VMAJ(r) (int)(r/1000000)
#define VMIN(r) (int)((r%1000000)/1000)
#define VREL(r) (int)((r%1000000)%1000)

const char *rwstatus(int t);
const char *typnam(int t);
void dispval(int type,XPRMalltypes *value,int rts);
void dispprocfct(const char *name,const char *parms, int type);
void disptyp(const char **c);

int main(int argc,char *argv[])
{
 XPRMdsolib dsolib;
 void *ref;
 const char *name,*parms,*comm,*path;
 int nbpar,type,priority;
 XPRMalltypes value;
 int i;
 unsigned int flag;

 if(argc!=2)
 {
  printf("Usage: %s libname\n",argv[0]);
  return 0;
 }
 else
 {
  i=XPRMinit();
  if((i!=0)&&(i!=32))                   /* Initialize Mosel */
   return 1;
 
  if((dsolib=XPRMpreloaddso(argv[1]))!=NULL)
  {
   XPRMgetdsoprop(dsolib,XPRM_PROP_VERSION,(XPRMalltypes *)&i);
   XPRMgetdsoprop(dsolib,XPRM_PROP_SYSCOM,(XPRMalltypes *)&comm);
   XPRMgetdsoprop(dsolib,XPRM_PROP_PATH,(XPRMalltypes *)&path);
   XPRMgetdsoprop(dsolib,XPRM_PROP_PRIORITY,(XPRMalltypes *)&priority);
   printf("Module `%s' version %d.%d.%d (%s)\n",
          argv[1],VMAJ(i),VMIN(i),VREL(i),(comm!=NULL)?comm:"");
   printf("File: %s/%s.dso\n",path,argv[1]);
   printf("Priority: %d\n",priority);
   ref=NULL;
   printf("Constants:\n");              /* List of constants */
   while((ref=XPRMgetnextdsoconst(dsolib,ref,&name,&type,&value))!=NULL)
   {
    printf(" %s=",name);                /* Display the value */
    dispval(type,&value,1);
   }
   printf("\n");

   ref=NULL;
   printf("Types:\n");              /* List of types */
   while((ref=XPRMgetnextdsotype(dsolib,ref,&name,&flag))!=NULL)
   {
    printf(" %s (",name);                /* Display the name */
    if(flag&XPRM_MTP_CREAT) printf("create");
    if(flag&XPRM_MTP_DELET) printf(",delete");
    if(flag&XPRM_MTP_PROB) printf(",problem");
    if(flag&XPRM_MTP_RFCNT) printf(",refcnt");
    if(flag&XPRM_MTP_TOSTR) printf(",tostring");
    if(flag&XPRM_MTP_PRTBL) printf("+");
    if(flag&XPRM_MTP_FRSTR) printf(",fromstring");
    if(flag&XPRM_MTP_TFBIN) printf(",bin");
    if(flag&XPRM_MTP_SHARE) printf(",share");
    if(flag&XPRM_MTP_ORD)   printf(",ord");
    if(flag&XPRM_MTP_CONST) printf(",const");
    if(flag&XPRM_MTP_ORSET) printf(",reset");
    if(flag&XPRM_MTP_COPY)  printf(",copy");
    if(flag&XPRM_MTP_APPND) printf("+");
    printf(")\n");
   }
   printf("\n");

   ref=NULL;
   printf("Control Parameters:\n");     /* List of control parameters */
   while((ref=XPRMgetnextdsoparam(dsolib,ref,&name,&comm,&type))!=NULL)
    if((comm!=NULL)&&(comm[0]!='\0'))
     printf(" %s: %s (%s,%s)\n",name,typnam(type),comm,rwstatus(type));
    else                                /* Display type and access rights */
     printf(" %s: %s (%s)\n",name,typnam(type),rwstatus(type));
   printf("\n");

   ref=NULL;
   printf("Procedure and functions:\n");/* Subroutines: show the prototype */
   while((ref=XPRMgetnextdsoproc(dsolib,ref,&name,&parms,&nbpar,&type))!=NULL)
    dispprocfct(name,parms,type);
   printf("\n");

   ref=NULL;
   printf("I/O drivers:\n");            /* IO drivers */
   do
   {
    do
     ref=XPRMgetnextiodrv(ref,&name,&parms,&comm);
    while((ref!=NULL)&&((parms==NULL)||(strcmp(parms,argv[1]))));
    if(ref!=NULL)
     printf(" %s:%s\n",name,comm!=NULL?comm:"");
   }while(ref!=NULL);
  }
 }
 return 0;
}

/************************************************/
/* Return the r/w status of a control parameter */
/************************************************/
const char *rwstatus(int t)
{
 if(t&XPRM_CPAR_READ)
 {
  if(t&XPRM_CPAR_WRITE)
   return "r/w";
  else
   return "r";
 }
 else
  if(t&XPRM_CPAR_WRITE)
   return "w";
  else
   return "?";
}

/**********************/
/* Return a type name */
/**********************/
const char *typnam(int t)
{
 switch(XPRM_TYP(t))
 {
  case XPRM_TYP_INT: return "integer";
  case XPRM_TYP_REAL: return "real";
  case XPRM_TYP_STRING: return "string";
  case XPRM_TYP_BOOL: return "boolean";
  case XPRM_TYP_MPVAR: return "mpvar";
  case XPRM_TYP_LINCTR: return "linctr";
  default: return "?";
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
  case XPRM_TYP_STRING: printf("`%s'",value->string!=NULL?value->string:"");break;
  case XPRM_TYP_BOOL: printf("%s",value->boolean==0?"false":"true");break;
  default: printf("?");
 }
 if(rts) printf("\n");
}

/***************************************/
/* Diplay a prototype from a signature */
/***************************************/
void dispprocfct(const char *name,const char *parms, int type)
{
 const char *c;
 char ext_typnam[128];
 int i;

 if(XPRM_TYP(type)!=XPRM_TYP_NOT)
  printf(" function %s",name);
 else
  printf(" procedure %s",name);

 if((parms!=NULL)&&(parms[0]!='\0'))
 {
  c=parms;
  if(XPRM_TYP(type)==XPRM_TYP_EXTN)
  {
   i=0;
   do
   {
    ext_typnam[i++]=*c;
    c++;
   } while(*c!=':');
   ext_typnam[i]='\0';
   c++;
   parms=c;             /* Do not display the 1st ',' */
  }
  if(*c!='\0')
  {
   printf("(");
   while(*c!='\0')
   {
    if(c>parms) printf(",");
    disptyp(&c);
    c++;
   }
   printf(")");
  }
 }

 if(XPRM_TYP(type)!=XPRM_TYP_NOT)
  printf(":%s\n",(XPRM_TYP(type)==XPRM_TYP_EXTN)?ext_typnam:typnam(type));
 else
  printf("\n");
}

/****************************************/
/* Display a type name from a signature */
/****************************************/
void disptyp(const char **c)
{
 const char *s;

 switch(**c)
 {
  case 'i': printf("integer");break;
  case 'r': printf("real");break;
  case 'S':
  case 's': printf("string");break;
  case 'b': printf("boolean");break;
  case 'v': printf("mpvar");break;
  case 'c': printf("linctr");break;
  case 'I': printf("range");break;
  case 'a': printf("array");break;
  case 'e': printf("set");break;
  case 'l': printf("list");break;
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
         disptyp(c);
         (*c)++;
        }
        printf(") of ");
        (*c)++;
        disptyp(c);
        break;
  case 'E':
        printf("set of ");
        (*c)++;
        disptyp(c);
        break;
  case 'L':
        printf("list of ");
        (*c)++;
        disptyp(c);
        break;
  case '*':
        printf("...");
        break;
  default: printf("?");
 }
}


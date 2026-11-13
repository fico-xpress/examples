#ifdef never_defined
(!            model source is at the end of this file                      )
#endif
/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexcbdrv.c                                    */
/*  ````````````````                                    */
/*  Example for the use of the Mosel libraries          */
/*  (using IOdrivers for data exchange)                 */
/*                                                      */
/*  (c) 2009 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2009, rev. July 2026      */
/********************************************************/

#include <stdio.h>
#include <string.h>
#include "xprm.h"

long XPRM_RTC cbmsg(XPRMmodel model,void *info,char *buf,unsigned long size);
int XPRM_RTC cbinit_from(XPRMcbinit cb,void *info,const char *label,int type,void *ref);
int XPRM_RTC cbinit_to(XPRMcbinit cb,void *info,const char *label,int type,void *ref);

int main(int argc,const char *argv[])
{
 char buf[256];
 int result,i;

 i=XPRMinit();
 if((i!=0)&&(i!=32))
  return 1;

#ifdef _WIN32
 sprintf(buf,"cb:%#Ix",(size_t)cbmsg);
#else
 sprintf(buf,"cb:%#lx",(unsigned long)cbmsg);
#endif
 XPRMsetdefstream(NULL,XPRM_F_WRITE,buf);
 
#ifdef _WIN32
 sprintf(buf,"ICBI='cb:%#Ix',ICBO='cb:%#Ix'",(size_t)cbinit_from,(size_t)cbinit_to);
#else
 sprintf(buf,"ICBI='cb:%#lx',ICBO='cb:%#lx'",(unsigned long)cbinit_from,(unsigned long)cbinit_to);
#endif
 return XPRMexecmod(NULL,"mmexcbdrv.c",buf,&result,NULL);
}

/**************************************/
/* Callback for handling Mosel output */
/**************************************/
long XPRM_RTC cbmsg(XPRMmodel model,void *info,char *buf,unsigned long size)
{
 printf("Mosel: %.*s",(int)size,buf);
 return 0;
}

/********************************************/
/* Callback for generating model input data */
/********************************************/
int XPRM_RTC cbinit_from(XPRMcbinit cb,void *info,const char *label,int type,void *ref)
{
 int i,j;
                          /* v_i:999 */
 if(strcmp(label,"v_i")==0)
 {
  XPRMcb_sendint(cb,999,0);
  return 0;
 }
 else                     /* v_r:999.99 */
 if(strcmp(label,"v_r")==0)
 {
  XPRMcb_sendreal(cb,999.99,0);
  return 0;
 }
 else                     /* v_b:false */
 if(strcmp(label,"v_b")==0)
 {
  XPRMcb_sendint(cb,0,0);
  return 0;
 }
 else                     /* v_s:"tralala" */
 if(strcmp(label,"v_s")==0)
 {
  XPRMcb_sendstring(cb,"tralala",-1,0);
  return 0;
 }
 else                     /* v_d:"2012-12-12" */
 if(strcmp(label,"v_d")==0)
 {
  XPRMcb_sendstring(cb,"2012-12-12",-1,0);
  return 0;
 }
 else                     /* s_i:[10 20 30 ... ] */
 if((strcmp(label,"s_i")==0)||(strcmp(label,"l_i")==0))
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=1;i<=10;i++)
   XPRMcb_sendint(cb,i*10,0);
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else                     /* s_d:["2001-01-11" "2002-02-21" ... ] */
 if((strcmp(label,"s_d")==0)||(strcmp(label,"l_d")==0))
 {
  char buf[64];

  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=1;i<=10;i++)
  {
   sprintf(buf,"%d-%02d-%02d",2000+i,i,i+1);
   XPRMcb_sendstring(cb,buf,-1,0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else                     /* a_i:[ (1) 10 (2) 20 ... ] */
 if(strcmp(label,"a_i")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=1;i<=10;i++)
  {
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENNDX,0);
    XPRMcb_sendint(cb,i,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSENDX,0);
   XPRMcb_sendint(cb,i*10,0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else                     /* a_x:[ (1) [ "aa1" 1.23 ] (2) [ "aa2" 2.46 ]...] */
 if(strcmp(label,"ax")==0)
 {
  char buf[32];

  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=1;i<=10;i++)
  {
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENNDX,0);
    XPRMcb_sendint(cb,i,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSENDX,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
    sprintf(buf,"aa%d",i);
    XPRMcb_sendstring(cb,buf,-1,0);
    XPRMcb_sendreal(cb,(double)i*1.23,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else                     /* r:[ 123 [ 10 20 30 ] ] */
 if(strcmp(label,"r")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  XPRMcb_sendint(cb,123,0);
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=1;i<=3;i++)
   XPRMcb_sendint(cb,i*10,0);
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else                     /* a_R:[ (1)[10 [10 20 30] ] (1)[20 [20 40 60] ]..] */
 if(strcmp(label,"a_R")==0)
 {
  XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
  for(i=1;i<=10;i++)
  {
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENNDX,0);
    XPRMcb_sendint(cb,i,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSENDX,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
   XPRMcb_sendint(cb,i*10,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_OPENLST,0);
   for(j=1;j<=3;j++)
    XPRMcb_sendint(cb,j*i*10,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
   XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  }
  XPRMcb_sendctrl(cb,XPRM_CBC_CLOSELST,0);
  return 0;
 }
 else
 {
  fprintf(stderr,"Label `%s' not found.\n",label);
  return 1;
 }
}

/******************************************/
/* Callback for getting model output data */
/******************************************/
int XPRM_RTC cbinit_to(XPRMcbinit cb,void *info,const char *label,int type,void *ref)
{
 static const char *typename[]={"none","integer","real","string","boolean"};
 const char *tn,*sn;

 if(XPRM_TYP(type)<=4) tn=typename[XPRM_TYP(type)];
 else tn="external";
 switch(XPRM_STR(type))
 {
  case XPRM_STR_CONST:
  case XPRM_STR_REF: sn="ref";break;
  case XPRM_STR_ARR: sn="array";break;
  case XPRM_STR_SET: sn="set";break;
  case XPRM_STR_LIST: sn="list";break;
  default: sn="unknown";
 }
 printf("C: %s %s %s %p\n",label,sn,tn,ref);
 return 0;
}

#ifdef never_defined
!)
 model tstcb
 uses 'mmsystem'
 parameters
  ICBI=""
  ICBO=""
 end-parameters

 public declarations
  v_i:integer
  v_r:real
  v_s:string
  v_b:boolean
  v_d:date

  s_i:set of integer
  l_i:list of integer

  s_d:set of date
  l_d:list of date

  a_i:array(range) of integer
  Rx:range
  a_s:array(Rx) of string
  a_r:array(Rx) of real

  R=public record
     i:integer
     s:set of integer
    end-record
  r:R
  a_R:array(range) of R
 end-declarations

 initialisations from ICBI
  v_i
  v_r
  v_s
  v_b
  v_d

  s_i
  l_i

  s_d
  l_d

  a_i
  [a_s,a_r] as "ax"

  r
  a_R
 end-initialisations

  writeln("v_i=",v_i)
  writeln("v_r=",v_r)
  writeln("v_s=",v_s)
  writeln("v_b=",v_b)
  writeln("v_d=",v_d)
  writeln("s_i=",s_i)
  writeln("l_i=",l_i)
  writeln("s_d=",s_d)
  writeln("l_d=",l_d)
  writeln("a_i=",a_i)
  writeln("a_r=",a_r)
  writeln("a_s=",a_s)
  writeln("r=",r)
  writeln("a_R=",a_R)

 initialisations to ICBO
  v_i
  v_r
  v_s
  v_b
  v_d

  s_i
  l_i

  s_d
  l_d

  a_i

  r
  a_R
 end-initialisations

 end-model
#endif

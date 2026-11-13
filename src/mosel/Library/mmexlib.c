/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexlib.c                                      */
/*  ``````````````                                      */
/*  Example for the use of the Mosel libraries          */
/*  (working with models and accessing the dynamic      */
/*  libraries loaded by Mosel)                          */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2001, rev. July 2026        */
/********************************************************/

#include <stdio.h>
#include "xprm.h"

int main()
{
 XPRMmodel mod;
 XPRMdsolib dso;
 const char *name,*syscom,*usrcom;
 int version,vmaj,vmin,vrel;
 int number,i;
 size_t size;

 i=XPRMinit();
 if((i!=0)&&(i!=32))                                   /* Initialize Mosel */
  return 1;

 if(XPRMloadmod("Models/burglari.bim",NULL)==NULL)     /* Load the BIM file */
  return 3;
 
 if(XPRMloadmod("Models/chess2.bim",NULL)==NULL)       /* Load the BIM file */
  return 5;

 if(XPRMloadmod("Models/trans.bim",NULL)==NULL)        /* Load the BIM file */
  return 7;
 
 printf("Models loaded:\n");
 mod = NULL;
 while((mod=XPRMgetnextmod(mod)) != NULL) /* Enumerate all loaded models */
 {                                      /* Get information about the models */
  XPRMgetmodprop(mod,XPRM_PROP_NAME,(XPRMalltypes *)&name);
  XPRMgetmodprop(mod,XPRM_PROP_ID,(XPRMalltypes *)&number);
  XPRMgetmodprop(mod,XPRM_PROP_SYSCOM,(XPRMalltypes *)&syscom);
  XPRMgetmodprop(mod,XPRM_PROP_USRCOM,(XPRMalltypes *)&usrcom);
  XPRMgetmodprop(mod,XPRM_PROP_SIZE,(XPRMalltypes *)&size);
  if(syscom==NULL) syscom="";
  if(usrcom==NULL) usrcom="";
  printf("  %d: %s (%s, `%s') size: %lu\n",number, name, syscom, usrcom,
         (unsigned long)size);
 }

 printf("Additional libraries loaded:\n");
 dso = NULL;
 while((dso=XPRMgetnextdso(dso)) != NULL) /* Enumerate loaded libraries */
 {                                      /* Get information about libraries */
  XPRMgetdsoprop(dso,XPRM_PROP_NAME,(XPRMalltypes *)&name);
  XPRMgetdsoprop(dso,XPRM_PROP_VERSION,(XPRMalltypes *)&version);
  XPRMgetdsoprop(dso,XPRM_PROP_NBREF,(XPRMalltypes *)&number);
  vmaj=(int)(version/1000000);
  vrel=version%1000000;
  vmin=(int)(vrel/1000);
  vrel%=1000;
  printf("  %s (version %d.%d.%d) used by %d model(s)\n",
         name, vmaj,vmin,vrel,number);
 }

 mod = XPRMfindmod(NULL, 2);              /* Get model with sequence number 2 */
 XPRMunloadmod(mod);                      /* Unload the model */
 printf("Unload model 2.\n");

 XPRMflushdso();                          /* Unload unused libraries */
 
 printf("Models loaded:\n");
 mod = NULL;
 while((mod=XPRMgetnextmod(mod)) != NULL) /* Enumerate all loaded models */
 {                                      /* Get information about the models */
  XPRMgetmodprop(mod,XPRM_PROP_NAME,(XPRMalltypes *)&name);
  XPRMgetmodprop(mod,XPRM_PROP_ID,(XPRMalltypes *)&number);
  XPRMgetmodprop(mod,XPRM_PROP_SYSCOM,(XPRMalltypes *)&syscom);
  XPRMgetmodprop(mod,XPRM_PROP_USRCOM,(XPRMalltypes *)&usrcom);
  if(syscom==NULL) syscom="";
  if(usrcom==NULL) usrcom="";
  printf("  %d: %s (%s, `%s')\n",number, name, syscom, usrcom);
 }

 printf("Additional libraries loaded:\n");
 dso = NULL;
 while((dso=XPRMgetnextdso(dso)) != NULL) /* Enumerate loaded libraries */
 {                                      /* Get information about libraries */
  XPRMgetdsoprop(dso,XPRM_PROP_NAME,(XPRMalltypes *)&name);
  XPRMgetdsoprop(dso,XPRM_PROP_NBREF,(XPRMalltypes *)&number);
  printf("  %s used by %d model(s)\n", name,number);
 }
 
 return 0;
}

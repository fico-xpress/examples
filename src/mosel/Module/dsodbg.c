/**************************************************
  Mosel NI Examples
  =================
    
  File dsodbg.c
  `````````````
  This program includes a module in order
  to compile it as static. This way, a
  debugger can be used with the module.
  The module name has to be replaced in the
  #include and #define lines below.
  
  Usage: dsodbg mosel_file [parameters]
  The source file is then executed with the
  provided parameters
  
  (c) 2008 Fair Isaac Corporation
      author: Y. Colombani, 2002, rev. July 2026
**************************************************/

/* Replace 'my_module' by the name of the module to compile */
#include "my_module.c"
#define SRC my_module

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprm.h"

#define dbg_xstr(s) dbg_str(s)
#define dbg_str(s) #s
#define dbg_xinit(s) dbg_init(s)
#define dbg_init(s) s ## _init

/*****************/
/* Main function */
/*****************/
int main(int argc,char *argv[])
{
 int result,i;
 char params[256];

 if(argc<2)
 {
  fprintf(stderr,"Usage: %s model_source [parameters]\n",argv[0]);
  fprintf(stderr,"Module included: " dbg_xstr(SRC) "\n");
  fprintf(stderr,"Compiled: "__DATE__" "__TIME__"\n");
  exit(1);
 }

 if(XPRMinit())
  return 1;

 /* Register the static module */
 if(XPRMregstatdso(dbg_xstr(SRC),dbg_xinit(SRC)))
  return 2;

 /* Compile,Load and Run the model using the provided parameters */
 if(argc>2)
 {
  strcpy(params,argv[1]);
  for(i=2;i<argc;i++)
  {
   strcat(params,",");
   strcat(params,argv[i]);
  }
 }
 else
  params[0]='\0';

 if(XPRMexecmod("g",argv[1],params,&result,NULL))
  return 3;

 return result;
}


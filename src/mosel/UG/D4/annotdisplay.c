/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file annotdisplay.c
   ```````````````````
   Compiling and loading a model to retrieve some
   annotations from it for display.
   
   (c) 2015 Fair Isaac Corporation
       author: S. Heipcke, Mar. 2015, rev. July 2026
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "xprm.h"

/* Maximum number of annotations */
#define MAXANN 100

int main()
{
 XPRMmodel mod;
 void *ref;
 const char *symb;                    /* A model object name */
 const char *ann[MAXANN*2];           /* List of annotations */
 int i,n;

 if(XPRMinit())                       /* Initialize Mosel */
  return 1;

 if(XPRMcompmod(NULL, "annottest.mos", NULL, "Annotation example"))
  return 2;                           /* Compile the model into a BIM */

 if((mod=XPRMloadmod("annottest.bim",NULL))==NULL)  /* Load the BIM file */
  return 3;

/* Retrieve and display global annotations */
 n=XPRMgetannotations(mod,NULL,NULL,ann,MAXANN*2);
 printf("Global annotations (total: %d):\n", n/2);
 for(i=0;i<n && i<MAXANN;i+=2)
  printf("   %s:%s\n",ann[i],(ann[i+1]!=NULL)?ann[i+1]:"");

/* Retrieve and display all annotations associated with model objects */
 printf("Annotations associated with objects:\n");
 ref=NULL;                             
 while((symb=XPRMgetnextanident(mod,&ref))!=NULL)
 {
  n=XPRMgetannotations(mod,symb,NULL,ann,MAXANN*2);
  printf(" %s->\n",symb);
  for(i=0;i<n && i<MAXANN;i+=2)
   printf("   %s:%s\n",ann[i],(ann[i+1]!=NULL)?ann[i+1]:"");
 }

/* Retrieve and display annotations for model object 'myint' */
 n=XPRMgetannotations(mod,"myint",NULL,ann,MAXANN*2);
 printf("Annotations defined for 'myint' (total: %d):\n", n/2);
 for(i=0;i<n && i<MAXANN;i+=2)
  printf("   %s:%s\n",ann[i],(ann[i+1]!=NULL)?ann[i+1]:"");

 return 0;
}

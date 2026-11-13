/********************************************************
   Mosel Library Examples
   ====================== 

   file mmexrec.c
   ``````````````
   Accessing modeling objects 
   (enumerating an array of records and
    printing the value of each record field).
   
   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2007, rev. July 2026
********************************************************/

#include <stdio.h>
#include "xprm.h"

typedef struct {                      /* For storing record field info: */
 const char *name;                    /*   field name */
 int type,num;                        /*   field type and number */
} s_field; 

int main()
{
 XPRMmodel mod;
 XPRMalltypes value,rvalue,avalue,fvalue;
 XPRMarray arr;
 XPRMset set;
 s_field fields[2];
 int i,result,type,num,utype,first,indices[1];
 void *ref;

 i=XPRMinit();
 if((i!=0)&&(i!=32))                  /* Initialize Mosel */
  return 1;
                                      /* Execute = compile/load/run a model */
 if(XPRMexecmod(NULL, "Models/burglar_rec.mos", NULL, &result, &mod))
  return 2;

 type=XPRMfindident(mod,"I",&value);  /* Get the model object named 'I' */
 if(XPRM_STR(type)!=XPRM_STR_ARR)     /* Check the type: it must be an array */
   return 3;
 type=XPRM_TYP(type);                 /* Expand type of a user type: */
 XPRMgettypeprop(mod, type, XPRM_TPROP_EXP, (XPRMalltypes*)&utype);
 if(XPRM_STR(utype)!=XPRM_STR_REC)    /* the structure must be a record */
  return 4;
 
          /* Retrieve record field info (we know there are 2 fields) */
 for(ref=NULL,i=0;i<2;i++)
  ref=XPRMgetnextfield(mod, ref, type, &(fields[i].name), &(fields[i].type),
                       &(fields[i].num));

          /* Enumerate the array (we know it has a single dimension) */ 
 arr=value.array;   
 XPRMgetarrsets(arr, &set);           /* Get the indexing set */
 
 XPRMgetfirstarrentry(arr, indices);  /* Get the first index tuple */
 do
 {                                       /* Retrieve the array index */
  printf("I(%s):  \t", XPRMgetelsetval(set,indices[0],&rvalue)->string);
  XPRMgetarrval(arr, indices, &avalue);  /* Retrieve array entry (=record) */
                                         /* Contents of 1st record field */
  XPRMgetfieldval(mod, type, avalue.ref, fields[0].num, &fvalue);
  printf("%s=%g ", fields[0].name, fvalue.real);
                                         /* Contents of 2nd record field */
  XPRMgetfieldval(mod, type, avalue.ref, fields[1].num, &fvalue);
  printf("%s=%g\n", fields[1].name, fvalue.real);
 } while(!XPRMgetnextarrentry(arr, indices));   /* Get the next index tuple */ 
 
 return 0;
}

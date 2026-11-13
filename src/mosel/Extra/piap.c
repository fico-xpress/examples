/********************************************************/
/*  XPRD library example                                */
/*  ====================                                */
/*                                                      */
/*  file piap.c                                         */
/*  ```````````                                         */
/* Example of use of XPRD: C version of piapprox.mos    */
/*       Calculations are performed by the Mosel models */
/*                                                      */
/*  (c) 2011 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2011, rev. Nov. 2018        */
/********************************************************/

#include <stdio.h>
#include "xprd.h"

#define N 1000000
#define K 20
#define NEWSUM 2

const char *nodelist[]={"",""};
#define M (sizeof(nodelist)/sizeof(nodelist[0]))

int main()
{
 char msg[256];
 XPRDcontext xprd;
 XPRDmosel moselInst[M];
 XPRDmodel modPar[K],sender;
 int modct,nbfinished,i,j;
 int cls;
 double val;
 double mypi;

 xprd=XPRDinit();
 for(i=0;i<M;i++)
 {
  moselInst[i]=XPRDconnect(xprd,nodelist[i],NULL,NULL,msg,sizeof(msg));
  if(moselInst[i]==NULL)
  {
   printf("Connection failed: %s\n",msg);
   return 2;
  }
 }

 if((i=XPRDcompmod(moselInst[0],"","rmt:piapprox.mos","rmt:p.bim",""))!=0)
 {
  printf("Compilation failed - error %d\n",i);
  return 3;
 }

 for(j=0;j<K;j++)
 {
  sprintf(msg,"NUM=%d,N=%d",j+1,N);
  modPar[j]=XPRDloadmod(moselInst[j%M],"rmt:p.bim");
  if(modPar[j]==NULL)
  {
   printf("Failed to load model %d\n",i);
   return 4;
  }
  XPRDrunmod(modPar[j],msg);
 }

 modct=0;
 mypi=0;
 nbfinished=0;
 while (nbfinished<K)
 {
  XPRDwaitevent(xprd,-1);
  XPRDgetevent(xprd,&sender,&cls,&val);
  if(cls==NEWSUM)
  {
   mypi+=val;
   modct+=1;
  }
  else
   nbfinished++;
 }
 printf("pi approximation: %.19f\n",mypi);
 
 for(i=0;i<M;i++)
  XPRDdisconnect(moselInst[i]);
 
 XPRDfinish(xprd);
 return 0;
}


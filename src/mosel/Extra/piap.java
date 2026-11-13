/********************************************************/
/*  XPRD library example                                */
/*  ====================                                */
/*                                                      */
/*  file piap.java                                      */
/*  ``````````````                                      */
/* Example of use of XPRD: Java version of piapprox.mos */
/*       Calculations are performed by the Mosel models */
/*                                                      */
/*  (c) 2011 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2011, rev. Nov. 2018        */
/********************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;

public class piap
{
 static final int N=1000000;
 static final int K=20;
 static final int NEWSUM=2;
 static final String nodelist[]={"",""};

 public static void main(String[] args) throws Exception
 {
  int M=nodelist.length;
  XPRD xprd=new XPRD();
  XPRDMosel moselInst[]=new XPRDMosel[M];
  XPRDModel modPar[]=new XPRDModel[K];
  XPRDEvent ev;
  int modct,nbfinished;
  double mypi;

  for(int i=0;i<M;i++)
   moselInst[i]=xprd.connect(nodelist[i]);

  try
  {
   moselInst[0].compile("","rmt:piapprox.mos","rmt:p.bim");
  }
  catch(Exception e)
  {
   System.out.println("Compilation failed:"+e);
   System.exit(1);
  }

  for(int j=0;j<K;j++)
  {
   modPar[j]=moselInst[j%M].loadModel("rmt:p.bim");
   modPar[j].setExecParam("N",N);
   modPar[j].setExecParam("NUM",j+1);
   modPar[j].run();
  }

  modct=0;
  mypi=0;
  nbfinished=0;
  while (nbfinished<K)
  {
   xprd.waitForEvent();
   ev=xprd.getNextEvent();
   if(ev.eventClass==NEWSUM)
   {
    mypi+=ev.value;
    modct+=1;
   }
   else
    nbfinished++;
  }
  System.out.println("pi approximation: "+mypi);
  
  for(int i=0;i<M;i++)
   moselInst[i].disconnect();
 }
}

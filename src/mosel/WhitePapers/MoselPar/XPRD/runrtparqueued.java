/*******************************************************
   Mosel Example Problems 
   ======================

   file runrtparqueued.java
   ````````````````````````
   Running several instances of a model from another
   Mosel model.
   - Queuing submodels for parallel execution in a
     distributed architecture (one or several models per node) -

   Before running this model, you need to set up the list
   NodeList with machine names/addresses of your local network.
   All nodes that are used need to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on these machines.

   The maximum number of models per node in array MaxMod needs
   to be adapted to the number of executions licensed on 
   the corresponding nodes.
   
   All files are local to the root node, no write access is
   required at remote nodes.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke & Y. Colombani, Apr. 2012
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.util.*;
import java.io.*;

public class runrtparqueued
{
 static final int J=10;             // Number of jobs to run
 static final int NUMPAR=2;         // Number of parallel model executions
                                    // (preferrably <= no. of processors)
 static int[] jobid;
 static int[] modid;
 static String[] modNode;

 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
                            // Use the name or IP address of a machine in
                            // your local network, or "" for current node
  String[] NodeList={"localhost","localhost"};
  final int nbNodes=(NodeList.length<J?NodeList.length:J);
  XPRDMosel[] mosInst=new XPRDMosel[nbNodes];
  int[] MaxMod=new int[nbNodes];
  XPRDModel[] modPar=new XPRDModel[nbNodes*NUMPAR];
  int nct;
  List<Integer> JobList=new ArrayList<Integer>();
  List<Integer> JobsRun=new ArrayList<Integer>();
  int JobSize;
  XPRDEvent event;
  int lastId=0;

  //**** Setting up remote Mosel instances ****
  for(int n=0;n<nbNodes;n++)
  {
   mosInst[n]=xprd.connect(NodeList[n]);
   MaxMod[n]= NUMPAR;
         // Adapt this setting to number of processors and licences per node
  }

                                    // Compile the model file on first node
  mosInst[0].compile("", "rmt:rtparams.mos", "rmt:rtparams.bim");

  //**** Loading model instances ****
  nct=0;
  for(int n=0;(n<nbNodes) && (nct<J);n++)
   for(int m=0;(m<MaxMod[n]) && (nct<J);m++)
   {                               // Load the bim file
    modPar[nct]=mosInst[n].loadModel("rmt:rtparams.bim");
    if(modPar[nct].getNumber()>lastId) lastId=modPar[nct].getNumber();
    nct++;
   }

  jobid=new int[lastId+1];
  modid=new int[lastId+1];
  modNode=new String[lastId+1];
  for(int j=0;j<nct;j++)
  {
   int i=modPar[j].getNumber();
   modid[i]=j;                    // Store the model ID
   modNode[i]=modPar[j].getMosel().getSystemInformation(XPRDMosel.SYS_NODE);
  }

  for(int i=0;i<J;i++)            // Define the list of jobs (instances)
   JobList.add(Integer.valueOf(i));
  JobSize=JobList.size();         // Store the number of jobs
  JobsRun.clear();                // List of terminated jobs is empty

  //**** Start initial lot of model runs ****
  for(int j=0;j<nct;j++)
   startNextJob(JobList,modPar[j]);

  //**** Run all remaining jobs ****
  while(JobsRun.size()<JobSize)
  {
   xprd.waitForEvent();           // Wait for model termination
   event=xprd.getNextEvent();     // We are only interested in "end" events
   if(event.eventClass==XPRDEvent.EVENT_END)
   {                              // Keep track of job termination
    JobsRun.add(Integer.valueOf(jobid[event.sender.getNumber()]));
    System.out.println("End of job "+ jobid[event.sender.getNumber()] +
                              " (model "+ modid[event.sender.getNumber()]+ ")");
    if(!JobList.isEmpty())        // Start a new run if queue not empty
     startNextJob(JobList,event.sender);
   }
  }

  for(int n=0;n<nbNodes;n++)
   mosInst[n].disconnect();                 // Terminate remote instances

  new File("rtparams.bim").delete();        // Cleaning up
 }

//******** Start the next job in a queue ********
 static void startNextJob(List<Integer> jobList,XPRDModel model) throws Exception
 {
  Integer job;
  int i;

  job=jobList.remove(0);      // Retrieve and remove first entry from job list
  i=job.intValue();
  System.out.println("Start job "+ job + " (model " + modid[model.getNumber()]+
                          ") on "+ modNode[model.getNumber()] );
  model.execParams = "PARAM1=" + i + ",PARAM2=" + (0.1*i) + 
                          ",PARAM3='a string " + i + "',PARAM4=" + (i%2==0);
  model.run();
  jobid[model.getNumber()]=i;
 }

}

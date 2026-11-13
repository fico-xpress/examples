/*******************************************************
   Mosel Example Problems 
   ======================

   file runrtpardistr.java
   ```````````````````````
   Running several instances of a model remotely from
   a host program.
   - Parallel submodels in distributed architecture -

   Before running this model, you need to set up the 
   NODENAMES with a machine name/address of your local network.
   All nodes that are used need to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on this machine.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Feb 2012, rev. Jan. 2013
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;

public class runrtpardistr
{
 static final int A=10;
 static final int B=5;

 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel[] mosInst=new XPRDMosel[B];
  XPRDModel[] modRP=new XPRDModel[A];
  String[] NODENAMES=new String[B];
  int i,j;

       // Use the name or IP address of a machine in
       // your local network, or "" for current node
  for(j=0;j<B;j++) NODENAMES[j] = "localhost";
 // String[] NODENAMES={"localhost","","","rcmd:mosel -r","rcmd:mosel -r"};

                               // Open connection to remote nodes
  for(j=0;j<B;j++)
    mosInst[j]=xprd.connect(NODENAMES[j]);

  for(j=0;j<B;j++)
    System.out.println("Submodel node: " + 
		       mosInst[j].getSystemInformation(XPRDMosel.SYS_NODE) + " on " +
		       mosInst[j].getSystemInformation(XPRDMosel.SYS_NAME));

                               // Compile the model file on one instance
  mosInst[0].compile("", "rmt:rtparams3.mos", "rmt:rp3.bim");
 
  for(i=0;i<A;i++)
  {	                       // Load the bim file into remote instances
   modRP[i]=mosInst[i%B].loadModel("rmt:rp3.bim"); 
                               // Run-time parameters
    modRP[i].execParams = "PARAM1=" + i + ",PARAM2=" + (0.1*i) + 
                          ",PARAM3='a string " + i + "',PARAM4=" + (i%2==0);
    modRP[i].run();            // Run the model
  }

  for(i=0;i<A;i++)
  {
    xprd.waitForEvent();       // Wait for model termination
    xprd.dropNextEvent();      // Ignore termination event message
  }

  for(j=0;j<B;j++)
    mosInst[j].disconnect();   // Disconnect remote instance

  new File("rp3.bim").delete();           // Cleaning up
 }
} 

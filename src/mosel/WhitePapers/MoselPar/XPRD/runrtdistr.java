/*******************************************************
   Mosel Example Problems 
   ======================

   file runrtdistr.java
   ````````````````````
   Running a model on a remote machine,
   passing runtime parameters to the submodel.

   Before running this model, you need to set up the 
   NODENAME with a machine name/address of your local network.
   The node that is used needs to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on this machine.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Jan 2012
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;

public class runrtdistr
{
 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel mosInst=null;
  XPRDModel modRP=null;

       // Use the name or IP address of a machine in
       // your local network, or "" for current node
  String NODENAME = "";
                               // Open connection to a remote node
  mosInst=xprd.connect(NODENAME);
                               // Compile the model file
  mosInst.compile("", "rmt:rtparams.mos", "tmp:rp.bim"); 
                               // Load the bim file into the remote instance
  modRP=mosInst.loadModel("tmp:rp.bim"); 
                               // Run-time parameters
  modRP.execParams = "PARAM1=" + 2 + ",PARAM2=" + 3.4 + 
                     ",PARAM3='a string'" + ",PARAM4=true";
  modRP.run();                 // Run the model
  xprd.waitForEvent();         // Wait for model termination
  xprd.dropNextEvent();        // Ignore termination event message

  System.out.println("`rtparams' returned: " + modRP.getResult());

  mosInst.disconnect();        // Disconnect remote instance
 }
} 

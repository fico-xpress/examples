/******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file runprimedistr.java 
   ```````````````````````
   Distributed computing: 
   Running a model on a remote Mosel instance.
  
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Feb 2012
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;

public class runprimedistr
{
 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel moselInst;
  XPRDModel modPrime;
  XPRDEvent event;

  moselInst=xprd.connect("");    // Open connection to remote nodes
                                 // "" means the node running this program
    
                                 // Compile the model file on remote instance
  moselInst.compile("", "rmt:prime.mos", "rmt:prime.bim");
  
	                         // Load the bim file into remote instance
  modPrime=moselInst.loadModel("rmt:prime.bim"); 

  modPrime.execParams = "LIMIT=50000";
  modPrime.run();                // Start execution and
  xprd.waitForEvent(2);          // wait 2 seconds for an event

  if (xprd.isQueueEmpty())       // No event has been sent...
  {
   System.out.println("Model too slow: stopping it!");
   modPrime.stop();              // ... stop the model, then wait
   xprd.waitForEvent();
  }
                                 // An event is available: model finished
  event=xprd.getNextEvent();
  System.out.println("Event value: " + event.value +
                     " sent by model " + event.sender.getNumber());
  System.out.println("Exit status: " + modPrime.getExecStatus());
  System.out.println("Exit code  : " + modPrime.getResult());

  moselInst.unloadModel(modPrime);  // Unload the submodel
  moselInst.disconnect();           // Terminate the connection

  new File("prime.bim").delete();   // Clean up temporary files
 }
} 


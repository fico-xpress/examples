/******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file runprimeiodistr.java 
   `````````````````````````
   Distributed computing: 
   Running a model on a remote Mosel instance
   and retrieving output data.
   
   - Data saved on the remote machine running the Mosel model -
     (Submodel writing to "bin:shmem:" or "bin:tmp:")
  
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke & Y. Colombani, Jan 2013
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.util.*;
import java.io.*;

public class runprimeiodistr
{
 static final int STOPMOD = 2;    // Identifier for "Stop submodel" user event
 static final int MODREADY = 3;   // Identifier for "Submodel ready" user event

// **** Decode the binary stream and display its contents ****
 static void showSolution(InputStream inbuf) throws Exception
 {
  BinDrvReader bdrv=new BinDrvReader(inbuf);  // Initialize binreader
  String label;
  ArrayList<Integer> setP=new ArrayList<Integer>();

  while(bdrv.nextToken()>=0)
  {
   bdrv.getControl();          // 'label'  (marker)
   label=bdrv.getString();        // Read a string
   if(label.equals("NumP"))
   {                              // Read an integer
    System.out.println("(" + bdrv.getInt() + " prime numbers.)");
   }
   else
   if(label.equals("SPrime"))
   {
    bdrv.getControl();         // [  (start marker)
    while(bdrv.nextToken()==BinDrvReader.TYP_INT) // or ] at end of list
    {                             // Read integers
     setP.add(Integer.valueOf(bdrv.getInt()));
    }
    bdrv.getControl();         // ]  (end marker)
   }
   else
   {
    System.out.println("Unexpected label: "+label);
    System.exit(0);
   }
  }

 // Display the contents of the set 'SPrime'
  Iterator<Integer> iprime=setP.iterator();
  System.out.print("Prime numbers={");
  while(iprime.hasNext())
  {
   Integer p=iprime.next();
   System.out.print(" "+p);
  }
  System.out.println(" }");
 }

/*******************************************************/
 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();          // Initialize XPRD
  XPRDMosel moselInst;
  XPRDModel modPrime;
  XPRDEvent event;
  InputStream resdata;

  moselInst=xprd.connect("");    // Open connection to remote nodes
                                 // "" means the node running this program
    
                                 // Compile the model file on remote instance
  moselInst.compile("", "rmt:primeio.mos", "tmp:primeio.bim");
  
	                         // Load the bim file into remote instance
  modPrime=moselInst.loadModel("tmp:primeio.bim"); 

                                 // Disable submodel output
  modPrime.setDefaultStream(modPrime.F_OUTPUT, "null:");

  modPrime.execParams = "LIMIT=50000,OUTPUTFILE=bin:shmem:resdata";
  modPrime.run();                     // Start execution and
  xprd.waitForEvent();                // ...wait for an event
  event=xprd.getNextEvent();          // Retrieve the event
  if (event.eventClass != MODREADY)   // Check the event class 
  {
    System.out.println("Problem with submodel run");
    System.exit(1);
  } 

  xprd.waitForEvent(2);               // Let the submodel run for 2 seconds

  if (xprd.isQueueEmpty())            // No event has been sent...
  {
   System.out.println("Model too slow: stopping it!");
   modPrime.sendEvent(STOPMOD, 0);    // ... stop the model, then 
   xprd.waitForEvent();               // wait for its termination
  }

  // Open the output file, retrieve and display the solution data
  resdata=moselInst.openForReading("shmem:resdata", moselInst.F_BINARY);
  showSolution(resdata);
  resdata.close();

  moselInst.unloadModel(modPrime);    // Unload the submodel
  moselInst.disconnect();             // Terminate the connection
 }
} 


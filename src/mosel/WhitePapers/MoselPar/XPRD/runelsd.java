/*******************************************************
   Mosel Example Problems 
   ======================

   file runelsd.java
   `````````````````
   Run several instances of the model elsd.mos in
   parallel and coordinate the solution update.

   Before running this program, you need to set up the array
   NodeNames with machine names/addresses of your local network.
   All nodes that are used need to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on these machines.
   
   All files are local to the root node, no write access is
   required at remote nodes.

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Jan 2012, rev. Jul. 2014
*******************************************************/

// javac -cp "xprd.jar;bindrv.jar;." runelsd.java

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;

public class runelsd
{
 static final String DATAFILE = "els.dat";
 static final int T = 15;                  // Time periods
 static final int P = 4;                   // No. of products
 static final int I = 2;                   // No. of (remote) Mosel instances
 static final int M = 6;                   // No. of Mosel models

 static final int NEWSOL = 2;              // Identifier for "sol. found" event
 static final int M1 = 1;
 static final int M2 = 3;

/* Setting up remote connections:
 * Use machine names within your local network, IP addresses, or 
 * empty string for the current node running this model */
 static final String NodeNames[] = {"", "localhost"};

 static double[][] solprod;               // Sol. values for var.s produce 
 static double[][] solsetup;              // Sol. values for var.s setup
 static double[][] DEMAND = new double[P][T];    // Demand per period


/***************** Reading result data ******************/
/**** Read a table of the form 'double ar[][]' ****/
/*
 static void assignTableVal(BinDrvReader bdrv, double[][] ar) throws IOException
 {
   int i,j,ctrl;

   if (bdrv.getControl()==BinDrvReader.CTRL_OPENLST)
   {
     while(bdrv.nextToken()>=0) {
     ctrl=bdrv.getControl();
     if(ctrl==BinDrvReader.CTRL_CLOSELST) break;
     else
       if(ctrl==BinDrvReader.CTRL_OPENNDX)
       {
         i=bdrv.getInt();
         j=bdrv.getInt();
// System.out.println(i + " " + j);
         if(bdrv.getControl()==BinDrvReader.CTRL_CLOSENDX)
         {
           ar[i-1][j-1]=bdrv.getReal();
// System.out.println(ar[i-1][j-1]);
         }
         else
           throw new IOException("Wrong file format. ')' expected.");
       }
       else
         throw new IOException("Wrong file format. '(' expected.");
     }
   }
   else
     throw new IOException("Wrong file format. '[' expected.");
 }
*/

/**** Fixed size version for reading 'double ar[][]' ****/
 static void assignFixedTableVal(BinDrvReader bdrv, double[][] ar) throws IOException
 {
   int i,j,ctrl,i2,j2;

   if (bdrv.getControl()==BinDrvReader.CTRL_OPENLST)
   {
     while(bdrv.nextToken()>=0) {
     ctrl=bdrv.getControl();
     if(ctrl==BinDrvReader.CTRL_CLOSELST) break;
     else
       if(ctrl==BinDrvReader.CTRL_OPENNDX)
       {
         i=bdrv.getInt();
         j=bdrv.getInt();
         if(bdrv.getControl()==BinDrvReader.CTRL_CLOSENDX)
         {
           for(j2=j;j2<=T;j2++) 
           {
             ar[i-1][j2-1]=bdrv.getReal();
// System.out.println(i + " " + j2 + " " + ar[i-1][j2-1]);
           }
         }
         else
           throw new IOException("Wrong file format. ')' expected.");
       }
       else
         throw new IOException("Wrong file format. '(' expected.");
     }
   }
   else
     throw new IOException("Wrong file format. '[' expected.");
 }
/**** Identify the label to read ****/
 static void readSol(String filename) throws IOException
 {
   BinDrvReader bdrv;
   FileInputStream f;
   String s;

   f=new FileInputStream(filename);
   bdrv=new BinDrvReader(f);              // Use Mosel bin reader

   while(bdrv.nextToken()>=0) {
     if(bdrv.getControl()==BinDrvReader.CTRL_LABEL)
     {
       s=bdrv.getString();
       System.out.println("Reading "+s);
       if (s.equals("solprod"))  assignFixedTableVal(bdrv, solprod);
       else if (s.equals("solsetup"))  assignFixedTableVal(bdrv, solsetup);
       else if (s.equals("DEMAND"))  assignFixedTableVal(bdrv, DEMAND);
       // Unknow labels are simply ignored
     }
     else
       throw new IOException("Wrong file format");
   }

   f.close();
 }

/****** Run a submodel to read demand data for pretty solution printing ******/
 static void readDem(XPRD xprd,XPRDMosel moselInst) throws IOException
 {
   XPRDModel modDem = null;                // Model

   try {                                   // Compile the model
     moselInst.compile("", "rmt:readelsdem.mos", "rmt:readelsdem.bim");
   }
   catch(Exception e) {
     System.out.println("Compilation failed: " + e);
     System.exit(2);
   }
   try {                                   // Load model
    modDem=moselInst.loadModel("rmt:readelsdem.bim");
   }
   catch(IOException e) {
     System.out.println("Loading failed: " + e);
     System.exit(3);
   }  
   modDem.execParams = "DATAFILE="+DATAFILE + ",T="+T + ",P="+P; 
   try {
     modDem.run();                         // Run the model
   }
   catch(Exception e)
   {
     System.out.println("Failed to run model: "+e);
     System.exit(4);
   }
   xprd.waitForEvent();                    // Wait for termination event

   readSol("Demand");                      // Read the demand data

   new File("readelsdem.bim").delete();    // Cleaning up temporary files
   new File("Demand").delete();
 }


/***************** Main ******************/

 public static void main(String[] args)
 {
  XPRD xprd = new XPRD();                  // Initialize XPRD
  XPRDMosel[] moselInst = new XPRDMosel[I];  // Mosel instances
  XPRDModel[] modELS = new XPRDModel[M];   // Models
  XPRDEvent Msg;                           // Messages sent by models 
  solprod = new double[P][T];
  solsetup = new double[P][T];
  double objval;
  int algsol,algopt,senderid=0;

  try {
    for(int i=0;i<I;i++)                   // Establish remote connections
      moselInst[i]=xprd.connect(NodeNames[i]);
  }
  catch(IOException e) {
    System.out.println("Connection failed: " + e);
    System.exit(1);
  }

  try {                                    // Compile the model
    moselInst[0].compile("", "rmt:elsd.mos", "rmt:elsd.bim");
  }
  catch(Exception e) {
    System.out.println("Compilation failed: " + e);
    System.exit(2);
  }
      
  try {                                    // Load models
    modELS[M1-1]=moselInst[0].loadModel("rmt:elsd.bim");
    modELS[M2-1]=moselInst[1].loadModel("rmt:elsd.bim");
  }
  catch(IOException e) {
    System.out.println("Loading failed: " + e);
    System.exit(3);
  }  

                                            // Run-time parameters
  modELS[M1-1].execParams = "ALG="+M1 + ",DATAFILE="+DATAFILE + 
	                    ",T="+T + ",P="+P;
  modELS[M2-1].execParams = "ALG="+M2 + ",DATAFILE="+DATAFILE + 
	                    ",T="+T + ",P="+P;
  try {
    modELS[M1-1].run();                     // Run the models
    modELS[M2-1].run();
  }
  catch(Exception e)
  {
   System.out.println("Failed to run model: "+e);
   System.exit(4);
  }

  objval = 1.0e+20;
  algsol = -1; algopt = -1;

  do
  {
    xprd.waitForEvent();           // Wait for the next event
    Msg = xprd.getNextEvent();     // Get the event
    if (Msg.eventClass==NEWSOL)    // Get the event class
    {                              // Identify the sending model
      senderid = Msg.sender.getNumber()==modELS[M1-1].getNumber()?M1:M2;
      if (Msg.value<objval)        // Value of the event (= obj. value)
      {                            
        algsol = senderid;
	objval = Msg.value;
        System.out.println("Improved solution " + objval + 
			   " found by model " + algsol);
	for(int m=0;m<M;m++)
          if (m!=algsol-1 && (m==M1-1 || m==M2-1))
            modELS[m].sendEvent(NEWSOL, objval);
      }
      else
        System.out.println("Solution " + Msg.value + 
			   " found by model " + senderid );
    }

  } while (Msg.eventClass!=XPRDEvent.EVENT_END);   // No model has finished
  
  algopt = senderid;              // Store ID of terminated model
  for(int m=0;m<M;m++)
    if (m==M1-1 || m==M2-1)
      modELS[m].stop();           // Stop all running models

  while((modELS[M1-1].getExecStatus()==XPRDModel.RT_RUNNING)||
        (modELS[M2-1].getExecStatus()==XPRDModel.RT_RUNNING)||
        !xprd.isQueueEmpty() )
  {
    xprd.waitForEvent();          // Wait for the next event
    xprd.dropNextEvent();         // Ignore termination event
  }

  if (algsol==-1)
  {
    System.out.println("No solution available");
    System.exit(1);
  }
  else                     
  {                  // Retrieve the best solution from shared memory
    try {
      readSol("sol_"+algsol); 
    } catch(IOException e) {
      System.out.println(e + "Could not read file " + algsol);
      System.exit(5);
    }
                     // Read the demand data by running another Mosel model
    try {
      readDem(xprd,moselInst[0]); 
    } catch(IOException e) {
      System.out.println(e+"Could not read demand data");
      System.exit(6);
    }
  
  // Solution printing
    System.out.println("Best solution found by model " + algsol); 
    System.out.println("Optimality proven by model " + algopt); 
    System.out.println("Objective value: " + objval); 
    System.out.print("Period  setup");
    for(int p=0;p<P;p++) System.out.print("      "+(p+1));
    for(int t=0;t<T;t++) 
    {	  
      double sp=0;
      for(int p=0;p<P;p++) sp+=solsetup[p][t];
      System.out.print("\n " + String.format("%2d",(t+1)) + 
                       String.format("%8.0f",sp) + "    ");
      for(int p=0;p<P;p++)

        System.out.print(String.format("%3.0f",solprod[p][t]) +
	                 " (" + String.format("%1.0f",DEMAND[p][t]) + ")");
    }
    System.out.println();
  }


// Cleaning up temporary files
  for(int m=0;m<M;m++) new File("sol_"+m).delete();
  new File("elsd.bim").delete();

  for(int i=0;i<I;i++)                   // Terminate remote connections
    moselInst[i].disconnect(); 
 }
}

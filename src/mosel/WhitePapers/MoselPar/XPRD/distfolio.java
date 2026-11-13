/*******************************************************
   Mosel Example Problems 
   ======================

   file distfolio.java
   ```````````````````
   Running a model on a remote machine,
   passing runtime parameters to the submodel.

   Before running this model, you need to set up the 
   NODENAME with a machine name/address of your local network.
   The node that is used needs to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on this machine.

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, Oct 2012
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;
import java.util.*;

public class distfolio
{
                    // Class to receive solution values of decision variables
 public static class MySolArray
 {
  public String ind;                // index name
  public double val;                // solution value
  MySolArray(String i,double v)
  {
   ind=i;
   val=v;
  }
 }

/***************** Reading result data ******************/

 static void readSol(String filename) throws IOException
 {
   BinDrvReader bdrv;
   FileInputStream f;
   String label;
   String index;
   ArrayList<MySolArray> solfrac=new ArrayList<MySolArray>();
   ArrayList<MySolArray> solbuy=new ArrayList<MySolArray>();

   f=new FileInputStream(filename);
   bdrv=new BinDrvReader(f);              // Use Mosel bin reader

  while(bdrv.nextToken()>=0)
  {
   bdrv.getControl();     // 'label'
   label=bdrv.getString();
   if(label.equals("RETSOL"))
    System.out.println("Total return: " + bdrv.getReal());
   else
   if(label.equals("NUMSHARES"))
    System.out.println("Number of shares: " + bdrv.getInt());
   else
   if(label.equals("SOLCOUNT"))
    System.out.println("Solution number: " + bdrv.getInt());
   else
   if(label.equals("SOLSTATUS"))
    System.out.println("Solution status: " + bdrv.getInt());
   else
   if(label.equals("BUY"))
   {
    bdrv.getControl();    // [
    while(bdrv.getControl()==BinDrvReader.CTRL_OPENNDX) // ( or ] at end of list
    {
     index=bdrv.getString();
     bdrv.getControl();   // )
     solbuy.add(new MySolArray(index,bdrv.getReal()));
    }
   }
   else
   if(label.equals("FRAC"))
   {
    bdrv.getControl();    // [
    while(bdrv.getControl()==BinDrvReader.CTRL_OPENNDX) // ( or ] at end of list
    {
     index=bdrv.getString();
     bdrv.getControl();   // )
     solfrac.add(new MySolArray(index,bdrv.getReal()));
    }
   }
   else
   {
    System.out.println("Unexpected label: "+label);
    System.exit(0);
   }
  }

  Iterator<MySolArray> ibuy=solbuy.iterator();
  Iterator<MySolArray> ifrac=solfrac.iterator();

  while(ibuy.hasNext() && ifrac.hasNext())
  {
   MySolArray buy=ibuy.next();
   MySolArray frac=ifrac.next();
   System.out.println(frac.ind + ": " + frac.val*100 + "% (" + 
       buy.val + ")");
  }
 }

/***************** Main ******************/

 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel mosel=null;
  XPRDModel mod=null;
                    // Model parameter settings
  double maxrisk = 1.0/3;
  double minreg = 0.2;
  double maxreg = 0.5;
  double maxsec = 0.25;
  double maxval = 0.2;
  double minval = 0.1;
  int maxnum = 9;

                       // Use the name or IP address of a machine in
                       // your local network, or "" for current node
  String NODENAME = "";
                       // Open connection to a remote node
  try{
   mosel = xprd.connect(NODENAME);    // Create a new Mosel instance
  }catch(IOException e){
                System.out.println("IO error" + e.getMessage());
                throw new java.lang.Exception("Failed to connect");
  }

  try{
   mosel.compile("","rmt:foliomemio.mos","rmt:foliomemio.bim");
                                      // Compile the model (only required
                                      // during development phase, deployed
				      // application would only use BIM)
  }catch(XPRDCompileException e){
   System.out.println(e.getMessage());
   System.exit(1);
  }
                       // Load the bim file into the remote instance 
  mod = mosel.loadModel("rmt:foliomemio.bim");
  new File("foliomemio.bim").delete();         // Cleaning up

                       // Pass model parameters through execution parameters
  mod.setExecParam("MAXRISK",maxrisk);
  mod.setExecParam("MINREG",minreg);
  mod.setExecParam("MAXREG",maxreg);
  mod.setExecParam("MAXSEC",maxsec);
  mod.setExecParam("MAXVAL",maxval);
  mod.setExecParam("MINVAL",minval);
  mod.setExecParam("MAXNUM",maxnum);
  mod.setExecParam("DATAFILE","rmt:folio250.dat");
  mod.setExecParam("OUTPUTFILE","bin:rmt:solfile.dat");
  System.out.println(mod.execParams);
  mod.run();                          // Run the model
  xprd.waitForEvent();                // Wait for model termination
  xprd.dropNextEvent();               // Ignore termination event message

  if(mod.getExecStatus() != XPRDModel.RT_OK){
   throw new java.lang.Exception("Error during model execution ");
  }
  if(mod.getResult() != 0){
   throw new java.lang.Exception("Problem not optimal");
  }                                  

  try {
   readSol("solfile.dat");      // Read+display result data
  } catch(IOException e) {
   System.out.println("Could not read result file");
   System.exit(1);
  }
  new File("solfile.dat").delete();

  mosel.disconnect();                 // Disconnect remote instance
 }
} 

/*******************************************************
   Mosel Example Problems 
   ======================

   file distfoliopar.java
   ``````````````````````
   Running several instances of a Mosel model
   in parallel using several (remote) Mosel instances.

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

public class distfoliopar
{
 static final int numpar = 2;      // Number of model instances to run

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

  System.out.println("Solution file: " + filename);

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
  System.out.println();
 }

/***************** Main ******************/

 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel[] mosel=new XPRDMosel[numpar];
  XPRDModel[] mod=new XPRDModel[numpar];
  String[] NODENAMES=new String[numpar];
  int j;

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
  for(j=0;j<numpar;j++) NODENAMES[j] = "localhost";

                       // Open connection to remote nodes
  for(j=0;j<numpar;j++)
   try{
    mosel[j] = xprd.connect(NODENAMES[j]);    // Create a new Mosel instance
   }catch(IOException e){
                System.out.println("IO error" + e.getMessage());
                throw new java.lang.Exception("Failed to connect");
   }

  for(j=0;j<numpar;j++)
    System.out.println("Submodel node: " + 
      mosel[j].getSystemInformation(XPRDMosel.SYS_NODE) + " on " +
      mosel[j].getSystemInformation(XPRDMosel.SYS_NAME));

                       // Compile the model file on one instance
  try{
   mosel[0].compile("","rmt:foliomemio.mos","rmt:foliomemio.bim");
  }catch(XPRDCompileException e){
   System.out.println(e.getMessage());
   System.exit(1);
  }
                       // Load the bim file into remote instances 
  for(j=0;j<numpar;j++)
   mod[j] = mosel[j].loadModel("rmt:foliomemio.bim");
  new File("foliomemio.bim").delete();         // Cleaning up

                       // Pass model parameters through execution parameters
  for(j=0;j<numpar;j++)
  {
   mod[j].setExecParam("MAXRISK",maxrisk);
   mod[j].setExecParam("MINREG",minreg);
   mod[j].setExecParam("MAXREG",maxreg);
   mod[j].setExecParam("MAXSEC",maxsec);
   mod[j].setExecParam("MAXVAL",maxval);
   mod[j].setExecParam("MINVAL",minval);
   mod[j].setExecParam("MAXNUM",maxnum-j);
   mod[j].setExecParam("DATAFILE","rmt:folio250.dat");
   mod[j].setExecParam("OUTPUTFILE","bin:rmt:solfile"+j+".dat");
   mod[j].run();                       // Run the model
  }

  for(j=0;j<numpar;j++)
  {  
   xprd.waitForEvent();                // Wait for model termination
   xprd.dropNextEvent();               // Ignore termination event message
  }

     
  for(j=0;j<numpar;j++)
  {
   try {
    readSol("solfile"+j+".dat");      // Read+display result data
   } catch(IOException e) {
    System.out.println("Could not read file "+j);
    System.exit(1);
   }
   new File("solfile"+j+".dat").delete();
  }

  for(j=0;j<numpar;j++)
   mosel[j].disconnect();              // Disconnect remote instance
 }
} 

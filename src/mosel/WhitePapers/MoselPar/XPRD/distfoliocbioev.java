/*******************************************************
   Mosel Example Problems
   ====================== 

   file distfoliocbioev.java
   `````````````````````````
   Running a Mosel model from a Java application
   with data exchange between model and host application.
   (use Mosel running remotely controlled via XPRD)
   -- Coordination via Mosel events exchanged 
      between Mosel model and Java program    --

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2011 Fair Isaac Corporation
       author: Y.Colombani, July 2011
********************************************************/

import java.io.*;
import java.util.*;
import com.dashoptimization.*;

public class distfoliocbioev
{
 static byte solbuf[]=null;
 static final int NEWSOL=2;
 static final int SOLREAD=3;

/**
  * An extension of 'ByteArrayOutputStream' for saving the array on closing.
  */
 static class myByteArrayOutputStream extends ByteArrayOutputStream
 {
  public void close()
  {
   solbuf=toByteArray();
  }
 }

/**
  * This file manager will directly access the file 'solfile'
  * to a local ArrayStream object.
  * Warning: concurrency not handled here!!!
  */
 static class FileManager implements XPRDFileManager
 {
  public OutputStream openForWriting(String fname,int mode) throws IOException
  {
   if(fname.equals("solfile"))
   {
    return new myByteArrayOutputStream();
   }
   else
    return null;    // Name not found: use default behaviour (open local file)
  }
  public InputStream openForReading(String fname,int mode) throws IOException
  {
   return null;     // Use default behaviour (open local file)
  }
 }
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

// Decode the binary file and display its content
 static void showSolution() throws Exception
 {
  ByteArrayInputStream inbuf=new ByteArrayInputStream(solbuf);
  BinDrvReader bdrv=new BinDrvReader(inbuf);
  String label;
  String index;
  ArrayList<MySolArray> solfrac=new ArrayList<MySolArray>();
  ArrayList<MySolArray> solbuy=new ArrayList<MySolArray>();

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

 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDFileManager fmgr=new FileManager();
  XPRDMosel mosel;
  XPRDModel mod;
                    // Model parameter settings
  double maxrisk = 1.0/3;
  double minreg = 0.2;
  double maxreg = 0.5;
  double maxsec = 0.25;
  double maxval = 0.2;
  double minval = 0.1;
  int maxnum = 15;
  int val;

  try{
   mosel = xprd.connect("",fmgr);     // Create a new Mosel instance
  }catch(IOException e){
                System.out.println("IO error" + e.getMessage());
                throw new java.lang.Exception("Failed to connect");
  }

  try{
   mosel.compile("","rmt:foliocbioev.mos","tmp:foliocbioev.bim");
                                      // Compile the model (only required
                                      // during development phase, deployed
				      // application would only use BIM)
  }catch(XPRDCompileException e){
   System.out.println(e.getMessage());
   System.exit(1);
  }
  
  mod = mosel.loadModel("tmp:foliocbioev.bim");  // Load the model

                    // Pass model parameters through execution parameters
  mod.setExecParam("MAXRISK",maxrisk);
  mod.setExecParam("MINREG",minreg);
  mod.setExecParam("MAXREG",maxreg);
  mod.setExecParam("MAXSEC",maxsec);
  mod.setExecParam("MAXVAL",maxval);
  mod.setExecParam("MINVAL",minval);
  mod.setExecParam("MAXNUM",maxnum);
  mod.setExecParam("DATAFILE","rmt:folio250.dat");
  mod.setExecParam("OUTPUTFILE","bin:rmt:solfile");
  mod.run();                          // Run the model
  do
  {
   xprd.waitForEvent();               // Wait for event sent by model
   val=xprd.getNextEvent().eventClass;
   if (val==NEWSOL)
   {
     showSolution();                  // Decode & display solution data
     mod.sendEvent(SOLREAD,0);
   }
  } while (val==NEWSOL);
  

  if(mod.getExecStatus() != XPRDModel.RT_OK){
   throw new java.lang.Exception("Error during model execution ");
  }
  if(mod.getResult() != 0){
   throw new java.lang.Exception("Problem not optimal");
  }                                  // Stop if no solution available

  mosel.disconnect();                // Disconnect instance

 }
}


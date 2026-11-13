/*******************************************************
   Mosel Example Problems
   ====================== 

   file runfoliocbio.java
   ``````````````````````
   Running a Mosel model from a Java application
   with data exchange between model and host application
   during the optimization run.
   (Passing data via callback)

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2011 Fair Isaac Corporation
       author: S. Heipcke, July 2011
********************************************************/

import java.io.*;
import com.dashoptimization.*;

public class runfoliocbio
{
                    // Class to receive solution values of decision variables
 public static class MySolArray
 {
  public String ind;                // index name
  public double val;                // solution value
 }

 static MySolArray[] solfrac;
 static MySolArray[] solbuy;
 static boolean ifprint;

                /*************************************************/
                /* A class to initialize model data via callback */
                /*************************************************/
 public static class modelInit implements XPRMInitializationTo
 {

  /**** Retrieving data from Mosel ****/
  public boolean initializeTo(String label, XPRMValue value)
  {
   double retsol;
   int numshares, solcount;
    XPRMArray solarr;
   XPRMSet[] sets;
   int[] indices;
   int asize, ct;

   if(label.equals("FRAC"))
   {
    solarr=(XPRMArray)value;
    asize=solarr.getSize();
    solfrac = new MySolArray[asize];
    for(int i=0;i<asize;i++) solfrac[i] = new MySolArray();

    sets = solarr.getIndexSets();          // Get the indexing sets
    ct=0;
    indices = solarr.getFirstTEIndex();    // Get the first entry of the array
    do
    {
     solfrac[ct].ind=sets[0].getAsString(indices[0]);
     solfrac[ct].val=solarr.getAsReal(indices);
     ct++;
    } while(solarr.nextTEIndex(indices));  // Get the next index  
    if (ifprint==false) 
    { ifprint=true; }
    else
    {
     ifprint=false;
     for(int i=0;i<asize;i++)
      System.out.println(solfrac[i].ind + ": " + solfrac[i].val*100 + "% (" + 
       solbuy[i].val + ")");
     solbuy = null;
     solfrac = null;  
    }
   }
   else if(label.equals("BUY"))
   {
    solarr=(XPRMArray)value;
    asize=solarr.getSize();
    solbuy = new MySolArray[asize];
    for(int i=0;i<asize;i++) solbuy[i] = new MySolArray();

    sets = solarr.getIndexSets();          // Get the indexing sets
    ct=0;
    indices = solarr.getFirstTEIndex();    // Get the first entry of the array
    do
    {
     solbuy[ct].ind=sets[0].getAsString(indices[0]);
     solbuy[ct].val=solarr.getAsReal(indices);
     ct++;
    } while(solarr.nextTEIndex(indices));  // Get the next index  
    if (ifprint==false) 
    { ifprint=true; }
    else
    {
     ifprint=false;
     for(int i=0;i<asize;i++)
      System.out.println(solfrac[i].ind + ": " + solfrac[i].val*100 + "% (" + 
       solbuy[i].val + ")");
     solbuy = null;
     solfrac = null;  
    }
   }
   else if(label.equals("RETSOL"))
   {
    retsol=value.asReal();
    System.out.println("Total return: " + retsol);
   }
   else if(label.equals("NUMSHARES"))
   {
    numshares=value.asInteger();
    System.out.println("Number of shares: " + numshares);
   }
   else if(label.equals("SOLCOUNT"))
   {
    solcount=value.asInteger();
    System.out.println("Solution number: " + solcount);
   }
   else System.out.println("Unknown output data item: " + label + "=" + value);
   return true;
  }
 }

                /*************************************************/
                /* Interface objects are static: no need to bind */
                /*************************************************/
 public static modelInit cbinit=new modelInit();


 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
                    // Model parameter settings
  double maxrisk = 1.0/3;
  double minreg = 0.2;
  double maxreg = 0.5;
  double maxsec = 0.25;
  double maxval = 0.2;
  double minval = 0.1;
  int maxnum = 15;
  ifprint = false;

  try{
   mosel = new XPRM();                // Initialize Mosel
  }catch(XPRMLicenseError e){
                System.out.println("License error" + e.getMessage());
                throw new java.lang.Exception("Error during execution");
  }

  try{
   mosel.compile("foliocbio.mos");   // Compile the model (only required
                                      // during development phase, deployed
				      // application would only use BIM)
  }catch(XPRMCompileException e){
   System.out.println(e.getMessage());
  }

  mod = mosel.loadModel("foliocbio.bim");  // Load the model

                    // Pass model parameters through execution parameters
  mod.setExecParam("MAXRISK",maxrisk);
  mod.setExecParam("MINREG",minreg);
  mod.setExecParam("MAXREG",maxreg);
  mod.setExecParam("MAXSEC",maxsec);
  mod.setExecParam("MAXVAL",maxval);
  mod.setExecParam("MINVAL",minval);
  mod.setExecParam("MAXNUM",maxnum);
  mod.setExecParam("DATAFILE","folio250.dat");
  mod.setExecParam("OUTPUTFILE","java:runfoliocbio.cbinit");

  mod.run();                          // Run the model

  if(mod.getExecStatus() != XPRMModel.RT_OK){
   throw new java.lang.Exception("Error during model execution");
  }
  if(mod.getProblemStatus() != mod.PB_OPTIMAL){
   throw new java.lang.Exception("Problem not optimal");
  }                                  // Stop if no solution available

  mod.close();                       // Unload model
 }
}

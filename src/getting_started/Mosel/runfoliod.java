/*******************************************************
   Mosel Example Problems
   ====================== 

   file runfoliod.java
   ```````````````````
   Running a Mosel model from a Java application
   with data exchange between model and host application.
   (Passing data via callback)

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Oct. 2009
********************************************************/

import java.io.*;
import com.dashoptimization.*;

public class runfoliod
{
                   // Data for 'initializations from' (input data values)
  static final String[] riskset = {"hardware1", "hardware2", "hardware3", "hardware4", 
   "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", 
   "hardware10", "theater1", "theater2", "theater3", "theater4", "theater5", 
   "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1",
   "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", 
   "telecom8", "telecom9", "telecom10", "software1", "software2", "software3", 
   "software4", "software5", "software6", "software7", "software8", 
   "software9", "software10", "electronics1", "electronics2", "electronics3", 
   "electronics4", "electronics5", "electronics6", "electronics7", 
   "electronics8", "electronics9", "electronics10"};
  static final int risksize=50;

  static final String[] locdataind1 = {"EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "EU", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC", "APAC"};
  static final String[] locdataind2 = {"treasury1", "treasury10", "hardware7", "hardware9", "theater8", "theater10", "telecom3", "telecom9", "telecom10", "brewery1", "brewery4", "brewery6", "brewery7", "brewery8", "highways2", "highways5", "highways7", "highways10", "cars4", "cars7", "cars8", "cars9", "cars10", "bank1", "bank3", "bank5", "bank6", "bank7", "bank10", "software3", "software4", "software5", "software6", "electronics2", "treasury2", "treasury6", "treasury7", "treasury8", "hardware4", "hardware6", "theater1", "theater3", "theater7", "theater9", "telecom2", "telecom4", "telecom8", "highways3", "highways4", "cars5", "cars6", "bank2", "bank4", "bank8", "bank9", "software1", "software2", "software9", "electronics3", "electronics5", "electronics6", "electronics8", "electronics9", "electronics10", "treasury4", "treasury5", "treasury9", "hardware1", "hardware2", "hardware5", "hardware8", "hardware10", "theater2", "theater4", "telecom1", "telecom5", "telecom6", "telecom7", "brewery9", "brewery10", "highways8", "cars1", "cars2", "software7", "software8", "software10", "electronics1", "electronics4", "electronics7"};
  static final int[] locdata = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  static final int locsize=89;

  static final String[] secdataind1 = {"bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "bonds", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "technology", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "entertainment", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "telecom", "food", "food", "food", "food", "food", "food", "food", "food", "food", "food", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "construction", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "manufacturing", "finance", "finance", "finance", "finance", "finance", "finance", "finance", "finance", "finance", "finance"};
  static final String[] secdataind2 = {"treasury1", "treasury2", "treasury3", "treasury4", "treasury5", "treasury6", "treasury7", "treasury8", "treasury9", "treasury10", "hardware1", "hardware2", "hardware3", "hardware4", "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", "hardware10", "software1", "software2", "software3", "software4", "software5", "software6", "software7", "software8", "software9", "software10", "electronics1", "electronics2", "electronics3", "electronics4", "electronics5", "electronics6", "electronics7", "electronics8", "electronics9", "electronics10", "theater1", "theater2", "theater3", "theater4", "theater5", "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1", "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", "telecom8", "telecom9", "telecom10", "brewery1", "brewery2", "brewery3", "brewery4", "brewery5", "brewery6", "brewery7", "brewery8", "brewery9", "brewery10", "highways1", "highways2", "highways3", "highways4", "highways5", "highways6", "highways7", "highways8", "highways9", "highways10", "cars1", "cars2", "cars3", "cars4", "cars5", "cars6", "cars7", "cars8", "cars9", "cars10", "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9", "bank10"};
  static final double[] secdata = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  static final int secsize=100;

  static final String[] retdataind = {"treasury1", "treasury2", "treasury3", "treasury4", "treasury5", "treasury6", "treasury7", "treasury8", "treasury9", "treasury10", "hardware1", "hardware2", "hardware3", "hardware4", "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", "hardware10", "theater1", "theater2", "theater3", "theater4", "theater5", "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1", "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", "telecom8", "telecom9", "telecom10", "brewery1", "brewery2", "brewery3", "brewery4", "brewery5", "brewery6", "brewery7", "brewery8", "brewery9", "brewery10", "highways1", "highways2", "highways3", "highways4", "highways5", "highways6", "highways7", "highways8", "highways9", "highways10", "cars1", "cars2", "cars3", "cars4", "cars5", "cars6", "cars7", "cars8", "cars9", "cars10", "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9", "bank10", "software1", "software2", "software3", "software4", "software5", "software6", "software7", "software8", "software9", "software10", "electronics1", "electronics2", "electronics3", "electronics4", "electronics5", "electronics6", "electronics7", "electronics8", "electronics9", "electronics10"};
  static final double[] retdata = {5.29, 2.8, 3.59, 6.97, 3.3, 3.02, 2.98, 3.41, 3.93, 3.86, 11.36, 17.23, 16.31, 21.1, 13.76, 9.31, 16.99, 24.85, 18.52, 12.79, 36.1, 19.4, 23.5, 34.61, 16.91, 27.04, 25.82, 24.99, 36.89, 31.71, 10.89, 17.98, 12.31, 6.53, 6.11, 15.89, 12.46, 11.11, 16.6, 13.93, 8.79, 9.35, 8.88, 11.63, 6.21, 9.26, 4.77, 5.98, 10.92, 5.26, 5.39, 4.85, 8.34, 8.48, 6.42, 11.04, 13.15, 10.01, 7.71, 11.07, 5.74, 7.13, 8.75, 5.08, 6.23, 8.19, 8.03, 5.96, 4.17, 8.11, 6.79, 3.01, 4.92, 4.14, 8.98, 8.81, 4.7, 8.06, 6.22, 4.47, 34.59, 44.94, 43.05, 15.58, 42.05, 20.66, 20.76, 19.85, 20.05, 45.38, 23.07, 19.1, 23.83, 16.54, 28.33, 25.88, 22.14, 22.65, 12.59, 28.1};
  static final int retsize=100;

                    // Class to receive solution values of decision variables
 public static class MySolArray
 {
  public String ind;                // index name
  public double val;                // solution value
 }

 static double retsol;
 static int numshares, solstatus;
 static MySolArray[] solfrac;
 static MySolArray[] solbuy;

                /*************************************************/
                /* A class to initialize model data via callback */
                /*************************************************/
 public static class modelInit implements XPRMInitializationFrom, XPRMInitializationTo
 {
  public boolean initializeFrom(XPRMInitializeContext ictx, String label, XPRMTyped type)
  {
   int i;

   try
   {
    if(label.equals("RISK"))
    {
     ictx.sendControl(ictx.CONTROL_OPENLST);
     for(i=0;i<risksize;i++)
     {
      ictx.send(riskset[i]);
     }
     ictx.sendControl(ictx.CONTROL_CLOSELST);
     return true;
    }
    else
    if(label.equals("RET"))
    {
     ictx.sendControl(ictx.CONTROL_OPENLST);
     for(i=0;i<retsize;i++)
     {
      ictx.sendControl(ictx.CONTROL_OPENNDX);
       ictx.send(retdataind[i]);
      ictx.sendControl(ictx.CONTROL_CLOSENDX);
      ictx.send(retdata[i]);
     }
     ictx.sendControl(ictx.CONTROL_CLOSELST);
     return true;
    }
    else
    if(label.equals("LOCTAB"))
    {
     ictx.sendControl(ictx.CONTROL_OPENLST);
     for(i=0;i<locsize;i++)
     {
      ictx.sendControl(ictx.CONTROL_OPENNDX);
       ictx.send(locdataind1[i]);
       ictx.send(locdataind2[i]);
      ictx.sendControl(ictx.CONTROL_CLOSENDX);
      ictx.send(locdata[i]);
     }
     ictx.sendControl(ictx.CONTROL_CLOSELST);
     return true;
    }
    else
    if(label.equals("SECTAB"))
    {
     ictx.sendControl(ictx.CONTROL_OPENLST);
     for(i=0;i<secsize;i++)
     {
      ictx.sendControl(ictx.CONTROL_OPENNDX);
       ictx.send(secdataind1[i]);
       ictx.send(secdataind2[i]);
      ictx.sendControl(ictx.CONTROL_CLOSENDX);
      ictx.send(secdata[i]);
     }
     ictx.sendControl(ictx.CONTROL_CLOSELST);
     return true;
    }
    else
    {
     System.err.println("Label `"+label+"' not found.");
     return false;
    }
   }
   catch(IOException e)
   {
    System.err.println("`"+label+"' could not be initialized - "+e);
    return false;
   }
  }

  /**** Retrieving data from Mosel ****/
  public boolean initializeTo(String label, XPRMValue value)
  {
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
   }
   else if(label.equals("RETSOL"))
   {
    retsol=value.asReal();
   }
   else if(label.equals("NUMSHARES"))
   {
    numshares=value.asInteger();
   }
   else if(label.equals("SOLSTATUS"))
   {
    solstatus=value.asInteger();
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

  try{
   mosel = new XPRM();                // Initialize Mosel
  }catch(XPRMLicenseError e){
                System.out.println("License error" + e.getMessage());
                throw new java.lang.Exception("Error during execution");
  }

  try{
   mosel.compile("foliomemio.mos");   // Compile the model (only required
                                      // during development phase, deployed
				      // application would only use BIM)
  }catch(XPRMCompileException e){
   System.out.println(e.getMessage());
  }

  mod = mosel.loadModel("foliomemio.bim");  // Load the model


                    // Pass model parameters through execution parameters
  mod.execParams = "MAXRISK=" + maxrisk + ",MINREG=" + minreg + 
                   ",MAXREG=" + maxreg + ",MAXSEC=" + maxsec + ",MAXVAL=" +
                   maxval + ",MINVAL=" + minval + ",MAXNUM=" + maxnum +
                                      // File names (IO driver)
                   ",DATAFILE='java:runfoliod.cbinit'," + 
                   "OUTPUTFILE='java:runfoliod.cbinit'";

  mod.run();                          // Run the model

  if(mod.getExecStatus() != XPRMModel.RT_OK){
   throw new java.lang.Exception("Error during model execution");
  }
  if(mod.getProblemStatus() != mod.PB_OPTIMAL){
   throw new java.lang.Exception("Problem not optimal");
  }                                  // Stop if no solution available

                    // Display solution values obtained from the model
  System.out.println("Total return: " + retsol);

  for(int i=0;i<numshares;i++)
   System.out.println(solfrac[i].ind + ": " + solfrac[i].val*100 + "% (" +  solbuy[i].val + ")");

  mod.close();                        // Unload model
 }
}

/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugiocb.java
   ````````````````
   Exchanging data between model and host application.
   - Callbacks for exchanging data (sparse data, string indices) -

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Nov. 2009
********************************************************/

import com.dashoptimization.*;

public class ugiocb
{                                   // Input data
 static final double[] vdata={15,100,90,60,40,15,10, 1};   // VALUE
 static final double[] wdata={ 2, 20,20,30,40,30,60,10};   // WEIGHT
 static final String[] ind={"camera", "necklace", "vase", "picture", 
     "tv", "video", "chest", "brick"};                     // Index names
 static final int datasize=8;

                        // Class to receive solution values
 public static class MySol
 {
  public String ind;                // index name
  public double val;                // solution value
 }
 static MySol[] solution;
 static int solsize;

                /*************************************************/
                /* A class to initialize model data via callback */
                /*************************************************/
 public static class modelInit implements XPRMInitializationFrom, XPRMInitializationTo
 {
  public boolean initializeFrom(XPRMInitializeContext ictx, String label, XPRMTyped type)
  {
   try
   {
    if(label.equals("DATA"))
    {
     ictx.sendControl(ictx.CONTROL_OPENLST);
     for(int i=0;i<datasize;i++)
     {
      ictx.sendControl(ictx.CONTROL_OPENNDX);
       ictx.send(ind[i]);
      ictx.sendControl(ictx.CONTROL_CLOSENDX);
      ictx.sendControl(ictx.CONTROL_OPENLST);
       ictx.send(vdata[i]);
       ictx.send(wdata[i]);
      ictx.sendControl(ictx.CONTROL_CLOSELST);
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
   catch(java.io.IOException e)
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
   int ct;

   if(label.equals("SOL"))
   {
    solarr=(XPRMArray)value;
    solsize=solarr.getSize();
    solution = new MySol[solsize];
    for(int i=0;i<solsize;i++) solution[i] = new MySol();

    sets = solarr.getIndexSets();          // Get the indexing sets
    ct=0;
    indices = solarr.getFirstTEIndex();    // Get the first entry of the array
    do
    {
     solution[ct].ind=sets[0].getAsString(indices[0]);
     solution[ct].val=solarr.getAsReal(indices);
     ct++;
    } while(solarr.nextTEIndex(indices));  // Get the next index  
   }
   else System.out.println("Unknown output data item: " + label + "=" + value);
   return true;
  }
 }

                /* Interface objects are static: no need to bind */
 public static modelInit cbinit=new modelInit();

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;  

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("burglar13.mos");     // Compile & load the model
  mod = mosel.loadModel("burglar13.bim");

                        // File names are passed through execution parameters
  mod.execParams = "DATAFILE='java:ugiocb.cbinit'," + 
                   "SOLFILE='java:ugiocb.cbinit'";

  mod.run();                          // Run the model

  if(mod.getProblemStatus()!=mod.PB_OPTIMAL) 
   System.exit(1);                    // Stop if no solution found

                        // Display solution values obtained from the model
  System.out.println("Objective value: " + mod.getObjectiveValue());
  for(int i=0;i<solsize;i++)
   System.out.println(" take(" + solution[i].ind + "): " + solution[i].val);

  mod.close();                        // Unload the model
 }
}

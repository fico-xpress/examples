/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugiodense.java
   ````````````````````
   Exchanging data between model and host application.
   - Dense data -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006
********************************************************/

import com.dashoptimization.*;

public class ugiodense
{                                     // Input data
 static final double[] vdata={15,100,90,60,40,15,10, 1};   // VALUE
 static final double[] wdata={ 2, 20,20,30,40,30,60,10};   // WEIGHT

                                      // Array to receive solution values
 static double[] solution = new double[8];  

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("burglar8.mos");      // Compile & load the model
  mod = mosel.loadModel("burglar8.bim");

                        // Associate the Java objects with names in Mosel
  mosel.bind("vdat", vdata);
  mosel.bind("wdat", wdata);
  mosel.bind("sol", solution);
                        // File names are passed through execution parameters
  mod.execParams =
   "VDATA='noindex,vdat',WDATA='noindex,wdat',SOL='noindex,sol'";

  mod.run();                          // Run the model

  if(mod.getProblemStatus()!=mod.PB_OPTIMAL) 
   System.exit(1);                    // Stop if no solution found

                        // Display solution values obtained from the model
  System.out.println("Objective value: " + mod.getObjectiveValue());
  for(int i=0;i<8;i++)
   System.out.println(" take(" + (i+1) + "): " + solution[i]);

  mod.close();                        // Unload the model
 }
}

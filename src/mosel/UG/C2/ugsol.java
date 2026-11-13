/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugsol.java
   ```````````````
   Accessing modeling objects and solution information.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2004, rev. Apr. 2013
********************************************************/

import com.dashoptimization.*;

public class ugsol
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMArray varr, darr;
  XPRMMPVar x;
  XPRMSet set;
  int[] indices;
  double val;

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("burglar3.mos");      // Compile, load & run the model
  mod = mosel.loadModel("burglar3.bim");
  mod.run();

  if(mod.getProblemStatus()!=mod.PB_OPTIMAL) 
   System.exit(1);                    // Stop if no solution found

  System.out.println("Objective value: " + mod.getObjectiveValue());
                                      // Print the objective function value

  varr=(XPRMArray)mod.findIdentifier("take");  // Get model object 'take',
                                               // it must be an array
  darr=(XPRMArray)mod.findIdentifier("VALUE"); // Get model object 'VALUE',
                                               // it must be an array
  set=(XPRMSet)mod.findIdentifier("ITEMS");    // Get model object 'ITEMS',
                                               // it must be a set

  indices = varr.getFirstIndex();     // Get the first entry of array varr
                                      // (we know that the array is dense) 
  do
  {
   x = varr.get(indices).asMPVar();   // Get a variable from varr
   val = darr.getAsReal(indices);     // Get the corresponding value
// Alternative:
//   System.out.println("take(" + varr.getIndexSets()[0].get(indices[0]) + "): " + 
   System.out.println("take(" + set.get(indices[0]) + "): " + 
                      x.getSolution() + "\t (item value: " + val + ")"); 
                                      // Print the solution value
  } while(varr.nextIndex(indices));   // Get the next index

  mod.close();                        // Unload the model
 }
}

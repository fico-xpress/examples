/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioobj.java
  ``````````````````
  Accessing model results.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2006
********************************************************/

import com.dashoptimization.*;

public class folioobj
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel model;
  XPRMArray varr, darr;
  XPRMMPVar x;
  XPRMSet set;
  int[] indices;

  mosel = new XPRM();                        // Initialize Mosel

  mosel.compile("foliodata.mos");            // Compile the model
  model = mosel.loadModel("foliodata.bim");  // Load compiled model
  model.run();                               // Run the model

                                      // Test whether a solution is found 
				      // and print the objective value 
  if(model.getProblemStatus()==XPRMModel.PB_OPTIMAL)
    System.out.println("Objective value: " + model.getObjectiveValue());

                                      // Retrieve the decision variables 
  varr=(XPRMArray)model.findIdentifier("frac"); // Get model object 'frac',
                                                // it must be an array

                                      // Retrieve the index names 
  set=(XPRMSet)model.findIdentifier("SHARES");  // Get model object 'SHARES',
                                                // it must be a set

  indices = varr.getFirstIndex();     // Get the first entry of array varr
                                      // (we know that the array is dense)
  do
  {
   x = varr.get(indices).asMPVar();   // Get a variable from varr
   System.out.println(set.get(indices[0]) + ":\t" + x.getSolution()*100 + "%");
				      // Print the solution value
  } while(varr.nextIndex(indices));   // Get the next index

  model.close();                      // Unload model
 }
}

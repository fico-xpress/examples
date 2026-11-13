/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugioscalar.java
   ````````````````````
   Exchanging data between model and host application.
   - Scalars -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, Mar. 2008
********************************************************/

import com.dashoptimization.*;

public class ugioscalar
{
 public static class MyData            // Scalars for data in/output
 {
  public int wmax;
  public int numitem;
  public double objval;
 }

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  MyData data=new MyData();

  data.wmax=100;

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("burglar11.mos");     // Compile & load the model
  mod = mosel.loadModel("burglar11.bim");

                        // Associate the Java object with a name in Mosel
  mosel.bind("data", data);
                        // File names are passed through execution parameters
  mod.execParams =
   "WMAX='data(wmax)',NUM='data(numitem)',SOLVAL='data(objval)'";

  mod.run();                          // Run the model

  if(mod.getProblemStatus()!=mod.PB_OPTIMAL) 
   System.exit(1);                    // Stop if no solution found

                        // Display solution values obtained from the model
  System.out.println("Objective value: " + data.objval);
  System.out.println("Total number of items: " + data.numitem);

  mod.close();                        // Unload the model
 }
}

/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugparam.java
   `````````````````
   Passing parameters to a Mosel program.
   Accessing modeling objects (sets).

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002
********************************************************/

import com.dashoptimization.*;

public class ugparam
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMSet set;
  int LIM=500, first, last;

  mosel = new XPRM();                   // Initialize Mosel

  System.out.println("Compiling `prime'");
  mosel.compile("prime.mos");

  System.out.println("Loading `prime'");
  mod = mosel.loadModel("prime.bim");

  System.out.println("Executing `prime'");
  mod.execParams = "LIMIT=" + LIM;
  mod.run();

  System.out.println("`prime' returned: " + mod.getResult());

  set=(XPRMSet)mod.findIdentifier("SPrime");  // Get the object 'SPrime'
                                              // it must be a set
  if(!set.isEmpty())
  {
   first = set.getFirstIndex();         // Get the number of the first index
   last = set.getLastIndex();           // Get the number of the last index
   System.out.println("Prime numbers from 2 to " + LIM);
   for(int i=first;i<=last;i++)         // Print all set elements
    System.out.print(" " + set.getAsInteger(i) + ",");
   System.out.println();  
  }

  mod.close();                          // Unload model
 }
}

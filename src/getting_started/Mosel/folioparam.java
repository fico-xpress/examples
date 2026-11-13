/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioparam.java
  ````````````````````
  Parameterized model execution.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2006
********************************************************/

import com.dashoptimization.*;

public class folioparam
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;

  mosel = new XPRM();                        // Initialize Mosel

  mosel.compile("foliodata.mos");            // Compile the model
                                             // Load compiled model
  try ( XPRMModel mod = mosel.loadModel("foliodata.bim") )
  {
                                             // Set the run-time parameters
   mod.execParams = "OUTFILE=result2.dat,MAXRISK=0.4,MAXVAL=0.25";
   mod.run();                                 // Run the model

   System.out.println("`foliodata' returned: " + mod.getResult());
  }
 }
}

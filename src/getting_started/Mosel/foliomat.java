/********************************************************
  Mosel Library Example Problems
  ==============================

  file foliomat.java
  ``````````````````
  Exporting a matrix.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2006
********************************************************/

import com.dashoptimization.*;

public class foliomat
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;

  mosel = new XPRM();                        // Initialize Mosel

  mosel.compile("foliodata.mos");            // Compile the model

                                             // Load compiled model
  try ( XPRMModel mod=mosel.loadModel("foliodata.bim") )
  {
   mod.run();                                 // Run the model

 /* Output the LP/MIP problem (or the portion of a problem that is specified 
  * via mpvar+linctr only, ignoring solver-specific extensions such as 
  * indicators or general constraints) */
   mod.exportProblem("p", "folio");
  }
 }
}

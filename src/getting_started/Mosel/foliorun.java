/********************************************************
  Mosel Library Example Problems
  ==============================

  file foliorun.java
  ``````````````````
  Loading and running a BIM file.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2006
********************************************************/

import com.dashoptimization.*;

public class foliorun
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;

  mosel = new XPRM();                        // Initialize Mosel

  mod = mosel.loadModel("foliodata.bim");    // Load compiled model
  mod.run();

  System.out.println("`foliodata' returned: " + mod.getResult());
  mod.close();                               // Unload model
 }
}

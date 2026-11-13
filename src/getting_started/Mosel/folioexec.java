/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioexec.java
  ```````````````````
  Compiling, loading and running a model file.

  (c) 2008 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2006
********************************************************/

import com.dashoptimization.*;

public class folioexec
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;

  mosel = new XPRM();                        // Initialize Mosel

  mosel.compile("foliodata.mos");            // Compile the model
  mod = mosel.loadModel("foliodata.bim");    // Load compiled model
  mod.run();                                 // Run the model

  System.out.println("`foliodata' returned: " + mod.getResult());
  mod.close();                               // Unload model
 }
}

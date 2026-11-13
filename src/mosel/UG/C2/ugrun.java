/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugrun.java
   ```````````````
   Compiling a model into a BIM file,
   then load and run it.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2002
********************************************************/

import com.dashoptimization.*;

public class ugrun
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;

  mosel = new XPRM();                      // Initialize Mosel

  System.out.println("Compiling `burglar2'");
  mosel.compile("burglar2.mos");

  System.out.println("Loading `burglar2'");
  mod = mosel.loadModel("burglar2.bim");

  System.out.println("Executing `burglar2'");
  mod.run();

  System.out.println("`burglar2' returned: " + mod.getResult());
  mod.close();                             // Unload model
 }
}

/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcomptmp.java
   ```````````````````
   Compiling a model into a BIM file saved in Mosel's
   temporary directory, then load and run it.

   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Apr. 2013
********************************************************/

import com.dashoptimization.*;

public class ugcomptmp
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;

  mosel = new XPRM();                      // Initialize Mosel

  System.out.println("Compiling `burglar2'");
  mosel.compile("", "burglar2.mos", "tmp:burglar2.bim", "");

  System.out.println("Loading `burglar2'");
  mod = mosel.loadModel("tmp:burglar2.bim");

  System.out.println("Executing `burglar2'");
  mod.run();

  System.out.println("`burglar2' returned: " + mod.getResult());
  mod.close();                             // Unload model
 }
}

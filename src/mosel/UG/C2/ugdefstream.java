/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugdefstream.java
   ```````````````````
   Redirecting model output.

   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Apr. 2013
********************************************************/

import com.dashoptimization.*;

public class ugdefstream
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

// Uncomment one of the following: 
                                  // Disable model output 
  mod.setDefaultStream(XPRM.F_OUTPUT, "null:"); 

                                  // Double model output (file+stdout) 
//  mod.setDefaultStream(XPRM.F_OUTPUT, "tee:burgres.txt&");

  System.out.println("Executing `burglar2'");
  mod.run();

  System.out.println("`burglar2' returned: " + mod.getResult());
  mod.close();                             // Unload model
 }
}

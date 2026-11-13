/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcompmem.java
   ```````````````````
   Compiling a model to memory.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006
********************************************************/

import java.nio.*;
import com.dashoptimization.*;

public class ugcompmem
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  ByteBuffer bimfile;                   // Buffer to store BIM file

  mosel = new XPRM();                   // Initialize Mosel

                                        // Prepare file names for compilation
  bimfile=ByteBuffer.allocateDirect(2048);   // Create a 2K byte buffer
  mosel.bind("mybim", bimfile);         // Associate Java obj. with Mosel name

  try
  {                                     // Compile model to memory
   mosel.compile("", "burglar2.mos", "java:mybim", "");
  }
  catch(XPRMCompileException e)
  {
   System.out.println(e.getMessage());
  }

  bimfile.limit(bimfile.position());    // Mark end of data in the buffer
  bimfile.rewind();                     // Back to the beginning
  System.out.println("BIM file uses "+bimfile.limit()+" bytes of memory.");
                                        // Load a BIM file from memory 
  try (XPRMModel mod=mosel.loadModel("java:mybim"))
  {
   mosel.unbind("mybim");                // Release memory
   bimfile=null;
   mod.run();                            // Run the model
  }
 }
}

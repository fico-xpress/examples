/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcb.java
   ``````````````
   Retrieve model output via callback-style functionality.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006
********************************************************/

import java.io.*;
import com.dashoptimization.*;

public class ugcb
{
                            // OutputStream class to handle default output
 public static class MyOut extends OutputStream
 {
  public void flush()
  { System.out.flush(); }
  public void write(byte[] b)
  { 
   System.out.print("Mosel: "); 
   System.out.write(b, 0, b.length);
  }
  // The following methods are not used by Mosel:
  public void write(byte[] b, int off, int len) {}
  public void write(int b) {}
  public void close() {}
 }

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  MyOut cbmsg = new MyOut();            // Define output stream as "MyOut"

  mosel = new XPRM();                   // Initialize Mosel

                                // Redirect error stream to default output
  mosel.setDefaultStream(XPRM.F_ERROR, "java:java.lang.System.out");

  mosel.bind("mycb", cbmsg);    // Associate Java object with a name in Mosel
                                // Set default output stream to cbmsg
  mosel.setDefaultStream(XPRM.F_OUTPUT|XPRM.F_LINBUF, "java:mycb");

  mosel.compile("burglar2.mos");        // Compile, load & run the model
  try (XPRMModel mod = mosel.loadModel("burglar2.bim"))
  { mod.run(); }
 }
}

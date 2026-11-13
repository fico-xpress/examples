/*******************************************************
   Mosel Example Problems
   ====================== 

   file runfoliob.java
   ```````````````````
   Running a Mosel model from a Java application
   with data exchange between model and host application.
   (Passing data through buffers)

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Mar. 2009
********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class runfoliob
{
 private static final String DATAFILE = "./folio10.dat";

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
                    // Model parameter settings
  double maxrisk = 1.0/3;
  double minreg = 0.2;
  double maxreg = 0.5;
  double maxsec = 0.25;
  double maxval = 0.2;
  double minval = 0.1;
  int maxnum = 15;
                    // Input and output handling                   
  ByteBuffer inputData;
  ByteBuffer resultOutput;
  StringBuffer dataBuffer = new StringBuffer();
  int last;
  byte[] output;

                   // Read input data from text file in Mosel format
  BufferedReader filereader = new BufferedReader(new FileReader(DATAFILE));
  String dataLine = filereader.readLine();
  while(dataLine != null)
  {
   dataBuffer.append(dataLine);       // For general case: add separator (space)
   dataLine = filereader.readLine();
  }
  filereader.close();

  inputData = ByteBuffer.wrap(dataBuffer.toString().getBytes());
  resultOutput = ByteBuffer.allocate(4096);   // Create 4K byte buffer


  try{
   mosel = new XPRM();                // Initialize Mosel
  }catch(XPRMLicenseError e){
                System.out.println("License error" + e.getMessage());
                throw new java.lang.Exception("Error during execution");
  }

  try{
   mosel.compile("foliomemio.mos");   // Compile the model (only required
                                      // during development phase, deployed
				      // application would only use BIM)
  }catch(XPRMCompileException e){
   System.out.println(e.getMessage());
  }

  mod = mosel.loadModel("foliomemio.bim");  // Load the model

                    // Associate the Java objects with names in Mosel
  mosel.bind("inputdata", inputData);
  mosel.bind("outputdata", resultOutput);

                    // Pass model parameters through execution parameters
  mod.execParams = "MAXRISK=" + maxrisk + ",MINREG=" + minreg + 
                   ",MAXREG=" + maxreg + ",MAXSEC=" + maxsec + ",MAXVAL=" +
                   maxval + ",MINVAL=" + minval + ",MAXNUM=" + maxnum +
                                      // File names (IO driver)
                   ",DATAFILE='java:inputdata',OUTPUTFILE='java:outputdata',";

  mod.run();                          // Run the model

  if(mod.getExecStatus() != XPRMModel.RT_OK){
   throw new java.lang.Exception("Error during model execution");
  }
  if(mod.getProblemStatus() != mod.PB_OPTIMAL){
   throw new java.lang.Exception("Problem not optimal");
  }                                  // Stop if no solution available

                    // Display result output obtained from the model
  System.out.println("Solution:");
  last = resultOutput.position();
  resultOutput.rewind();
  output = resultOutput.array();
  for (int i=0; i<last; i++)
   System.out.print((char)output[i]);

  mod.close();                        // Unload the model
 }
}

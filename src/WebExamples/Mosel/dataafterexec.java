/*******************************************************
   Mosel Example Problems
   ====================== 

   file dataafterexec.java
   ```````````````````````
   Retrieving data from a model after its execution.
   - Sparse data format -

   (c) 2008 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2006
********************************************************/

import com.dashoptimization.*;

public class dataafterexec
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMArray dataout;
  int[] indices;
  double val;

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("data_out.mos");      // Compile & load the model
  mod = mosel.loadModel("data_out.bim");
  mod.run();                          // Run the model

  dataout=(XPRMArray)mod.findIdentifier("A"); // Get model object 'A',
                                              // it must be an array

                                      // Display values obtained from the model
  indices = dataout.getFirstTEIndex();        // Get the first array entry 
  do
  {
   val = dataout.getAsReal(indices);          // Get the corresponding value
   System.out.println(" A(" + indices[0] + "," + indices[1] + "): " + val); 
  } while(dataout.nextTEIndex(indices));      // Get the next index tuple

  mod.close();                        // Unload the model
 }
}

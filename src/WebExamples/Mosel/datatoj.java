/*******************************************************
   Mosel Example Problems
   ====================== 

   file datatoj.java
   `````````````````
   Exchanging data between model and host application
   using IO drivers.
   - Sparse data format -
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, Apr. 2006
********************************************************/

import com.dashoptimization.*;

public class datatoj
{
                  // Class to receive output data values
 public static class MyOutData
 {
  public int index1,index2;           // index names
  public double value;                // data value
 }

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  MyOutData[] outdata=new MyOutData[9];

  for(int i=0;i<9;i++) outdata[i] = new MyOutData();

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("dataoutputj.mos");   // Compile & load the model
  mod = mosel.loadModel("dataoutputj.bim");

                  // Associate the Java object with a name in Mosel
  mosel.bind("outdt", outdata);
                  // File names are passed through execution parameters:
                  // use fields index1, index2, value of Mosel object 'outdt'
  mod.execParams = "DOUT='outdt(index1,index2,value)'";

  mod.run();                          // Run the model

  for(int i=0;i<9;i++)                // Display values obtained from the model
   System.out.println(" A(" + outdata[i].index1 + "," + outdata[i].index2 
                      + "): " + outdata[i].value);

  mod.close();                        // Unload the model
 }
}

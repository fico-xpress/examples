/*******************************************************
   Mosel Example Problems
   ====================== 

   file datafromj.java
   ```````````````````
   Exchanging data between model and host application
   using IO drivers.
   - Sparse data format -
   Executing a model file.

   (c) 2008 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2006
********************************************************/

import com.dashoptimization.*;

public class datafromj
{
                   // Class to store initial values for array 'data'
 public static class MyData
 {
  public int index_i,index_j;         // index names
  public double value;                // data value
  MyData(int i, int j, double v)
  { index_i=i; index_j=j; value=v; }
 }

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  MyData data[]={new MyData(1,1,12.5), new MyData(2,3,5.6), 
                 new MyData(10,9,-7.1), new MyData(3,2,1)};

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("datainputj.mos");    // Compile & load the model
  mod = mosel.loadModel("datainputj.bim");

                  // Associate the Java object with a name in Mosel
  mosel.bind("dt", data);
                  // File names are passed through execution parameters:
                  // use fields index_i, index_j, value of Mosel object 'dt'
  mod.execParams = "DATA='dt(index_i,index_j,value)'";

  mod.run();                          // Run the model

  mod.close();                        // Unload the model
 }
}

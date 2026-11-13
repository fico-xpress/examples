/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugstreamdensescrmt.java
   ````````````````````````````
   Exchanging data between model and host application
   using Java streams.
   - Dense data array and scalar data -

   (c) 2022 Fair Isaac Corporation
       author: S. Heipcke, Jan 2022
********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class ugstreamdensescrmt
{                                     // Input data
 static final double[] vdata={15,100,90,60,40,15,10, 1};   // VALUE
 static final double[] wdata={ 2, 20,20,30,40,30,60,10};   // WEIGHT

 public static class MyData            // Scalars for data in/output
 {
  public int wmax;
  public double objval;
 }
                                      // Array to receive solution values
 static final int SOLSIZE=8;
 static double[] solution = new double[SOLSIZE];  

 // A stream to send the 'MyData' array in a form suitable for the 'raw:' driver
 public static class MyInitStream extends InputStream
 {
  int ndx;
  double mydata[];

  MyInitStream(double data[])
  {
   ndx=0;
   mydata=data;
  }

  public int read(byte[] b)
  {
   ByteBuffer buf;
   byte [] bs;

   if(ndx<mydata.length)
   {
                        // Handle the buffer as a ByteBuffer
    buf=ByteBuffer.wrap(b);
                        // Set the byte ordering for binary data
    buf.order(ByteOrder.nativeOrder());                  
    buf.rewind();       // Start from the beginning
                        // Put the data values (8 bytes each)
    buf.putDouble(mydata[ndx]);
                        // Increase counter for next call
    ndx++;
    return buf.position();
   }
   else
    return 0;
  }

  // Other methods are not used by Mosel
  public int read(byte[] b,int off,int len) { return 0; }
  public int read() { return 0; }
  public void close() {}
 }

 // A stream to retrieve the 'MySol' array in a form suitable for the 'raw:' driver
 public static class MyOutStream extends OutputStream
 {
  int ndx;
  double mysol[];

  MyOutStream(double data[])
  {
   ndx=0;
   mysol=data;
  }

  public void write(byte[] b)
  {
   ByteBuffer buf;
   byte[] bs = new byte[16];

                        // Handle the buffer as a ByteBuffer
   buf=ByteBuffer.wrap(b);
                        // Set the byte ordering for binary data
   buf.order(ByteOrder.nativeOrder());            
   buf.rewind();        // Start from the beginning

   try
   {
    while(buf.hasRemaining())
    {                   // Get the value (8 bytes)
     mysol[ndx] = buf.getDouble();
     ndx++;             // Increase counter for next call    
    }
   }
   catch(Exception e)
   {
    System.err.println(e.getMessage());
    System.exit(1);   
   } 
  }

  // Other methods are not used by Mosel
  public void write(byte[] b,int off,int len) {}
  public void write(int i) {}
  public void close() {}
 }

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  MyInitStream myvdatastream=new MyInitStream(vdata);
  MyInitStream mywdatastream=new MyInitStream(wdata);
  MyOutStream mysolstream=new MyOutStream(solution);
  MyData data=new MyData();

  data.wmax=102;

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("runburglar8sdc.mos");   // Compile & load the model
  mod = mosel.loadModel("runburglar8sdc.bim");

                        // Associate the Java objects with names in Mosel
  mosel.bind("vdat", myvdatastream);
  mosel.bind("wdat", mywdatastream);
  mosel.bind("sol", mysolstream);
  mosel.bind("data", data);
                        // File names are passed through execution parameters
  mod.execParams =
   "VDATA='java:vdat',WDATA='java:wdat',SOL='java:sol',WMAX='data(wmax)',OBJVAL='data(objval)'";

  mod.run();                          // Run the model

  if(mod.getExecStatus()!=mod.RT_OK || mod.getExitCode()!=0) {
   System.out.println("(Sub)model execution error");
   System.exit(1);                    // Stop if an error has occurred
  }
                        // Display solution values obtained from the model
  System.out.println("Objective value: " + data.objval);
  for(int i=0;i<SOLSIZE;i++)
   System.out.println(" take(" + (i+1) + "): " + solution[i]);

  mod.close();                        // Unload the model
 }
}

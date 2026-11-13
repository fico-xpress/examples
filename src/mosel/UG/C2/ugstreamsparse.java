/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugstreamsparse.java
   ````````````````````````
   Exchanging data between model and host application
   using Java streams.
   - Sparse data (string indices) -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, July 2008
********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class ugstreamsparse
{
                        // Class to store initial values for array 'data'
 public static class MyData
 {
  public String ind;                // index name
  public double val,wght;           // value and weight data entries
  MyData(String i, double v, double w)
  { ind=i; val=v; wght=w; }
 }
                        // Class to receive solution values
 public static class MySol
 {
  public String ind;                // index name
  public double val;                // solution value
 }

 // A stream to send the 'MyData' array in a form suitable for the 'raw:' driver
 public static class MyInitStream extends InputStream
 {
  int ndx;
  MyData mydata[];

  MyInitStream(MyData data[])
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
                        // Convert the string into a byte array
    bs=mydata[ndx].ind.getBytes();                 
    buf.put(bs);        // Copy the array into the ByteBuffer
              // Strings are of fixed size (Mosel default 16 characters): 
              // we padd with 0
    for(int i=bs.length;i<16;i++)
     buf.put((byte)0);
                        // Put the data values (8 bytes each)
    buf.putDouble(mydata[ndx].val);
    buf.putDouble(mydata[ndx].wght);
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
  MySol mysol[];

  MyOutStream(MySol data[])
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
    {            
     buf.get(bs);       // Copy the array into the ByteBuffer
                        // Convert the string into a byte array
     mysol[ndx].ind = new String(bs);
                        // Get the value (8 bytes)
     mysol[ndx].val = buf.getDouble();
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
  MyData data[]={new MyData("camera",15,2), new MyData("necklace",100,20), 
                 new MyData("vase",90,20), new MyData("picture",60,30), 
                 new MyData("tv",40,40), new MyData("video",15,30), 
                 new MyData("chest",10,60), new MyData("brick",1,10)};
  MyInitStream mydatastream=new MyInitStream(data);
  MySol[] solution=new MySol[8];
  MyOutStream mysolstream=new MyOutStream(solution);

  for(int i=0;i<8;i++) solution[i] = new MySol();

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("burglar9s.mos");     // Compile & load the model
  mod = mosel.loadModel("burglar9s.bim");

                        // Associate the Java objects with names in Mosel
  mosel.bind("dt", mydatastream);
  mosel.bind("sol", mysolstream);
                        // File names are passed through execution parameters
  mod.execParams = "DATA='dt',SOL='sol'";

  mod.run();                          // Run the model

  if(mod.getProblemStatus()!=mod.PB_OPTIMAL) 
   System.exit(1);                    // Stop if no solution found

                        // Display solution values obtained from the model
  System.out.println("Objective value: " + mod.getObjectiveValue());
  for(int i=0;i<8;i++)
   System.out.println(" take(" + solution[i].ind + "): " + solution[i].val);

  mod.close();                        // Unload the model
 }
}

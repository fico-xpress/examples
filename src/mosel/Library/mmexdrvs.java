/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexdrvs.java                                  */
/*  ``````````````````                                  */
/*  Example for the use of the Mosel libraries          */
/*  (using IOdrivers for data exchange)                 */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2004                      */
/********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class mmexdrvs
{
                /***************************************/
                /* The source of the model as a string */
                /***************************************/
 static final String source_of_model=
"model drivers\n"+

"parameters\n"+
" DATA=''\n"+
" SOL=''\n"+
"end-parameters\n"+

"declarations\n"+
" S:set of string\n"+
" R:range\n"+
" data:array(S,R) of real\n"+
" sol:array(1..10) of real\n"+
"end-declarations\n"+

"initialisations from 'jraw:'\n"+
" data as DATA\n"+
"end-initialisations\n"+

"writeln('set   S=',S)\n"+
"writeln('range R=',R)\n"+
"writeln('array data=',data)\n"+

"forall(i in 1..10) sol(i):=i^2\n"+

"initialisations to 'jraw:'\n"+
" sol as SOL\n"+
"end-initialisations\n"+

"end-model";
                /***************************************************/
                /* Class to store initial values for array 'data': */
                /***************************************************/
public static class MyData
{
 public String s;           // text for the first index
 public int r;              // integer value for the second index
 public double v;           // corresponding value: data(s,r)
 MyData(String i1,int i2,double v0)
 { s=i1; r=i2; v=v0; }
}

                /************************************/
                /* Array to receive solution values */
                /************************************/
static double[] solution=new double[10];

                /***********************************************/
                /* OutputStream class to handle default output */
                /***********************************************/
public static class MyOut extends OutputStream
{
 public void flush()
 { System.out.flush(); }
 public void write(byte[] b)
 { System.out.print("Mosel: "); System.out.write(b,0,b.length); }
 // Other methods are not used by Mosel
 public void write(byte[] b,int off,int len) {}
 public void write(int b) {}
 public void close() {}
}

                /*****************/
                /* Main function */
                /*****************/
public static void main(String[] args) throws Exception
{
 XPRM mosel;                 // An instance of Mosel
 XPRMModel mod;              // The model
 ByteBuffer bimfile;         // Buffer to store BIM file
 ByteBuffer mosfile;         // Buffer to store source file
 MyOut cbmsg=new MyOut();    // Output stream as "MyOut"
 MyData[] data={new MyData("one",2,12.5),new MyData("two",1,15),
                new MyData("three",16,9),new MyData("hundred",2,17)};



 mosel=new XPRM();           // Initialize Mosel

                             // Set default output stream to cbmsg
 mosel.bind("cbmsg",cbmsg);                   // Save a reference
 mosel.setDefaultStream(XPRM.F_OUTPUT|XPRM.F_LINBUF,"java:cbmsg");

                        /*************************************************/
                        /* Prepare file names for compilation            */
                        /*************************************************/
                                              // Wrap source in a byte buffer
 mosfile=ByteBuffer.wrap(source_of_model.getBytes());
 mosel.bind("mosfile",mosfile);               // Save a reference
 bimfile=ByteBuffer.allocateDirect(1024);     // Create a 1K byte buffer
 mosel.bind("bimfile",bimfile);               // Save a reference

                                         // Compile model from memory to memory
 try
 {
  mosel.compile("","java:mosfile","java:bimfile","test mem comp");
 }
 catch(XPRMCompileException e)
 {
  System.out.println(e.getMessage());
 }
 mosel.unbind("mosfile");                     // Release memory
 mosfile=null;

 bimfile.limit(bimfile.position());           // Mark end of data in the buffer
 bimfile.rewind();                            // Back to the beginning
 System.out.println("BIM file uses "+bimfile.limit()+" bytes of memory.");

 mod=mosel.loadModel("java:bimfile");         // Load a BIM file from memory
 mosel.unbind("bimfile");                     // Release memory
 bimfile=null;

                        // file names are passed through execution parameters
 mod.bind("data",data);
 mod.setExecParam("DATA","data(s,r,v)");
 mod.bind("solution",solution);
 mod.setExecParam("SOL","noindex,solution");

 mod.run();                                   // Run the model
 mod.close();                                 // Unload the model

                        // Display solution values obtained from the model
 System.out.print("Solution values:");
 for(int i=0;i<10;i++)
  System.out.print(" "+solution[i]);
 System.out.println();
}
}

/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexcbdrv.java                                 */
/*  ```````````````````                                 */
/*  Example for the use of the Mosel libraries          */
/*  (using IOdrivers for data exchange)                 */
/*                                                      */
/*  (c) 2009 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2009                      */
/********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class mmexcbdrv
{
                /***************************************/
                /* The source of the model as a string */
                /***************************************/
 static final String source_of_model=
"model tstcb\n"+
"uses 'mmsystem'\n"+
"parameters\n"+
" ICB=''\n"+
"end-parameters\n"+

"public declarations\n"+
" v_i:integer\n"+
" v_r:real\n"+
" v_s:string\n"+
" v_b:boolean\n"+
" v_d:date\n"+

" s_i:set of integer\n"+
" l_i:list of integer\n"+

" s_d:set of date\n"+
" l_d:list of date\n"+

" a_i:array(range) of integer\n"+
" Rx:range\n"+
" a_s:array(Rx) of string\n"+
" a_r:array(Rx) of real\n"+

" R=public record\n"+
"    i:integer\n"+
"    s:set of integer\n"+
"   end-record\n"+
" r:R\n"+
" a_R:array(range) of R\n"+
"end-declarations\n"+

"initialisations from ICB\n"+
" v_i\n"+
" v_r\n"+
" v_s\n"+
" v_b\n"+
" v_d\n"+

" s_i\n"+
" l_i\n"+

" s_d\n"+
" l_d\n"+

" a_i\n"+
" [a_s,a_r] as 'ax'\n"+

" r\n"+
" a_R\n"+
"end-initialisations\n"+

" writeln('v_i=',v_i)\n"+
" writeln('v_r=',v_r)\n"+
" writeln('v_s=',v_s)\n"+
" writeln('v_b=',v_b)\n"+
" writeln('v_d=',v_d)\n"+
" writeln('s_i=',s_i)\n"+
" writeln('l_i=',l_i)\n"+
" writeln('s_d=',s_d)\n"+
" writeln('l_d=',l_d)\n"+
" writeln('a_i=',a_i)\n"+
" writeln('a_r=',a_r)\n"+
" writeln('a_s=',a_s)\n"+
" writeln('r=',r)\n"+
" writeln('a_R=',a_R)\n"+

"initialisations to ICB\n"+
" v_i\n"+
" v_r\n"+
" v_s\n"+
" v_b\n"+
" v_d\n"+

" s_i\n"+
" l_i\n"+

" s_d\n"+
" l_d\n"+

" a_i\n"+

" r\n"+
" a_R\n"+
"end-initialisations\n"+
"end-model";

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

                /*************************************************/
                /* A class to initialize model data via callback */
                /*************************************************/
public static class modelInit implements XPRMInitializationFrom,XPRMInitializationTo
{
 public boolean initializeFrom(XPRMInitializeContext ictx,String label,XPRMTyped type)
 {
  int i,j;

  try {                 // v_i:999
   if(label.equals("v_i"))
   {
    ictx.send(999);
    return true;
   }
   else                 // v_r:999.99
   if(label.equals("v_r"))
   {
    ictx.send(999.99);
    return true;
   }
   else                 // v_b:false
   if(label.equals("v_b"))
   {
    ictx.send(false);
    return true;
   }
   else                // v_s:"tralala"
   if(label.equals("v_s"))
   {
    ictx.send("tralala");
    return true;
   }
   else                // v_d:"2012-12-12"
   if(label.equals("v_d"))
   {
    ictx.send("2012-12-12");
    return true;
   }
   else                // s_i:[10 20 30 ... ]
   if(label.equals("s_i")||label.equals("l_i"))
   {
    ictx.sendControl(ictx.CONTROL_OPENLST);
    for(i=1;i<=10;i++)
     ictx.send(i*10);
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    return true;
   }
   else                // s_d:["2001-01-11" "2002-02-21" ... ]
   if(label.equals("s_d")||label.equals("l_d"))
   {
    ictx.sendControl(ictx.CONTROL_OPENLST);
    for(i=1;i<=10;i++)
     ictx.send(String.valueOf(2000+i)+"-"+i+"-"+i+1);
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    return true;
   }
   else                // a_i:[ (1) 10 (2) 20 ... ]
   if(label.equals("a_i"))
   {
    ictx.sendControl(ictx.CONTROL_OPENLST);
    for(i=1;i<=10;i++)
    {
     ictx.sendControl(ictx.CONTROL_OPENNDX);
      ictx.send(i);
     ictx.sendControl(ictx.CONTROL_CLOSENDX);
     ictx.send(i*10);
    }
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    return true;
   }
   else                // a_x:[ (1) [ "aa1" 1.23 ] (2) [ "aa2" 2.46 ]...]
   if(label.equals("ax"))
   {
    ictx.sendControl(ictx.CONTROL_OPENLST);
    for(i=1;i<=10;i++)
    {
     ictx.sendControl(ictx.CONTROL_OPENNDX);
      ictx.send(i);
     ictx.sendControl(ictx.CONTROL_CLOSENDX);
     ictx.sendControl(ictx.CONTROL_OPENLST);
      ictx.send("aa"+i);
      ictx.send((double)i*1.23);
     ictx.sendControl(ictx.CONTROL_CLOSELST);
    }
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    return true;
   }
   else                // r:[ 123 [ 10 20 30 ] ]
   if(label.equals("r"))
   {
    ictx.sendControl(ictx.CONTROL_OPENLST);
    ictx.send(123);
    ictx.sendControl(ictx.CONTROL_OPENLST);
    for(i=1;i<=3;i++)
     ictx.send(i*10);
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    return true;
   }
   else                // a_R:[ (1) [10 [10 20 30] ] (1) [20 [20 40 60] ] ... ]
   if(label.equals("a_R"))
   {
    ictx.sendControl(ictx.CONTROL_OPENLST);
    for(i=1;i<=10;i++)
    {
     ictx.sendControl(ictx.CONTROL_OPENNDX);
      ictx.send(i);
     ictx.sendControl(ictx.CONTROL_CLOSENDX);
     ictx.sendControl(ictx.CONTROL_OPENLST);
     ictx.send(i*10);
     ictx.sendControl(ictx.CONTROL_OPENLST);
     for(j=1;j<=3;j++)
      ictx.send(j*i*10);
     ictx.sendControl(ictx.CONTROL_CLOSELST);
     ictx.sendControl(ictx.CONTROL_CLOSELST);
    }
    ictx.sendControl(ictx.CONTROL_CLOSELST);
    return true;
   }
   else
   {
    System.err.println("Label `"+label+"' not found.");
    return false;
   }
  }
  catch(IOException e)
  {
   System.err.println("`"+label+"' could not be initialized - "+e);
   return false;
  }
 }

 /* A method to retrive data from Mosel */
 public boolean initializeTo(String label,XPRMValue value)
 {
  System.out.println("Java: "+label+"="+value);
  return true;
 }
}

                /*************************************************/
                /* Interface objects are static: no need to bind */
                /*************************************************/
public static modelInit cbinit=new modelInit();
public static MyOut cbmsg=new MyOut();

                /*****************/
                /* Main function */
                /*****************/
public static void main(String[] args) throws Exception
{
 XPRM mosel;                 // An instance of Mosel
 XPRMModel mod;              // The model
 ByteBuffer bimfile;         // Buffer to store BIM file
 ByteBuffer mosfile;         // Buffer to store source file

 mosel=new XPRM();           // Initialize Mosel

                             // Set default output stream to cbmsg
 mosel.setDefaultStream(XPRM.F_OUTPUT|XPRM.F_LINBUF,"java:mmexcbdrv.cbmsg");

                        /*************************************************/
                        /* Prepare file names for compilation            */
                        /*************************************************/
                                              // Wrap source in a byte buffer
 mosfile=ByteBuffer.wrap(source_of_model.getBytes());
 mosel.bind("mosfile",mosfile);               // Save a reference
 bimfile=ByteBuffer.allocateDirect(3072);     // Create a 3K byte buffer
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

                                              // Load a BIM file from memory
 mod=mosel.loadModel("java:bimfile");
 mosel.unbind("bimfile");                     // Release memory
 bimfile=null;

 mod.setExecParam("ICB","java:mmexcbdrv.cbinit");

 mod.run();                                   // Run the model
 mod.close();                                 // Unload the model
}
}

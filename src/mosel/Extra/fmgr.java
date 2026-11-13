/********************************************************/
/*  XPRD library example                                */
/*  ====================                                */
/*                                                      */
/*  file fmgr.java                                      */
/*  ``````````````                                      */
/* Example of use of XPRD:                              */
/*                    implementation of a file manager  */
/*                                                      */
/*  (c) 2011 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2011                      */
/********************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;

public class fmgr
{
 static byte bimbuf[]=null;
 static String srcbuf=
	"model a\n"+
	"uses 'mmjobs';\n"+
	"send(9,-1)\n"+
	"wait\n"+
	"writeln('Got:',getnextevent)\n"+
	"fopen('tmp:file.txt',F_INPUT)\n"+
	"s:='';readln(s); fclose(F_INPUT)\n"+
	"writeln('Read:',s)\n"+
	"writeln('my ID:',getparam('jobid'))\n"+
	"send(10,12.5)\n"+
	"fopen('tmp:file.txt',F_OUTPUT)\n"+
	"writeln('Some text from remote')\n"+
	"fclose(F_OUTPUT)\n"+
	"writeln('End of model')\n"+
	"end-model";

 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel m;
  XPRDModel mod;
  XPRDFileManager fmgr=new FileManager();

  // Create an instance with a user-defined file manager
  m=xprd.connect("",fmgr);	   // in a separate process
//  m=xprd.connect("localhost",fmgr);// from am xprmsrv server
  System.out.println("Connected to:\n"+m.getBanner());
  System.out.println("Host: "+m.getSystemInformation(m.SYS_NODE));

  // Set error stream to a local file
  try
  { m.setDefaultStream(m.F_ERROR,"rmt:err.txt"); }
  catch(Exception e)
  {
   System.out.println("could not set error stream:"+e);
   System.exit(1);
  }

  // Now compile the source from a local buffer to a local buffer(!)
  try
  {
   m.compile("","rmt:a.mos","rmt:bimfile");
  }
  catch(Exception e)
  {
   System.out.println("Compilation failed:"+e);
   System.exit(1);
  }

  // Save something in the temp directory of the remote instance
  try
  {
   OutputStreamWriter outs;
   outs=new OutputStreamWriter(m.openForWriting("tmp:file.txt",0),"UTF-8");
   outs.write("Hello_from_a_file\n");
   outs.close();
  }
  catch(Exception e)
  {
   System.out.println("Output failed:"+e);
   System.exit(1);
  }

  // use the local buffer to load a model
  mod=m.loadModel("rmt:bimfile");

  // Redirect output stream of the model to the 'outremote' callback
  try
  { mod.setDefaultStream(mod.F_OUTPUT+mod.F_LINBUF,"rmt:outremote"); }
  catch(Exception e)
  {
   System.out.println("could not set output stream:"+e);
   System.exit(1);
  }

  System.out.println("run");
  mod.run();
  xprd.waitForEvent();
  System.out.println(xprd.getNextEvent());
  mod.sendEvent(5,3.33);
  do
  {
   xprd.waitForEvent();
   System.out.println(xprd.getNextEvent());
  }while(mod.getExecStatus()==mod.RT_RUNNING);

  // Read something from the temp directory of the remote instance
  try
  {
   int i;
   char buf[]=new char[128];
   InputStreamReader ins;

   ins=new InputStreamReader(m.openForReading("tmp:file.txt",0),"UTF-8");
   i=ins.read(buf);
   ins.close();
   System.out.print("Read from remote file: ");
   for(int j=0;j<i;j++) System.out.write(buf[j]);
   System.out.println();
  }
  catch(Exception e)
  {
   System.out.println("Input failed:"+e);
  }

  // Unload model
  m.unloadModel(mod);

  // Disconnect instance
  m.disconnect();

 }

                /************************************************/
                /* This file manager will direct accesses to    */
		/* files 'bimfile' and 'a.mos' to local objects */
                /* Warning: concurrency not handled here!!!     */
                /************************************************/
 static class FileManager implements XPRDFileManager
 {
  public OutputStream openForWriting(String fname,int mode) throws IOException
  {
   if(fname.equals("outremote"))
   {
    return new OutRemote();
   }
   else
   if(fname.equals("bimfile"))
   {
    return new myByteArrayOutputStream();
   }
   else
    return null;  // name not found: use default behaviour
  }
  public InputStream openForReading(String fname,int mode) throws IOException
  {
   if(fname.equals("bimfile"))
   {
    return new ByteArrayInputStream(bimbuf);
   }
   else
   if(fname.equals("a.mos"))	// 'a.mos' is not a physical file...
   {
    return new ByteArrayInputStream(srcbuf.getBytes("UTF-8"));
   }
   else
    return null;  // name not found: use default behaviour
  }
 }

                /*******************************************/
                /* An extension of 'ByteArrayOutputStream' */
		/* for saving the array on closing.        */
                /*******************************************/
 static class myByteArrayOutputStream extends ByteArrayOutputStream
 {
  public void close()
  {
   bimbuf=toByteArray();
  }
 }

                /***********************************************/
                /* OutputStream class to handle default output */
                /***********************************************/
 static class OutRemote extends OutputStream
 {
  public void write(byte[] b,int off,int len)
  { System.out.print("REMOTE: "); System.out.write(b,off,len); }
  public void write(int b) {}
 }
}

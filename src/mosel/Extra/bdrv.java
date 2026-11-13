/********************************************************/
/*  BinDrv library example                              */
/*  ======================                              */
/*                                                      */
/*  file bdrv.java                                      */
/*  ``````````````                                      */
/* Example of use of BinDrv                             */
/*                                                      */
/*  (c) 2011 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2011                      */
/********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class bdrv
{
 public static void main(String[] args) throws Exception
 {
  ByteBuffer buf=ByteBuffer.allocate(128);

  writeIt();
  readIt();

  writeIt(buf);
  buf.rewind();
  readIt(buf);
 }

 /*****************************/
 /* Create a BinDrv data file */
 /*****************************/
 static void writeIt() throws IOException
 {
  BinDrvWriter bdrv;
  FileOutputStream f;
  byte[] data={1,2,3,4,5,6,7,8};

  f=new FileOutputStream("bindata");
  bdrv=new BinDrvWriter(f);

  bdrv.putControl(bdrv.CTRL_LABEL).put("A");
  bdrv.putControl(bdrv.CTRL_OPENLST);
  bdrv.putControl(bdrv.CTRL_OPENNDX).put(1).putControl(bdrv.CTRL_CLOSENDX);
  bdrv.put(0.45).put(-31.02).put(-123);
  bdrv.putControl(bdrv.CTRL_CLOSELST);
  bdrv.putControl(bdrv.CTRL_LABEL).put("B").put(true);
  bdrv.putControl(bdrv.CTRL_LABEL).put("C").put("sometext");
  bdrv.putControl(bdrv.CTRL_LABEL).put("D").put(Long.MAX_VALUE/2);
  bdrv.putControl(bdrv.CTRL_LABEL).put("E").put(data);
  f.close();
 }

 /***************************************/
 /* Read and display a BinDrv data file */
 /***************************************/
 static void readIt() throws IOException
 {
  BinDrvReader bdrv;
  FileInputStream f;

  f=new FileInputStream("bindata");
  bdrv=new BinDrvReader(f);

  while(bdrv.nextToken()>=0)
  {
   switch(bdrv.nextToken())
   {
    case BinDrvReader.TYP_INT:
	System.out.print(" "+bdrv.getInt());
	break;
    case BinDrvReader.TYP_DATA:
	{
	 byte[] data;
	 data=bdrv.getData();
	 System.out.print(" ");
	 if(data!=null)
	 {
	  for(int i=0;i<data.length;i++)
	   System.out.print(String.format("%02x",data[i]));
	 }
	 break;
	}
    case BinDrvReader.TYP_REAL:
	System.out.print(" "+bdrv.getReal());
	break;
    case BinDrvReader.TYP_STR:
	System.out.print(" "+bdrv.getString());
	break;
    case BinDrvReader.TYP_BOOL:
	System.out.print(" "+bdrv.getBoolean());
	break;
    case BinDrvReader.TYP_CTRL:
	switch(bdrv.getControl())
	{
	 case BinDrvReader.CTRL_SKIP: System.out.print(" *"); break;
	 case BinDrvReader.CTRL_LABEL: System.out.print("\n"+bdrv.getString()+":"); break;
	 case BinDrvReader.CTRL_OPENLST: System.out.print("["); break;
	 case BinDrvReader.CTRL_CLOSELST: System.out.print("]"); break;
	 case BinDrvReader.CTRL_OPENNDX: System.out.print("("); break;
	 case BinDrvReader.CTRL_CLOSENDX: System.out.print(")"); break;
	 case BinDrvReader.CTRL_NIL: System.out.print(" ?"); break;
         default:
	  System.out.println("Unexpected Control");
	  System.exit(0);
	}
	break;
    case BinDrvReader.TYP_LONG:
	System.out.print(" "+bdrv.getLong());
	break;
    default:
	System.out.println("Unexpected token: "+bdrv.nextToken());
	System.exit(0);
   }
  }
  f.close();
  System.out.println("");
 }

 /*******************************/
 /* Create a BinDrv data buffer */
 /*******************************/
 static void writeIt(ByteBuffer buf)
 {
  byte[] data={1,2,3,4,5,6,7,8};

  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_LABEL);
  BinDrvWriter.put(buf,"A");
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_OPENLST);
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_OPENNDX);
  BinDrvWriter.put(buf,1);
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_CLOSENDX);
  BinDrvWriter.put(buf,0.45);
  BinDrvWriter.put(buf,-31.02);
  BinDrvWriter.put(buf,-123);
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_CLOSELST);
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_LABEL);
  BinDrvWriter.put(buf,"B");
  BinDrvWriter.put(buf,true);
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_LABEL);
  BinDrvWriter.put(buf,"C");
  BinDrvWriter.put(buf,"sometext");
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_LABEL);
  BinDrvWriter.put(buf,"D");
  BinDrvWriter.put(buf,Long.MAX_VALUE/2);
  BinDrvWriter.putControl(buf,BinDrvWriter.CTRL_LABEL);
  BinDrvWriter.put(buf,"E");
  BinDrvWriter.put(buf,data);
  buf.limit(buf.position());
 }

 /*****************************************/
 /* Read and display a BinDrv data buffer */
 /*****************************************/
 static void readIt(ByteBuffer buf) throws IOException
 {
  while(BinDrvReader.nextToken(buf)>=0)
  {
   switch(BinDrvReader.nextToken(buf))
   {
    case BinDrvReader.TYP_INT:
	System.out.print(" "+BinDrvReader.getInt(buf));
	break;
    case BinDrvReader.TYP_DATA:
	{
	 byte[] data;

	 data=BinDrvReader.getData(buf);
	 System.out.print(" ");
	 if(data!=null)
	 {
	  for(int i=0;i<data.length;i++)
	   System.out.print(String.format("%02x",data[i]));
	 }
	 break;
	}
    case BinDrvReader.TYP_REAL:
	System.out.print(" "+BinDrvReader.getReal(buf));
	break;
    case BinDrvReader.TYP_STR:
	System.out.print(" "+BinDrvReader.getString(buf));
	break;
    case BinDrvReader.TYP_BOOL:
	System.out.print(" "+BinDrvReader.getBoolean(buf));
	break;
    case BinDrvReader.TYP_CTRL:
	switch(BinDrvReader.getControl(buf))
	{
	 case BinDrvReader.CTRL_SKIP: System.out.print(" *"); break;
	 case BinDrvReader.CTRL_LABEL: System.out.print("\n"+BinDrvReader.getString(buf)+":"); break;
	 case BinDrvReader.CTRL_OPENLST: System.out.print("["); break;
	 case BinDrvReader.CTRL_CLOSELST: System.out.print("]"); break;
	 case BinDrvReader.CTRL_OPENNDX: System.out.print("("); break;
	 case BinDrvReader.CTRL_CLOSENDX: System.out.print(")"); break;
	 case BinDrvReader.CTRL_NIL: System.out.print(" ?"); break;
         default:
	  System.out.println("Unexpected Control");
	  System.exit(0);
	}
	break;
    case BinDrvReader.TYP_LONG:
	System.out.print(" "+BinDrvReader.getLong(buf));
	break;
    default:
	System.out.println("Unexpected token: "+BinDrvReader.nextToken(buf));
	System.exit(0);
   }
  }
  System.out.println("");
 }
}

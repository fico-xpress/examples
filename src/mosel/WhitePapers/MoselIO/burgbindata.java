/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file burgbindata.java
   `````````````````````
   Example of use of BinDrv for reading and writing files.

   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/

import java.io.*;
import java.nio.*;
import com.dashoptimization.*;

public class burgbindata
{                                     // Input data
 static final double[] vdata={15,100,90,60,40,15,10, 1};   // VALUE
 static final double[] wdata={ 2, 20,20,30,40,30,60,10};   // WEIGHT
 static final String[] ind={"camera", "necklace", "vase", "picture", "tv", 
                   "video", "chest", "brick" };            // Index names
 static final int datasize=8;

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;

  writeBurgBin();

  mosel = new XPRM();                       // Initialize Mosel
  mosel.compile("burglar2m.mos");
  try( XPRMModel mod = mosel.loadModel("burglar2m.bim") )
  { mod.run(); }

  readBurgBin();
 }

 /*****************************/
 /* Create a BinDrv data file */
 /*****************************/
 static void writeBurgBin() throws IOException
 {
  BinDrvWriter bdrv;
  FileOutputStream f;

  f=new FileOutputStream("burgdatabin");
  bdrv=new BinDrvWriter(f);

  bdrv.putControl(bdrv.CTRL_LABEL).put("BurgData");      // label:
  bdrv.putControl(bdrv.CTRL_OPENLST);                    // [
  for(int i=0;i<datasize;i++)
  {                                                      //  (index)
   bdrv.putControl(bdrv.CTRL_OPENNDX).put(ind[i]).putControl(bdrv.CTRL_CLOSENDX);
   bdrv.putControl(bdrv.CTRL_OPENLST).put(vdata[i]).put(wdata[i]);  
   bdrv.putControl(bdrv.CTRL_CLOSELST);                 //    [val1 val2]
  }
  bdrv.putControl(bdrv.CTRL_CLOSELST);                  //  ]
  f.close();
 }

 /***************************************/
 /* Read and display a BinDrv data file */
 /***************************************/
 static void readBurgBin() throws IOException
 {
  BinDrvReader bdrv;
  FileInputStream f;
  int c;

  f=new FileInputStream("resdatabin");
  bdrv=new BinDrvReader(f);

  if(bdrv.getControl()==BinDrvReader.CTRL_LABEL)
  {
   while(bdrv.nextToken()>=0)
   {
    String s=bdrv.getString();                            // label:
    if(s.equals("SolTake"))
    {
     System.out.println("Solution values:");
     bdrv.getControl();
     while(bdrv.nextToken()>=0)
     {
      c=bdrv.getControl();                                // [
      if(c==BinDrvReader.CTRL_LABEL) break;
      switch(c)
      {
       case BinDrvReader.CTRL_OPENNDX:                    //  (
          System.out.print(" take("+bdrv.getString()+")="); // index
          bdrv.getControl();                              //  )
          System.out.println(bdrv.getReal());             //   value
          break;
       case BinDrvReader.CTRL_CLOSELST:                   // ]
          System.out.println(""); break;
       default:
          System.out.println("Unexpected Control");
          System.exit(0);
      }
     }
    }
    else if(s.equals("Objective"))
      System.out.println("Objective value = "+bdrv.getReal());
    else
    {
     System.out.println("Unexpected label "+s);
     System.exit(0);
    }
   }
  }
  else
  {
   System.out.println("Unexpected token");
   System.exit(1);
  }

  f.close();
 }
} 

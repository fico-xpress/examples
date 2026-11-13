/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugiosparse.java
   ````````````````````
   Exchanging data between model and host application.
   - Sparse data (string indices) -

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006
********************************************************/

import com.dashoptimization.*;

public class ugiosparse
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

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  MyData data[]={new MyData("camera",15,2), new MyData("necklace",100,20), 
                 new MyData("vase",90,20), new MyData("picture",60,30), 
		 new MyData("tv",40,40), new MyData("video",15,30), 
                 new MyData("chest",10,60), new MyData("brick",1,10)};
  MySol[] solution=new MySol[8];

  for(int i=0;i<8;i++) solution[i] = new MySol();

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("burglar9.mos");      // Compile & load the model
  mod = mosel.loadModel("burglar9.bim");

                        // Associate the Java objects with names in Mosel
  mosel.bind("dt", data);
  mosel.bind("sol", solution);
                        // File names are passed through execution parameters
  mod.execParams = "DATA='dt(ind,val,wght)',SOL='sol(ind,val)'";

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

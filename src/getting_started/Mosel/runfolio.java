/*******************************************************
   Mosel Example Problems
   ====================== 

   file runfolio.java
   ``````````````````
   Running a Mosel model from a Java application
   with data exchange between model and host application.
   (Sparse data format using string indices)

   (c) 2009 Fair Isaac Corporation
       author: S. Heipcke, Jan. 2009
********************************************************/

import com.dashoptimization.*;

public class runfolio
{
                    // Classes to store initial values for data
 public static class MyArrayTwo
 {
  public String ind1,ind2;          // index names
  public boolean val;               // data value
  MyArrayTwo(String i1, String i2, boolean v)
  { ind1=i1; ind2=i2; val=v; }
  MyArrayTwo(String i1, String i2, int v)
  { ind1=i1; ind2=i2; val=(v==1); }
 }
 public static class MyArrayOne
 {
  public String ind;                // index name
  public double val;                // data value
  MyArrayOne(String i, double v)
  { ind=i; val=v; }
 }
                    // Class to receive solution values of decision variables
 public static class MySolArray
 {
  public String ind;                // index name
  public double val;                // solution value
 }
                    // Class with scalar solution information
                    // (scalars cannot be passed directly)
 public static class MySol
 {
  public double objval;             // objective function value
  public int numshares;             // number of shares in solution
  public int status;                // problem status
 }

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
                    // Model parameter settings
  double maxrisk = 1.0/3;
  double minreg = 0.2;
  double maxreg = 0.5;
  double maxsec = 0.25;
  double maxval = 0.2;
  double minval = 0.1;
  int maxnum = 15;
                   // Data for 'initializations from' (input data values)
  String[] riskset = {"hardware1", "hardware2", "hardware3", "hardware4", 
   "hardware5", "hardware6", "hardware7", "hardware8", "hardware9", 
   "hardware10", "theater1", "theater2", "theater3", "theater4", "theater5", 
   "theater6", "theater7", "theater8", "theater9", "theater10", "telecom1",
   "telecom2", "telecom3", "telecom4", "telecom5", "telecom6", "telecom7", 
   "telecom8", "telecom9", "telecom10", "software1", "software2", "software3", 
   "software4", "software5", "software6", "software7", "software8", 
   "software9", "software10", "electronics1", "electronics2", "electronics3", 
   "electronics4", "electronics5", "electronics6", "electronics7", 
   "electronics8", "electronics9", "electronics10"};
  MyArrayTwo locdata[] = {new MyArrayTwo("EU","treasury1",1),
   new MyArrayTwo("EU","treasury10",1), new MyArrayTwo("EU","hardware7",1),
   new MyArrayTwo("EU","hardware9",1), new MyArrayTwo("EU","theater8",1), 
   new MyArrayTwo("EU","theater10",1), new MyArrayTwo("EU","telecom3",1), 
   new MyArrayTwo("EU","telecom9",1), new MyArrayTwo("EU","telecom10",1), 
   new MyArrayTwo("EU","brewery1",1), new MyArrayTwo("EU","brewery4",1), 
   new MyArrayTwo("EU","brewery6",1), new MyArrayTwo("EU","brewery7",1), 
   new MyArrayTwo("EU","brewery8",1), new MyArrayTwo("EU","highways2",1), 
   new MyArrayTwo("EU","highways5",1), new MyArrayTwo("EU","highways7",1), 
   new MyArrayTwo("EU","highways10",1), new MyArrayTwo("EU","cars4",1), 
   new MyArrayTwo("EU","cars7",1), new MyArrayTwo("EU","cars8",1), 
   new MyArrayTwo("EU","cars9",1), new MyArrayTwo("EU","cars10",1), 
   new MyArrayTwo("EU","bank1",1), new MyArrayTwo("EU","bank3",1), 
   new MyArrayTwo("EU","bank5",1), new MyArrayTwo("EU","bank6",1), 
   new MyArrayTwo("EU","bank7",1), new MyArrayTwo("EU","bank10",1),
   new MyArrayTwo("EU","software3",1), new MyArrayTwo("EU","software4",1), 
   new MyArrayTwo("EU","software5",1), new MyArrayTwo("EU","software6",1),
   new MyArrayTwo("EU","electronics2",1), new MyArrayTwo("NA","treasury2",1), 
   new MyArrayTwo("NA","treasury6",1), new MyArrayTwo("NA","treasury7",1),
   new MyArrayTwo("NA","treasury8",1), new MyArrayTwo("NA","hardware4",1), 
   new MyArrayTwo("NA","hardware6",1), new MyArrayTwo("NA","theater1",1),
   new MyArrayTwo("NA","theater3",1), new MyArrayTwo("NA","theater7",1), 
   new MyArrayTwo("NA","theater9",1), new MyArrayTwo("NA","telecom2",1),
   new MyArrayTwo("NA","telecom4",1), new MyArrayTwo("NA","telecom8",1), 
   new MyArrayTwo("NA","highways3",1), new MyArrayTwo("NA","highways4",1),
   new MyArrayTwo("NA","cars5",1), new MyArrayTwo("NA","cars6",1), 
   new MyArrayTwo("NA","bank2",1), new MyArrayTwo("NA","bank4",1), 
   new MyArrayTwo("NA","bank8",1), new MyArrayTwo("NA","bank9",1), 
   new MyArrayTwo("NA","software1",1), new MyArrayTwo("NA","software2",1), 
   new MyArrayTwo("NA","software9",1), new MyArrayTwo("NA","electronics3",1), 
   new MyArrayTwo("NA","electronics5",1), new MyArrayTwo("NA","electronics6",1),
   new MyArrayTwo("NA","electronics8",1), new MyArrayTwo("NA","electronics9",1),
   new MyArrayTwo("NA","electronics10",1), new MyArrayTwo("APAC","treasury4",1),
   new MyArrayTwo("APAC","treasury5",1), new MyArrayTwo("APAC","treasury9",1), 
   new MyArrayTwo("APAC","hardware1",1), new MyArrayTwo("APAC","hardware2",1), 
   new MyArrayTwo("APAC","hardware5",1), new MyArrayTwo("APAC","hardware8",1),  
   new MyArrayTwo("APAC","hardware10",1), new MyArrayTwo("APAC","theater2",1), 
   new MyArrayTwo("APAC","theater4",1), new MyArrayTwo("APAC","telecom1",1),
   new MyArrayTwo("APAC","telecom5",1), new MyArrayTwo("APAC","telecom6",1), 
   new MyArrayTwo("APAC","telecom7",1), new MyArrayTwo("APAC","brewery9",1), 
   new MyArrayTwo("APAC","brewery10",1), new MyArrayTwo("APAC","highways8",1), 
   new MyArrayTwo("APAC","cars1",1), new MyArrayTwo("APAC","cars2",1), 
   new MyArrayTwo("APAC","software7",1), new MyArrayTwo("APAC","software8",1), 
   new MyArrayTwo("APAC","software10",1),
   new MyArrayTwo("APAC","electronics1",1), 
   new MyArrayTwo("APAC","electronics4",1), 
   new MyArrayTwo("APAC","electronics7",1)};
  MyArrayTwo secdata[] = {new MyArrayTwo("bonds","treasury1",1), 
   new MyArrayTwo("bonds","treasury10",1), 
   new MyArrayTwo("bonds","treasury2",1), 
   new MyArrayTwo("bonds","treasury6",1), 
   new MyArrayTwo("bonds","treasury7",1),
   new MyArrayTwo("bonds","treasury8",1), 
   new MyArrayTwo("bonds","treasury4",1),
   new MyArrayTwo("bonds","treasury5",1), 
   new MyArrayTwo("bonds","treasury9",1), 
   new MyArrayTwo("bonds","treasury3",1), 
   new MyArrayTwo("technology","hardware7",1),
   new MyArrayTwo("technology","hardware9",1), 
   new MyArrayTwo("technology","software3",1), 
   new MyArrayTwo("technology","software4",1), 
   new MyArrayTwo("technology","software5",1), 
   new MyArrayTwo("technology","software6",1), 
   new MyArrayTwo("technology","electronics2",1), 
   new MyArrayTwo("technology","hardware4",1), 
   new MyArrayTwo("technology","hardware6",1),
   new MyArrayTwo("technology","software1",1), 
   new MyArrayTwo("technology","software2",1), 
   new MyArrayTwo("technology","software9",1), 
   new MyArrayTwo("technology","electronics3",1), 
   new MyArrayTwo("technology","electronics5",1), 
   new MyArrayTwo("technology","electronics6",1), 
   new MyArrayTwo("technology","electronics8",1), 
   new MyArrayTwo("technology","electronics9",1),
   new MyArrayTwo("technology","electronics10",1), 
   new MyArrayTwo("technology","hardware1",1), 
   new MyArrayTwo("technology","hardware2",1), 
   new MyArrayTwo("technology","hardware5",1), 
   new MyArrayTwo("technology","hardware8",1), 
   new MyArrayTwo("technology","hardware10",1), 
   new MyArrayTwo("technology","software7",1), 
   new MyArrayTwo("technology","software8",1),
   new MyArrayTwo("technology","software10",1), 
   new MyArrayTwo("technology","electronics1",1), 
   new MyArrayTwo("technology","electronics4",1), 
   new MyArrayTwo("technology","electronics7",1), 
   new MyArrayTwo("technology","hardware3",1),
   new MyArrayTwo("entertainment","theater8",1), 
   new MyArrayTwo("entertainment","theater10",1), 
   new MyArrayTwo("entertainment","theater1",1), 
   new MyArrayTwo("entertainment","theater3",1), 
   new MyArrayTwo("entertainment","theater7",1),
   new MyArrayTwo("entertainment","theater9",1), 
   new MyArrayTwo("entertainment","theater2",1), 
   new MyArrayTwo("entertainment","theater4",1), 
   new MyArrayTwo("entertainment","theater5",1), 
   new MyArrayTwo("entertainment","theater6",1),
   new MyArrayTwo("telecom","telecom3",1), 
   new MyArrayTwo("telecom","telecom9",1), 
   new MyArrayTwo("telecom","telecom10",1), 
   new MyArrayTwo("telecom","telecom2",1), 
   new MyArrayTwo("telecom","telecom4",1), 
   new MyArrayTwo("telecom","telecom8",1),
   new MyArrayTwo("telecom","telecom1",1), 
   new MyArrayTwo("telecom","telecom5",1), 
   new MyArrayTwo("telecom","telecom6",1), 
   new MyArrayTwo("telecom","telecom7",1), 
   new MyArrayTwo("food","brewery1",1), new MyArrayTwo("food","brewery4",1), 
   new MyArrayTwo("food","brewery6",1), new MyArrayTwo("food","brewery7",1), 
   new MyArrayTwo("food","brewery8",1), new MyArrayTwo("food","brewery9",1),
   new MyArrayTwo("food","brewery10",1), new MyArrayTwo("food","brewery2",1), 
   new MyArrayTwo("food","brewery3",1), new MyArrayTwo("food","brewery5",1), 
   new MyArrayTwo("construction","highways2",1), 
   new MyArrayTwo("construction","highways5",1),
   new MyArrayTwo("construction","highways7",1), 
   new MyArrayTwo("construction","highways10",1), 
   new MyArrayTwo("construction","highways3",1), 
   new MyArrayTwo("construction","highways4",1), 
   new MyArrayTwo("construction","highways8",1),
   new MyArrayTwo("construction","highways1",1), 
   new MyArrayTwo("construction","highways6",1), 
   new MyArrayTwo("construction","highways9",1), 
   new MyArrayTwo("manufacturing","cars4",1), 
   new MyArrayTwo("manufacturing","cars7",1), 
   new MyArrayTwo("manufacturing","cars8",1), 
   new MyArrayTwo("manufacturing","cars9",1), 
   new MyArrayTwo("manufacturing","cars10",1), 
   new MyArrayTwo("manufacturing","cars5",1), 
   new MyArrayTwo("manufacturing","cars6",1), 
   new MyArrayTwo("manufacturing","cars1",1), 
   new MyArrayTwo("manufacturing","cars2",1), 
   new MyArrayTwo("manufacturing","cars3",1), 
   new MyArrayTwo("finance","bank1",1), new MyArrayTwo("finance","bank3",1),
   new MyArrayTwo("finance","bank5",1), new MyArrayTwo("finance","bank6",1), 
   new MyArrayTwo("finance","bank7",1), new MyArrayTwo("finance","bank10",1), 
   new MyArrayTwo("finance","bank2",1), new MyArrayTwo("finance","bank4",1), 
   new MyArrayTwo("finance","bank8",1), new MyArrayTwo("finance","bank9",1)};
  MyArrayOne retdata[] = {new MyArrayOne("treasury1",5.29),
   new MyArrayOne("treasury2",2.8), new MyArrayOne("treasury3",3.59), 
   new MyArrayOne("treasury4",6.97), new MyArrayOne("treasury5",3.3), 
   new MyArrayOne("treasury6",3.02), new MyArrayOne("treasury7",2.98), 
   new MyArrayOne("treasury8",3.41), new MyArrayOne("treasury9",3.93), 
   new MyArrayOne("treasury10",3.86), new MyArrayOne("hardware1",11.36), 
   new MyArrayOne("hardware2",17.23), new MyArrayOne("hardware3",16.31), 
   new MyArrayOne("hardware4",21.1), new MyArrayOne("hardware5",13.76), 
   new MyArrayOne("hardware6",9.31), new MyArrayOne("hardware7",16.99), 
   new MyArrayOne("hardware8",24.85), new MyArrayOne("hardware9",18.52), 
   new MyArrayOne("hardware10",12.79), new MyArrayOne("theater1",36.1), 
   new MyArrayOne("theater2",19.4), new MyArrayOne("theater3",23.5), 
   new MyArrayOne("theater4",34.61), new MyArrayOne("theater5",16.91), 
   new MyArrayOne("theater6",27.04), new MyArrayOne("theater7",25.82), 
   new MyArrayOne("theater8",24.99), new MyArrayOne("theater9",36.89), 
   new MyArrayOne("theater10",31.71), new MyArrayOne("telecom1",10.89), 
   new MyArrayOne("telecom2",17.98), new MyArrayOne("telecom3",12.31), 
   new MyArrayOne("telecom4",6.53), new MyArrayOne("telecom5",6.11), 
   new MyArrayOne("telecom6",15.89), new MyArrayOne("telecom7",12.46), 
   new MyArrayOne("telecom8",11.11), new MyArrayOne("telecom9",16.6), 
   new MyArrayOne("telecom10",13.93), new MyArrayOne("brewery1",8.79), 
   new MyArrayOne("brewery2",9.35), new MyArrayOne("brewery3",8.88), 
   new MyArrayOne("brewery4",11.63), new MyArrayOne("brewery5",6.21), 
   new MyArrayOne("brewery6",9.26), new MyArrayOne("brewery7",4.77),
   new MyArrayOne("brewery8",5.98), new MyArrayOne("brewery9",10.92), 
   new MyArrayOne("brewery10",5.26), new MyArrayOne("highways1",5.39), 
   new MyArrayOne("highways2",4.85), new MyArrayOne("highways3",8.34), 
   new MyArrayOne("highways4",8.48), new MyArrayOne("highways5",6.42), 
   new MyArrayOne("highways6",11.04), new MyArrayOne("highways7",13.15), 
   new MyArrayOne("highways8",10.01), new MyArrayOne("highways9",7.71), 
   new MyArrayOne("highways10",11.07), new MyArrayOne("cars1",5.74), 
   new MyArrayOne("cars2",7.13), new MyArrayOne("cars3",8.75), 
   new MyArrayOne("cars4",5.08), new MyArrayOne("cars5",6.23), 
   new MyArrayOne("cars6",8.19), new MyArrayOne("cars7",8.03), 
   new MyArrayOne("cars8",5.96), new MyArrayOne("cars9",4.17), 
   new MyArrayOne("cars10",8.11), new MyArrayOne("bank1",6.79), 
   new MyArrayOne("bank2",3.01), new MyArrayOne("bank3",4.92), 
   new MyArrayOne("bank4",4.14), new MyArrayOne("bank5",8.98), 
   new MyArrayOne("bank6",8.81), new MyArrayOne("bank7",4.7), 
   new MyArrayOne("bank8",8.06), new MyArrayOne("bank9",6.22), 
   new MyArrayOne("bank10",4.47), new MyArrayOne("software1",34.59), 
   new MyArrayOne("software2",44.94), new MyArrayOne("software3",43.05), 
   new MyArrayOne("software4",15.58), new MyArrayOne("software5",42.05), 
   new MyArrayOne("software6",20.66), new MyArrayOne("software7",20.76), 
   new MyArrayOne("software8",19.85), new MyArrayOne("software9",20.05), 
   new MyArrayOne("software10",45.38), new MyArrayOne("electronics1",23.07), 
   new MyArrayOne("electronics2",19.1), new MyArrayOne("electronics3",23.83), 
   new MyArrayOne("electronics4",16.54), new MyArrayOne("electronics5",28.33), 
   new MyArrayOne("electronics6",25.88), new MyArrayOne("electronics7",22.14), 
   new MyArrayOne("electronics8",22.65), new MyArrayOne("electronics9",12.59), 
   new MyArrayOne("electronics10",28.1)};

                   // Data structures for 'initializations to' (solution)
  MySol solution = new MySol();
  MySolArray[] solfrac = new MySolArray[maxnum];
  MySolArray[] solbuy = new MySolArray[maxnum]; 
  for(int i=0;i<maxnum;i++)
  { solfrac[i] = new MySolArray(); solbuy[i] = new MySolArray(); }


  try{
   mosel = new XPRM();                // Initialize Mosel
  }catch(XPRMLicenseError e){
                System.out.println("License error" + e.getMessage());
                throw new java.lang.Exception("Error during execution");
  }

  try{
   mosel.compile("foliomemio.mos");   // Compile the model (only required
                                      // during development phase, deployed
				      // application would only use BIM)
  }catch(XPRMCompileException e){
   System.out.println(e.getMessage());
  }

  mod = mosel.loadModel("foliomemio.bim");  // Load the model

                    // Associate the Java objects with names in Mosel
  mosel.bind("risk", riskset);
  mosel.bind("loc", locdata);
  mosel.bind("sec", secdata);
  mosel.bind("ret", retdata);
  mosel.bind("sfrac", solfrac);
  mosel.bind("sbuy", solbuy);
  mosel.bind("sol", solution);

                    // Pass model parameters through execution parameters
  mod.execParams = "MAXRISK=" + maxrisk + ",MINREG=" + minreg + 
                   ",MAXREG=" + maxreg + ",MAXSEC=" + maxsec + ",MAXVAL=" +
                   maxval + ",MINVAL=" + minval + ",MAXNUM=" + maxnum +
                                      // File names (IO driver)
                   ",DATAFILE='jraw:',OUTPUTFILE='jraw:'," +
                                      // Data locations in memory
                   "RISKDATA='risk',LOCDATA='loc(ind1,ind2,val)'," +
                   "RETDATA='ret(ind,val)',SECDATA='sec(ind1,ind2,val)'," +
                   "FRACSOL='sfrac(ind,val)',BUYSOL='sbuy(ind,val)'," +
                   "RETSOL='sol(objval)',NUMSHARES='sol(numshares)'," +
		   "SOLSTATUS='sol(status)'";

  mod.run();                          // Run the model

  if(mod.getExecStatus() != XPRMModel.RT_OK){
   throw new java.lang.Exception("Error during model execution");
  }
  if(mod.getProblemStatus() != mod.PB_OPTIMAL){
   throw new java.lang.Exception("Problem not optimal");
  }                                  // Stop if no solution available

                    // Display solution values obtained from the model
  System.out.println("Total return: " + solution.objval);
// Alternatively:
//  System.out.println("Objective value: " + mod.getObjectiveValue());
  for(int i=0;i<solution.numshares;i++)
   System.out.println(solfrac[i].ind + ": " + solfrac[i].val*100 + "% (" +  solbuy[i].val + ")");

  mod.close();                        // Unload model
 }
}

/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcompfrmem.cs
   ```````````````````
   Retrieve model output via callback-style functionality.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Mar. 2013 
               J.Farmer, Mar. 2021
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugcompfrmem.cs {

  public class ugcompfrmem {
    /// <summary>
    /// String containing the model
    /// </summary>
    const string modelSource=
  "model Burglar\n"+
  "uses 'mmxprs'\n"+
 
  "declarations\n"+
  " WTMAX = 102                    ! Maximum weight allowed\n"+
  " ITEMS = 1..8                   ! Index range for items\n"+
  " VALUE: array(ITEMS) of real    ! Value of items\n"+
  " WEIGHT: array(ITEMS) of real   ! Weight of items\n"+
  " take: array(ITEMS) of mpvar    ! 1 if we take item i; 0 otherwise\n"+
  "end-declarations\n"+

  "VALUE :: [15, 100, 90, 60, 40, 15, 10,  1]\n"+
  "WEIGHT:: [ 2,  20, 20, 30, 40, 30, 60, 10]\n"+

  "! Objective: maximize total value\n"+
  "MaxVal:= sum(i in ITEMS) VALUE(i)*take(i)\n"+

  "! Weight restriction\n"+
  "sum(i in ITEMS) WEIGHT(i)*take(i) <= WTMAX\n"+
  "! All variables are 0/1\n"+
  "forall(i in ITEMS) take(i) is_binary\n"+

  "maximize(MaxVal)                ! Solve the problem\n"+

  "! Print out the solution\n"+
  "writeln(\"Solution:\\n Objective: \", getobjval)\n"+
  "forall(i in ITEMS)  writeln(' take(', i, '): ', getsol(take(i)))\n"+

  "end-model"; 
    
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();

      // Compile the Mosel model to a physical file
      FileStream file= new FileStream("burglar2.bim", FileMode.Create,  
        FileAccess.Write);
      mosel.Compile("", new StringReader(modelSource), file); 
      file.Close();

      // Load the Mosel model
      XPRMModel model = mosel.LoadModel("burglar2.bim");

// Alternative version: compile+load without writing the BIM
//      XPRMModel model = mosel.CompileAndLoad(new StringReader(modelSource));
          
      // Run the model
      model.Run();
    }
  }

}

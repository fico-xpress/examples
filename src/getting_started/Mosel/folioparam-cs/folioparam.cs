/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioparam.cs
  ``````````````````
  Parameterized model execution.

  (c) 2009 Fair Isaac Corporation
      author: J.Farmer, Jun. 2009, rev. May. 2021
********************************************************/

using System;
using System.IO;
using Mosel;

namespace mosel_getting_started {
 public class folioparam
 {
  public static void Main(string[] args)
  {
   XPRM mosel;
   XPRMModel mod;
 
   mosel = XPRM.Init();                       // Initialize Mosel
   mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
                                              // Set Mosel work directory to folder containing our example files
 
   mosel.Compile("foliodata.mos");            // Compile the model
   mod = mosel.LoadModel("foliodata.bim");    // Load compiled model
                                              // Set the run-time parameters
   mod.ExecParams = "OUTFILE=result2.dat,MAXRISK=0.4,MAXVAL=0.25";
   mod.Run();                                 // Run the model
   
   Console.WriteLine("`foliodata' returned: " + mod.Result);
  }
 }
}

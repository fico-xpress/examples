/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioexec.cs
  `````````````````
  Compiling, loading and running a model file.

  (c) 2009 Fair Isaac Corporation
      author: J.Farmer,  Jun. 2009, rev. May. 2021
********************************************************/

using Mosel;
using System;
using System.IO;

namespace mosel_getting_started {
 public class folioexec
 {
  public static void Main(string[] args)
  {
   XPRM mosel;
   XPRMModel mod;
 
   mosel = XPRM.Init();                       // Initialize Mosel
   mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
                                              // Set Mosel work directory to folder containing our example
   
   mosel.Compile("foliodata.mos");            // Compile the model
   mod = mosel.LoadModel("foliodata.bim");    // Load compiled model
   mod.Run();                                 // Run the model
   
   Console.WriteLine("`foliodata' returned: " + mod.Result);
  }
 }

}

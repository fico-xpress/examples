/********************************************************
  Mosel Library Example Problems
  ==============================

  file foliorun.cs
  ````````````````
  Loading and running a BIM file.

  (c) 2009 Fair Isaac Corporation
      author: J.Farmer, Jun. 2009, rev. May. 2021
********************************************************/

using System;
using System.IO;
using Mosel;

namespace mosel_getting_started {
 public class foliorun
 {
  public static void Main(string[] args)
  {
   XPRM mosel;
   XPRMModel mod;
 
   mosel = XPRM.Init();                        // Initialize Mosel
   mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
                                              // Set Mosel work directory to folder containing our example files
 
   mod = mosel.LoadModel("foliodata.bim");     // Load compiled model
   mod.Run();
   
   Console.WriteLine("`foliodata' returned: " + mod.Result);
  }
 }
}

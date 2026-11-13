/********************************************************
  Mosel Library Example Problems
  ==============================

  file foliomat.cs
  ````````````````
  Exporting a matrix.

  (c) 2009 Fair Isaac Corporation
      author: J.Farmer,  Jun. 2009, rev. May. 2021
********************************************************/

using Mosel;
using System.IO;

namespace mosel_getting_started {
 public class foliomat
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

  // Output the LP/MIP problem (or the portion of a problem that is specified 
  // via mpvar+linctr only, ignoring solver-specific extensions such as 
  // indicators or general constraints)
   mod.ExportProblem("p", "folio");
  }
 }
}

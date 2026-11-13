/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexprob.cs                                    */
/*  ````````````````                                    */
/*  Example for the use of the Mosel libraries          */
/*  (accessing problems and solution information)       */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J.Farmer & S. Heipcke                   */
/********************************************************/

using System;
using System.IO;
using Mosel;

namespace mmexprob {
  public class mmexprobClass {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRM mosel;
      XPRMModel mod;
      XPRMArray varr, darr;
      XPRMLinCtr lgrade;

      // Initialize Mosel
      mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      // Compile and load a model file
      mod = mosel.CompileAndLoad("Models/blend2.mos");
      // Run the model (it includes optimization)
      mod.Run();

      // Export the problem to a file in LP format (maximization)
      mod.ExportProblem("p", "blend");

      // Test whether optimal is found
      if (mod.ProblemStatus==XPRMProblemStatus.PB_OPTIMAL)
        Console.WriteLine("Solution is optimal.");

      // Print out the objective function value
      Console.WriteLine("Objective value: {0}", mod.ObjectiveValue);

      // Get the model objects 'x' and 'COST' (both arrays)
      varr = (XPRMArray) mod.FindIdentifier("x");
      darr = (XPRMArray) mod.FindIdentifier("COST");

      // For each entry in array varr, display solution value and
      // corresponding cost
      foreach(int[] indices in varr.Indices) {
        Console.WriteLine(
          "x{0}={1} (COST: {2})",
          varr.IndexToString(indices),
          varr.Get(indices).AsMPVar().Solution,
          darr.GetAsReal(indices)
        );
      }

      // Get the model object 'LoGrade'
      // It must be a reference to a linear constraint
      lgrade = ((XPRMReference) mod.FindIdentifier("LoGrade")).Value.AsLinCtr();
      Console.WriteLine(
        "LoGrade: activity={0}, dual={1}",
        lgrade.Activity,
        lgrade.Dual
      );

      mod.Reset();
    }
  }
}


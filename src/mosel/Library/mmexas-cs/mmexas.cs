/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexas.cs                                      */
/*  ``````````````                                      */
/*  Example for the use of the Mosel libraries          */
/*  (using arrays with index sets: different ways       */
/*   of enumerating arrays)                             */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J.Farmer & S. Heipcke                   */
/********************************************************/


using System;
using System.IO;
using Mosel;

namespace mmexas {
  public class mmexasClass {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRM mosel;
      XPRMModel mod;
      XPRMArray varr;
      XPRMSet[] sets;
      XPRMValue[] vindex;
      int dim;

      // Initialise Mosel
      mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      // Compile and load a model from a file
      mod = mosel.CompileAndLoad("Models/trans.mos");
      // Run the model;
      mod.Run();


      // Get the model object named 'x'
      // It must be an array
      varr = (XPRMArray) mod.FindIdentifier("x");
      // Get the number of dimensions of the array
      dim = varr.Dim;
      // Get the indexing sets
      sets = varr.IndexSets;

      // We could use varr.ToString() to print out a summary of the
      // array contents, but instead we'll demonstrate how to iterate
      // over the array contents directly

      Console.WriteLine("\n1. Logic entries:");
      // Get the first entry of varr
      foreach(int[] indices in varr.Indices) {
        Console.Write("x(");
        // Get the values for this index
        vindex = varr.DereferenceIndex(indices);
        // Now, output them.  Note that we could call the utility method
        // varr.IndexToString(indices) instead of this and the previous
        // line
        for (int i=0;i<dim-1;i++)
          Console.Write(vindex[i] + ",");
        Console.Write(vindex[dim-1] + "), ");
      };

      // Now enumerate over the true entries - in a dense array this would
      // be no different from the previous method, so skip it in this case.
      if (varr.IsDynamic) {
        Console.WriteLine("\n\n2. True entries:");
        foreach(int[] indices in varr.TEIndices) {
          Console.Write("x(");
          vindex = varr.DereferenceIndex(indices);
          for (int i=0;i<dim-1;i++)
            Console.Write(vindex[i] + ",");
          Console.Write(vindex[dim-1] + "), ");
        };
      }
      Console.WriteLine();

      mod.Reset();
    }
  }
 }


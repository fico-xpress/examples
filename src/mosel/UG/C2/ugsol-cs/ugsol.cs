/********************************************************
   Mosel User Guide Example Problems
   ================================= 


   file ugsol.cs
   `````````````
   Accessing modeling objects and solution information.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugsol.cs {

  public class ugsol {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static int Main(string[] args) {
      XPRMArray varr, darr;
      XPRMSet set;
      XPRMMPVar x;
      double val;

      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile and load a model 
      XPRMModel model = mosel.CompileAndLoad("burglar3.mos");

      // Run the model
      model.Run();

      if(model.ProblemStatus!=XPRMProblemStatus.PB_OPTIMAL) 
        return 1;                    // Stop if no solution found

      Console.WriteLine("Objective value: " + model.ObjectiveValue);
                                     // Print the objective function value

      // Get model object 'take', it must be an array
      varr=(XPRMArray)model.FindIdentifier("take");  
      // Get model object 'VALUE', it must be an array
      darr=(XPRMArray)model.FindIdentifier("VALUE");
      // Get model object 'ITEMS', it must be a set
      set=(XPRMSet)model.FindIdentifier("ITEMS");

      // Enumerate all entries of 'varr' (dense array)
      foreach(int[] indices in varr.Indices)
      {
	x = varr.Get(indices).AsMPVar();   // Get a variable from varr
        val = darr.GetAsReal(indices);     // Get the corresponding value
//        Console.WriteLine("take(" + set.GetAsString(indices[0]) + "): " +
        Console.WriteLine("take" + varr.IndexToString(indices) + ": " +
	                x.Solution + "\t (item value: " + val + ")");
      }

      model.Reset();                         // Reset the model
      return 0;
    }
  }
}

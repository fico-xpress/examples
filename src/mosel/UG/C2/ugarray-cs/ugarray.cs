/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugarray.cs
   ```````````````
   Accessing modeling objects (sparse arrays).
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
               J.Farmer, May. 2021
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugarray.cs {

  public class ugarray {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRMArray varr;
      XPRMSet[] sets;
      XPRMValue[] vindex;
      int dim;

      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
                                              
      // Compile and load a model 
      XPRMModel model = mosel.CompileAndLoad("transport.mos");

      // Run the model
      model.Run();

      // Get model object 'flow', it must be an array
      varr=(XPRMArray)model.FindIdentifier("flow");
      dim = varr.Dim;           // Get the number of dimensions of the array
      sets = varr.IndexSets;    // Get the indexing sets

      // Enumerate over the true entries
      foreach(int[] indices in varr.TEIndices)
      {
        // Get the values for this index
        vindex = varr.DereferenceIndex(indices);
        Console.Write("flow(");
        for(int i=0;i<dim-1;i++)
          Console.Write(vindex[i] + ",");
        Console.Write(vindex[dim-1] + "), ");

        // Alternative printing method:
        // Console.Write("flow" + varr.IndexToString(indices) + ", ");
      }  
      Console.WriteLine();

      model.Reset();                         // Reset the model 
    }
  }
}

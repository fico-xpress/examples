/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugparam.cs
   ```````````````
   Passing parameters to a Mosel program
   Accessing modeling objects (sets).
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugparam.cs {

  public class ugparam {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRMSet set;
      int LIM=500, first, last;

      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile and load a model 
      XPRMModel model = mosel.CompileAndLoad("prime.mos");

      // Run the model
      model.ExecParams = "LIMIT=" + LIM;
      model.Run();
      Console.WriteLine("`prime' returned: " + model.Result);

      // Get model object 'SPrime', it must be a set
      set=(XPRMSet)model.FindIdentifier("SPrime");                              

      // Enumerate the set elements
      if (!set.IsEmpty)
      {
        first = set.FirstIndex;         // Get the number of the first index
        last = set.LastIndex;           // Get the number of the last index
        Console.WriteLine("Prime numbers from 2 to " + LIM);
        for (int i=first;i<=last;i++)   // Print all set elements
          Console.Write(" {0},", set.GetAsInteger(i));
        Console.WriteLine();
      }
    }
  }
}

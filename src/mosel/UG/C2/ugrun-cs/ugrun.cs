/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugrun.cs
   `````````````
   Compile+load, then run a model.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugrun.cs {

  public class ugrun {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile and load a model 
      XPRMModel model = mosel.CompileAndLoad("burglar2.mos");

      // Run the model
      model.Run();

      Console.WriteLine("`burglar2' returned: " + model.Result);
    }
  }

}

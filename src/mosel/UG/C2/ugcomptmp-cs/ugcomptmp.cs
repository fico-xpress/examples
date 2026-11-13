/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcomptmp.cs
   `````````````````
   Compiling a model into a BIM file saved in Mosel's
   temporary directory, then load and run it.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugcomptmp.cs {

  public class ugcomptmp {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile the Mosel model, save the BIM file in Mosel's temp. dir.
      mosel.Compile("", "burglar2.mos", "tmp:burglar2.bim");

      // Load the BIM file
      XPRMModel model = mosel.LoadModel("tmp:burglar2.bim");

      // Run the model
      model.Run();
    }
  }

}

/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcompmem.cs
   `````````````````
   Compiling a model to memory.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Mar. 2013 
               J.Farmer, Jul. 2019, rev. May. 2021
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugcompmem.cs {

  public class ugcompmem {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example files
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile the Mosel model to a MemoryStream
      MemoryStream bimStream = new MemoryStream();
      mosel.Compile("",mosel.WorkDir+"/burglar2.mos",bimStream);

      // Reset stream pointer to beginning so we can read the bytes we've just written
      bimStream.Seek(0,SeekOrigin.Begin);

      // Load the Mosel model
      XPRMModel model;
      mosel.Bind("bimblk", bimStream);
      try {
        model = mosel.LoadModel("dotnet:bimblk");
      } finally {
        mosel.Unbind("bimblk");
      }
      
      // Run the model
      model.Run();
    }
  }

}

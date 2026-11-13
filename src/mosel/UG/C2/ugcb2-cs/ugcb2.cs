/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcb.cs
   ````````````
   Retrieve model output via callback-style functionality.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Mar. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugcb.cs {

  public class ugcb {
    
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Redirect error stream to stdout
      mosel.SetDefaultStream(XPRMStreamType.F_ERROR, Console.Out);

      // Compile and load the Mosel model
      XPRMModel model = mosel.CompileAndLoad("burglar2.mos");
      
      // Collect the model's output into a string
      StringWriter modelOut = new StringWriter();
      model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, modelOut);
    
      // Run the model
      model.Run();

      // Print the output 
      string modelOutText = modelOut.ToString();
      Console.WriteLine("Model output: {0}", modelOutText);
    }
  }

}

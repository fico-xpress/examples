/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugdefstream.cs
   ```````````````````
   Redirecting model output.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugdefstream.cs {

  public class ugdefstream {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile and load the Mosel model 
      XPRMModel model = mosel.CompileAndLoad("burglar2.mos");

 // Redirect the model output. Uncomment one of the following lines:
      // Disable model output
 //     model.SetDefaultStream(XPRMStreamType.F_OUTPUT, "null:");
      // Double model output (file+stdout)
      model.SetDefaultStream(XPRMStreamType.F_OUTPUT, "tee:burglog.txt&");

      // Run the model
      model.Run();

      Console.WriteLine("'burglar2' returned: {0}", model.Result);
    }
  }

}

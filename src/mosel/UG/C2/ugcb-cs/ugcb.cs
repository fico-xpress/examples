/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcb.cs
   ````````````
   Retrieve model output via callback-style functionality.
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, J.Farmer, Mar. 2013, rev. May. 2021
********************************************************/


using System;
using System.IO;
using System.Text;
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

      // Associate .NET object with a name in Mosel
      mosel.Bind("mycb", new MyOut());

      // Redirect error stream to stdout
      mosel.SetDefaultStream(XPRMStreamType.F_ERROR, Console.Out);

      // Compile and load the Mosel model
      XPRMModel model = mosel.CompileAndLoad("burglar2.mos");
      
      // Redirect the model's output to a custom TextWriter
      MyOut modelOut = new MyOut();
      model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, modelOut);

      // Alternative:
      // Redirect the model's output to our printing function 'cbmsg'
      model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, "dotnet:mycb");

      // Run the model
      model.Run();

    }
  }

  public class MyOut: TextWriter
  {
   private bool atStartOfLine = true;
   public override void Write(char b)
   {
     if (atStartOfLine) {
      Console.Write("Mosel: ");
      atStartOfLine=false;
     }
     if (b=='\n') {
      Console.WriteLine();
      atStartOfLine=true;
     }
     else if (b=='\r') {
      // ignore 
     }
     else {
      Console.Write(b);
     }
   }
   public override Encoding Encoding {
    get {
     return Encoding.UTF8;
    }
   }
  }
}

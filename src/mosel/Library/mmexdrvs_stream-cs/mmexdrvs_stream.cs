/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexdrvs_stream.cs                             */
/*  ```````````````````````                             */
/*  Example for the use of the Mosel libraries          */
/*  (using 'dotnet' IOdriver for data exchange)         */
/*                                                      */
/*  (c) 2010 Fair Isaac Corporation                     */
/*      author: J. Farmer                               */
/********************************************************/


using System;
using System.IO;
using Mosel;


namespace mmexdrvs_stream.cs {

  public class mmexdrvs_stream_Class {
    /// <summary>
    /// String containing the model
    /// </summary>
    const string BlendMos = 
      "model Blend\n" +
      "uses \"mmxprs\"\n" +
      "public declarations\n" +
      " ROres = 1..2\n" +
      " REV = 125\n" +
      " MINGRADE = 4\n" +
      " MAXGRADE = 5\n" +
      " COST: array(ROres) of real\n" +
      " AVAIL: array(ROres) of real\n" +
      " GRADE: array(ROres) of real\n" + 
      " x: array(ROres) of mpvar\n" +
      "end-declarations\n" +
      "\n" +
      "initializations from 'dotnet:BlendIni'\n" +
      " COST\n" +
      " AVAIL\n" + 
      " GRADE\n" +
      "end-initializations\n" +
      "\n" +
      "Profit:= sum(o in ROres) (REV-COST(o))*x(o)\n" +
      "LoGrade:= sum(o in ROres) (GRADE(o)-MINGRADE) * x(o) >= 0\n" +
      "UpGrade:= sum(o in ROres) (MAXGRADE-GRADE(o)) * x(o) >= 0\n" +
      "\n" +
      "forall(o in ROres) x(o)<=AVAIL(o)\n" +
      "\n" +
      "maximize(Profit)\n" +
      "writeln(\"Objective:\", getobjval)\n" +
      "end-model";
    
    /// <summary>
    /// String containing initialization data for the model
    /// </summary>
    const string BlendDat =
      "COST: [85 93]\n" +
      "AVAIL: [60 45]\n" +
      "GRADE: [2.1 6.3]\n";
  
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      
      // Use a StringReader to compile and load the Mosel model directly from a .NET string
      XPRMModel model = mosel.CompileAndLoad(new StringReader(BlendMos));
      // Bind a stream based on the BlendDat data to the name 'BlendIni' where the model
      // will expect to find its initialization data
      model.Bind("BlendIni", new StringReader(BlendDat));
      
      // Collect the model's output into a string
      StringWriter modelOut = new StringWriter();
      model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, modelOut);
      
      // Run the model
      model.Run();
      
      // Print the output 
      string modelOutText = modelOut.ToString();
      Console.WriteLine("The model's output was: {0}", modelOutText);
    }
  }

}

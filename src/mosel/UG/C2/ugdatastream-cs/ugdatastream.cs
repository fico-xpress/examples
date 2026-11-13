/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugdatastream.cs
   ````````````````````
   Example for the use of the Mosel libraries
   (using 'dotnet' IO driver for data exchange)

   (c) 2012 Fair Isaac Corporation
       author: S.Heipcke, Oct. 2012   
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugdatastream.cs {

  public class ugdatastream_Class {
    
    /// <summary>
    /// String containing initialization data for the model
    /// </summary>
    const string BurglarDat =
      "DATA: [(\"camera\") [15 2] (\"necklace\") [100 20] (\"vase\") [90 20]" + 
      "(\"picture\") [60 30] (\"tv\") [40 40] (\"video\") [15 30] " + 
      "(\"chest\") [10 60] (\"brick\") [1 10] ]\n";
  
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
      XPRMModel model = mosel.CompileAndLoad("burglar13.mos");
      
      // Bind a stream based on the BurglarDat data to the name 'BurglarIni' 
      // where the model will expect to find its initialization data
      model.Bind("BurglarIni", new StringReader(BurglarDat));

      // Pass data location as a parameter to the model 
      model.SetExecParam("DATAFILE","dotnet:BurglarIni");
      
      // Run the model
      model.Run();
    }
  }

}

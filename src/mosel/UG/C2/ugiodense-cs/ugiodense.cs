/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugiodense.cs
   `````````````````
   Exchanging data between model and host application.
   - Dense data -
   
   (c) 2012 Fair Isaac Corporation
       author: S.Heipcke, Oct. 2012   
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugiodense.cs {

  public class ugiodense {
    
    /// <summary>
    /// Arrays containing initialization data for the model
    /// </summary>
    static double[] vdata = new double[] {15,100,90,60,40,15,10, 1};  // VALUE
    static double[] wdata = new double[] { 2, 20,20,30,40,30,60,10};  // WEIGHT

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
      XPRMModel model = mosel.CompileAndLoad("burglar8d.mos");
      
                        // Associate the .NET objects with names in Mosel
      model.Bind("vdat", vdata);
      model.Bind("wdat", wdata);

      // Create a new array for solution data and bind that to the name 'SOL'
      double[] solution = new double[8];
      mosel.Bind("sol", solution);

      // Pass data location as a parameter to the model 
      model.ExecParams = "VDATA='noindex,vdat',WDATA='noindex,wdat',SOL='noindex,sol'";
      
      // Run the model
      model.Run();

      // Print the solution
      Console.WriteLine("Objective value: {0}", model.ObjectiveValue);
      for (int i=0;i<8;i++)
        Console.Write(" take({0}): {1}", (i+1), solution[i]);
      Console.WriteLine();
    }
  }

}

/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugioscalar.cs
   ``````````````````
   Exchanging data between model and host application.
   - Scalars -
   
   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Apr. 2013 
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugioscalar.cs {

  public class ugioscalar {
    /// <summary>
    /// Structure to receive solution values
    /// </summary>
    class MyData {
      public int wmax;
      public int numitem;
      public double objval;
    }

    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static int Main(string[] args) {
      MyData data=new MyData();
      data.wmax=100;
      
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;

      // Compile and load a model 
      XPRMModel model = mosel.CompileAndLoad("burglar11.mos");

      // Associate the .NET object with a name in Mosel
      model.Bind("data", data);

      // Run the model, passing data location as parameters
      model.ExecParams =
        "WMAX='data(wmax)',NUM='data(numitem)',SOLVAL='data(objval)'," +
	"IODRV='dotnetraw:'";
      model.Run();

      if(model.ProblemStatus!=XPRMProblemStatus.PB_OPTIMAL) 
        return 1;                    // Stop if no solution found

                        // Display solution values obtained from the model
      Console.WriteLine("Objective value: " + data.objval);
      Console.WriteLine("Total number of items: " + data.numitem);
      
      model.Reset();                             // Reset the model 
      return 0;
    }
  }
}

/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugiocb.cs
   ``````````````
   Example for the use of the Mosel libraries
   (using 'dotnet' IO driver for data exchange)

   (c) 2013 Fair Isaac Corporation
       author: S.Heipcke, Mar. 2013  
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugiocb.cs {

  public class ugiocb {
    /// <summary>
    /// Arrays containing initialization data for the model
    /// </summary>
    static double[] vdata=new double[] {15,100,90,60,40,15,10, 1};  // VALUE
    static double[] wdata=new double[] { 2, 20,20,30,40,30,60,10};  // WEIGHT
    static string[] ind=new string[] {"camera", "necklace", "vase", "picture",
                  "tv", "video", "chest", "brick"};            // Index names
    static int datasize=8;

    /// <summary>
    /// Structure to receive solution values
    /// </summary>
    class MySol {
      public string ind;                // index name
      public double val;                // solution value
    }
    static MySol[] solution;
    static int solsize;

    /// <summary>
    /// A function to initialize the Mosel data-structures via callback
    /// </summary>
    public static bool initializeFrom(XPRMInitializeContext ictx,string label,XPRMTyped type)
    {
      try {      
        switch (label) {
          case "DATA":
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=0;i<datasize;i++) {
              ictx.Send(XPRMInitializeControl.OpenIndices);
               ictx.Send(ind[i]);
              ictx.Send(XPRMInitializeControl.CloseIndices);
	      ictx.Send(XPRMInitializeControl.OpenList);
                ictx.Send(vdata[i]);
                ictx.Send(wdata[i]);
	      ictx.Send(XPRMInitializeControl.CloseList);
            }
            ictx.Send(XPRMInitializeControl.CloseList);
            return true;
          default:
            Console.WriteLine("Label '{0}' not found", label);
            return false;
        }
      } catch (Exception e) {
        Console.WriteLine("Label '{0}' could not be initialized - {1}", label, e.Message);
        return false;
      }
    }
    

    /// <summary>
    /// A method to retrieve data from Mosel
    /// </summary>
    public static bool initializeTo(string label,XPRMValue val) {
//      Console.WriteLine(".NET: {0} = {1}", label, val);

      XPRMArray solarr;
      XPRMValue[] vindex;

      switch (label) {
        case "SOL":
          solarr=(XPRMArray)val;
          solsize=solarr.Size;
          solution = new MySol[solsize];
          for(int i=0;i<solsize;i++) solution[i] = new MySol();

          int ct=0;
          // Enumerate solarr as sparse array
          foreach(int [] indices in solarr.TEIndices) {
            vindex = solarr.DereferenceIndex(indices);
            solution[ct].ind = vindex[0].AsString();
            solution[ct].val = solarr.GetAsReal(indices);
            ct++;
          }
          return true;
        default:
           Console.WriteLine("Unknown output data item: '{0}'={1} not found", label, val);
           return false;
      }
    } 
  

    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static int Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      
      // Compile and load the Mosel model
      XPRMModel model = mosel.CompileAndLoad("burglar13.mos");
      
      // Set the execution parameters and bind the variables
      model.SetExecParam("DATAFILE","dotnet:cbinitfrom");
      model.SetExecParam("SOLFILE","dotnet:cbinitto");
      model.Bind("cbinitfrom", new XPRMInitializationFrom(initializeFrom));
      model.Bind("cbinitto", new XPRMInitializationTo(initializeTo));

      // Run the model
      model.Run();
 
      if(model.ProblemStatus!=XPRMProblemStatus.PB_OPTIMAL) 
        return 1;                      // Stop if no solution found

     // Display solution values obtained from the model
      Console.WriteLine("Objective value: {0}", model.ObjectiveValue);
      for(int i=0;i<solsize;i++)
        Console.WriteLine(" take({0}): {1}", solution[i].ind, solution[i].val); 

      model.Reset();                   // Reset the model
      return 0;
    }
  }
}

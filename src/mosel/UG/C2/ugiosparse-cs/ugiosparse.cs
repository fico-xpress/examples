/********************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugiosparse.cs
   ``````````````````
   Exchanging data between model and host application.
   - Sparse data -
   
   (c) 2012 Fair Isaac Corporation
       author: S.Heipcke, Oct. 2012   
********************************************************/


using System;
using System.IO;
using Mosel;


namespace ugiosparse.cs {

  public class ugiosparse {
    
    /// <summary>
    /// Arrays containing initialization data for the model
    /// </summary>
    static double[] vdata = new double[] {15,100,90,60,40,15,10, 1};  // VALUE
    static double[] wdata = new double[] { 2, 20,20,30,40,30,60,10};  // WEIGHT

    /// <summary>
    /// Structure to store initial values for the array 'data'
    /// </summary>
    class MyData {
      public string ind;
      public double val;
      public double wght;
      
      public MyData(string i, double v, double w) {
        this.ind = i;
        this.val = v;
        this.wght = w;
      }
    }

    /// <summary>
    /// Structure to receive solution values
    /// </summary>
    class MySol {
      public string ind;
      public double val;
    }

    /// <summary>
    /// The initial values for the array 'data'
    /// </summary>
    private static MyData[] data = new MyData[] {
                 new MyData("camera",15,2), new MyData("necklace",100,20), 
                 new MyData("vase",90,20), new MyData("picture",60,30), 
		 new MyData("tv",40,40), new MyData("video",15,30), 
                 new MyData("chest",10,60), new MyData("brick",1,10) };


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
      XPRMModel model = mosel.CompileAndLoad("burglar9d.mos");
      
      // Associate the .NET object with a name in Mosel
      model.Bind("dt", data);

      // Create a new array for solution data and bind that to the name 'SOL'
      MySol[] solution=new MySol[8];
      for(int i=0;i<8;i++) solution[i] = new MySol();
      mosel.Bind("sol", solution);

      // Pass data location as a parameter to the model 
      model.ExecParams = "DATA='dt(ind,val,wght)',SOL='sol(ind,val)'";
      
      // Run the model
      model.Run();

      // Print the solution
      Console.WriteLine("Objective value: {0}", model.ObjectiveValue);
      for (int i=0;i<8;i++)
        Console.Write(" take({0}): {1}",  solution[i].ind, solution[i].val);
      Console.WriteLine();
    }
  }

}

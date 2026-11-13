/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexdrvs_raw.cs                                */
/*  ````````````````````                                */
/*  Example for the use of the Mosel libraries          */
/*  (using 'dotnetraw' IOdriver for data exchange)      */
/*                                                      */
/*  (c) 2009 Fair Isaac Corporation                     */
/*      author: J. Farmer                               */
/********************************************************/


using System;
using System.IO;
using Mosel;


namespace mmexdrvs_raw.cs {

  public class mmexdrvs_raw_Class {
    /// <summary>
    /// String containing the model
    /// </summary>
    const string ModelSourceCode = 
      "model drivers\n" +

      "parameters\n" +
      " DATA=''\n" +
      " SOL=''\n" +
      "end-parameters\n" +

      "public declarations\n" +
      " S:set of string\n" +
      " R:range\n" +
      " data:array(S,R) of real\n" +
      " sol:array(1..10) of real\n" +
      "end-declarations\n" +

      "initialisations from 'dotnetraw:'\n" +
      " data as 'DATA(s,r,v)'\n" +
      "end-initialisations\n" +

      "writeln('set   S=',S)\n" +
      "writeln('range R=',R)\n" +
      "writeln('array data=',data)\n" +

      "forall(i in 1..10) sol(i):=i^2\n" +

      "initialisations to 'dotnetraw:noindex'\n" +
      " sol as 'SOL'\n" +
      "end-initialisations\n" +

      "end-model"; 


    /// <summary>
    /// Structure to store initial values for the array 'data'
    /// </summary>
    class ModelDataElement {
      public string s;
      public int r;
      public double v;
      
      public ModelDataElement(string s, int r, double v) {
        this.s = s;
        this.r = r;
        this.v = v;
      }
    }

    /// <summary>
    /// The initial values for the array 'data'
    /// </summary>
    private static ModelDataElement[] ModelData = new ModelDataElement[] {
      new ModelDataElement( "one"    , 2  , 12.5 ),
      new ModelDataElement( "two"    , 1  , 15   ),
      new ModelDataElement( "three"  , 16 , 9    ),
      new ModelDataElement(" hundred", 2  , 17   )
    };

 
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      // Initialize Mosel
      XPRM mosel = XPRM.Init();
      
      // Use a StringReader to compile and load the Mosel model directly from a .NET string
      XPRMModel model = mosel.CompileAndLoad(new StringReader(ModelSourceCode));
      // Bind the initializaton data to the name "DATA" which the dotnetraw: driver
      // will find.
      mosel.Bind("DATA", ModelData);
      // Create a new array for solution data and bind that to the name 'SOL'
      double[] solution = new double[10];
      mosel.Bind("SOL", solution);
      
      // Collect the model's output into a string
      StringWriter modelOut = new StringWriter();
      model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, modelOut);
      
      // Run the model
      model.Run();
    
      // Print the solution
      Console.Write("Solution values:");
      for (int i=0;i<10;i++)
        Console.Write(" {0}", solution[i]);
      Console.WriteLine();
      Console.WriteLine();

      // Print the output 
      string modelOutText = modelOut.ToString();
      if (modelOutText.Length>0)
        Console.WriteLine("The model's output was:\n{0}", modelOutText);
      
    }
  }

}

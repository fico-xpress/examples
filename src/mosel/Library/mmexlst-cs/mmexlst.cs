/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexlst.cs                                     */
/*  ```````````````                                     */
/*  Accessing modeling objects                          */
/* (enumerating the elements of a list)                 */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J. Farmer                               */
/********************************************************/


using System;
using System.IO;
using Mosel;


namespace mmexlst {

  /// <summary>
  /// Records example
  /// </summary>
  class mmexlstClass {

    [STAThread]
    static void Main(string[] args) {
      
      /* Initialize Mosel */
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      
      /* Compile & load the model */
      mosel.Compile("Models/euler.mos");
      XPRMModel mod = mosel.CompileAndLoad("Models/euler.mos");
      
      /* Run the model */
      mod.Run();
      
      /* Get the model object named 'TOUR' */
      XPRMList lst = (XPRMList) mod.FindIdentifier("TOUR");
      
      /* Print out all the list elements */
      Console.Write("Tour: ");
      int count=0;
      foreach (XPRMValue val in lst) {
        Console.Write(val.AsInteger());
        if ((++count)<lst.Count)
          Console.Write(" -> ");
      }
      Console.WriteLine();

      
    }


  }

}
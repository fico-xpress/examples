/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexset.cs                                     */
/*  ```````````````                                     */
/*  Example for the use of the Mosel libraries          */
/*  (accessing sets in Mosel)                           */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J. Farmer & S. Heipcke                  */
/********************************************************/

using System;
using System.IO;
using Mosel;

namespace mmexset {
  public class mmexsetClass {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRM mosel;
      XPRMModel mod;
      XPRMSet set;
      int first, last;

      // Initialise Mosel
      mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      /// Load a BIM file
      mod = mosel.CompileAndLoad("Models/burglari.mos");
      // Ru the model
      mod.Run();

      // Get the model object named 'ITEMS'
      // (it must be a set)
      set = (XPRMSet) mod.FindIdentifier("ITEMS");

      if (!set.IsEmpty) {
        // Items in a set are indexed by numbers
        // So get the number of the first and last indexes
        first = set.FirstIndex;
        last = set.LastIndex;
        Console.WriteLine("Elements of set ITEMS:");
        for (int i=first;i<=last;i++)
          Console.Write(" {0}, ", set.GetAsString(i));
        Console.WriteLine();
      }
      // We've written this explicitely to demonstrate set access, but the set
      // actually knows how to output itself.  Uncomment the following line to
      // see how it does this.
      // Console.WriteLine(set);

      if (set.GetIndex("CD player")<0)
        Console.WriteLine("'CD player' is not contained in 'ITEMS',");
    }
  }
}

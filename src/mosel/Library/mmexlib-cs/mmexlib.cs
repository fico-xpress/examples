/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexlib.cs                                     */
/*  ```````````````                                     */
/*  Example for the use of the Mosel libraries          */
/*  (working with models and accessing the dynamic      */
/*  libraries loaded by Mosel)                          */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J.Farmer / S. Heipcke                   */
/********************************************************/

using System;
using System.IO;
using Mosel;

namespace mmexlib {
  public class mmexlibClass {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args)
    {
      XPRM mosel;
      XPRMModel[] mods = new XPRMModel[3];

      // Initialise Mosel
      mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      // Load the BIM files
      mods[0] = mosel.CompileAndLoad("Models/burglari.mos");
      mods[1] = mosel.CompileAndLoad("Models/chess2.mos");
      mods[2] = mosel.CompileAndLoad("Models/trans.mos");
      Console.WriteLine("Models loaded");

      // Display basic information about the models
      foreach (XPRMModel m in mods) {
        Console.WriteLine(
          "  {0}: {1} ({2}, `{3}' size:{4})",
          m.Number,
          m.Name,
          m.SysComment,
          m.UserComment,
          m.Size
        );
      }
      Console.WriteLine();

      // Enumerate all loaded modules and display information
      Console.WriteLine("Additional libraries loaded:");
      foreach (XPRMModule mo in mosel.Modules) {
        Console.WriteLine(
          "  {0} (version {1}) used by {2} model(s)",
          mo.Name,
          mo.Version,
          mo.NumberOfReferences
        );
      }
    }

  }
}


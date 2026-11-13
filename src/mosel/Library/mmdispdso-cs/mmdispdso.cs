/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmdispdso.cs                                   */
/*  `````````````````                                   */
/*  Example for the use of the Mosel libraries          */
/*  (display the contents of a module)                  */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J.Farmer & Y. Colombani                 */
/********************************************************/

using System;
using Mosel;

namespace mmdispdso {
  public class mmdispdsoClass {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRM mosel;
      XPRMModule module;

      if (args.Length!=1) {
        Console.WriteLine("Usage: mmdispdso modulename");
        return;
      }

      // Initialize Mosel
      mosel = XPRM.Init();
      // Load the module
      module = mosel.LoadModule(args[0]);
      // And output basic information
      Console.WriteLine(
        "Module `{0}' version {1}",
        module.Name,
        module.Version
      );
      if (module.Certificate!=null && module.Certificate.Length>0)
        Console.WriteLine(" ({0})", module.Certificate);
      else
        Console.WriteLine();

      Console.WriteLine();  // spacing

      // Output a list of defined constants
      Console.WriteLine("Constants:");
      foreach (XPRMConstant p in module.Constants) {
        Console.WriteLine(" {0}={1}", p.Name, p.Value);
      }
      Console.WriteLine();  // spacing

      // Output a list of types defined within the module
      Console.WriteLine("Types:");
      foreach (XPRMNativeType nt in module.Types) {
        Console.Write(" {0} (", nt.Name);
        if (nt.HasCreate) Console.Write("create");
        if (nt.HasDelete) Console.Write(",delete");
        if (nt.HasToString) Console.Write(",tostring");
        if (nt.HasPRTBL) Console.Write("+");
        if (nt.HasFromString) Console.Write(",fromstring");
        Console.WriteLine(")");
      }
      Console.WriteLine();  // spacing

      // List of control parameters
      Console.WriteLine("Control Parameters:");
      foreach (XPRMParameter p in module.Parameters) {
        Console.Write(" {0}: {1} (", p.Name, p.TypeName);
        if (p.Description!=null && p.Description.Length>0)
          Console.Write("{0},", p.Description);
        Console.WriteLine(rwstatus(p) + ")");
      }
      Console.WriteLine();  // spacing

      // List of subroutines
      Console.WriteLine("Procedures and Functions:");
      foreach (XPRMProcedure p in module.Procedures) {
        dispProcFct(p);
      }
      Console.WriteLine();  // spacing

      // List of IO drivers
      Console.WriteLine("I/O drivers:");
      foreach(XPRMIODriver iod in module.IODrivers) {
        Console.WriteLine(
          " {0}:{1}",
          iod.Name,
          (iod.Info!=null)?iod.Info:""
        );
      }
    }


    /// <summary>
    /// Return the r/w status of a control parameter
    /// </summary>
    static string rwstatus(XPRMParameter p) {
      if (p.IsReadable)
        if (p.IsWriteable)
          return "r/w";
        else
          return "r";
      else
        if (p.IsWriteable)
          return "w";
        else
          return "?";
    }


    /// <summary>
    /// Display a prototype from a signature
    /// </summary>
    static void dispProcFct(XPRMProcedure proc) {
      char[] parms;
      int i;
      if (proc.TypeCode!=XPRMVarType.NOT)
        Console.Write(" function {0}", proc.Name);
      else
        Console.Write(" procedure {0}", proc.Name);

      if (proc.NbParameters>0) {
        Console.Write("(");
        parms = proc.ParameterTypes.ToCharArray();
        i = 0;
        while (i<parms.Length) {
          if (i>0) Console.Write(",");
          i = dispType(i, parms)+1;
        }
        Console.Write(")");
      }

      if (proc.TypeCode!=XPRMVarType.NOT)
        Console.Write(":{0}", proc.TypeName);
      Console.WriteLine();
    }

    /// <summary>
    /// Display a type name from a signature
    /// </summary>
    static int dispType(int i, char[] parms) {
      int j;
      switch (parms[i]) {
        case 'i': Console.Write("integer"); break;
        case 'r': Console.Write("real"); break;
        case 's': Console.Write("string"); break;
        case 'b': Console.Write("boolean"); break;
        case 'v': Console.Write("mpvar"); break;
        case 'c': Console.Write("linctr"); break;
        case 'I': Console.Write("range"); break;
        case 'a': Console.Write("array"); break;
        case 'e': Console.Write("set"); break;
        case '|':
          i++;
          do {
            Console.Write(parms[i++]);
          } while (parms[i]!='|');
          break;
        case '!':
          i++;
          do {
            Console.Write(parms[i++]);
          } while (parms[i]!='!');
          break;
        case 'A':
          Console.Write("array (");
          j=++i;
          while (parms[i]!='.') {
            if (j!=i) Console.Write(",");
            i=dispType(i, parms)+1;
          }
          Console.Write(") of ");
          i=dispType(i+1, parms);
          break;
        case 'E':
          Console.Write("set of ");
          i = dispType(++i, parms);
          break;
        default:
          Console.Write("?");
          break;
      }
      return i;
    }

  }
}
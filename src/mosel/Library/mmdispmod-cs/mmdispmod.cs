/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmdispmod.cs                                   */
/*  `````````````````                                   */
/*  Example for the use of the Mosel libraries          */
/*  (display the contents of a model)                   */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J.Farmer & Y. Colombani                 */
/********************************************************/

using System;
using Mosel;

namespace mmdispmod {
  public class mmdispmodClass {
    /// <summary>
    /// Main entry point for the application
    /// </summary>
    [STAThread]
    static void Main(string[] args) {
      XPRM mosel;
      XPRMModel mod;
      XPRMProcedure proc;

      if(args.Length!=1)
      {
        Console.WriteLine("Usage: mmdispmod modname.bim");
        return;
      }

      // Initialize Mosel
      mosel = XPRM.Init();
      // Load a BIM file
      mod = mosel.LoadModel(args[0]);

      // List of model parameters
      foreach(XPRMParameter p in mod.Parameters) {
        Console.WriteLine(" {0}", p.Name);
      }
      Console.WriteLine();

      // List of symbols
      foreach(XPRMIdentifier symb in mod.Identifiers) {
        switch (symb.StructCode) {
          case XPRMVarStruct.CONST: // Constant: display value
            Console.WriteLine(
              " {0}= {1}",
              symb.Name,
              ((XPRMConstant) symb).Value
            );

            break;
          case XPRMVarStruct.REF:   // Reference: display type
            Console.WriteLine(
              " {0}: {1}",
              symb.Name,
              symb.TypeName
            );
            break;
          case XPRMVarStruct.ARRAY: // Array: display type
            Console.WriteLine(
              " {0}: array of {1}",
              symb.Name,
              symb.TypeName
            );
            break;
          case XPRMVarStruct.SET:   // Set: display type
            Console.WriteLine(
              " {0}: set of {1}",
              symb.Name,
              symb.TypeName
            );
            break;
          case XPRMVarStruct.PROC:  // Subroutine
            proc = (XPRMProcedure) symb;
            do { // Look for all overloading proc/func
              dispProcFct(proc);  // Display the prototype
            } while ((proc=proc.Next())!=null);
            break;
          default:
            Console.WriteLine(" {0}: ?", symb.Name);
            break;
        }
      }
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
          i=dispType(i, parms)+1;
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
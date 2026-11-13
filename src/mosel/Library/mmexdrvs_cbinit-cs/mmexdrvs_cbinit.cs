/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexcbdrvs_cbinit.cs                           */
/*  ```````````````````                                 */
/*  Example for the use of the Mosel libraries          */
/*  (using dotnet: I/O driver for data exchange via     */
/*  callbacks                                           */
/*                                                      */
/*  (c) 2010 Fair Isaac Corporation                     */
/*      author: J.Farmer, Y. Colombani, 2010            */
/********************************************************/

using System;
using System.IO;
using Mosel;

namespace mmexdrvs_cbinit {
  public class mmexdrvs_cbinit_Class {
                    /***************************************/
                    /* The source of the model as a string */
                    /***************************************/
    const string source_of_model=
      "model tstcb\n"+
      "uses 'mmsystem'\n"+
      "parameters\n"+
      " ICB_INITFROM=''\n"+
      " ICB_INITTO=''\n"+
      "end-parameters\n"+

      "public declarations\n"+
      " v_i:integer\n"+
      " v_r:real\n"+
      " v_s:string\n"+
      " v_b:boolean\n"+
      " v_d:date\n"+

      " s_i:set of integer\n"+
      " l_i:list of integer\n"+

      " s_d:set of date\n"+
      " l_d:list of date\n"+

      " a_i:array(range) of integer\n"+
      " Rx:range\n"+
      " a_s:array(Rx) of string\n"+
      " a_r:array(Rx) of real\n"+

      " R=record\n"+
      "    public i:integer\n"+
      "    public s:set of integer\n"+
      "   end-record\n"+
      " r:R\n"+
      " a_R:array(range) of R\n"+
      "end-declarations\n"+

      "initialisations from ICB_INITFROM\n"+
      " v_i\n"+
      " v_r\n"+
      " v_s\n"+
      " v_b\n"+
      " v_d\n"+

      " s_i\n"+
      " l_i\n"+

      " s_d\n"+
      " l_d\n"+

      " a_i\n"+
      " [a_s,a_r] as 'ax'\n"+

      " r\n"+
      " a_R\n"+
      "end-initialisations\n"+

      " writeln('v_i=',v_i)\n"+
      " writeln('v_r=',v_r)\n"+
      " writeln('v_s=',v_s)\n"+
      " writeln('v_b=',v_b)\n"+
      " writeln('v_d=',v_d)\n"+
      " writeln('s_i=',s_i)\n"+
      " writeln('l_i=',l_i)\n"+
      " writeln('s_d=',s_d)\n"+
      " writeln('l_d=',l_d)\n"+
      " writeln('a_i=',a_i)\n"+
      " writeln('a_r=',a_r)\n"+
      " writeln('a_s=',a_s)\n"+
      " writeln('r=',r)\n"+
      " writeln('a_R=',a_R)\n"+

      "initialisations to ICB_INITTO\n"+
      " v_i\n"+
      " v_r\n"+
      " v_s\n"+
      " v_b\n"+
      " v_d\n"+

      " s_i\n"+
      " l_i\n"+

      " s_d\n"+
      " l_d\n"+

      " a_i\n"+

      " r\n"+
      " a_R\n"+
      "end-initialisations\n"+
      "end-model";


    /// <summary>
    /// A function to initialize the Mosel data-structures via callback
    /// </summary>
    public static bool initializeFrom(XPRMInitializeContext ictx,string label,XPRMTyped type)
    {

      try {
      
        switch (label) {
        
          case "v_i":  // v_i:999
            ictx.Send(999);
            return true;

          case "v_r":  // v_r:999.99
            ictx.Send(999.99);
            return true;

          case "v_b":  // v_b:false
            ictx.Send(false);
            return true;

          case "v_s":  // v_s:"tralala"
            ictx.Send("tralala");
            return true;

          case "v_d":  // v_d:"2012-12-12"
            ictx.Send("2012-12-12");
            return true;

          case "s_i":  // s_i:[10 20 30 ... ]
          case "l_i":
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=1;i<=10;i++)
              ictx.Send(i*10);
            ictx.Send(XPRMInitializeControl.CloseList);
            return true;

          case "s_d":  // s_d:["2001-01-11" "2002-02-21" ... ]
          case "l_d":
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=1;i<=10;i++)
              ictx.Send(string.Format("{0}-{1}-{2}",2000+i,i,i+1));
            ictx.Send(XPRMInitializeControl.CloseList);
            return true;

          case "a_i":  // a_i:[ (1) 10 (2) 20 ... ]
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=1;i<=10;i++) {
              ictx.Send(XPRMInitializeControl.OpenIndices);
              ictx.Send(i);
              ictx.Send(XPRMInitializeControl.CloseIndices);
              ictx.Send(i*10);
            }
            ictx.Send(XPRMInitializeControl.CloseList);
            return true;

          case "ax":  // ax:[ (1) [ "aa1" 1.23 ] (2) [ "aa2" 2.46 ] ... ]
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=1;i<=10;i++) {
              ictx.Send(XPRMInitializeControl.OpenIndices);
              ictx.Send(i);
              ictx.Send(XPRMInitializeControl.CloseIndices);
              ictx.Send(XPRMInitializeControl.OpenList);
              ictx.Send(string.Format("aa{0}",i));
              ictx.Send((double)i*1.23);
              ictx.Send(XPRMInitializeControl.CloseList);
            }
            ictx.Send(XPRMInitializeControl.CloseList);
            return true;

          case "r":  // r:[ 123 [ 10 20 30 ] ]
            ictx.Send(XPRMInitializeControl.OpenList);
            ictx.Send(123);
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=1;i<=3;i++) 
              ictx.Send(i*10);
            ictx.Send(XPRMInitializeControl.CloseList);
            ictx.Send(XPRMInitializeControl.CloseList);
            return true;

          case "a_R":  // a_R:[ (1) [10 [10 20 30] ] (1) [20 [20 40 60] ] ... ]
            ictx.Send(XPRMInitializeControl.OpenList);
            for (int i=1;i<=10;i++) {
              ictx.Send(XPRMInitializeControl.OpenIndices);
              ictx.Send(i);
              ictx.Send(XPRMInitializeControl.CloseIndices);
              ictx.Send(XPRMInitializeControl.OpenList);
              ictx.Send(i*10);
              ictx.Send(XPRMInitializeControl.OpenList);
              for (int j=1;j<=3;j++)
                ictx.Send(j*i*10);
              ictx.Send(XPRMInitializeControl.CloseList);
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
      Console.WriteLine(".NET: {0} = {1}", label, val);
      return true;
    }



    /// <summary>
    /// Main function
    /// </summary>
    [STAThread]
    public static void Main(string[] args) {
      XPRM mosel;                 // An instance of Mosel
      XPRMModel mod;              // The model

      // Initialize Mosel
      mosel=XPRM.Init();

      // Set default output stream to stdout
      mosel.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF,Console.Out);

      // Compile and load the model
      mod = mosel.CompileAndLoad(new StringReader(source_of_model));

      // Set the execution parameters and bind the variables
      mod.SetExecParam("ICB_INITFROM","dotnet:cbinitfrom");
      mod.SetExecParam("ICB_INITTO","dotnet:cbinitto");
      mod.Bind("cbinitfrom", new XPRMInitializationFrom(initializeFrom));
      mod.Bind("cbinitto", new XPRMInitializationTo(initializeTo));

      // Run the model
      mod.Run();

    }
  }
}

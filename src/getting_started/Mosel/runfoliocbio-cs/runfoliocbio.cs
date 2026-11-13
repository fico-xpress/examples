/*******************************************************
   Mosel Example Problems
   ====================== 

   file runfoliocbio.cs
   ````````````````````
   Running a Mosel model from a C# application
   with data exchange between model and host application
   during the optimization run.
   (Passing data via callback)

   *** The model started by this program cannot be run with 
       a Community Licence for the provided data instance ***

   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke, July 2011
               J. Farmer, May 2012
********************************************************/

using System;
using System.IO;
using Mosel;

namespace mosel_examples {
  // Class to receive solution values of decision variables
   public class MySolArray
   {
    public string ind;                // index name
    public double val;                // solution value
   }
  public class runfoliocbio
  {

   static MySolArray[] solfrac;
   static MySolArray[] solbuy;
   static bool ifprint;

  /*************************************************/
  /* A method to initialize model data via callback */
  /*************************************************/
  /**** Retrieving data from Mosel ****/
  public static bool initializeTo(string label, XPRMValue value)
  {
   double retsol;
   int numshares, solcount;
   XPRMArray solarr;
   XPRMSet[] sets;
   int[] indices;
   int asize, ct;

   if(label.Equals("FRAC"))
   {
    solarr=(XPRMArray)value;
    asize=solarr.Size;
    solfrac = new MySolArray[asize];
    for(int i=0;i<asize;i++) solfrac[i] = new MySolArray();

    sets = solarr.IndexSets;          // Get the indexing sets
    ct=0;
    indices = solarr.FirstTEIndex;    // Get the first entry of the array
    do
    {
     solfrac[ct].ind=sets[0].GetAsString(indices[0]);
     solfrac[ct].val=solarr.GetAsReal(indices);
     ct++;
    } while(solarr.NextTEIndex(indices));  // Get the next index  
    if (ifprint==false) 
    { ifprint=true; }
    else
    {
     ifprint=false;
     for(int i=0;i<asize;i++)
      Console.WriteLine(solfrac[i].ind + ": " + solfrac[i].val*100 + "% (" + 
       solbuy[i].val + ")");
     solbuy = null;
     solfrac = null;  
    }
   }
   else if(label.Equals("BUY"))
   {
    solarr=(XPRMArray)value;
    asize=solarr.Size;
    solbuy = new MySolArray[asize];
    for(int i=0;i<asize;i++) solbuy[i] = new MySolArray();

    sets = solarr.IndexSets;          // Get the indexing sets
    ct=0;
    indices = solarr.FirstTEIndex;    // Get the first entry of the array
    do
    {
     solbuy[ct].ind=sets[0].GetAsString(indices[0]);
     solbuy[ct].val=solarr.GetAsReal(indices);
     ct++;
    } while(solarr.NextTEIndex(indices));  // Get the next index  
    if (ifprint==false) 
    { ifprint=true; }
    else
    {
     ifprint=false;
     for(int i=0;i<asize;i++)
      Console.WriteLine(solfrac[i].ind + ": " + solfrac[i].val*100 + "% (" + 
       solbuy[i].val + ")");
     solbuy = null;
     solfrac = null;  
    }
   }
   else if(label.Equals("RETSOL"))
   {
    retsol=value.AsReal();
    Console.WriteLine("Total return: " + retsol);
   }
   else if(label.Equals("NUMSHARES"))
   {
    numshares=value.AsInteger();
    Console.WriteLine("Number of shares: " + numshares);
   }
   else if(label.Equals("SOLCOUNT"))
   {
    solcount=value.AsInteger();
    Console.WriteLine("Solution number: " + solcount);
   }
   else Console.WriteLine("Unknown output data item: " + label + "=" + value);
   return true;
  }


   public static void Main(string[] args)
   {
    XPRM mosel;
    XPRMModel mod;
                      // Model parameter settings
    double maxrisk = 1.0/3;
    double minreg = 0.2;
    double maxreg = 0.5;
    double maxsec = 0.25;
    double maxval = 0.2;
    double minval = 0.1;
    int maxnum = 15;
    ifprint = false;

    try{
     mosel = XPRM.Init();                // Initialize Mosel
    }catch(XPRMException e){
                  Console.WriteLine("License error" + e.Message);
                  throw new Exception("Error during execution");
    }
    mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
                                              // Set Mosel work directory to folder containing our example files

    try{
     mosel.Compile("foliocbio.mos");   // Compile the model (only required
                                        // during development phase, deployed
                // application would only use BIM)
    }catch(XPRMCompileException e){
     Console.WriteLine(e.Message);
    }

    mod = mosel.LoadModel("foliocbio.bim");  // Load the model

                      // Pass model parameters through execution parameters
    mod.SetExecParam("MAXRISK",maxrisk);
    mod.SetExecParam("MINREG",minreg);
    mod.SetExecParam("MAXREG",maxreg);
    mod.SetExecParam("MAXSEC",maxsec);
    mod.SetExecParam("MAXVAL",maxval);
    mod.SetExecParam("MINVAL",minval);
    mod.SetExecParam("MAXNUM",maxnum);
    mod.SetExecParam("DATAFILE","rmt:folio250.dat");
    mod.SetExecParam("OUTPUTFILE","dotnet:runfoliocbio_cbinit");
    mod.Bind("runfoliocbio_cbinit", new XPRMInitializationTo(initializeTo));

    mod.Run();                          // Run the model

    if(mod.ExecStatus != XPRMRunResult.RT_OK){
     throw new Exception("Error during model execution");
    }
    if(mod.ProblemStatus != XPRMProblemStatus.PB_OPTIMAL){
     throw new Exception("Problem not optimal");
    }                                  // Stop if no solution available

    mod.Reset();                       // Reset the model
   }
  }

}

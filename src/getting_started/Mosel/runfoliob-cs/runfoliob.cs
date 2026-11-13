/*******************************************************
   Mosel Example Problems
   ====================== 

   file runfoliob.cs
   `````````````````
   Running a Mosel model from a .NET application
   with data exchange between model and host application.
   (Passing data through buffers)
   
   (c) 2009 Fair Isaac Corporation
       author: J. Farmer,  Jun. 2009, rev. May. 2021
********************************************************/

using Mosel;
using System;
using System.IO;

namespace mosel_getting_started {
 public class runfoliob
 {
  private const string DATAFILE = "folio10.dat";
 
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
                     // Input and output handling                   
   MemoryStream inputData = new MemoryStream();
   MemoryStream resultOutput;
   long last;
   byte[] output;
   
                    // Read input data from text file in Mosel format, into the input buffer
   string dataDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;                    
   FileStream filereader = new FileStream(dataDir+"/"+DATAFILE, FileMode.Open, FileAccess.Read);
   byte[] buf = new byte[128];
   int b;
   while ((b=filereader.Read(buf,0,buf.Length))!=0)
   {
    inputData.Write(buf, 0, b);
   }
   filereader.Close();
 
   inputData.Seek(0, SeekOrigin.Begin);
   resultOutput = new MemoryStream(4096);   // Create 4K byte buffer
 
 
   try{
    mosel = XPRM.Init();               // Initialize Mosel
   }catch(XPRMException e){
     Console.WriteLine("License error" + e.Message);
     throw new Exception("Error during execution");
   }
   mosel.WorkDir = dataDir;            // Set Mosel work directory to folder containing our example files
 
   try{
    mosel.Compile("foliomemio.mos");   // Compile the model (only required
                                       // during development phase, deployed
                                       // application would only use BIM)
   }catch(XPRMException e){
    Console.WriteLine(e.Message);
   }
   
   mod = mosel.LoadModel("foliomemio.bim");  // Load the model
 
                     // Associate the .NET objects with names in Mosel
   mosel.Bind("inputdata", inputData);
   mosel.Bind("outputdata", resultOutput);
 
                     // Pass model parameters through execution parameters
   mod.ExecParams = "MAXRISK=" + maxrisk + ",MINREG=" + minreg + 
                    ",MAXREG=" + maxreg + ",MAXSEC=" + maxsec + ",MAXVAL=" +
                    maxval + ",MINVAL=" + minval + ",MAXNUM=" + maxnum +
                                       // File names (IO driver)
                    ",DATAFILE='dotnetstream:inputdata',OUTPUTFILE='dotnetstream:outputdata',";
 
   mod.Run();                          // Run the model
 
   if(mod.ExecStatus != XPRMRunResult.RT_OK){
    throw new Exception("Error during model execution");
   }
   if(mod.ProblemStatus != XPRMProblemStatus.PB_OPTIMAL){
    throw new Exception("Problem not optimal");
   }                                   // Stop if no solution available
 
                     // Display result output obtained from the model
   Console.WriteLine("Solution:");
   last = resultOutput.Length;
   output = resultOutput.GetBuffer();
   for (long i=0; i<last; i++)
    Console.Write((char)output[i]);
 
   mod.Reset();                        // Reset the model
  }
 }
}


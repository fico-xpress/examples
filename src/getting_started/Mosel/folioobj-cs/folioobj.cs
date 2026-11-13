/********************************************************
  Mosel Library Example Problems
  ==============================

  file folioobj.cs
  ````````````````
  Accessing model results.

  (c) 2009 Fair Isaac Corporation
      author: J.Farmer,  Jun. 2009, rev. May. 2021
********************************************************/

using Mosel;
using System;
using System.IO;

namespace mosel_getting_started {
 public class folioobj
 {
  public static void Main(string[] args)
  {
   XPRM mosel;
   XPRMModel model;
   XPRMArray varr;
   XPRMMPVar x;
   XPRMSet shares;
   int[] indices;
 
   mosel = XPRM.Init();                       // Initialize Mosel
   mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
                                              // Set Mosel work directory to folder containing our example files
 
   mosel.Compile("foliodata.mos");            // Compile the model
   model = mosel.LoadModel("foliodata.bim");  // Load compiled model
   model.Run();                               // Run the model
   
                                       // Test whether a solution is found 
                                       // and print the objective value 
   if(model.ProblemStatus==XPRMProblemStatus.PB_OPTIMAL)
     Console.WriteLine("Objective value: " + model.ObjectiveValue);
 
                                       // Retrieve the decision variables 
   varr=(XPRMArray)model.FindIdentifier("frac"); // Get model object 'frac',
                                                 // it must be an array
 
                                       // Retrieve the index names 
   shares=(XPRMSet)model.FindIdentifier("SHARES");  // Get model object 'SHARES',
                                                 // it must be a set
 
   indices = varr.FirstIndex;         // Get the first entry of array varr
                                       // (we know that the array is dense)
   do
   {
    x = varr.Get(indices).AsMPVar();   // Get a variable from varr
    Console.WriteLine(shares.Get(indices[0]) + ":\t" + x.Solution*100 + "%");
              // Print the solution value
   } while(varr.NextIndex(indices));   // Get the next index
 
  }
 }
}

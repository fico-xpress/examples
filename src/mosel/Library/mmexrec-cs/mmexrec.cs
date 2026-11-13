/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexrec.cs                                     */
/*  ```````````````                                     */
/*  Accessing modeling objects                          */
/* (enumerating an array of records and                 */
/*  printing the value of each record field).           */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: J. Farmer                               */
/********************************************************/


using System;
using System.IO;
using Mosel;


namespace mmexrec {

  /// <summary>
  /// Records example
  /// </summary>
  class mmexrecClass {

    [STAThread]
    static void Main(string[] args) {
      
      /* Initialize Mosel */
      XPRM mosel = XPRM.Init();
      // Set Mosel work directory to folder containing our example source code
      mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly().Location).FullName;
      
      /* Compile & load the model */
      mosel.Compile("Models/burglar_rec.mos");
      XPRMModel mod = mosel.LoadModel("Models/burglar_rec.bim");
      
      /* Run the model */
      mod.Run();
      
      /* Get the model object named 'I' */
      XPRMArray arr = (XPRMArray) mod.Identifiers["I"];

      /* Iterate over the fields */
      XPRMRecordFields fields = ((XPRMUserType)mod.ExpandType(arr.TypeCode)).Fields;
      
      Console.WriteLine("Record has {0} fields:", fields.Count);
      foreach (XPRMRecordField f in fields)
        Console.WriteLine(f.Name);
      
      /* Enumerate the array (we know it has a single dimension) */
      XPRMSet[] indSets = arr.IndexSets;
      
      /* Get the first index tuple */
      int[] indices = arr.FirstIndex;
      do {
        /* Display the array index */
        Console.Write("I({0}): \t", indSets[0].Get(indices[0]));
        /* Retrieve the array entry (=record) */
        XPRMRecord rec = arr.Get(indices).AsRecord();
        /* Output contents of 1st record field */
        Console.Write("{0}={1} ", fields[0].Name, rec.GetValueAsReal(fields[0]));
        /* Output contents of 2nd record field */
        Console.Write("{0}={1} ", fields[1].Name, rec.GetValueAsReal(fields[1]));
        Console.WriteLine();
      } while (arr.NextIndex(indices)); /* And move onto the next index tuple */
      
      
      /* Reset the model */
      mod.Reset();
      
    }


  }

}
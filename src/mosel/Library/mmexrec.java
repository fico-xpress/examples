/*******************************************************
   Mosel Library Examples
   ====================== 

   file mmexrec.java
   `````````````````
   Accessing modeling objects 
   (enumerating an array of records and
    printing the value of each record field).

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2007
********************************************************/

import com.dashoptimization.*;

public class mmexrec
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMArray arr;
  XPRMUserType type;
  XPRMSet[] set;
  XPRMRecord rec;
  XPRMRecordField[] field=new XPRMRecordField[2];
  int ct;
  int[] indices;

  mosel = new XPRM();                      // Initialize Mosel

  mosel.compile("Models/burglar_rec.mos"); // Compile, load & run the model
  mod = mosel.loadModel("Models/burglar_rec.bim");
  mod.run();

  arr=(XPRMArray)mod.findIdentifier("I");  // Get the model object named 'I'
                                           // Expand type of a user type
  type=(XPRMUserType)mod.expandType(arr.getTypeCode()); 
//  if (type.getStructCode()!=type.STR_REC)  

          // Retrieve record field info (we know there are 2 fields)
  ct=0;
  for(XPRMRecordFields fields=type.fields(); fields.hasNext();) 
   field[ct++]=fields.next();

          // Enumerate the array (we know it has a single dimension) 
  set=arr.getIndexSets();                  // Get the indexing set

  indices=arr.getFirstIndex();             // Get the first index tuple
  do
  {                                        // Retrieve the array index
   System.out.print("I(" + set[0].get(indices[0]) + "):  \t");
   rec=arr.get(indices).asRecord();        // Retrieve array entry (=record)
                                           // Contents of 1st record field
   System.out.print(field[0].getName() + "=" + 
                      rec.getValueAsReal(field[0]) + " ");
                                           // Contents of 2nd record field
   System.out.println(field[1].getName() + "=" + 
                      rec.getValueAsReal(field[1]) + " ");
  } while(arr.nextIndex(indices));         // Get the next index tuple 

  mod.close();                             // Reset the model 
 }
}

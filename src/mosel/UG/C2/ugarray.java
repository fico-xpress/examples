/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugarray.java
   `````````````````
   Accessing modeling objects (sparse arrays).

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2004
********************************************************/

import com.dashoptimization.*;

public class ugarray
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMArray varr;
  XPRMSet[] sets;
  int[] indices;
  int dim;

  mosel = new XPRM();                  // Initialize Mosel

  mosel.compile("transport.mos");      // Compile, load & run the model
  mod = mosel.loadModel("transport.bim");
  mod.run();

  varr=(XPRMArray)mod.findIdentifier("flow"); // Get model object 'flow'
                                       // it must be an array
  dim = varr.getDimension();           // Get the number of dimensions 
                                       // of the array
  sets = varr.getIndexSets();          // Get the indexing sets

  indices = varr.getFirstTEIndex();    // Get the first true entry index
  do
  {
   System.out.print("flow(");
   for(int i=0;i<dim-1;i++)
    System.out.print(sets[i].get(indices[i]) + ",");
   System.out.print(sets[dim-1].get(indices[dim-1]) + "), ");
  } while(varr.nextTEIndex(indices));  // Get next true entry index tuple
  System.out.println();

  mod.close();                         // Unload the model 
 }
}

/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexas.java                                    */
/*  ````````````````                                    */
/*  Example for the use of the Mosel libraries          */
/*  (using arrays with index sets: different ways       */
/*   of enumerating arrays)                             */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2004                        */
/********************************************************/

import com.dashoptimization.*;

public class mmexas
{
public static void main(String[] args) throws Exception
{
 XPRM mosel;
 XPRMModel mod;
 XPRMArray varr;
 XPRMSet[] sets;
 XPRMArrayIndices arrind;
 int[] indices;
 int dim;

 mosel=new XPRM();                        // Initialize Mosel
 mod=mosel.loadModel("Models/trans.bim"); // Load a BIM file
 mod.run();                               // Run the model

 varr=(XPRMArray)mod.findIdentifier("x"); // Get the model object named 'x'
                                          // it must be an array

 dim=varr.getDimension();                 // Get the number of dimensions of
                                          // the array
 sets=varr.getIndexSets();                // Get the indexing sets

 System.out.println("\n1. Logic entries:");
 indices=varr.getFirstIndex();            // Get the first entry of varr
 do
 {
  System.out.print("x(");
  for(int i=0;i<dim-1;i++)
   System.out.print(sets[i].get(indices[i])+",");
  System.out.print(sets[dim-1].get(indices[dim-1])+"), ");
 } while(varr.nextIndex(indices));        // Get the next index tuple

 if(varr.isDynamic())                     /* There would be no difference to
                                             the first way of enumerating 
                                             in the case of a dense array */
 { 
  System.out.println("\n\n2. True entries:");
  indices=varr.getFirstTEIndex();          // Get the first true entry index
  do
  {
   System.out.print("x(");
   for(int i=0;i<dim-1;i++)
    System.out.print(sets[i].get(indices[i])+",");
   System.out.print(sets[dim-1].get(indices[dim-1])+"), ");
  } while(varr.nextTEIndex(indices));     // Get next true entry index tuple

  System.out.println("\n\n3. True entries with iterator:");
  arrind=varr.indices(true);
  while(arrind.hasNext())
  {
   indices=arrind.next();
   System.out.print("x(");
   for(int i=0;i<dim-1;i++)
    System.out.print(sets[i].get(indices[i])+",");
   System.out.print(sets[dim-1].get(indices[dim-1])+"), ");
  }
 } 
 System.out.println();

 mod.close();
}
}

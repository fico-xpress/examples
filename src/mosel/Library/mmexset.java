/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexset.java                                   */
/*  `````````````````                                   */
/*  Example for the use of the Mosel libraries          */
/*  (accessing sets in Mosel)                           */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2004                        */
/********************************************************/

import com.dashoptimization.*;

public class mmexset
{
public static void main(String[] args) throws Exception
{
 XPRM mosel;
 XPRMModel mod;
 XPRMSet set;
 int first,last;

 mosel=new XPRM();                          // Initialize Mosel
 mod=mosel.loadModel("Models/burglari.bim");// Load a BIM file
 mod.run();                                 // Run the model

 set=(XPRMSet)mod.findIdentifier("ITEMS"); // Get the model object named 'ITEMS'
                                           // it must be a set

 if(!set.isEmpty())
 {
  first = set.getFirstIndex();       // Get the number of the first index
  last = set.getLastIndex();         // Get the number of the last index
  System.out.println("Elements of set ITEMS:");
  for(int i=first;i<=last;i++)       // Print names of all set elements
   System.out.print(" " + set.getAsString(i) + ",");
  System.out.println();  
 }

 if(set.getIndex("CD player")<0)
  System.out.println("'CD player' is not contained in 'ITEMS'."); 

 mod.close();
}
}

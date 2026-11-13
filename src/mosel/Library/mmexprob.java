/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexprob.java                                  */
/*  ``````````````````                                  */
/*  Example for the use of the Mosel libraries          */
/*  (accessing problems and solution information)       */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2004                        */
/********************************************************/

import com.dashoptimization.*;

public class mmexprob
{
public static void main(String[] args) throws Exception
{
 XPRM mosel;
 XPRMModel mod;
 XPRMArray varr, darr;
 XPRMLinCtr lgrade;
 int[] indices;

 mosel=new XPRM();                        // Initialize Mosel
 mod=mosel.loadModel("Models/blend2.bim");// Load a BIM file
 mod.run();                               // Run the model
                                          // (it includes optimization)


 mod.exportProblem("p","blend");          // Export problem to a file in LP 
                                          // format (maximization)

 if(mod.getProblemStatus()==mod.PB_OPTIMAL) // Test whether optimal is found
  System.out.println("Solution is optimal");

 System.out.println("Objective value: "+mod.getObjectiveValue());
                                          // Print the objective function value

 varr=(XPRMArray)mod.findIdentifier("x"); // Get the model object named 'x'
                                          // it must be an array

 darr=(XPRMArray)mod.findIdentifier("COST");// Get the model object 'COST'
                                            // it must be an array

 indices=varr.getFirstIndex();       // Get the first entry of the array varr
 do
 {
  // Display solution value and corresponding cost 
  System.out.println(
    "x(" + indices[0] + ")=" + varr.get(indices).asMPVar().getSolution() +
    "(COST: " + darr.getAsReal(indices) +
    ")");
 } while(varr.nextIndex(indices));   // Get the next index

                                     // Get the model object 'LoGrade'
                                     // it must be a reference to
                                     // a linear constraint
 lgrade=((XPRMReference)mod.findIdentifier("LoGrade")).asLinCtr();


 System.out.println(                 // Display the activity and dual values
   "LoGrade: activity=" + lgrade.getActivity() +
   ", dual=" + lgrade.getDual() );

 mod.close();
}
}

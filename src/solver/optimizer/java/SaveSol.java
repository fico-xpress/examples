/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file SaveSol.java
   ```````````````
   Show how to save a postsolved solution to memory and access the
   results of a global search. The program demonstrates the use of the
   integer solution callback.

   We take the knapsack problem in burglar.mps and instigate a global
   search. Whenever an integer solution is found it is postsolved,
   stored in memory, and printed to an output file. The best and final
   solution values, and other global search information, are displayed
   on screen. A log file is also created.

   (c) 2020-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import static com.dashoptimization.XPRSenumerations.ObjSense;
import static com.dashoptimization.XPRSenumerations.MIPStatus;
import static com.dashoptimization.XPRSenumerations.CutStrategy;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSintSolListener;
import com.dashoptimization.XPRSprob;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.io.IOException;

public class SaveSol {
    /** Run this example.
     * @param args Command line arguments. One optional argument is
     *             allowed: the model to solve.
     */
    public static void main(String[] args) throws FileNotFoundException {
        // Setup the file names we will use.
        String problem = args.length == 0 ? "../data/burglar" : args[0];
        File outFile = new File("SaveSol.out");
        String logFile = "SaveSol.log";

        outFile.delete();
        try (PrintStream output = new PrintStream(outFile);
             XPRSprob prob = new XPRSprob(null))
        {
            // Enable default output
            prob.addMessageListener(DefaultMessageListener::console);

            // Allow no cuts - so the problem does not solve too quickly
            prob.controls().setCutStrategy(CutStrategy.NONE);

            // Read the problem file
            prob.readProb(problem);

            /*** Tell Optimizer to postsolve every integer solution found,
             *   save it to memory and print the solution values to the
             * output file ***/

            // Call PrintSol whenever an integer solution is found
            PrintSol ps = new PrintSol(prob, output);
            prob.addIntSolListener(ps);

            /*** Search for integer solutions ***/
            System.out.printf("Solving problem %s:%n%n", problem);
            output.printf("Solving problem %s:%n%n", problem);
            prob.chgObjSense(ObjSense.MAXIMIZE);
            prob.optimize();


            /*** Retrieve the results of the global search ***/

            // Get the number of integer solutions found
            int nSol = prob.attributes().getMIPSols();

            // Get the objective value of the best integer solution found
            double bestObj = prob.attributes().getMIPObjVal();

            // Get the number of outstanding nodes
            int activeNodes = prob.attributes().getActiveNodes();

            // Get the node at which the last feasible integer solution was found
            int lastNode = prob.attributes().getMIPSolNode();

            // Get the number of nodes solved
            int nodes = prob.attributes().getNodes();

            // Get the value of the best bound
            double bestBnd = prob.attributes().getBestBound();

            // Get the global status
            MIPStatus status = prob.attributes().getMIPStatus();

            /*** Display the results of the global search ***/
            if ( status.equals(MIPStatus.NO_SOL_FOUND) ) {
                System.out.println("Global search incomplete");
                System.out.println("   No integer solution found");
                System.out.printf("   %d nodes searched%n", nodes);
                System.out.printf("   %d nodes remaining in search%n", activeNodes);
                System.out.printf("   Best bound %g%n", bestBnd);
            }
            else if ( status.equals(MIPStatus.SOLUTION) ) {
                double[] intSol = ps.getIntSol();
                System.out.println("Global search incomplete");
                System.out.printf("   %d integer solution%s found\n", nSol, (nSol==1) ? "" : "s");
                System.out.printf("   %d nodes searched%n", nodes);
                System.out.printf("   %d nodes remaining in search%n", activeNodes);
                System.out.printf("   Best bound %g%n", bestBnd);
                System.out.printf("   Best integer solution at node %d%n", lastNode);
                System.out.printf("      Objective value %g%n", bestObj);
                System.out.printf("      Solution values");
                for (int i = 0; i < intSol.length; i++)
                    System.out.printf("          x[%d]=%g%n", i, intSol[i]);
                System.out.println();
            }
            else if ( status.equals(MIPStatus.INFEAS) ) {
                System.out.println("Global search complete");
                System.out.println("   No integer solution found");
                System.out.printf("   %d nodes searched%n", nodes);
                System.out.printf("   Best bound %g%n", bestBnd);
            }
            else if ( status.equals(MIPStatus.OPTIMAL) ) {
                double[] intSol = ps.getIntSol();
                System.out.println("Global search complete");
                System.out.printf("   %d nodes searched%n", nodes);
                System.out.printf("   %d integer solution%s found%n", nSol, (nSol==1) ? "" : "s");
                System.out.printf("   Best integer solution at node %d%n", lastNode);
                System.out.printf("      Objective value %g%n", bestObj);
                System.out.printf("      Solution values%n");
                for (int i = 0; i < intSol.length; i++)
                    System.out.printf("          x[%d]=%g%n", i, intSol[i]);
                System.out.println();
            }
            else {
                System.out.println("Global search did not take place");
            }
        }
    }

    /** Save the postsolved nodal integer solution to memory, retrieve it
     * and print the solution values to the output file.
     * The callback is called when an integer solution is found at a node
     * in the branch and bound tree.
     */
    private static class PrintSol implements XPRSintSolListener {
        private final PrintStream output;
        private double[] intSol;

        public PrintSol(XPRSprob prob, PrintStream out) {
            output = out;
        }

        /** Get the last solution stored by the callback. */
        public double[] getIntSol() { return intSol; }

        /** This is the function that is invoked by the optimizer. */
        @Override
        public void XPRSintSolEvent(XPRSprob prob, Object vContext) {
            // Get the current node number
            int nodeNum = prob.attributes().getCurrentNode();

            // Get the objective value of the current integer solution
            double objVal = prob.attributes().getLPObjVal();

            // Retrieve the postsolved solution values from memory
            intSol = prob.getCallbackSolution();

            // Print the solution to the output file
            output.printf("Node %d%n", nodeNum);
            output.printf("   Integer solution has objective value %g%n", objVal);
            output.println("   Postsolved solution values are:");
            for (int i = 0; i < intSol.length; i++)
                output.printf("      x[%d]=%g%n", i, intSol[i]);
            output.println();
        }
    }
}

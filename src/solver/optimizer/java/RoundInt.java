/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file RoundInt.java
   ```````````````
   Apply an integer fixing heuristic to a MIP problem. The program
   also demonstrates the use of the integer solution callback.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSintSolListener;
import com.dashoptimization.XPRSprob;

import static com.dashoptimization.XPRSenumerations.CutStrategy;

/** Apply an integer fixing heuristic to a MIP problem.
 *
 * The program also demonstrates the use of the integer solution callback.
 *
 * We take the power generation problem stored in hpw15.mps which
 * seeks to optimise the operating pattern of a group of electricity
 * generators. We solve this MIP with a very loose integer tolerance
 * and then fix all binary and integer variables to their rounded
 * integer values, changing both their lower and upper bounds. We then
 * solve the resulting LP.
 *
 * The results are displayed on screen and the problem statistics
 * stored in a logfile.
 */
public class RoundInt {
    /** The very loose integrality tolerance we use in the example. */
    public static final double TOL = 0.3;

    /** Run the example.
     * @param args The problem name. This is optional. If no problem name
     *             is specified then "hpw15" is used.
     */
    public static void main(String[] args) {
        String problem = args.length == 0 ? "../data/hpw15" : args[0];
        if (args.length > 0)
            problem = args[0];
        String logFile = "RountInt.log";

        try (XPRSprob prob = new XPRSprob(null)) {
            // Delete and define log file
            new java.io.File(logFile).delete();
            prob.setLogFile(logFile);

            // Install default output: We only print warning and error messages.
            prob.addMessageListener(new DefaultMessageListener(null, System.err, System.err));

            // Read the problem file
            prob.readProb(problem);


            // Disable cuts - or the optimal solution will be found at the top
            // node and the integer solution callback will hardly be used.
            prob.controls().setCutStrategy(CutStrategy.NONE);

            // Solve MIP with loose integer tolerance

            // Set integer feasibility tolerance
            prob.controls().setMIPTol(TOL);

            // Tell Optimizer to print out MIP information at each node and
            // call IntSol whenever an integer solution is found
            prob.controls().setMIPLog(3);
            prob.addIntSolListener(new IntSol());

            // Get the number of columns
            int cols = prob.attributes().getCols();

            // Solve the MIP problem and get the solution values
            System.out.printf("Applying an integer fixing heuristic to problem %s:-%n%n", problem);
            System.out.printf("Solving MIP:%n%n");
            prob.optimize();
            double[] x = prob.getSolution();

            // Round off the values of all binary and integer variables
            XPRSprob.MIPEntityInfo info = prob.getDiscreteCols();
            int mipents = info.coltype.length;
            int[] glInd = info.colind;
            byte[] glType = info.coltype;

            // Allocate memory for bound arrays
            int[] bndInd = new int[mipents];
            double[] bndVal = new double[mipents];
            byte[] bndType = new byte[mipents];

            // Initialise bound counter
            int nBnd = 0;

            // Go through MIP entities
            for (int k = 0; k < mipents; ++k) {
                // Test whether each is a binary or integer variable
                if (glType[k] == 'B' || glType[k] == 'I') {

                    // Hold the index of the variable
                    int j = glInd[k];

                    // Store the index, round the variable's value to the
                    // nearest integer, and reset both its upper and lower
                    // bounds
                    bndInd[nBnd] = j;
                    bndVal[nBnd] = Math.round(x[j]);
                    bndType[nBnd] = 'B';
                    ++nBnd;
                }
            }

            // Instruct Optimizer to change the bounds
            prob.chgBounds(nBnd, bndInd, bndType, bndVal);
            System.out.printf("\nAfter MIP search, %d binary and integer variables were fixed%n%n", nBnd);

            // Solve the resulting LP

            // Solve the LP and get the solution values
            prob.lpOptimize();
            double[] y = prob.getSolution();

            // Get and print the LP objective function value
            System.out.printf("After solving the LP the final objective value was %g%n%n", prob.attributes().getLPObjVal());

            // Print the non-zero MIP and corresponding LP solution values for
            // comparison:
            System.out.printf("The non-zero MIP and LP solution values are:%n%n");
            for (int j = 0; j < cols; ++j) {
                if (x[j] != 0.0) {
                    System.out.printf("   x[%2d] = %5.1f  %5.3g%n", j, x[j], y[j]);
                }
            }
            System.out.println();
        }
    }

    /** Callback to display objective value associated with a node. */
    private static final class IntSol implements XPRSintSolListener {

        /** Displays the objective value associated with a node in the MIP
         * search.
         * This function is called every time an integer solution has been
         * found.
         * @param prob The problem for which a solution was found.
         * @param vContext unused.
         */
        @Override
        public void XPRSintSolEvent(XPRSprob prob, Object vContext) {
            // Get the node number
            int node = prob.attributes().getCurrentNode();
            // Get the objective function value
            double obj = prob.attributes().getLPObjVal();
            System.out.printf("   Node %d (Objective value = %g)%n", node, obj);
        }
    }
}

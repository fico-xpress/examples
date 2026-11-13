/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file Knapsack.java
   ````````````````
   Apply a primal heuristic to a knapsack problem.
   The program demonstrates the use of the global log callback.

   We take the knapsack problem stored in burglar.mps and instigate a
   global search. At each node, so long as the current solution is
   both LP optimal and integer infeasible, we truncate the solution
   values to create a feasible integer solution. We then update the
   cutoff, if the new objective value has improved it, and continue
   the search.

   (c) 2020-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.AbstractOptNodeListener;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;
import static com.dashoptimization.XPRSenumerations.CutStrategy;
import static com.dashoptimization.XPRSenumerations.ObjSense;
import static com.dashoptimization.XPRSenumerations.LPStatus;

/** Example implementation.
 * <em>Note</em> that this example does not address issues like parallel
 * execution of callbacks, restarts and other things that are important
 * when implementing callbacks. For a detailed discussion and implementation
 * of this refer to the {@link MostViolated} example.
 */
public final class Knapsack {

    /**
     * Truncate the LP nodal solution values to create a feasible integer
     * solution, and update the cutoff value if the new objective value
     * has improved it. The heuristic is only applied when the nodal
     * solution is both LP optimal and integer infeasible.
     * This callback is called at each node in the global search.
     */
    private static final class TruncSol extends AbstractOptNodeListener {
        /** Number of columns in the problem. */
        private final int cols;
        /** Objective function coefficients. */
        private final double[] coef;
        /** Integrality tolerance. */
        private final double intTol;

        public TruncSol(XPRSprob prob) {
            cols = prob.attributes().getCols();
            coef = new double[cols];
            prob.getObj(coef, 0, cols - 1);
            intTol = prob.controls().getMIPTol();
        }

        /** The actual callback function. */
        @Override
        public int optNodeEvent(XPRSprob prob, int feas) {
            // Get the current node number
            int nodNum = prob.attributes().getCurrentNode();

            // Get objective value at the current node
            double objVal = prob.attributes().getLPObjVal();

            // Get the current cutoff value
            double cutoff = prob.controls().getMIPAbsCutoff();

            // Apply heuristic if nodal solution is LP optimal and integer
            // infeasible
            LPStatus lpStatus = prob.attributes().getLPStatus();
            int intInfeas = prob.attributes().getMIPInfeas();
            if (lpStatus.equals(LPStatus.OPTIMAL) && intInfeas > 0) {
                // Get LP solution
                double[] x = prob.getCallbackSolution();

                // Truncate each solution value - making allowance for the
                // integer tolerance - and calculate the new "heuristic"
                // objective value
                double heurObj = 0.0;
                for (int j = 0; j < cols; ++j)
                    heurObj += coef[j] * Math.floor(x[j] + intTol);
                System.out.printf("   Node %d: Objective Value: ORIGINAL %g;  HEURISTIC %g%n",
                                  nodNum, objVal, heurObj);

                // If the "heuristic" objective value is better, update the
                // cutoff - we assume that all the objective coefficents are
                // integers
                if (heurObj > cutoff) {
                    prob.controls().setMIPAbsCutoff(heurObj + 0.9999);
                    System.out.printf("              ** Cutoff updated to %g **%n", heurObj + 1.0);
                }
            }

            // If the nodal solution is not LP optimal do not apply the heuristic
            else if (!lpStatus.equals(LPStatus.OPTIMAL)) {
                System.out.printf("   Node %d: LP solution not optimal, not applying heuristic%n", nodNum);
                System.out.printf("           (%s)%n", lpStatus.toString());
            }

            // If the nodal solution is integer feasible print the objective value
            else if (intInfeas == 0)
                System.out.printf("   Node %d: Integer solution found: Objective Value %g%n", nodNum, objVal);

            return feas;
        }
    }

    /** Run the example.
     * The model solved is either the default model <code>burglar</code>
     * or is the model specified as first command line argument.
     * <em>Note</em>: The code assumes that the model to be solved is indeed
     *                a knapsack problem with objective sense "maximize".
     *                You may get wrong results if the model does not satisfy
     *                this.
     */
    public static void main(String[] args) {
        String logFile = "knapsack.log";
        String problem = args.length == 0 ? "../data/burglar" : args[0];

        try (XPRSprob prob = new XPRSprob(null)) {
            // Delete and define log file
            new java.io.File(logFile).delete();
            prob.setLogFile(logFile);

            // Install default output
            prob.addMessageListener(DefaultMessageListener::console);

            // Turn off presolve and disallow cuts - to slow solution and
            // allow the effect of the heuristic to be seen
            prob.controls().setPresolve(XPRS.PRESOLVE_NONE);
            prob.controls().setCutStrategy(CutStrategy.NONE);
            prob.controls().setHeurEmphasis(0);

            // Read the problem file
            prob.readProb(problem);

            /*** Prepare to apply the heuristic ***/



            // Tell Optimizer to print global information to the log file
            // at each node
            prob.controls().setMIPLog(3);

            // Tell Optimizer to call TruncSol at each node and apply the
            // heuristic
            prob.addOptNodeListener(new TruncSol(prob));

            /*** Perform the global search - in the course of which the
                 heuristic will be applied ***/
            prob.chgObjSense(ObjSense.MAXIMIZE);
            System.out.println("Applying a primal heuristic to problem " + problem);
            prob.optimize();

            System.out.printf("Optimal objective value: %f%n", prob.attributes().getObjVal());
        }
    }


}

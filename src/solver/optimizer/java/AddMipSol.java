/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file AddMipSol.java
   ````````````
   Apply an LP rounding heuristic at each node and inject feasible
   solutions found.

   (c) 2020-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.AbstractOptNodeListener;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSoptNodeListener;
import com.dashoptimization.XPRSprob;
import static com.dashoptimization.XPRSenumerations.CutStrategy;
import static com.dashoptimization.XPRSenumerations.MIPStopStage;

public class AddMipSol {

    /** Callback that runs a simple rounding heuristic at each node.
     *
     * The callback fixes all integer fractional variables to their rounded
     * values and solves the resulting problem. If that leads to a feasible
     * solution then that solution is injected back into the solver as a
     * new incumbent.
     */
    private static final class OptNode extends AbstractOptNodeListener {

        /** Problem name. */
        final String probName;
        /** Number of columns. */
        final int nCol;
        /** Number of MIP entities. */
        final int nMipEnt;
        /** Information about MIP entities. */
        final XPRSprob.MIPEntityInfo mipEntInfo;

        public OptNode(XPRSprob prob) {
            // Capture some global statistics that will be needed in the
            // heuristic.
            probName = prob.getProbName();
            nCol = prob.attributes().getCols();
            nMipEnt = prob.attributes().getMIPEnts();
            mipEntInfo = prob.getDiscreteCols();
        }

        /** Callback main method.
         * This is the method that is invoked by the solver.
         */
        public int optNodeEvent(XPRSprob prob, int feas) {
            // Only run once per node (since adding a solution will call
            // the callback to be fired again)
            if (prob.attributes().getCallbackCount_OptNode() > 1)
                return feas;

            // Only run the search for solutions with small numbers of
            // integer infeasibilities */
            if (prob.attributes().getMIPInfeas() > 0.25 * nMipEnt)
                return feas;

            fixedSearch(prob);

            return feas;
        }

        /** Run the heuristic that fixes all fractional integer variables
         * to their rounded values.
         */
        private void fixedSearch(XPRSprob parent) {
            // Create resources we need
            int[] bndInd = new int[nCol];
            double[] bndVal = new double[nCol];
            byte[] bndType = new byte[nCol];
            Thread me = Thread.currentThread(); // to get unique names

            // Create a problem containing the original problem
            try (XPRSprob prob = new XPRSprob(null)) {
                prob.copyProb(parent);
                prob.postSolve();

                // Reset the controls of the problem.
                prob.setDefaults();

                // Rename the problem so we don't get collisions with
                // temporary files produced by the optimizer
                prob.setProbname(probName + "_" + me);

                // Output all messages.
                prob.addMessageListener(DefaultMessageListener::console);

                // Get information from the solution at the current node
                // of 'parent' problem */
                boolean hasMipIncumbent = parent.attributes().getMIPSols() != 0;
                double incumbentObj = parent.attributes().getMIPObjVal();
                double[] x = parent.getCallbackSolution();

                // Initialise bound counter
                int nBnd = 0;
                double mipTol = parent.controls().getMIPTol();

                // Go through MIP entities
                for (int i = 0; i < nMipEnt; i++) {
                    // Test whether each is a binary or integer variable
                    if (mipEntInfo.coltype[i] == 'B' || mipEntInfo.coltype[i] == 'I') {
                        int j = mipEntInfo.colind[i];

                        // Fix variables that are fractional
                        double rounded = Math.round(x[j]);
                        if (Math.abs(rounded - x[j]) <= mipTol) {
                            continue;
                        }
                        bndInd[nBnd] = j;
                        bndVal[nBnd] = rounded;
                        bndType[nBnd] = 'B';
                        nBnd++;
                    }
                }

                // Instruct Optimizer to change the bounds
                prob.chgBounds(nBnd, bndInd, bndType, bndVal);
                System.out.println(me + ": After MIP search, " + nBnd +
                                   " binary and integer variables were fixed");

                // Ensure we only do a small search in the fixed problem and
                // use the current incumbent objective from the main problem
                prob.controls().setMaxNode(100);
                prob.controls().setMaxMIPSol(1);
                if (hasMipIncumbent) {
                    // Set a cut-off to help the fixed search
                    prob.controls().setMIPAbsCutoff(incumbentObj);
                }
                prob.controls().setMIPLog(3);

                // Read the basis for the LP relaxation to reduce run time
                prob.readBasis(probName, "");

                // Run the optimizer
                prob.optimize();

                // If we found a solution then load it into the main problem
                if (prob.attributes().getMIPSols() > 0) {
                    x = prob.getSolution();
                    parent.addMipSol(nCol, x, null, null);
                }
            }
        }
    }

    public static void main(String[] args) {
        // Get problem name from command line or use a default problem.
        String model = args.length == 0 ? "../data/addmipsol" : args[0];

        try (XPRSprob prob = new XPRSprob(null)) {
            prob.readProb(model, "");
            prob.addMessageListener(DefaultMessageListener::console);
            // Register the callback that will perform heuristic search.
            prob.addOptNodeListener(new OptNode(prob));

            // Make the problem difficult for the optimizer to solve.
            prob.controls().setCutStrategy(CutStrategy.NONE);
            prob.controls().setHeurEmphasis(0);
            prob.controls().setSBBest(0);
            prob.controls().setMIPThreads(20);

            // Log every node in the branch-and-bound search.
            prob.controls().setMIPLog(3);

            // Solve the root node relaxation and write out the basis
            // (to reduce solve time of the fixed problems) */
            prob.controls().setMIPStopStage(MIPStopStage.INITIALRELAXATION);
            prob.optimize();
            prob.controls().resetMIPStopStage();
            prob.writeBasis("", "");

            // Run the MIP search
            prob.optimize("");

            System.out.printf("Optimal objective value: %f%n",
                              prob.attributes().getMIPObjVal());

        }
    }
}

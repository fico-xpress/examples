/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file FixBV.java
   ````````````
   Apply a binary fixing heuristic to an unpresolved MIP problem.

   We take a production plan model and solve its LP relaxation.
   Next we fix those binary variables that are very
   near zero to 0.0, and those that are almost one to 1.0, by changing
   their respective upper and lower bounds. Finally, we solve the
   modified problem as a MIP.
   This heuristic will speed up solution - though may fail to optimise
   the problem.
   The results are displayed on screen and the problem statistics
   stored in a log file.

   (c) 2020-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;
import static com.dashoptimization.XPRSenumerations.CutStrategy;
import static com.dashoptimization.XPRSenumerations.MIPStatus;
import static com.dashoptimization.XPRSenumerations.MIPStopStage;
import static com.dashoptimization.XPRSenumerations.ObjSense;

import java.io.File;

public class FixBV {
    /** Tolerance on binary variables */
    private static final double TOL = 0.0005;
    public static void main(String[] args) {
        String logName = "FixBV.log";
        String problem = args.length == 0 ? "../data/coco" : args[0];

        try (XPRSprob prob = new XPRSprob(null)) {
            // Delete and define log file
            File logFile = new File(logName);
            logFile.delete();
            prob.setLogFile(logFile.getAbsolutePath());

            // Enable output
            prob.addMessageListener(DefaultMessageListener::console);

            // Turn off presolve and permit no cuts - to slow down solution
            // and allow the effect of the heuristic to be be seen
            prob.controls().setPresolve(XPRS.PRESOLVE_NONE);
            prob.controls().setCutStrategy(CutStrategy.NONE);

            // Read the problem file
            prob.readProb(problem, "");

            /*** Solve the LP relaxation ***/

            // Solve the root node relaxation
            prob.chgObjSense(ObjSense.MAXIMIZE);
            prob.controls().setMIPStopStage(MIPStopStage.INITIALRELAXATION);
            prob.optimize();
            prob.controls().resetMIPStopStage();

            // Get LP solution values
            double[] x = prob.getLpSolX();

            /*** Fix the binary variables that are at their bounds ***/

            int nMipEnt = prob.attributes().getMIPEnts();
            XPRSprob.MIPEntityInfo info = prob.getDiscreteCols();

            // Allocate memory for bound arrays
            int[] bndInd = new int[nMipEnt];
            double[] bndVal = new double[nMipEnt];
            byte[] bndType = new byte[nMipEnt];

            // Go through the MIP entities and fix
            int nBnd = 0;
            for (int k = 0; k < nMipEnt; ++k) {
                // Test whether binary variable
                if (info.coltype[k] == 'B') {
                    int j = info.colind[k];

                    // If the value of the BV is within TOL of zero,
                    // store its index, set its upper bound to 0, and
                    // increment the bound counter */
                    if (x[j] <= TOL) {
                        bndInd[nBnd] = j;
                        bndType[nBnd] = 'U';
                        bndVal[nBnd] = 0.0;
                        nBnd++;
                    }
                    // If the value of the BV is within TOL of one, store
                    // its index, set its lower bound to 1, and increment
                    // the bound counter */
                    else if ((1-x[j]) <= TOL) {
                        bndInd[nBnd] = j;
                        bndType[nBnd] = 'L';
                        bndVal[nBnd] = 1.0;
                        nBnd++;
                    }
                }
            }

            // Apply the fixings
            prob.chgBounds(nBnd, bndInd, bndType, bndVal);
            System.out.println("Solving problem " + problem + " with a binary fixing heuristic:");
            System.out.println("   After the LP optimisation " + nBnd + " binary variables were fixed");

             /*** Solve the modified problem as a MIP ***/

            // Search for an integer solution
            prob.optimize();

            // Get the number of nodes solved in the MIP search
            int nNodes = prob.attributes().getNodes();

            // Get the objective value of the best integer solution
            double objVal = prob.attributes().getMIPObjVal();

            // Check the MIP status and display the results of the
            // MIP search */
            MIPStatus status = prob.attributes().getMIPStatus();

            if (status.equals(MIPStatus.NOT_LOADED))
                System.out.println("   Problem has not been loaded");
            else if (status.equals(MIPStatus.LP_NOT_OPTIMAL))
                System.out.println("   Search has not begun - LP has not been optimised");
            else if (status.equals(MIPStatus.LP_OPTIMAL))
                System.out.println("   Search has not begun - LP has been optimised");
            else if (status.equals(MIPStatus.NO_SOL_FOUND))
                System.out.println("   Search interrupted - No integer solution was found");
            else if (status.equals(MIPStatus.SOLUTION))
                System.out.println("   Search interrupted - Integer solution found: " + objVal);
            else if (status.equals(MIPStatus.INFEAS))
                System.out.println("   No integer solution was found");
            else if (status.equals(MIPStatus.OPTIMAL))
                System.out.println("   Integer solution found: " + objVal);

            System.out.println("   The MIP optimisation took " + nNodes + " nodes");

        }
    }
}

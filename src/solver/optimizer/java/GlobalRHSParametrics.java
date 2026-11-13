/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file GlobalRHSParametrics.java
   ```````````````
   Perform RHS parametrics on a global problem.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;

import static com.dashoptimization.XPRSenumerations.ObjSense;
import static com.dashoptimization.XPRSenumerations.MIPStatus;
import static com.dashoptimization.XPRSenumerations.MIPStopStage;

/** Perform RHS parametrics on a global problem.
 * We take a production plan model and observe how the optimal
 * value of the objective function changes as we vary
 * the RHS element RESMAX(2), the resources available in Month 2.
 * The program decrements RESMAX(2) from 6 to 1, and for each of these
 * values assesses the feasibility of the revised problem and, where
 * possible, finds the best integer solution.
 * The results are displayed on screen and the problem statistics
 * stored in a log file.
 */
public class GlobalRHSParametrics {

    /** Run the example.
     * @param args If non-empty then <code>args[0]</code> is used as problem,
     *             otherwise "pplan" is used.
     */
    public static void main(String[] args) {
        String problem = args.length == 0 ? "../data/pplan" : args[0];
        if (args.length > 0)
            problem = args[0];
        String logFile = "GlobalRHSParametrics.log";

        try (XPRSprob prob = new XPRSprob(null);
             XPRSprob copy = new XPRSprob(null)) {
            // Delete and define log file
            new java.io.File(logFile).delete();
            prob.setLogFile(logFile);

            // Install default output: We only print warning and error messages.
            prob.addMessageListener(new DefaultMessageListener(null, System.err, System.err));

            // Read the problem file
            prob.readProb(problem);

            // Set the objective sense
            prob.chgObjSense(ObjSense.MAXIMIZE);

            // Get the number rows and columns
            int rows = prob.attributes().getRows();
            int cols = prob.attributes().getCols();

            // Get the row index of RESMAX(2) */
            int index = prob.getIndex(1, "ResMax02");

            // Allocate memory for the basis status arrays
            int[] rowStatus = new int[rows];
            int[] colStatus = new int[cols];

            System.out.printf("The results of the parameter changes on pplan are:%n%n");

            // Decrement RESMAX(2) from 6 to 1
            for (int i = 6; i >= 1; --i) {
                double resmax2 = (double) i;

                // Change the RHS
                prob.chgRHS(index, resmax2);

                // Store the current matrix - as global will later change it
                copy.copyProb(prob);

                // Restore the previous optimal basis - for efficiency
                if (i < 6)
                    copy.loadBasis(rowStatus, colStatus);

                // Solve the root node relaxation
                copy.controls().setMIPStopStage(MIPStopStage.INITIALRELAXATION);
                copy.optimize();
                copy.controls().resetMIPStopStage();

                // Get the status of the root solve
                MIPStatus status = copy.attributes().getMIPStatus();

                // If the root node could not be solved to optimality then exit the loop
                if (status == MIPStatus.LP_NOT_OPTIMAL) {
                    System.out.printf("   RESMAX(2)=%2.0f;  LP not solved to optimality%n", resmax2);
                    break;
                }

                // Get the optimal basis
                copy.getBasis(rowStatus, colStatus);

                // Search for an integer solution */
                copy.optimize();

                // Check the global status
                status = copy.attributes().getMIPStatus();
                if (status == MIPStatus.INFEAS) {
                    System.out.printf("   RESMAX(2)=%2.0f;  infeasible%n", resmax2);
                    continue;
                }
                else if (status != MIPStatus.OPTIMAL) {
                    System.out.printf("   RESMAX(2)=%2.0f;  error %s%n", resmax2, status.toString());
                    continue;
                }

                // Get, and then print, the objective value of the best integer solution
                double obj = copy.attributes().getMIPObjVal();
                System.out.printf("   RESMAX(2)=%2.0f;  dObjValue=%4.1f%n", resmax2, obj);

            }
            System.out.println();
        }
    }
}

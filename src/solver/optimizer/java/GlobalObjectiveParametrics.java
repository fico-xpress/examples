/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file GlobalObjectiveParametrics.java
   ```````````````
   Perform objective function parametrics on a global problem.
   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;
import static com.dashoptimization.XPRSenumerations.ObjSense;
import static com.dashoptimization.XPRSenumerations.MIPStopStage;

/** Perform objective function parametrics on a global problem.
 * We take a production plan model and observe how the optimal
 * value of the objective function changes as we vary
 * BEN(3), the benefit per month from finishing Project 3.
 * The program increments BEN(3) from 8 to 15, and for each of these
 * values revises the objective coefficients of the variables x(3,t),t=1:2
 * and finds the best integer solution. Note that, for each t, the
 * coefficient of x(3,t) is BEN(3)*(3-t) = BEN(3)*(6-t-4+1).
 * The results are displayed on screen and the problem statistics stored
 * in a log file.
 */
public class GlobalObjectiveParametrics {
    /** Run the example.
     * @param args If non-empty then <code>args[0]</code> is used as problem,
     *             otherwise "pplan" is used.
     */
    public static void main(String[] args) {
        String problem = args.length == 0 ? "../data/pplan" : args[0];
        if (args.length > 0)
            problem = args[0];
        String logFile = "GlobalObjectiveParametrics.log";

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

            // Get the column indices for x(3,t),t=1:2
            int[] x3 = new int[]{ prob.getIndex(2, "x___0301"),
                                  prob.getIndex(2, "x___0302") };

            // Allocate memory for the basis status arrays
            int[] rowStatus = new int[rows];
            int[] colStatus = new int[cols];

            System.out.printf("The results of the parameter changes on pplan are:%n%n");

            // Increment BEN(3) from 8 to 15
            for (int i = 8; i <= 15; ++i) {
                double ben3 = (double) i;

                // Revise the objective coefficients of x(3,t),t=1:2
                double[] newobj = new double[]{ ben3*(3.0-1.0),
                                                ben3*(3.0-2.0) };

                // Change the objective function
                prob.chgObj(2, x3, newobj);

                // Store the current matrix - as global will later change it
                copy.copyProb(prob);

                // Restore the previous optimal basis - for efficiency
                if (i > 8)
                    copy.loadBasis(rowStatus, colStatus);

                // Solve the root node relaxation
                copy.controls().setMIPStopStage(MIPStopStage.INITIALRELAXATION);
                copy.optimize();
                copy.controls().resetMIPStopStage();

                // Get the optimal basis
                copy.getBasis(rowStatus, colStatus);

                // Search for an integer solution
                copy.optimize();

                // Get, and then print, the objective value of the best integer solution
                double objval = copy.attributes().getMIPObjVal();
                System.out.printf("   BEN(3) = %2.0f; objective = %4.1f%n", ben3, objval);

            }

            System.out.println();
        }
    }
}

/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file Tableau.java
   ```````````````
   Read in a user-specified LP and solve it, displaying both the
   initial and optimal tableau.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;
import static com.dashoptimization.XPRSenumerations.ObjSense;

import java.util.Arrays;

/** Read in a user-specified LP and solve it, displaying both the
 * initial and optimal tableau.
 *
 * Inputs an MPS matrix file and required optimisation sense, and
 * proceeds to solve the problem with optimize(). The simplex
 * algorithm is interrupted to get its intial basis, and a tableau is
 * requested with a call to function {@link showTab(String, XPRSprob)}.
 * Once the solution is found, a second call produces the optimal tableau.
 * Function {@link showTab(String, XPRSprob)} retrieves the pivot order of
 * the basic variables, along with other problem information, and then
 * constructs (and displays) the tableau row-by-row using the backwards
 * transformation, btran.
 */
public class Tableau {


    /** Run the example.
     * @param args Command line arguments:
     *             - <code>args[0]</code> problem name
     *             - <code>args[1]</code> objective sense ("min" or "max"),
     *                                    if this argument is not provided the
     *                                    problem will be minimised, by default.
     */
    public static void main(String[] args) {
        // Validate command line
        if (args.length > 2) {
            System.err.println("Usage: java Tableau [matrix] [min|max]");
            System.exit(1);
        }
        String problem = args.length > 0 ? args[0] : "../data/tablo.mps";
        ObjSense sense = ObjSense.MINIMIZE;
        if (args.length == 2) {
            if (args[1].equals("min"))
                sense = ObjSense.MINIMIZE;
            else if (args[1].equals("max"))
                sense = ObjSense.MAXIMIZE;
            else {
                System.err.printf("Invalid objective sense '%s'%n", args[1]);
                System.exit(1);
            }
        }
        String logFile = "Tableau.log";

        try (XPRSprob prob = new XPRSprob(null)) {

            // Delete and define log file
            new java.io.File(logFile).delete();
            prob.setLogFile(logFile);

            // Install default output: We only print warning and error messages.
            prob.addMessageListener(new DefaultMessageListener(null, System.err, System.err));

            // Input the matrix
            prob.readProb(problem);

            // Turn presolve off as we want to display the initial tableau
            prob.controls().setPresolve(XPRS.PRESOLVE_NONE);

            // Set the number of simplex iterations to zero
            prob.controls().setLPIterLimit(0);

            // Perform the first step in the simplex algorithm
            prob.chgObjSense(sense);
            prob.optimize();


            // Display the initial tableau
            showTab("Initial", prob);

            // Continue with the simplex algorithm
            prob.controls().setLPIterLimit(1000000);
            prob.optimize();

            // Get and display the objective function value and
            // the iteration count */
            System.out.printf("M%simal solution with value %g found in %d iterations.%n",
                              (sense == ObjSense.MINIMIZE) ? "in" : "ax",
                              prob.attributes().getLPObjVal(),
                              prob.attributes().getSimplexIter());

            // Display the optimal tableau
            showTab("Optimal", prob);
        }
    }

    /** Display tableau on screen.
     * @param state Problem state
     * @param problem Problem with a resident tableau.
     */
    private static void showTab(String state, XPRSprob prob) {
        // Get problem information
        int rows = prob.attributes().getRows();
        int cols = prob.attributes().getCols();
        int spare = prob.attributes().getSpareRows();
        int vector = rows + cols + 1;

        // Allocate memory to the arrays
        double[] y = new double[rows];
        double[] z = new double[vector];
        int[] rowind = new int[rows];
        double[] matelem = new double[rows];
        int[] pivot = new int[rows];
        double[] x = new double[cols];
        double[] slack = new double[rows];
        int[] start = new int[2];

        // Retrieve the pivot order of the basic variables
        prob.getPivotOrder(pivot);

        // Construct and display the tableau

        // Printer header of matrix names
        System.out.printf("%n%s tableau of problem %s%n", state,
                          prob.getProbName());
        System.out.println("Note: slacks on G type rows range from -infinity to 0");
        System.out.println();
        System.out.print("     ");
        // Get and print the individual row names (we only print the first 3 chars)
        for (String rowName : prob.getRowNames(0, rows - 1))
            System.out.printf(" %-3.3s   ", rowName);
        // Get and print the individual column names (we only print the first 3 chars)
        for (String colName : prob.getColumnNames(0, cols - 1))
            System.out.printf(" %-3.3s   ", colName);
        System.out.println(" RHS\n");

        // Get the tableau row-by-row using the backwards transformation, btran

        // For each row iRow in turn, calculate z = e_irow * B^-1 * A
        for (int i = 0; i < rows; ++i) {
            // Set up the unit vector e_irow
            Arrays.fill(y, 0.0);
            y[i] = 1.0;

            //  y = e_irow * B^-1
            prob.btran(y);

            // Form z = y * A for each slack column of A
            System.arraycopy(y, 0, z, 0, rows);

            // Form z = y * A, for each structural column of A
            for (int j = 0; j < cols; ++j) {
                int nzs = prob.getCols(start, rowind, matelem, rows, j, j);
                double d = 0.0;
                for (int k = 0; k < nzs; ++k)
                    d += y[rowind[k]] * matelem[k];
                z[rows + j] = d;
            }

            // Form z for RHS
            prob.getSolution(null, x, 0, x.length - 1);
            prob.getSlacks(null, slack, 0, slack.length - 1);
            if (pivot[i] >= rows)
                z[vector - 1] = x[pivot[i] - rows - spare];
            else
                z[vector - 1] = slack[pivot[i]];

            // Display single tableau row
            String name;
            if (pivot[i] < rows) {
                // Pivot is a row
                name = prob.getRowName(pivot[i]);
            }
            else {
                // Pivot is a column
                name = prob.getColumnName(pivot[i] - rows - spare);
            }
            System.out.printf("%-3.3s:", name);
            for (int j = 0; j < vector; ++j) {
                if (Math.abs(z[j]) > 0.1) // Consider small values as zero
                    System.out.printf("%5.1f  ",z[j]);
                else
                    System.out.print("       ");
            }
            System.out.println();
        }
        System.out.println();
    }
}

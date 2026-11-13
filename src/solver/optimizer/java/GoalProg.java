/***********************************************************************
  Goal programming example
  ========================

  An example of lexicographic goal programming using the Xpress
  multi-objective API.

  A company produces two electrical products, A and B. Both require
  two stages of production: wiring and assembly. The production plan
  must meet several goals:
  1. A profit of $200
  2. A contractual requirement of 40 units of product B
  3. To fully utilize the available wiring department hours
  4. To avoid overtime in the assembly department

  (c) 2022-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.XPRSprob;
import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations.ObjControl;
import com.dashoptimization.XPRSenumerations.SolveStatus;
import com.dashoptimization.XPRSenumerations.SolStatus;

import static com.dashoptimization.XPRSconstants.PLUSINFINITY;

public final class GoalProg {

    private static final int COLS     = 8;    // Number of columns
    private static final int ROWS     = 4;    // Number of rows
    private static final int ENTITIES = 2;    // Number of entities

    // Column indices
    private static final int COL_PRODUCE_A       = 0;
    private static final int COL_PRODUCE_B       = 1;
    private static final int COL_SURPLUS_WIRING  = 2;
    private static final int COL_DEFICIT_WIRING  = 3;
    private static final int COL_SURPLUS_ASSEM   = 4;
    private static final int COL_DEFICIT_ASSEM   = 5;
    private static final int COL_DEFICIT_PROFIT  = 6;
    private static final int COL_DEFICIT_PROD_B  = 7;

    /**
     * Set up and solve the goal programming example
     */
    public void solve() {
        try (XPRSprob prob = new XPRSprob(null)) {

            // Create two integer variables and six continuous variables
            for (int i = 0; i < 8; i++) {
                prob.addColumn(0, PLUSINFINITY, i < 2 ? ColumnType.Integer : ColumnType.Continuous, null);
            }

            // Add one row per goal
            // Goal 1: profit must meet or exceed $200
            prob.addRow(new int[] {COL_PRODUCE_A, COL_PRODUCE_B, COL_DEFICIT_PROFIT},
                        new double[] {7, 6, 1}, 'G', 200);
            // Goal 2: production of product B must meet or exceed 40 units
            prob.addRow(new int[] {COL_PRODUCE_B, COL_DEFICIT_PROD_B},
                        new double[] {1, 1}, 'G', 40);
            // Goal 3: hours of wiring should be close to the available 120 hours
            prob.addRow(new int[] {COL_PRODUCE_A, COL_PRODUCE_B, COL_SURPLUS_WIRING, COL_DEFICIT_WIRING},
                        new double[] {2, 3, -1, 1}, 'E', 120);
            // Goal 4: hours of assembly should be close to the available 300 hours
            prob.addRow(new int[] {COL_PRODUCE_A, COL_PRODUCE_B, COL_SURPLUS_ASSEM, COL_DEFICIT_ASSEM},
                        new double[] {6, 5, -1, 1}, 'E', 300);

            // Define objectives to minimize deviations, in priority order
            // Goal 1: minimize profit deficit
            prob.chgObj(1, new int[] {COL_DEFICIT_PROFIT}, new double[] {1});
            // Goal 2: minimize production deficit
            prob.chgObjN(1, 1, new int[] {COL_DEFICIT_PROD_B}, new double[] {1});
            // Goal 3: minimize deviation from wiring hours target
            prob.chgObjN(2, 2, new int[] {COL_SURPLUS_WIRING, COL_DEFICIT_WIRING}, new double[] {1, 1});
            // Goal 4: minimize deviation from assembly hours target
            prob.chgObjN(3, 2, new int[] {COL_SURPLUS_ASSEM, COL_DEFICIT_ASSEM}, new double[] {1, 1});

            // Set up objective priorities and tolerances
            for (int i = 0; i < 4; i++) {
                prob.setObjIntControl(i, ObjControl.PRIORITY, 4 - i);
                prob.setObjDblControl(i, ObjControl.ABSTOL, 0);
                prob.setObjDblControl(i, ObjControl.RELTOL, 0);
            }

            // Add a message listener to display log information.
            prob.addMessageListener(DefaultMessageListener::console);

            // Solve the problem
            prob.optimize();


            // Print the result
            if (prob.attributes().getSolveStatus() == SolveStatus.COMPLETED &&
                prob.attributes().getSolStatus() == SolStatus.OPTIMAL) {
                double[] sol = prob.getSolution();
                System.out.println("Production plan:");
                System.out.println("Product A: " + ((int) sol[COL_PRODUCE_A]) + " units");
                System.out.println("Product B: " + ((int) sol[COL_PRODUCE_B]) + " units");
                System.out.println("Profit: $" + (int) (7 * sol[COL_PRODUCE_A] + 6 * sol[COL_PRODUCE_B]));


                if (sol[COL_DEFICIT_PROFIT] > 0) {
                    System.out.println("Profit goal missed by $" + ((int) sol[COL_DEFICIT_PROFIT]));
                }
                if (sol[COL_DEFICIT_PROD_B] > 0) {
                    System.out.println("Contractual goal for product B missed by " + ((int) sol[COL_DEFICIT_PROD_B]) + " units");
                }
                if (sol[COL_SURPLUS_WIRING] > 0) {
                    System.out.println("Unused wiring department hours: " + ((int) sol[COL_SURPLUS_WIRING]));
                }
                if (sol[COL_DEFICIT_WIRING] > 0) {
                    System.out.println("Wiring department overtime: " + ((int) sol[COL_DEFICIT_WIRING]));
                }
                if (sol[COL_SURPLUS_ASSEM] > 0) {
                    System.out.println("Unused assembly department hours: " + ((int) sol[COL_SURPLUS_ASSEM]));
                }
                if (sol[COL_DEFICIT_ASSEM] > 0) {
                    System.out.println("Assembly department overtime: " + ((int) sol[COL_DEFICIT_ASSEM]));
                }
            } else {
                System.out.println("Problem could not be solved");
            }
        }
    }

    public static void main(String[] args) {
        new GoalProg().solve();
    }
}

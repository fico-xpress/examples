/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file repair.c
   `````````````
   Demonstrates the FICO repairinfeas utility.

   Prints a relaxation summary and creates the relaxed subproblem.

   (c) 2020-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;

import java.util.Arrays;

public final class Repair {

    public static void main(String[] args) {
        if ( args.length > 1 )
            throw new IllegalArgumentException("syntax: repair <filename>");
        String problem = args.length > 0 ? args[0] : "../data/iisexample";

        // Initialize Optimizer
        try (XPRSprob prob = new XPRSprob(null)) {
            // Forward messages to screen
            prob.addMessageListener(DefaultMessageListener::console);

            // Load problem file and query dimensions
            prob.readProb(problem, "");
            int rows = prob.attributes().getRows();
            int cols = prob.attributes().getCols();

            // Allocate and set the preference arrays (we give the same
            // preference to all potential violations)
            double[] rowPref = new double[rows];   // row preferences
            double[] groupPref = new double[rows]; // group preferences
            double[] lbPref = new double[cols];    // lower bound preferences
            double[] ubPref = new double[cols];    // upper bound preferences
            Arrays.fill(rowPref, 1.0);
            Arrays.fill(groupPref, 1.0);
            Arrays.fill(lbPref, 1.0);
            Arrays.fill(ubPref, 1.0);

            // Allocate memory for the infeasibility breakers
            double[] boundviolations = new double[cols];
            double[] rowviolations = new double[rows];

            // Call repairinfeas
            int status = prob.repairWeightedInfeas(rowPref, groupPref, lbPref, ubPref,
                                                   'n', 0.001, "");
            System.out.printf("Repairinfeas return code (%d): ", status);
            switch (status) {
            case 0: System.out.printf("relaxed optimum found%n"); break;
            case 1: System.out.printf("relaxed problem is infeasible%n"); break;
            case 2: System.out.printf("relaxed problem is unbounded%n"); break;
            case 3: System.out.printf("solution of the relaxed problem regarding the original objective is nonoptimal%n"); break;
            case 4: System.out.printf("error%n"); break;
            case 5: System.out.printf("numerical instability%n"); break;
            }

            int nSets = prob.attributes().getSets();
            int nEnts = prob.attributes().getMIPEnts();

            // Get solution
            double[] x = prob.getSolution();
            double[] slacks = prob.getSlacks();

            // Get the values of the breaker variables
            generateInfeasibilityBreakers(prob, x, slacks, rowviolations, boundviolations, 1.0e-6);

            // Print report
            generateInfeasilityReport(prob, rowviolations, boundviolations);

            // Repair and save problem
            applyRepairInfeasResult(prob, rowviolations, boundviolations);
            prob.writeProb("repaired.lp","lp");
            System.out.printf("Repaired problem is written as repaired.lp.%n");
            System.out.printf("(Please note that this matrix might show slightly different behaviour then the repairinfeas problem)%n%n");
        }
    }


    /** This function returns the lower bound for a row. */
    private static double getRowLB(XPRSprob prob, int i) {
        double rhs = prob.getRHS(i);
        double range = prob.getRHSrange(i);
        byte type = prob.getRowType(i);
        switch (type) {
        case 'L': return XPRS.MINUSINFINITY;
        case 'E': return rhs;
        case 'G': return rhs;
        case 'R': return rhs - range;
        default: return XPRS.MINUSINFINITY;
        }
    }

    /** This function returns the upper bound for a row. */
    private static double getRowUB(XPRSprob prob, int i) {
        double rhs = prob.getRHS(i);
        double range = prob.getRHSrange(i);
        byte type = prob.getRowType(i);
        switch (type) {
        case 'L': return rhs;
        case 'E': return rhs;
        case 'G': return XPRS.PLUSINFINITY;
        case 'R': return rhs;
        default: return XPRS.PLUSINFINITY;
        }
    }

    /** This function sets new lower and upper bounds for a row. */
    private static void setRowBounds(XPRSprob prob, int i, double rowLB, double rowUB) {
        double lb, ub;

        // Normalize bounds for range constraints (we assume that we
        // do not attempt to set contradictory bounds).
        if (rowLB <= rowUB) {
            lb = rowLB;
            ub = rowUB;
        } else {
            lb = rowUB;
            ub = rowLB;
        }

        // determine row type and set bounds
        if (lb <= XPRS.MINUSINFINITY && ub >= XPRS.PLUSINFINITY) {
            prob.chgRowType(i, (byte)'N');
        }
        else if (lb <= XPRS.MINUSINFINITY) {
            prob.chgRowType(i, (byte)'L');
            prob.chgRHS(i, ub);
        }
        else if (ub >= XPRS.PLUSINFINITY) {
            prob.chgRowType(i, (byte)'G');
            prob.chgRHS(i, lb);
        }
        else if (lb == ub) {
            prob.chgRowType(i, (byte)'E');
            prob.chgRHS(i, ub);
        }
        else {
            prob.chgRowType(i, (byte)'R');
            prob.chgRHS(i, ub);
            prob.chgRHSrange(i, ub - lb);
        }
    }

    /** This function calculates the infeasibilities of a solution
     * (i.e. the values of the feasibility breakers after a repairinfeas
     * call)
     */
    private static void generateInfeasibilityBreakers(XPRSprob prob, double[] x, double[] slacks, double[] constraints, double[] bounds, double tolarence) {
        // Get problem dimensions
        int rows = prob.attributes().getInputRows();
        int cols = prob.attributes().getInputCols();

        // Check constraints
        for (int i = 0; i < rows; i++) {
            double rhs = prob.getRHS(i);
            double activity = rhs - slacks[i];
            double lb = getRowLB(prob, i);
            double ub = getRowUB(prob, i);

            if (activity < lb - tolarence) {
                // a negative value indicates that the lower bound is relaxed
                constraints[i] = activity - lb;
            }
            else if (activity > ub+tolarence) {
                // a positive value indicates that the upper bound is relaxed
                constraints[i] = activity - ub;
            }
            else {
                constraints[i] = 0.0;
            }
        }

        // Check bounds
        for (int j = 0; j < cols; j++) {
            double lb = prob.getLB(j);
            double ub = prob.getUB(j);

            if (x[j] < lb - tolarence) {
                // a negative value indicates that the lower bound is relaxed
                bounds[j] = x[j] - lb;
            }
            else if (x[j] > ub + tolarence) {
                // a positive value indicates that the upper bound is relaxed
                bounds[j] = x[j] - ub;
            }
            else {
                bounds[j] = 0.0;
            }
        }
    }

    /** This function just converts a double to a string for reporting,
     * converting infinity values as "NONE"
     */
    private static String boundToString(double bound) {
        if (bound > XPRS.MINUSINFINITY && bound < XPRS.PLUSINFINITY) {
            if (bound >= 0)
                return String.format(" %.5e", bound);
            else
                return String.format("%.5e", bound);
        }
        else
            return "    NONE     ";
    }

    /** Prints a summary of the infeasibily breakers using the already
     * calculated infeasibities
     */
    private static void generateInfeasilityReport(XPRSprob prob, double[] constraintviolations, double []boundviolations) {
        double suminf = 0;   // Sum if infeasibility
        int    ninf = 0;     // Number of violateed rows and columns

        int rows = prob.attributes().getInputRows();
        int cols = prob.attributes().getInputCols();

        System.out.printf("%n");
        System.out.printf("%n");
        System.out.printf("XPRESS-MP FEASIBILITY REPAIR REPORT.%n");
        System.out.printf("%n");
        System.out.printf("The following constraints were repaired.%n");
        System.out.printf("%n");

        System.out.printf("Index     Name%4s  Lower bound    Repaired        Upper bound    Repaired %n", "");

        for (int i = 0; i < rows; i++) {
            if (constraintviolations[i] != 0) {

                // Get constraint name
                String name = prob.getRowName(i);
                double lb = getRowLB(prob, i);
                double ub = getRowUB(prob, i);

                suminf += Math.abs(constraintviolations[i]);
                ninf++;

                String lbShift, ubShift;
                if (constraintviolations[i] < 0) {
                    lbShift = boundToString(lb + constraintviolations[i]);
                    ubShift = boundToString(ub);
                } else {
                    lbShift = boundToString(lb);
                    ubShift = boundToString(ub + constraintviolations[i]);
                }

                System.out.printf("%5d  %8s    %7s  %7s   %7s  %7s%n", i, name, boundToString(lb), lbShift, boundToString(ub), ubShift);
            }
        }

        System.out.printf("%n");
        System.out.printf("The following bound constraints were repaired.%n");
        System.out.printf("%n");

        System.out.printf("Index   Name%4s   Lower bound      Repaired     Upper bound      Repaired %n", "");

        for (int j = 0; j < cols; j++) {
            if (boundviolations[j] != 0) {
                // Get column name
                String name = prob.getColumnName(j);
                double lb = prob.getLB(j);
                double ub = prob.getUB(j);

                suminf += Math.abs(boundviolations[j]);
                ninf++;

                String lbShift, ubShift;
                if (constraintviolations[j] < 0) {
                    lbShift = boundToString(boundviolations[j]);
                    ubShift = boundToString(ub);
                }
                else {
                    lbShift = boundToString(lb);
                    ubShift = boundToString(boundviolations[j]);
                }

                System.out.printf("%5d  %8s    %7s  %7s   %7s  %7s%n", j, name, boundToString(lb), lbShift, boundToString(ub), ubShift);
            }
        }

        System.out.printf("%n");
        System.out.printf("Sum of Violations %f%n", suminf);
        System.out.printf("Number of corrections: %d%n", ninf);
        System.out.printf("%n");
    }

    /** Relaxes the problem given the values of the feasibility breakers.
     */
    private static void applyRepairInfeasResult(XPRSprob prob, double[] constraintviolations, double[] boundviolations) {
        int rows = prob.attributes().getInputRows();
        int cols = prob.attributes().getInputCols();

        // Apply changes to rows
        for (int i = 0; i < rows; i++) {
            if (constraintviolations[i] != 0) {
                double lb = getRowLB(prob, i);
                double ub = getRowUB(prob, i);
                // Apply relaxation
                if (constraintviolations[i] > 0)
                    ub += constraintviolations[i];
                else
                    lb += constraintviolations[i];
                setRowBounds(prob, i, lb, ub);
            }
        }

        // Apply changes to columns
        for (int j = 0; j < cols; j++) {
            if (boundviolations[j] != 0) {
                double lb = prob.getLB(j);
                double ub = prob.getUB(j);
                if (boundviolations[j] > 0)
                    prob.chgUB(j, ub + boundviolations[j]);
                else
                    prob.chgLB(j, lb + boundviolations[j]);
            }
        }

        System.out.printf("%n");
        System.out.printf("Problem repaired.");
        System.out.printf("%n");
    }
}

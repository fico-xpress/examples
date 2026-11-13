// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;

import java.io.IOException;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Capital budgeting example. The problem is solved using three multi-objective
 * approaches: - Lexicographic approach, solving first to minimize capital
 * expended and second to maximize return - Blended approach, solving a weighted
 * combination of both objectives simultaneously - Lexicographic approach, with
 * the objective priorities reversed
 */
public class CapitalBudgeting {
    /**
     * Required capital for each project
     */
    static final double[] CAPITAL = { 104000, 53000, 29000, 187000, 98000, 32000, 75000, 200000 };
    /**
     * Required personnel for each project
     */
    static final double[] PERSONNEL = { 22, 12, 7, 36, 24, 10, 20, 41 };
    /**
     * Expected return of each project
     */
    static final double[] RETURN = { 124000, 74000, 42000, 188000, 108000, 56000, 88000, 225000 };
    /**
     * Target capital to invest
     */
    static final double CAPITAL_TARGET = 478000;
    /**
     * Target return on investment
     */
    static final double ROI_TARGET = 550000;
    /**
     * Available personnel
     */
    static final int PERSONNEL_MAX = 106;

    private final XpressProblem prob;

    /**
     * Binary decision variables indicating which projects will be implemented
     */
    private Variable[] select;

    /**
     * Constraint for the number of personnel available
     */
    private Inequality personnel;

    /**
     * Primary objective: minimize capital expended
     */
    private LinExpression capital;

    /**
     * Secondary objective: maximize return on investment
     */
    private LinExpression roi;

    public CapitalBudgeting(XpressProblem prob) {
        this.prob = prob;
    }

    public static void main(String[] args) throws IOException {
        try (XpressProblem prob = new XpressProblem()) {
            new CapitalBudgeting(prob).solve();
        }
    }

    public void solve() {
        // Binary decision variables indicating which projects will be implemented
        select = prob.addVariables(CAPITAL.length).withType(ColumnType.Binary).withName("select_%d").toArray();

        // Constraint: at least 3 projects must be implemented
        prob.addConstraint(sum(select).geq(3));

        // Constraint for the number of personnel available
        personnel = prob.addConstraint(scalarProduct(select, PERSONNEL).leq(PERSONNEL_MAX));

        // Primary objective: minimize capital expended
        capital = scalarProduct(select, CAPITAL);
        // The first objective has priority=1 (weight=1 by default)
        prob.setObjective(capital, XPRSenumerations.ObjSense.MINIMIZE);
        prob.setObjIntControl(0, XPRSenumerations.ObjControl.PRIORITY, 1);

        // Secondary objective: maximize return on investment
        roi = scalarProduct(select, RETURN);
        // The second objective has weight=-1 to maximize this expression, and
        // priority=0
        // to cause this objective to be solved in a second optimization
        prob.addObjective(roi, 0, -1);

        prob.optimize();
        printSolution("Higher priority for 'Minimize Capital' objective:");

        // Now set the same priority for both objectives. This will result in a single
        // solve
        // using the weighted sum of the two objectives.
        prob.setObjIntControl(1, XPRSenumerations.ObjControl.PRIORITY, 1);
        prob.optimize();
        printSolution("Equal priority for objectives:");

        // Finally, give the first objective a lower priority
        prob.setObjIntControl(0, XPRSenumerations.ObjControl.PRIORITY, 0);
        prob.optimize();
        printSolution("Higher priority for 'Maximize Return' objective:");
    }

    /**
     * Prints the current solution to the problem.
     *
     * @param title a title to print before the solution
     */
    private void printSolution(String title) {
        System.out.println(title);
        if (prob.attributes().getSolveStatus() != XPRSenumerations.SolveStatus.COMPLETED
                || prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL) {
            System.out.println("  Problem not solved");
        } else {
            System.out.println("  Objectives: " + prob.attributes().getObjectives() + " solved objectives: "
                    + prob.attributes().getSolvedObjs());

            double[] sol = prob.getSolution();
            double capitalUsed = capital.evaluate(sol);
            double roiGained = roi.evaluate(sol);
            if (capitalUsed > CAPITAL_TARGET) {
                System.out.println("  Unable to meet Capital target");
            }
            if (roiGained < ROI_TARGET) {
                System.out.println("  Unable to meet Return target");
            }

            System.out.print("  Projects undertaken:");
            for (int p = 0; p < select.length; p++) {
                if (select[p].getSolution() >= 0.99) {
                    System.out.print(" " + p);
                }
            }
            System.out.println();

            System.out.println(
                    "  Used capital: $" + capitalUsed + (CAPITAL_TARGET >= capitalUsed ? " unused: $" : " overused: $")
                            + Math.abs(CAPITAL_TARGET - capitalUsed));
            System.out.println("  Total return: $" + roiGained);
            System.out.println("  Unused personnel: " + (int) personnel.getSlack() + " persons");
        }
    }
}

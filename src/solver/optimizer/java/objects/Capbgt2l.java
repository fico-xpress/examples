// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Capital budgeting problem.
 * Illustrates logical conditions, formulation using logic constraints
 * and indicator constraints.
 */
public class Capbgt2l {

    /** A project. */
    public static final class Project {
        /** The name of this project. */
        public final String name;
        /** Payout for the project. */
        public final double payout;
        /** Capital investment for the project. */
        public final double investment;
        /** Number of personnel required for the project. */
        public final int personnel;

        public Project(String name, double payout, double investment, int personnel) {
            this.name = name;
            this.payout = payout;
            this.investment = investment;
            this.personnel = personnel;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    /** The projects used in this example. */
    private static final Project[] projectArray = new Project[] { new Project("Alpha", 124000.0, 104000.0, 22),
            new Project("Beta", 74000.0, 53000.0, 12), new Project("Gamma", 42000.0, 29000.0, 7),
            new Project("Delta", 188000.0, 187000.0, 36), new Project("Epsilon", 108000.0, 98000.0, 24),
            new Project("Zeta", 56000.0, 32000.0, 10), new Project("Eta", 88000.0, 75000.0, 20),
            new Project("Theta", 225000.0, 200000.0, 41) };

    /** The resource constraints used in this example. */
    static final double budget = 478000.0;
    static final int workforce = 106;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/

            // Whether to undertake a specific project
            Variable[] x = prob.addVariables(projectArray.length).withName(i -> String.format("x_%d", i))
                    .withType(ColumnType.Binary).toArray();

            // objective function: sum of payouts of all undertaken projects
            Expression totalProfit = sum(projectArray.length, i -> (x[i].mul(projectArray[i].payout)));

            prob.setObjective(totalProfit, XPRSenumerations.ObjSense.MAXIMIZE);

            // limits on resource availability
            // sum of investments of all undertaken projects should not exceed budget
            Inequality investmentLimit = prob
                    .addConstraint(sum(projectArray.length, i -> (x[i].mul(projectArray[i].investment))).leq(budget));
            // sum of personnel committed of all undertaken projects should not exceed
            // workforce
            Inequality workforceLimit = prob
                    .addConstraint(sum(projectArray.length, i -> (x[i].mul(projectArray[i].personnel))).leq(workforce));

            // project alpha can only be done if both gamma and zeta happen
            prob.addConstraint(x[0].leq(x[2]));
            prob.addConstraint(x[0].leq(x[5]));

            // project zeta can only be done if project epsilon happens
            prob.addConstraint(x[5].leq(x[4]));

            // projects alpha and beta as well as gamma and delta can only happen together
            prob.addConstraint(x[0].eq(x[1]));
            prob.addConstraint(x[2].eq(x[3]));

            // exactly one of those pairs should be invested in, i.e., if project alpha is
            // performed,
            // neither gamma nor delta can be invested in, and if project alpha does not
            // happen, then
            // projects gamma and delta have to be performed
            prob.addConstraint(x[0].ifThen(sum(x[2], x[3]).eq(0.0)));
            prob.addConstraint(x[0].ifNotThen(sum(x[2], x[3]).eq(2.0)));

            // Dump the problem to disk so that we can inspect it.
            prob.writeProb("capbgt2l.lp", "l");

            // Solve
            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                    && prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();

            // Print the objective
            System.out.println("Objective: " + prob.attributes().getObjVal());

            // Print the interesting slacks
            System.out.println("Remaining Budget: " + investmentLimit.getSlack());
            System.out.println("Remaining Workers: " + (int) workforceLimit.getSlack());

            // Print out the solution
            for (int i = 0; i < projectArray.length; ++i) {
                if (x[i].getValue(sol) > 0.5) {
                    System.out.println("Undertaking project " + projectArray[i].toString() + ".");
                }
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Modeling a small LP problem to perform portfolio optimization. -- Limiting
 * the total number of assets --
 */
public class FolioMip1 {
    /* Max. number of different assets */
    private static final int MAXNUM = 4;
    /* Number of shares */
    private static final int NSHARES = 10;
    /* Number of high-risk shares */
    private static final int NRISK = 5;
    /* Number of North-American shares */
    private static final int NNA = 4;
    /* Estimated return in investment */
    private static final double[] RET = new double[] { 5, 17, 26, 12, 8, 9, 7, 6, 31, 21 };
    /* High-risk values among shares */
    private static final int[] RISK = new int[] { 1, 2, 3, 8, 9 };
    /* Shares issued in N.-America */
    private static final int[] NA = new int[] { 0, 1, 2, 3 };

    private static void printProblemStatus(XpressProblem prob) {
        System.out.println(String.format("Problem status:%n\tSolve status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getSolStatus()));
    }

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/
            Variable[] frac = prob.addVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("frac_%d", i))
                    /* Upper bounds on the investment per share */
                    .withUB(0.3).toArray();

            Variable[] buy = prob.addVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("buy_%d", i)).withType(ColumnType.Binary).toArray();

            /**** CONSTRAINTS ****/
            /* Limit the percentage of high-risk values */
            prob.addConstraint(sum(NRISK, i -> frac[RISK[i]]).leq(1.0 / 3.0).setName("Risk"));

            /* Minimum amount of North-American values */
            prob.addConstraint(sum(NNA, i -> frac[NA[i]]).geq(0.5).setName("NA"));

            /* Spend all the capital */
            prob.addConstraint(sum(frac).eq(1.0).setName("Cap"));

            /* Limit the total number of assets */
            prob.addConstraint(sum(buy).leq(MAXNUM).setName("MaxAssets"));

            /* Linking the variables */
            /* frac .<= buy */
            prob.addConstraints(NSHARES, i -> frac[i].leq(buy[i]).setName(String.format("link_%d", i)));

            /* Objective: maximize total return */
            prob.setObjective(scalarProduct(frac, RET), XPRSenumerations.ObjSense.MAXIMIZE);

            /* Solve */
            prob.optimize();

            /* Solution printing */
            printProblemStatus(prob);
            System.out.println("Total return: " + prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            range(0, NSHARES).forEach(i -> System.out.println(String.format("%s : %.2f%s (%.1f)", frac[i].getName(),
                    100.0 * frac[i].getValue(sol), "%", buy[i].getValue(sol))));
        }
    }
}

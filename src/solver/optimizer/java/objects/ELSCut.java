// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.LinTermMap;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;
import com.dashoptimization.objects.XpressProblem.CallbackAPI.OptNodeCallback;

/** Economic lot sizing, ELS, problem.
 * Solved by adding (l,S)-inequalities in a branch-and-cut heuristic
 * (using the cut manager).
 *
 * ELS considers production planning over a horizon of T periods. In period t,
 * t=1,...,T, there is a given demand DEMAND[t] that must be satisfied by
 * production prod[t] in period t and by inventory carried over from previous
 * periods. There is a set-up up cost SETUPCOST[t] associated with production in
 * period t. The unit production cost in period t is PRODCOST[t]. There is no
 * inventory or stock-holding cost.
 *
 * *** This model cannot be run with a Community Licence ***
 */
public class ELSCut {

    private static final double EPS = 1e-6;
    private static final int T = 6; /* Number of time periods */

    /* Data */
    private static final double[] DEMAND = { 1, 3, 5, 3, 4, 2 }; /* Demand per period */
    private static final double[] SETUPCOST = { 17, 16, 11, 6, 9, 6 }; /* Setup cost / period */
    private static final double[] PRODCOST = { 5, 3, 2, 1, 3, 1 }; /* Prod. cost / period */
    private static double[][] D; /* Total demand in periods t1 - t2 */

    /* Variables and constraints */
    private static Variable[] prod; /* Production in period t */
    private static Variable[] setup; /* Setup in period t */

    private static void printProblemStatus(XpressProblem prob) {
        System.out.println(String.format(
                "Problem status:%n\tSolve status: %s%n\tLP status: %s%n\tMIP status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getLPStatus(), prob.attributes().getMIPStatus(),
                prob.attributes().getSolStatus()));
    }

    /** Cut generation algorithm (implemented as optnode callback).
     * - get the solution values
     * - identify and set up violated constraints
     * - add cuts to the problem
     */
    static class CutNodeCallback implements OptNodeCallback {
        public int optNode(XpressProblem p) {
            double[] sol, slack, duals, djs;
            int ncut = 0;
            // Add cut only to optimal relaxations
            if (p.attributes().getLPStatus() != XPRSenumerations.LPStatus.OPTIMAL) {
                return 0;
            }

            sol = p.getCallbackSolution();
            slack = p.getCallbackSlacks();
            duals = p.getCallbackDuals();
            djs = p.getCallbackRedCosts();
            // Search for violated constraints:
            for (int l = 0; l < T; l++) {
                double ds = 0.0;
                for (int t = 0; t <= l; t++) {
                    if (prod[t].getValue(sol) < D[t][l] * setup[t].getValue(sol) + EPS) {
                        ds += prod[t].getValue(sol);
                    } else {
                        ds += D[t][l] * setup[t].getValue(sol);
                    }
                }

                // Add the violated inequality: the minimum of the actual production
                // prod[t] and the maximum potential production D[t][l]*setup[t]
                // in periods 0 to l must at least equal the total demand in periods
                // 0 to l.
                // sum(t=1:l) min(prod[t], D[t][l]*setup[t]) >= D[0][l] */
                if (ds < D[0][l] - EPS) {
                    LinExpression cut = new LinTermMap(0);
                    for (int t = 0; t <= l; t++) {
                        if (prod[t].getValue(sol) < D[t][l] * setup[t].getValue(sol) + EPS) {
                            cut.addTerm(prod[t], 1.0);
                        } else {
                            cut.addTerm(setup[t], D[t][l]);
                        }
                    }
                    p.addCut(0, cut.geq(D[0][1]));
                    ncut++;
                }
            }

            if (ncut > 0) {
                System.out.println(String.format("Cuts added: %d (depth %d, node %d)", ncut,
                        p.attributes().getNodeDepth(), p.attributes().getNodes()));
            }
            return 0;
        }
    }

    /***********************************************************************/
    private static void modEls(XpressProblem p) {
        D = new double[T][T];
        for (int s = 0; s < T; s++)
            for (int t = 0; t < T; t++)
                for (int k = s; k <= t; k++)
                    D[s][t] += DEMAND[k];

        // Variables
        prod = p.addVariables(T).withType(ColumnType.Continuous).withName(t -> String.format("prod%d", t + 1))
                .toArray();

        setup = p.addVariables(T).withType(ColumnType.Binary).withName(t -> String.format("setup%d", t + 1)).toArray();

        // Objective: Minimize total cost
        p.setObjective(sum(scalarProduct(setup, SETUPCOST), scalarProduct(prod, PRODCOST)),
                XPRSenumerations.ObjSense.MINIMIZE);

        // Constraints

        // Production in period t must not exceed the total demand for the
        // remaining periods; if there is production during t then there
        // is a setup in t
        // for all t in [0,T[
        // prod[t] <= setup[t] * D[t][T-1]
        p.addConstraints(T, t -> prod[t].leq(setup[t].mul(D[t][T - 1])).setName(String.format("Production_%d", t)));

        // Production in periods 0 to t must satisfy the total demand
        // during this period of time
        // for all t in [0,T[
        // sum(s in [0,t+1[) prod[s] >= D[0][t]
        p.addConstraints(T, t -> sum(t + 1, s -> prod[s]).geq(D[0][t]).setName(String.format("Demand_%d", t)));
        p.writeProb("ELSCut.lp", "l");
    }

    /** Solve the model.
     * Cuts are added dynamically with the optnode callback.
     * @param p The problem to solve.
     */
    private static void solveEls(XpressProblem p) {
        p.callbacks.addMessageCallback(DefaultMessageListener::console);
        p.controls().setLPLog(0);
        p.controls().setMIPLog(3);
        // Disable automatic cuts - we use our own
        p.controls().setCutStrategy(XPRSenumerations.CutStrategy.NONE);
        // Switch presolve off
        p.controls().setPresolve(XPRSenumerations.Presolve.NONE);
        p.controls().setNodePresolve(XPRSenumerations.NodePresolve.OFF);

        /* Instantiate the cut class callback */
        CutNodeCallback cb = new CutNodeCallback();
        p.callbacks.addOptNodeCallback(cb);

        /* Solve the MIP */
        p.optimize();
        if (p.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
            throw new RuntimeException("optimization failed with status " + p.attributes().getSolStatus());
        /* Get the solution values: */
        double[] sol = p.getSolution();

        /* Print out the solution: */
        for (int t = 0; t < T; t++) {
            System.out.println(String.format("Period %d: prod %.1f (demand: %.0f, cost: %.0f), setup %.0f (cost %.0f)",
                    t + 1, prod[t].getValue(sol), DEMAND[t], PRODCOST[t], setup[t].getValue(sol), SETUPCOST[t]));
        }
        printProblemStatus(p);
    }

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            modEls(prob); // Model the problem
            solveEls(prob); // Solve the problem
        }
    }

}

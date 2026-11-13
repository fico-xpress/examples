// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.XPRSprobException;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.LinTermMap;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Cutting stock problem. The problem is solved by column (= cutting pattern)
 * generation heuristic looping over the root node.
 */
public class CuttingStock {

    private static final int NWIDTHS = 5;
    private static final int MAXWIDTH = 94;
    private static final double EPS = 1e-6;
    private static final int MAXCOL = 10;

    /**** Data ****/
    static final double[] width = { 17, 21, 22.5, 24, 29.5 }; /* Possible widths */
    static final int[] demand = { 150, 96, 48, 108, 227 }; /* Demand per width */

    /**** Shared Variables ****/
    static Variable[] vPattern; /* Rolls per pattern */
    static Inequality[] cDemand; /* Demand constraints */
    static LinTermMap eObj; /* Objective function */

    private static void printProblemSolution(XpressProblem prob) {
        double[] sol = prob.getSolution();
        System.out.println("Objective: " + prob.attributes().getObjVal());
        System.out.printf("Variables:%n\t");
        for (Variable v : prob.getVariables()) {
            System.out.print(String.format("[%s: %.2f] ", v.getName(), v.getValue(sol)));
        }
        System.out.println();
    }

    /**************************************************************************/
    /* Integer Knapsack Algorithm for solving the integer knapsack problem */
    /* z = max{cx : ax <= R, x <= d, x in Z^N} */
    /*                                                                        */
    /* Input data: */
    /* N: Number of item types */
    /* c[i]: Unit profit of item type i, i=1..n */
    /* a[i]: Unit resource use of item type i, i=1..n */
    /* R: Total resource available */
    /* d[i]: Demand for item type i, i=1..n */
    /* Return values: */
    /* xbest[i]: Number of items of type i used in optimal solution */
    /* zbest: Value of optimal solution */
    /**************************************************************************/
    private static double solveKnapsack(int N, double[] c, double[] a, double R, int[] d, int[] xbest) {
        try (XpressProblem prob = new XpressProblem()) {
            Variable[] x = prob.addVariables(N).withType(ColumnType.Integer).withName(i -> String.format("x_%d", i))
                    .withLB(i -> Double.valueOf(d[i])).toArray();

            prob.addConstraint(sum(N, i -> x[i].mul(a[i])).leq(R));

            prob.setObjective(sum(N, i -> x[i].mul(c[i])), XPRSenumerations.ObjSense.MAXIMIZE);

            prob.optimize();
            double zbest = prob.attributes().getObjVal();
            double[] sol = prob.getSolution();
            range(0, N).forEach(j -> xbest[j] = (int) Math.round(x[j].getValue(sol)));

            return zbest;
        }
    }

    private static void modCutStock(XpressProblem p) {
        // (Basic) cutting patterns
        int[][] patterns = new int[NWIDTHS][NWIDTHS];
        range(0, NWIDTHS).forEach(j -> patterns[j][j] = (int) Math.floor((double) MAXWIDTH / width[j]));

        vPattern = p.addVariables(NWIDTHS).withType(ColumnType.Integer).withLB(0)
                .withUB(i -> Math.ceil((double) demand[i] / patterns[i][i]))
                .withName(i -> String.format("pat_%d", i + 1)).toArray();

        // Minimize total number of rolls
        eObj = new LinTermMap();
        range(0, NWIDTHS).forEach(i -> eObj.addTerm(vPattern[i], 1.0));
        p.setObjective(eObj);

        // Satisfy the demand per width;
        // Sum of patterns must exceed demand
        cDemand = p
                .addConstraints(NWIDTHS,
                        i -> sum(NWIDTHS, j -> vPattern[j].mul(patterns[i][j])).geq(demand[i]).setName("Demand_" + i))
                .toArray(Inequality[]::new);
    }

    /**************************************************************************/
    /* Column generation loop at the top node: */
    /* solve the LP and save the basis */
    /* get the solution values */
    /* generate new column(s) (=cutting pattern) */
    /* load the modified problem and load the saved basis */
    /**************************************************************************/
    private static void solveCutStock(XpressProblem p) throws XPRSprobException {
        // Basis information
        int[] rowstat = new int[p.attributes().getRows()];
        int[] colstat = new int[p.attributes().getCols()];
        int[] x = new int[NWIDTHS];
        long starttime = System.currentTimeMillis();

        range(0, MAXCOL).forEach(npass -> {
            p.lpOptimize(); // Solve the LP
            if (p.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("failed to optimize with status " + p.attributes().getSolStatus());
            p.getBasis(rowstat, colstat); // Save the current basis

            /* solve integer knpsack problem */
            double z = solveKnapsack(NWIDTHS, p.getDuals(cDemand), width, MAXWIDTH, demand, x);

            System.out.println(String.format("Iteration %d, %.2f sec", npass + 1,
                    (System.currentTimeMillis() - starttime) / 1000.0));

            if (z < 1 + EPS) {
                System.out.println("No profitable column found.");
                System.out.println();
            } else {
                System.out.println("New pattern found with marginal cost " + (z - 1));
                System.out.print("   Widths distribution: ");
                /* Create a new variable for this pattern: */
                Variable v = p.addVariable(0.0, XPRSconstants.PLUSINFINITY, ColumnType.Integer,
                        String.format("pat_%d", p.attributes().getCols() + 1));
                /* Add new var. to the objective */
                v.chgObj(1.0);
                /* Add new var. to demand constraints */
                double ub = 0.0;
                for (int i = 0; i < NWIDTHS; ++i) {
                    if (x[i] > 0) {
                        cDemand[i].chgCoef(v, x[i]);
                        ub = Math.max(ub, Math.ceil(demand[i] / (double) x[i]));
                    }
                }
                /* change the upper bound on the new variable */
                v.setUB(ub);
                p.loadBasis(rowstat, colstat);
            }
        });

        p.optimize();
        if (p.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
            throw new RuntimeException("failed to optimize with status " + p.attributes().getSolStatus());
        printProblemSolution(p);
    }

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);
            prob.controls().setOutputLog(0);
            prob.controls().setMIPLog(0);

            modCutStock(prob);
            solveCutStock(prob);
        }
    }

}

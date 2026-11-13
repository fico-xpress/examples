
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
 * Modeling a small LP problem to perform portfolio optimization. -- Heuristic
 * solution --
 */
public class FolioHeuristic {
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

    public XpressProblem prob;
    /* Fraction of capital used per share */
    public Variable[] frac;
    /* 1 if asset is in portfolio, 0 otherwise */
    public Variable[] buy;

    public FolioHeuristic(XpressProblem p) {
        prob = p;
        /**** VARIABLES ****/
        frac = prob.addVariables(NSHARES)
                /* Fraction of capital used per share */
                .withName(i -> String.format("frac_%d", i))
                /* Upper bounds on the investment per share */
                .withUB(0.3).toArray();

        buy = prob.addVariables(NSHARES)
                /* Fraction of capital used per share */
                .withName(i -> String.format("buy_%d", i)).withType(ColumnType.Binary).toArray();
    }

    private static void printProblemStatus(XpressProblem prob) {
        System.out.println(String.format(
                "Problem status:%n\tSolve status: %s%n\tLP status: %s%n\tMIP status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getLPStatus(), prob.attributes().getMIPStatus(),
                prob.attributes().getSolStatus()));
    }

    private static void printProblemSolution(FolioHeuristic folio, String solveFlag) {
        XpressProblem prob = folio.prob;
        System.out.println(String.format("Total return (%s): %s", solveFlag, prob.attributes().getObjVal()));
        double[] sol = prob.getSolution();
        range(0, NSHARES).forEach(i -> System.out.println(String.format("%s: %.2f%s (%.1f)", folio.frac[i].getName(),
                100.0 * folio.frac[i].getValue(sol), "%", folio.buy[i].getValue(sol))));
    }

    private static void model(FolioHeuristic folio) {
        XpressProblem prob = folio.prob;
        // Output all messages.
        prob.callbacks.addMessageCallback(DefaultMessageListener::console);

        /**** CONSTRAINTS ****/
        /* Limit the percentage of high-risk values */
        prob.addConstraint(sum(NRISK, i -> folio.frac[RISK[i]]).leq(1.0 / 3.0).setName("Risk"));

        /* Minimum amount of North-American values */
        prob.addConstraint(sum(NNA, i -> folio.frac[NA[i]]).geq(0.5).setName("NA"));

        /* Spend all the capital */
        prob.addConstraint(sum(folio.frac).eq(1.0).setName("Cap"));

        /* Limit the total number of assets */
        prob.addConstraint(sum(folio.buy).leq(MAXNUM).setName("MaxAssets"));

        /* Linking the variables */
        prob.addConstraints(NSHARES, i -> folio.frac[i].leq(folio.buy[i]).setName(String.format("link_%d", i)));

        /* Objective: maximize total return */
        prob.setObjective(scalarProduct(folio.frac, RET), XPRSenumerations.ObjSense.MAXIMIZE);
    }

    private static void solveHeuristic(FolioHeuristic folio) {
        XpressProblem p = folio.prob;
        /* Disable automatic cuts */
        p.controls().setCutStrategy(XPRSenumerations.CutStrategy.NONE);
        // Switch presolve off
        p.controls().setPresolve(XPRSenumerations.Presolve.NONE);
        p.controls().setNodePresolve(XPRSenumerations.NodePresolve.OFF);
        /* Get feasibility tolerance */
        double tol = p.controls().getFeasTol();

        /* Solve the LP-problem */
        p.lpOptimize();

        /* Get Solution */
        double[] sol = p.getSolution();

        /* Basis information */
        int[] rowstat = new int[p.attributes().getRows()];
        int[] colstat = new int[p.attributes().getCols()];
        /* Save the current basis */
        p.getBasis(rowstat, colstat);

        /*
         * Fix all variables `buy' for which `frac' is at 0 or at a relatively large
         * value
         */
        double[] fsol = new double[NSHARES];
        range(0, NSHARES).forEach(i -> {
            /* Get the solution values of `frac' */
            fsol[i] = folio.frac[i].getValue(sol);
            if (fsol[i] < tol)
                folio.buy[i].fix(0);
            else if (fsol[i] > 0.2 - tol)
                folio.buy[i].fix(1);
        });

        /* Solve with the new bounds on 'buy' */
        p.mipOptimize();

        printProblemStatus(p);
        printProblemSolution(folio, "Heuristic solution");

        /* Reset variables to their original bounds */
        range(0, NSHARES).forEach(i -> {
            if ((fsol[i] < tol) || (fsol[i] > 0.2 - tol)) {
                folio.buy[i].setLB(0);
                folio.buy[i].setUB(1);
            }
        });

        /* Load basis */
        p.loadBasis(rowstat, colstat);
    }

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            /* Solve with heuristic */
            FolioHeuristic folio = new FolioHeuristic(prob);
            model(folio);
            solveHeuristic(folio);
        }

        try (XpressProblem prob = new XpressProblem()) {
            FolioHeuristic folio = new FolioHeuristic(prob);
            model(folio);
            /* Solve */
            folio.prob.optimize();
            /* Solution printing */
            printProblemStatus(prob);
            printProblemSolution(folio, "Exact Solve");
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.ConstantExpression.constant;
import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.LinTermMap;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Recursion solving a non-linear financial planning problem. The problem is to
 * solve
 * <pre>
 *     net(t) = Payments(t) - interest(t)
 *     balance(t) = balance(t-1) - net(t)
 *     interest(t) = (92/365) * balance(t) * interest_rate
 *   where
 *     balance(0) = 0
 *     balance[T] = 0
 *   for interest_rate
 * <pre>
 */
public class RecursiveFinancialPlanning {

    private static final int T = 6;

    /* Data */
    /* An INITIAL GUESS as to interest rate x */
    private static double X = 0.00;
    /* An INITIAL GUESS as to balances b(t) */
    private static final double[] B = { 1, 1, 1, 1, 1, 1 };
    private static final double[] P = { -1000, 0, 0, 0, 0, 0 }; /* Payments */
    private static final double[] R = { 206.6, 206.6, 206.6, 206.6, 206.6, 0 }; /* " */
    private static final double[] V = { -2.95, 0, 0, 0, 0, 0 }; /* " */

    /* Variables and constraints */
    static Variable[] b; /* Balance */
    static Variable x; /* Interest rate */
    static Variable dx; /* Change to x */
    static Inequality[] interest;
    static Inequality ctrd;

    private static void printIteration(XpressProblem prob, int it, double variation) {
        double[] sol = prob.getSolution();
        System.out.println(String.format("---------------- Iteration %d ----------------", it));
        System.out.println(String.format("Objective: %.2f", prob.attributes().getObjVal()));
        System.out.println(String.format("Variation: %.2f", variation));
        System.out.println(String.format("x: %.2f", x.getValue(sol)));
        System.out.println(String.format("----------------------------------------------", it));
    }

    private static void printProblemSolution(XpressProblem prob) {
        double[] sol = prob.getSolution();
        System.out.println(String.format("Objective: %.2f", prob.attributes().getObjVal()));
        System.out.println(String.format("Interest rate: %.2f percent", x.getValue(sol) * 100));
        System.out.printf("Variables:%n\t");
        for (Variable v : prob.getVariables()) {
            System.out.print(String.format("[%s: %.2f] ", v.getName(), v.getValue(sol)));
        }
        System.out.println();
    }

    /***********************************************************************/
    private static void modFinNLP(XpressProblem p) {
        interest = new Inequality[T];

        // Balance
        b = p.addVariables(T).withType(ColumnType.Continuous).withName(t -> String.format("b_%d", t))
                .withLB(XPRSconstants.MINUSINFINITY).toArray();

        // Interest rate
        x = p.addVariable(0.0, XPRSconstants.PLUSINFINITY, ColumnType.Continuous, "x");

        // Interest rate change
        dx = p.addVariable(XPRSconstants.MINUSINFINITY, XPRSconstants.PLUSINFINITY, ColumnType.Continuous, "dx");

        Variable[] i = p.addVariables(T).withType(ColumnType.Continuous).withName(t -> String.format("i_%d", t))
                .toArray();

        Variable[] n = p.addVariables(T).withType(ColumnType.Continuous).withName(t -> String.format("n_%d", t))
                .withLB(XPRSconstants.MINUSINFINITY).toArray();

        Variable[] epl = p.addVariables(T).withType(ColumnType.Continuous).withName(t -> String.format("epl_%d", t))
                .toArray();

        Variable[] emn = p.addVariables(T).withType(ColumnType.Continuous).withName(t -> String.format("emn_%d", t))
                .toArray();

        // Fixed variable values
        i[0].setLB(0).setUB(0);
        b[T - 1].setLB(0).setUB(0);

        // Objective
        p.setObjective(sum(sum(epl), sum(emn)), XPRSenumerations.ObjSense.MINIMIZE);

        // Constraints
        // net = payments - interest
        p.addConstraints(T,
                t -> n[t].eq(sum(constant(P[t] + R[t] + V[t]), i[t].mul(-1))).setName(String.format("net_%d", t)));

        // Money balance across periods
        p.addConstraints(T, t -> b[t].eq(t > 0 ? b[t - 1] : constant(0.0)).setName(String.format("bal_%d", t)));

        // i(t) = (92/365)*( b(t-1)*X + B(t-1)*dx ) approx.
        range(1, T).forEach(t -> {
            LinExpression iepx = new LinTermMap();
            iepx.addTerm(b[t - 1], X);
            iepx.addTerm(dx, B[t - 1]);
            iepx.addTerm(epl[t], 1.0);
            iepx.addTerm(emn[t], 1.0);
            interest[t] = p.addConstraint(i[t].mul(365 / 92.0).eq(iepx).setName(String.format("int_%d", t)));
        });

        // x = dx + X
        ctrd = p.addConstraint(x.eq(sum(dx, constant(X)))).setName("def");
        p.writeProb("Recur.lp", "l");
    }

    /**************************************************************************/
    /* Recursion loop (repeat until variation of x converges to 0): */
    /* save the current basis and the solutions for variables b[t] and x */
    /* set the balance estimates B[t] to the value of b[t] */
    /* set the interest rate estimate X to the value of x */
    /* reload the problem and the saved basis */
    /* solve the LP and calculate the variation of x */
    /**************************************************************************/
    private static void solveFinNLP(XpressProblem p) {
        double variation = 1.0;

        // Switch automatic cut generation off
        p.controls().setCutStrategy(XPRSconstants.CUTSTRATEGY_NONE);
        // Solve the problem
        p.lpOptimize();
        if (p.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
            throw new RuntimeException("failed to optimize with status " + p.attributes().getSolStatus());

        for (int it = 1; variation > 1e-6; ++it) {
            // Optimization solution
            final double[] sol = p.getSolution();

            printIteration(p, it, variation);
            printProblemSolution(p);
            // Change coefficients in interest[t]
            // Note: when inequalities are added to a problem then all variables are moved
            // to
            // the left-hand side and all constants are moved to the right-hand side. Since
            // we
            // are changing these extracted inequalities directly, we have to use negative
            // coefficients below.
            range(1, T).forEach(t -> {
                p.chgCoef(interest[t], dx, -b[t - 1].getValue(sol));
                p.chgCoef(interest[t], b[t - 1], -x.getValue(sol));
            });

            // Change constant term of ctrd
            ctrd.setRhs(x.getValue(sol));

            // Solve the problem
            p.optimize();
            double[] newsol = p.getSolution();
            if (p.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("failed to optimize with status " + p.attributes().getSolStatus());
            variation = Math.abs(x.getValue(newsol) - x.getValue(sol));
        }
        printProblemSolution(p);
    }

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);
            prob.controls().setMIPLog(0);

            modFinNLP(prob);
            solveFinNLP(prob);
        }
    }
}

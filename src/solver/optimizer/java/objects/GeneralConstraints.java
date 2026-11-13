// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import java.util.Locale;

import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * A simple example that formulates some constraints on the minimum and absolute
 * values of linear combinations of variables.
 */
public class GeneralConstraints {
    public static void main(String[] args) {
        System.out.println("Formulating the general constraint example problem");

        final int R = 3;

        try (XpressProblem prob = new XpressProblem()) {
            Variable[] x = prob.addVariables(R).withName("x_%d").withUB(20).toArray();

            Variable y = prob.addVariable("y");
            Variable z = prob.addVariable("z");

            Expression objective = sum(x);

            // We want to formulate abs(x(0)-2*x(1)) <= 10. We need to introduce
            // two auxiliary variables for the argument of the abs function
            // and then break down the abs expression in multiple steps
            Variable diff1 = prob.addVariable("diff1");
            Variable absOfDiff1 = prob.addVariable("absOfDiff1");

            prob.addConstraint(diff1.eq(x[0].minus(x[1].mul(2))));
            prob.addConstraint(absOfDiff1.absOf(diff1));
            prob.addConstraint(absOfDiff1.leq(10));

            // We link a new variable to the minimum of the x(i) and
            // require this variable to be >= 5
            // Clearly, this bound constraint could also be achieved by simply setting
            // the bounds of each x variable.
            Variable minOfX = prob.addVariable("minOfX");
            prob.addConstraint(minOfX.minOf(x));
            prob.addConstraint(minOfX.geq(5));

            // We link variable y to the maximum of other variables, expressions, and
            // constants
            // y = max(x(2), 20, x(0)-z)
            Variable diff2 = prob.addVariable("diff2");
            prob.addConstraint(diff2.eq(x[0].minus(z)));

            // the below code is equivalent to using the MaxOf function on the resultant y
            // prob.addConstraint(y.MaxOf(new Variable[] {x[2], diff2},
            // 20).setName("max_constraint"));
            prob.addConstraint(y.maxOf(new Variable[] { x[2], diff2 }, new double[] { 20 }, "max_constraint"));

            // set objective function with a maximization sense
            prob.setObjective(objective, XPRSenumerations.ObjSense.MAXIMIZE);

            // write the problem in LP format for manual inspection
            System.out.println("Writing the problem to 'GeneralConstraints.lp'");
            prob.writeProb("GeneralConstraints.lp", "l");

            // Solve the problem
            System.out.println("Solving the problem");
            prob.optimize();

            // check the solution status
            System.out.println("Problem finished with SolStatus " + prob.attributes().getSolStatus());
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL) {
                throw new RuntimeException("Problem not solved to optimality");
            }

            // print the optimal solution of the problem to the console
            System.out.printf(Locale.US, "Solution has objective value (profit) of %g%n",
                    prob.attributes().getObjVal());
            System.out.println("");
            System.out.println("*** Solution ***");
            double[] sol = prob.getSolution();

            for (int r = 0; r < R; r++) {
                String delim = r < R - 1 ? ", " : "\n";
                System.out.printf(Locale.US, "x_%d = %g%s", r, x[r].getValue(sol), delim);
            }
            System.out.printf(Locale.US, "y = %g, z = %g%n", y.getValue(sol), z.getValue(sol));
            System.out.printf(Locale.US, "ABS ( x[0] - 2*x[1] ) = %g, minOfX = %g%n", absOfDiff1.getValue(sol),
                    minOfX.getValue(sol));
            System.out.println("");
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.SOS.sos2;
import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;

import java.util.Locale;

import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Approximation of a nonlinear function by a special ordered set (SOS-2). An
 * SOS-2 is a constraint that allows at most 2 of its variables to have a
 * nonzero value. In addition, these variables have to be adjacent.
 *
 * - Example discussed in mipformref whitepaper -
 */
public class SpecialOrderedSets {
    public static void main(String[] args) {

        final int NB = 4; // number of breakpoints
        double[] B_X = // X coordinates of breakpoints
                { 1, 2.5, 4.5, 6.5 };

        double[] B_Y = // Y coordinates of breakpoints
                { 1.5, 6, 3.5, 2.5 };

        System.out.println("Formulating the special ordered sets example problem");
        try (XpressProblem prob = new XpressProblem()) {
            // create one w variable for each breakpoint. We express
            Variable[] w = prob.addVariables(NB).withName("w_%d").withUB(1).toArray();

            Variable x = prob.addVariable("x");
            Variable y = prob.addVariable("y");

            // Define the SOS-2 with weights from B_X. This is necessary
            // to establish the ordering between the w variables.
            prob.addConstraint(sos2(w, B_X, "SOS_2"));

            // We use the w variables to express a convex combination.
            // In combination with the above SOS-2 condition,
            // X and Y are represented as a convex combination of 2 adjacent
            // breakpoints.
            prob.addConstraint(sum(w).eq(1));

            // The below constraints express the actual locations of X and Y
            // in the plane as a convex combination of the breakpoints, subject
            // to the assignment found for the w variables.
            prob.addConstraint(x.eq(scalarProduct(w, B_X)));
            prob.addConstraint(y.eq(scalarProduct(w, B_Y)));

            // set lower and upper bounds on x
            x.setLB(2);
            x.setUB(6);

            // set objective function with a minimization sense
            prob.setObjective(y, XPRSenumerations.ObjSense.MINIMIZE);

            // write the problem in LP format for manual inspection
            System.out.println("Writing the problem to 'SpecialOrderedSets.lp'");
            prob.writeProb("SpecialOrderedSets.lp", "l");

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
            System.out.println("*** Solution ***");
            double[] sol = prob.getSolution();

            for (int b = 0; b < NB; b++) {
                String delim = b < NB - 1 ? ", " : System.lineSeparator();
                System.out.printf(Locale.US, "w_%d = %g%s", b, w[b].getValue(sol), delim);
            }

            System.out.printf(Locale.US, "x = %g, y = %g%n", x.getValue(sol), y.getValue(sol));
        }
    }
}

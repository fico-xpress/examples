// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.SOS.sos2;
import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;

import java.util.Locale;

import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Approximation of a quadratic function in 2 variables by special ordered sets
 * (SOS-2). An SOS-2 is a constraint that allows at most 2 of its variables to
 * have a nonzero value. In addition, these variables have to be adjacent.
 *
 * - Example discussed in mipformref whitepaper -
 */
public class SpecialOrderedSetsQuadratic {
    public static void main(String[] args) {

        final int NX = 10; // number of breakpoints on the X-axis
        final int NY = 10; // number of breakpoints on the Y-axis
        double[] X = // X coordinates of grid points
                new double[NX];

        double[] Y = // Y coordinates of breakpoints
                new double[NY];

        double[][] F_XY = // two dimensional array of function values on the grid points
                new double[NX][NY];

        // assign the toy data
        for (int i = 0; i < NX; i++)
            X[i] = i + 1;
        for (int i = 0; i < NY; i++)
            Y[i] = i + 1;
        for (int i = 0; i < NX; i++)
            for (int j = 0; j < NY; j++)
                F_XY[i][j] = (X[i] - 5) * (Y[j] - 5);

        System.out.println("Formulating the special ordered sets quadratic example problem");
        try (XpressProblem prob = new XpressProblem()) {
            // create one w variable for each X breakpoint. We express
            Variable[] wx = prob.addVariables(NX).withName("wx_%d")
                    // this upper bound i redundant because of the convex combination constraint on
                    // the sum of the wx
                    .withUB(1).toArray();
            // create one w variable for each Y breakpoint. We express
            Variable[] wy = prob.addVariables(NY).withName("wy_%d")
                    // this upper bound i redundant because of the convex combination constraint on
                    // the sum of the wy
                    .withUB(1).toArray();

            // create a two-dimensional array of w variable for each grid point. We express
            Variable[][] wxy = prob.addVariables(NX, NY).withName("wxy_%d_%d")
                    // this upper bound is redundant because of the convex combination constraint on
                    // the sum of the wy
                    .withUB(1).toArray();

            Variable x = prob.addVariable("x");
            Variable y = prob.addVariable("y");
            Variable fxy = prob.addVariable("fxy");

            // make fxy a free variable
            fxy.setLB(Double.NEGATIVE_INFINITY);

            // Define the SOS-2 constraints with weights from X and Y.
            // This is necessary to establish the ordering between
            // variables in wx and in wy.
            prob.addConstraint(sos2(wx, X, "SOS_2_X"));
            prob.addConstraint(sos2(wy, Y, "SOS_2_Y"));
            prob.addConstraint(sum(wx).eq(1));
            prob.addConstraint(sum(wy).eq(1));

            // link the wxy variables to their 1-dimensional colleagues
            prob.addConstraints(NX, i -> (wx[i].eq(sum(NY, j -> wxy[i][j]))));
            prob.addConstraints(NY, j -> (wy[j].eq(sum(NX, i -> wxy[i][j]))));

            // now express the actual x, y, and f(x,y) coordinates
            prob.addConstraint(x.eq(scalarProduct(wx, X)));
            prob.addConstraint(y.eq(scalarProduct(wy, Y)));
            prob.addConstraint(fxy.eq(sum(NX, i -> sum(NY, j -> wxy[i][j].mul(F_XY[i][j])))));

            // set lower and upper bounds on x and y
            x.setLB(2);
            x.setUB(10);
            y.setLB(2);
            y.setUB(10);

            // set objective function with a minimization sense
            prob.setObjective(fxy, XPRSenumerations.ObjSense.MINIMIZE);

            // write the problem in LP format for manual inspection
            System.out.println("Writing the problem to 'SpecialOrderedSetsQuadratic.lp'");
            prob.writeProb("SpecialOrderedSetsQuadratic.lp", "l");

            // Solve the problem
            System.out.println("Solving the problem");
            prob.optimize();

            // check the solution status
            System.out.println("Problem finished with SolStatus " + prob.attributes().getSolStatus());
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL) {
                throw new RuntimeException("Problem not solved to optimality");
            }

            // print the optimal solution of the problem to the console
            System.out.printf(Locale.US, "Solution has objective value (profit) of %g%n%n",
                    prob.attributes().getObjVal());
            System.out.println("*** Solution ***");
            double[] sol = prob.getSolution();

            for (int i = 0; i < NX; i++) {
                String delim = i < NX - 1 ? ", " : System.lineSeparator();
                System.out.printf(Locale.US, "wx_%d = %g%s", i, wx[i].getValue(sol), delim);
            }
            for (int j = 0; j < NY; j++) {
                String delim = j < NY - 1 ? ", " : System.lineSeparator();
                System.out.printf(Locale.US, "wy_%d = %g%s", j, wy[j].getValue(sol), delim);
            }

            System.out.printf(Locale.US, "x = %g, y = %g%n", x.getValue(sol), y.getValue(sol));
        }
    }
}

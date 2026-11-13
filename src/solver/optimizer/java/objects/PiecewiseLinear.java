// (c) 2023-2026 Fair Isaac Corporation

import java.util.Locale;

import com.dashoptimization.PwlBreakpoint;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * An example that demonstrates how to model a piecewise linear cost function. A
 * piecewise linear cost function f(x) assigns a different linear cost function
 * for different intervals of the domain of its argument x. This situation
 * occurs in real-world problems if there are price discounts available starting
 * from a certain quantity of sold goods.
 *
 * - Example discussed in mipformref whitepaper -
 */
public class PiecewiseLinear {
    public static void main(String[] args) {
        System.out.println("Formulating the piecewise linear example problem");

        try (XpressProblem prob = new XpressProblem()) {
            Variable x = prob.addVariable("x");
            Variable fx = prob.addVariable("fx");
            Expression objective = fx;

            double[] BREAKPOINT_X = // the breakpoints x values
                    { 0, 50, 50, 120, 120, 200 };

            double[] BREAKPOINT_Y = // the breakpoints y values
                    { 0, 50, 75, 180, 240, 400 };

            // Create the break points from the x,y data
            PwlBreakpoint[] breakpoints = new PwlBreakpoint[BREAKPOINT_X.length];
            for (int i = 0; i < breakpoints.length; ++i)
                breakpoints[i] = new PwlBreakpoint(BREAKPOINT_X[i], BREAKPOINT_Y[i]);

            // add the piecewise linear constraints with the breakpoints
            prob.addConstraint(fx.pwlOf(x, breakpoints, "pwl_with_breakpoints"));

            // ! Add a lower bound on x to get a somewhat more meaningful model
            // x >= 150
            x.setLB(150);

            // set objective function with a minimization sense
            prob.setObjective(objective, XPRSenumerations.ObjSense.MINIMIZE);

            // write the problem in LP format for manual inspection
            System.out.println("Writing the problem to 'PiecewiseLinear.lp'");
            prob.writeProb("PiecewiseLinear.lp", "l");

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

            System.out.printf(Locale.US, "x = %g, fx = %g%n", x.getValue(sol), fx.getValue(sol));

            System.out.println("");
        }
    }
}

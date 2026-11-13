// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations.ObjSense;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.LinTermList;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * QCQP problem (linear objective, convex quadratic constraints) Based on AMPL
 * model catenary.mod (Source:
 * http://www.orfe.princeton.edu/~rvdb/ampl/nlmodels/) This model finds the
 * shape of a hanging chain by minimizing its potential energy.
 */
public class Catenary {
    static final int N = 100; // Number of chainlinks
    static final int L = 1; // Difference in x-coordinates of endlinks

    static final double H = 2.0 * L / N; // Length of each link

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            int i;
            Variable[] x, y;
            LinExpression obj;

            ///// VARIABLES
            x = prob.addVariables(N + 1).withName("x(%s)").withLB(XPRSconstants.MINUSINFINITY).toArray();

            y = prob.addVariables(N + 1).withName("y(%s)").withLB(XPRSconstants.MINUSINFINITY).toArray();

            // Bounds: positions of endpoints
            // Left anchor
            x[0].fix(0);
            y[0].fix(0);
            // Right anchor
            x[N].fix(L);
            y[N].fix(0);

            ///// OBJECTIVE

            /* Minimise the potential energy: sum(j in 1..N) (y(j-1)+y(j))/2 */
            obj = new LinTermList();
            for (i = 1; i <= N; i++) {
                obj.addTerm(y[i - 1], 0.5).addTerm(y[i], 0.5);
            }
            prob.setObjective(obj, ObjSense.MINIMIZE);

            ///// CONSTRAINTS

            /*
             * Positions of chainlinks: forall(j in 1..N) (x(j)-x(j-1))^2+(y(j)-y(j-1))^2 <=
             * H^2
             */
            for (i = 1; i <= N; i++) {
                prob.addConstraint(sum(LinExpression.create().addTerm(x[i]).addTerm(x[i - 1], -1).square(),
                        LinExpression.create().addTerm(y[i]).addTerm(y[i - 1], -1).square()).leq(H * H)
                        .setName("Link_" + i));
            }

            ///// SOLVING + OUTPUT
            prob.optimize(""); // Solve the problem

            System.out.println("Solution: " + prob.attributes().getObjVal());
            for (i = 0; i <= N; i++) {
                System.out.println(i + ": " + x[i].getSolution() + ", " + y[i].getSolution());
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations.LPStatus;
import com.dashoptimization.XPRSenumerations.ObjSense;
import com.dashoptimization.objects.QuadExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Small Quadratic Programming example.
 *
 * <pre>
 *   minimize x1 + x1^2 +2x1x2 +2x2^2 +x4^2
 *   s.t.
 *     C1:  x1 +2x2 -4x4 &gt;= 0
 *     C2: 3x1 -2x3 - x4 &lt;= 100
 *     C3: 10 &lt;= x1 +3x2 +3x3 -2x4 &lt;= 30
 *     0 &lt;= x1 &lt;= 20
 *     0 &lt;= x2,x3
 *     x4 free
 * </pre>
 */
public class QuadraticProgramming {
    static final int N = 4;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            QuadExpression obj;
            Variable[] x;

            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            ///// VARIABLES
            x = new Variable[N];
            x[0] = prob.addVariable(0, 20, ColumnType.Continuous, "x1");
            x[1] = prob.addVariable("x2");
            x[2] = prob.addVariable("x3");
            x[3] = prob.addVariable(Double.NEGATIVE_INFINITY, Double.POSITIVE_INFINITY, ColumnType.Continuous, "x4");

            ///// OBJECTIVE
            obj = QuadExpression.create();
            obj.addTerm(x[0], 1);
            obj.addTerm(x[0].mul(x[0]));
            obj.addTerm(x[0], x[1], 2);
            obj.addTerm(x[1].mul(x[1]).mul(2));
            obj.addTerm(x[3].square());
            prob.setObjective(obj, ObjSense.MINIMIZE);

            ///// CONSTRAINTS
            prob.addConstraint(sum(x[0], x[1].mul(2), x[3].mul(-4)).geq(0).setName("C1"));
            prob.addConstraint(sum(x[0].mul(3), x[2].mul(-2), x[3].mul(-1)).leq(100).setName("C2"));

            prob.addConstraint(sum(x[0], x[1].mul(3), x[2].mul(3), x[3].mul(-2)).in(10, 30).setName("C3"));

            ///// SOLVING + OUTPUT
            prob.writeProb("qp.lp");
            prob.optimize();

            System.out.println("Problem status: " + prob.attributes().getLPStatus());
            if (prob.attributes().getLPStatus() != LPStatus.OPTIMAL)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getLPStatus());

            System.out.println("Objective function value: " + prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            for (int i = 0; i < N; i++)
                System.out.print(x[i].getName() + ": " + x[i].getValue(sol) + ", ");
            System.out.println();
        }
    }
}

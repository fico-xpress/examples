import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.IntHolder;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations.ObjSense;
import com.dashoptimization.XPRSprob;
import com.dashoptimization.XPRSprob.VecMapFunction;
import com.dashoptimization.XPRSprob.VecMapFunctor;

import java.util.ArrayList;


/** Code example that uses a user function of type "vecmap".
<pre>
   Xpress Optimizer Examples
   =========================

   Maximize the area of polygon of N vertices and diameter of 1
   The position of vertices is indicated as (rho,theta) coordinates
   where rho denotes the distance to the base point
   (vertex with number N) and theta the angle from the x-axis.

   (c) 2021-2026 Fair Isaac Corporation
</pre>

   Polygon example: maximise the area of an N sided polygon

   *** Demonstrating using a simple map (R^2-&gt;R) userfunction ***

<pre>
   Variables:

    rho   : 0..N-1   ! Distance of vertex from the base point
    theta : 0..N-1   ! Angle from x-axis

   Objective:
     (sum (i in 1..N-2) (rho(i)*rho(i-1)*sin(theta(i)-theta(i-1)))) * 0.5

   Constraints:
     Vertices in increasing degree order:
        theta(i) >= theta(i-1) +.0001  : i = 1..N-2
     Boundary conditions:
        theta(N-1) <= Pi
        0.1 <= rho(i) <= 1   : i = 0..N-2
     Third side of all triangles <= 1
        rho(i)^2 + rho(j)^2 - rho(i)*rho(j)*2*cos(theta(j)-theta(i)) <= 1    : i in 0..N-3, j in i..N-2
</pre>
 *
 * In this example we create two user functions that take two arguments each.
 * One computes the sine of the difference of its arguments, the other computes
 * the cosine of the difference of its arguments.
 */

public final class PolygonVecMap {

    /** User function that maps two doubles to a double.
     * Computes <code>sin(x[0] - x[1])</code>.
     */
    private static final VecMapFunctor mySinDiff = new VecMapFunctor() {
            @Override
            public double map(double[] value) {
                return Math.sin(value[0] - value[1]);
            }
        };
    /** Function that maps two doubles to a double.
     * Computes <code>cos(x[0] - x[1])</code>.
     * As opposed to {@link #mySinDiff} this is implemented as a (plain)
     * function and added using PolygonVecMap::myCosDiff later.
     * The same could be done with instance methods as well.
     */
    private static double myCosDiff(double[] value) {
        return Math.cos(value[0] - value[1]);
    }

    public static void main(String[] args) {
        try (XPRSprob prob = new XPRSprob(null)) {
            prob.addMessageListener(new DefaultMessageListener());

            // Number of sides of the Polygon
            int nSide = 5;

            // Theta
            int[] theta = prob.addColumns(nSide - 1)
                .withUB(Math.PI)
                .withName(i -> String.format("THETA%d", i + 1))
                .toArray();
            // Rho
            int[] rho = prob.addColumns(nSide - 1)
                .withLB(0.01)
                .withUB(1.0)
                .withName(i -> String.format("RHO%d", i + 1))
                .toArray();

            // Add the user functions.
            // Both user functions take 2 input arguments. This must be
            // specified here.
            VecMapFunction sinDiff = prob.nlpAddUserFunction("mySinDiff", 2, 0, mySinDiff);
            VecMapFunction cosDiff = prob.nlpAddUserFunction("myCosDiff", 2, 0, PolygonVecMap::myCosDiff);

            // Objective function. We build the objective function as
            // a formula in infix notation. See below for submitting a
            // formula as string.
            // Tokens are always integers, while values may be integers
            // (for example operator or delimiter constants) or double
            // values (actual numbers). That is why the `val` list has
            // elements of type Number.
            ArrayList<Integer> tok = new ArrayList<Integer>();
            ArrayList<Number> val = new ArrayList<Number>();
            for (int i = 1; i < nSide - 1; ++i) {
                if ( tok.size() > 0 ) {
                    tok.add(XPRSconstants.TOK_OP);
                    val.add(XPRSconstants.OP_PLUS);
                }
                tok.add(XPRSconstants.TOK_COL);             // RHO(i)
                val.add(rho[i]);
                tok.add(XPRSconstants.TOK_OP);              // *
                val.add(XPRSconstants.OP_MULTIPLY);
                tok.add(XPRSconstants.TOK_COL);             // RHO(i-1)
                val.add(rho[i-1]);
                tok.add(XPRSconstants.TOK_OP);              // *
                val.add(XPRSconstants.OP_MULTIPLY);
                tok.add(XPRSconstants.TOK_FUN);             // mySinDiff
                val.add(sinDiff.getId());
                tok.add(XPRSconstants.TOK_LB);              // (
                val.add(XPRSconstants.TOK_LB);
                tok.add(XPRSconstants.TOK_COL);             // THETA(i)
                val.add(theta[i]);
                tok.add(XPRSconstants.TOK_DEL);             // ,
                val.add(XPRSconstants.DEL_COMMA);
                tok.add(XPRSconstants.TOK_COL);             // THETA(i-1)
                val.add(theta[i-1]);
                tok.add(XPRSconstants.TOK_RB);              // )
                val.add(XPRSconstants.TOK_RB);
                tok.add(XPRSconstants.TOK_OP);              // *
                val.add(XPRSconstants.OP_MULTIPLY);
                tok.add(XPRSconstants.TOK_CON);             // 0.5
                val.add(0.5);
            }
            tok.add(XPRSconstants.TOK_EOF);
            val.add(0.0);
            // Since nonlinear objectives cannot be directly expressed in Xpress, we maximize a free
            // variable objx and constrain this variable to be equal to the nonlinear objective.
            int objx = prob.addColumn(XPRSconstants.MINUSINFINITY, XPRSconstants.PLUSINFINITY, ColumnType.Continuous, null);
            prob.chgObj(objx, 1.0);
            int objeq = prob.addRow(new int[]{objx}, new double[]{-1.0}, 'E', 0.0);
            prob.nlpChgFormula(objeq, 0,
                               tok.stream().mapToInt(i -> i).toArray(),
                               val.stream().mapToDouble(d -> d.doubleValue()).toArray());
            prob.chgObjSense(ObjSense.MAXIMIZE);

            // Vertices in increasing degree order
            for (int i = 0; i < nSide - 2; ++i)
                prob.addRow(new int[]{ theta[i], theta[i+1] },
                            new double[]{ -1.0, 1.0 },
                            'G',
                            0.001);

            // Third side of all triangles <= 1
            for (int i = 1; i < nSide - 1; i++) {
                for (int j = i + 1; j < nSide; j++) {
                    int row = prob.addRow(new int[0], new double[0], 'L', 1.0);
                    prob.nlpChgFormulaStr(row,
                        String.format("RHO%d ^ 2 + RHO%d ^ 2 - RHO%d * RHO%d * 2 * myCosDiff ( THETA%d , THETA%d )",
                        i, j, i, j, j, i));
                }
            }

            //prob.writeProb("vecmap.lp", "l");
            IntHolder solvestatus = new IntHolder();
            IntHolder solstatus = new IntHolder();
			//solve problem to local optimality
			prob.controls().setNLPSolver(XPRSconstants.NLPSOLVER_LOCAL);
            prob.optimize("s", solvestatus, solstatus);
            System.out.printf("solvestatus: %s%n", solvestatus);
            System.out.printf("solstatus:   %s%n", solstatus);
            System.out.printf("Solution value: %f%n", prob.attributes().getObjVal());

            double[] x = prob.getSolution();
            for (int i = 0; i < rho.length; ++i)
                System.out.printf("RHO%d = %f%n", i + 1, x[rho[i]]);
            for (int i = 0; i < theta.length; ++i)
                System.out.printf("THETA%d = %f%n", i + 1, x[theta[i]]);

        }
    }
}

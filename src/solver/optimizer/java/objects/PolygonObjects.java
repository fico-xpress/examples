// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.cos;
import static com.dashoptimization.objects.Utils.sin;
import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Maximize the area of a polygon of N vertices and a diameter of 1. The
 * position of vertices is indicated as (r,theta) coordinates where r denotes
 * the distance to the base point (vertex with number N) and theta the angle
 * from the x-axis.
 */
public class PolygonObjects {

    // the number of vertices/sides of the polygon
    private static final int NSIDES = 5;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/

            // r corresponds to the distance from the base point to vertex i
            Variable[] r = prob.addVariables(NSIDES - 1).withName(i -> String.format("r_%d", i)).withUB(1.0).toArray();

            // theta corresponds to the angle relative to the x-axis of vertex i
            Variable[] theta = prob.addVariables(NSIDES - 1).withName(i -> String.format("theta_%d", i)).withUB(Math.PI)
                    .toArray();

            /**** OBJECTIVE ****/

            // objective transfer column
            Variable objtransfercol = prob.addVariable(XPRSconstants.MINUSINFINITY, XPRSconstants.PLUSINFINITY,
                    ColumnType.Continuous, "objTransferCol");

            // Set objective: maximize area of the polygon: r_i*r_j*sin(theta_i+1-theta_i)/2
            Expression area = sum(NSIDES - 2, i -> r[i].mul(r[i + 1]).mul(sin(theta[i + 1].minus(theta[i]))).div(2));

            // To make the objective linear, just maximize the objtransfercol
            prob.setObjective(objtransfercol, XPRSenumerations.ObjSense.MAXIMIZE);

            // Add the objective transfer row: area = objtransfercol
            prob.addConstraint(area.eq(objtransfercol));

            /**** CONSTRAINTS ****/

            // any two non-origin nodes should have a distance <= 1 to satisfy the diameter
            for (int i = 0; i < NSIDES - 1; i++) {
                for (int j = i + 1; j < NSIDES - 1; j++) {
                    // r_i^2 + r_j^2 - 2 * r_i * r_j * cos(theta_j - theta_i) <= 1
                    prob.addConstraint(r[i].square().plus(r[j].square())
                            .minus(r[i].mul(r[j]).mul(cos(theta[j].minus(theta[i])))).leq(1.0));
                }
            }

            // Ordering of the vertices: theta_i+1 >= theta_i
            prob.addConstraints(NSIDES - 2, i -> theta[i + 1].geq(theta[i]));

            // Dump the problem to disk so that we can inspect it.
            prob.writeProb("polygon.lp", "lp");

            // Solve to local optimality
            prob.controls().setNLPSolver(XPRSconstants.NLPSOLVER_LOCAL);

            // Solve
            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                    && prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();
            System.out.println("Objective: " + prob.attributes().getObjVal());
            // Print out the solution
            for (Variable var : r) {
                System.out.print(var.getName() + ":" + var.getValue(sol) + " ");
            }
            System.out.println();
            for (Variable var : theta) {
                System.out.print(var.getName() + ":" + var.getValue(sol) + " ");
            }
            System.out.println();
        }
    }
}

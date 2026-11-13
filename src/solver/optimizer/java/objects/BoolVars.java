// (c) 2023-2026 Fair Isaac Corporation

import java.util.Arrays;
import java.util.Locale;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/** Stating logic clauses that resemble SAT-type formulations */
public class BoolVars {
    public static void main(String[] args) {
        final int R = 5;

        try (XpressProblem prob = new XpressProblem()) {
            // create boolean variables and their negations
            Variable[] x = prob.addVariables(R).withType(ColumnType.Binary).withName("x_%d").withUB(1).toArray();
            Variable[] xNeg = prob.addVariables(R).withType(ColumnType.Binary).withName("xNeg_%d").withUB(1).toArray();

            // add 2 helper variables for stating logical constraints
            Variable trueVar = prob.addVariable(0, 1, ColumnType.Binary, "TRUE");
            Variable falseVar = prob.addVariable(0, 1, ColumnType.Binary, "FALSE");

            // fix these helper variables to TRUE and FALSE, respectively.
            trueVar.fix(1);
            falseVar.fix(0);

            // add the complement relation between each binary variable and its negation.
            prob.addConstraints(R, r -> x[r].plus(xNeg[r]).eq(1));

            // add a direct equation constraint between certain variables.
            prob.addConstraint(x[2].eq(xNeg[3]));

            // express some clauses on the variables
            // require that x(0) and xNeg(4) = true
            // we use trueVar as the resultant of a logical "and" constraint
            prob.addConstraint(trueVar.andOf(new Variable[] { x[0], xNeg[4] }));

            // ! 'x(0) or x(2)' must be true
            prob.addConstraint(trueVar.orOf(new Variable[] { x[0], x[2] }));

            // for more complicated expressions, we need auxiliary variables.
            Variable andResultant1 = prob.addVariable(0, 1, ColumnType.Binary, "andresultant1");
            Variable andResultant2 = prob.addVariable(0, 1, ColumnType.Binary, "andresultant2");

            // both constraints below could be formulated using andResultant[12].AndOf(...)
            // here, however, we highlight the connection to general constraints
            // the first AND is between x[0] .. x[2]
            prob.addConstraint(andResultant1.andOf(Arrays.copyOfRange(x, 0, 3)));
            // the second AND is between xNeg[3] and xNeg[4]
            prob.addConstraint(andResultant2.andOf(Arrays.copyOfRange(xNeg, 3, xNeg.length)));

            // add those two individual AND resultants by requiring at least one to be
            // satisfied
            prob.addConstraint(trueVar.orOf(new Variable[] { andResultant1, andResultant2 }));

            // finally, add a constraint that none of xNeg[0 .. 2] should be true
            prob.addConstraint(falseVar.orOf(Arrays.copyOfRange(xNeg, 0, 3)));

            // write the problem in LP format for manual inspection
            System.out.println("Writing the problem to 'BoolVars.lp'");
            prob.writeProb("BoolVars.lp", "l");

            // Solve the problem
            System.out.println("Solving the problem");
            prob.optimize();

            // check the solution status
            System.out.println("Problem finished with SolStatus " + prob.attributes().getSolStatus());
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL) {
                throw new RuntimeException("Problem not solved to optimality");
            }

            // print the solution of the problem to the console
            System.out.printf(Locale.US, "Solution has objective value (profit) of %g%n",
                    prob.attributes().getObjVal());
            System.out.println("");
            System.out.println("*** Solution ***");
            double[] sol = prob.getSolution();

            for (int r = 0; r < R; r++) {
                String delim = r < R - 1 ? ", " : "\n";
                System.out.printf(Locale.US, "x_%d = %g%s", r, x[r].getValue(sol), delim);
            }

            for (int r = 0; r < R; r++) {
                String delim = r < R - 1 ? ", " : "\n";
                System.out.printf(Locale.US, "xNeg_%d = %g%s", r, xNeg[r].getValue(sol), delim);
            }

            System.out.println("");
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.ConstantExpression.constant;
import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.RowType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/** Working with multiple problems. */
public class MultipleProblems {

    private static void printProblemStatus(XpressProblem prob) {
        System.out.println(String.format(
                "Problem status:%n\tSolve status: %s%n\tLP status: %s%n\tMIP status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getLPStatus(), prob.attributes().getMIPStatus(),
                prob.attributes().getSolStatus()));
    }

    private static void printProblemSolution(XpressProblem prob) {
        double[] sol = prob.getSolution();
        System.out.println("Objective: " + prob.attributes().getObjVal());
        System.out.printf("Variables:%n\t");
        for (Variable v : prob.getVariables()) {
            System.out.print(String.format("[%s: %.2f] ", v.getName(), v.getValue(sol)));
        }
        System.out.println();
    }

    private static void printConstraintSolution(XpressProblem prob) {
        System.out.print("Constrains:%n");
        for (Inequality c : prob.getInequalities()) {
            System.out
                    .println(String.format("\t[%s, Dual: %.2f, Slack: %.2f]", c.getName(), c.getDual(), c.getSlack()));
        }
    }

    /**** Expl 2: changing bounds and operations on constraints ****/
    public static XpressProblem expl2() {
        XpressProblem prob = new XpressProblem();
        double[] objcof = { 2.0, 1.0, 1.0, 1.0 };
        prob.callbacks.addMessageCallback(DefaultMessageListener::console);

        // Define 4 integer variables in 0,...,100
        Variable[] x = prob.addVariables(4).withType(ColumnType.Integer).withLB(0).withUB(100)
                .withName(i -> String.format("x_%d", i)).toArray();

        // Create the constraints:
        // ctr0: x0 +10 <= x1
        // ctr1: x1 <= x3
        // ctr2: x1 + 8 <= x2 */
        Inequality[] ctr = new Inequality[4];
        ctr[0] = prob.addConstraint(sum(x[0], constant(10)).leq(x[1])).setName("ctr0");
        ctr[1] = prob.addConstraint(x[1].leq(x[3])).setName("ctr1");
        ctr[2] = prob.addConstraint(sum(x[0], constant(8)).leq(x[2])).setName("ctr2");

        // Select objective function
        prob.setObjective(scalarProduct(x, objcof), XPRSenumerations.ObjSense.MINIMIZE);
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Change Constraint RHS:>>>>>>>>");
        ctr[0].setRhs(-5);
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);
        System.out.println("<<<<<<<<Restore original RHS value:>>>>>>>>");
        ctr[0].setRhs(10);
        System.out.println("<<<<<<<<Variable bound changed:>>>>>>>>");
        x[1].setLB(15);
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Constraint coefficient and RHS changed:>>>>>>>>");
        // Change constraint coefficient and RHS
        prob.chgCoef(ctr[1], x[1], -3); // ctr1: -3x1 <= x3
        ctr[0].setRhs(-20); // ctr0: x0 + 20 <= x1
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Constraint type changed:>>>>>>>>");
        // Change constraint type
        ctr[2].setType(RowType.GEQ); // ctr2: x1 + 8 >= x2
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Constraint added:>>>>>>>>");
        // Add another constraint ctr3: x0 + 37 <= x2
        ctr[3] = prob.addConstraint(sum(x[0], constant(37)).leq(x[2])).setName("ctr3");
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Constraint deleted:>>>>>>>>");
        // Delete a constraint
        Inequality[] todelete = { ctr[2] };
        prob.delInequalities(todelete);
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);

        return prob;
    }

    /**** Expl 3: Knapsack problem: accessing variables ****/
    public static XpressProblem expl3() {
        XpressProblem prob = new XpressProblem();
        double[] coeff = { 30.0, 32.0, 27.0, 11.0 };
        double[] objcof = { 9.0, 15.0, 8.0, 3.0 };
        prob.callbacks.addMessageCallback(DefaultMessageListener::console);

        // Define 4 binary variables
        Variable[] x = prob.addVariables(4).withType(ColumnType.Binary).withName(i -> String.format("x_%d", i))
            .toArray();

        // Create the knapsack constraint:
        // sum_i coeff[i]*x[i] <= 70
        prob.addConstraint(scalarProduct(x, coeff).leq(70));
        // Set objective function
        prob.setObjective(scalarProduct(x, objcof), XPRSenumerations.ObjSense.MAXIMIZE);
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);
        printConstraintSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Variable type changed from Binary to Integer>>>>>>>>");
        x[1].setType(ColumnType.Integer);
        System.out.println(
                           x[1].getName() + ": bounds: " + x[1].getLB() + " " + x[1].getUB() + ", type: " + x[1].getType());
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);
        printConstraintSolution(prob);

        System.out.println();
        System.out.println("<<<<<<<<Variable bound changed: no matrix regeneration>>>>>>>>");
        // Change variable bound
        x[1].setUB(3);
        System.out.println(
                           x[1].getName() + ": bounds: " + x[1].getLB() + " " + x[1].getUB() + ", type: " + x[1].getType());
        prob.optimize();
        printProblemStatus(prob);
        printProblemSolution(prob);
        printConstraintSolution(prob);

        return prob;
    }

    /**** Expl 4: a small unbounded problem ****/
    public static void expl4() {
        try (XpressProblem prob = new XpressProblem()) {
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            // Define 2 variables in [0,PLUSINFINITY]
            Variable[] x = prob.addVariables(2).withName(i -> String.format("x_%d", i)).withLB(0).toArray();

            // Create the constraints:
            // ctr0: 4*x0 + x1 >= 4
            // ctr1: x0 + x1 >= 3
            // ctr2: x0 + 2*x1 >= 4 */
            prob.addConstraint(sum(x[0].mul(-4), x[1]).geq(4).setName("ctr0"));
            prob.addConstraint(sum(x[0], x[1]).geq(3).setName("ctr1"));
            prob.addConstraint(sum(x[0], x[1].mul(2)).geq(4).setName("ctr2"));

            // Set objective function
            prob.setObjective(sum(x), XPRSenumerations.ObjSense.MAXIMIZE);

            prob.optimize();
            printProblemStatus(prob);
            printProblemSolution(prob);
        }
    }

    /*** Expl5: Working with different problems ****/
    public static void expl5(XpressProblem p2, XpressProblem p3) {
        try (XpressProblem prob = new XpressProblem()) {
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            System.out.println();
            System.out.println("<<<<<<<<Re-solve problem p2 >>>>>>>>");
            p2.optimize();
            printProblemStatus(p2);
            printProblemSolution(p2);

            System.out.println();
            System.out.println("<<<<<<<<Re-solve problem p3 >>>>>>>>");
            p3.optimize();
            printProblemStatus(p3);
            printProblemSolution(p3);
        }
    }

    public static void main(String[] args) {
        try (XpressProblem p2 = expl2();
             XpressProblem p3 = expl3()) {
            expl4();
            expl5(p2, p3);
        }
    }
}

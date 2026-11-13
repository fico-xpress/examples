// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static java.util.stream.Collectors.toMap;

import java.util.Arrays;
import java.util.Locale;
import java.util.Map;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Knapsack problem. This example shows how to setup a simple knapsack type
 * problem in which an item selection should be made maximally profitable under
 * some weight restriction. We first solve a pure knapsack problem. Then, we
 * refine the constraints by explicitly forbidding certain item combinations,
 * thereby showing multiple ways how to create indicator constraints.
 */
public class BinBurglar {

    private static final double[] values = new double[] { 15, 100, 90, 60, 40, 15, 10, 1 };

    private static final double[] weights = new double[] { 2, 20, 20, 30, 40, 30, 60, 10 };

    private static final String[] names = { "camera", "necklace", "vase", "picture", "tv", "video", "chest", "brick" };

    private static final double WTMAX = 102;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            /* Output all messages. */
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);
            /* VARIABLES */
            Variable[] x = prob.addVariables(values.length)
                    /* 1 if we take item i; 0 otherwise */
                    .withType(ColumnType.Binary).withName(i -> names[i]).toArray();

            /* CONSTRAINT */
            /* Weight restriction */
            prob.addConstraint(scalarProduct(x, weights).leq(WTMAX));

            /* OBJECTIVE */
            /* Set objective: maximize total value */
            prob.setObjective(scalarProduct(x, values), XPRSenumerations.ObjSense.MAXIMIZE);
            prob.writeProb("BinBurglar.mps");
            /* Solve */
            prob.optimize();

            /*
             * Check the status, which should be optimal and the solve should be completed
             */
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());

            /* Print out the solution */
            double[] sol = prob.getSolution();
            System.out.println("Objective: " + prob.attributes().getObjVal());
            for (Variable var : x) {
                System.out.print(var.getName() + ":" + sol[var.getIndex()] + " ");
            }
            System.out.println();

            /* Create a map to look up the variables by name */
            Map<String, Variable> varByName = Arrays.stream(x).collect(toMap(v -> v.getName(), v -> v));

            /*
             * Now add constraints to consider the items vase and picture as one pair and
             * the items "tv" and "video" as a second pair so that the pairs are always
             * taken together
             */
            Variable vase = varByName.get("vase");

            /* x["vase"] == x["picture"] */
            prob.addConstraint(vase.eq(varByName.get("picture")));

            /* x["tv"] == x["video"] */
            prob.addConstraint(varByName.get("tv").eq(varByName.get("video")));

            /*
             * Logical constraint: Either take the first pair "vase" and "picture" or the
             * second pair "tv" and "video" (but not both pairs).
             */
            Expression tvPlusVideo = varByName.get("tv").plus(varByName.get("video"));

            /* x["vase"]=1 -> x["tv"]+x["video"]=0 */
            prob.addConstraint(vase.ifThen(tvPlusVideo.leq(0.0)));

            /*
             * x["vase"]=0 -> x["tv"]+x["video"]=2, this uses an alternative way to create
             * the indicator
             */
            Inequality enablePairTwo = prob.addConstraint(tvPlusVideo.geq(2.0));
            prob.setIndicator(vase, false, enablePairTwo);

            /*
             * By writing out this small problem we can easily verify that everything looks
             * as expected
             *
             * prob.writeProb("BinBurglarL.lp", "l");
             */

            /* Solve again */
            prob.optimize();

            /* Get results of solve */
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());

            /* Print out the solution */
            sol = prob.getSolution();
            System.out.printf(Locale.US, "Objective: %f%n", prob.attributes().getObjVal());
            for (Variable var : x) {
                System.out.printf(Locale.US, "%s: %g ", var.getName(), sol[var.getIndex()]);
            }
            System.out.println();
        }
    }
}

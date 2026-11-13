// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Inequality;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Holy Grail. When the Sangraal (Holy Grail) is almost won the hero arrives at
 * a castle where he finds 8 imprisoned knights. He is facing the task to bring
 * the largest possible number of knights for the arrival of the Sangraal in
 * twenty minutes' time. The time required for freeing a knight depends on his
 * state of binding. A freed knight then needs a given amount of time to wash
 * and recover himself physically.
 *
 * Model formulation using indicator constraints.
 *
 * Description and original model by M. Chlond:
 * http://ite.pubs.informs.org/Vol4No3/Chlond
 */
public class Sangraalind {

    /** A knight. */
    public static final class Knight {
        /** The name of the knight. */
        public final String name;
        /** How long it takes to free the knight. */
        public final double freeTime;
        /** How long it takes the knight to wash and prepare himself. */
        public final double washTime;

        public Knight(String name, double freeTime, double washTime) {
            this.name = name;
            this.freeTime = freeTime;
            this.washTime = washTime;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    /** The knights used in this example. */
    private static final Knight[] knightArray = new Knight[] { new Knight("Agravain", 1, 15), new Knight("Bors", 1, 5),
            new Knight("Caradoc", 2, 15), new Knight("Dagonet", 2, 5), new Knight("Ector", 3, 10),
            new Knight("Feirefiz", 4, 15), new Knight("Gareth", 5, 10), new Knight("Harry", 6, 5) };

    /** The resource constraints used in this example. */
    static final double maxTime = 20.0;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/

            // Whether to free knight i as the j-th knight
            Variable[][] order = prob.addVariables(knightArray.length, knightArray.length)
                    .withName((i, j) -> String.format("order_%d_%d", i, j)).withType(ColumnType.Binary).toArray();

            // Whether the knight who was freed as the i-th knight will be ready in time
            Variable[] onTime = prob.addVariables(knightArray.length).withName(i -> String.format("onTime_%d", i))
                    .withType(ColumnType.Binary).toArray();

            // At what time the i-th freed knight will be ready
            Variable[] readyTime = prob.addVariables(knightArray.length).withName(i -> String.format("readyTime_%d", i))
                    .toArray();

            // Maximize the number of knights that are ready after 20 minutes
            Expression totalReady = sum(onTime);

            prob.setObjective(totalReady, XPRSenumerations.ObjSense.MAXIMIZE);

            // Exactly one knight should be freed as the i-th knight
            prob.addConstraints(knightArray.length, i -> sum(knightArray.length, j -> (order[i][j])).eq(1));

            // Each knight should be freed exactly once
            prob.addConstraints(knightArray.length, i -> sum(knightArray.length, j -> (order[j][i])).eq(1));

            // The time each knight is ready, computed as the sum of times it took to free
            // all
            // previous knights, plus the time it took to free this knight plus the time it
            // takes
            // the knight to wash and get ready

            // loop over all positions
            for (int p = 0; p < knightArray.length; p++) {
                LinExpression timeBeforeFree = LinExpression.create();
                // loop over all knights
                for (int k = 0; k < knightArray.length; k++) {
                    // loop over all positions before the current one
                    for (int q = 0; q < p; q++) {
                        // If this knight was freed before the current position, add the time it took to
                        // free them
                        timeBeforeFree.addTerm(order[k][q].mul(knightArray[k].freeTime));
                    }
                    // if knight k was freed in this position, add the time it took to free them and
                    // for them to prepare
                    timeBeforeFree.addTerm(order[k][p].mul((knightArray[k].freeTime + knightArray[k].washTime)));
                }
                // add the actual constraint
                prob.addConstraint(timeBeforeFree.eq(readyTime[p]));
            }

            // The i-th freed knight will be ready iff they were ready by time 20
            for (int p = 0; p < knightArray.length; p++) {
                Inequality isReady = prob.addConstraint(readyTime[p].leq(maxTime));
                prob.setIndicator(onTime[p], true, isReady);
            }

            // Dump the problem to disk so that we can inspect it.
            prob.writeProb("sangraalind.lp", "l");

            // Solve
            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                    && prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();

            // Print the solution
            System.out.println((int) prob.attributes().getObjVal() + " knights ready in time:");
            for (int p = 0; p < knightArray.length; p++) {
                for (int k = 0; k < knightArray.length; k++) {
                    if (order[k][p].getValue(sol) > 0.5) {
                        String pos;
                        if (p == 0) {
                            pos = "1st";
                        } else if (p == 1) {
                            pos = "2nd";
                        } else if (p == 2) {
                            pos = "3rd";
                        } else {
                            pos = p + 1 + "th";
                        }
                        System.out.println(pos + " freed knight: " + knightArray[k].toString() + " ready at time "
                                + readyTime[p].getValue(sol));
                    }
                }
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.div;
import static com.dashoptimization.objects.Utils.mul;
import static com.dashoptimization.objects.Utils.plus;
import static com.dashoptimization.objects.Utils.pow;
import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * A craftsman makes small wooden boxes for sale. He has four different shapes
 * or styles of box, and can make each of them in any size (keeping all the
 * dimensions in proportion). The profit he makes on a box depends on the size.
 * He has only a limited amount of the necessary wood available and a limited
 * amount of time in the week to do the work. How many boxes should he make, and
 * what size should they be, in order to maximize his profit?
 */
public class Boxes02 {

    /** A box. */
    public static final class Box {
        /** The name of this box. */
        public final String name;
        /** Relative length of this box. */
        public final double lengthCoeff;
        /** Relative width of this box. */
        public final double widthCoeff;
        /** Relative height of this box. */
        public final double heightCoeff;
        /** Coefficient for the profit of this box, relative to its size. */
        public final double profitCoeff;
        /** Coefficient for the production time of this box, relative to its ply. */
        public final double timeCoeff;

        public Box(String name, double lengthCoeff, double widthCoeff, double heightCoeff, double profitCoeff,
                double timeCoeff) {
            this.name = name;
            this.lengthCoeff = lengthCoeff;
            this.widthCoeff = widthCoeff;
            this.heightCoeff = heightCoeff;
            this.profitCoeff = profitCoeff;
            this.timeCoeff = timeCoeff;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    /** The boxes used in this example. */
    private static final Box[] boxArray = new Box[] { new Box("Cube", 1, 1, 1, 20, 1),
            new Box("Oblong", 1, 2, 1, 27.3, 1), new Box("Flat", 4, 4, 1, 90, 1),
            new Box("Economy", 1, 2, 1, 10, 0.2) };

    /** The resource constraints used in this example. */
    static final double maxSize = 2.0;
    static final int maxNumProduced = 6;
    static final double maxBattens = 200.0;
    static final double maxPly = 210.0;
    static final double maxMakeTime = 35.0;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/

            // the number of each of the boxes that should be produced
            Variable[] numProduced = prob.addVariables(boxArray.length)
                    .withName(i -> String.format("numProduced_%d", i)).withType(ColumnType.Integer)
                    .withUB(maxNumProduced).toArray();

            // the relative size (a factor of length/width/height) of each of the boxes that
            // should be produced
            Variable[] size = prob.addVariables(boxArray.length).withName(i -> String.format("size_%d", i))
                    .withUB(maxSize).toArray();

            // objective transfer column
            Variable objtransfercol = prob.addVariable(XPRSconstants.MINUSINFINITY, XPRSconstants.PLUSINFINITY,
                    ColumnType.Continuous, "objTransferCol");

            // some general characteristics of the produced boxes that will be used in the
            // model
            final Expression[] battens = new Expression[boxArray.length];
            final Expression[] ply = new Expression[boxArray.length];
            final Expression[] profit = new Expression[boxArray.length];
            final Expression[] makeTime = new Expression[boxArray.length];

            for (int i = 0; i < boxArray.length; ++i) {
                // battens = 4 * (lengthCoeff + widhtCoeff + heightCoeff) * size
                battens[i] = size[i]
                        .mul(4.0 * (boxArray[i].lengthCoeff + boxArray[i].widthCoeff + boxArray[i].heightCoeff));
                // ply = 2 * (lengthCoeff * widthCoeff + widthC * heightC + heightC * lengthC) *
                // size^2
                ply[i] = mul(pow(size[i], 2.0),
                        2.0 * (boxArray[i].lengthCoeff * boxArray[i].widthCoeff
                                + boxArray[i].widthCoeff * boxArray[i].heightCoeff
                                + boxArray[i].heightCoeff * boxArray[i].lengthCoeff));
                // profit = profitCoeff * size^1.5
                profit[i] = mul(boxArray[i].profitCoeff, pow(size[i], 1.5));
                // makeTime = 1 + timeCoeff * 1.5^(ply/10)
                makeTime[i] = plus(1.0, mul(boxArray[i].timeCoeff, pow(1.5, div(ply[i], 10.0))));
            }

            // objective function: sum(boxes) numProduced[box] * profit[box]
            Expression totalProfit = sum(boxArray.length, i -> mul(numProduced[i], profit[i]));

            // To make the objective linear, just maximize the objtransfercol
            prob.setObjective(objtransfercol, XPRSenumerations.ObjSense.MAXIMIZE);

            // Add the objective transfer row: objtransfercol = totalProfit
            prob.addConstraint(objtransfercol.eq(totalProfit));

            // limits on resource availability
            // sum of battens of all produced boxes <= maxBattens
            prob.addConstraint(sum(boxArray.length, i -> mul(numProduced[i], battens[i])).leq(maxBattens));
            // sum of ply of all produced boxes <= maxPly
            prob.addConstraint(sum(boxArray.length, i -> mul(numProduced[i], ply[i])).leq(maxPly));
            // sum of make time of all produced boxes <= maxMakeTime
            prob.addConstraint(sum(boxArray.length, i -> mul(numProduced[i], makeTime[i])).leq(maxMakeTime));

            // Dump the problem to disk so that we can inspect it.
            prob.writeProb("boxes02.lp", "l");

            // By default we will solve to global optimality, uncomment for an MISLP solve
            // to local optimality
            // prob.controls().setNLPSolver(XPRSconstants.NLPSOLVER_LOCAL);

            // Solve
            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                    && prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();
            System.out.println("Objective: " + prob.attributes().getObjVal());
            // Print out the solution
            for (int i = 0; i < boxArray.length; ++i) {
                if (numProduced[i].getValue(sol) > 0.0) {
                    System.out.println("Producing " + numProduced[i].getValue(sol) + " " + boxArray[i].toString()
                            + " boxes of size " + size[i].getValue(sol) + ".");
                }
            }
        }
    }
}

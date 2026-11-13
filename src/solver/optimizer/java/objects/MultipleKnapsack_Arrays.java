// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.ConstantExpression.constant;
import static com.dashoptimization.objects.SOS.sos1;
import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import com.dashoptimization.ColumnType;
import com.dashoptimization.PwlBreakpoint;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * A very simple multiple knapsack problem. The problem asks for assigning a set
 * of items to multiple knapsacks. The objective is to maximize profit while
 * respecting the per-knapsack weight restrictions. Each knapsack has a capacity
 * that limits the weight of items that can be put into it. The capacity can be
 * extended by a constant at a fixed cost.
 *
 * In this example, data for the knapsacks and items is given by arrays and
 * variables are arranged in arrays as well. They are indexed by knapsack and
 * item id.
 */
public final class MultipleKnapsack_Arrays {
    /** An item. */
    public static final class Item {
        /** The weight of this item. */
        public final double weight;
        /** The profit (objective value) of this item. */
        public final double profit;
        /** The name of this item. */
        public final String name;

        public Item(double weight, double profit, String name) {
            this.weight = weight;
            this.profit = profit;
            this.name = name;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    /** A knapsack. */
    public static final class Knapsack {
        /** Capacity of this item. */
        public final double capacity;
        /**
         * Extra capacity that can be bought for this item at cost
         * <code>extraCost</code>.
         */
        public final double extraCapacity;
        /**
         * The cost for increasing this knapsack's capacity by
         * <code>extraCapacity<code>.
         */
        public final double extraCost;
        /** The name of this knapsack. */
        public final String name;

        public Knapsack(double capacity, double extraCapacity, double extraCost, String name) {
            this.capacity = capacity;
            this.extraCapacity = extraCapacity;
            this.extraCost = extraCost;
            this.name = name;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    /** The items to assign in this multiple knapsack instance. */
    private static final Item[] items = new Item[] { new Item(2, 2, "item1"), new Item(3, 3, "item2"),
            new Item(4, 4, "item3"), new Item(5, 5, "item4"), new Item(6, 6, "item5"), new Item(7, 7, "item6"),
            new Item(8, 8, "item7"), new Item(9, 9, "item8") };

    /** The knapsacks in this multiple knapsack instance. */
    private static final Knapsack[] knapsacks = new Knapsack[] { new Knapsack(15, 5, 2, "knapsack1"),
            new Knapsack(16, 4, 3, "knapsack2") };

    /**
     * In this multiple knapsack instance we allow for buying some extra capacity in
     * each knapsack. There are different ways to model this extra capacity. This
     * enumeration indicates how this should be modeled.
     */
    public static enum ExtendedCapacityModeling {
        /** Do not allow any extra capacity. */
        NoExtraCapacity,
        /** Use binary variables to model the extra capacity. */
        BinaryVariables,
        /** Use indicator constraints to model the extra capacity. */
        IndicatorConstraints,
        /** Use a piecewise linear function to model extra capacity. */
        PiecewiseLinear,
        /** Use SOS constraints to model extra capacity. */
        SOS
    }

    public static ExtendedCapacityModeling extendedCapacityModeling = ExtendedCapacityModeling.NoExtraCapacity;

    /**
     * Model and solve the multiple knapsack problem using the data that is provided
     * as arrays.
     */
    public static void main(String[] args) {
        // If an argument was given on the command line then we assume that
        // this argument selects the way in which we are supposed to model
        // the extra capacity constraint.
        if (args.length > 0)
            extendedCapacityModeling = ExtendedCapacityModeling.valueOf(args[0]);
        try (XpressProblem prob = new XpressProblem()) {
            // Create the variables.
            // This creates a two-dimensional array of variables.
            // The first index is the item index, the second index is the
            // the knapsack index.
            // All variables are binary and the name for variable x[i][j]
            // is "i@j".
            Variable[][] x = prob.addVariables(items.length, knapsacks.length).withType(ColumnType.Binary)
                    .withName((i, k) -> String.format("%s@%s", i, k)).toArray();

            // Create the expression for the objective function.
            // sum(i in items) sum(k in knapsacks) i.profit * x[i][k]
            Expression profit = sum(items.length, knapsacks.length, (i, k) -> x[i][k].mul(items[i].profit));

            // Right-hand side for capacity constraint.
            final Expression[] capacityRhs = new Expression[knapsacks.length];
            switch (extendedCapacityModeling) {
            case NoExtraCapacity:
                // No extra capacity at all.
                for (int k = 0; k < knapsacks.length; ++k)
                    capacityRhs[k] = constant(knapsacks[k].capacity);
                break;
            case BinaryVariables:
            // Model extra capacity using an additional binary variable y[k]
            // for each knapsack k.
            // These variables are added to the objective with the extra
            // cost for the knapsack.
            // They are also added to capacityRhs[k] with the knapsack's
            // extra capacity as factor.
            // This way we get extra capacity and the cost for it if and
            // only if the binary variable is set to 1.
            {
                Variable[] y = prob.addVariables(knapsacks.length).withType(ColumnType.Binary).withName("y_%s")
                        .toArray();
                profit = sum(profit, sum(knapsacks.length, k -> y[k].mul(-knapsacks[k].extraCost)));

                for (int k = 0; k < knapsacks.length; ++k)
                    capacityRhs[k] = y[k].mul(knapsacks[k].extraCapacity).plus(knapsacks[k].capacity);
            }
                break;
            case IndicatorConstraints:
            // We model the additional capacity as follows:
            // - unconditionally add the extra capacity to each knapsack
            // - add an additional row that limits the capacity to the
            // original capacity
            // - activate this new row only if a newly introduced indicator
            // variable y is zero
            // - add the indicator variable to the objective function.
            {
                Variable[] y = prob.addVariables(knapsacks.length).withType(ColumnType.Binary).withName("y_%s")
                        .toArray();
                profit = sum(profit, sum(knapsacks.length, k -> y[k].mul(-knapsacks[k].extraCost)));
                for (int k = 0; k < knapsacks.length; ++k) {
                    int kIdx = k; // Effectively final for lambda
                    Knapsack knapsack = knapsacks[kIdx];
                    capacityRhs[k] = constant(knapsack.capacity + knapsack.extraCapacity);
                    prob.addConstraint(y[kIdx]
                            .ifNotThen(sum(items.length, i -> x[i][kIdx].mul(items[i].weight)).leq(knapsack.capacity)));
                }
            }
                break;
            case PiecewiseLinear:
            // Model the extra capacity using a piecewise linear function:
            // - Unconditionally add the extra capacity to knapsack k
            // - Create a new variable z[k] that is set to the weight
            // in knapsack k
            // - Create a variable y[k] that is added to the objective with
            // with the cost for extra capacity
            // - Create a piecewise linear
            // y = f(z)
            // where f is 0 on [0,capacity] and
            // 1 on [capacity+1,capacity+extraCapacity]
            {
                for (int k = 0; k < knapsacks.length; ++k)
                    capacityRhs[k] = constant(knapsacks[k].capacity + knapsacks[k].extraCapacity);
                Variable[] y = prob.addVariables(knapsacks.length).withName("y_%s").toArray();
                Variable[] z = prob.addVariables(knapsacks.length).withName("z_%s").toArray();
                profit = sum(profit, sum(knapsacks.length, k -> y[k].mul(-knapsacks[k].extraCost)));
                prob.addConstraints(knapsacks.length,
                        k -> sum(items.length, i -> x[i][k].mul(items[i].weight)).eq(z[k]));
                prob.addConstraints(knapsacks.length,
                        k -> y[k].pwlOf(z[k], new PwlBreakpoint(0, 0), new PwlBreakpoint(knapsacks[k].capacity, 0),
                                new PwlBreakpoint(knapsacks[k].capacity + 1, 1),
                                new PwlBreakpoint(knapsacks[k].capacity + knapsacks[k].extraCapacity, 1)));

            }
                break;
            case SOS:
            // Model extra capacity using an SOS1 constraint:
            // - Create two binary variables y[k][0] and y[k][1] for
            // each knapsack k.
            // - Set the right-hand side of the capacity constraint to
            // k.capacity * y[k][0] +
            // (k.capacity + k.extraCapacity) * y[k][1]
            // - Charge k.extraCost on y[k][1]
            // - Add an SOS1 constraint (y[k][0], y[k][1])
            {
                Variable[][] y = range(0, knapsacks.length).mapToObj(k -> prob.addVariables(2)
                        .withType(ColumnType.Binary).withName(i -> String.format("y(%d)(%d)", k, i)).toArray())
                        .toArray(Variable[][]::new);
                profit = sum(profit, sum(knapsacks.length, k -> y[k][1].mul(-knapsacks[k].extraCost)));
                for (int k = 0; k < knapsacks.length; ++k) {
                    capacityRhs[k] = sum(y[k][0].mul(knapsacks[k].capacity),
                            y[k][1].mul(knapsacks[k].capacity + knapsacks[k].extraCapacity));
                    prob.addConstraint(sos1(y[k], null, String.format("sos%d", k)));
                }
            }
                break;
            }
            // Set the objective function.
            // We want to maximize the profit, so we set the sense to
            // MAXIMIZE.
            prob.setObjective(profit, XPRSenumerations.ObjSense.MAXIMIZE);

            // Create the capacity restriction.
            // There is one restriction for each knapsack.
            // The left-hand side is always the sum of the x variables for
            // this knapsack multiplied by the item weights.
            // The right hand side depends on the way how extra capacity
            // is modeled and contains the initial capacity plus any terms
            // that allow for extra capacity.
            prob.addConstraints(knapsacks.length,
                    k -> sum(items.length, i -> x[i][k].mul(items[i].weight)).leq(capacityRhs[k]));

            // Add a constraint that each item should be put in at most one knapsack
            prob.addConstraints(items.length, i -> sum(knapsacks.length, k -> x[i][k]).leq(1));

            // Dump the problem to disk so that we can inspect it.
            prob.writeProb("Array.lp");

            // Finally solve, check the solution status and report.
            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("no optimal solution found");
            System.out.printf("Maximum profit: %f%n", prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            for (int i = 0; i < items.length; ++i) {
                int target = -1;
                for (int k = 0; k < knapsacks.length; ++k) {
                    if (x[i][k].getValue(sol) > 0.5) {
                        target = k;
                        break;
                    }
                }
                System.out.printf("  item %s %s%n", items[i].name,
                        target < 0 ? "not picked" : String.format("into %s", knapsacks[target].name));
            }
        }
    }
}

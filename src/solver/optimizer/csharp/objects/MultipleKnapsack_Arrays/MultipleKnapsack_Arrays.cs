// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using Optimizer.Objects;
using PwlBreakpoint = Optimizer.PwlBreakpoint;
using ColumnType = Optimizer.ColumnType;
using static Optimizer.Objects.Utils;
using static Optimizer.XPRSprob;
using static Optimizer.Objects.ConstantExpression;
using static Optimizer.Objects.SOS;

namespace XpressExamples
{
    /// <summary>A very simple multiple knapsack problem.</summary>
    /// <remarks>
    /// The problem asks for assigning a set of items to multiple knapsacks.
    /// The objective is to maximize profit while respecting the per-knapsack weight
    /// restrictions. Each knapsack has a capacity that limits the weight of items
    /// that can be put into it. The capacity can be extended by a constant at a
    /// fixed cost.
    ///
    /// In this example, data for the knapsacks and items is given by arrays and variables
    /// are arranged in arrays as well.They are indexed by knapsack and item id.
    public class MultipleKnapsack_Arrays
    {
        /// <summary>An item.</summary>
        public sealed class Item
        {
            /// <summary>The weight of this item.</summary>
            public readonly double weight;
            /// <summary>The profit (objective value) of this item.</summary>
            public readonly double profit;
            /// <summary>The name of this item.</summary>
            public readonly string name;
            public Item(double weight, double profit, string name)
            {
                this.weight = weight;
                this.profit = profit;
                this.name = name;
            }
            public override string ToString() { return name; }
        }
        /// <summary>A knapsack.</summary>
        public sealed class Knapsack
        {
            /// <summary>Capacity of this item.</summary>
            public readonly double capacity;
            /// <summary>Extra capacity that can be bought for this item at cost <c>extraCost</c>.</summary>
            public readonly double extraCapacity;
            /// <summary>The cost for increasing this knapsack's capacity by <c>extraCapacity</c>.</summary>
            public readonly double extraCost;
            /// <summary>The name of this knapsack.</summary>
            public readonly string name;
            public Knapsack(double capacity, double extraCapacity, double extraCost, string name)
            {
                this.capacity = capacity;
                this.extraCapacity = extraCapacity;
                this.extraCost = extraCost;
                this.name = name;
            }
            public override string ToString() { return name; }
        }

        /// <summary>The items to assign in this multiple knapsack instance.</summary>
        private static readonly Item[] items = new Item[]{
                new Item(2, 2, "item1"), new Item(3, 3, "item2"),
                new Item(4, 4, "item3"), new Item(5, 5, "item4"),
                new Item(6, 6, "item5"), new Item(7, 7, "item6"),
                new Item(8, 8, "item7"), new Item(9, 9, "item8")
            };

        /// <summary>The knapsacks in this multiple knapsack instance.</summary>
        private static readonly Knapsack[] knapsacks = new Knapsack[]{
                new Knapsack(15, 5, 2, "knapsack1"),
                new Knapsack(16, 4, 3, "knapsack2")
            };

        /// <summary>
        /// In this multiple knapsack instance we allow for buying some extra
        /// capacity in each knapsack. There are different ways to model this
        /// extra capacity. This enumeration indicates how this should be modeled.
        /// </summary>
        public enum ExtendedCapacityModeling
        {
            /// <summary>Do not allow any extra capacity.</summary>
            NoExtraCapacity,
            /// <summary>Use binary variables to model the extra capacity.</summary>
            BinaryVariables,
            /// <summary>Use indicator constraints to model the extra capacity.</summary>
            IndicatorConstraints,
            /// <summary>Use a piecewise linear function to model extra capacity.</summary>
            PiecewiseLinear,
            /// <summary>Use SOS constraints to model extra capacity.</summary>
            SOS
        }
        public static ExtendedCapacityModeling extendedCapacityModeling = ExtendedCapacityModeling.NoExtraCapacity;

        /// <summary>Model and solve the multiple knapsack problem using the data that is provided as arrays.</summary>
        public static void Main(string[] args)
        {
            // If an argument was given on the command line then we assume that
            // this argument selects the way in which we are supposed to model
            // the extra capacity constraint.
            if (args.Length > 0)
                extendedCapacityModeling = (ExtendedCapacityModeling)Enum.Parse(typeof(ExtendedCapacityModeling), args[0], true);
            using (XpressProblem prob = new XpressProblem())
            {
                // Create the variables.
                // This creates a two-dimensional array of variables.
                // The first index is the item index, the second index is the
                // the knapsack index.
                // All variables are binary and the name for variable x[i][j]
                // is "put_i_into_j".
                Variable[,] x = prob.AddVariables(items.Length,
                                                   knapsacks.Length)
                    .WithType(ColumnType.Binary)
                    .WithName((i, k) => $"{i}@{k}")
                    .ToArray();

                // Create the expression for the objective function.
                //   sum(i in items) sum(k in knapsacks) i.profit * x[i][k]
                Expression profit = Sum(items.Length, knapsacks.Length,
                                        (i, k) => x[i, k] * items[i].profit);


                // Right-hand side for capacity constraint.
                Expression[] capacityRhs = new Expression[knapsacks.Length];
                switch (extendedCapacityModeling)
                {
                    case ExtendedCapacityModeling.NoExtraCapacity:
                        // No extra capacity at all.
                        for (int k = 0; k < knapsacks.Length; ++k)
                            capacityRhs[k] = Constant(knapsacks[k].capacity);
                        break;
                    case ExtendedCapacityModeling.BinaryVariables:
                        // Model extra capacity using an additional binary variable y[k]
                        // for each knapsack k.
                        // These variables are added to the objective with the extra
                        // cost for the knapsack.
                        // They are also added to capacityRhs[k] with the knapsack's
                        // extra capacity as factor.
                        // This way we get extra capacity and the cost for it if and
                        // only if the binary variable is set to 1.
                        {
                            Variable[] y = prob.AddVariables(knapsacks.Length)
                                .WithType(ColumnType.Binary)
                                .WithName(k => $"y_{k}")
                                .ToArray();
                            profit = Sum(profit,
                                         Sum(knapsacks.Length,
                                             k => y[k] * -knapsacks[k].extraCost));

                            for (int k = 0; k < knapsacks.Length; ++k)
                                capacityRhs[k] = knapsacks[k].extraCapacity * y[k] + knapsacks[k].capacity;
                        }
                        break;
                    case ExtendedCapacityModeling.IndicatorConstraints:
                        // We model the additional capacity as follows:
                        // - unconditionally add the extra capacity to each knapsack
                        // - add an additional row that limits the capacity to the
                        //   original capacity
                        // - activate this new row only if a newly introduced indicator
                        //   variable y is zero
                        // - add the indicator variable to the objective function.
                        {
                            Variable[] y = prob.AddVariables(knapsacks.Length)
                                .WithType(ColumnType.Binary)
                                .WithName(k => $"y_{k}")
                                .ToArray();
                            profit = Sum(profit,
                                         Sum(knapsacks.Length,
                                             k => y[k] * -knapsacks[k].extraCost));
                            for (int k = 0; k < knapsacks.Length; ++k)
                            {
                                capacityRhs[k] = Constant(knapsacks[k].capacity +
                                                          knapsacks[k].extraCapacity);
                                prob.AddConstraint(y[k].IfNotThen(Sum(items.Length,
                                                                                i => x[i, k] * items[i].weight)
                                                                  <= knapsacks[k].capacity));
                            }
                        }
                        break;
                    case ExtendedCapacityModeling.PiecewiseLinear:
                        // Model the extra capacity using a piecewise linear function:
                        // - Unconditionally add the extra capacity to knapsack k
                        // - Create a new variable z[k] that is set to the weight
                        //   in knapsack k
                        // - Create a variable y[k] that is added to the objective with
                        //   with the cost for extra capacity
                        // - Create a piecewise linear
                        //         y = f(z)
                        //   where f is 0 on [0,capacity] and
                        //              1 on [capacity+1,capacity+extraCapacity]
                        {
                            for (int k = 0; k < knapsacks.Length; ++k)
                                capacityRhs[k] = Constant(knapsacks[k].capacity +
                                                          knapsacks[k].extraCapacity);

                            Variable[] y = prob.AddVariables(knapsacks.Length)
                                .WithName(k => $"y_{k}")
                                .ToArray();

                            Variable[] z = prob.AddVariables(knapsacks.Length)
                                .WithName(k => $"z_{k}")
                                .ToArray();

                            profit = Sum(profit,
                                         Sum(knapsacks.Length,
                                                       k => y[k] * -knapsacks[k].extraCost));

                            prob.AddConstraints(knapsacks.Length,
                                                k => Sum(items.Length,
                                                         i => x[i, k] * items[i].weight) == z[k]);

                            prob.AddConstraints(knapsacks.Length,
                                                k => y[k].PwlOf(z[k],
                                                                new PwlBreakpoint(0, 0),
                                                                new PwlBreakpoint(knapsacks[k].capacity, 0),
                                                                new PwlBreakpoint(knapsacks[k].capacity + 1, 1),
                                                                new PwlBreakpoint(knapsacks[k].capacity + knapsacks[k].extraCapacity, 1)));

                        }
                        break;
                    case ExtendedCapacityModeling.SOS:
                        // Model extra capacity using an SOS1 constraint:
                        // - Create two binary variables y[k][0] and y[k][1] for
                        //   each knapsack k.
                        // - Set the right-hand side of the capacity constraint to
                        //     k.capacity * y[k][0] +
                        //     (k.capacity + k.extraCapacity) * y[k][1]
                        // - Charge k.extraCost on y[k][1]
                        // - Add an SOS1 constraint (y[k][0], y[k][1])
                        {
                            Variable[][] y = Enumerable.Range(0, knapsacks.Length)
                                .Select(k => prob.AddVariables(2)
                                          .WithType(ColumnType.Binary)
                                          .WithName(i => $"y_{k}_sos{i}")
                                          .ToArray())
                                .ToArray();
                            profit = Sum(profit,
                                         Sum(knapsacks.Length,
                                                       k => y[k][1] * -knapsacks[k].extraCost));
                            for (int k = 0; k < knapsacks.Length; ++k)
                            {
                                capacityRhs[k] = knapsacks[k].capacity * y[k][0] +
                                                 (knapsacks[k].capacity + knapsacks[k].extraCapacity) * y[k][1];
                                prob.AddConstraint(Sos1(y[k], null, $"sos_knapsack{k + 1}"));
                            }
                        }
                        break;
                }
                // Set the objective function.
                // We want to maximize the profit, so we set the sense to
                // MAXIMIZE.
                prob.SetObjective(profit, Optimizer.ObjSense.Maximize);

                // Create the capacity restriction.
                // There is one restriction for each knapsack.
                // The left-hand side is always the sum of the x variables for
                // this knapsack multiplied by the item weights.
                // The right hand side depends on the way how extra capacity
                // is modeled and contains the initial capacity plus any terms
                // that allow for extra capacity.
                prob.AddConstraints(knapsacks.Length,
                                    k => Sum(items.Length,
                                             i => x[i, k] * items[i].weight)
                                    <= capacityRhs[k]);

                // Add a constraint that each item should be put in at most one knapsack
                prob.AddConstraints(items.Length,
                                    i => Sum(knapsacks.Length, k => x[i, k])
                                    <= 1);

                // Dump the problem to disk so that we can inspect it.
                prob.WriteProb("Array.lp");

                // Finally solve, check the solution status and report.
                prob.Optimize();
                if (prob.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("no optimal solution found");
                Console.WriteLine("Maximum profit: {0:f}", prob.ObjVal);
                double[] sol = prob.GetSolution();
                for (int i = 0; i < items.Length; ++i)
                {
                    int target = -1;
                    for (int k = 0; k < knapsacks.Length; ++k)
                    {
                        if (x[i, k].GetValue(sol) > 0.5)
                        {
                            target = k;
                            break;
                        }
                    }
                    Console.WriteLine("  item {0} {1}", items[i].name,
                                      target < 0 ? "not picked" : $"into {knapsacks[target].name}");
                }
            }
        }
    }
}

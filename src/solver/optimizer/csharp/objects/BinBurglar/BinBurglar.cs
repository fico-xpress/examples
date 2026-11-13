// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Maps;
using System.Linq;
using ColumnType = Optimizer.ColumnType;
using static Optimizer.Objects.Utils;
using System.Collections.Generic;
using Optimizer.Objects;

namespace XpressExamples
{
    /// <summary>Burglar problem, binary variable formulation.</summary>
    /// <remarks>
    /// Select items to maximize profit while respecting a maximum weight constraint.
    ///
    /// This example shows how to setup a simple knapsack type problem in which an item
    /// selection should be made maximally profitable under some weight restriction.
    /// We first solve a pure knapsack problem. Then, we refine the constraints by
    /// explicitly forbidding certain item combinations, thereby showing multiple ways
    /// how to create indicator constraints.
    /// </remarks>
    class BinBurglar
    {
        /// <summary>Item values.</summary>
        /// <remarks>The goal is to maximize the sum of selected items.</remarks>
        private static readonly double[] values = new double[]{
            15, 100, 90, 60, 40, 15, 10, 1
        };

        /// <summary>Item weights.</summary>
        /// <remarks>The weights of selected items must not exceed <c>WTMAX</c>.</remarks>
        private static readonly double[] weights = new double[]{
            2, 20, 20, 30, 40, 30, 60, 10
        };

        private static readonly string[] itemNames = new string[] {
            "camera", "necklace", "vase", "picture", "tv", "video", "chest", "brick"
        };

        /// <summary>Maximum weight for selected items.</summary>
        private static readonly double WTMAX = 102;

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                Variable[] x = prob.AddVariables(values.Length)
                    // 1 if we take item i; 0 otherwise
                    .WithType(ColumnType.Binary)
                    .WithName(i => itemNames[i])
                    .ToArray();

                // Weight restriction
                prob.AddConstraint(ScalarProduct(x, weights) <= WTMAX);

                // Set objective: maximize total value
                prob.SetObjective(ScalarProduct(x, values), Optimizer.ObjSense.Maximize);

                // Solve
                prob.Optimize();
                if (prob.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("optimization failed with status " + prob.SolStatus);
                double[] sol = prob.GetSolution();
                // Print out the solution
                Console.WriteLine("Objective: {0}", prob.ObjVal);
                foreach (Variable v in x)
                {
                    Console.Write($"{v.GetName()}: {v.GetValue(sol)} ");
                }
                Console.WriteLine();

                Console.WriteLine("Amending some additional constraints");

                // create a dictionary of key value pairs for easy access to variables by name
                Dictionary<string, Variable> xByName = itemNames.Zip(x).ToDictionary(kvp => kvp.First, kvp => kvp.Second);

                Variable vase = xByName["vase"];

                //
                // Now add constraints to consider the items vase and picture as one pair and
                // the items "tv" and "video" as a second pair so that the pairs are always
                // taken together
                //

                // x["vase"] == x["picture"]
                prob.AddConstraint(vase == xByName["picture"]);

                // x["tv"] == x["video"]
                prob.AddConstraint(xByName["tv"] == xByName["video"]);

                //
                // Logical constraint: Either take the first pair "vase" and "picture" or the
                // second pair "tv" and "video" (but not both pairs).
                //
                Expression tvPlusVideo = 1.0 * xByName["tv"] + 1.0 * xByName["video"];

                // x["vase"] = 1 -> x["tv"] + x["video"]=0
                prob.AddConstraint(vase.IfThen(tvPlusVideo.Leq(0.0)));

                //
                // x["vase"]=0 -> x["tv"]+x["video"]=2
                // Below, we use SetIndicator as an alternative way to create
                // the indicator
                //
                Inequality enablePairTwo = prob.AddConstraint(tvPlusVideo >= 2.0);
                // note that using "false" in the below function
                // means that the inequality is enabled if x["vase"] is 0
                prob.SetIndicator(vase, false, enablePairTwo);

                // save the final problem to file for manual inspection
                prob.WriteProb("BinBurglarIndicator.lp", "l");

                // Solve
                prob.Optimize();
                if (prob.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("optimization failed with status " + prob.SolStatus);

                sol = prob.GetSolution();
                // Print out the solution
                Console.WriteLine("Objective: {0}", prob.ObjVal);
                foreach (Variable v in x)
                {
                    Console.Write($"{v.GetName()}: {v.GetValue(sol)} ");
                }
                Console.WriteLine();
            }
        }
    }
}

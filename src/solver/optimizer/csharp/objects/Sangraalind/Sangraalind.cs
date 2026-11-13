// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Collections.Generic;
using System.Linq;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XPRSexamples
{
    /// <summary>Holy Grail</summary>
    /// <remarks>
    /// When the Sangraal (Holy Grail) is almost won the hero arrives at
    /// a castle where he finds 8 imprisoned knights. He is facing the task to bring
    /// the largest possible number of knights for the arrival of the Sangraal in
    /// twenty minutes' time. The time required for freeing a knight depends on his
    /// state of binding. A freed knight then needs a given amount of time to wash
    /// and recover himself physically.
    /// Model formulation using indicator constraints.
    ///
    /// Description and original model by M. Chlond:
    /// <see href="http://ite.pubs.informs.org/Vol4No3/Chlond">http://ite.pubs.informs.org/Vol4No3/Chlond</see>
    /// </remarks>
    public class Sangraalind
    {
        /// <summary>A knight.</summary>
        public sealed class Knight
        {
            /// <summary>The name of the knight.</summary>
            public readonly string Name;
            /// <summary>How long it takes to free the knight.</summary>
            public readonly double FreeTime;
            /// <summary>How long it takes the knight to wash and prepare himself.</summary>
            public readonly double WashTime;

            public Knight(string name, double freeTime, double washTime)
            {
                this.Name = name;
                this.FreeTime = freeTime;
                this.WashTime = washTime;
            }

            public override string ToString()
            {
                return Name;
            }
        }

        /// <summary>The knights used in this example.</summary>
        private static readonly Knight[] knightArray = new Knight[] { new Knight("Agravain", 1, 15), new Knight("Bors", 1, 5),
            new Knight("Caradoc", 2, 15), new Knight("Dagonet", 2, 5), new Knight("Ector", 3, 10),
            new Knight("Feirefiz", 4, 15), new Knight("Gareth", 5, 10), new Knight("Harry", 6, 5) };

        /// <summary>The resource constraints used in this example.</summary>
        static readonly double maxTime = 20.0;

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /**** VARIABLES ****/

                // Whether to free knight i as the j-th knight
                Variable[,] order = prob.AddVariables(knightArray.Length, knightArray.Length)
                        .WithName((i, j) => $"order_{i}_{j}")
                        .WithType(ColumnType.Binary)
                        .ToArray();

                // Whether the knight who was freed as the i-th knight will be ready in time
                Variable[] onTime = prob.AddVariables(knightArray.Length)
                    .WithName(i => $"onTime_{i}")
                        .WithType(ColumnType.Binary)
                        .ToArray();

                // At what time the i-th freed knight will be ready
                Variable[] readyTime = prob.AddVariables(knightArray.Length)
                    .WithName(i => $"readyTime_{i}")
                        .ToArray();

                // Maximize the number of knights that are ready after 20 minutes
                Expression totalReady = Sum(onTime);

                prob.SetObjective(totalReady, ObjSense.Maximize);

                // Exactly one knight should be freed as the i-th knight
                prob.AddConstraints(knightArray.Length, i => Sum(knightArray.Length, j => order[i,j]) == 1);

                // Each knight should be freed exactly once
                prob.AddConstraints(knightArray.Length, i => Sum(knightArray.Length, j => order[j,i]) == 1);

                // The time each knight is ready, computed as the sum of times it took to free
                // all previous knights, plus the time it took to free this knight plus the
                // time it takes the knight to wash and get ready

                // loop over all positions
                for (int p = 0; p < knightArray.Length; p++)
                {
                    LinExpression timeBeforeFree = LinExpression.Create();
                    // loop over all knights
                    for (int k = 0; k < knightArray.Length; k++)
                    {
                        // loop over all positions before the current one
                        for (int q = 0; q < p; q++)
                        {
                            // If this knight was freed before the current position, add the time it took to
                            // free them
                            timeBeforeFree.AddTerm(order[k,q] * knightArray[k].FreeTime);
                        }
                        // if knight k was freed in this position, add the time it took to free them and
                        // for them to prepare
                        timeBeforeFree.AddTerm(order[k,p] * (knightArray[k].FreeTime + knightArray[k].WashTime));
                    }
                    // add the actual constraint
                    prob.AddConstraint(timeBeforeFree == readyTime[p]);
                }

                // The i-th freed knight will be ready iff they were ready by time 20
                for (int p = 0; p < knightArray.Length; p++)
                {
                    Inequality isReady = prob.AddConstraint(readyTime[p] <= maxTime);
                    prob.SetIndicator(onTime[p], true, isReady);
                }

                // Dump the problem to disk so that we can inspect it.
                prob.WriteProb("sangraalind.lp");

                // Solve
                prob.Optimize();
                if (prob.SolStatus != SolStatus.Optimal && prob.SolStatus != SolStatus.Feasible)
                    throw new Exception($"optimization failed with status {prob.SolStatus}");
                double[] sol = prob.GetSolution();

                // Print the solution
                Console.WriteLine($"{Math.Round(prob.ObjVal)} knights ready in time:");
                for (int p = 0; p < knightArray.Length; p++)
                {
                    for (int k = 0; k < knightArray.Length; k++)
                    {
                        if (order[k,p].GetValue(sol) > 0.5)
                        {
                            string pos;
                            if (p == 0)
                            {
                                pos = "1st";
                            }
                            else if (p == 1)
                            {
                                pos = "2nd";
                            }
                            else if (p == 2)
                            {
                                pos = "3rd";
                            }
                            else
                            {
                                pos = p + 1 + "th";
                            }
                            Console.WriteLine($"{pos} freed knight: {knightArray[k].ToString()} ready at time {readyTime[p].GetValue(sol)}");
                        }
                    }
                }
            }
        }
    }
}

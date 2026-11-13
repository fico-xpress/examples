using System;
using System.Collections.Generic;
using System.Linq;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XPRSexamples
{
    /// <summary>A craftsman makes small wooden boxes for sale.</summary>
    /// <remarks>
    /// He has four different shapes or styles of box, and can make each
    /// of them in any size (keeping all the dimensions in proportion).
    /// The profit he makes on a box depends on the size.
    /// He has only a limited amount of the necessary wood available and a
    /// limited amount of time in the week to do the work.
    /// How many boxes should he make, and what size should they be,
    /// in order to maximize his profit?
    /// This example illustrates how to state non-linear constraints.
    /// </remarks>
    internal class Boxes02
    {
        /// <summary>A box.</summary>
        public sealed class Box
        {
            /// <summary>The name of this box.</summary>
            public readonly string Name;
            /// <summary>Relative length of this box.</summary>
            public readonly double LengthCoeff;
            /// <summary>Relative width of this box.</summary>
            public readonly double WidthCoeff;
            /// <summary>Relative height of this box.</summary>
            public readonly double HeightCoeff;
            /// <summary>Coefficient for the profit of this box, relative to its size.</summary>
            public readonly double ProfitCoeff;
            /// <summary>Coefficient for the production time of this box, relative to its ply.</summary>
            public readonly double TimeCoeff;

            public Box(string name, double lengthCoeff, double widthCoeff, double heightCoeff, double profitCoeff,
                    double timeCoeff)
            {
                this.Name = name;
                this.LengthCoeff = lengthCoeff;
                this.WidthCoeff = widthCoeff;
                this.HeightCoeff = heightCoeff;
                this.ProfitCoeff = profitCoeff;
                this.TimeCoeff = timeCoeff;
            }

            public override string ToString()
            {
                return Name;
            }
        }

        /// <summary>The boxes used in this example.</summary>
        private static readonly Box[] boxArray = new Box[] { new Box("Cube", 1, 1, 1, 20, 1),
            new Box("Oblong", 1, 2, 1, 27.3, 1), new Box("Flat", 4, 4, 1, 90, 1),
            new Box("Economy", 1, 2, 1, 10, 0.2) };

        // The resource constraints used in this example.
        const double maxSize = 2.0;
        const int maxNumProduced = 6;
        const double maxBattens = 200.0;
        const double maxPly = 210.0;
        const double maxMakeTime = 35.0;

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /**** VARIABLES ****/

                // the number of each of the boxes that should be produced
                Variable[] numProduced = prob.AddVariables(boxArray.Length)
                        .WithName(i => $"numProduced_{i}")
                        .WithType(ColumnType.Integer)
                        .WithUB(maxNumProduced)
                        .ToArray();

                // the relative size (a factor of length/width/height) of each of the boxes that
                // should be produced
                Variable[] size = prob.AddVariables(boxArray.Length)
                        .WithName(i => $"size_{i}")
                        .WithUB(maxSize)
                        .ToArray();

                // objective transfer column
                Variable objcol = prob.AddVariable(XPRS.MINUSINFINITY, XPRS.PLUSINFINITY,
                        ColumnType.Continuous, "objCol");

                // some general characteristics of the produced boxes that will be used in the
                // model
                Expression[] battens = new Expression[boxArray.Length];
                Expression[] ply = new Expression[boxArray.Length];
                Expression[] profit = new Expression[boxArray.Length];
                Expression[] makeTime = new Expression[boxArray.Length];

                for (int i = 0; i < boxArray.Length; ++i)
                {
                    // battens = 4 * (lengthCoeff + widhtCoeff + heightCoeff) * size
                    Box b = boxArray[i];
                    battens[i] = size[i] * (4.0 * (b.LengthCoeff + b.WidthCoeff + b.HeightCoeff));
                    // ply = 2 * (lengthCoeff * widthCoeff + widthC * heightC + heightC * lengthC) *
                    // size^2
                    ply[i] = Pow(size[i], 2.0) *
                            2.0 * (b.LengthCoeff * b.WidthCoeff
                                    + b.WidthCoeff * b.HeightCoeff
                                    + b.HeightCoeff * b.LengthCoeff);
                    // profit = profitCoeff * size^1.5
                    profit[i] = b.ProfitCoeff * Pow(size[i], 1.5);
                    // makeTime = 1 + timeCoeff * 1.5^(ply/10)
                    makeTime[i] = 1.0 + b.TimeCoeff * Pow(1.5, ply[i] / 10.0);
                }

                // objective function: sum(boxes) numProduced[box] * profit[box]
                Expression totalProfit = Sum(boxArray.Length, i => numProduced[i] * profit[i]);

                // To make the objective linear, just maximize the objcol
                prob.SetObjective(objcol, ObjSense.Maximize);

                // Add the objective transfer row: objcol = totalProfit
                prob.AddConstraint(objcol == totalProfit);

                // limits on resource availability
                // sum of battens of all produced boxes <= maxBattens
                prob.AddConstraint(Sum(boxArray.Length, i => numProduced[i] * battens[i]) <= maxBattens);
                // sum of ply of all produced boxes <= maxPly
                prob.AddConstraint(Sum(boxArray.Length, i => numProduced[i] * ply[i]) <= maxPly);
                // sum of make time of all produced boxes <= maxMakeTime
                prob.AddConstraint(Sum(boxArray.Length, i => numProduced[i] * makeTime[i]) <= maxMakeTime);

                // Dump the problem to disk so that we can inspect it.
                prob.WriteProb("boxes02.lp");

                // By default we will solve to global optimality, uncomment for an MISLP solve
                // to local optimality
                // prob.controls().setNLPSolver(XPRSconstants.NLPSOLVER_LOCAL);

                // Solve
                prob.Optimize();
                if (prob.SolStatus != SolStatus.Optimal && prob.SolStatus != SolStatus.Feasible)
                    throw new Exception($"optimization failed with status {prob.SolStatus}");
                double[] sol = prob.GetSolution();
                Console.WriteLine($"Objective: {prob.ObjVal}");
                // Print out the solution
                for (int i = 0; i < boxArray.Length; ++i)
                {
                    if (numProduced[i].GetValue(sol) > 0.0)
                    {
                        Console.WriteLine($"Producing {numProduced[i].GetValue(sol)} {boxArray[i].ToString()} boxes of size {size[i].GetValue(sol)}.");
                    }
                }
            }
        }
    }
}

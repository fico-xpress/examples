// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.ConstantExpression;

namespace XpressExamples
{
    /// <summary>Cutting stock problem</summary>
    /// <remarks>
    ///   Solved by column (= cuttingpattern) generation heuristic looping
    ///   over the root node.
    /// </remarks>
    internal class CuttingStock
    {
        const int NWIDTHS = 5;
        const int MAXWIDTH = 94;
        const double EPS = 1e-6;
        const int MAXCOL = 10;

        /****Data****/
        static readonly double[] width = { 17, 21, 22.5, 24, 29.5 }; /* Possible widths */
        static readonly int[] demand = { 150, 96, 48, 108, 227 };  /* Demand per width */

        /****Shared Variables****/
        static Variable[] vPattern; /* Rolls per pattern */
        static Inequality[] cDemand; /* Demand constraints */
        static LinTermMap eObj; /* Objective function */

        private static void PrintProblemSolution(XpressProblem prob)
        {
            double[] sol = prob.GetSolution();
            Console.WriteLine("Objective: " + prob.ObjVal);
            Console.WriteLine("Variables:");
            Console.Write("\t");
            foreach (Variable v in prob.GetVariables())
            {
                Console.Write("[{0}: {1:f2}] ", v.GetName(), v.GetValue(sol));
            }
            Console.WriteLine();
        }


        /**************************************************************************/
        /* Integer Knapsack Algorithm for solving the integer knapsack problem    */
        /*    z = max{cx : ax <= R, x <= d, x in Z^N}                             */
        /*                                                                        */
        /* Input data:                                                            */
        /*   N:        Number of item types                                       */
        /*   c[i]:     Unit profit of item type i, i=1..n                         */
        /*   a[i]:     Unit resource use of item type i, i=1..n                   */
        /*   R:        Total resource available                                   */
        /*   d[i]:     Demand for item type i, i=1..n                             */
        /* Return values:                                                         */
        /*   xbest[i]: Number of items of type i used in optimal solution         */
        /*   zbest:    Value of optimal solution                                  */
        /**************************************************************************/
        private static double SolveKnapsack(int N, double[] c, double[] a, double R, int[] d, int[] xbest)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                Variable[] x = prob.AddVariables(N)
                    .WithType(ColumnType.Integer)
                    .WithName(i => $"x_{i}")
                    .WithLB(i => d[i])
                    .ToArray();

                prob.AddConstraint(
                    ScalarProduct(x, a) <= R
                );

                prob.SetObjective(
                    ScalarProduct(x, c),
                    Optimizer.ObjSense.Maximize
                );

                prob.Optimize();
                double zbest = prob.ObjVal;
                double[] sol = prob.GetSolution();
                xbest = sol.Select(value => (int)Math.Round(value)).ToArray();

                return zbest;
            }
        }

        private static void ModCutStock(XpressProblem p)
        {
            // (Basic) cutting patterns
            int[,] patterns = new int[NWIDTHS, NWIDTHS];
            foreach (int j in Enumerable.Range(0, NWIDTHS))
            {
                patterns[j, j] = (int)Math.Floor((double)MAXWIDTH / width[j]);
            }

            vPattern = p.AddVariables(NWIDTHS)
                .WithType(ColumnType.Integer)
                .WithUB(i => Math.Ceiling((double)demand[i] / patterns[i, i]))
                .WithName(i => $"pat_{i + 1}")
                .ToArray();

            // Minimize total number of rolls
            p.SetObjective(Sum(vPattern));


            // Satisfy the demand per width;
            // Sum of patterns must exceed demand
            cDemand = p.AddConstraints(NWIDTHS,
                    i => (Sum(NWIDTHS, j => vPattern[j] * patterns[i, j]) >= demand[i]).SetName($"Demand_{i}")).ToArray();
        }

        /**************************************************************************/
        /*  Column generation loop at the top node:                               */
        /*    solve the LP and save the basis                                     */
        /*    get the solution values                                             */
        /*    generate new column(s) (=cutting pattern)                           */
        /*    load the modified problem and load the saved basis                  */
        /**************************************************************************/
        private static void SolveCutStock(XpressProblem p)
        {
            // Basis information
            int[] rowstat = new int[p.Rows];
            int[] colstat = new int[p.Cols];
            int[] x = new int[NWIDTHS];
            long starttime = Environment.TickCount64;

            foreach (int npass in Enumerable.Range(0, MAXCOL))
            {
                p.LpOptimize();                // Solve the LP
                if (p.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("failed to optimize with status " + p.SolStatus);
                p.GetBasis(rowstat, colstat);  // Save the current basis

                // solve integer knpsack problem
                double z = SolveKnapsack(NWIDTHS, p.GetDuals(cDemand), width, MAXWIDTH, demand, x);

                Console.WriteLine("Iteration {0:d}, {1:f2} sec",
                    npass + 1, (Environment.TickCount64 - starttime) / 1000.0
                );

                if (z < 1 + EPS)
                {
                    Console.WriteLine("No profitable column found.");
                    Console.WriteLine();
                }
                else
                {
                    Console.WriteLine("New pattern found with marginal cost {0}", z - 1);
                    // Create a new variable for this pattern:
                    Variable v = p.AddVariable(
                        0.0,
                        Double.PositiveInfinity,
                        ColumnType.Integer,
                        String.Format("pat_{0:d}", p.Cols + 1)
                    );
                    // Add new variable to the objective
                    v.ChgObj(1.0);
                    // Add new variable to demand constraints
                    double ub = 0.0;
                    for (int i = 0; i < NWIDTHS; ++i)
                    {
                        if (x[i] > 0)
                        {
                            cDemand[i].ChgCoef(v, x[i]);
                            ub = Math.Max(ub, Math.Ceiling(demand[i] / (double)x[i]));
                        }
                    }
                    // Change the upper bound on the new variable
                    v.SetUB(ub);
                    p.LoadBasis(rowstat, colstat);
                }
            }

            p.Optimize();
            if (p.SolStatus != Optimizer.SolStatus.Optimal)
                throw new Exception("failed to optimize with status " + p.SolStatus);
            PrintProblemSolution(p);
        }

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);
                prob.OutputLog = 0;
                prob.MIPLog = 0;

                ModCutStock(prob);
                SolveCutStock(prob);
            }
        }
    }
}

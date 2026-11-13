// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using Optimizer.Objects;
using Optimizer;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary>Modeling a small MIP problem to perform portfolio optimization.</summary>
    class FolioMip1
    {
        /* Max. number of different assets */
        private static readonly int MAXNUM = 4;
        /* Number of shares */
        private static readonly int NSHARES = 10;
        /* Number of high-risk shares */
        private static readonly int NRISK = 5;
        /* Number of North-American shares */
        private static readonly int NNA = 4;
        /* Estimated return in investment  */
        private static readonly double[] RET = new double[] { 5, 17, 26, 12, 8, 9, 7, 6, 31, 21 };
        /* High-risk values among shares */
        private static readonly int[] RISK = new int[] { 1, 2, 3, 8, 9 };
        /* Shares issued in N.-America */
        private static readonly int[] NA = new int[] { 0, 1, 2, 3 };


        private static void PrintProblemStatus(XpressProblem prob)
        {
            Console.WriteLine("Problem status:");
            Console.WriteLine($"\tSolve status: {prob.SolveStatus}");
            Console.WriteLine($"\tSol status:   {prob.SolStatus}");
        }


        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /****VARIABLES****/
                Variable[] frac = prob.AddVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .WithName(i => $"frac_{i}")
                    /* Upper bounds on the investment per share */
                    .WithUB(0.3)
                    .ToArray();

                Variable[] buy = prob.AddVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .WithName(i => $"buy_{i}")
                    .WithType(ColumnType.Binary)
                    .ToArray();

                /**** CONSTRAINTS ****/
                /* Limit the percentage of high-risk values */
                prob.AddConstraint(Sum(NRISK, i => frac[RISK[i]]).Leq(1.0 / 3.0).SetName("Risk"));

                /* Minimum amount of North-American values */
                prob.AddConstraint(Sum(NNA, i => frac[NA[i]]).Geq(0.5).SetName("NA"));

                /* Spend all the capital */
                prob.AddConstraint(Sum(frac).Eq(1.0).SetName("Cap"));

                /* Limit the total number of assets */
                prob.AddConstraint(Sum(buy).Leq(MAXNUM).SetName("MaxAssets"));

                /* Linking the variables */
                /* frac .<= buy */
                prob.AddConstraints(NSHARES,
                    i => frac[i].Leq(buy[i]).SetName($"link_{i}")
                );

                /* Objective: maximize total return */
                prob.SetObjective(
                    ScalarProduct(frac, RET),
                    Optimizer.ObjSense.Maximize
                );

                /* Solve */
                prob.Optimize();

                /* Solution printing */
                PrintProblemStatus(prob);
                double[] sol = prob.GetSolution();
                Console.WriteLine($"Total return: {prob.ObjVal}");
                foreach (String v in frac.Zip(buy, (f, b) => String.Format("{0} : {1:f2}% ({2:f1})", f.GetName(), 100.0 * f.GetValue(sol), b.GetValue(sol))))
                {
                    Console.WriteLine(v);
                }
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer.Objects;
using Optimizer;
using static Optimizer.Objects.Utils;
using static System.Linq.Enumerable;
using QuadExpression = Optimizer.Objects.QuadExpression;

namespace XpressExamples
{
    /// <summary>Modeling a small QCQP problem to perform portfolio optimization.</summary>
    /// <remarks>Maximize return with limit on variance</remarks>
    class FolioQC
    {
        /* Max. allowed variance */
        private static readonly double MAXVAR = 0.55;
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
        /* Variance/covariance matrix of estimated returns */
        private static readonly double[,] VAR = new double[,]{
            // trs  haw  thr  tel  brw  hgw  car  bnk  sof  elc
            {0.1, 0,   0,   0,   0,   0,   0,   0,   0,   0}, // treasury
            {0,  19,  -2,   4,   1,   1,   1, 0.5,  10,   5}, // hardware
            {0,  -2,  28,   1,   2,   1,   1,   0,  -2,  -1}, // theater
            {0,   4,   1,  22,   0,   1,   2,   0,   3,   4}, // telecom
            {0,   1,   2,   0,   4,-1.5,  -2,  -1,   1,   1}, // brewery
            {0,   1,   1,   1,-1.5, 3.5,   2, 0.5,   1, 1.5}, // highways
            {0,   1,   1,   2,  -2,   2,   5, 0.5,   1, 2.5}, // cars
            {0, 0.5,   0,   0,  -1, 0.5, 0.5,   1, 0.5, 0.5}, // bank
            {0,  10,  -2,   3,   1,   1,   1, 0.5,  25,   8}, // software
            {0,   5,  -1,   4,   1, 1.5, 2.5, 0.5,   8,  16}  // electronics
        };

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

                /**** CONSTRAINTS ****/
                /* Limit variance */
                QuadExpression variance = QuadExpression.Create();
                foreach (int s in Range(0, NSHARES))
                    foreach (int t in Range(0, NSHARES))
                        variance.AddTerm(frac[s], frac[t], VAR[s, t]);

                prob.AddConstraint(variance.Leq(MAXVAR).SetName("Variance"));

                /* Minimum amount of North-American values */
                prob.AddConstraint(Sum(NNA, i => frac[NA[i]]).Geq(0.5).SetName("NA"));

                /* Spend all the capital */
                prob.AddConstraint(Sum(frac).Eq(1.0).SetName("Cap"));

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
                Console.WriteLine($"With of max variance of {MAXVAR} total return: {prob.ObjVal}");
                foreach (Variable f in frac)
                {
                    Console.WriteLine(String.Format("{0} : {1:f2}%", f.GetName(), 100.0 * f.GetValue(sol)));
                }
            }
        }
    }
}

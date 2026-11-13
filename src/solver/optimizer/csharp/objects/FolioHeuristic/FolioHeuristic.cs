// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections;
using Optimizer.Objects;
using Optimizer;
using static System.Linq.Enumerable;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary> Modeling a small LP problem to perform portfolio optimization.</summary>
    /// <remarks>Heuristic solution</remarks>
    class FolioHeuristic
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


        public XpressProblem prob;
        /* Fraction of capital used per share */
        public Variable[] frac;
        /* 1 if asset is in portfolio, 0 otherwise */
        public Variable[] buy;

        public FolioHeuristic(XpressProblem p)
        {
            prob = p;
            /****VARIABLES****/
            frac = p.AddVariables(NSHARES)
                /* Fraction of capital used per share */
                .WithName(i => $"frac_{i}")
                /* Upper bounds on the investment per share */
                .WithUB(0.3)
                .ToArray();

            buy = p.AddVariables(NSHARES)
                /* Fraction of capital used per share */
                .WithName(i => $"buy_{i}")
                .WithType(ColumnType.Binary)
                .ToArray();
        }


        private static void PrintProblemStatus(XpressProblem prob)
        {
            Console.WriteLine("Problem status:");
            Console.WriteLine($"\tSolve status: {prob.SolveStatus}");
            Console.WriteLine($"\tLP status:    {prob.LPStatus}");
            Console.WriteLine($"\tMIP status:   {prob.MIPStatus}");
            Console.WriteLine($"\tSol status:   {prob.SolStatus}");
        }

        private static void PrintProblemSolution(FolioHeuristic folio, String solveFlag)
        {
            XpressProblem prob = folio.prob;
            double[] sol = prob.GetSolution();
            Console.WriteLine($"Total return {solveFlag}: {prob.ObjVal}");
            foreach (int i in Range(0, NSHARES))
            {
                Console.WriteLine(String.Format("{0} : {1:f2}% ({2:f1})", folio.frac[i].GetName(), 100.0 * folio.frac[i].GetValue(sol), folio.buy[i].GetValue(sol)));
            }
        }

        private static void Model(FolioHeuristic folio)
        {
            XpressProblem prob = folio.prob;
            // Output all messages.
            prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

            /**** CONSTRAINTS ****/
            /* Limit the percentage of high-risk values */
            prob.AddConstraint(Sum(NRISK, i => folio.frac[RISK[i]]).Leq(1.0 / 3.0).SetName("Risk"));

            /* Minimum amount of North-American values */
            prob.AddConstraint(Sum(NNA, i => folio.frac[NA[i]]).Geq(0.5).SetName("NA"));

            /* Spend all the capital */
            prob.AddConstraint(Sum(folio.frac).Eq(1.0).SetName("Cap"));

            /* Limit the total number of assets */
            prob.AddConstraint(Sum(folio.buy).Leq(MAXNUM).SetName("MaxAssets"));

            /* Linking the variables */
            /* frac .<= buy */
            prob.AddConstraints(NSHARES,
                i => folio.frac[i].Leq(folio.buy[i]).SetName($"link_{i}")
            );

            /* Objective: maximize total return */
            prob.SetObjective(
                ScalarProduct(folio.frac, RET),
                Optimizer.ObjSense.Maximize
            );
        }

        private static void SolveHeuristic(FolioHeuristic folio)
        {
            XpressProblem p = folio.prob;
            /* Disable automatic cuts - we use our own */
            p.CutStrategy = (int)Optimizer.CutStrategy.None;
            /* Switch presolve off */
            p.Presolve = (int)Optimizer.Presolve.None;
            p.NodePresolve = (int)Optimizer.NodePresolve.Off;
            /* Get feasibility tolerance */
            double tol = p.FeasTol;

            /* Solve the LP-problem */
            p.LpOptimize();

            /* Get Solution */
            double[] sol = p.GetSolution();

            /* Basis information */
            int[] rowstat = new int[p.Rows];
            int[] colstat = new int[p.Cols];
            /* Save the current basis */
            p.GetBasis(rowstat, colstat);

            /* Fix all variables `buy' for which `frac' is at 0 or at a relatively large value */
            double[] fsol = new double[NSHARES];
            foreach (int i in Range(0, NSHARES))
            {
                /* Get the solution values of `frac' */
                fsol[i] = folio.frac[i].GetValue(sol);
                if (fsol[i] < tol) folio.buy[i].Fix(0);
                else if (fsol[i] > 0.2 - tol) folio.buy[i].Fix(1);
            }

            /* Solve with the new bounds on 'buy' */
            p.MipOptimize();
            PrintProblemStatus(p);
            PrintProblemSolution(folio, "Heuristic solution");

            /* Reset variables to their original bounds */
            foreach (int i in Range(0, NSHARES))
            {
                if ((fsol[i] < tol) || (fsol[i] > 0.2 - tol))
                {
                    folio.buy[i].SetLB(0);
                    folio.buy[i].SetUB(1);
                }
            }

            /* Load basis */
            p.LoadBasis(rowstat, colstat);
        }


        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                /* Solve with heuristic */
                FolioHeuristic folio = new FolioHeuristic(prob);
                Model(folio);
                SolveHeuristic(folio);
            }

            using (XpressProblem prob = new XpressProblem())
            {
                FolioHeuristic folio = new FolioHeuristic(prob);
                Model(folio);
                /* Solve */
                folio.prob.Optimize();
                /* Solution printing */
                PrintProblemStatus(prob);
                PrintProblemSolution(folio, "Exact Solve");
            }
        }
    }
}

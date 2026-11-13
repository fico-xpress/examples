// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections.Generic;
using System.Globalization;
using Optimizer.Objects;
using Optimizer;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary>Modeling a MIP problem to perform portfolio optimization.</summary>
    /// <remarks>Defining an integer solution callback</remarks>
    class FolioCB
    {
        /* Path to Data file */
        private static readonly String DATAFILE = (Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") != null ? Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") : ".") + "/folio10.cdat";
        private static readonly int MAXNUM = 15;         /* Max. number of different assets */
        private static readonly double MAXRISK = 1.0 / 3;  /* Max. investment into high-risk values */
        private static readonly double MINREG = 0.2;     /* Min. investment per geogr. region */
        private static readonly double MAXREG = 0.5;     /* Max. investment per geogr. region */
        private static readonly double MAXSEC = 0.25;    /* Max. investment per ind. sector */
        private static readonly double MAXVAL = 0.2;     /* Max. investment per share */
        private static readonly double MINVAL = 0.1;     /* Min. investment per share */
        private static double[] RET;              /* Estimated return in investment  */
        private static int[] RISK;                /* High-risk values among shares */
        private static bool[,] LOC;               /* Geogr. region of shares */
        private static bool[,] SEC;               /* Industry sector of shares */
        private static String[] SHARES;
        private static String[] REGIONS;
        private static String[] TYPES;

        /* Fraction of capital used per share */
        private static Variable[] frac;
        /* 1 if asset is in portfolio, 0 otherwise */
        private static Variable[] buy;

        private static void PrintProblemStatus(XpressProblem prob)
        {
            Console.WriteLine("Problem status:");
            Console.WriteLine($"\tSolve status: {prob.SolveStatus}");
            Console.WriteLine($"\tSol status:   {prob.SolStatus}");
        }

        private static void PrintProblemSolution(XpressProblem prob, bool isCallback)
        {
            double[] sol = isCallback ? prob.GetCallbackSolution() : prob.GetSolution();
            Console.WriteLine($"Total return: {(isCallback ? prob.LPObjVal : prob.ObjVal)}");
            foreach (int i in Enumerable.Range(0, SHARES.Length))
            {
                if (buy[i].GetValue(sol) > 0.5)
                    Console.WriteLine(String.Format("{0} : {1:f2}% ({2:f1})", i, 100.0 * frac[i].GetValue(sol), buy[i].GetValue(sol)));
            }
        }

        public static void Main(string[] args)
        {
            ReadData();
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /****VARIABLES****/
                frac = prob.AddVariables(SHARES.Length)
                    /* Fraction of capital used per share */
                    .WithName(i => $"frac_{i}")
                    /* Upper bounds on the investment per share */
                    .WithUB(MAXVAL)
                    .ToArray();

                buy = prob.AddVariables(SHARES.Length)
                    .WithName(i => $"buy_{i}")
                    .WithType(ColumnType.Binary)
                    .ToArray();

                /**** CONSTRAINTS ****/
                /* Limit the percentage of high-risk values */
                prob.AddConstraint(Sum(RISK.Length, i => frac[RISK[i]]).Leq(MAXRISK).SetName("Risk"));

                /* Limits on geographical distribution */
                prob.AddConstraints(REGIONS.Length,
                    r => Sum(Enumerable.Range(0, SHARES.Length).Where(s => LOC[r, s]).Select(v => frac[v])).In(MINREG, MAXREG)
                );

                /* Diversification across industry sectors */
                prob.AddConstraints(TYPES.Length,
                    t => Sum(Enumerable.Range(0, SHARES.Length).Where(s => SEC[t, s]).Select(v => frac[v])) <= MAXSEC
                );

                /* Spend all the capital */
                prob.AddConstraint(Sum(frac).Eq(1.0).SetName("Cap"));

                /* Limit the total number of assets */
                prob.AddConstraint(Sum(buy).Leq(MAXNUM).SetName("MaxAssets"));

                /* Linking the variables */
                prob.AddConstraints(SHARES.Length,
                    i => frac[i].Geq(buy[i].Mul(MINVAL)).SetName($"link_lb_{i}")
                );
                prob.AddConstraints(SHARES.Length,
                    i => frac[i].Leq(buy[i].Mul(MAXVAL)).SetName($"link_ub_{i}")
                );

                /* Objective: maximize total return */
                prob.SetObjective(
                    ScalarProduct(frac, RET),
                    Optimizer.ObjSense.Maximize
                );

                /* Callback for each new integer solution found */
                prob.callbacks.AddIntsolCallback(p =>
                {
                    PrintProblemSolution(p, true);
                });

                /* Solve */
                prob.Optimize();

                /* Solution printing */
                PrintProblemStatus(prob);
                PrintProblemSolution(prob, false);
            }
        }

        /// <summary>Read a data vector</summary>
        /// <typeparam name="T">Data type.</typeparam>
        /// <param name="tokens">Token provider</param>
        /// <param name="makeData">Function to turn a <c>string</c> token into an instance of <c>T</c>.</param>
        /// <returns>The next vector read from <c>tokens</c>.</returns>
        private static T[] ReadVector<T>(IEnumerator<string> tokens, Func<string, T> makeData)
        {
            List<T> data = new List<T>();
            while (tokens.MoveNext())
            {
                string token = tokens.Current;
                if (token.Equals(";")) // Semicolon terminates vector
                    break;
                data.Add(makeData(token));
            }
            return data.ToArray();
        }
        /// <summary>Read a table of booleans.</summary>
        /// <remarks>
        /// Returns an <c>nrow</c> by <c>ncol</c> table of booleans that is true only in the
        /// positions that are specified in <c>tokens</c>.
        /// </remarks>
        /// <param name="tokens">Token provider.</param>
        /// <param name="nrow">Number of rows.</param>
        /// <param name="ncol">Number of columns.</param>
        /// <returns><c>nrow</c> by <c>ncol</c> boolean array.</returns>
        private static bool[,] ReadBoolTable(IEnumerator<string> tokens, int nrow, int ncol)
        {
            bool[,] table = new bool[nrow, ncol];
            for (int r = 0; r < nrow; ++r)
            {
                while (tokens.MoveNext())
                {
                    string token = tokens.Current;
                    if (token.Equals(";"))
                        break; // Semiconlon terminates row
                    table[r, Int32.Parse(token)] = true;
                }
            }

            return table;
        }

        /// <summary>Fill the static data fields.</summary>
        private static void ReadData()
        {
            // Split the file content into tokens
            IEnumerator<string> tokens = System.IO.File.ReadAllLines(DATAFILE)
                .SelectMany(s => System.Text.RegularExpressions.Regex.Split(s, "\\s+")) // Split tokens at whitespace
                .SelectMany(s => (s.Length > 1 && s.EndsWith(";")) ? new string[] { s.Substring(0, s.Length - 1), ";" } : new string[] { s }) // Split comma into separate token
                .Where(s => s.Length > 0) // filter empty strings
                .GetEnumerator();
            while (tokens.MoveNext())
            {
                string token = tokens.Current;
                if (token.Equals("SHARES:"))
                {
                    SHARES = ReadVector(tokens, s => s);
                }
                else if (token.Equals("REGIONS:"))
                {
                    REGIONS = ReadVector(tokens, s => s);
                }
                else if (token.Equals("TYPES:"))
                {
                    TYPES = ReadVector(tokens, s => s);
                }
                else if (token.Equals("RISK:"))
                {
                    RISK = ReadVector(tokens, s => Int32.Parse(s));
                }
                else if (token.Equals("RET:"))
                {
                    RET = ReadVector(tokens, s => Double.Parse(s, new CultureInfo("en-US")));
                }
                else if (token.Equals("LOC:"))
                    LOC = ReadBoolTable(tokens, REGIONS.Length, SHARES.Length);
                else if (token.Equals("SEC:"))
                    SEC = ReadBoolTable(tokens, TYPES.Length, SHARES.Length);
            }
        }
    }
}

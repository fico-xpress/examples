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
    /// <remarks>
    ///  Same model as in FolioMip3.cs.
    ///  Uses infeasible model parameter values and illustrates
    ///  retrieving MIIS
    /// </remarks>
    class FolioMipIIS
    {
        /* Path to Data file */
        private static readonly String DATAFILE = (Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") != null ? Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") : ".") + "/folio10.cdat";
        private static readonly int MAXNUM = 5;          /* Max. number of different assets */
        private static readonly double MAXRISK = 1.0 / 4;  /* Max. investment into high-risk values */
        private static readonly double MINREG = 0.1;     /* Min. investment per geogr. region */
        private static readonly double MAXREG = 0.25;    /* Max. investment per geogr. region */
        private static readonly double MAXSEC = 0.15;    /* Max. investment per ind. sector */
        private static readonly double MAXVAL = 0.225;   /* Max. investment per share */
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
            Console.WriteLine($"\tLP status:    {prob.LPStatus}");
            Console.WriteLine($"\tMIP status:   {prob.MIPStatus}");
            Console.WriteLine($"\tSol status:   {prob.SolStatus}");
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
                    r => Sum(Enumerable.Range(0, SHARES.Length).Where(s => LOC[r, s]).Select(v => frac[v])).In(MINREG, MAXREG).SetName($"MinMaxReg_{REGIONS[r]}")
                );

                /* Diversification across industry sectors */
                prob.AddConstraints(TYPES.Length,
                    t => Sum(Enumerable.Range(0, SHARES.Length).Where(s => SEC[t, s]).Select(v => frac[v])).In(0.0, MAXSEC).SetName($"LimSec({TYPES[t]})")
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

                /* Solve */
                prob.Optimize();

                /* Solution printing */
                PrintProblemStatus(prob);
                if (prob.SolStatus == Optimizer.SolStatus.Infeasible)
                {
                    Console.WriteLine("MIP infeasible. Retrieving IIS.");
                    // Check there is at least one IIS
                    int status = prob.FirstIIS(1);
                    if (status != 0)
                        throw new Exception("FirstIIS() failed with status " + status);
                    XPRSprob.IISStatusInfo info = prob.IISStatus();
                    Console.WriteLine($"IIS has {info.rowsizes[1]} constraints and {info.colsizes[1]} columns, {info.numinfeas[1]} infeasibilities with an infeasibility of {info.suminfeas[1]}");
                    IIS data = prob.GetIIS(1);
                    Console.WriteLine("Variables in IIS:");
                    foreach (IISVariable v in data.Variables)
                    {
                        // Note that the IISVariable class has more fields than
                        // we print here. See the reference documentation for
                        // details.
                        Console.WriteLine($"\t{v.Variable.GetName()} {v.Domain}");
                    }
                    Console.WriteLine("Constraints in IIS:");
                    foreach (IISConstraint c in data.Constraints)
                    {
                        // Note that the IISVariable class has more fields than
                        // we print here. See the reference documentation for
                        // details.
                        Console.WriteLine($"\t{c.Constraint.GetName()}");
                    }
                }
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

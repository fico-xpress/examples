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
    ///   There are a number of shares (NSHARES) available in which to invest.
    ///   The problem is to split the available capital between these shares
    ///   to maximize return on investment, while satisfying certain
    ///   constraints on the portfolio:
    ///   <list type="bullet'>
    ///     <item><description>
    ///       A maximum of 7 distinct shares can be invest in.
    ///     </description></item>
    ///     <item><description>
    ///       Each share has an associated industry sector and geographic
    ///       region, and the capital may not be overly concentrated in any
    ///       one sector or region.
    ///     </description></item>
    ///     <item><description>
    ///       Some of the shares are considered to be high-risk, and the
    ///       maximum investment in these shares is limited.
    ///     </description></item>
    ///   </list>
    /// </remarks>
    class Folio
    {
        static readonly String DATAFILE = (Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") != null ? Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") : ".") + "/folio10.cdat";

        static readonly int MAXNUM = 7;          /* Max. number of different assets */
        static readonly double MAXRISK = 1.0 / 3;  /* Max. investment into high-risk values */
        static readonly double MINREG = 0.2;     /* Min. investment per geogr. region */
        static readonly double MAXREG = 0.5;     /* Max. investment per geogr. region */
        static readonly double MAXSEC = 0.25;    /* Max. investment per ind. sector */
        static readonly double MAXVAL = 0.2;     /* Max. investment per share */
        static readonly double MINVAL = 0.1;     /* Min. investment per share */


        static double[] RET;          /* Estimated return in investment  */
        static int[] RISK;            /* High-risk values among shares */
        static bool[,] LOC;           /* Geogr. region of shares */
        static bool[,] SEC;           /* Industry sector of shares */

        static String[] SHARES;
        static String[] REGIONS;
        static String[] TYPES;

        public static void Main(string[] args)
        {
            ReadData();                     // Read data from file

            using (XpressProblem prob = new XpressProblem())
            {

                // Create the decision variables
                // Fraction of capital used per share
                Variable[] frac = prob.AddVariables(SHARES.Length)
                        .WithUB(MAXVAL)
                        .WithName("frac {0}")
                        .ToArray();
                // 1 if asset is in portfolio, 0 otherwise
                Variable[] buy = prob.AddVariables(SHARES.Length)
                        .WithType(ColumnType.Binary)
                        .WithName("buy {0}")
                        .ToArray();

                // Objective: total return
                prob.SetObjective(ScalarProduct(frac, RET), Optimizer.ObjSense.Maximize);

                // Limit the percentage of high-risk values
                prob.AddConstraint(Sum(RISK, v => frac[v]) <= MAXRISK);

                // Limits on geographical distribution
                prob.AddConstraints(REGIONS.Length,
                    r => Sum(Enumerable.Range(0, SHARES.Length).Where(s => LOC[r, s]).Select(v => frac[v])).In(MINREG, MAXREG)
                );

                // Diversification across industry sectors
                prob.AddConstraints(TYPES.Length,
                        t => Sum(Enumerable.Range(0, SHARES.Length).Where(s => SEC[t, s]).Select(v => frac[v])) <= MAXSEC);

                // Spend all the capital
                prob.AddConstraint(Sum(frac) == 1);

                // Limit the total number of assets
                prob.AddConstraint(Sum(buy) <= MAXNUM);

                // Linking the variables
                for (int s = 0; s < SHARES.Length; s++)
                {
                    prob.AddConstraint(frac[s] <= buy[s] * MAXVAL);
                    prob.AddConstraint(frac[s] >= buy[s] * MINVAL);
                }

                // Set a time limit of 10 seconds
                prob.TimeLimit = 10.0;

                // Solve the problem
                prob.Optimize();

                Console.WriteLine("Problem status: " + prob.MIPStatus);
                if (prob.MIPStatus != Optimizer.MIPStatus.Solution &&
                        prob.MIPStatus != Optimizer.MIPStatus.Optimal)
                    throw new Exception("optimization failed with status " + prob.MIPStatus);

                // Solution printing
                Console.WriteLine("Total return: " + prob.ObjVal);
                double[] sol = prob.GetSolution();
                for (int s = 0; s < SHARES.Length; s++)
                    if (buy[s].GetValue(sol) > 0.5)
                        Console.WriteLine("  " + s + ": " + frac[s].GetValue(sol) * 100 + "% (" +
                                buy[s].GetValue(sol) + ")");
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

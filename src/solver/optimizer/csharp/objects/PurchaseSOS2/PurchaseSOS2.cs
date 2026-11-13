// (c) 2023-2026 Fair Isaac Corporation

using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.SOS;
using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text.RegularExpressions;
using System.Globalization;

namespace XpressExamples
{
    /// <summary>Purchasing problem with price breaks using Special Ordered Set (SOS) type 2 constraints</summary>
    /// <remarks>
    ///   There are three suppliers of a good, and they have quoted various
    ///   prices for various quantities of product.
    ///   We want to buy at least total cost, yet not buy too much from any
    ///   one supplier.
    ///
    ///   Each supplier offers decreasing prices for increased lot size, in
    ///   the form of incremental discounts.
    /// </remarks>
    class PurchaseSOS2
    {
        static readonly String DATAFILE = (Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") != null ? Environment.GetEnvironmentVariable("EXAMPLE_DATA_DIR") : ".") + "/purchase.cdat";

        const int NB = 4;     /* Number of breakpoints */
        static double REQ;              /* Total quantity required */
        static int[] SUPPLIERS;         /* Suppliers */
        static double[] MAXPERC;        /* Maximum percentages for each supplier */
        static double[][] COST;          /* Industry sector of shares */
        static double[][] BREAKP;        /* Breakpoints (quantities at which unit cost changes) */

        public static void Main(string[] args)
        {
            ReadData();                     // Read data from file

            using (XpressProblem prob = new XpressProblem())
            {

                // Create the decision variables
                // Quantity to purchase from supplier s
                Variable[] buy = prob.AddVariables(SUPPLIERS.Length)
                        .WithUB(s => MAXPERC[s] * REQ / 100.0)
                        .WithName("buy {0}")
                        .ToArray();

                // Weights at breakpoint k for supplier s
                Variable[][] weight = Enumerable.Range(0, SUPPLIERS.Length)
                                .Select(s => prob.AddVariables(NB)
                                          .WithName(b => $"w({s})({b})")
                                          .ToArray())
                                .ToArray();

                // The minimum quantity that must be bought
                prob.AddConstraint(Sum(buy) >= REQ);

                // Define relation between bought quantities and price paid per supplier
                for (int s = 0; s < SUPPLIERS.Length; s++)
                {
                    // The convexity row (weight sum to 1)
                    prob.AddConstraint(Sum(NB, b => weight[s][b]) == 1);

                    // Define buy and also order the weight variables by breakpoint quantities
                    prob.AddConstraint(Sum(NB, b => weight[s][b] * BREAKP[s][b]) == buy[s]);

                    // Define the weight as SOS-2
                    prob.AddConstraint(Sos2(weight[s], BREAKP[s], $"sos_{s}"));
                }

                // Function to calculate cost at breakpoints
                double[][] COSTBREAK = new double[SUPPLIERS.Length][];
                for (int s = 0; s < SUPPLIERS.Length; s++)
                {
                    COSTBREAK[s] = new double[NB];
                    COSTBREAK[s][0] = 0;
                    for (int b = 1; b < NB; b++)
                        COSTBREAK[s][b] = COSTBREAK[s][b - 1] + COST[s][b] * (BREAKP[s][b] - BREAKP[s][b - 1]);
                }

                // Objective: total return
                prob.SetObjective(Sum(NB,
                   b => Sum(SUPPLIERS.Length, s => weight[s][b] * COSTBREAK[s][b])));

                // Solve the problem
                prob.Optimize("");

                Console.WriteLine("Problem status: " + prob.MIPStatus);
                if (prob.MIPStatus != Optimizer.MIPStatus.Solution &&
                        prob.MIPStatus != Optimizer.MIPStatus.Optimal)
                    throw new Exception("optimization failed with status " + prob.MIPStatus);

                // Solution printing
                Console.WriteLine("Total cost: " + prob.ObjVal);
                double[] sol = prob.GetSolution();
                for (int s = 0; s < SUPPLIERS.Length; s++)
                    if (buy[s].GetValue(sol) > 0.5)
                        Console.WriteLine("Supp. " + SUPPLIERS[s] + ": buy:" + buy[s].GetValue(sol));
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
        /// <summary>Read a table of doubles.</summary>
        /// <remarks>
        /// Returns an <c>nrow</c> by <c>ncol</c> table of doubles.
        /// </remarks>
        /// <param name="tokens">Token provider.</param>
        /// <param name="nrow">Number of rows.</param>
        /// <param name="ncol">Number of columns.</param>
        /// <returns><c>nrow</c> by <c>ncol</c> double array.</returns>
        private static double[][] ReadTable(IEnumerator<string> tokens, int nrow, int ncol)
        {
            double[][] table = new double[nrow][];

            for (int r = 0; r < nrow; ++r)
            {
                table[r] = new double[ncol];
                table[r] = ReadVector(tokens, s => Double.Parse(s, new CultureInfo("en-US")));

            }
            return table;
        }

        /// <summary>Fill the static data fields.</summary>
        private static void ReadData()
        {
            // Split the file content into tokens
            IEnumerator<string> tokens = File.ReadAllLines(DATAFILE)
                .SelectMany(s => Regex.Split(s, "\\s+")) // Split tokens at whitespace
                .SelectMany(s => (s.Length > 1 && s.EndsWith(";")) ? new string[] { s.Substring(0, s.Length - 1), ";" } : new string[] { s }) // Split comma into separate token
                .Where(s => s.Length > 0) // filter empty strings
                .GetEnumerator();
            while (tokens.MoveNext())
            {
                string token = tokens.Current;
                if (token.Equals("SUPPLIERS:"))
                {
                    SUPPLIERS = ReadVector(tokens, s => Int32.Parse(s));
                }
                else if (token.Equals("MAXPERC:"))
                {
                    MAXPERC = ReadVector(tokens, s => Double.Parse(s, new CultureInfo("en-US")));
                }
                else if (token.Equals("REQ:"))
                {
                    tokens.MoveNext();
                    REQ = Double.Parse(tokens.Current, new CultureInfo("en-US"));
                }
                else if (token.Equals("BREAKP:"))
                {
                    BREAKP = ReadTable(tokens, SUPPLIERS.Length, NB);
                }
                else if (token.Equals("COST:"))
                {
                    COST = ReadTable(tokens, SUPPLIERS.Length, NB);
                }
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary>Contract allocation example.</summary>
    class ContractAllocation
    {
        static readonly int NDISTRICT = 6;   /* Number of districts */
        static readonly int NCONTRACT = 10;  /* Number of contracts */

        private static readonly double[] output = new double[]{ /* Max. output per district */
        50, 40, 10, 20, 70, 50
    };

        private static readonly double[] cost = new double[]{ /* Cost per district */
        50, 20, 25, 30, 45, 40
    };

        private static readonly double[] volume = new double[]{ /* Volume of contracts */
        20, 10, 30, 15, 20, 30, 10, 50, 10, 20
    };

        public static void Main(string[] args)
        {

            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /**** VARIABLES ****/
                /* Variables indicating whether a project is chosen */
                Variable[,] x = prob.AddVariables(NDISTRICT, NCONTRACT)
                    .WithType(ColumnType.Binary)
                    .WithName((d, c) => $"x_d{d + 1}_c{c + 1}")
                    .ToArray();
                /* Quantities allocated to contractors */
                Variable[,] y = prob.AddVariables(NDISTRICT, NCONTRACT)
                    .WithType(ColumnType.SemiContinuous)
                    .WithUB((d, c) => output[d])
                    .WithName((d, c) => $"q_d{d + 1}_c{c + 1}")
                    .WithLimit(5) // Set limit for the semi-continous variable
                    .ToArray();

                /**** CONSTRAINTS ****/
                // Cover the required volume
                //  for all c in [0,NCONTRACT[
                //    sum(d in [0,NDISTRICT[) y[d][c] >= volume[c]
                prob.AddConstraints(NCONTRACT,
                        c => Sum(NDISTRICT, d => y[d, c]).Geq(volume[c]).SetName($"Size_{c}")
                        );

                // "Min": at least 2 districts / contract
                //  for all c in [0,NCONTRACT[
                //    sum(d in [0,NDISTRICT[) x[d][c] >= 2
                prob.AddConstraints(NCONTRACT,
                        c => Sum(NDISTRICT, d => x[d, c]).Geq(2.0).SetName($"Min_{c}")
                        );

                // Do not exceed max. output
                //  for all d in [0,NDISTRICT[
                //    sum(c in [0,NCONTRACT[) y[d][c] <= output[d]
                prob.AddConstraints(NDISTRICT,
                        d => Sum(NCONTRACT, c => y[d, c]).Leq(output[d]).SetName($"Output_{d}")
                        );

                // If a contract is allocated to a district, then at least 1 unit is allocated to it
                //  for all d in [0,NDISTRICT[
                //    for all c in [0,NCONTRACT[
                //      x[d][c] <= y[d][c]
                prob.AddConstraints(NDISTRICT, NCONTRACT,
                        (d, c) => x[d, c].Leq(y[d, c]).SetName($"XY_{d}{c}")
                        );

                /****OBJECTIVE****/
                prob.SetObjective(
                    Sum(
                        NCONTRACT,
                        c => Sum(NDISTRICT, d => y[d, c] * cost[d])),
                    Optimizer.ObjSense.Minimize
                );

                // Output the matrix in LP format
                prob.WriteProb("Contract.lp", "l");
                // Solve
                prob.Optimize();
                if (prob.SolStatus != Optimizer.SolStatus.Optimal &&
                        prob.SolStatus != Optimizer.SolStatus.Feasible)
                    throw new Exception("optimization failed with status " + prob.SolStatus);
                // Print out the solution
                double[] sol = prob.GetSolution();
                Console.WriteLine("Objective: {0}", prob.ObjVal);
                foreach (Variable v in y)
                {
                    if (v.GetValue(sol) > 0.5)
                        Console.WriteLine($"{v.GetName()}:{v.GetValue(sol)} ");
                }
            }
        }
    }
}

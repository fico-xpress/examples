using System;
using System.Collections.Generic;
using System.Linq;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XPRSexamples
{
    /// <summary>Capital budgeting problem.</summary>
    /// <remarks>
    /// Illustrates logical conditions, formulation using logic constraints
    /// and indicator constraints.
    /// </remarks>
    internal class CapBgt2l
    {
        /// <summary>A project.</summary>
        public sealed class Project
        {
            /// <summary>The name of this project.</summary>
            public readonly string Name;
            /// <summary>Payout for the project.</summary>
            public readonly double Payout;
            /// <summary>Capital investment for the project.</summary>
            public readonly double Investment;
            /// <summary>Number of personnel required for the project.</summary>
            public readonly int Personnel;

            public Project(String name, double payout, double investment, int personnel)
            {
                this.Name = name;
                this.Payout = payout;
                this.Investment = investment;
                this.Personnel = personnel;
            }

            public override string ToString()
            {
                return Name;
            }
        }

        /** The projects used in this example. */
        private static readonly Project[] projectArray = new Project[] { new Project("Alpha", 124000.0, 104000.0, 22),
            new Project("Beta", 74000.0, 53000.0, 12), new Project("Gamma", 42000.0, 29000.0, 7),
            new Project("Delta", 188000.0, 187000.0, 36), new Project("Epsilon", 108000.0, 98000.0, 24),
            new Project("Zeta", 56000.0, 32000.0, 10), new Project("Eta", 88000.0, 75000.0, 20),
            new Project("Theta", 225000.0, 200000.0, 41) };

        // The resource constraints used in this example.
        const double budget = 478000.0;
        const int workforce = 106;

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /**** VARIABLES ****/

                // Whether to undertake a specific project
                Variable[] x = prob.AddVariables(projectArray.Length)
                        .WithName(i => $"x_{i}")
                        .WithType(ColumnType.Binary)
                        .ToArray();

                // objective function: sum of payouts of all undertaken projects
                Expression totalProfit = Sum(projectArray.Length, i => x[i] * projectArray[i].Payout);

                prob.SetObjective(totalProfit, ObjSense.Maximize);

                // limits on resource availability
                // sum of investments of all undertaken projects should not exceed budget
                Inequality investmentLimit = prob
                        .AddConstraint(Sum(projectArray.Length, i => x[i] * projectArray[i].Investment) <= budget);
                // sum of personnel committed of all undertaken projects should not exceed
                // workforce
                Inequality workforceLimit = prob
                        .AddConstraint(Sum(projectArray.Length, i => x[i] * projectArray[i].Personnel) <= workforce);

                // project alpha can only be done if both gamma and zeta happen
                prob.AddConstraint(x[0] <= x[2]);
                prob.AddConstraint(x[0] <= x[5]);

                // project zeta can only be done if project epsilon happens
                prob.AddConstraint(x[5] <= x[4]);

                // projects alpha and beta as well as gamma and delta can only happen together
                prob.AddConstraint(x[0] == x[1]);
                prob.AddConstraint(x[2] == x[3]);

                // exactly one of those pairs should be invested in, i.e., if project alpha is
                // performed,
                // neither gamma nor delta can be invested in, and if project alpha does not
                // happen, then
                // projects gamma and delta have to be performed
                prob.AddConstraint(x[0].IfThen(x[2] + x[3] <= 0.0));
                prob.AddConstraint(x[0].IfNotThen(x[2] + x[3] == 2.0));

                // Dump the problem to disk so that we can inspect it.
                prob.WriteProb("capbgt2l.lp");

                // Solve
                prob.Optimize();
                if (prob.SolStatus != SolStatus.Optimal && prob.SolStatus != SolStatus.Feasible)
                    throw new Exception($"optimization failed with status {prob.SolStatus}");
                double[] sol = prob.GetSolution();

                // Print the objective
                Console.WriteLine($"Objective: {prob.ObjVal}");

                // Print the interesting slacks
                Console.WriteLine($"Remaining Budget: {investmentLimit.GetSlack()}");
                Console.WriteLine($"Remaining Workers: {Math.Round(workforceLimit.GetSlack())}");

                // Print out the solution
                for (int i = 0; i < projectArray.Length; ++i)
                {
                    if (x[i].GetValue(sol) > 0.5)
                    {
                        Console.WriteLine($"Undertaking project {projectArray[i].ToString()}.");
                    }
                }
            }
        }
    }
}

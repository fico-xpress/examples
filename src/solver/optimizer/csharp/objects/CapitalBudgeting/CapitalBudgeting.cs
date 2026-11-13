// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary>Capital budgeting example, solved using three multi-objective approaches</summary>
    /// <remarks>
    /// <list type='number'>
    /// <item><description>
    ///   Lexicographic approach, solving first to minimize capital expended
    ///   and second to maximize return
    /// </description></item>
    /// <item><description>
    ///   Blended approach, solving a weighted combination of both objectives
    ///   simultaneously
    /// </description></item>
    /// <item><description>
    ///   Lexicographic approach, with the objective priorities reversed
    /// </description></item>
    /// </list>
    class CapitalBudgeting : IDisposable
    {
        /// Required capital for each project
        static readonly double[] CAPITAL = { 104000, 53000, 29000, 187000, 98000, 32000, 75000, 200000 };
        /// Required personnel for each project
        static readonly double[] PERSONNEL = { 22, 12, 7, 36, 24, 10, 20, 41 };
        /// Expected return of each project
        static readonly double[] RETURN = { 124000, 74000, 42000, 188000, 108000, 56000, 88000, 225000 };
        /// Target capital to invest
        static readonly double CAPITAL_TARGET = 478000;
        /// Target return on investment
        static readonly double ROI_TARGET = 550000;
        /// Available personnel
        static readonly int PERSONNEL_MAX = 106;

        private readonly XpressProblem prob;

        /// Binary decision variables indicating which projects will be implemented
        private Variable[] select;

        /// Constraint for the number of personnel available
        private Inequality personnel;

        /// Primary objective: minimize capital expended
        private LinExpression capital;

        /// Secondary objective: maximize return on investment
        private LinExpression roi;

        public CapitalBudgeting()
        {
            prob = new XpressProblem();
        }

        public void Dispose()
        {
            prob.Dispose();
        }

        public static void Main(string[] args)
        {
            using (CapitalBudgeting capBudget = new CapitalBudgeting())
            {
                capBudget.Solve();
            }
        }

        public void Solve()
        {
            // Binary decision variables indicating which projects will be implemented
            select = prob.AddVariables(CAPITAL.Length)
                .WithType(ColumnType.Binary)
                .WithName("select_{0}")
                .ToArray();

            // Constraint: at least 3 projects must be implemented
            prob.AddConstraint(Sum(select) >= 3);

            // Constraint for the number of personnel available
            personnel = prob.AddConstraint(ScalarProduct(select, PERSONNEL) <= PERSONNEL_MAX);

            // Primary objective: minimize capital expended
            capital = ScalarProduct(select, CAPITAL);
            // The first objective has priority=1 (weight=1 by default)
            prob.SetObjective(capital, ObjSense.Minimize);
            prob.SetObjIntControl(0, ObjControl.Priority, 1);

            // Secondary objective: maximize return on investment
            roi = ScalarProduct(select, RETURN);
            // The second objective has weight=-1 to maximize this expression, and priority=0
            // to cause this objective to be solved in a second optimization
            prob.AddObjective(roi, 0, -1);

            prob.Optimize();
            PrintSolution("Higher priority for 'Minimize Capital' objective:");

            // Now set the same priority for both objectives. This will result in a single solve
            // using the weighted sum of the two objectives.
            prob.SetObjIntControl(1, ObjControl.Priority, 1);
            prob.Optimize();
            PrintSolution("Equal priority for objectives:");

            // Finally, give the first objective a lower priority
            prob.SetObjIntControl(0, ObjControl.Priority, 0);
            prob.Optimize();
            PrintSolution("Higher priority for 'Maximize Return' objective:");
        }

        /// <summary>
        /// Prints the current solution to the problem.
        /// </summary>
        /// <param name="title">a title to print before the solution</param>
        private void PrintSolution(String title)
        {
            Console.WriteLine(title);
            if (prob.SolveStatus != SolveStatus.Completed ||
                prob.SolStatus != SolStatus.Optimal)
            {
                Console.WriteLine("  Problem not solved");
            }
            else
            {
                Console.WriteLine($"  Objectives: {prob.Objectives}, solved objectives: {prob.SolvedObjs}");

                double[] sol = prob.GetSolution();
                double capitalUsed = capital.Evaluate(sol);
                double roiGained = roi.Evaluate(sol);
                if (capitalUsed > CAPITAL_TARGET)
                {
                    Console.WriteLine("  Unable to meet Capital target");
                }
                if (roiGained < ROI_TARGET)
                {
                    Console.WriteLine("  Unable to meet Return target");
                }

                Console.Write("  Projects undertaken:");
                for (int p = 0; p < select.Length; p++)
                {
                    if (select[p].GetSolution() >= 0.99)
                    {
                        Console.Write($" {p}");
                    }
                }
                Console.WriteLine();

                Console.WriteLine($"  Used capital: ${capitalUsed}," +
                    (CAPITAL_TARGET >= capitalUsed ? " unused: $" : " overused: $") +
                    Math.Abs(CAPITAL_TARGET - capitalUsed));
                Console.WriteLine($"  Total return: ${roiGained}");
                Console.WriteLine($"  Unused personnel: {(int)personnel.GetSlack()} persons");
            }
        }
    }
}

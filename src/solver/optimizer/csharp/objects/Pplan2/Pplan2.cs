// (c) 2023-2026 Fair Isaac Corporation

using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.SOS;
using System;
using System.Linq;

namespace XpressExamples
{
    /// <summary>Project planning: Defining SOS1</summary>
    /// <remarks>
    ///   A company has several projects that it must undertake in the next
    ///   few months.
    ///   Each project lasts for a given time (its duration) and uses up one
    ///   resource as soon as it starts.
    ///   The resource profile is the amount of the resource that is used in
    ///   the months following the start of the project.
    ///   For instance, project 1 uses up 3 units of resource in the month it
    ///   starts, 4 units in its second month, and 2 units in its last month.
    ///   The problem is to decide when to start each project, subject to not
    ///   using more of any resource in a given month than is available.
    ///   The benefit from the project only starts to accrue when the project
    ///   has been completed, and then it accrues at BEN[p] per month for
    ///   project p, up to the end of the time horizon.
    /// </remarks>
    class Pplan2
    {
        static readonly string[] PROJ = { "A", "B", "C" };                    /* Set of projects */
        const int NM = 6;                                                   /* Time horizon (months) */
        static readonly int[] MONTHS = System.Linq.Enumerable.Range(1, NM).ToArray();   /* Set of time periods (months) to plan for */
        static readonly int[] DUR = { 3, 3, 4 };                              /* Duration of project p */
        static readonly int[] RESMAX = { 5, 6, 7, 7, 6, 6 };                  /* Resource available in month m */
        static readonly double[] BEN = { 10.2, 12.3, 11.2 };                  /* Benefit per month once project finished */
        static readonly int[][] RESUSE =                                    /* Res. usage of proj. p in its t'th month */
            new int[][] { new int[] { 3, 4, 2, 0, 0, 0 }, new int[] { 4, 1, 5, 0, 0, 0 }, new int[] { 3, 2, 1, 2, 0, 0 } };

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {

                // Create the decision variables
                // 1 if proj p starts in month m, else 0
                Variable[][] start = System.Linq.Enumerable.Range(0, PROJ.Length)
                                .Select(p => prob.AddVariables(NM)
                                    .WithName(m => $"start({PROJ[p]})({MONTHS[m]})")
                                    .ToArray())
                                .ToArray();

                // Each project starts once and only once
                prob.AddConstraints(PROJ.Length, p => Sum(start[p]) == 1);

                // Resource availability. A project starting in month m is in its k-m month in month k:
                // sum(p in PROJ, m in 0..k) RESUSE(p,k-m)*start(p,m) <= RESMAX(k)
                prob.AddConstraints(NM, k => Sum(PROJ.Length, k + 1, (p, m) => start[p][m] * RESUSE[p][k - m]) <= RESMAX[k]);

                /* Define SOS-1 sets that ensure that at most one start(p,m) is non-zero
                   for each project p. Use month index to order the variables */
                prob.AddConstraints(PROJ.Length, p => Sos1(start[p], null, $"sos_{p}"));

                /* Objective: Maximize Benefit
                    If project p starts in month m, it finishes in month m+DUR(p)-1 and
                    contributes a benefit of BEN(p) for the remaining NM-(m+DUR(p)-1) months:  */
                LinExpression obj = LinExpression.Create();
                for (int p = 0; p < PROJ.Length; p++)
                {
                    for (int m = 0; m < NM - DUR[p]; m++)
                    {
                        obj.AddTerm(start[p][m], BEN[p] * (NM - m - DUR[p]));
                    }
                }
                prob.SetObjective(obj, ObjSense.Maximize);

                // Solve the problem
                prob.Optimize("");

                System.Console.WriteLine("Problem status: " + prob.MIPStatus);
                if (prob.MIPStatus != Optimizer.MIPStatus.Solution &&
                        prob.MIPStatus != Optimizer.MIPStatus.Optimal)
                    throw new System.Exception("optimization failed with status " + prob.MIPStatus);

                // Solution printing
                System.Console.WriteLine("Solution value is: " + prob.ObjVal);
                double[] sol = prob.GetSolution();
                for (int p = 0; p < PROJ.Length; p++)
                    for (int m = 0; m < NM; m++)
                        if (start[p][m].GetValue(sol) > 0.5)
                            System.Console.WriteLine("Project " + PROJ[p] + " starts in month " + MONTHS[m]);
            }
        }
    }
}

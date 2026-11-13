// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary>Economic lot sizing, ELS, problem</summary>
    /// <remarks>
    ///   Solved by adding (l,S)-inequalities in several rounds looping over
    ///   the root node.
    ///
    ///   ELS considers production planning over a horizon
    ///   of T periods. In period t, t=1,...,T, there is a
    ///   given demand DEMAND[t] that must be satisfied by
    ///   production prod[t] in period t and by inventory
    ///   carried over from previous periods. There is a
    ///   set-up up cost SETUPCOST[t] associated with
    ///   production in period t. The unit production cost
    ///   in period t is PRODCOST[t]. There is no inventory
    ///   or stock-holding cost.
    /// </remarks>
    class ELS
    {
        private static readonly double EPS = 1e-6;
        private static readonly int T = 6;                 /* Number of time periods */

        /* Data */
        private static readonly double[] DEMAND = { 1, 3, 5, 3, 4, 2 };  /* Demand per period */
        private static readonly double[] SETUPCOST = { 17, 16, 11, 6, 9, 6 };  /* Setup cost / period */
        private static readonly double[] PRODCOST = { 5, 3, 2, 1, 3, 1 };  /* Prod. cost / period */
        private static double[,] D;                       /* Total demand in periods t1 - t2 */

        /* Variables and constraints */
        private static Variable[] prod;                  /* Production in period t */
        private static Variable[] setup;                 /* Setup in period t */

        /***********************************************************************/
        private static void ModEls(XpressProblem p)
        {
            D = new double[T, T];
            for (int s = 0; s < T; s++)
                for (int t = 0; t < T; t++)
                    for (int k = s; k <= t; k++)
                        D[s, t] += DEMAND[k];

            // Variables
            prod = p.AddVariables(T)
                .WithType(ColumnType.Continuous)
                .WithName(t => $"prod{t + 1}")
                .ToArray();

            setup = p.AddVariables(T)
                .WithType(ColumnType.Binary)
                .WithName(t => $"setup{t + 1}")
                .ToArray();

            // Objective: Minimize total cost
            p.SetObjective(
                Sum(
                    ScalarProduct(setup, SETUPCOST),
                    ScalarProduct(prod, PRODCOST)
                ),
                Optimizer.ObjSense.Minimize
            );

            // Constraints

            // Production in period t must not exceed the total demand for the
            // remaining periods; if there is production during t then there
            // is a setup in t
            //  for all t in [0,T[
            //    prod[t] <= setup[t] * D[t][T-1]
            p.AddConstraints(T,
                    t => prod[t].Leq(setup[t] * D[t, T - 1]).SetName($"Production_{t}")
                    );

            // Production in periods 0 to t must satisfy the total demand
            // during this period of time
            //  for all t in [0,T[
            //    sum(s in [0,t+1[) prod[s] >= D[0][t]
            p.AddConstraints(T,
                    t => Sum(t + 1, t => prod[t]).Geq(D[0, t]).SetName($"Demand_{t}")
                    );
            p.WriteProb("ELS.lp", "l");
        }

        /**************************************************************************/
        /*  Cut generation loop at the top node:                                  */
        /*    solve the LP and save the basis                                     */
        /*    get the solution values                                             */
        /*    identify and set up violated constraints                            */
        /*    load the modified problem and load the saved basis                  */
        /**************************************************************************/
        private static void SolveEls(XpressProblem p)
        {
            // Output all messages.
            p.callbacks.AddMessageCallback(DefaultMessageListener.Console);
            /* Disable automatic cuts - we use our own */
            p.CutStrategy = (int)Optimizer.CutStrategy.None;
            /* Switch presolve off */
            p.Presolve = (int)Optimizer.Presolve.None;

            int ncut = 0, npass = 0, npcut = 0;
            long starttime = Environment.TickCount64;
            double[] sol;

            do
            {
                p.WriteProb("model" + npass + ".lp", "l");
                npass++;
                npcut = 0;
                // Solve the LP-problem
                p.LpOptimize();
                if (p.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("failed to optimize with status " + p.SolStatus);
                // Get the solution values:
                sol = p.GetSolution();
                // Search for violated constraints:
                for (int l = 0; l < T; l++)
                {
                    double ds = 0.0;
                    for (int t = 0; t <= l; t++)
                    {
                        if (prod[t].GetValue(sol) < D[t, l] * setup[t].GetValue(sol) + EPS)
                        {
                            ds += prod[t].GetValue(sol);
                        }
                        else
                        {
                            ds += D[t, l] * setup[t].GetValue(sol);
                        }
                    }

                    /* Add the violated inequality: the minimum of the actual production
                       prod[t] and the maximum potential production D[t][l]*setup[t]
                       in periods 0 to l must at least equal the total demand in periods
                       0 to l.
                       sum(t=1:l) min(prod[t], D[t][l]*setup[t]) >= D[0][l] */
                    if (ds < D[0, l] - EPS)
                    {
                        LinExpression cut = new LinTermMap(0);
                        for (int t = 0; t <= l; t++)
                        {
                            if (prod[t].GetValue(sol) < D[t, l] * setup[t].GetValue(sol) + EPS)
                                cut.AddTerm(prod[t]);
                            else
                                cut.AddTerm(setup[t] * D[t, l]);
                        }
                        p.AddConstraint((cut >= D[0, l]).SetName($"cut_{ncut + 1}"));
                        ncut++;
                        npcut++;
                    }
                }
                Console.WriteLine("Iteration {0:d}, {1:f2} sec, objective value: {2:f}, cuts added: {3:d} (total {4:d})",
                    npass,
                    (Environment.TickCount64 - starttime) / 1000.0,
                    p.ObjVal,
                    npcut,
                    ncut
                );

                if (npcut == 0)
                    Console.WriteLine("Optimal integer solution found:");

            } while (npcut > 0);

            // Print out the solution:
            for (int t = 0; t < T; t++)
            {
                Console.WriteLine(
                        "Period {0:d}: prod {1:f1} (demand: {2:f0}, cost: {3:f0}), setup {4:f0} (cost {5:f0})",
                        t + 1,
                        prod[t].GetValue(sol),
                        DEMAND[t],
                        PRODCOST[t],
                        setup[t].GetValue(sol),
                        SETUPCOST[t]
                );
            }
        }

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                ModEls(prob);        // Model the problem
                SolveEls(prob);      // Solve the problem
            }
        }
    }
}

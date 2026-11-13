// (c) 2025-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XpressExamples
{
    /// <summary>Demonstrates how to implement cutting planes as part of a MIP branch-and-bound search using the cutround callback.</summary>
    /// <remarks>
    /// Cuts are added as user cuts using XpressProblem.AddManagedCut().
    ///
    /// Economic lot sizing problem. Solved by adding (l,S)-inequalities
    /// in a branch-and-cut heuristic (using the cutround callback).
    ///
    /// ELS considers production planning over a horizon of T periods. In period t,
    /// t=1,...,T, there is a given demand DEMAND[p,t] that must be satisfied by
    /// production produce[p,t] in period t and by inventory carried over
    /// from previous periods.
    /// There is a set-up cost SETUPCOST[t] associated with production in
    /// period t and the total production capacity per period is limited. The unit
    /// production cost in period t is PRODCOST[p,t]. There is no
    /// inventory or stock-holding cost.
    ///
    /// A well-known class of valid inequalities for ELS are the
    /// (l,S)-inequalities.  Let D(p,q) denote the demand in periods p
    /// to q and y(t) be a binary variable indicating whether there is any
    /// production in period t.  For each period l and each subset of periods S
    /// of 1 to l, the (l,S)-inequality is
    /// <pre>
    ///      sum (t=1:l | t in S) x(t) + sum (t=1:l | t not in S) D(t,l)/// y(t)
    ///          &gt;= D(1,l)
    /// </pre>
    ///
    /// It says that actual production x(t) in periods included S plus maximum
    /// potential production D(t,l)*y(t) in the remaining periods (those not in
    /// S) must at least equal total demand in periods 1 to l.  Note that in
    /// period t at most D(t,l) production is required to meet demand up to
    /// period l.
    ///
    /// Based on the observation that
    /// <pre>
    ///      sum (t=1:l | t in S) x(t) + sum (t=1:l | t not in S) D(t,l)/// y(t)
    ///          &gt;= sum (t=1:l) min(x(t), D(t,l)/// y(t))
    ///          &gt;= D(1,l)
    /// </pre>
    /// it is easy to develop a separation algorithm and thus a cutting plane
    /// algorithm based on these (l,S)-inequalities.
    /// </remarks>
    class ELSManagedCuts
    {
        /** Tolerance for satisfiability. */
        private const double EPS = 1e-6;

        /** Number of time periods. */
        private const int DIM_TIMES = 15;
        /** Number of products to produce. */
        private const int DIM_PRODUCTS = 4;

        /** Demand per product (first dim) and time period (second dim). */
        private static readonly int[,] DEMAND = new int[,]{
            { 2, 3, 5, 3, 4, 2, 5, 4, 1, 3, 4, 2, 3, 5, 2},
            {3, 1, 2, 3, 5, 3, 1, 2, 3, 3, 4, 5, 1, 4, 1},
            {3, 5, 2, 1, 2, 1, 3, 3, 5, 2, 2, 1, 3, 2, 3},
            {2, 2, 1, 3, 2, 1, 2, 2, 3, 3, 2, 2, 3, 1, 2}
        };
        /** Setup cost. */
        private static readonly double[] SETUPCOST = new double[]{
            17, 14, 11, 6, 9, 6, 15, 10, 8, 7, 12, 9, 10, 8, 12
        };
        /** Production cost per product (first dim) and time period (second dim). */
        private static readonly double[,] PRODCOST = new double[,]{
            {5, 3, 2, 1, 3, 1, 4, 3, 2, 2, 3, 1, 2, 3, 2},
            {1, 4, 2, 3, 1, 3, 1, 2, 3, 3, 3, 4, 4, 2, 2},
            {3, 3, 3, 4, 4, 3, 3, 3, 2, 2, 1, 1, 3, 3, 3},
            {2, 2, 2, 3, 3, 3, 4, 4, 4, 3, 3, 2, 2, 2, 3}
        };
        /** Capacity. */
        private static int[] CAP = new int[]{
            12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
        };

        /** Cut round callback for separating our cutting planes. */
        class Callback
        {
            private Variable[,] produce;
            private Variable[,] setup;
            private double[,,] sumDemand;
            public Callback(Variable[,] produce, Variable[,] setup, double[,,] sumDemand)
            {
                this.produce = produce;
                this.setup = setup;
                this.sumDemand = sumDemand;
            }
            public void CutRound(XpressProblem prob, int ifxpresscuts, ref int action)
            {
                // Apply only one round of cutting on each node.
                // Because the CutRound callback is fired *before* a round of
                // cutting, the CUTROUNDS attribute will start from 0 on the first
                // invocation of the callback.
                if (prob.CutRounds >= 1)
                    return;

                // Get the solution vector.
                // Xpress will only fire the CutRound callback when a solution is
                // available, so there is no need to check whether a solution is
                // available.
                double[] sol = prob.GetCallbackSolution();

                // Search for violated constraints : the minimum of the actual
                // production produce[p,t] and the maximum potential production
                // D[p,t,l]*setup[p,t] in periods 0 to l must at least equal
                // the total demand in periods 0 to l.
                //    sum(t=1:l) min(prod[p,t], D[p,t,l]*setup[p,t]) >= D[p,0,l]
                int nCutsAdded = 0;
                for (int p = 0; p < DIM_PRODUCTS; p++)
                {
                    for (int l = 0; l < DIM_TIMES; l++)
                    {
                        double sum = 0.0;
                        for (int t = 0; t <= l; t++)
                        {
                            if (produce[p, t].GetValue(sol) < sumDemand[p, t, l] * setup[p, t].GetValue(sol) + EPS)
                            {
                                sum += produce[p, t].GetValue(sol);
                            }
                            else
                            {
                                sum += sumDemand[p, t, l] * setup[p, t].GetValue(sol);
                            }
                        }
                        if (sum < sumDemand[p, 0, l] - EPS)
                        {
                            // Create the violated cut.
                            LinExpression cut = LinExpression.Create();
                            for (int t = 0; t <= l; t++)
                            {
                                if (produce[p, t].GetValue(sol) < sumDemand[p, t, l] * setup[p, t].GetValue(sol))
                                {
                                    cut.AddTerm(1.0, produce[p, t]);
                                }
                                else
                                {
                                    cut.AddTerm(sumDemand[p, t, l], setup[p, t]);
                                }
                            }
                            // Give the cut to Xpress to manage.
                            // It will automatically be presolved.
                            prob.AddManagedCut(true, cut >= sumDemand[p, 0, l]);
                            ++nCutsAdded;

                            // If we modified the problem in the callback, Xpress
                            // will automatically trigger another roound of cuts,
                            // so there is no need to set the action return
                            // argument.
                        }
                    }
                }
                if (nCutsAdded > 0)
                {
                    Console.WriteLine("Cuts added: {0}", nCutsAdded);
                }
            }
        }

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                // Create an economic lot sizing problem.
                // Calculate demand D(p, s, t) as the demand for product p from
                // time s to time t(inclusive).
                double[,,] sumDemand = new double[DIM_PRODUCTS, DIM_TIMES, DIM_TIMES];
                for (int p = 0; p < DIM_PRODUCTS; p++)
                {
                    for (int s = 0; s < DIM_TIMES; s++)
                    {
                        double thisSumDemand = 0.0;
                        for (int t = s; t < DIM_TIMES; t++)
                        {
                            thisSumDemand += DEMAND[p, t];
                            sumDemand[p, s, t] = thisSumDemand;
                        }
                    }
                }

                Variable[,] produce = prob.AddVariables(DIM_PRODUCTS, DIM_TIMES)
                    .WithName((p, t) => "produce(" + p + "," + t + ")")
                    .ToArray();
                Variable[,] setup = prob.AddVariables(DIM_PRODUCTS, DIM_TIMES)
                    .WithType(ColumnType.Binary)
                    .WithName((p, t) => "setup(" + p + "," + t + ")")
                    .ToArray();

                // Add the objective function :
                // MinCost:= sum(t in TIMES) (SETUPCOST(t) * sum(p in PRODUCTS) setup(p,t) +
                //           sum(p in PRODUCTS) PRODCOST(p, t) *produce(p, t) )
                prob.SetObjective(Sum(DIM_TIMES, DIM_PRODUCTS,
                                      (t, p) => SETUPCOST[t] * setup[p, t] + PRODCOST[p, t] * produce[p, t]));

                // Add constraints.
                // Production in periods 0 to t must satisfy the total demand
                // during this period of time, for all t in [0,T[
                //    forall(p in PRODUCTS, t in TIMES)
                //      Dem(t) : = sum(s in 1..t) produce(p,s) >= sum(s in 1..t) DEMAND(s)
                prob.AddConstraints(DIM_PRODUCTS, DIM_TIMES,
                                    (p, t) => Sum(t + 1, s => produce[p, s]) >= sumDemand[p, 0, t]);

                // If there is production during t then there is a setup in t :
                //    forall(p in PRODUCTS, t in TIMES)
                //       ProdSetup(t) : = produce(t) <= D(t,TIMES) * setup(t)
                for (int p = 0; p < DIM_PRODUCTS; ++p)
                {
                    for (int t = DIM_TIMES - 1; t >= 0; --t)
                    {
                        prob.AddConstraint(produce[p, t] <= sumDemand[p, t, DIM_TIMES - 1] * setup[p, t]);
                    }
                }

                // Capacity limits :
                //    forall(t in TIMES) Capacity(t) : = sum(p in PRODUCTS) produce(p, t) <= CAP(t)
                prob.AddConstraints(DIM_TIMES,
                                    t => Sum(DIM_PRODUCTS, p => produce[p, t]) <= CAP[t]);

                // Add a CutRound callback for separating our cuts.
                prob.callbacks.AddCutRoundCallback(new Callback(produce, setup, sumDemand).CutRound);

                prob.WriteProb("elsmanagedcuts.lp");

                prob.Optimize();

                if (prob.SolveStatus == SolveStatus.Completed && prob.SolStatus == SolStatus.Optimal)
                {
                    Console.WriteLine("Solved problem to optimality.");
                }
                else
                {
                    Console.WriteLine("Failed to solve problem with solvestatus {0} and solstatus {1}",
                                      prob.SolveStatus,
                                      prob.SolStatus);
                }
            }
        }
    }
}

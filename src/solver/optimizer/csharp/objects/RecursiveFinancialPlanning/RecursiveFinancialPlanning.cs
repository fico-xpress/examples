// (c) 2023-2026 Fair Isaac Corporation


using System;
using System.Linq;
using ColumnType = Optimizer.ColumnType;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.ConstantExpression;

namespace XpressExamples
{
    /// <summary>Recursion solving a non-linear financial planning problem</summary>
    /// <remarks>
    ///  The problem is to solve
    ///  <code>
    ///    net(t) = Payments(t)  - interest(t)
    ///    balance(t) = balance(t-1) - net(t)
    ///    interest(t) = (92/365) * balance(t) * interest_rate
    ///  where
    ///    balance(0) = 0
    ///    balance[T] = 0
    ///  for interest_rate
    ///  </code>
    /// </remarks>
    internal class RecursiveFinancialPlanning
    {
        private static readonly int T = 6;

        /* Data */
        /* An INITIAL GUESS as to interest rate x */
        private static double X = 0.00;
        /* An INITIAL GUESS as to balances b(t) */
        private static readonly double[] B = { 1, 1, 1, 1, 1, 1 };
        private static readonly double[] P = { -1000, 0, 0, 0, 0, 0 };            /* Payments */
        private static readonly double[] R = { 206.6, 206.6, 206.6, 206.6, 206.6, 0 }; /*  "  */
        private static readonly double[] V = { -2.95, 0, 0, 0, 0, 0 };                 /*  "  */

        /* Variables and constraints */
        static Variable[] b; /* Balance */
        static Variable x;               /* Interest rate */
        static Variable dx;              /* Change to x */
        static Inequality[] interest;
        static Inequality ctrd;


        private static void PrintIteration(XpressProblem prob, int it, double variation)
        {
            double[] sol = prob.GetSolution();
            Console.WriteLine("---------------- Iteration {0:d} ----------------", it);
            Console.WriteLine("Objective: {0:f2}", prob.ObjVal);
            Console.WriteLine("Variation: {0:f2}", variation);
            Console.WriteLine("x: {0:f2}", x.GetValue(sol));
            Console.WriteLine("----------------------------------------------");
        }

        private static void PrintProblemSolution(XpressProblem prob)
        {
            double[] sol = prob.GetSolution();
            Console.WriteLine("Objective: {0:f2}", prob.ObjVal);
            Console.WriteLine("Interest rate: {0:f2 percent}", x.GetValue(sol) * 100);
            Console.WriteLine("Variables:");
            Console.Write("\t");
            foreach (Variable v in prob.GetVariables())
            {
                Console.Write("[{0}: {1:f2}] ", v.GetName(), v.GetValue(sol));
            }
            Console.WriteLine();
        }

        /***********************************************************************/
        private static void ModFinNLP(XpressProblem p)
        {
            interest = new Inequality[T];

            // Balance
            b = p.AddVariables(T)
                .WithType(ColumnType.Continuous)
                .WithName(t => $"b_{t}")
                .WithLB(Double.NegativeInfinity)
                .ToArray();

            // Interest rate
            x = p.AddVariable(
                0.0,
                Double.PositiveInfinity,
                ColumnType.Continuous,
                "x"
            );

            // Interest rate change
            dx = p.AddVariable(
                Double.NegativeInfinity,
                Double.PositiveInfinity,
                ColumnType.Continuous,
                "dx"
            );

            Variable[] i = p.AddVariables(T)
                .WithType(ColumnType.Continuous)
                .WithName(t => $"i_{t}")
                .ToArray();

            Variable[] n = p.AddVariables(T)
                .WithType(ColumnType.Continuous)
                .WithName(t => $"n_{t}")
                .WithLB(Double.NegativeInfinity)
                .ToArray();

            Variable[] epl = p.AddVariables(T)
                .WithType(ColumnType.Continuous)
                .WithName(t => $"epl_{t}")
                .ToArray();

            Variable[] emn = p.AddVariables(T)
                .WithType(ColumnType.Continuous)
                .WithName(t => $"emn_{t}")
                .ToArray();

            // Fixed variable values
            i[0].Fix(0);
            b[T - 1].Fix(0);

            // Objective
            p.SetObjective(
                Sum(
                    Sum(epl),
                    Sum(emn)
                ),
                Optimizer.ObjSense.Minimize
            );

            // Constraints
            //   net = payments - interest
            p.AddConstraints(T,
                    t => n[t].Eq(Sum(Constant(P[t] + R[t] + V[t]), -i[t])).SetName($"net_{t}")
                    );

            // Money balance across periods
            p.AddConstraints(T,
                    t => b[t].Eq(t > 0 ? b[t - 1] : Constant(0.0)).SetName($"bal_{t}")
                    );

            // i(t) = (92/365)*( b(t-1)*X + B(t-1)*dx ) approx.
            foreach (int t in Enumerable.Range(1, T - 1))
            {
                LinExpression iepx = new LinTermMap();
                iepx.AddTerm(X * b[t - 1]);
                iepx.AddTerm(B[t - 1] * dx);
                iepx.AddTerm(epl[t]);
                iepx.AddTerm(emn[t]);
                interest[t] = p.AddConstraint(i[t] * (365 / 92.0) == iepx).SetName($"int_{t}");
            }

            // x = dx + X
            ctrd = p.AddConstraint(x == Sum(dx, Constant(X))).SetName("def");
            p.WriteProb("Recur.lp", "l");
        }

        /**************************************************************************/
        /*  Recursion loop (repeat until variation of x converges to 0):          */
        /*    save the current basis and the solutions for variables b[t] and x   */
        /*    set the balance estimates B[t] to the value of b[t]                 */
        /*    set the interest rate estimate X to the value of x                  */
        /*    reload the problem and the saved basis                              */
        /*    solve the LP and calculate the variation of x                       */
        /**************************************************************************/
        private static void solveFinNLP(XpressProblem p)
        {
            double variation = 1.0;

            // Switch automatic cut generation off
            p.CutStrategy = (int)Optimizer.CutStrategy.None;
            // Solve the problem
            p.Optimize();
            if (p.SolStatus != Optimizer.SolStatus.Optimal)
                throw new Exception("failed to optimize with status " + p.SolStatus);

            for (int it = 1; variation > 1e-6; ++it)
            {
                // Optimization solution
                double[] sol = p.GetSolution();

                PrintIteration(p, it, variation);
                PrintProblemSolution(p);
                // Change coefficients in interest[t]
                // Note: when inequalities are added to a problem then all variables are moved to
                // the left-hand side and all constants are moved to the right-hand side. Since we
                // are changing these extracted inequalities directly, we have to use negative
                // coefficients below.
                foreach (int t in Enumerable.Range(1, T - 1))
                {
                    p.ChgCoef(interest[t], dx, -b[t - 1].GetValue(sol));
                    p.ChgCoef(interest[t], b[t - 1], -x.GetValue(sol));
                }

                // Change constant term of ctrd
                ctrd.SetRhs(x.GetValue(sol));

                // Solve the problem
                p.Optimize();
                double[] newsol = p.GetSolution();
                if (p.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("failed to optimize with status " + p.SolStatus);
                variation = Math.Abs(x.GetValue(newsol) - x.GetValue(sol));
            }
            PrintProblemSolution(p);
        }

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);
                prob.MIPLog = 0;

                ModFinNLP(prob);
                solveFinNLP(prob);
            }
        }
    }
}

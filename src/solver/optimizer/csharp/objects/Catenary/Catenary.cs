// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.LinExpression;

namespace XpressExamples
{
    /// <summary>QCQP problem (linear objective, convex quadratic constraints)</summary>
    /// <remarks>
    ///   Based on AMPL model catenary.mod
    ///  (Source: http://www.orfe.princeton.edu/~rvdb/ampl/nlmodels/)
    ///
    ///   This model finds the shape of a hanging chain by minimizing
    ///   its potential energy.
    /// </remarks>
    class Catenary
    {
        static readonly int N = 100;              /// Number of chainlinks
        static readonly int L = 1;                /// Difference in x-coordinates of endlinks

        static readonly double H = 2.0 * L / N;   /// Length of each link

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Variables: x-coordinates
                Variable[] x = prob.AddVariables(N + 1)
                    .WithName("x({0})")
                    .WithLB(XPRS.MINUSINFINITY)
                    .ToArray();

                // Variables: y-coordinates
                Variable[] y = prob.AddVariables(N + 1)
                    .WithName("y({0})")
                    .WithLB(XPRS.MINUSINFINITY)
                    .ToArray();

                // Bounds: positions of endpoints
                // Left anchor
                x[0].Fix(0);
                y[0].Fix(0);
                // Right anchor
                x[N].Fix(L);
                y[N].Fix(0);

                /// Objective: Minimise the potential energy
                /// sum(j in 1..N) (y(j-1)+y(j))/2
                LinExpression obj = LinExpression.Create();
                for (int i = 1; i <= N; i++)
                {
                    obj.AddTerm(0.5 * y[i - 1]).AddTerm(0.5 * y[i]);
                }
                prob.SetObjective(obj, ObjSense.Minimize);

                /// Constraint: positions of chainlinks
                /// forall(j in 1..N) (x(j)-x(j-1))^2+(y(j)-y(j-1))^2 <= H^2
                for (int i = 1; i <= N; i++)
                {
                    prob.AddConstraint(Sum(LinExpression.Create().AddTerm(x[i]).AddTerm(-x[i - 1]).Square(),
                                           LinExpression.Create().AddTerm(y[i]).AddTerm(-y[i - 1]).Square())
                                       .Leq(H * H).SetName($"Link_{i}"));
                }

                /// Solve the problem and write the solution
                prob.Optimize();

                Console.WriteLine($"Solution: {prob.ObjVal}");
                for (int i = 0; i <= N; i++)
                {
                    Console.WriteLine($"{i}: {x[i].GetSolution()}, {y[i].GetSolution()}");
                }
            }
        }
    }
}

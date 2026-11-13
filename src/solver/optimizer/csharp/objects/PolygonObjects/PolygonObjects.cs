// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;


namespace XpressExamples
{
    /// <summary>
    /// Code example showing how to build nonlinear formulae
    /// </summary>
    /// <remarks>
    ///   Maximize the area of polygon of N vertices and diameter of 1
    ///   The position of vertices is indicated as (rho,theta) coordinates
    ///   where rho denotes the distance to the base point
    ///   (vertex with number N - 1) and theta the angle from the x-axis.
    ///
    ///   Demonstrating how to build nonlinear formulae from strings and token sequences
    ///
    /// <code>
    ///     Variables:
    ///
    ///     r     : 0..N-1   ! Distance of vertex from the base point
    ///     theta : 0..N-1   ! Angle from x-axis
    ///
    ///     Objective:
    ///         (sum (i in 1..N-2) (r(i)*r(i-1)*sin(theta(i)-theta(i-1)))) * 0.5
    ///
    ///     Constraints:
    ///         Vertices in increasing degree order:
    ///         theta(i) >= theta(i-1) +.0001  : i = 1..N-2
    ///         Boundary conditions:
    ///         theta(N-1) <= Pi
    ///         0.1 <= r(i) <= 1   : i = 0..N-2
    ///         Third side of all triangles <= 1
    ///         r(i)^2 + r(j)^2 - r(i)*r(j)*2*cos(theta(j)-theta(i)) <= 1    : i in 0..N-3, j in i..N-2
    /// </code>
    /// </remarks>
    public class PolygonObjects
    {
        /// the number of vertices/sides of the polygon
        const int NSIDES = 5;

        public static void Main(string[] args)
        {

            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /****VARIABLES****/
                /// THETA corresponds to the angle relative to the x-axis of vertex i
                Variable[] theta = prob.AddVariables(NSIDES - 1)
                    .WithType(ColumnType.Continuous)
                    .WithUB(Math.PI)
                    .WithName("theta_{0}")
                    .ToArray();

                /// R corresponds to the distance from the base point to vertex i
                Variable[] r = prob.AddVariables(NSIDES - 1)
                    .WithType(ColumnType.Continuous)
                    .WithLB(0.1)
                    .WithUB(1.0)
                    .WithName("r_{0}")
                    .ToArray();

                /// Objective transfer column
                Variable objective = prob.AddVariable(
                    XPRS.MINUSINFINITY,
                    XPRS.PLUSINFINITY,
                    ColumnType.Continuous,
                    "objective");

                Expression[] area = new Expression[NSIDES - 2];
                /// Objective transfer row
                for (int i = 0; i < NSIDES - 2; i++)
                {
                    area[i] = 0.5 * r[i] * r[i + 1] * Sin(theta[i + 1] - theta[i]);
                }
                prob.AddConstraint(Sum(area).Eq(objective));

                /// Set the objective
                prob.SetObjective(objective, ObjSense.Maximize);

                /// The distance between any pair of points is at most 1
                for (int i = 0; i < NSIDES - 1; i++)
                {
                    for (int j = i + 1; j < NSIDES - 1; j++)
                    {
                        prob.AddConstraint(r[i] * r[i] + r[j] * r[j] - 2.0 * r[i] * r[j] * Cos(theta[j] - theta[i]) <= 1.0);
                    }
                }

                /// Ordering the vertices: theta_i+1 >= theta_i
                prob.AddConstraints(NSIDES - 2,
                        (i) => Sum(theta[i + 1], theta[i].Mul(-1)).Geq(0.0));

                /// Optimize
                prob.Optimize("s");
                if (prob.SolStatus != Optimizer.SolStatus.Optimal &&
                        prob.SolStatus != Optimizer.SolStatus.Feasible)
                    throw new Exception("optimization failed with status " + prob.SolStatus);
                double[] sol = prob.GetSolution();
                /* Print out the solution */
                Console.WriteLine("Objective: {0:f4}", prob.ObjVal);
                foreach (Variable v in r)
                {
                    Console.Write("{0}: {1:f4}\n", v.GetName(), v.GetValue(sol));
                }
                Console.WriteLine();
                foreach (Variable v in theta)
                {
                    Console.Write("{0}: {1:f4}\n", v.GetName(), v.GetValue(sol));
                }
                Console.WriteLine();
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using System;

namespace XpressExamples
{

    /// <summary>Small Quadratic Programming example.</summary>
    /// <remarks>
    /// <code>
    ///    minimize x1 + x1^2 +2x1x2 +2x2^2 +x4^2
    ///    s.t.
    ///      C1:  x1 +2x2 -4x4 &gt;= 0
    ///      C2: 3x1 -2x3 - x4 &lt;= 100
    ///      C3: 10 &lt;= x1 +3x2 +3x3 -2x4 &lt;= 30
    ///      0 &lt;= x1 &lt;= 20
    ///      0 &lt;= x2,x3
    ///      x4 free
    /// </code>
    /// </remarks>
    public class QuadraticProgramming
    {
        static readonly int N = 4;

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                ///// VARIABLES
                Variable[] x = new Variable[N];
                x[0] = prob.AddVariable(0, 20, ColumnType.Continuous, "x1");
                x[1] = prob.AddVariable("x2");
                x[2] = prob.AddVariable("x3");
                x[3] = prob.AddVariable(Double.NegativeInfinity, Double.PositiveInfinity, ColumnType.Continuous, "x4");

                ///// OBJECTIVE
                QuadExpression obj = QuadExpression.Create();
                obj.AddTerm(x[0]);
                obj.AddTerm(x[0], x[0]);
                obj.AddTerm(2 * x[0] * x[1]);
                obj.AddTerm(2 * x[1] * x[1]);
                obj.AddTerm(x[3].Square());
                prob.SetObjective(obj, ObjSense.Minimize);

                ///// CONSTRAINTS
                prob.AddConstraint((x[0] + 2 * x[1] - 4 * x[3] >= 0).SetName("C1"));
                prob.AddConstraint((3 * x[0] - 2 * x[2] - x[3] <= 100).SetName("C2"));
                prob.AddConstraint((x[0] + 3 * x[1] + 3 * x[2] - 2 * x[3]).In(10, 30).SetName("C3"));

                ///// SOLVING + OUTPUT
                prob.WriteProb("qp.lp");
                prob.Optimize();

                Console.WriteLine("Problem status: " + prob.LPStatus);
                if (prob.LPStatus != LPStatus.Optimal)
                    throw new Exception("optimization failed with status " + prob.LPStatus);

                Console.WriteLine("Objective function value: " + prob.ObjVal);
                double[] sol = prob.GetSolution();
                for (int i = 0; i < N; i++)
                    Console.Write(x[i].GetName() + ": " + x[i].GetValue(sol) + ", ");
                Console.WriteLine();
            }
        }
    }
}

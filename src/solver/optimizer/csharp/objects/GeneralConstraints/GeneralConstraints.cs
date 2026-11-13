// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections.Generic;
using Optimizer.Maps;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.LinExpression;
using static Optimizer.XPRSprob;
using static Optimizer.Objects.ConstantExpression;
using Optimizer;

namespace XpressExamples
{
    /// <summary>
    /// A simple example that formulates some constraints on the minimum and absolute values
    /// of linear combinations of variables.
    /// </summary>
    public class GeneralConstraints
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("Formulating the general constraint example problem");

            const int R = 3;

            using (XpressProblem prob = new XpressProblem())
            {
                Variable[] x = prob.AddVariables(R)
                  .WithName("x_{0}")
                  .WithUB(20)
                  .ToArray();

                Variable y = prob.AddVariable("y");
                Variable z = prob.AddVariable("z");

                Expression objective = Sum(x);

                // We want to formulate abs(x(0)-2*x(1)) <= 10. We need to introduce
                // two auxiliary variables for the argument of the abs function
                //  and then break down the abs expression in multiple steps
                Variable diff1 = prob.AddVariable("diff1");
                Variable absOfDiff1 = prob.AddVariable("absOfDiff1");

                prob.AddConstraint(diff1 == x[0] - 2 * x[1]);
                prob.AddConstraint(absOfDiff1.AbsOf(diff1));
                prob.AddConstraint(absOfDiff1 <= 10);

                // We link a new variable to the minimum of the x(i) and
                // require this variable to be >= 5
                // Clearly, this bound constraint could also be achieved by simply setting
                // the bounds of each x variable.
                Variable minOfX = prob.AddVariable("minOfX");
                prob.AddConstraint(minOfX.MinOf(x));
                prob.AddConstraint(minOfX >= 5);

                // We link variable y to the maximum of other variables, expressions, and constants
                // y = max(x(2), 20, x(0)-z)
                Variable diff2 = prob.AddVariable("diff2");
                prob.AddConstraint(diff2 == x[0] - 1 * z);

                // the below code is equivalent to using the MaxOf function on the resultant y
                // prob.AddConstraint(y.MaxOf(new Variable[] {x[2], diff2}, 20).SetName("max_constraint"));
                prob.AddConstraint(y.MaxOf(new Variable[] { x[2], diff2 }, new double[] { 20 }, "max_constraint"));

                // set objective function with a maximization sense
                prob.SetObjective(objective, Optimizer.ObjSense.Maximize);

                // write the problem in LP format for manual inspection
                Console.WriteLine("Writing the problem to 'GeneralConstraints.lp'");
                prob.WriteProb("GeneralConstraints.lp", "l");

                // Solve the problem
                Console.WriteLine("Solving the problem");
                prob.Optimize();

                // check the solution status
                Console.WriteLine("Problem finished with SolStatus {0}", prob.SolStatus);
                if (prob.SolStatus != Optimizer.SolStatus.Optimal)
                {
                    throw new Exception("Problem not solved to optimality");
                }

                // print the optimal solution of the problem to the console
                Console.WriteLine("Solution has objective value (profit) of {0}", prob.ObjVal);
                Console.WriteLine("");
                Console.WriteLine("*** Solution ***");
                double[] sol = prob.GetSolution();

                for (int r = 0; r < R; r++)
                {
                    string delim = r < R - 1 ? ", " : "\n";
                    Console.Write($"x_{r} = {x[r].GetValue(sol)}{delim}");
                }
                Console.WriteLine($"y = {y.GetValue(sol)}, z = {z.GetValue(sol)}");
                Console.WriteLine($"ABS ( x[0] - 2*x[1] ) = {absOfDiff1.GetValue(sol)}, minOfX = {minOfX.GetValue(sol)}");

                Console.WriteLine("");
            }
        }
    }
}

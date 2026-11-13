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
    /// An example that demonstrates how to model a piecewise linear cost function.
    /// A piecewise linear cost function f(x) assigns a different linear cost function
    /// for different intervals of the domain of its argument x.
    /// This situation occurs in real-world problems if
    /// there are price discounts available starting from a certain quantity of sold goods.
    ///
    /// - Example discussed in mipformref whitepaper -
    /// </summary>
    public class PiecewiseLinear
    {
        public static void Main(string[] args)
        {

            const int NB = 6;

            // define breakpoints as (x,y)-coordinates. Note that
            // some breakpoints share the same x-coordinate, but have
            // different values for y.
            // The optimizer handles this case correctly.
            // In practice, such noncontinuous piecewise linear functions
            // can model price discounts for example.
            PwlBreakpoint[] breakpoints = new PwlBreakpoint[NB] {
              new PwlBreakpoint(  0,   0),
              new PwlBreakpoint( 50,  50),
              new PwlBreakpoint( 50,  75),
              new PwlBreakpoint(120, 180),
              new PwlBreakpoint(120, 240),
              new PwlBreakpoint(200, 400)
            };

            Console.WriteLine("Formulating the piecewise linear example problem");

            using (XpressProblem prob = new XpressProblem())
            {
                Variable x = prob.AddVariable("x");
                Variable fx = prob.AddVariable("fx");
                Expression objective = fx;

                prob.AddConstraint(fx.PwlOf(x, breakpoints, "pwl_with_breakpoints"));

                // ! Add a lower bound on x to get a somewhat more meaningful model
                // x >= 150
                x.SetLB(150);


                // set objective function with a minimization sense
                prob.SetObjective(objective, Optimizer.ObjSense.Minimize);

                // write the problem in LP format for manual inspection
                Console.WriteLine("Writing the problem to 'PiecewiseLinear.lp'");
                prob.WriteProb("PiecewiseLinear.lp", "l");

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

                Console.WriteLine($"x = {x.GetValue(sol)}, fx = {fx.GetValue(sol)}");

                Console.WriteLine("");
            }
        }
    }
}

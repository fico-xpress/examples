// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections.Generic;
using Optimizer.Maps;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.XPRSprob;
using static Optimizer.Objects.SOS;
using Optimizer;

namespace XpressExamples
{
    /// <summary>
    /// Approximation of a nonlinear function by a special ordered set (SOS-2).
    /// </summary>
    /// <remarks>
    /// An SOS-2 is a constraint that allows at most 2 of its variables to
    /// have a nonzero value. In addition, these variables have to be adjacent.
    ///
    /// Example discussed in mipformref whitepaper
    /// </remarks>
    public class SpecialOrderedSets
    {
        public static void Main(string[] args)
        {

            const int NB = 4; // number of breakpoints
            double[] B_X = // X coordinates of breakpoints
                new double[NB] { 1, 2.5, 4.5, 6.5 };

            double[] B_Y = // Y coordinates of breakpoints
                new double[NB] { 1.5, 6, 3.5, 2.5 };


            Console.WriteLine("Formulating the special ordered sets example problem");
            using (XpressProblem prob = new XpressProblem())
            {
                // create one w variable for each breakpoint. We express
                Variable[] w = prob.AddVariables(NB)
                  .WithName("w_{0}")
                  .WithUB(1)
                  .ToArray();

                Variable x = prob.AddVariable("x");
                Variable y = prob.AddVariable("y");

                // Define the SOS-2 with weights from B_X. This is necessary to
                // establish the ordering between the w variables.
                prob.AddConstraint(SOS.Sos2(w, B_X, "SOS_2"));

                // We use the w variables to express a convex combination.
                // In combination with the above SOS-2 condition,
                // X and Y are represented as a convex combination of 2 adjacent
                // breakpoints.
                prob.AddConstraint(Sum(w) == 1);

                // The below constraints express the actual locations of X and Y
                // in the plane as a convex combination of the breakpoints, subject
                // to the assignment found for the w variables.
                prob.AddConstraint(x == ScalarProduct(w, B_X));
                prob.AddConstraint(y == ScalarProduct(w, B_Y));

                // set lower and upper bounds on x
                x.SetLB(2); x.SetUB(6);

                // set objective function with a maximization sense
                prob.SetObjective(y, Optimizer.ObjSense.Minimize);

                // write the problem in LP format for manual inspection
                Console.WriteLine("Writing the problem to 'SpecialOrderedSets.lp'");
                prob.WriteProb("SpecialOrderedSets.lp", "l");

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
                Console.WriteLine("Solution has objective value (profit) of {0}\n", prob.ObjVal);
                Console.WriteLine("*** Solution ***");
                double[] sol = prob.GetSolution();

                for (int b = 0; b < NB; b++)
                {
                    string delim = b < NB - 1 ? ", " : "\n";
                    Console.Write($"w_{b} = {w[b].GetValue(sol)}{delim}");
                }

                Console.WriteLine($"x = {x.GetValue(sol)}, y = {y.GetValue(sol)}");
            }
        }
    }
}

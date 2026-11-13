// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections.Generic;
using Optimizer.Maps;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.XPRSprob;
using static Optimizer.Objects.ConstantExpression;
using static Optimizer.Objects.SOS;
using Optimizer;

namespace XpressExamples
{
    /// <summary>
    /// Approximation of a quadratic function in 2 variables by special ordered sets (SOS-2).
    /// </summary>
    /// <remarks>
    /// An SOS-2 is a constraint that allows at most 2 of its variables to
    /// have a nonzero value. In addition, these variables have to be adjacent.
    ///
    /// Example discussed in mipformref whitepaper.
    /// </remarks>
    public class SpecialOrderedSetsQuadratic
    {
        public static void Main(string[] args)
        {

            const int NX = 10; // number of breakpoints on the X-axis
            const int NY = 10; // number of breakpoints on the Y-axis
            double[] X = // X coordinates of grid points
                new double[NX];

            double[] Y = // Y coordinates of breakpoints
                new double[NY];

            double[,] F_XY = // two dimensional array of function values on the grid points
                new double[NX, NY];

            // assign the toy data
            for (int i = 0; i < NX; i++) X[i] = i + 1;
            for (int i = 0; i < NY; i++) Y[i] = i + 1;
            for (int i = 0; i < NX; i++) for (int j = 0; j < NY; j++) F_XY[i, j] = (X[i] - 5) * (Y[j] - 5);

            Console.WriteLine("Formulating the special ordered sets quadratic example problem");
            using (XpressProblem prob = new XpressProblem())
            {
                // create one w variable for each X breakpoint. We express
                Variable[] wx = prob.AddVariables(NX)
                  .WithName("wx_{0}")
                  .WithUB(1)  // this upper bound i redundant because of the convex combination constraint on the sum of the wx
                  .ToArray();
                // create one w variable for each Y breakpoint. We express
                Variable[] wy = prob.AddVariables(NY)
                  .WithName("wy_{0}")
                  .WithUB(1)  // this upper bound i redundant because of the convex combination constraint on the sum of the wy
                  .ToArray();

                // create a two-dimensional array of w variable for each grid point. We express
                Variable[,] wxy = prob.AddVariables(NX, NY)
                  .WithName("wxy_{0}_{1}")
                  .WithUB(1)  // this upper bound i redundant because of the convex combination constraint on the sum of the wy
                  .ToArray();

                Variable x = prob.AddVariable("x");
                Variable y = prob.AddVariable("y");
                Variable fxy = prob.AddVariable("fxy");

                // make fxy a free variable
                fxy.SetLB(double.NegativeInfinity);

                // Define the SOS-2 constraints with weights from X and Y. This
                // is necessary to establish the ordering between variables in wx
                // and in wy.
                prob.AddConstraint(SOS.Sos2(wx, X, "SOS_2_X"));
                prob.AddConstraint(SOS.Sos2(wy, Y, "SOS_2_Y"));

                prob.AddConstraint(Sum(wx) == 1);
                prob.AddConstraint(Sum(wy) == 1);

                // link the wxy variables to their 1-dimensional colleagues
                prob.AddConstraints(NX,
                  i => (wx[i] == Sum(NY, j => wxy[i, j]))
                );
                prob.AddConstraints(NY,
                  j => (wy[j] == Sum(NX, i => wxy[i, j]))
                );


                // now express the actual x, y, and f(x,y) coordinates
                prob.AddConstraint(x == ScalarProduct(wx, X));
                prob.AddConstraint(y == ScalarProduct(wy, Y));
                prob.AddConstraint(fxy == Sum(NX, i => Sum(NY, j => wxy[i, j] * F_XY[i, j])));

                // set lower and upper bounds on x and y
                x.SetLB(2); x.SetUB(10);
                y.SetLB(2); y.SetUB(10);

                // set objective function with a minimization sense
                prob.SetObjective(fxy, Optimizer.ObjSense.Minimize);

                // write the problem in LP format for manual inspection
                Console.WriteLine("Writing the problem to 'SpecialOrderedSetsQuadratic.lp'");
                prob.WriteProb("SpecialOrderedSetsQuadratic.lp", "l");

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

                for (int i = 0; i < NX; i++)
                {
                    string delim = i < NX - 1 ? ", " : "\n";
                    Console.Write($"wx_{i} = {wx[i].GetValue(sol)}{delim}");
                }
                for (int j = 0; j < NY; j++)
                {
                    string delim = j < NY - 1 ? ", " : "\n";
                    Console.Write($"wy_{j} = {wy[j].GetValue(sol)}{delim}");
                }

                Console.WriteLine($"x = {x.GetValue(sol)}, y = {y.GetValue(sol)}");
            }
        }
    }
}

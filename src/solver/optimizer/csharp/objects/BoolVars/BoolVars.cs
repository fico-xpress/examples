// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections.Generic;
using Optimizer.Maps;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.LinExpression;
using static Optimizer.Objects.ConstantExpression;
using Optimizer;

namespace XpressExamples
{
    /// <summary>
    /// Stating logic clauses that resemble SAT-type formulations
    /// </summary>
    public class BoolVars
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("Formulating the bool vars example problem");

            const int R = 5;

            using (XpressProblem prob = new XpressProblem())
            {
                // create boolean variables and their negations
                Variable[] x = prob.AddVariables(R)
                  .WithType(ColumnType.Binary)
                  .WithName("x_{0}")
                  .WithUB(1)
                  .ToArray();
                Variable[] x_neg = prob.AddVariables(R)
                  .WithType(ColumnType.Binary)
                  .WithName("x_neg_{0}")
                  .WithUB(1)
                  .ToArray();


                // add 2 helper variables for stating logical constraints
                Variable trueVar = prob.AddVariable(0, 1, ColumnType.Binary, "TRUE");
                Variable falseVar = prob.AddVariable(0, 1, ColumnType.Binary, "FALSE");

                // fix these helper variables to TRUE and FALSE, respectively.
                prob.AddConstraint(trueVar == 1);
                prob.AddConstraint(falseVar == 0);

                // add the complement relation between each binary variable and its negation.
                prob.AddConstraints(R,
                    r => (1 * x[r] + 1 * x_neg[r] == 1));

                // add a direct equation constraint between certain variables.
                prob.AddConstraint(x[2] == x_neg[3]);

                // express some clauses on the variables
                // require that  x(0) and x_neg(4) = true
                // we use trueVar as the resultant of a logical "and" constraint
                prob.AddConstraint(trueVar.AndOf(new Variable[] { x[0], x_neg[4] }));

                // ! 'x(0) or x(2)' must be true
                prob.AddConstraint(trueVar.OrOf(new Variable[] { x[0], x[2] }));

                //  for more complicated expressions, we need auxiliary variables.
                Variable andResultant1 = prob.AddVariable(0, 1, ColumnType.Binary, "andresultant1");
                Variable andResultant2 = prob.AddVariable(0, 1, ColumnType.Binary, "andresultant2");

                // both constraints below could be formulated using andResultant[12].AndOf(...)
                // here, however, we highlight the connection to general constraints
                // the first AND is between x[0] .. x[2]
                prob.AddConstraint(andResultant1.AndOf(x.Take(3).ToArray()));
                //  the second AND is between x_neg[3] and x_neg[4]
                prob.AddConstraint(andResultant2.AndOf(x_neg.Skip(3).ToArray()));

                // add those two individual AND resultants by requiring at least one to be satisfied
                prob.AddConstraint(trueVar.OrOf(new Variable[] { andResultant1, andResultant2 }));

                // finally, add a constraint that none of x_neg[0 .. 2] should be true
                prob.AddConstraint(falseVar.OrOf(x_neg.Take(3).ToArray()));

                // set constant objective function with a maximization sense
                prob.SetObjective(Constant(0), Optimizer.ObjSense.Maximize);

                // write the problem in LP format for manual inspection
                Console.WriteLine("Writing the problem to 'BoolVars.lp'");
                prob.WriteProb("BoolVars.lp", "l");

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

                for (int r = 0; r < R; r++)
                {
                    string delim = r < R - 1 ? ", " : "\n";
                    Console.Write($"x_neg_{r} = {x_neg[r].GetValue(sol)}{delim}");
                }

                Console.WriteLine("");
            }
        }
    }
}

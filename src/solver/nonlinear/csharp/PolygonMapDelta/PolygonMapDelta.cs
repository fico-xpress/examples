using System;
using Optimizer;
using System.Collections.Generic;

namespace Examples
{
    /// <summary>
    /// Code example that uses a user function of type "mapdelta".
    /// </summary>
    /// <remarks>
    /// <code>
    ///     Xpress Optimizer Examples
    ///     =========================
    ///
    ///     Maximize the area of polygon of N vertices and diameter of 1
    ///     The position of vertices is indicated as (rho,theta) coordinates
    ///     where rho denotes the distance to the base point
    ///     (vertex with number N) and theta the angle from the x-axis.
    ///
    ///     (c) 2021-2026 Fair Isaac Corporation
    /// </code>
    ///
    ///     Polygon example: maximise the area of an N sided polygon
    ///
    ///     *** Demonstrating using a simple map (R->R) userfunction ***
    ///
    /// <code>
    ///     Variables:
    ///
    ///     rho   : 0..N-1   ! Distance of vertex from the base point
    ///     theta : 0..N-1   ! Angle from x-axis
    ///
    ///     Objective:
    ///         (sum (i in 1..N-2) (rho(i)*rho(i-1)*sin(theta(i)-theta(i-1)))) * 0.5
    ///
    ///     Constraints:
    ///         Vertices in increasing degree order:
    ///         theta(i) >= theta(i-1) +.0001  : i = 1..N-2
    ///         Boundary conditions:
    ///         theta(N-1) <= Pi
    ///         0.1 <= rho(i) <= 1   : i = 0..N-2
    ///         Third side of all triangles <= 1
    ///         rho(i)^2 + rho(j)^2 - rho(i)*rho(j)*2*cos(theta(j)-theta(i)) <= 1    : i in 0..N-3, j in i..N-2
    /// </code>
    /// </remarks>
    public class PolygonMapDelta
    {
        /// <summary>
        /// User function that maps a double to a double.
        /// </summary>
        /// <remarks>
        /// This just forwards to <code>sin()</code>.
        /// It also fills in the value for the derivative if that is requested.
        /// </remarks>
        /// <param name="value">The point at which to evaluate.</param>
        /// <param name="delta">Non-zero if a derivative is requested.</param>
        /// <param name="partial">Where to store the derivative.</param>
        /// <returns>The value of the function evaluated at <c>value</c>.</returns>
        private static double MySin(double value, double delta, double[] partial)
        {
            if (delta != 0.0)
                partial[0] = Math.Cos(value);
            return Math.Sin(value);
        }
        /// <summary>
        /// User function that maps a double to a double.
        /// </summary>
        /// <remarks>
        /// This just forwards to <code>cos()</code>.
        /// It also fills in the value for the derivative if that is requested.
        /// </remarks>
        /// <param name="value">The point at which to evaluate.</param>
        /// <param name="delta">Non-zero if a derivative is requested.</param>
        /// <param name="partial">Where to store the derivative.</param>
        /// <returns>The value of the function evaluated at <c>value</c>.</returns>
        private static double MyCos(double value, double delta, double[] partial)
        {
            if (delta != 0.0)
                partial[0] = -Math.Sin(value);
            return Math.Cos(value);
        }

        public static void Main(String[] args)
        {
            using (XPRSprob prob = new XPRSprob(null))
            {
                // Number of sides of the Polygon
                int nSide = 5;

                // Theta
                int[] theta = prob.AddColumns(nSide - 1)
                    .WithUB(Math.PI)
                    .WithName(i => "THETA" + (i + 1))
                    .ToArray();
                // Rho
                int[] rho = prob.AddColumns(nSide - 1)
                    .WithLB(0.01)
                    .WithUB(1.0)
                    .WithName(i => "RHO" + (i + 1))
                    .ToArray();

                // Add the user functions
                XPRSprob.MapDeltaFunction sin = prob.NlpAddUserFunction("mySin", 0, MySin);
                XPRSprob.MapDeltaFunction cos = prob.NlpAddUserFunction("myCos", 0, MyCos);

                // Objective function. We build the objective function as
                // a formula in infix notation. See below for submitting a
                // formula as string.
                List<int> tok = new List<int>();
                List<double> val = new List<double>();
                for (int i = 1; i < nSide - 1; ++i)
                {
                    if (tok.Count > 0)
                    {
                        tok.Add(Constants.TOK_OP);
                        val.Add(Constants.OP_PLUS);
                    }
                    tok.Add(Constants.TOK_COL);             // RHO(i)
                    val.Add(rho[i]);
                    tok.Add(Constants.TOK_OP);              // *
                    val.Add(Constants.OP_MULTIPLY);
                    tok.Add(Constants.TOK_COL);             // RHO(i-1)
                    val.Add(rho[i - 1]);
                    tok.Add(Constants.TOK_OP);              // *
                    val.Add(Constants.OP_MULTIPLY);
                    tok.Add(Constants.TOK_FUN);             // mySin
                    val.Add(sin.GetId());
                    tok.Add(Constants.TOK_LB);              // (
                    val.Add(Constants.TOK_LB);
                    tok.Add(Constants.TOK_COL);             // THETA(i)
                    val.Add(theta[i]);
                    tok.Add(Constants.TOK_OP);              // -
                    val.Add(Constants.OP_MINUS);
                    tok.Add(Constants.TOK_COL);             // THETA(i-1)
                    val.Add(theta[i - 1]);
                    tok.Add(Constants.TOK_RB);              // )
                    val.Add(Constants.TOK_RB);
                    tok.Add(Constants.TOK_OP);              // *
                    val.Add(Constants.OP_MULTIPLY);
                    tok.Add(Constants.TOK_CON);             // 0.5
                    val.Add(0.5);
                }
                tok.Add(Constants.TOK_EOF);
                val.Add(0.0);
                // Since nonlinear objectives cannot be directly expressed in Xpress, we maximize a free
                // variable objx and constrain this variable to be equal to the nonlinear objective.
                int objx = prob.AddColumn(XPRS.MINUSINFINITY, XPRS.PLUSINFINITY, ColumnType.Continuous, null);
                prob.ChgObj(objx, 1.0);
                int objeq = prob.AddRow(new int[]{objx}, new double[]{-1.0}, 'E', 0.0);
                prob.NlpChgFormula(objeq, 0,
                                    tok.ToArray(),
                                    val.ToArray());
                prob.ChgObjSense(ObjSense.Maximize);

                // Vertices in increasing degree order
                for (int i = 0; i < nSide - 2; ++i)
                    prob.AddRow(new int[] { theta[i], theta[i + 1] },
                                new double[] { -1.0, 1.0 },
                                'G',
                                0.001);

                // Third side of all triangles <= 1
                for (int i = 1; i < nSide - 1; i++)
                {
                    for (int j = i + 1; j < nSide; j++)
                    {
                        int row = prob.AddRow(new int[0], new double[0], 'L', 1.0);
                        prob.NlpChgFormulaStr(row,
                            String.Format("RHO{0:d} ^ 2 + RHO{1:d} ^ 2 - RHO{2:d} * RHO{3:d} * 2 * myCos ( THETA{4:d} - THETA{5:d} )",
                            i, j, i, j, j, i));
                    }
                }

                //prob.writeProb("map.lp", "l");
                int solvestatus = -1;
                int solstatus = -1;
				/* Solve the problem to local optimality */
		        prob.NLPSolver = Optimizer.Constants.NLPSOLVER_LOCAL;
                prob.Optimize("s", out solvestatus, out solstatus);
                System.Console.WriteLine("solvestatus: " + solvestatus);
                System.Console.WriteLine("solstatus:   " + solstatus);
                System.Console.WriteLine("Solution value: " + prob.ObjVal);

                double[] x = prob.GetSolution();
                for (int i = 0; i < rho.Length; ++i)
                    System.Console.WriteLine("RHO" + (i + 1) + " = " + x[rho[i]]);
                for (int i = 0; i < theta.Length; ++i)
                    System.Console.WriteLine("THETA" + (i + 1) + " = " + x[theta[i]]);
            }
        }
    }
}

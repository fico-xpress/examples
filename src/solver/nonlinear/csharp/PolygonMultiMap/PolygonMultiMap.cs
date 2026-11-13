using System;
using Optimizer;
using System.Collections.Generic;

namespace Examples {
    /// <summary>
    /// Code example that uses a user function of type "multimap".
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
    ///     *** Demonstrating using a multimap (R^2->R^2) userfunction ***
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
    public class PolygonMultiMap {

        /// <summary>
        /// User function that maps two doubles to two doubles.
        /// </summary>
        /// <remarks>
        /// Computes <c>sin(x[0] - x[1]), cos(x[0] - x[1])</c>.
        /// </remarks>
        /// <param name="value">The point at which to evaluate.</param>
        /// <returns>The computed values of the function.</returns>
        private static double[] MyTrigonometrics(double[] value)
        {
            return new double[] {
                Math.Sin(value[0] - value[1]),
                Math.Cos(value[0] - value[1])
            };
        }

        public static void Main(String[] args) {
            using (XPRSprob prob = new XPRSprob(null)) {
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

                // Add the user function
                XPRSprob.MultiMapFunction trig = prob.NlpAddUserFunction("myTrigonometrics", 2, 2, 0, MyTrigonometrics);

                // Objective function. We build the objective function as
                // a formula in infix notation. See below for submitting a
                // formula as string.
                List<int> tok = new List<int>();
                List<double> val = new List<double>();
                for (int i = 1; i < nSide - 1; ++i) {
                    if ( tok.Count > 0 ) {
                        tok.Add(Constants.TOK_OP);
                        val.Add(Constants.OP_PLUS);
                    }
                    tok.Add(Constants.TOK_COL);             // RHO(i)
                    val.Add(rho[i]);
                    tok.Add(Constants.TOK_OP);              // *
                    val.Add(Constants.OP_MULTIPLY);
                    tok.Add(Constants.TOK_COL);             // RHO(i-1)
                    val.Add(rho[i-1]);
                    tok.Add(Constants.TOK_OP);              // *
                    val.Add(Constants.OP_MULTIPLY);
                    tok.Add(Constants.TOK_FUN);             // myTrigonometrics
                    val.Add(trig.GetId());
                    tok.Add(Constants.TOK_LB);              // (
                    val.Add(Constants.TOK_LB);
                    tok.Add(Constants.TOK_COL);             // THETA(i)
                    val.Add(theta[i]);
                    tok.Add(Constants.TOK_DEL);             // -
                    val.Add(Constants.DEL_COMMA);
                    tok.Add(Constants.TOK_COL);             // THETA(i-1)
                    val.Add(theta[i-1]);
                    tok.Add(Constants.TOK_DEL);             // :
                    val.Add(Constants.DEL_COLON);
                    tok.Add(Constants.TOK_CON);             // 1
                    val.Add(1);
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
                    prob.AddRow(new int[]{ theta[i], theta[i+1] },
                                new double[]{ -1.0, 1.0 },
                                'G',
                                0.001);

                // Third side of all triangles <= 1
                for (int i = 1; i < nSide - 1; i++) {
                    for (int j = i + 1; j < nSide; j++) {
                        int row = prob.AddRow(new int[0], new double[0], 'L', 1.0);
                        prob.NlpChgFormulaStr(row,
                            String.Format("RHO{0:d} ^ 2 + RHO{1:d} ^ 2 - RHO{2:d} * RHO{3:d} * 2 * myTrigonometrics ( THETA{4:d} , THETA{5:d} : 2 )",
                            i, j, i, j, j, i));
                    }
                }

                //prob.writeProb("map.lp", "l");
                int solvestatus = -1;
                int solstatus = -1;
                prob.Optimize("s", out solvestatus, out solstatus);
                System.Console.WriteLine("solvestatus: " + solvestatus);
                System.Console.WriteLine("solstatus:   " + solstatus);
                System.Console.WriteLine("Solution value: " + prob.ObjVal);

                double[] x = prob.GetSolution();
                for (int i = 0; i < rho.Length; ++i)
                    System.Console.WriteLine("RHO" + (i+1) + " = " + x[rho[i]]);
                for (int i = 0; i < theta.Length; ++i)
                    System.Console.WriteLine("THETA" + (i + 1) + " = " + x[theta[i]]);
            }
        }
    }
}

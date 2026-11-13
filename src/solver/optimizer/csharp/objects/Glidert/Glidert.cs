// (c) 2023-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XPRSexamples
{
    /// <summary>Maximize the total horizontal distance a hang glider flies.</summary>
    /// <remarks>
    /// Configure the wind conditions by setting the parameter wind:
    /// wind = 0 : no wind
    /// wind = 1 : thermal uplift 250m ahead (positive in a neighborhood of 250m
    ///            but drops to zero exponentially away from this point)
    /// wind = 2 : wind shear (a rapidly diminishing head wind that may cause
    ///            the glider to stall)
    ///
    /// Trapezoidal discretization.
    ///
    /// Based on AMPL models hang_midpt.mod, shear_midpt.mod, stableair_midpt.mod
    /// Source: <see href="https://vanderbei.princeton.edu/ampl/nlmodels/hang/"/>
    /// Reference: R. Bulirsch, E. Nerz, H.J. Pesch, and O. von Stryk, "Combining Direct and
    /// Indirect Methods in Optimal Control: Range Maximization of a Hang Glider", in
    /// "Optimal Control: Calculus of Variations, Optimal Control Theory and
    /// Numerical Methods", ed. by R. Bulirsch, A. Miele, J. Stoer, and K.H. Well
    /// (1993) Birkhauser
    ///
    /// *** This model cannot be run with a Community Licence for the provided data
    /// instance ***
    /// </remarks>
    internal class Glidert
    {
        /// <summary>Wind conditions; possible values: 0, 1, 2 (see description at the beginning of the file).</summary>
        static readonly int wind = 1;
        /// <summary>Number of discretization points.</summary>
        const int n = 150;

        /// <summary>Initial horizontal position.</summary>
        const double x0 = 0.0;
        /// <summary>Initial vertical position.</summary>
        const double y0 = 1000.0;
        /// <summary>Initial horizontal velocity.</summary>
        const double vx0 = 13.2275675;
        /// <summary>Initial vertical velocity.</summary>
        const double vy0 = -1.28750052;
        /// <summary>Final vertical position.</summary>
        const double yN = 900.0;
        /// <summary>Final horizontal velocity.</summary>
        const double vxN = 13.2275675;
        /// <summary>Final vertical velocity.</summary>
        const double vyN = -1.28750052;
        /// <summary>Upward wind-velocity coefficient (m/sec).</summary>
        const double um = 2.5;
        /// <summary>Upward wind-velocity ratio (m).</summary>
        const double r = 100.0;
        /// <summary>First drag coefficient constant.</summary>
        const double c0 = 0.034;
        /// <summary>Second drag coefficient constant.</summary>
        const double k = 0.069662;
        /// <summary>Mass of glider &amp; pilot.</summary>
        const double mass = 100.0;
        /// <summary>Wing area.</summary>
        const double span = 14.0;
        /// <summary>Air density.</summary>
        const double rho = 1.13;
        /// <summary>Acceleration due to gravity.</summary>
        const double grav = 9.80665;
        /// <summary>Weight of glider &amp; pilot (= m*g).</summary>
        const double weight = mass * grav;
        /// <summary>Minimum lift coefficient.</summary>
        const double clMin = 0.0;
        /// <summary>Maximum lift coefficient.</summary>
        const double clMax = 1.4;
        /// <summary>Minimum velocity.</summary>
        const double velMin = -100.0;
        /// <summary>Maximum velocity.</summary>
        const double velMax = 100.0;
        /// <summary>Minimum acceleration.</summary>
        const double accMin = -3.0;
        /// <summary>Maximum acceleration.</summary>
        const double accMax = 3.0;
        /// <summary>Minimum duration.</summary>
        const double durMin = 10.0;
        /// <summary>Maximum duration.</summary>
        const double durMax = 200.0;

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Output all messages.
                prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);

                /**** VARIABLES ****/

                // Total duration of the flight
                Variable dur = prob.AddVariable(durMin, durMax, ColumnType.Continuous, "dur");

                // Horizontal position at time step t
                Variable[] x = prob.AddVariables(n + 1).WithName(t => $"x_{t}").ToArray();

                // Vertical position at time step t
                Variable[] y = prob.AddVariables(n + 1).WithName(t => $"y_{t}").ToArray();

                // Horizontal velocity at time step t
                Variable[] vx = prob.AddVariables(n + 1).WithName(t => $"vx_{t}")
                        .WithLB(velMin).WithUB(velMax).ToArray();

                // Vertical velocity at time step t
                Variable[] vy = prob.AddVariables(n + 1).WithName(t => $"vy_{t}")
                        .WithLB(velMin).WithUB(velMax).ToArray();

                // Derivative of vx at time step t
                Variable[] vxDot = prob.AddVariables(n + 1).WithName(t => $"vxDot_{t}")
                        .WithLB(accMin).WithUB(accMax).ToArray();

                // Derivative of vy at time step t
                Variable[] vyDot = prob.AddVariables(n + 1).WithName(t => $"vyDot_{t}")
                        .WithLB(accMin).WithUB(accMax).ToArray();

                // Lift coefficient (control variable) at time step t
                Variable[] cLift = prob.AddVariables(n + 1).WithName(t => $"cLift_{t}")
                        .WithLB(clMin).WithUB(clMax).ToArray();

                // Objective is to maximize the horizontal position in the final time step
                prob.SetObjective(x[n], ObjSense.Maximize);

                // Initial and final states
                prob.AddConstraint(x[0] == x0);
                prob.AddConstraint(y[0] == y0);
                if (wind == 2)
                    prob.AddConstraint(vx[0] == vx0 - 2.5 * (1 + Math.Atan((y0 - 30) / 10)));
                else
                    prob.AddConstraint(vx[0] == vx0);
                prob.AddConstraint(vy[0] == vy0);
                prob.AddConstraint(y[n] == yN);
                prob.AddConstraint(vx[n] == vxN);
                prob.AddConstraint(vy[n] == vyN);

                // Monotonicity
                prob.AddConstraints(n, i => x[i + 1] >= x[i]);

                // Trapezoidal discretization
                // x[i+1] = x[i] + 0.5*(dur/n)*(vx[i+1] + vx[i])
                prob.AddConstraints(n, i => x[i + 1] == x[i] + 0.5 * dur / n * (vx[i + 1] + vx[i]));
                // y[i+1] = y[i] + 0.5*(dur/n)*(vy[i+1] + vy[i])
                prob.AddConstraints(n, i => y[i + 1] == y[i] + 0.5 * dur / n * (vy[i + 1] + vy[i]));
                // vx[i+1] = vx[i] + 0.5*(dur/n)*(vxDot[i+1] + vxDot[i])
                prob.AddConstraints(n, i => vx[i + 1] == vx[i] + 0.5 * dur / n * (vxDot[i + 1] + vxDot[i]));
                // vy[i+1] = vy[i] + 0.5*(dur/n)*(vyDot[i+1] + vyDot[i])
                prob.AddConstraints(n, i => vy[i + 1] == vy[i] + 0.5 * dur / n * (vyDot[i + 1] + vyDot[i]));

                // Adjust velocity for the effects of the wind
                Expression[] vxWindAdjusted = new Expression[n + 1];
                Expression[] vyWindAdjusted = new Expression[n + 1];
                Expression[] drag = new Expression[n + 1];
                Expression[] lift = new Expression[n + 1];
                Expression[] sinEta = new Expression[n + 1];
                Expression[] cosEta = new Expression[n + 1];

                for (int i = 0; i <= n; i++)
                {
                    if (wind == 2)
                        // vxAdj = vx + 5*(1 + arctan((y - 30) / 10))
                        vxWindAdjusted[i] = vx[i] + 5.0 * (1.0 + Arctan((y[i] - 30.0) / 10.0));
                    else
                        vxWindAdjusted[i] = vx[i];

                    if (wind == 1)
                    {
                        // upWindVel = (x[i]/r - 2.5)^2
                        Expression upWindVel = Pow(x[i] / r - 2.5, 2.0);
                        // vyAdj = vy - um * (1 - upWindVel) * e^(-upWindVel)
                        vyWindAdjusted[i] = vy[i] - um * (1.0 - upWindVel) * Exp(-1.0 * upWindVel);
                    }
                    else
                        vyWindAdjusted[i] = vy[i];

                    // vr = sqrt(vx^2 + vy^2)
                    Expression vr = Sqrt(Pow(vxWindAdjusted[i], 2.0) + Pow(vyWindAdjusted[i], 2.0));
                    // dragCoef = c0 + k * cLift^2
                    Expression dragCoef = c0 + k * Pow(cLift[i], 2.0);
                    // drag = 0.5 * rho * span * dragCoef * vr^2
                    drag[i] = 0.5 * rho * span * dragCoef * Pow(vr, 2.0);
                    // lift = 0.5 * rho * span * cLift * vr^2
                    lift[i] = 0.5 * rho * span * cLift[i] * Pow(vr, 2.0);
                    // sinEta = vy / vr
                    sinEta[i] = vyWindAdjusted[i] / vr;
                    // cosEta = vx / vr
                    cosEta[i] = vxWindAdjusted[i] / vr;
                }

                // Equations of motion: F = m * a
                // vxDot = (-Lift * sinEta - Drag * cosEta) / mass
                prob.AddConstraints(n, i => vxDot[i + 1] ==
                        (-1.0 * lift[i + 1] * sinEta[i + 1] - drag[i + 1] * cosEta[i + 1]) / mass);
                // vyDot = (Lift * cosEta - Drag * sinEta - weight) / mass
                prob.AddConstraints(n, i => vyDot[i + 1] ==
                        (lift[i + 1] * cosEta[i + 1] - drag[i + 1] * sinEta[i + 1] - weight) / mass);

                // Dump the problem to disk so that we can inspect it.
                prob.WriteProb("glidert.lp", "l");

                // Solve to local optimality since solving to global optimality takes some time
                prob.NLPSolver = Optimizer.Constants.NLPSOLVER_LOCAL;
                // LocalSolver defaults to automatic selection, which picks Knitro for
                // this problem. Uncomment the line below to use SLP instead.
                // prob.LocalSolver = Optimizer.Constants.LOCALSOLVER_XSLP;

                // Set initial values for the local solve
                double[] xInitial = new double[n + 1];
                double[] yInitial = new double[n + 1];
                double[] vxInitial = new double[n + 1];
                double[] vyInitial = new double[n + 1];
                double[] vxDotInitial = new double[n + 1];
                double[] vyDotInitial = new double[n + 1];
                double[] cLiftInitial = new double[n + 1];
                for (int i = 0; i <= n; i++)
                {
                    xInitial[i] = 5000.0 * i / n;
                    yInitial[i] = -100.0 * i / n + 1000.0;
                    vxInitial[i] = vx0;
                    vyInitial[i] = vy0;
                    vxDotInitial[i] = 0.0;
                    vyDotInitial[i] = 0.0;
                    cLiftInitial[i] = 0.0;
                }
                prob.NlpSetInitVal(x, xInitial);
                prob.NlpSetInitVal(y, yInitial);
                prob.NlpSetInitVal(vx, vxInitial);
                prob.NlpSetInitVal(vy, vyInitial);
                prob.NlpSetInitVal(vxDot, vxDotInitial);
                prob.NlpSetInitVal(vyDot, vyDotInitial);
                prob.NlpSetInitVal(cLift, cLiftInitial);

                // Solve
                prob.Optimize();
                if (prob.SolStatus != SolStatus.Optimal && prob.SolStatus != SolStatus.Feasible)
                    throw new Exception($"optimization failed with status {prob.SolStatus}");
                double[] sol = prob.GetSolution();

                Console.WriteLine($"Flight distance {x[n].GetValue(sol)} over a time of {dur.GetValue(sol)}.");
            }
        }
    }
}

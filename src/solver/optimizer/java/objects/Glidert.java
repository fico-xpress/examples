// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.arctan;
import static com.dashoptimization.objects.Utils.div;
import static com.dashoptimization.objects.Utils.exp;
import static com.dashoptimization.objects.Utils.minus;
import static com.dashoptimization.objects.Utils.mul;
import static com.dashoptimization.objects.Utils.plus;
import static com.dashoptimization.objects.Utils.pow;
import static com.dashoptimization.objects.Utils.sqrt;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Maximize the total horizontal distance a hang glider flies.
 *
 * Configure the wind conditions by setting the parameter wind: wind = 0 : no
 * wind wind = 1 : thermal uplift 250m ahead (positive in a neighborhood of 250m
 * but drops to zero exponentially away from this point) wind = 2 : wind shear
 * (a rapidly diminishing head wind that may cause the glider to stall)
 *
 * Trapezoidal discretization.
 *
 * Based on AMPL models hang_midpt.mod, shear_midpt.mod, stableair_midpt.mod
 * Source: http://www.orfe.princeton.edu/~rvdb/ampl/nlmodels/hang/ Reference: R.
 * Bulirsch, E. Nerz, H.J. Pesch, and O. von Stryk, "Combining Direct and
 * Indirect Methods in Optimal Control: Range Maximization of a Hang Glider", in
 * "Optimal Control: Calculus of Variations, Optimal Control Theory and
 * Numerical Methods", ed. by R. Bulirsch, A. Miele, J. Stoer, and K.H. Well
 * (1993) Birkhauser
 *
 * *** This model cannot be run with a Community Licence for the provided data
 * instance ***
 */
public class Glidert {

    /**
     * Wind conditions; possible values: 0, 1, 2 (see description at the beginning
     * of the file).
     */
    static int wind = 1;
    /** Number of discretization points */
    static final int n = 150;

    /** Initial horizontal position */
    static final double x0 = 0.0;
    /** Initial vertical position */
    static final double y0 = 1000.0;
    /** Initial horizontal velocity */
    static final double vx0 = 13.2275675;
    /** Initial vertical velocity */
    static final double vy0 = -1.28750052;
    /** Final vertical position */
    static final double yn = 900.0;
    /** Final horizontal velocity */
    static final double vxn = 13.2275675;
    /** Final vertical velocity */
    static final double vyn = -1.28750052;
    /** Upward wind-velocity coefficient (m/sec) */
    static final double um = 2.5;
    /** Upward wind-velocity ratio (m) */
    static final double r = 100.0;
    /** First drag coefficient constant */
    static final double c0 = 0.034;
    /** Second drag coefficient constant */
    static final double k = 0.069662;
    /** Mass of glider & pilot */
    static final double mass = 100.0;
    /** Wing area */
    static final double span = 14.0;
    /** Air density */
    static final double rho = 1.13;
    /** Acceleration due to gravity */
    static final double grav = 9.80665;
    /** Weight of glider & pilot (= m*g) */
    static final double weight = mass * grav;
    /** Minimum lift */
    static final double clMin = 0.0;
    /** Maximum lift */
    static final double clMax = 1.4;
    /** Minimum velocity */
    static final double velMin = -100.0;
    /** Maximum velocity */
    static final double velMax = 100.0;
    /** Minimum acceleration */
    static final double accMin = -3.0;
    /** Maximum acceleration */
    static final double accMax = 3.0;
    /** Minimum duration */
    static final double durMin = 10.0;
    /** Maximum duration */
    static final double durMax = 200.0;

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/

            // total duration of the flight
            Variable dur = prob.addVariable(durMin, durMax, ColumnType.Continuous, "dur");

            // horizontal position at time step t
            Variable[] x = prob.addVariables(n + 1).withName(t -> String.format("x_%d", t)).toArray();

            // vertical position at time step t
            Variable[] y = prob.addVariables(n + 1).withName(t -> String.format("y_%d", t)).toArray();

            // horizontal velocity at time step t
            Variable[] vx = prob.addVariables(n + 1).withName(t -> String.format("vx_%d", t)).withLB(velMin)
                    .withUB(velMax).toArray();

            // vertical velocity at time step t
            Variable[] vy = prob.addVariables(n + 1).withName(t -> String.format("vy_%d", t)).withLB(velMin)
                    .withUB(velMax).toArray();

            // derivative of vx at time step t
            Variable[] vxDot = prob.addVariables(n + 1).withName(t -> String.format("vxDot_%d", t)).withLB(accMin)
                    .withUB(accMax).toArray();

            // derivative of vy at time step t
            Variable[] vyDot = prob.addVariables(n + 1).withName(t -> String.format("vyDot_%d", t)).withLB(accMin)
                    .withUB(accMax).toArray();

            // Lift coefficient (control variable) at time step t
            Variable[] cLift = prob.addVariables(n + 1).withName(t -> String.format("cLift_%d", t)).withLB(clMin)
                    .withUB(clMax).toArray();

            // Objective is to maximize the horizontal position in the final time step
            prob.setObjective(x[n], XPRSenumerations.ObjSense.MAXIMIZE);

            // initial and final states
            prob.addConstraint(x[0].eq(x0));
            prob.addConstraint(y[0].eq(y0));
            if (wind == 2) {
                prob.addConstraint(vx[0].eq(vx0 - 2.5 * (1 + Math.atan((y0 - 30) / 10))));
            } else {
                prob.addConstraint(vx[0].eq(vx0));
            }
            prob.addConstraint(vy[0].eq(vy0));
            prob.addConstraint(y[n].eq(yn));
            prob.addConstraint(vx[n].eq(vxn));
            prob.addConstraint(vy[n].eq(vyn));

            // monotonicity
            prob.addConstraints(n, k -> x[k + 1].geq(x[k]));

            // trapezoidal discretization
            // x[i+1] = x[i] + 0.5*(dur/N)*(vx[i+1] + vx[i])
            prob.addConstraints(n,
                    i -> plus(x[i], mul(0.5, mul(mul((double) 1 / n, dur), plus(vx[i + 1], vx[i])))).eq(x[i + 1]));
            // y[i+1] = y[i] + 0.5*(dur/N)*(vy[i+1] + vy[i])
            prob.addConstraints(n,
                    i -> plus(y[i], mul(0.5, mul(mul((double) 1 / n, dur), plus(vy[i + 1], vy[i])))).eq(y[i + 1]));
            // vx[i+1] = vx[i] + 0.5*(dur/N)*(vxDot[i+1] + vxDot[i])
            prob.addConstraints(n,
                    i -> plus(vx[i], mul(0.5, mul(mul((double) 1 / n, dur), plus(vxDot[i + 1], vxDot[i]))))
                            .eq(vx[i + 1]));
            // vy[i+1] = vy[i] + 0.5*(dur/N)*(vyDot[i+1] + vyDot[i])
            prob.addConstraints(n,
                    i -> plus(vy[i], mul(0.5, mul(mul((double) 1 / n, dur), plus(vyDot[i + 1], vyDot[i]))))
                            .eq(vy[i + 1]));

            // Adjust velocity for the effects of the wind
            final Expression[] vxWindAdjusted = new Expression[n + 1];
            final Expression[] vyWindAdjusted = new Expression[n + 1];
            final Expression[] drag = new Expression[n + 1];
            final Expression[] lift = new Expression[n + 1];
            final Expression[] sinEta = new Expression[n + 1];
            final Expression[] cosEta = new Expression[n + 1];

            for (int i = 0; i <= n; i++) {
                if (wind == 2) {
                    // vx_adj = vx + 5*(1 + arctan((y - 30) / 10))
                    vxWindAdjusted[i] = plus(vx[i], mul(5.0, plus(1, arctan(div(minus(y[i], 30.0), 10.0)))));
                } else {
                    vxWindAdjusted[i] = vx[i];
                }
                if (wind == 1) {
                    // upWindVel = (x[j]/R - 2.5)^2
                    Expression upWindVel = pow(minus(div(x[i], r), 2.5), 2.0);
                    // vy_adj = vy - UM * (1 - upWindVel) * e^(-upWindVel)
                    vyWindAdjusted[i] = minus(vy[i], mul(um, mul(minus(1, upWindVel), exp(mul(-1, upWindVel)))));
                } else {
                    vyWindAdjusted[i] = vy[i];
                }
                // vr = sqrt(vx^2 + vy^2)
                Expression vr = sqrt(plus(pow(vxWindAdjusted[i], 2.0), pow(vyWindAdjusted[i], 2.0)));
                // drag coefficient = c0 + K * clift^2
                Expression drag_coef = plus(c0, mul(k, pow(cLift[i], 2.0)));
                // drag = 0.5 * rho * span * drag_coef * vr^2
                drag[i] = mul(0.5 * rho * span, mul(drag_coef, pow(vr, 2.0)));
                // lift = 0.5 * rho * span * cLift * vr^2
                lift[i] = mul(0.5 * rho * span, mul(cLift[i], pow(vr, 2.0)));
                // sinEta = vy / vr
                sinEta[i] = div(vyWindAdjusted[i], vr);
                // cosEta = vx / vr
                cosEta[i] = div(vxWindAdjusted[i], vr);
            }

            // Equations of motion: F = m * a
            // vxDot = (-Lift * sinEta - Drag * cosEta) / mass
            prob.addConstraints(n,
                    i -> div(minus(mul(mul(lift[i + 1], -1.0), sinEta[i + 1]), mul(drag[i + 1], cosEta[i + 1])), mass)
                            .eq(vxDot[i + 1]));
            // vyDot = (Lift * cosEta - Drag * sinEta - weight) / mass
            prob.addConstraints(n,
                    i -> div(minus(minus(mul(lift[i + 1], cosEta[i + 1]), mul(drag[i + 1], sinEta[i + 1])), weight),
                            mass).eq(vyDot[i + 1]));

            // Dump the problem to disk so that we can inspect it.
            prob.writeProb("glidert.lp", "l");

            // Solve to local optimality since solving to global optimality takes some time
            prob.controls().setNLPSolver(XPRSconstants.NLPSOLVER_LOCAL);
            // Choose between solving with SLP or Knitro
            // prob.controls().setLocalSolver(XPRSconstants.LOCALSOLVER_XSLP);

            // Set initial values for the local solve
            final double[] xInitial = new double[n + 1];
            final double[] yInitial = new double[n + 1];
            final double[] vxInitial = new double[n + 1];
            final double[] vyInitial = new double[n + 1];
            final double[] vxDotInitial = new double[n + 1];
            final double[] vyDotInitial = new double[n + 1];
            final double[] cLiftInitial = new double[n + 1];
            for (int i = 0; i <= n; i++) {
                xInitial[i] = 5000 * i / n;
                yInitial[i] = -100 * i / (n + 1000);
                vxInitial[i] = vx0;
                vyInitial[i] = vy0;
                vxDotInitial[i] = 0.0;
                vyDotInitial[i] = 0.0;
                cLiftInitial[i] = 0.0;
            }
            prob.nlpSetInitVal(x, xInitial);
            prob.nlpSetInitVal(y, yInitial);
            prob.nlpSetInitVal(vx, vxInitial);
            prob.nlpSetInitVal(vy, vyInitial);
            prob.nlpSetInitVal(vxDot, vxDotInitial);
            prob.nlpSetInitVal(vyDot, vyDotInitial);
            prob.nlpSetInitVal(cLift, cLiftInitial);

            // Solve
            prob.optimize();

            // print solution
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                    && prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();

            System.out.println("Flight distance " + x[n].getValue(sol) + " over a time of " + dur.getValue(sol) + ".");
        }
    }
}

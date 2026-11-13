// (c) 2024-2026 Fair Isaac Corporation

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

#include <cmath>
#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
// The functions we use for building the model.
using xpress::objects::utils::arctan;
using xpress::objects::utils::div;
using xpress::objects::utils::exp;
using xpress::objects::utils::pow;
using xpress::objects::utils::sqrt;

class Glidert {
 public:
  /**
   * Wind conditions; possible values: 0, 1, 2 (see description at the beginning
   * of the file).
   */
  static inline int wind = 1;
  /** Number of discretization points */
  static constexpr int const n = 150;
  /** Initial horizontal position */
  static constexpr double const x_0 = 0.0;
  /** Initial vertical position */
  static constexpr double const y_0 = 1000.0;
  /** Initial horizontal velocity */
  static constexpr double const vx_0 = 13.2275675;
  /** Initial vertical velocity */
  static constexpr double const vy_0 = -1.28750052;
  /** Final vertical position */
  static constexpr double const yN = 900.0;
  /** Final horizontal velocity */
  static constexpr double const vxN = 13.2275675;
  /** Final vertical velocity */
  static constexpr double const vyN = -1.28750052;
  /** Upward wind-velocity coefficient (m/sec) */
  static constexpr double const um = 2.5;
  /** Upward wind-velocity ratio (m) */
  static constexpr double const r = 100.0;
  /** First drag coefficient constant */
  static constexpr double const c0 = 0.034;
  /** Second drag coefficient constant */
  static constexpr double const k = 0.069662;
  /** Mass of glider & pilot */
  static constexpr double const mass = 100.0;
  /** Wing area */
  static constexpr double const span = 14.0;
  /** Air density */
  static constexpr double const rho = 1.13;
  /** Acceleration due to gravity */
  static constexpr double const grav = 9.80665;
  /** Weight of glider & pilot (= m*g) */
  static constexpr double const weight = mass * grav;
  /** Minimum lift */
  static constexpr double const clMin = 0.0;
  /** Maximum lift */
  static constexpr double const clMax = 1.4;
  /** Minimum velocity */
  static constexpr double const velMin = -100.0;
  /** Maximum velocity */
  static constexpr double const velMax = 100.0;
  /** Minimum acceleration */
  static constexpr double const accMin = -3.0;
  /** Maximum acceleration */
  static constexpr double const accMax = 3.0;
  /** Minimum duration */
  static constexpr double const durMin = 10.0;
  /** Maximum duration */
  static constexpr double const durMax = 200.0;

  static int solve() {
    XpressProblem prob;
    // Output all messages.
    prob.callbacks.addMessageCallback(XpressProblem::console);

    /**** VARIABLES ****/

    // total duration of the flight
    auto dur = prob.addVariable(durMin, durMax, ColumnType::Continuous, "dur");

    // horizontal position at time step t
    auto x = prob.addVariables(n + 1).withName("x_%d").toArray();

    // vertical position at time step t
    auto y = prob.addVariables(n + 1).withName("y_%d").toArray();

    // horizontal velocity at time step t
    auto vx = prob.addVariables(n + 1)
                  .withName("vx_%d")
                  .withLB(velMin)
                  .withUB(velMax)
                  .toArray();

    // vertical velocity at time step t
    auto vy = prob.addVariables(n + 1)
                  .withName("vy_%d")
                  .withLB(velMin)
                  .withUB(velMax)
                  .toArray();

    // derivative of vx at time step t
    auto vxDot = prob.addVariables(n + 1)
                     .withName("vxDot_%d")
                     .withLB(accMin)
                     .withUB(accMax)
                     .toArray();

    // derivative of vy at time step t
    auto vyDot = prob.addVariables(n + 1)
                     .withName("vyDot_%d")
                     .withLB(accMin)
                     .withUB(accMax)
                     .toArray();

    // Lift coefficient (control variable) at time step t
    auto cLift = prob.addVariables(n + 1)
                     .withName("cLift_%d")
                     .withLB(clMin)
                     .withUB(clMax)
                     .toArray();

    // Objective is to maximize the horizontal position in the final time step
    prob.setObjective(x[n], ObjSense::Maximize);

    // initial and final states
    prob.addConstraint(x[0] == x_0);
    prob.addConstraint(y[0] == y_0);
    if (wind == 2) {
      prob.addConstraint(vx[0] == vx_0 - 2.5 * (1 + atan((y_0 - 30) / 10)));
    } else {
      prob.addConstraint(vx[0] == vx_0);
    }
    prob.addConstraint(vy[0] == vy_0);
    prob.addConstraint(y[n] == yN);
    prob.addConstraint(vx[n] == vxN);
    prob.addConstraint(vy[n] == vyN);

    // monotonicity
    prob.addConstraints(n, [&](auto k) { return x[k + 1] >= x[k]; });

    // trapezoidal discretization
    // x[i+1] = x[i] + 0.5*(dur/N)*(vx[i+1] + vx[i])
    prob.addConstraints(n, [&](auto i) {
      return x[i + 1] == x[i] + 0.5 * dur / n * (vx[i + 1] + vx[i]);
    });
    // y[i+1] = y[i] + 0.5*(dur/N)*(vy[i+1] + vy[i])
    prob.addConstraints(n, [&](auto i) {
      return y[i + 1] == y[i] + 0.5 * dur / n * (vy[i + 1] + vy[i]);
    });
    // vx[i+1] = vx[i] + 0.5*(dur/N)*(vxDot[i+1] + vxDot[i])
    prob.addConstraints(n, [&](auto i) {
      return vx[i + 1] == vx[i] + 0.5 * dur / n * (vxDot[i + 1] + vxDot[i]);
    });
    // vy[i+1] = vy[i] + 0.5*(dur/N)*(vyDot[i+1] + vyDot[i])
    prob.addConstraints(n, [&](auto i) {
      return vy[i + 1] == vy[i] + 0.5 * dur / n * (vyDot[i + 1] + vyDot[i]);
    });

    // Adjust velocity for the effects of the wind
    std::vector<Expression> vxWindAdjusted(n + 1);
    std::vector<Expression> vyWindAdjusted(n + 1);
    std::vector<Expression> drag(n + 1);
    std::vector<Expression> lift(n + 1);
    std::vector<Expression> sinEta(n + 1);
    std::vector<Expression> cosEta(n + 1);

    for (int i = 0; i <= n; i++) {
      if (wind == 2) {
        // vx_adj = vx + 5*(1 + arctan((y - 30) / 10))
        vxWindAdjusted[i] = vx[i] + 5.0 * (1.0 + arctan((y[i] - 30.0) / 10.0));
      } else {
        vxWindAdjusted[i] = vx[i];
      }
      if (wind == 1) {
        // upWindVel = (x[i]/R - 2.5)^2
        Expression upWindVel = pow(x[i] / r - 2.5, 2.0);
        // vy_adj = vy - UM * (1 - upWindVel) * e^(-upWindVel)
        vyWindAdjusted[i] = vy[i] - um * (1.0 - upWindVel) * exp(-upWindVel);
      } else {
        vyWindAdjusted[i] = vy[i];
      }
      // vr = sqrt(vx^2 + vy^2)
      Expression vr =
          sqrt(pow(vxWindAdjusted[i], 2.0) + pow(vyWindAdjusted[i], 2.0));
      // drag coefficient = c0 + K * clift^2
      Expression drag_coef = c0 + k * pow(cLift[i], 2.0);
      // drag = 0.5 * rho * span * drag_coef * vr^2
      drag[i] = 0.5 * rho * span * drag_coef * pow(vr, 2.0);
      // lift = 0.5 * rho * span * cLift * vr^2
      lift[i] = 0.5 * rho * span * cLift[i] * pow(vr, 2.0);
      // sinEta = vy / vr
      sinEta[i] = vyWindAdjusted[i] / vr;
      // cosEta = vx / vr
      cosEta[i] = vxWindAdjusted[i] / vr;
    }

    // Equations of motion: F = m * a
    // vxDot = (-Lift * sinEta - Drag * cosEta) / mass
    prob.addConstraints(n, [&](auto i) {
      return vxDot[i + 1] ==
             (-lift[i + 1] * sinEta[i + 1] - drag[i + 1] * cosEta[i + 1]) / mass;
    });

    // vyDot = (Lift * cosEta - Drag * sinEta - weight) / mass
    prob.addConstraints(n, [&](auto i) {
      return vyDot[i + 1] == (lift[i + 1] * cosEta[i + 1] -
                              drag[i + 1] * sinEta[i + 1] - weight) /
                                 mass;
    });

    // Solve to local optimality since solving to global optimality takes some
    // time
    prob.controls.setNlpSolver(XPRS_NLPSOLVER_LOCAL);
    // Choose between solving with SLP or Knitro
    // prob.controls().setLocalSolver(XPRS_LOCALSOLVER_XSLP);

    // Set initial values for the local solve
    std::vector<double> xInitial(n + 1);
    std::vector<double> yInitial(n + 1);
    std::vector<double> vxInitial(n + 1);
    std::vector<double> vyInitial(n + 1);
    std::vector<double> vxDotInitial(n + 1);
    std::vector<double> vyDotInitial(n + 1);
    std::vector<double> cLiftInitial(n + 1);
    for (int i = 0; i <= n; i++) {
      xInitial[i] = 5000 * i / n;
      yInitial[i] = -100 * i / (n + 1000);
      vxInitial[i] = vx_0;
      vyInitial[i] = vy_0;
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

    // Dump the problem to disk so that we can inspect it.
    prob.writeProb("glidert.lp", "l");

    // Solve
    prob.optimize();

    // print solution
    if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
        prob.attributes.getSolStatus() != SolStatus::Feasible)
      throw std::runtime_error("optimization failed with status " +
                               to_string(prob.attributes.getSolStatus()));
    auto sol = prob.getSolution();

    std::cout << "Flight distance " << x[n].getValue(sol) << " over a time of "
              << dur.getValue(sol) << "." << std::endl;

    return 0;
  }
};

int
main(void)
{
  Glidert::solve();
  return 0;
}

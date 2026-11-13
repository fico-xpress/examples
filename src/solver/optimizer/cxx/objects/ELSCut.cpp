// (c) 2024-2026 Fair Isaac Corporation

/** Economic lot sizing, ELS, problem.
 * Solved by adding (l,S)-inequalities in a branch-and-cut heuristic
 * (using the cut manager).
 *
 * ELS considers production planning over a horizon of T periods. In period t,
 * t=1,...,T, there is a given demand DEMAND[t] that must be satisfied by
 * production prod[t] in period t and by inventory carried over from previous
 * periods. There is a set-up up cost SETUPCOST[t] associated with production in
 * period t. The unit production cost in period t is PRODCOST[t]. There is no
 * inventory or stock-holding cost.
 *
 * *** This model cannot be run with a Community Licence ***
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

double const EPS = 1e-6;
int const T = 6; /* Number of time periods */

/* Data */
std::vector<double> DEMAND{1, 3, 5, 3, 4, 2};       /* Demand per period */
std::vector<double> SETUPCOST{17, 16, 11, 6, 9, 6}; /* Setup cost / period */
std::vector<double> PRODCOST{5, 3, 2, 1, 3, 1};     /* Prod. cost / period */
std::vector<std::vector<double>>
    D(T, std::vector<double>(T)); /* Total demand in periods t1 - t2 */

void printProblemStatus(XpressProblem const &prob) {
  std::cout << "Problem status:" << std::endl
            << "\tSolve status: " << prob.attributes.getSolveStatus()
            << std::endl
            << "\tLP status: " << prob.attributes.getLpStatus() << std::endl
            << "\tMIP status: " << prob.attributes.getMipStatus() << std::endl
            << "\tSol status: " << prob.attributes.getSolStatus() << std::endl;
}

/**
 * Cut generation algorithm:
 * - get the solution values
 * - identify and set up violated constraints
 * - add cuts to the problem
 * @param p      Xpress solver instance as passed to the callback.
 * @param prod   Production variables.
 * @param setup  Setup variables.
 */
void separate(XpressProblem &p, std::vector<Variable> &prod,
              std::vector<Variable> &setup) {
  int ncut = 0;
  // Add cut only to optimal relaxations
  if (p.attributes.getLpStatus() != LPStatus::Optimal)
    return;

  auto sol = p.getCallbackSolution();
  auto slack = p.getCallbackSlacks();
  auto duals = p.getCallbackDuals();
  auto djs = p.getCallbackRedCosts();
  // Search for violated constraints:
  for (int l = 0; l < T; l++) {
    double ds = 0.0;
    for (int t = 0; t <= l; t++) {
      if (prod[t].getValue(sol) < D[t][l] * setup[t].getValue(sol) + EPS) {
        ds += prod[t].getValue(sol);
      } else {
        ds += D[t][l] * setup[t].getValue(sol);
      }
    }

    // Add the violated inequality: the minimum of the actual production
    // prod[t] and the maximum potential production D[t][l]*setup[t]
    // in periods 0 to l must at least equal the total demand in periods
    // 0 to l.
    // sum(t=1:l) min(prod[t], D[t][l]*setup[t]) >= D[0][l] */
    if (ds < D[0][l] - EPS) {
      Expression cut = sum(l + 1, [&](auto t) {
        if (prod[t].getValue(sol) < D[t][l] * setup[t].getValue(sol) + EPS)
          return 1.0 * prod[t];
        else
          return D[t][l] * setup[t];
      });
      p.addCut(0, cut >= D[0][1]);
      ncut++;
    }
  }

  if (ncut > 0) {
    std::cout << "Cuts added: " << ncut << "(depth "
              << p.attributes.getNodeDepth() << ", node "
              << p.attributes.getNodes() << ")" << std::endl;
  }
}

/** Create the model in P.
 * @param p      The Xpress Solver instance in which the model is created.
 * @param prod   Array to store the production variables.
 * @param setup  Array to store the setup variables.
 */
void modEls(XpressProblem &p, std::vector<Variable> &prod,
            std::vector<Variable> &setup) {
  for (int s = 0; s < T; s++)
    for (int t = 0; t < T; t++)
      for (int k = s; k <= t; k++)
        D[s][t] += DEMAND[k];

  // Variables
  prod = p.addVariables(T)
             .withType(ColumnType::Continuous)
             .withName([](auto t) { return "prod" + std::to_string(t + 1); })
             .toArray();

  setup = p.addVariables(T)
              .withType(ColumnType::Binary)
              .withName([](auto t) { return "setup" + std::to_string(t + 1); })
              .toArray();

  // Objective: Minimize total cost
  p.setObjective(
      sum(scalarProduct(setup, SETUPCOST), scalarProduct(prod, PRODCOST)),
      ObjSense::Minimize);

  // Constraints

  // Production in period t must not exceed the total demand for the
  // remaining periods; if there is production during t then there
  // is a setup in t
  // for all t in [0,T[
  // prod[t] <= setup[t] * D[t][T-1]
  p.addConstraints(T,
                   [&](auto t) { return prod[t] <= setup[t] * D[t][T - 1]; });

  // Production in periods 0 to t must satisfy the total demand
  // during this period of time
  // for all t in [0,T[
  // sum(s in [0,t+1[) prod[s] >= D[0][t]
  p.addConstraints(T, [&](auto t) {
    return sum(t + 1, [&](auto s) { return prod[s]; }) >= D[0][t];
  });
  p.writeProb("ELSCut.lp", "l");
}

/** Solve the model.
 * Cuts are added dynamically with the optnode callback.
 * @param p      The Xpress Solver instance that is populated with the model.
 * @param prod   Production variables.
 * @param setup  Setup variables.
 */
void solveEls(XpressProblem &p, std::vector<Variable> &prod,
              std::vector<Variable> &setup) {
  p.callbacks.addMessageCallback(XpressProblem::console);
  p.controls.setLpLog(0);
  p.controls.setMipLog(3);
  // Disable automatic cuts - we use our own
  p.controls.setCutStrategy(XPRS_CUTSTRATEGY_NONE);
  // Switch presolve off
  p.controls.setPresolve(XPRS_PRESOLVE_NONE);
  p.controls.setNodePresolve(NodePresolve::Off);

  p.callbacks.addOptnodeCallback(
      [&](auto &prob, auto) { separate(prob, prod, setup); });

  // Solve the MIP
  p.optimize();
  if (p.attributes.getSolStatus() != SolStatus::Optimal)
    throw std::runtime_error("optimization failed with status " +
                             to_string(p.attributes.getSolStatus()));
  // Get the solution values:
  auto sol = p.getSolution();

  // Print out the solution:
  for (int t = 0; t < T; t++) {
    std::cout << "Period " << (t + 1) << ": prod" << prod[t].getValue(sol)
              << " (demand: " << DEMAND[t] << ", cost: " << PRODCOST[t] << ")"
              << ", setup " << setup[t].getValue(sol) << " (cost "
              << SETUPCOST[t] << ")" << std::endl;
  }
  printProblemStatus(p);
}

int main() {
  XpressProblem prob;
  std::vector<Variable> prod;
  std::vector<Variable> setup;
  modEls(prob, prod, setup);   // Model the problem
  solveEls(prob, prod, setup); // Solve the problem
  return 0;
}

// (c) 2024-2026 Fair Isaac Corporation

/**
 * Recursion solving a non-linear financial planning problem. The problem is to
 * solve
 * <pre>
 *     net(t) = Payments(t) - interest(t)
 *     balance(t) = balance(t-1) - net(t)
 *     interest(t) = (92/365) * balance(t) * interest_rate
 *   where
 *     balance(0) = 0
 *     balance[T] = 0
 *   for interest_rate
 * </pre>
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

int const T = 6;

/* Data */
/* An INITIAL GUESS as to interest rate x */
double const X = 0.00;
/* An INITIAL GUESS as to balances b(t) */
std::vector<double> B{1, 1, 1, 1, 1, 1};
std::vector<double> P{-1000, 0, 0, 0, 0, 0};                 /* Payments */
std::vector<double> R{206.6, 206.6, 206.6, 206.6, 206.6, 0}; /* " */
std::vector<double> V{-2.95, 0, 0, 0, 0, 0};                 /* " */

struct RecursiveFinancialPlanning {
  /** The optimizer instance. */
  XpressProblem prob;
  // Variables and constraints
  std::vector<Variable> b; /**< Balance */
  Variable x;              /**< Interest rate */
  Variable dx;             /**< Change to x */
  // Constraints that will be modified.
  std::vector<Inequality> interest;
  Inequality ctrd;

  void printIteration(int it, double variation) {
    auto sol = prob.getSolution();
    std::cout << "---------------- Iteration " << it << " ----------------"
              << std::endl;
    std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;
    std::cout << "Variation: " << variation << std::endl;
    std::cout << "x: " << x.getValue(sol) << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
  }

  void printProblemSolution() {
    auto sol = prob.getSolution();
    std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;
    std::cout << "Interest rate: " << (x.getValue(sol) * 100) << " percent"
              << std::endl;
    std::cout << "Variables:" << std::endl << "t";
    for (Variable const &v : prob.getVariables()) {
      std::cout << "[" << v.getName() << ": " << v.getValue(sol) << "] ";
    }
    std::cout << std::endl;
  }

  /***********************************************************************/
  void modFinNLP() {
    interest = std::vector<Inequality>(T);

    // Balance
    b = prob.addVariables(T)
            .withName("b_%d")
            .withLB(XPRS_MINUSINFINITY)
            .toArray();

    // Interest rate
    x = prob.addVariable("x");

    // Interest rate change
    dx = prob.addVariable(XPRS_MINUSINFINITY, XPRS_PLUSINFINITY,
                          ColumnType::Continuous, "dx");

    std::vector<Variable> i = prob.addVariables(T).withName("i_%d").toArray();

    std::vector<Variable> n = prob.addVariables(T)
                                  .withName("n_%d")
                                  .withLB(XPRS_MINUSINFINITY)
                                  .toArray();

    std::vector<Variable> epl =
        prob.addVariables(T).withName("epl_%d").toArray();

    std::vector<Variable> emn =
        prob.addVariables(T).withName("emn_%d").toArray();

    // Fixed variable values
    i[0].fix(0);
    b[T - 1].fix(0);

    // Objective
    prob.setObjective(sum(epl) + sum(emn), ObjSense::Minimize);

    // Constraints
    // net = payments - interest
    prob.addConstraints(T, [&](auto t) {
      return (n[t] == (P[t] + R[t] + V[t]) - i[t])
          .setName(xpress::format("net_%d", t));
    });

    // Money balance across periods
    prob.addConstraints(T, [&](auto t) {
      if (t > 0)
        return (b[t] == b[t - 1]).setName(xpress::format("bal_%d", t));
      else
        return (b[t] == 0.0).setName(xpress::format("bal_%d", t));
    });

    // i(t) = (92/365)*( b(t-1)*X + B(t-1)*dx ) approx.
    for (int t = 1; t < T; ++t) {
      LinExpression iepx = LinExpression::create();
      iepx.addTerm(b[t - 1], X);
      iepx.addTerm(dx, B[t - 1]);
      iepx.addTerm(epl[t], 1.0);
      iepx.addTerm(emn[t], 1.0);
      interest[t] = prob.addConstraint((365 / 92.0) * i[t] == iepx)
                        .setName(xpress::format("int_%d", t));
    }

    // x = dx + X
    ctrd = prob.addConstraint((x == dx + X).setName("def"));
    prob.writeProb("Recur.lp", "l");
  }

  /**************************************************************************/
  /* Recursion loop (repeat until variation of x converges to 0): */
  /* save the current basis and the solutions for variables b[t] and x */
  /* set the balance estimates B[t] to the value of b[t] */
  /* set the interest rate estimate X to the value of x */
  /* reload the problem and the saved basis */
  /* solve the LP and calculate the variation of x */
  /**************************************************************************/
  void solveFinNLP() {
    double variation = 1.0;

    prob.callbacks.addMessageCallback(XpressProblem::console);
    prob.controls.setMipLog(0);

    // Switch automatic cut generation off
    prob.controls.setCutStrategy(XPRS_CUTSTRATEGY_NONE);
    // Solve the problem
    prob.optimize();
    if (prob.attributes.getSolStatus() != SolStatus::Optimal)
      throw std::runtime_error("failed to optimize with status " +
                               to_string(prob.attributes.getSolStatus()));

    for (int it = 1; variation > 1e-6; ++it) {
      // Optimization solution
      auto sol = prob.getSolution();

      printIteration(it, variation);
      printProblemSolution();
      // Change coefficients in interest[t]
      // Note: when inequalities are added to a problem then all variables are
      // moved to the left-hand side and all constants are moved to the
      // right-hand side. Since we are changing these extracted inequalities
      // directly, we have to use negative coefficients below.
      for (int t = 1; t < T; ++t) {
        prob.chgCoef(interest[t], dx, -b[t - 1].getValue(sol));
        prob.chgCoef(interest[t], b[t - 1], -x.getValue(sol));
      }

      // Change constant term of ctrd
      ctrd.setRhs(x.getValue(sol));

      // Solve the problem
      prob.optimize();
      auto newsol = prob.getSolution();
      if (prob.attributes.getSolStatus() != SolStatus::Optimal)
        throw std::runtime_error("failed to optimize with status " +
                                 to_string(prob.attributes.getSolStatus()));
      variation = fabs(x.getValue(newsol) - x.getValue(sol));
    }
    printProblemSolution();
  }
};

int main() {
  RecursiveFinancialPlanning planning;
  planning.modFinNLP();
  planning.solveFinNLP();
  return 0;
}

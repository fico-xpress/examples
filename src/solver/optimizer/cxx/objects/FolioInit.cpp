// (c) 2024-2026 Fair Isaac Corporation

/** Modeling a small LP problem to perform portfolio optimization. */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/* Number of shares */
int const NSHARES = 10;
/* Number of high-risk shares */
int const NRISK = 5;
/* Number of North-American shares */
int const NNA = 4;
/* Estimated return in investment */
std::vector<double> const RET{5, 17, 26, 12, 8, 9, 7, 6, 31, 21};
/* High-risk values among shares */
std::vector<int> RISK{1, 2, 3, 8, 9};
/* Shares issued in N.-America */
std::vector<int> NA{0, 1, 2, 3};

void printProblemStatus(XpressProblem const &prob) {
  std::cout << "Problem status:" << std::endl
            << "\tSolve status: " << prob.attributes.getSolveStatus()
            << std::endl
            << "\tSol status: " << prob.attributes.getSolStatus() << std::endl;
}

int main(void) {
  XpressProblem prob;
  // Output all messages.
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /**** VARIABLES ****/
  std::vector<Variable> frac =
      prob.addVariables(NSHARES)
          /* Fraction of capital used per share */
          .withName("frac_%d")
          /* Upper bounds on the investment per share */
          .withUB(0.3)
          .toArray();

  /**** CONSTRAINTS ****/
  /* Limit the percentage of high-risk values */
  prob.addConstraint(sum(NRISK, [&](auto i) { return frac[RISK[i]]; }) <=
                     1.0 / 3.0)
      .setName("Risk");

  /* Minimum amount of North-American values */
  prob.addConstraint(sum(NNA, [&](auto i) { return frac[NA[i]]; }) >= 0.5)
      .setName("NA");

  /* Spend all the capital */
  prob.addConstraint(sum(frac) == 1.0).setName("Cap");

  /* Objective: maximize total return */
  prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);

  /* Solve */
  prob.optimize();

  /* Solution printing */
  printProblemStatus(prob);
  std::cout << "Total return: " << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (int i = 0; i < NSHARES; ++i) {
    std::cout << frac[i].getName() << ": " << (100.0 * frac[i].getValue(sol))
              << "%" << std::endl;
  }

  return 0;
}

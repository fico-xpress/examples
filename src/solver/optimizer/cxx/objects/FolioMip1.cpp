// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a small LP problem to perform portfolio optimization. -- Limiting
 * the total number of assets
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/* Max. number of different assets */
int const MAXNUM = 4;
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

int main() {
  XpressProblem prob;
  // Output all messages.
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /**** VARIABLES ****/
  /* Fraction of capital used per share */
  std::vector<Variable> frac =
      prob.addVariables(NSHARES)
          .withName("frac_%d")
          /* Upper bounds on the investment per share */
          .withUB(0.3)
          .toArray();

  /* Fraction of capital used per share */
  std::vector<Variable> buy = prob.addVariables(NSHARES)
                                  .withName("buy_%d")
                                  .withType(ColumnType::Binary)
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

  /* Limit the total number of assets */
  prob.addConstraint(sum(buy) <= MAXNUM).setName("MaxAssets");

  /* Linking the variables */
  /* frac .<= buy */
  prob.addConstraints(NSHARES, [&](auto i) {
    return (frac[i] <= buy[i]).setName("link_" + std::to_string(i));
  });

  /* Objective: maximize total return */
  prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);

  /* Solve */
  prob.optimize();

  /* Solution printing */
  printProblemStatus(prob);
  std::cout << "Total return: " << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (int i = 0; i < NSHARES; ++i)
    std::cout << frac[i].getName() << ": " << (100.0 * frac[i].getValue(sol))
              << "%"
              << " (" << buy[i].getValue(sol) << ")" << std::endl;
  return 0;
}

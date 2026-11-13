// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a small LP problem to perform portfolio optimization. -- Heuristic
 * solution
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
std::vector<int> const RISK{1, 2, 3, 8, 9};
/* Shares issued in N.-America */
std::vector<int> const NA{0, 1, 2, 3};

class FolioHeuristic {
  /** Optimizer problem. */
  XpressProblem prob;
  /** Fraction of capital used per share */
  std::vector<Variable> frac;
  /** 1 if asset is in portfolio, 0 otherwise */
  std::vector<Variable> buy;

public:
  FolioHeuristic()
      : prob(), frac(prob.addVariables(NSHARES)
                         /* Fraction of capital used per share */
                         .withName("frac_%d")
                         /* Upper bounds on the investment per share */
                         .withUB(0.3)
                         .toArray()),
        buy(prob.addVariables(NSHARES)
                /* Fraction of capital used per share */
                .withName("buy_%d")
                .withType(ColumnType::Binary)
                .toArray()) {}

  /** Print current problem status. */
  void printProblemStatus() const {
    std::cout << "Problem status:" << std::endl
              << "\tSolve status: " << prob.attributes.getSolveStatus()
              << std::endl
              << "\tLP status: " << prob.attributes.getLpStatus() << std::endl
              << "\tMIP status: " << prob.attributes.getMipStatus() << std::endl
              << "\tSol status: " << prob.attributes.getSolStatus()
              << std::endl;
  }

  /** Print current solution.
   * @param solveFlag What kind of solution this is.
   */
  void printProblemSolution(std::string solveFlag) {
    std::cout << "Total return (" << solveFlag
              << "): " << prob.attributes.getObjVal() << std::endl;
    auto sol = prob.getSolution();
    for (int i = 0; i < NSHARES; ++i) {
      std::cout << frac[i].getName() << ": " << (100. * frac[i].getValue(sol))
                << "%"
                << " (" << buy[i].getValue(sol) << ")" << std::endl;
    }
  }

  /** Load the model into the internal data structures. */
  void model() {
    // Output all messages.
    prob.callbacks.addMessageCallback(XpressProblem::console);

    /**** CONSTRAINTS ****/
    /* Limit the percentage of high-risk values */
    prob.addConstraint(sum(NRISK, [&](auto i) { return frac[RISK[i]]; }) <=
                       1.0 / 3.0);

    /* Minimum amount of North-American values */
    prob.addConstraint(sum(NNA, [&](auto i) { return frac[NA[i]]; }) >= 0.5);

    /* Spend all the capital */
    prob.addConstraint(sum(frac) == 1.0);

    /* Limit the total number of assets */
    prob.addConstraint(sum(buy) <= MAXNUM);

    /* Linking the variables */
    prob.addConstraints(NSHARES, [&](auto i) { return frac[i] <= buy[i]; });

    /* Objective: maximize total return */
    prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);
  }

  /** Solve resident model with a heuristic. */
  void solveHeuristic() {
    /* Disable automatic cuts */
    prob.controls.setCutStrategy(XPRS_CUTSTRATEGY_NONE);
    // Switch presolve off
    prob.controls.setPresolve(XPRS_PRESOLVE_NONE);
    prob.controls.setNodePresolve(NodePresolve::Off);
    /* Get feasibility tolerance */
    double tol = prob.controls.getFeasTol();

    /* Solve the LP-problem */
    prob.lpOptimize();

    /* Get Solution */
    auto sol = prob.getSolution();

    /* Basis information */
    std::vector<int> rowstat(prob.attributes.getRows());
    std::vector<int> colstat(prob.attributes.getCols());
    /* Save the current basis */
    prob.getBasis(rowstat, colstat);

    /*
     * Fix all variables `buy' for which `frac' is at 0 or at a relatively large
     * value
     */
    std::vector<double> fsol(NSHARES);
    for (int i = 0; i < NSHARES; ++i) {
      /* Get the solution values of `frac' */
      fsol[i] = frac[i].getValue(sol);
      if (fsol[i] < tol)
        buy[i].fix(0);
      else if (fsol[i] > 0.2 - tol)
        buy[i].fix(1);
    }

    /* Solve with the new bounds on 'buy' */
    prob.mipOptimize();

    printProblemStatus();
    printProblemSolution("Heuristic solution");

    /* Reset variables to their original bounds */
    for (int i = 0; i < NSHARES; ++i) {
      if ((fsol[i] < tol) || (fsol[i] > 0.2 - tol)) {
        buy[i].setLB(0);
        buy[i].setUB(1);
      }
    }

    /* Load basis */
    prob.loadBasis(rowstat, colstat);
  }

  /** Solve resident model with optimizer. */
  void optimize() { prob.optimize(); }
};

int main() {
  FolioHeuristic heur;
  heur.model();
  heur.solveHeuristic();

  FolioHeuristic exact;
  exact.model();
  exact.optimize();
  /* Solution printing */
  exact.printProblemStatus();
  exact.printProblemSolution("Exact Solve");

  return 0;
}

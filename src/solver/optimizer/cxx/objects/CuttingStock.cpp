// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;

/**
 * Cutting stock problem. The problem is solved by column (= cutting pattern)
 * generation heuristic looping over the root node.
 */

class CuttingStock {
public:
  const std::vector<double> width = {17, 21, 22.5, 24, 29.5}; // Possible widths
  const std::vector<int> demand = {150, 96, 48, 108, 227}; // Demand per width

  XpressProblem &prob;

  std::vector<Variable> patternVars; // Nr of rolls made with vPattern[i]
  std::vector<Inequality> demandConstraints; // 5 demand constraints
  LinTermMap objExpr;                        // Objective function

  CuttingStock(XpressProblem &prob) : prob(prob){}; // Class constructor
  void printSolution(); // To print the solution to console
  void modelCutStock(); // Initial problem definition with few variables
  void
  solveCutStock(); // Iteratively adds variables (i.e. columns) until optimality

private:
  const int NWIDTHS = 5;   // Number of unique widths to produce
  const int MAXWIDTH = 94; // Length of each roll / raw material
  const int MAXCOL = 10;   // Stop after generating 10 extra columns
  const double EPS = 1e-6; // Epsilon, for some inprecision tolerance
};

void CuttingStock::printSolution() {
  std::vector<double> sol = prob.getSolution();
  std::cout << "Objective Value: " << prob.attributes.getObjVal() << std::endl;
  std::cout << "Variables:" << std::endl << "\t";
  std::ios_base::fmtflags oldFlags(std::cout.flags());
  std::cout << std::fixed << std::setprecision(2);
  for (Variable v : prob.getVariables()) {

    std::cout << "[" << v.getName() << ": " << v.getValue(sol) << "]";
  }
  std::cout.flags(oldFlags);
  std::cout << std::endl;
}

/* ************************************************************************ */
/*  Integer Knapsack Algorithm for solving the integer knapsack problem     */
/*     z = max{cx : ax <= R, x <= d, x in Z^N}                              */
/*                                                                          */
/*  Input data:                                                             */
/*    N:        Number of item types                                        */
/*    c[i]:     Unit profit of item type i, i=1..n                          */
/*    a[i]:     Unit resource use of item type i, i=1..n                    */
/*    R:        Total resource available                                    */
/*    d[i]:     Demand for item type i, i=1..n                              */
/*  Return values:                                                          */
/*    xbest[i]: Number of items of type i used in optimal solution          */
/*    zbest:    Value of optimal solution                                   */
/* ************************************************************************ */
double solveKnapsack(int N, std::vector<double> c, std::vector<double> a,
                     double R, std::vector<int> d, std::vector<int> &xbest) {

  XpressProblem subProb;
  std::vector<Variable> x =
      subProb
          .addVariables(N) // Variable for each unique width to produce
          .withType(ColumnType::Integer)
          .withName("x_%d")
          .withUB([&](int i) { return double(d[i]); })
          .toArray();

  subProb.addConstraint(scalarProduct(x, a) <= R);

  subProb.setObjective(scalarProduct(x, c), ObjSense::Maximize);

  subProb.optimize();
  // Check the solution status
  if (subProb.attributes.getSolStatus() != SolStatus::Optimal &&
      subProb.attributes.getSolStatus() != SolStatus::Feasible) {
    std::ostringstream oss;
    oss << subProb.attributes
               .getSolStatus(); // Convert xpress::SolStatus to String
    throw std::runtime_error("Optimization of subproblem failed with status " +
                             oss.str());
  }

  double zbest = subProb.attributes.getObjVal();
  std::vector<double> sol = subProb.getSolution();

  // Return values
  for (int i = 0; i < N; i++)
    xbest[i] = int(std::round(x[i].getValue(sol)));
  return zbest;
}

// Function to initially model the cutting stock problem
void CuttingStock::modelCutStock() {
  // Generate NWIDTHS initial basic patterns. The basic patterns only use one
  // unique width i.e. (1,0,0,0,0), (0,2,0,0,0), (0,0,6,0,0), etc.
  int nrInitialPatterns = NWIDTHS;
  std::vector<std::vector<int>> patterns(NWIDTHS,
                                         std::vector<int>(nrInitialPatterns));
  for (int w = 0; w < NWIDTHS; w++) {
    // Here we generate the initial trivial patterns.
    patterns[w][w] = int(std::floor(double(MAXWIDTH) / width[w]));
  }

  patternVars =
      prob.addVariables(nrInitialPatterns)
          .withType(ColumnType::Integer)
          .withLB(0)
          .withUB([&](int p) {
            return std::ceil(double(demand[p]) / patterns[p][p]);
          })
          .withName([&](int p) { return xpress::format("pat_%d", p + 1); })
          .toArray();

  // Minimize total number of rolls
  std::for_each(patternVars.begin(), patternVars.end(),
                [&](Variable pattern) { objExpr.addTerm(pattern); });
  prob.setObjective(objExpr);

  // Satisfy the demand per width;
  // Sum of patterns producing width[w] must exceed demand[w]. The patterns p
  // producing the width[w] are exactly those p for which patterns[w][p] is
  // nonzero, so we can use scalarProduct
  xpress::Iterable<Inequality> constraintsIter =
      prob.addConstraints(NWIDTHS, [&](int w) {
        return (scalarProduct(patternVars, patterns[w]) >= demand[w])
            .setName("Demand_" + std::to_string(w));
      });
  // Save the Inequality objects for access later
  demandConstraints =
      std::vector<Inequality>(constraintsIter.begin(), constraintsIter.end());
}

/* ************************************************************************ */
/*   Column generation loop at the top node:                                */
/*     solve the LP-relaxation and save the basis                           */
/*     generate new column(s) (=cutting pattern)                            */
/*     add the new column to the main problem                               */
/*     load the modified problem and load the saved basis                   */
/* ************************************************************************ */
void CuttingStock::solveCutStock() {
  // Variable we will contain the return value of the knapsack subproblem
  // solution's x-variables
  std::vector<int> subProbSolX(NWIDTHS);

  std::cout << "Generating columns" << std::endl;
  for (int c = 0; c < MAXCOL; c++) {

    // Solve the LP-relaxation of the problem. (This is so we can get access to
    // dual values, as for MIPs dual values do not exist)
    prob.lpOptimize();

    // Check the solution status
    if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
        prob.attributes.getSolStatus() != SolStatus::Feasible) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Optimization failed with status " + oss.str());
    }

    // Save the current basis (so we can reload it for faster resolve)
    std::vector<int> rowstat(prob.attributes.getRows());
    std::vector<int> colstat(prob.attributes.getCols());
    prob.getBasis(rowstat, colstat);

    // Solve subproblem
    double z = solveKnapsack(NWIDTHS, prob.getDuals(demandConstraints), width,
                             MAXWIDTH, demand, subProbSolX);

    if (z < 1 + EPS) {
      std::cout << "  No profitable column found." << std::endl;
      break;
    } else {
      std::cout << "  New pattern found with marginal cost " << z - 1
                << std::endl;
      // Create a new variable for this pattern:
      Variable new_var = prob.addVariable(
          ColumnType::Integer,
          xpress::format("pat_%d", prob.attributes.getCols() + 1));

      // Add the basis-status of the new variable to colstat. The status=0 means
      // non-basic The new variable is non-basic since it has value 0 (i.e.
      // pattern is unused so far)
      colstat.push_back(0);

      // Add new var. to the objective
      new_var.chgObj(1.0);
      // Add new var. to demand constraints
      for (int i = 0; i < NWIDTHS; ++i) {
        if (subProbSolX[i] > 0) {
          demandConstraints[i].chgCoef(new_var, subProbSolX[i]);
        }
      }

      // Load back the previous optimal basis for faster resolve
      prob.loadBasis(rowstat, colstat);
    }
  }

  // Write the problem to .lp file
  prob.writeProb("CuttingStock.lp", "l");

  // After adding all the columns to the LP-relaxation, now solve the MIP
  std::cout << "Solving the final MIP problem" << std::endl;
  prob.mipOptimize();

  // Check the solution status
  if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
      prob.attributes.getSolStatus() != SolStatus::Feasible) {
    std::ostringstream oss;
    oss << prob.attributes
               .getSolStatus(); // Convert xpress::SolStatus to String
    throw std::runtime_error("Optimization failed with status " + oss.str());
  }
}

int main() {
  try {
    // Create a problem instance
    XpressProblem prob;

    // Output all messages
    prob.callbacks.addMessageCallback(XpressProblem::console);
    prob.controls.setOutputLog(0); // Disable outputlog

    // Model, Solve, and Print the cutting stock problem
    CuttingStock cutStockProb = CuttingStock(prob);
    cutStockProb.modelCutStock();
    cutStockProb.solveCutStock();
    cutStockProb.printSolution();
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

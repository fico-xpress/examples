// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a small QP problem to perform portfolio optimization. -- 1. QP:
 * minimize variance 2. MIQP: limited number of assets ---
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/** The file from which data for this example is read. */
char const *const DATAFILE = "foliocppqp.dat";
/* Target yield */
int const TARGET = 9;
/* Max. number of different assets */
int const MAXNUM = 4;
/* Number of shares */
int const NSHARES = 10;
/* Number of North-American shares */
int const NNA = 4;
/* Estimated return in investment */
std::vector<double> RET{5, 17, 26, 12, 8, 9, 7, 6, 31, 21};
/* Shares issued in N.-America */
std::vector<int> NA{0, 1, 2, 3};
/* Variance/covariance matrix of estimated returns */
std::vector<std::vector<double>> VAR;

void readData();

void printProblemStatus(XpressProblem const &prob) {
  std::cout << "Problem status:" << std::endl
            << "\tSolve status: " << prob.attributes.getSolveStatus()
            << std::endl
            << "\tSol status: " << prob.attributes.getSolStatus() << std::endl;
}

int main() {
  readData();
  XpressProblem prob;

  // Output all messages.
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /***** First problem: unlimited number of assets *****/

  /**** VARIABLES ****/
  std::vector<Variable> frac =
      prob.addVariables(NSHARES)
          /* Fraction of capital used per share */
          .withName("frac_%d")
          /* Upper bounds on the investment per share */
          .withUB(0.3)
          .toArray();

  /**** CONSTRAINTS ****/
  /* Minimum amount of North-American values */
  prob.addConstraint(sum(NNA, [&](auto i) { return frac[NA[i]]; }) >= 0.5);

  /* Spend all the capital */
  prob.addConstraint(sum(frac) == 1.0);

  /* Target yield */
  prob.addConstraint(scalarProduct(frac, RET) >= TARGET);

  /* Objective: minimize mean variance */
  Expression variance = sum(NSHARES, [&](auto s) {
    return sum(NSHARES, [&](auto t) { return VAR[s][t] * frac[s] * frac[t]; });
  });
  prob.setObjective(variance, ObjSense::Minimize);

  /* Solve */
  prob.optimize();

  /* Solution printing */
  printProblemStatus(prob);
  std::cout << "With a target of " << TARGET << " minimum variance is "
            << prob.attributes.getObjVal() << std::endl;
  auto sollp = prob.getSolution();
  for (int i = 0; i < NSHARES; ++i)
    std::cout << frac[i].getName() << " : " << 100.0 * frac[i].getValue(sollp)
              << "%" << std::endl;

  /***** Second problem: limit total number of assets *****/
  std::vector<Variable> buy = prob.addVariables(NSHARES)
                                  /* Fraction of capital used per share */
                                  .withName("buy_%d")
                                  .withType(ColumnType::Binary)
                                  .toArray();

  /* Limit the total number of assets */
  prob.addConstraint(sum(buy) <= MAXNUM);

  /* Linking the variables */
  /* frac .<= buy */
  prob.addConstraints(NSHARES, [&](auto i) { return frac[i] <= buy[i]; });

prob.controls.setThreads(8);

  /* Solve */
  prob.optimize();

  /* Solution printing */
  printProblemStatus(prob);
  std::cout << "With a target of " << TARGET << " and at most " << MAXNUM
            << " assets"
            << ", minimum variance is " << prob.attributes.getObjVal()
            << std::endl;
  auto solmip = prob.getSolution();
  for (int i = 0; i < NSHARES; ++i)
    std::cout << frac[i].getName() << " : " << 100.0 * frac[i].getValue(solmip)
              << "%" << std::endl;
  return 0;
}

#include <fstream>
#include <sstream>
#include <cctype>
void readData() {
  std::string dataDir("../../data");
#ifdef _WIN32
  size_t len;
  char buffer[1024];
  if ( !getenv_s(&len, buffer, sizeof(buffer), "EXAMPLE_DATA_DIR") &&
       len && len < sizeof(buffer) )
    dataDir = buffer;
#else
  char const *envDir = std::getenv("EXAMPLE_DATA_DIR");
  if (envDir)
    dataDir = envDir;
#endif
  std::string dataFile = dataDir + "/" + DATAFILE;
  std::ifstream ifs(dataFile);
  if (!ifs)
    throw std::runtime_error("Could not open " + dataFile);

  VAR = std::vector<std::vector<double>>(NSHARES, std::vector<double>(NSHARES));

  std::string line;
  int row = 0;
  while (std::getline(ifs, line)) {
    // Strip comments (comments start with a '!')
    std::string::size_type comment = line.find('!');
    if (comment != std::string::npos)
      line = line.substr(0, comment);
    // Remove leading whitespace
    line.erase(line.begin(),
               std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                 return !std::isspace(ch);
               }));
    // skip empty lines
    if (line.size() == 0)
      continue;
    // Now read NSHARES values into the current row in VAR
    std::stringstream s(line);
    for (int i = 0; i < NSHARES; ++i)
      s >> VAR[row][i];
    ++row;
  }
}

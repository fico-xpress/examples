// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a small QCQP problem to perform portfolio optimization. -- Maximize
 * return with limit on variance ---
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/** The file from which data for this example is read. */
char const *const DATAFILE = "/foliocppqp.dat";
/** Max. allowed variance */
double const MAXVAR = 0.55;
/** Number of shares */
int const NSHARES = 10;
/** Number of North-American shares */
int const NNA = 4;
/** Estimated return in investment */
std::vector<double> const RET{5, 17, 26, 12, 8, 9, 7, 6, 31, 21};
/** Shares issued in N.-America */
std::vector<int> const NA{0, 1, 2, 3};
/** Variance/covariance matrix of estimated returns */
std::vector<std::vector<double>> VAR;

void printProblemStatus(XpressProblem const &prob) {
  std::cout << "Problem status:" << std::endl
            << "\tSolve status: " << prob.attributes.getSolveStatus()
            << std::endl
            << "\tLP status: " << prob.attributes.getLpStatus() << std::endl
            << "\tMIP status: " << prob.attributes.getMipStatus() << std::endl
            << "\tSol status: " << prob.attributes.getSolStatus() << std::endl;
}

void readData();

int main() {
  readData();
  XpressProblem prob;
  // Output all messages.
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /**** VARIABLES ****/
  // Fraction of capital used per share
  std::vector<Variable> frac =
      prob.addVariables(NSHARES)
          .withName("frac_%d")
          /* Upper bounds on the investment per share */
          .withUB(0.3)
          .toArray();

  /**** CONSTRAINTS ****/
  // Limit variance
  Expression variance = sum(NSHARES, [&](auto s) {
    return sum(NSHARES, [&](auto t) {
      // v * fs * ft
      return VAR[s][t] * frac[s] * frac[t];
    });
  });
  prob.addConstraint(variance <= MAXVAR).setName("Variance");

  // Minimum amount of North-American values
  prob.addConstraint(sum(NNA, [&](auto i) { return frac[NA[i]]; }) >= 0.5)
      .setName("NA");

  // Spend all the capital
  prob.addConstraint(sum(frac) == 1.0).setName("Cap");

  // Objective: maximize total return
  prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);

  // Solve
  prob.optimize();

  // Solution printing
  printProblemStatus(prob);
  std::cout << "With of max variance of " << MAXVAR << " total return is "
            << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (int i = 0; i < NSHARES; ++i)
    std::cout << frac[i].getName() << " : " << (100.0 * frac[i].getValue(sol))
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

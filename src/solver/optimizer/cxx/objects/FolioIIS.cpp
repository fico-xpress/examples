// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a MIP problem to perform portfolio optimization.
 * Used infeasible model parameter values and illustrates retrieving IIS.
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/** The file from which data for this example is read. */
char const *const DATAFILE = "folio10.cdat";

int const MAXNUM = 5;           /* Max. number of different assets */
double const MAXRISK = 1.0 / 3; /* Max. investment into high-risk values */
double const MINREG = 0.1;      /* Min. investment per geogr. region */
double const MAXREG = 0.2;      /* Max. investment per geogr. region */
double const MAXSEC = 0.1;      /* Max. investment per ind. sector */
double const MAXVAL = 0.2;      /* Max. investment per share */
double const MINVAL = 0.1;      /* Min. investment per share */

std::vector<double> RET;            /* Estimated return in investment */
std::vector<int> RISK;              /* High-risk values among shares */
std::vector<std::vector<bool>> LOC; /* Geogr. region of shares */
std::vector<std::vector<bool>> SEC; /* Industry sector of shares */

std::vector<std::string> SHARES;
std::vector<std::string> REGIONS;
std::vector<std::string> TYPES;

void readData();

void printProblemStatus(XpressProblem const &prob) {
  std::cout << "Problem status:" << std::endl
            << "\tSolve status: " << prob.attributes.getSolveStatus()
            << std::endl
            << "\tLP status: " << prob.attributes.getLpStatus() << std::endl
            << "\tSol status: " << prob.attributes.getSolStatus() << std::endl;
}

int main(void) {
  readData();
  XpressProblem prob;
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /**** VARIABLES ****/
  /* Fraction of capital used per share */
  std::vector<Variable> frac =
      prob.addVariables(SHARES.size())
          /* Fraction of capital used per share */
          .withName([&](auto i) { return "frac" + SHARES[i]; })
          /* Upper bounds on the investment per share */
          .withUB(MAXVAL)
          .toArray();

  /* 1 if asset is in portfolio, 0 otherwise */
  std::vector<Variable> buy =
      prob.addVariables(SHARES.size())
          .withName([&](auto i) { return "buy_" + SHARES[i]; })
          .withType(ColumnType::Binary)
          .toArray();

  /**** CONSTRAINTS ****/
  /* Limit the percentage of high-risk values */
  prob.addConstraint(
      (sum(RISK.size(), [&](auto i) { return frac[RISK[i]]; }) <= MAXRISK)
          .setName("Risk"));

  /* Limits on geographical distribution */
  for (unsigned r = 0; r < REGIONS.size(); ++r) {
    Expression MinReg = sum(SHARES.size(), [&](auto s) {
      return (LOC[r][s] ? 1.0 : 0.0) * frac[s];
    });
    Expression MaxReg = sum(SHARES.size(), [&](auto s) {
      return (LOC[r][s] ? 1.0 : 0.0) * frac[s];
    });
    prob.addConstraint(
        (MinReg <= MINREG).setName("MinReg(" + REGIONS[r] + ")"));
    prob.addConstraint(
        (MaxReg <= MAXREG).setName("MaxReg(" + REGIONS[r] + ")"));
  }

  /* Diversification across industry sectors */
  for (unsigned t = 0; t < TYPES.size(); ++t) {
    Expression LimSec = sum(SHARES.size(), [&](auto s) {
      return (SEC[t][s] ? 1.0 : 0.0) * frac[s];
    });
    prob.addConstraint((LimSec <= MAXSEC).setName("LimSec(" + TYPES[t] + ")"));
  }

  /* Spend all the capital */
  prob.addConstraint((sum(frac) == 1.0).setName("Cap"));

  /* Limit the total number of assets */
  prob.addConstraint((sum(buy) >= MAXNUM).setName("MaxAssets"));

  /* Linking the variables */
  prob.addConstraints(SHARES.size(), [&](auto i) {
    return (frac[i] >= MINVAL * buy[i]).setName("link_lb_" + std::to_string(i));
  });
  prob.addConstraints(SHARES.size(), [&](auto i) {
    return (frac[i] <= MAXVAL * buy[i])
        .setName("link_ub_%d" + std::to_string(i));
  });

  /* Objective: maximize total return */
  prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);

  /* Solve */
  prob.optimize();

  /* Solution printing */
  printProblemStatus(prob);
  if (prob.attributes.getSolStatus() == SolStatus::Infeasible) {
    std::cout << "LP infeasible. Retrieving IIS." << std::endl;
    // Check there is at least one IIS
    int status = prob.firstIIS(1);
    if (status != 0)
      throw std::runtime_error("firstIIS() failed with status " +
                               std::to_string(status));
    int iisIndex = 1; // First IIS has index 1
    do {
      XPRSProblem::IISStatusInfo info = prob.IISStatus();
      std::cout << "IIS has " << info.rowsizes[iisIndex] << " constraints"
                << " and " << info.colsizes[iisIndex] << " columns"
                << ", " << info.numinfeas[iisIndex] << " infeasibilities"
                << " with an infeasibility of " << info.suminfeas[iisIndex]
                << std::endl;
      IIS data = prob.getIIS(iisIndex);
      std::cout << "Variables in IIS:" << std::endl;
      for (IISVariable const &v : data.getVariables()) {
        // Note that the IISVariable class has more fields than
        // we print here. See the reference documentation for
        // details.
        std::cout << "\t" << v.getVariable().getName() << " " << v.getDomain()
                  << std::endl;
      }
      std::cout << "Constraints in IIS:" << std::endl;
      for (IISConstraint const &c : data.getConstraints()) {
        // Note that the IISVariable class has more fields than
        // we print here. See the reference documentation for
        // details.
        std::cout << "\t" << std::get<Inequality>(c.getConstraint()).getName()
                  << std::endl;
      }
      ++iisIndex; // Prepare for next IIS (if any)
    } while (prob.nextIIS() == 0);
  }

  return 0;
}

// Minimalistic data parsing.
#include <fstream>
#include <iterator>

/**
 * Read a list of strings. Iterates <code>it</code> until a semicolon is
 * encountered or the iterator ends.
 *
 * @param it The token sequence to read.
 * @param conv  Function that converts a string to <code>T</code>.
 * @return A vector of all tokens before the first semicolon.
 */
template <typename T>
std::vector<T> readStrings(std::istream_iterator<std::string> &it,
                           std::function<T(std::string const &)> conv) {
  std::vector<T> result;
  while (it != std::istream_iterator<std::string>()) {
    std::string token = *it++;
    if (token.size() > 0 && token[token.size() - 1] == ';') {
      if (token.size() > 1) {
        result.push_back(conv(token.substr(0, token.size() - 1)));
      }
      break;
    } else {
      result.push_back(conv(token));
    }
  }
  return result;
}

/**
 * Read a sparse table of booleans. Allocates a <code>nrow</code> by
 * <code>ncol</code> boolean table and fills it by the sparse data from the
 * token sequence. <code>it</code> is assumed to hold <code>nrow</code>
 * sequences of indices, each of which is terminated by a semicolon. The indices
 * in those vectors specify the <code>true</code> entries in the corresponding
 * row of the table.
 *
 * @tparam R     Type of row count.
 * @tparam C     Type of column count.
 * @param it     Token sequence.
 * @param nrow   Number of rows in the table.
 * @param ncol   Number of columns in the table.
 * @return The boolean table.
 */
template<typename R,typename C>
std::vector<std::vector<bool>>
readBoolTable(std::istream_iterator<std::string> &it, R nrow, C ncol) {
  std::vector<std::vector<bool>> tbl(nrow, std::vector<bool>(ncol));
  for (R r = 0; r < nrow; r++) {
    for (auto i : readStrings<int>(it, [](auto &s) { return stoi(s); }))
      tbl[r][i] = true;
  }
  return tbl;
}

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
  std::stringstream data(std::string((std::istreambuf_iterator<char>(ifs)),
                                     (std::istreambuf_iterator<char>())));
  std::istream_iterator<std::string> it(data);
  while (it != std::istream_iterator<std::string>()) {
    std::string token = *it++;
    if (token == "SHARES:")
      SHARES = readStrings<std::string>(it, [](auto &s) { return s; });
    else if (token == "REGIONS:")
      REGIONS = readStrings<std::string>(it, [](auto &s) { return s; });
    else if (token == "TYPES:")
      TYPES = readStrings<std::string>(it, [](auto &s) { return s; });
    else if (token == "RISK:")
      RISK = readStrings<int>(it, [](auto &s) { return stoi(s); });
    else if (token == "RET:")
      RET = readStrings<double>(it, [](auto &s) { return stod(s); });
    else if (token == "LOC:")
      LOC = readBoolTable(it, REGIONS.size(), SHARES.size());
    else if (token == "SEC:")
      SEC = readBoolTable(it, TYPES.size(), SHARES.size());
  }
}

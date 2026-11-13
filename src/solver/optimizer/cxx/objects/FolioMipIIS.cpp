// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a MIP problem to perform portfolio optimization.
 * Same model as in FolioMip3.java.
 * Uses infeasible model parameter values and illustrates retrieving MIIS.
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
double const MAXRISK = 1.0 / 4; /* Max. investment into high-risk values */
double const MINREG = 0.1;      /* Min. investment per geogr. region */
double const MAXREG = 0.25;     /* Max. investment per geogr. region */
double const MAXSEC = 0.15;     /* Max. investment per ind. sector */
double const MAXVAL = 0.225;    /* Max. investment per share */
double const MINVAL = 0.1;      /* Min. investment per share */

std::vector<double> RET;            /* Estimated return in investment */
std::vector<int> RISK;              /* High-risk values among shares */
std::vector<std::vector<bool>> LOC; /* Geogr. region of shares */
std::vector<std::vector<bool>> SEC; /* Industry sector of shares */

std::vector<std::string> SHARES;
std::vector<std::string> REGIONS;
std::vector<std::string> TYPES;

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
  /* Fraction of capital used per share */
  std::vector<Variable> frac =
      prob.addVariables(SHARES.size())
          .withName("frac_%d")
          /* Upper bounds on the investment per share */
          .withLB(0.0)
          .withUB(MAXVAL)
          .toArray();

  std::vector<Variable> buy = prob.addVariables(SHARES.size())
                                  .withName("buy_%d")
                                  .withType(ColumnType::Binary)
                                  .toArray();

  /**** CONSTRAINTS ****/
  /* Limit the percentage of high-risk values */
  prob.addConstraint(sum(RISK.size(),
                         [&](auto i) { return frac[RISK[i]]; }) <=
                     MAXRISK)
      .setName("Risk");

  /* Limits on geographical distribution */
  prob.addConstraints(REGIONS.size(), [&](auto r) {
    return sum(SHARES.size(),
               [&](auto s) { return (LOC[r][s] ? 1.0 : 0.0) * frac[s]; })
        .in(MINREG, MAXREG)
        .setName("MinMaxReg_" + REGIONS[r]);
  });

  /* Diversification across industry sectors */
  prob.addConstraints(TYPES.size(), [&](auto t) {
    return sum(SHARES.size(),
               [&](auto s) { return (SEC[t][s] ? 1.0 : 0.0) * frac[s]; })
        .in(0.0, MAXSEC)
        .setName("LimSec(" + TYPES[t] + ")");
  });

  /* Spend all the capital */
  prob.addConstraint(sum(frac) == 1.0).setName("Cap");

  /* Limit the total number of assets */
  prob.addConstraint(sum(buy) <= MAXNUM).setName("MaxAssets");

  /* Linking the variables */
  prob.addConstraints(SHARES.size(), [&](auto i) {
    return (frac[i] >= MINVAL * buy[i]).setName("link_lb_" + std::to_string(i));
  });
  prob.addConstraints(SHARES.size(), [&](auto i) {
    return (frac[i] <= MAXVAL * buy[i]).setName("link_ub_" + std::to_string(i));
  });

  /* Objective: maximize total return */
  prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);

  /* Solve */
  prob.optimize();

  /* Solution printing */
  printProblemStatus(prob);
  if (prob.attributes.getSolStatus() == SolStatus::Infeasible) {
    std::cout << "MIP infeasible. Retrieving IIS." << std::endl;
    // Check there is at least one IIS
    int status = prob.firstIIS(1);
    if (status != 0)
      throw std::runtime_error("firstIIS() failed with status " +
                               std::to_string(status));
    XPRSProblem::IISStatusInfo info = prob.IISStatus();
    std::cout << "IIS has " << info.rowsizes[1] << " constraints and "
              << info.colsizes[1] << " columns, " << info.numinfeas[1]
              << " infeasibilities with an infeasibility of "
              << info.suminfeas[1] << std::endl;
    IIS data = prob.getIIS(1);
    std::cout << "Variables in IIS:" << std::endl;
    for (auto &v : data.getVariables()) {
      // Note that the IISVariable class has more fields than
      // we print here. See the reference documentation for
      // details.
      std::cout << "\t" << v.getVariable().getName() << " " << v.getDomain()
                << std::endl;
    }
    std::cout << "Constraints in IIS:" << std::endl;
    for (auto &c : data.getConstraints()) {
      // Note that the IISVariable class has more fields than
      // we print here. See the reference documentation for
      // details.
      std::cout << "\t" << std::get<Inequality>(c.getConstraint()).getName()
                << std::endl;
    }
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

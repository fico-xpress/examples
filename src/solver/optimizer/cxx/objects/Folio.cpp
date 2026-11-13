// (c) 2024-2026 Fair Isaac Corporation

/**
 * Modeling a MIP problem to perform portfolio optimization. <br>
 * There are a number of shares available in which to invest. The problem is to
 * split the available capital between these shares to maximize return on
 * investment, while satisfying certain constraints on the portfolio:
 * <ul>
 * <li>A maximum of 7 distinct shares can be invest in.</li>
 * <li>Each share has an associated industry sector and geographic region, and
 * the capital may not be overly concentrated in any one sector or region.</li>
 * <li>Some of the shares are considered to be high-risk, and the maximum
 * investment in these shares is limited.</li>
 * </ul>
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/** The file from which data for this example is read. */
char const *const DATAFILE = "folio10.cdat";

int const MAXNUM = 7;           /* Max. number of different assets */
double const MAXRISK = 1.0 / 3; /* Max. investment into high-risk values */
double const MINREG = 0.2;      /* Min. investment per geogr. region */
double const MAXREG = 0.5;      /* Max. investment per geogr. region */
double const MAXSEC = 0.25;     /* Max. investment per ind. sector */
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

int main() {
  readData(); // Read data from file

  XpressProblem prob;

  // Create the decision variables
  // Fraction of capital used per share
  std::vector<Variable> frac = prob.addVariables(SHARES.size())
                                   .withUB(MAXVAL)
                                   .withName("frac %d")
                                   .toArray();
  // 1 if asset is in portfolio, 0 otherwise
  std::vector<Variable> buy = prob.addVariables(SHARES.size())
                                  .withType(ColumnType::Binary)
                                  .withName("buy %d")
                                  .toArray();

  // Objective: total return
  prob.setObjective(scalarProduct(frac, RET), ObjSense::Maximize);

  // Limit the percentage of high-risk values
  prob.addConstraint(sum(RISK, [&](auto v) { return frac[v]; }) <= MAXRISK);

  // Limits on geographical distribution
  prob.addConstraints(REGIONS.size(), [&](auto r) {
    return sum(SHARES.size(),
               [&](auto s) { return (LOC[r][s] ? 1.0 : 0.0) * frac[s]; })
        .in(MINREG, MAXREG);
  });

  // Diversification across industry sectors
  prob.addConstraints(TYPES.size(), [&](auto t) {
    return sum(SHARES.size(), [&](auto s) {
             return (SEC[t][s] ? 1.0 : 0.0) * frac[s];
           }) <= MAXSEC;
  });

  // Spend all the capital
  prob.addConstraint(sum(frac) == 1);

  // Limit the total number of assets
  prob.addConstraint(sum(buy) <= MAXNUM);

  // Linking the variables
  for (unsigned s = 0; s < SHARES.size(); s++) {
    prob.addConstraint(frac[s] <= MAXVAL * buy[s]);
    prob.addConstraint(frac[s] >= MINVAL * buy[s]);
  }

  // Set a time limit of 10 seconds
  prob.controls.setTimeLimit(10.0);

  // Solve the problem
  prob.optimize("");

  std::cout << "Problem status: " << prob.attributes.getMipStatus()
            << std::endl;
  if (prob.attributes.getMipStatus() != MIPStatus::Solution &&
      prob.attributes.getMipStatus() != MIPStatus::Optimal)
    throw std::runtime_error("optimization failed with status " +
                             to_string(prob.attributes.getMipStatus()));

  // Solution printing
  std::cout << "Total return: " << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (unsigned s = 0; s < SHARES.size(); s++)
    if (buy[s].getValue(sol) > 0.5)
      std::cout << "  " << s << ": " << frac[s].getValue(sol) * 100 << "% ("
                << buy[s].getValue(sol) << ")" << std::endl;
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

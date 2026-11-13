// (c) 2024-2026 Fair Isaac Corporation

/**
 * Purchasing problem with price breaks using PieceWise Linear constraints.
 * There are three suppliers of a good, and they have quoted various prices for
 * various quantities of product. We want to buy at least total cost, yet not
 * buy too much from any one supplier. Each supplier offers decreasing prices
 * for increased lot size, in the form of incremental discounts.
 */

#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

void readData();

/** The file from which data for this example is read. */
char const *const DATAFILE = "purchase.cdat";

int const NB = 4;            /* Number of breakpoints */
double REQ;                  /* Total quantity required */
std::vector<int> SUPPLIERS;  /* Suppliers */
std::vector<double> MAXPERC; /* Maximum percentages for each supplier */
std::vector<std::vector<double>> COST; /* Unit cost */
std::vector<std::vector<double>>
    BREAKP; /* Breakpoints (quantities at which unit cost changes) */

int main() {
  readData(); // Read data from file

  XpressProblem prob;

  // Create the decision variables
  // Quantity to purchase from supplier s
  std::vector<Variable> buy =
      prob.addVariables(SUPPLIERS.size())
          .withUB([&](auto i) { return MAXPERC[i] * REQ / 100.0; })
          .withName("buy %d")
          .toArray();

  // Cost incurred from supplier s
  std::vector<Variable> cost =
      prob.addVariables(SUPPLIERS.size())
          .withName("cost %d")
          .toArray();

  // The minimum quantity that must be bought
  prob.addConstraint(sum(buy) >= REQ);

  // Function to calculate cost at breakpoints
  std::vector<std::vector<double>> COSTBREAK(SUPPLIERS.size(),
                                             std::vector<double>(NB));
  for (unsigned s = 0; s < SUPPLIERS.size(); s++) {
    COSTBREAK[s][0] = 0;
    for (int b = 1; b < NB; b++)
      COSTBREAK[s][b] =
          COSTBREAK[s][b - 1] + COST[s][b] * (BREAKP[s][b] - BREAKP[s][b - 1]);
  }

  // Define relation between bought quantities and price paid per supplier
  for (unsigned s = 0; s < SUPPLIERS.size(); s++) {
    prob.addConstraint(cost[s].pwlOf(buy[s], BREAKP[s], COSTBREAK[s]));
  }

  // Objective: sum of costs
  prob.setObjective(sum(cost));

  // Solve the problem
  prob.optimize("");

  std::cout << "Problem status: " << prob.attributes.getMipStatus()
            << std::endl;
  if (prob.attributes.getMipStatus() != MIPStatus::Solution &&
      prob.attributes.getMipStatus() != MIPStatus::Optimal)
    throw std::runtime_error("optimization failed with status " +
                             to_string(prob.attributes.getMipStatus()));

  // Solution printing
  std::cout << "Total cost: " << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (unsigned s = 0; s < SUPPLIERS.size(); s++)
    std::cout << "Supp. " << SUPPLIERS[s] << ": buy:" << buy[s].getValue(sol)
              << ": cost:" << cost[s].getValue(sol) << std::endl;

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
 * Read a table of doubles. Allocates a <code>nrow</code> by <code>ncol</code>
 * double table and fills it by the sparse data from the token sequence.
 * <code>it</code> is assumed to hold <code>nrow</code> sequences of
 * indices, each of which is terminated by a semicolon. The indices in those
 * vectors specify the entries in the corresponding row of the
 * table.
 *
 * @tparam R     Type for row count.
 * @paraam C     Type for column count.
 * @param it     Token sequence.
 * @param nrow   Number of rows in the table.
 * @param ncol   Number of columns in the table.
 * @return The double table.
 */
template<typename R,typename C>
std::vector<std::vector<double>>
readTable(std::istream_iterator<std::string> &it, R nrow, C ncol) {
  std::vector<std::vector<double>> tbl(nrow);
  for (R r = 0; r < nrow; r++) {
    tbl[r] = readStrings<double>(it, [](auto &s) { return std::stod(s); });
    if (static_cast<C>(tbl[r].size()) != ncol)
      throw std::runtime_error("not enough columns in file");
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
    if (token == "SUPPLIERS:")
      SUPPLIERS = readStrings<int>(it, [](auto &s) { return std::stoi(s); });
    else if (token == "MAXPERC:")
      MAXPERC = readStrings<double>(it, [](auto &s) { return std::stod(s); });
    else if (token == "REQ:")
      REQ = std::stod(*it++);
    else if (token == "BREAKP:")
      BREAKP = readTable(it, SUPPLIERS.size(), NB);
    else if (token == "COST:")
      COST = readTable(it, SUPPLIERS.size(), NB);
  }
}

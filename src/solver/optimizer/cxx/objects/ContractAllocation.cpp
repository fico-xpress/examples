// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/* Shows the use of a matrix of variables (i.e. x[i][j]) and different ways to
 * specify contraints*/

/* Contract Allocation example. Given a number of districts, we have to produce
 * enough of a certain resource to fulfill all contracts. Each district has a
 * maximum amount of resource they can produce. Minimize costs to fulfill
 * contracts.
 */

const int NDISTRICT = 6;  // Number of districts
const int NCONTRACT = 10; // Number of contracts

const std::vector<double> output = {50, 40, 10,
                                    20, 70, 50}; // Max. output per district
const std::vector<double> cost = {50, 20, 25,
                                  30, 45, 40}; // Cost per unit per district
// Required volume of resources by contracts:
const std::vector<double> volume = {20, 10, 30, 15, 20, 30, 10, 50, 10, 20};

int main() {
  try {
    // Create a problem instance with verbose messages printed to Console
    XpressProblem prob;
    prob.callbacks.addMessageCallback(XpressProblem::console);

    /* VARIABLES */

    // Variables indicating whether a project is chosen
    std::vector<std::vector<Variable>> x =
        prob.addVariables(NDISTRICT, NCONTRACT)
            .withType(ColumnType::Binary)
            .withName("x_d%d_c%d")
            .toArray();

    // Quantities allocated to contractors
    std::vector<std::vector<Variable>> q =
        prob.addVariables(NDISTRICT, NCONTRACT)
            .withType(ColumnType::SemiContinuous)
            .withUB([&](int d, int) { return output[d]; })
            .withLimit(5)
            .withName("q_d%d_c%d")
            .toArray();

    /* CONSTRAINTS */

    // "Size": Produce the required volume of resource for each contract
    // for all c in [0,NCONTRACT]
    //      sum(d in [0,NDISTRICT]) q[d][c] >= volume[c]
    prob.addConstraints(NCONTRACT, [&](int c) {
      auto coveredVolume = sum(NDISTRICT, [&](int d) { return q[d][c]; });
      return coveredVolume.geq(volume[c]).setName("Size_" + std::to_string(c));
    });

    // "Min": at least 2 districts per contract
    // for all c in [0,NCONTRACT]
    //      sum(d in [0,NDISTRICT]) x[d][c] >= 2
    prob.addConstraints(NCONTRACT, [&](int c) {
      LinExpression districtsPerContract = LinExpression::create();
      for (int d = 0; d < NDISTRICT; d++) {
        districtsPerContract.addTerm(x[d][c]);
      }
      return districtsPerContract.geq(2.0).setName("Min_" + std::to_string(c));
    });

    // Do not exceed max. output
    // for all d in [0,NDISTRICT]
    //      sum(c in [0,NCONTRACT]) q[d][c] <= output[d]
    prob.addConstraints(NDISTRICT, [&](int d) {
      return (sum(q[d]) <= output[d]).setName("Output_" + std::to_string(d));
    });

    // If a contract is allocated to a district, then at least 1 unit is
    // allocated to it for all d in [0,NDISTRICT[
    //      for all c in [0,NCONTRACT[
    //           x[d][c] <= q[d][c]
    prob.addConstraints(NDISTRICT, NCONTRACT, [&](int d, int c) {
      return x[d][c].leq(q[d][c]).setName("XQ_" + std::to_string(d) + "_" +
                                          std::to_string(c));
    });

    /* OBJECTIVE */

    LinExpression obj = LinExpression::create();
    for (int c = 0; c < NCONTRACT; c++) {
      for (int d = 0; d < NDISTRICT; d++) {
        obj.addTerm(q[d][c], cost[d]);
      }
    }
    prob.setObjective(obj, ObjSense::Minimize);

    /* SOLVE & PRINT */

    prob.writeProb("Contract.lp", "l");
    prob.optimize();

    // Check the solution status
    if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
        prob.attributes.getSolStatus() != SolStatus::Feasible) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Optimization failed with status " + oss.str());
    }

    // Print the solution
    std::vector<double> sol = prob.getSolution();
    std::cout << "*** Solution ***" << std::endl;
    std::cout << "Objective value: " << prob.attributes.getObjVal()
              << std::endl;
    for (std::vector<Variable> q_d : q) {
      for (Variable q_dc : q_d) {
        if (q_dc.getValue(sol) > 0.0) {
          std::cout << q_dc.getName() << ": " << q_dc.getValue(sol) << ", ";
        }
      }
      std::cout << std::endl;
    }
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

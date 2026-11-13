// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/** Capital budgeting problem.
 * Illustrates logical conditions, formulation using logic constraints
 * and indicator constraints.
 */

// A project.
class Project {
public:
  const std::string name;  // The name of this project.
  const double payout;     // Payout for the project.
  const double investment; // Capital investment for the project.
  const int personnel;     // Number of personnel required for the project.

  Project(std::string name, double payout, double investment, int personnel)
      : name(name), payout(payout), investment(investment),
        personnel(personnel) {}

  // For printing
  std::string toString() const { return name; }
};

// The projects used in this example.
const std::vector<Project> projectArray = {
    Project("Alpha", 124000.0, 104000.0, 22),
    Project("Beta", 74000.0, 53000.0, 12),
    Project("Gamma", 42000.0, 29000.0, 7),
    Project("Delta", 188000.0, 187000.0, 36),
    Project("Epsilon", 108000.0, 98000.0, 24),
    Project("Zeta", 56000.0, 32000.0, 10),
    Project("Eta", 88000.0, 75000.0, 20),
    Project("Theta", 225000.0, 200000.0, 41)};

// The resource constraints used in this example.
const double budget = 478000.0;
const int workforce = 106;

int main() {
  try {
    // Create a problem instance with or without verbose messages printed to
    // Console
    XpressProblem prob;
    // prob.callbacks.addMessageCallback(XpressProblem::console);

    /* VARIABLES */

    // Whether each project should be invested in or not
    std::vector<Variable> x =
        prob.addVariables(projectArray.size())
            .withType(ColumnType::Binary)
            .withName("x%d")
            .toArray();

    /* RESOURCE AVAILABILITY CONSTRAINTS */

    // Investment limit: sum of investments of all undertaken projects should
    // not exceed budget
    Expression requiredInvestment =
        sum(projectArray.size(),
            [&](auto i) { return x[i].mul(projectArray[i].investment); });
    Inequality investmentLimit =
        prob.addConstraint(requiredInvestment <= budget);

    // Workforce limit: sum of personnel committed of all undertaken projects
    // should not exceed workforce
    LinExpression requiredWorkforce = LinExpression::create();
    for (std::size_t i = 0; i < projectArray.size(); i++) {
      requiredWorkforce.addTerm(x[i], projectArray[i].personnel);
    }
    Inequality workforceLimit =
        prob.addConstraint(requiredWorkforce <= workforce);

    // Project alpha can only be done if both gamma and zeta happen
    prob.addConstraint(x[0] <= x[2]);
    prob.addConstraint(x[0] <= x[5]);

    // Project zeta can only be done if project epsilon happens
    prob.addConstraint(x[5] <= x[4]);

    // Projects alpha and beta as well as gamma and delta can only happen
    // together
    prob.addConstraint(x[0] == x[1]);
    prob.addConstraint(x[2] == x[3]);

    // Exactly one of those pairs should be invested in, i.e., if project alpha
    // is performed, neither gamma nor delta can be invested in, and if project
    // alpha does not happen, then projects gamma and delta have to be performed
    prob.addConstraint(x[0].ifThen(sum(x[2], x[3]) == 0.0));
    prob.addConstraint(x[0].ifNotThen(sum(x[2], x[3]) == 2.0));

    /* OBJECTIVE */

    // Objective function: sum of payouts of all undertaken projects
    Expression totalProfit =
        sum(projectArray.size(),
            [&](auto i) { return x[i].mul(projectArray[i].payout); });
    prob.setObjective(totalProfit, xpress::ObjSense::Maximize);

    /* INSPECT, SOLVE & PRINT */

    // Dump the problem to disk so that we can inspect it.
    prob.writeProb("capbgt2l.lp");

    // Solve
    prob.optimize();

    // Check the solution status
    if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
        prob.attributes.getSolStatus() != SolStatus::Feasible) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Optimization failed with status " + oss.str());
    }

    // Get the solution and print it
    std::vector<double> sol = prob.getSolution();
    std::cout << std::endl << "*** Solution ***" << std::endl;
    std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;

    // Print the interesting slacks
    std::cout << "Remaining Budget: " << investmentLimit.getSlack()
              << " (out of " << budget << ")" << std::endl;
    std::cout << "Remaining Workers: " << workforceLimit.getSlack()
              << " (out of " << workforce << ")" << std::endl;

    // Print out the variables
    for (std::size_t i = 0; i < projectArray.size(); ++i) {
      if (x[i].getValue(sol) > 0.5) {
        std::cout << "Undertaking project " << projectArray[i].toString()
                  << std::endl;
      }
    }
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

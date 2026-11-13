// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/*
 * Capital budgeting example, showing multi-objective optimization.
 * The problem is solved using three multi-objective approaches:
 *  - Lexicographic approach, solving first to minimize capital expended and
 * second to maximize return
 *  - Blended approach, solving a weighted combination of both objectives
 * simultaneously
 *  - Lexicographic approach, with the objective priorities reversed
 */

class CapitalBudgeting {
public:
  // Required capital for each project
  const std::vector<double> CAPITAL = {104000, 53000, 29000, 187000,
                                       98000,  32000, 75000, 200000};
  // Required personnel for each project
  const std::vector<double> PERSONNEL = {22, 12, 7, 36, 24, 10, 20, 41};
  // Expected return of each project
  const std::vector<double> RETURN = {124000, 74000, 42000, 188000,
                                      108000, 56000, 88000, 225000};

  const double CAPITAL_TARGET = 478000; // Target capital to invest
  const double ROI_TARGET = 550000;     // Target return on investment
  const int PERSONNEL_MAX = 106;        // Available personnel

  CapitalBudgeting(){}; // Class constructor
  void model(); // Function to make variables, constraints, and objective
  void solveThreeTimes(); // To optimize with different variations of objectives
  void printSolution(std::string title); // To print the solution to console

private:
  XpressProblem prob;

  // Binary decision variables indicating which projects will be implemented
  std::vector<Variable> selectProject;

  Inequality personnel;  // Constraint for the number of personnel available
  LinExpression capital; // Primary objective: minimize capital expended
  LinExpression roi;     // Secondary objective: maximize return on investment
};

void CapitalBudgeting::model() {

  /* VARIABLES */

  // Binary decision variables indicating which projects will be implemented
  selectProject = prob.addVariables(CAPITAL.size())
                      .withType(ColumnType::Binary)
                      .withName("selectProject_%d")
                      .toArray();

  /* CONSTRAINTS */

  // Constraint: at least 3 projects must be implemented
  prob.addConstraint(sum(selectProject) >= 3);

  // Constraint for the number of personnel available
  personnel = prob.addConstraint(scalarProduct(selectProject, PERSONNEL) <=
                                 PERSONNEL_MAX);

  /* OBJECTIVES */

  // Primary objective: minimize capital expended
  capital = scalarProduct(selectProject, CAPITAL);
  prob.setObjective(capital, ObjSense::Minimize);

  // Secondary objective: maximize return on investment
  roi = scalarProduct(selectProject, RETURN);
  // We add the second objective with priority=0 (same as default given
  // to the first objective) and weight=-1 (to maximize this expression)
  prob.addObjective(roi, 0, -1);
};

void CapitalBudgeting::solveThreeTimes() {
  // Set the first objective (with id=0) to priority=1 to give higher priority
  // than 2nd objective
  prob.setObjIntControl(0, ObjControl::Priority, 1);
  // Optimize & print
  prob.optimize();
  printSolution("*** Higher priority for 'Minimize Capital' objective ***");

  // Now set the same priority for both objectives (i.e. we set the the second
  // objective (with id=1) to have priority=1). This will result in a single
  // solve using the weighted sum of the two objectives.
  prob.setObjIntControl(1, ObjControl::Priority, 1);
  // Optimize & print
  prob.optimize();
  printSolution("*** Equal priority for Both objectives ***");

  // Finally, give the first objective (with id=0) a lower priority (=0)
  prob.setObjIntControl(0, ObjControl::Priority, 0);
  // Optimize & print
  prob.optimize();
  printSolution("*** Higher priority for 'Maximize Return' objective ***");
}

void CapitalBudgeting::printSolution(std::string title) {
  std::cout << std::endl << title << std::endl;

  // Check the solution status
  if (prob.attributes.getSolveStatus() != SolveStatus::Completed) {
    std::cout << "Problem not solved" << std::endl;
  } else if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
             prob.attributes.getSolStatus() != SolStatus::Feasible) {
    throw std::runtime_error("Optimization failed with status " +
                             to_string(prob.attributes.getSolStatus()));
  }

  std::cout << "  Objectives: " << prob.attributes.getObjectives() << "\t";
  std::cout << "solved objectives: " << prob.attributes.getSolvedObjs()
            << std::endl;

  std::vector<double> sol = prob.getSolution();
  double capitalUsed = capital.evaluate(sol);
  double roiGained = roi.evaluate(sol);

  if (capitalUsed > CAPITAL_TARGET)
    std::cout << "  Unable to meet Capital target" << std::endl;
  if (roiGained < ROI_TARGET)
    std::cout << "  Unable to meet Return target" << std::endl;

  std::cout << "  Projects undertaken:";
  for (std::size_t p = 0; p < selectProject.size(); p++) {
    if (selectProject[p].getSolution() == 1.0) {
      std::cout << " " << p;
    }
  }
  std::cout << std::endl;

  std::cout << "  Used capital: $" << capitalUsed
            << (CAPITAL_TARGET >= capitalUsed ? " unused: $" : " overused: $")
            << std::abs(CAPITAL_TARGET - capitalUsed) << std::endl;
  std::cout << "  Total return: $" << roiGained << std::endl;
  std::cout << "  Unused personnel: " << int(personnel.getSlack()) << " persons"
            << std::endl;
};

int main() {
  try {
    CapitalBudgeting capbudget_problem;
    capbudget_problem.model();
    capbudget_problem.solveThreeTimes();
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

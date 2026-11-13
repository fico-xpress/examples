// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;

/*
 * Knapsack problem. This example shows how to setup a simple knapsack type
 * problem in which an item selection should be made maximally profitable under
 * some weight restriction. We first solve a pure knapsack problem. Then, we
 * add more constraints by explicitly forbidding certain item combinations,
 * thereby showing multiple ways how to create indicator constraints.
 */

std::vector<double> values = {15, 100, 90, 60, 40, 15, 10, 1};
std::vector<double> weights = {2, 20, 20, 30, 40, 30, 60, 10};
std::vector<std::string> names = {"camera", "necklace", "vase",  "picture",
                                  "tv",     "video",    "chest", "brick"};
int MAX_WEIGHT = 102;

int main() {
  try {
    // Create a problem instance with verbose messages printed to Console
    XpressProblem prob;
    prob.callbacks.addMessageCallback(XpressProblem::console);

    // Binary variables, 1 if we put item i in the knapsack, 0 if not
    std::vector<Variable> x = prob.addVariables(values.size())
                                  .withType(ColumnType::Binary)
                                  .withName([&](auto i) { return names[i]; })
                                  .toArray();

    // Weight restriction on the knapsack
    prob.addConstraint(scalarProduct(x, weights) <= double(MAX_WEIGHT));

    // Objective: Maximze total value
    prob.setObjective(scalarProduct(x, values), xpress::ObjSense::Maximize);

    // Optimize
    // prob.writeProb("BinBurglar.lp");
    prob.optimize();

    // Check the solution status
    if (prob.attributes.getSolStatus() != xpress::SolStatus::Optimal) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Problem not solved to optimality: " +
                               oss.str());
    }

    // Print out the solution
    std::cout << std::endl << "*** Solution ***" << std::endl;
    std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;
    std::vector<double> sol = prob.getSolution();
    for (Variable v : x)
      std::cout << v.getName() << " = " << v.getValue(sol) << std::endl;
    std::cout << std::endl;

    /*
     * Now to show some additionaly functionaly, we add constraints to consider
     * the items "vase" and "picture" as one pair and the items "tv" and "video"
     * as a second pair so that the pairs are always taken together
     */
    // Create a map (using for_each and lambda function) to facilitate easy look
    // up of the variables by name
    std::map<std::string, Variable> nameToVariableMap;
    std::for_each(x.begin(), x.end(), [&](Variable var) {
      nameToVariableMap[var.getName()] = var;
    });

    // Add constraint x["vase"] == x["picture"]
    prob.addConstraint(nameToVariableMap["vase"] ==
                       nameToVariableMap["picture"]);
    // Add constraint x["tv"] == x["video"]
    prob.addConstraint(nameToVariableMap["tv"] == nameToVariableMap["video"]);

    /*
     * Logical constraint: Take exactly one of the two pairs of variables (i.e.
     * take either the first pair "vase" and "picture" or the second pair "tv"
     * and "video", but not both pairs)
     */
    // x["vase"]=1 -> x["tv"]+x["video"]=0 */
    Variable vase = nameToVariableMap["vase"];
    Expression tvPlusVideo =
        nameToVariableMap["tv"] + nameToVariableMap["video"];
    prob.addConstraint(vase.ifThen(tvPlusVideo <= 0.0));

    // x["vase"]=0 -> x["tv"]+x["video"]=2
    // Instead of `vase.ifNotThen()`, we can use this alternative way to create
    // the implication constraint
    Inequality enablePairTwo = prob.addConstraint(tvPlusVideo >= 2.0);
    prob.setIndicator(vase, false, enablePairTwo);

    // By inspecting the problem, we can see the two methods of adding
    // implication constraints indeed yield similar constraint
    prob.writeProb("BinBurglar.lp");

    // Optimize again
    prob.optimize();

    // Check the solution status again
    if (prob.attributes.getSolStatus() != xpress::SolStatus::Optimal) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Problem not solved to optimality: " +
                               oss.str());
    }

    // Print out the new solution
    std::cout << std::endl << "*** Solution ***" << std::endl;
    std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;
    sol = prob.getSolution();
    for (Variable v : x)
      std::cout << v.getName() << " = " << v.getValue(sol) << std::endl;
    std::cout << std::endl;
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

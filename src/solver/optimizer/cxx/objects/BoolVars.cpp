// (c) 2024-2026 Fair Isaac Corporation
#include <iomanip>   // For setting precision when printing doubles to console
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;

/* Problem showing the use of binary variables and how to model
constraints such as OR, AND using 'resultant variables'
*/
int main() {
  try {
    // Number of variables to create
    int R = 5;

    // Create a problem instance
    XpressProblem prob;
    // prob.callbacks.addMessageCallback(XpressProblem::console);

    // Create boolean variables x
    std::vector<xpress::objects::Variable> x = prob.addVariables(R)
                                                   .withType(ColumnType::Binary)
                                                   .withName("x_%d")
                                                   .toArray();

    // Create boolean variables xNeg (intended as xNeg = 1-x, but the problem
    // does not know this yet)
    auto xNeg = prob.addVariables(R)
                    .withType(ColumnType::Binary)
                    .withName("xNeg_%d")
                    .toArray();

    // Now add the relation that x[r] + xNeg[r] = 1 for all r in [0...R[
    prob.addConstraints(R, [&](int r) { return x[r] + xNeg[r] == 1.0; });

    // Add some random constraints
    prob.addConstraint(x[2].eq(xNeg[3])); // Add constraint x[2] == xNeg[3]

    // Now we are going to construct constraints using OR and AND operators on
    // the variables We need a 'resultant variable' to construct these OR and
    // AND constraints We create two here, with names 'TRUE' and 'FALSE'
    Variable trueVar = prob.addVariable(ColumnType::Binary, "TRUE");
    Variable falseVar = prob.addVariable(ColumnType::Binary, "FALSE");

    // Fix these helper variables to 1 (true) and 0 (false), respectively,
    // (hence the naming of these 'variables')
    trueVar.fix(1);  // Add constraint trueVar  == 1
    falseVar.fix(0); // Add constraint falseVar == 0

    // We can now use trueVar as placeholder for '== 1':
    prob.addConstraint(trueVar.andOf(
        x[0], xNeg[4])); // Add constraint trueVar == AND(x[0], xNeg[4])
    prob.addConstraint(
        trueVar.orOf(x[0], x[2])); // Add constraint trueVar == OR(x[0], x[2])

    // For more complicated expressions, we need non-fixed resultant variables
    // for each operator
    Variable andResultant1 =
        prob.addVariable(ColumnType::Binary, "andresultant1");
    Variable andResultant2 =
        prob.addVariable(ColumnType::Binary, "andresultant2");

    // Add constraint that AND(x[0] + x[1] + x[2]) == andResultant1
    std::vector<Variable> subrange1(
        x.begin(),
        x.begin() + 3); // We first explicitly create a subarray of variables
    prob.addConstraint(andResultant1.andOf(subrange1));

    // Add constraint that AND(xNeg[3] + xNeg[4]) == andResultant2. Now we
    // create the subarray within the function call
    prob.addConstraint(andResultant2.andOf(
        std::vector<Variable>(xNeg.begin() + 3, xNeg.end())));

    // Now finally create constraint definition that OR(andResultant1,
    // andResultant2) == trueVar (which equals 1)
    GeneralConstraintDefinition new_constraint_def =
        trueVar.orOf(andResultant1, andResultant2);
    // Now actually add the GeneralConstraintDefinition to the problem to get a
    // GeneralConstraint
    GeneralConstraint new_constraint = prob.addConstraint(new_constraint_def);

    // Finally, add a constraint that none of xNeg[0...2] should be true
    prob.addConstraint(
        falseVar.orOf(std::vector<Variable>(xNeg.begin(), xNeg.begin() + 3)));

    // write the problem in LP format for manual inspection
    std::cout << "Writing the problem to 'BoolVars.lp'" << std::endl;
    prob.writeProb("BoolVars.lp", "l");

    // Solve the problem
    std::cout << "Solving the problem" << std::endl;
    prob.optimize();

    // Check the solution status
    std::cout << "Problem finished with SolStatus "
              << prob.attributes.getSolStatus() << std::endl;
    if (prob.attributes.getSolStatus() != xpress::SolStatus::Optimal) {
      throw std::runtime_error("Problem not solved to optimality");
    }

    // Print the solution to console (first set precision to e.g. 5)
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "Solution has objective value (profit) of "
              << prob.attributes.getObjVal() << std::endl;
    std::cout << std::endl << "*** Solution ***" << std::endl;

    // Retrieve the solution values in one go
    std::vector<double> sol = prob.getSolution();

    // Loop over the relevant variables and print their name and value
    for (Variable x_r : x)
      std::cout << x_r.getName() << " = " << x_r.getValue(sol) << std::endl;
    std::cout << std::endl;

    for (Variable xNeg_r : xNeg)
      std::cout << xNeg_r.getName() << " = " << xNeg_r.getValue(sol)
                << std::endl;
    std::cout << std::endl;

    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

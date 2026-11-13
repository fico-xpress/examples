// (c) 2024-2026 Fair Isaac Corporation

/**
 * A simple example that formulates some constraints on the minimum and absolute
 * values of linear combinations of variables.
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

int main() {
  std::cout << "Formulating the general constraint example problem"
            << std::endl;
  int const R = 3;
  XpressProblem prob;

  std::vector<Variable> x =
      prob.addVariables(R).withName("x_%d").withUB(20).toArray();

  Variable y = prob.addVariable("y");
  Variable z = prob.addVariable("z");

  Expression objective = sum(x);

  // We want to formulate abs(x(0)-2*x(1)) <= 10. We need to introduce
  // two auxiliary variables for the argument of the abs function
  // and then break down the abs expression in multiple steps
  Variable diff1 = prob.addVariable("diff1");
  Variable absOfDiff1 = prob.addVariable("absOfDiff1");

  prob.addConstraint(diff1 == x[0] - 2 * x[1]);
  prob.addConstraint(absOfDiff1.absOf(diff1));
  prob.addConstraint(absOfDiff1 <= 10); // Could also be a bound

  // We link a new variable to the minimum of the x(i) and
  // require this variable to be >= 5
  // Clearly, this bound constraint could also be achieved by simply setting
  // the bounds of each x variable.
  Variable minOfX = prob.addVariable("minOfX");
  prob.addConstraint(minOfX.minOf(x));
  prob.addConstraint(minOfX >= 5);

  // We link variable y to the maximum of other variables, expressions, and
  // constants
  // y = max(x(2), 20, x(0)-z)
  Variable diff2 = prob.addVariable("diff2");
  prob.addConstraint(diff2 == x[0] - z);

  // the below code is equivalent to using the MaxOf function on the resultant y
  // prob.addConstraint(y.MaxOf(new Variable[] {x[2], diff2},
  // 20).setName("max_constraint"));
  prob.addConstraint(y.maxOf(std::vector<Variable>{x[2], diff2},
                             std::vector<double>{20}, "max_constraint"));

  // set objective function with a maximization sense
  prob.setObjective(objective, ObjSense::Maximize);

  // write the problem in LP format for manual inspection
  std::cout << "Writing the problem to 'GeneralConstraints.lp'" << std::endl;
  prob.writeProb("GeneralConstraints.lp", "l");

  // Solve the problem
  std::cout << "Solving the problem" << std::endl;
  prob.optimize();

  // check the solution status
  std::cout << "Problem finished with SolStatus "
            << to_string(prob.attributes.getSolStatus()) << std::endl;
  if (prob.attributes.getSolStatus() != SolStatus::Optimal) {
    throw std::runtime_error("Problem not solved to optimality");
  }

  // print the optimal solution of the problem to the console
  std::cout << "Solution has objective value (profit) of "
            << prob.attributes.getObjVal() << std::endl;
  std::cout << "*** Solution ***" << std::endl;
  auto sol = prob.getSolution();

  for (int r = 0; r < R; r++) {
    if (r)
      std::cout << ", ";
    std::cout << "x_" << r << " = " << x[r].getValue(sol);
  }
  std::cout << std::endl;
  std::cout << "y = " << y.getValue(sol) << ", z = " << z.getValue(sol)
            << std::endl;
  std::cout << "ABS ( x[0] - 2*x[1] ) = " << absOfDiff1.getValue(sol)
            << ", minOfX = " << minOfX.getValue(sol) << std::endl;

  return 0;
}

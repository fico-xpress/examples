// (c) 2024-2026 Fair Isaac Corporation

/**
 * Approximation of a nonlinear function by a special ordered set (SOS-2). An
 * SOS-2 is a constraint that allows at most 2 of its variables to have a
 * nonzero value. In addition, these variables have to be adjacent.
 *
 * - Example discussed in mipformref whitepaper -
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

int main() {

  int NB = 4;                                // number of breakpoints
  std::vector<double> B_X{1, 2.5, 4.5, 6.5}; // X coordinates of breakpoints
  std::vector<double> B_Y{1.5, 6, 3.5, 2.5}; // Y coordinates of breakpoints

  std::cout << "Formulating the special ordered sets example problem"
            << std::endl;
  XpressProblem prob;
  // create one w variable for each breakpoint. We express
  auto w = prob.addVariables(NB).withName("w_%d").withUB(1).toArray();

  Variable x = prob.addVariable("x");
  Variable y = prob.addVariable("y");

  // Define the SOS-2 with weights from B_X. This is necessary
  // to establish the ordering between the w variables.
  prob.addConstraint(SOS::sos2(w, B_X, "SOS_2"));

  // We use the w variables to express a convex combination.
  // In combination with the above SOS-2 condition,
  // X and Y are represented as a convex combination of 2 adjacent
  // breakpoints.
  prob.addConstraint(sum(w) == 1);

  // The below constraints express the actual locations of X and Y
  // in the plane as a convex combination of the breakpoints, subject
  // to the assignment found for the w variables.
  prob.addConstraint(x == scalarProduct(w, B_X));
  prob.addConstraint(y == scalarProduct(w, B_Y));

  // set lower and upper bounds on x
  x.setLB(2);
  x.setUB(6);

  // set objective function with a minimization sense
  prob.setObjective(y, ObjSense::Minimize);

  // write the problem in LP format for manual inspection
  std::cout << "Writing the problem to 'SpecialOrderedSets.lp'" << std::endl;
  prob.writeProb("SpecialOrderedSets.lp", "l");

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

  for (int b = 0; b < NB; b++) {
    std::cout << "w_" << b << " = " << w[b].getValue(sol);
    if (b < NB - 1)
      std::cout << ", ";
    else
      std::cout << std::endl;
  }

  std::cout << "x = " << x.getValue(sol) << ", y = " << y.getValue(sol)
            << std::endl;

  return 0;
}

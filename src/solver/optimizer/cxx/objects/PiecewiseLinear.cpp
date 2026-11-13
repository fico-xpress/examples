// (c) 2024-2026 Fair Isaac Corporation

/**
 * An example that demonstrates how to model a piecewise linear cost function. A
 * piecewise linear cost function f(x) assigns a different linear cost function
 * for different intervals of the domain of its argument x. This situation
 * occurs in real-world problems if there are price discounts available starting
 * from a certain quantity of sold goods.
 *
 * - Example discussed in mipformref whitepaper -
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;

int main(void) {
  std::cout << "Formulating the piecewise linear example problem" << std::endl;

  XpressProblem prob;
  Variable x = prob.addVariable("x");
  Variable fx = prob.addVariable("fx");
  Expression objective = fx;

  std::vector<double> BREAKPOINT_X{0,   50,  50,
                                   120, 120, 200}; // the breakpoints x values
  std::vector<double> BREAKPOINT_Y{0,   50,  75,
                                   180, 240, 400}; // the breakpoints y values

  // Create the break points from the x,y data
  std::vector<PwlBreakpoint> breakpoints(BREAKPOINT_X.size());
  for (unsigned i = 0; i < breakpoints.size(); ++i)
    breakpoints[i] = PwlBreakpoint(BREAKPOINT_X[i], BREAKPOINT_Y[i]);

  // add the piecewise linear constraints with the breakpoints
  prob.addConstraint(fx.pwlOf(x, breakpoints, "pwl_with_breakpoints"));

  // ! Add a lower bound on x to get a somewhat more meaningful model
  // x >= 150
  x.setLB(150);

  // set objective function with a minimization sense
  prob.setObjective(objective, ObjSense::Minimize);

  // write the problem in LP format for manual inspection
  std::cout << "Writing the problem to 'PiecewiseLinear.lp'" << std::endl;
  prob.writeProb("PiecewiseLinear.lp", "l");

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
  std::cout << std::endl;
  std::cout << "*** Solution ***" << std::endl;
  auto sol = prob.getSolution();

  std::cout << "x = " << x.getValue(sol) << ", fx = " << fx.getValue(sol)
            << std::endl;
  return 0;
}

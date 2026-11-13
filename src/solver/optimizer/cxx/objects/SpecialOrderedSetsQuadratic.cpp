// (c) 2024-2026 Fair Isaac Corporation

/**
 * Approximation of a quadratic function in 2 variables by special ordered sets
 * (SOS-2). An SOS-2 is a constraint that allows at most 2 of its variables to
 * have a nonzero value. In addition, these variables have to be adjacent.
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

  int const NX = 10;         // number of breakpoints on the X-axis
  int const NY = 10;         // number of breakpoints on the Y-axis
  std::vector<double> X(NX); // X coordinates of grid points
  std::vector<double> Y(NY); // Y coordinates of breakpoints
  // two dimensional array of function values on the grid points
  std::vector<std::vector<double>> F_XY(NX, std::vector<double>(NY));

  // assign the toy data
  for (int i = 0; i < NX; i++)
    X[i] = i + 1;
  for (int i = 0; i < NY; i++)
    Y[i] = i + 1;
  for (int i = 0; i < NX; i++)
    for (int j = 0; j < NY; j++)
      F_XY[i][j] = (X[i] - 5) * (Y[j] - 5);

  std::cout << "Formulating the special ordered sets quadratic example problem"
            << std::endl;
  XpressProblem prob;
  // create one w variable for each X breakpoint. We express
  auto wx = prob.addVariables(NX)
                .withName("wx_%d")
                // this upper bound i redundant because of the convex
                // combination constraint on the sum of the wx
                .withUB(1)
                .toArray();
  // create one w variable for each Y breakpoint. We express
  auto wy = prob.addVariables(NY)
                .withName("wy_%d")
                // this upper bound i redundant because of the convex
                // combination constraint on the sum of the wy
                .withUB(1)
                .toArray();

  // create a two-dimensional array of w variable for each grid point. We
  // express
  auto wxy = prob.addVariables(NX, NY)
                 .withName("wxy_%d_%d")
                 // this upper bound is redundant because of the convex
                 // combination constraint on the sum of the wy
                 .withUB(1)
                 .toArray();

  Variable x = prob.addVariable("x");
  Variable y = prob.addVariable("y");
  Variable fxy = prob.addVariable("fxy");

  // make fxy a free variable
  fxy.setLB(XPRS_MINUSINFINITY);

  // Define the SOS-2 constraints with weights from X and Y.
  // This is necessary to establish the ordering between
  // variables in wx and in wy.
  prob.addConstraint(SOS::sos2(wx, X, "SOS_2_X"));
  prob.addConstraint(SOS::sos2(wy, Y, "SOS_2_Y"));
  prob.addConstraint(sum(wx) == 1);
  prob.addConstraint(sum(wy) == 1);

  // link the wxy variables to their 1-dimensional colleagues
  prob.addConstraints(NX, [&](auto i) {
    return wx[i] == sum(NY, [&](auto j) { return wxy[i][j]; });
  });
  prob.addConstraints(NY, [&](auto j) {
    return wy[j] == sum(NX, [&](auto i) { return wxy[i][j]; });
  });

  // now express the actual x, y, and f(x,y) coordinates
  prob.addConstraint(x == scalarProduct(wx, X));
  prob.addConstraint(y == scalarProduct(wy, Y));
  prob.addConstraint(fxy == sum(NX, [&](auto i) {
                       return sum(
                           NY, [&](auto j) { return wxy[i][j] * F_XY[i][j]; });
                     }));

  // set lower and upper bounds on x and y
  x.setLB(2);
  x.setUB(10);
  y.setLB(2);
  y.setUB(10);

  // set objective function with a minimization sense
  prob.setObjective(fxy, ObjSense::Minimize);

  // write the problem in LP format for manual inspection
  std::cout << "Writing the problem to 'SpecialOrderedSetsQuadratic.lp'"
            << std::endl;
  prob.writeProb("SpecialOrderedSetsQuadratic.lp", "l");

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

  for (int i = 0; i < NX; i++) {
    std::cout << "wx_" << i << " = " << wx[i].getValue(sol);
    if (i < NX - 1)
      std::cout << ", ";
    else
      std::cout << std::endl;
  }
  for (int j = 0; j < NY; j++) {
    std::cout << "wy_" << j << " = " << wy[j].getValue(sol);
    if (j < NX - 1)
      std::cout << ", ";
    else
      std::cout << std::endl;
  }

  std::cout << "x = " << x.getValue(sol) << ", y = " << y.getValue(sol)
            << std::endl;
  return 0;
}

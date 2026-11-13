// (c) 2024-2026 Fair Isaac Corporation

/**
 * Maximize the area of a polygon of N vertices and a diameter of 1. The
 * position of vertices is indicated as (r,theta) coordinates where r denotes
 * the distance to the base point (vertex with number N) and theta the angle
 * from the x-axis.
 */
#ifdef _WIN32
   // To get M_PI
#  define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::cos;
using xpress::objects::utils::sin;
using xpress::objects::utils::sum;

// the number of vertices/sides of the polygon
#define NSIDES 5

int main() {
  XpressProblem prob;
  // Output all messages.
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /**** VARIABLES ****/

  // r corresponds to the distance from the base point to vertex i
  auto r = prob.addVariables(NSIDES - 1).withName("r_%d").withUB(1.0).toArray();

  // theta corresponds to the angle relative to the x-axis of vertex i
  auto theta =
      prob.addVariables(NSIDES - 1).withName("theta_%d").withUB(M_PI).toArray();

  /**** OBJECTIVE ****/

  // objective transfer column
  Variable objtransfercol =
      prob.addVariable(XPRS_MINUSINFINITY, XPRS_PLUSINFINITY,
                       ColumnType::Continuous, "objTransferCol");

  // Set objective: maximize area of the polygon:
  // r_i*r_j*sin(theta_i+1-theta_i)/2
  Expression area = sum(NSIDES - 2, [&](auto i) {
    return r[i] * r[i + 1] * sin(theta[i + 1] - theta[i]) / 0.5;
  });

  // To make the objective linear, just maximize the objtransfercol
  prob.setObjective(objtransfercol, ObjSense::Maximize);

  // Add the objective transfer row: area = objtransfercol
  prob.addConstraint(area == objtransfercol);

  /**** CONSTRAINTS ****/

  // any two non-origin nodes should have a distance <= 1 to satisfy the
  // diameter
  for (int i = 0; i < NSIDES - 1; i++) {
    for (int j = i + 1; j < NSIDES - 1; j++) {
      // r_i^2 + r_j^2 - 2 * r_i * r_j * cos(theta_j - theta_i) <= 1
      prob.addConstraint(r[i].square() + r[j].square() -
                             2 * r[i] * r[j] * cos(theta[j] - theta[i]) <=
                         1.0);
    }
  }

  // Ordering of the vertices: theta_i+1 >= theta_i
  prob.addConstraints(NSIDES - 2,
                      [&](auto i) { return theta[i + 1] >= theta[i]; });

  // Dump the problem to disk so that we can inspect it.
  prob.writeProb("polygon.lp", "lp");

  // Solve to local optimality
  prob.controls.setNlpSolver(XPRS_NLPSOLVER_LOCAL);

  // Solve
  prob.optimize();
  if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
      prob.attributes.getSolStatus() != SolStatus::Feasible)
    throw std::runtime_error("optimization failed with status " +
                             to_string(prob.attributes.getSolStatus()));
  auto sol = prob.getSolution();
  std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;
  // Print out the solution
  for (auto &var : r) {
    std::cout << var.getName() << ":" << var.getValue(sol) << " ";
  }
  std::cout << std::endl;
  for (auto &var : theta) {
    std::cout << var.getName() << ":" << var.getValue(sol) << " ";
  }
  std::cout << std::endl;

  return 0;
}

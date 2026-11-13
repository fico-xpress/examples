// (c) 2024-2026 Fair Isaac Corporation

/**
 * Small Quadratic Programming example.
 *
 * <pre>
 *   minimize x1 + x1^2 +2x1x2 +2x2^2 +x4^2
 *   s.t.
 *     C1:  x1 +2x2 -4x4 &gt;= 0
 *     C2: 3x1 -2x3 - x4 &lt;= 100
 *     C3: 10 &lt;= x1 +3x2 +3x3 -2x4 &lt;= 30
 *     0 &lt;= x1 &lt;= 20
 *     0 &lt;= x2,x3
 *     x4 free
 * </pre>
 */

#include <iostream>
#include <vector>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

int main() {
  XpressProblem prob;

  prob.callbacks.addMessageCallback(XpressProblem::console);

  ///// VARIABLES
  std::vector<Variable> x(4);
  x[0] = prob.addVariable(0, 20, ColumnType::Continuous, "x1");
  x[1] = prob.addVariable("x2");
  x[2] = prob.addVariable("x3");
  x[3] = prob.addVariable(XPRS_MINUSINFINITY, XPRS_PLUSINFINITY,
                          ColumnType::Continuous, "x4");

  ///// OBJECTIVE
  QuadExpression obj = QuadExpression::create();
  obj.addTerm(x[0]);
  obj.addTerm(x[0] * x[0]);
  obj.addTerm(2 * x[0] * x[1]);
  obj.addTerm(2.0 * x[1] * x[1]);
  obj.addTerm(x[3].square());
  prob.setObjective(obj, ObjSense::Minimize);

  ///// CONSTRAINTS
  prob.addConstraint(sum(x[0], 2 * x[1], -4 * x[3]) >= 0);
  prob.addConstraint(sum(3 * x[0], -2 * x[2], -x[3]) <= 100);
  prob.addConstraint(sum(x[0], 3 * x[1], 3 * x[2], -2 * x[3]).in(10, 30));

  ///// SOLVING + OUTPUT
  prob.writeProb("qp.lp");
  prob.optimize();

  std::cout << "Problem status: " << to_string(prob.attributes.getLpStatus())
            << std::endl;
  if (prob.attributes.getLpStatus() != LPStatus::Optimal)
    throw std::runtime_error("optimization failed with status " +
                             to_string(prob.attributes.getLpStatus()));

  std::cout << "Objective function value: " << prob.attributes.getObjVal()
            << std::endl;
  auto sol = prob.getSolution();
  for (int i = 0; i < 4; i++)
    std::cout << x[i].getName() << ": " << x[i].getValue(sol) << ", ";
  std::cout << std::endl;
  return 0;
}

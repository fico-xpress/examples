/******************************************************
   Xpress C++ Example Problems
   ===========================

   file indicators.cpp
   ```````````````````
   Indicator constraint definition example
   - Example discussed in mipformref whitepaper -   

   (c) 2024 Fair Isaac Corporation
       author: D. Salvagnin, Sep. 2024
*******************************************************/
#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;
using namespace std;

static const int N = 2;

int main()
{
  XpressProblem prob;

  // Create the decision variables
  auto x = prob.addVariables(N).withType(ColumnType::Continuous).withName("x_%d").toArray();
  auto b = prob.addVariables(N).withType(ColumnType::Binary).withName("b_%d").toArray();

  // b[0] = 1 -> x[0]+x[1] >= 12
  prob.addConstraint(b[0].ifThen(x[0] + x[1] >= 12));
  // b[1] = 0 -> x[1] <= 5
  prob.addConstraint(b[1].ifNotThen(x[1] <= 5));

  // Set objective
  prob.setObjective(b[0] + b[1], ObjSense::Maximize);

  // Write out the model in case we want to look at it.
  prob.writeProb("indicators.lp", "l");

  // Solve the problem
  prob.optimize();

  auto mipStatus = prob.attributes.getMipStatus();
  switch (mipStatus) {
    case MIPStatus::NotLoaded:
    case MIPStatus::LPNotOptimal:
      cout << "Solving not started" << endl;
      break;
    case MIPStatus::LPOptimal:
      cout << "Root LP solved" << endl;
      break;
    case MIPStatus::Unbounded:
      cout << "LP unbounded" << endl;
      break;
    case MIPStatus::NoSolutionFound:
    case MIPStatus::Infeasible:
      cout << "MIP search started, no solution" << endl;
      break;
    case MIPStatus::Solution:
    case MIPStatus::Optimal:
      cout << "MIP solution: " << prob.attributes.getObjVal() << endl;
      break;
  }

  for (int i = 0; i < N; i++) {
    cout << x[i].getName() << ": " << x[i].getSolution() << endl;
  }

  return 0;
}

/******************************************************
   Xpress C++ Example Problems
   ===========================

   file gencons.cpp
   ````````````````
   General constraints definition example
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


int main()
{
  XpressProblem prob;

  // Create the decision variables
  auto x = prob.addVariables(3).withName("x_%d").withUB(20).toArray();
  auto y = prob.addVariable("y");
  auto z = prob.addVariable("z");

  // abs(x_0 - 2*x_1) <= 10
  auto diff1 = prob.addVariable("diff1");
  auto absOfDiff1 = prob.addVariable("absOfDiff1");
  prob.addConstraint(diff1 == x[0] - 2*x[1]);
  prob.addConstraint(absOfDiff1.absOf(diff1));
  absOfDiff1.setUB(10);

  // min(x_i) >= 5
  auto minOfX = prob.addVariable("minOfX");
  prob.addConstraint(minOfX.minOf(x));
  minOfX.setLB(5);

  // y = max(x_2, 20, x_0 - z)
  auto diff2 = prob.addVariable("diff2");
  prob.addConstraint(diff2 == x[0] - z);
  std::array<Variable,2> maxVarArgs = {x[2], diff2};
  prob.addConstraint(y.maxOf(maxVarArgs, 20));

  // Objective
  prob.setObjective(sum(x), ObjSense::Maximize);

  // Write out the model in case we want to look at it.
  prob.writeProb("gencons.lp", "l");

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

  return 0;
}

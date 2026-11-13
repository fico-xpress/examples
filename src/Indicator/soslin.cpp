/******************************************************
   Xpress C++ Example Problems
   ===========================

   file soslin.cpp
   ```````````````
   Approximation of a nonlinear function by a SOS-2.
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

static std::vector<int> I = {0, 1, 2, 3};

int main()
{
  XpressProblem prob;
  std::vector<double> R = {1, 2.5, 4.5, 6.5};
  std::vector<double> FR = {1.5, 6, 3.5, 2.5};

  // Create the decision variables
  auto x = prob.addVariable(R[0],R[3],ColumnType::Continuous,"x");
                           // x-values interval defined by the breakpoints
// equivalently: x.setLB(R[0]); x.setUB(R[3]);

  auto y = prob.addVariable("y");
  auto w = prob.addVariables(I.size()).withName("w_%d").toArray();

  // Define the SOS-2 with weights R
  prob.addConstraint(SOS::sos(SetType::SOS2, w, R, "Defx"));

  // Weights must sum up to 1
  prob.addConstraint(sum(I, [&](auto i) { return w[i];}) == 1.0);

  // The variable and the corresponding function value we want to approximate
  prob.addConstraint(x == sum(I, [&](auto i) { return R[i]*w[i];}));
  prob.addConstraint(y == sum(I, [&](auto i) { return FR[i]*w[i];}));

  // Set a lower bound on x to make the problem more interesting
  x.setLB(2.0);

  // Set objective
  prob.setObjective(y);

  // Write out the model in case we want to look at it.
  prob.writeProb("soslin.lp", "l");

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

  cout << x.getName() << ": " << x.getSolution() << endl;
  cout << y.getName() << ": " << y.getSolution() << endl;

  return 0;
}

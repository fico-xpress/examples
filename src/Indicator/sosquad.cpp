/******************************************************
   Xpress C++ Example Problems
   ===========================

   file sosquad.cpp
   ````````````````
   Approximation of a nonlinear function in two variables 
   by two SOS-2.
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

  // problem data
  static const int NX = 10;
  static const int NY = 10;

  std::array<double,NX> X;
  for (int i = 0; i < NX; i++)  X[i] = (double)(i+1);
  std::array<double,NY> Y;
  for (int j = 0; j < NY; j++)  Y[j] = (double)(j+1);
  std::array<std::array<double,NY>, NX> FXY;
  for (int i = 0; i < NX; i++) {
    for (int j = 0; j < NY; j++) {
      FXY[i][j] = (double)(i-4)*(j-4);
    }
  }

  // Create the decision variables
  auto x = prob.addVariable("x");
  auto y = prob.addVariable("y");
  auto f = prob.addVariable(XPRS_MINUSINFINITY, XPRS_PLUSINFINITY, ColumnType::Continuous, "f");
  auto wx = prob.addVariables(NX).withName("wx_%d").toArray();
  auto wy = prob.addVariables(NY).withName("wy_%d").toArray();
  auto wxy = prob.addVariables(NX, NY).withName("wxy_%d_%d").toArray();

  // Define the SOS-2 for coordinate x
  prob.addConstraint(SOS::sos(SetType::SOS2, wx, X, "sos_x")); 
  // Define the SOS-2 for coordinate y
  prob.addConstraint(SOS::sos(SetType::SOS2, wy, Y, "sos_y")); 

  // Weights must sum up to 1 along the two coordinates
  prob.addConstraint(sum(NX, [&](auto i) { return wx[i]; }) == 1.0);
  prob.addConstraint(sum(NY, [&](auto j) { return wy[j]; }) == 1.0);

  // wx, wy and wxy must be consistent
  prob.addConstraints(NX, [&](auto i) {
    return wx[i] == sum(NY, [&](auto j) { return wxy[i][j]; });
  });
  prob.addConstraints(NY, [&](auto j) {
    return wy[j] == sum(NX, [&](auto i) { return wxy[i][j]; });
  });
  
  // The coordinates and the corresponding function value we want to approximate
  prob.addConstraint(x == sum(NX, [&](auto i) { return X[i]*wx[i]; }));
  prob.addConstraint(y == sum(NY, [&](auto j) { return Y[j]*wy[j]; }));
  prob.addConstraint(f == sum(NX, [&](auto i) {
    return sum(NY, [&](auto j) { return FXY[i][j]*wxy[i][j]; });
  }));

  // Set a lower bound on x and y to make the problem more interesting
  x.setLB(2.0);
  y.setLB(2.0);

  // Set objective
  prob.setObjective(f);

  // Write out the model in case we want to look at it.
  prob.writeProb("sosquad.lp", "l");

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
  cout << f.getName() << ": " << f.getSolution() << endl;

  return 0;
}

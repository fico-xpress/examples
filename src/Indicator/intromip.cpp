/******************************************************
   Xpress C++ Example Problems
   ===========================

   file intromip.cpp
   `````````````````
   Various code snippest from introductory section of
   the mipformref whitepaper 

   (c) 2024 Fair Isaac Corporation
       author: D. Salvagnin, Sep. 2024, rev. Aug. 2026
*******************************************************/
#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using namespace std;

static const int MAXVAL = 50;
static const int NP = 5;
static const int NL = 6;

int main()
{
  XpressProblem prob;

  Variable d = prob.addVariable(ColumnType::Binary, "d"); // Single binary variable

  auto ifmake = prob.addVariables(NP, NL)
    .withType(ColumnType::Binary)
    .withName("ifmake_%d_%d")
    .toArray();

  Variable x = prob.addVariable(0, MAXVAL, ColumnType::Integer, "x"); // An integer variable

  //...

  x.setType(ColumnType::SemiContinuous); // Change type to semi-continuous
  x.setLimit(10);

  //...

  x.setType(ColumnType::Continuous); // Change type to continuous

  prob.setObjective(d);
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

  return 0;
}

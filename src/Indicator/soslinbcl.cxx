/********************************************************
  Xpress BCL C++ Example Problems
  ===============================

  file soslin.cxx
  ```````````````
  Using SOS-2 for approximating a function in one variable.
  - Example discussed in mipformref whitepaper -  

  (c) 2009 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2009, rev. Feb. 2022
********************************************************/

#include <iostream>
#include "xprb_cpp.h"

using namespace std;
using namespace ::dashoptimization;

#define I 4

int main(int argc, char **argv)
{
 XPRBprob prob("testsos");
 XPRBvar x, y, w[I];
 XPRBexpr Defx, le, ly;
 double R[] ={1, 2.5, 4.5, 6.5};
 double FR[]={1.5, 6, 3.5, 2.5};
 int i;

// ...assign values to arrays R and F...

// Create the decision variables
 x = prob.newVar("x"); y = prob.newVar("y");
 for(i=0; i<I; i++) w[i] = prob.newVar("w");

// Define the SOS-2 with "reference row" coefficients from R
 for(i=0; i<I; i++) Defx += R[i]*w[i];
 prob.newSos("Defx", XPRB_S2, Defx); 
 for(i=0; i<I; i++) le += w[i];
 prob.newCtr("One", le == 1); 

// The variable and the corresponding function value we want to approximate
 prob.newCtr("Eqx", x == Defx);
 for(i=0; i<I; i++) ly += FR[i]*w[i];
 prob.newCtr("Eqy", y == ly);

// Set a lower bound on x to make the problem more interesting
 x.setLB(2);

// Solve the problem
 prob.setObj(y);
 prob.mipOptimize("");

 int mipstatus = prob.getMIPStat();
 switch (mipstatus) {
  case XPRB_MIP_NOT_LOADED:
  case XPRB_MIP_LP_NOT_OPTIMAL:
    cout << "Solving not started" << endl;
    break;
  case XPRB_MIP_LP_OPTIMAL:
    cout << "Root LP solved" << endl;
    break;
  case XPRB_MIP_UNBOUNDED:
    cout << "LP unbounded" << endl;
    break;
  case XPRB_MIP_NO_SOL_FOUND:
  case XPRB_MIP_INFEAS:
    cout << "MIP search started, no solution" << endl;
    break;
  case XPRB_MIP_SOLUTION:
  case XPRB_MIP_OPTIMAL:
    cout << "MIP solution: " << prob.getObjVal() << endl;
    break;
 }

 cout << x.getName() << ": " << x.getSol() << endl;
}

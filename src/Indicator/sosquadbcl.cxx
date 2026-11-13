/********************************************************
  Xpress BCL C++ Example Problems
  ===============================

  file sosquad.cxx
  ````````````````
  Using SOS-2 for approximating a function in two variables.
  - Example discussed in mipformref whitepaper -  

  (c) 2009 Fair Isaac Corporation
      author: S.Heipcke, Mar. 2009, rev. Mar. 2011
********************************************************/

#include <iostream>
#include "xprb_cpp.h"

using namespace std;
using namespace ::dashoptimization;

#define NX 10
#define NY 10
#define EPS 0.001

int main(int argc, char **argv)
{
 XPRBprob prob("testsos");
 XPRBvar x, y, f;
 XPRBvar wx[NX], wy[NY], wxy[NX][NY];       // Weights
 XPRBexpr Defx, Defy, le, lexy; 
 double X[NX];
 double Y[NY];
 double FXY[NX][NY];
 int i,j;

// ...assign values to arrays X and Y...
 for(i=0; i<NX; i++) X[i]=i+1;
 for(j=0; j<NY; j++) Y[j]=j+1;
 for(i=0; i<NX; i++)
  for(j=0; j<NY; j++) FXY[i][j]=(i-4)*(j-4);
    
// Create the decision variables
 x = prob.newVar("x"); y = prob.newVar("y"); f = prob.newVar("f", XPRB_PL, -XPRB_INFINITY, XPRB_INFINITY);
 for(i=0; i<NX; i++) wx[i] = prob.newVar("wx");
 for(j=0; j<NY; j++) wy[j] = prob.newVar("wy");
 for(i=0; i<NX; i++)
  for(j=0; j<NY; j++) wxy[i][j] = prob.newVar("wxy");

// Definition of SOS
 for(i=0; i<NX; i++) Defx += X[i]*wx[i];
 prob.newSos("Defx", XPRB_S2, Defx); 
 for(j=0; j<NY; j++) Defy += Y[j]*wy[j];
 prob.newSos("Defy", XPRB_S2, Defy); 

// Constraints
 for(i=0; i<NX; i++) {
  le = 0;
  for(j=0; j<NY; j++) le += wxy[i][j];
  prob.newCtr("Sumx", le == wx[i]); 
 }
 for(j=0; j<NY; j++) {
  le = 0;
  for(i=0; i<NX; i++) le += wxy[i][j];
  prob.newCtr("Sumy", le == wy[j]); 
 }
 le = 0;
 for(i=0; i<NX; i++) le += wx[i];
 prob.newCtr("Convx", le == 1); 
 le = 0;
 for(j=0; j<NY; j++) le += wy[j];
 prob.newCtr("Convy", le == 1); 
 
// Calculate x, y and the corresponding function value f we want to approximate
 prob.newCtr("Eqx", x == Defx);
 prob.newCtr("Eqy", y == Defy);
 for(i=0; i<NX; i++)
  for(j=0; j<NY; j++) lexy += FXY[i][j]*wxy[i][j];
 prob.newCtr("Eqy", f == lexy);

// Set bounds to make the problem more interesting
 x.setLB(2); y.setLB(2);

// Solve the problem
 prob.setObj(f);
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
 cout << y.getName() << ": " << y.getSol() << endl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                 */
/* Copyright (C) 2015-2026 Fair Isaac Corporation  */
/*                            All rights reserved  */
/*                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * */


 /*
   SLP example demonstrating

   1, how to add user functions implemented locally to an SLP problem,
   2, tracking how function evaluations happen
   3, demonstrating how to define user functions that return derivastives.
   4, demonstrating how to solve a problem iteration by iteration
   5, demonstrating how to write out the linearization of each iteration for analysis

   Solve

   Miminze t                               (opt = 27)
     s.t.
     t = y ^ 2 + z - v + w ^ 2
     x ^ 2 + t ^ 2 <= 730                  (= 27*27+1)
     and
     1 <= x <= 2
     2 <= y <= 3
     3 <= z <= 4
     4 <= v <= 5
     5 <= w <= 6

     (c) 2021-2026 Fair Isaac Corporation

 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include <xprs.h>

/* Perform the Xpress specified function call and check its return value. */
#define CHECK_XPRSCALL(call)                             \
  do {                                                   \
    int result_ = call;                                  \
    if ( result_ != 0 ) {                                \
      fprintf(stderr, "Line %d: Failed call to `%s`.\n", \
              __LINE__, #call);                          \
      goto returnWithError;                              \
    }                                                    \
  } while (0)


void Message(XPRSprob my_prob, void* my_object,
  const char* msg, int len, int msgtype)
{
  (void)my_prob;
  (void)my_object;
  switch (msgtype)
  {
  case 4:  /* error */
  case 3:  /* warning */
  case 2:  /* dialogue */
  case 1:  /* information */
    if (len == 0)
      printf("\n");
    else
      printf("%s\n", msg);
    break;
  default: /* exiting - buffers need flushing */
    fflush(stdout);
    break;
  }
}

// user function implementing
// y^2 + z - v + w^2
// with derivatives
int MyObj(double* InputValues, double* Deltas, double* Evaluation, double* Partials, void* Context)
{
  // the input values
  double y = 0;
  double z = 0;
  double v = 0;
  double w = 0;
  // value of the function
  double Value = 0;
  // the return array counter
  int nNeedDeltas = 0;
  (void)Context;

  // set up local variables
  y = InputValues[0];
  z = InputValues[1];
  v = InputValues[2];
  w = InputValues[3];

  // calculate the function value
  *Evaluation = y * y + z - v + w * w;

  // report calculation
  printf("MyObj(%f,%f,%f,%f) = %f\n", y, z, v, w, Value);

  if (Deltas && (Deltas[0] != 0.0 || Deltas[1] != 0.0 || Deltas[2] != 0.0 || Deltas[3] != 0.0)) {
    nNeedDeltas = 1;
  }

  // return deltas (y^2 + z - v + w^2)
  // derivatives are expected to follow only for those for which the Delta array holds a nonzero value
  if (nNeedDeltas) {
    if (Deltas[0]) Partials[0] = 2 * y;
    if (Deltas[1]) Partials[1] = 1;
    if (Deltas[2]) Partials[2] = -1;
    if (Deltas[3]) Partials[3] = 2 * w;
    // report derivatives
    printf("Diff_MyObj(%f,%f,%f,%f)) = %f,%f,%f,%f\n", y, z, v, w, 2 * y, 1., -1., 2 * w);
  }

  // return value indicates if the function call was sucessfull
  return 0;
}

// user function implementing
// x ^ 2 + t ^ 2
// derivatives will be calculated using tangential approximation, note the perturbed calls in the output
double Ball(double* InputValues, void* context)
{
  // input values
  double x = 0;
  double t = 0;
  // value of the function
  double Value = 0;
  // the return array counter
  (void)context;

  x = InputValues[0];
  t = InputValues[1];

  Value = x * x + t * t;

  // report calculation
  printf("Ball(%f,%f) = %f\n", x, t, Value);

  // for functions returning a single value, the returm value is the calculated value of the function
  return Value;
}

int main(void)
{

  XPRSprob xprob = NULL;
  int ReturnValue = 0;

  char buffer[1000];
  int idBall, idMyObj;

  /* Initialise Optimizer */
  CHECK_XPRSCALL( XPRSinit(NULL) );
  CHECK_XPRSCALL( XPRSgetbanner(buffer); printf("%s\n", buffer) );

  /* Initialise problem object */
  CHECK_XPRSCALL(XPRScreateprob(&xprob));

  // set messaginf callback
  CHECK_XPRSCALL(XPRSaddcbmessage(xprob, Message, NULL, 0));

  // use XSLP
  CHECK_XPRSCALL(XPRSsetintcontrol(xprob, XPRS_LOCALSOLVER, 0));

  /*  Miminze t
      s.t.
      t = y ^ 2 + z - v + w ^ 2
      x ^ 2 + t ^ 2 <= 730                  (27*27+1)
      and
      1 <= x <= 2
      2 <= y <= 3
      3 <= z <= 4
      4 <= v <= 5
      5 <= w <= 6

      Optimum should be 27 with y=2, z=3, v=5, w=5 (forced by objective) and x being 1 (forced by the second constrained) */

  {
    /* Linear data */
    int nRow = 2;                  /* Number of rows */
    char sRowType[] = { 'E','L' };          /* Row types */
    double dRHS[] = { 0,730 };            /* RHS values */
    double* dRange = NULL;               /* Range values - none in this example */
    char sRowName[] = "obj\0ball";        /* Row names */

    int nCol = 7;                  /* Number of columns */
    double dObj[] = { 1,0,0,0,0,0,0 };      /* Objective function coefficients */
    double dLowerBd[] = { 0,1,2,3,4,5,1 };      /* Lower bounds on the columns */
    double dUpperBd[] = { XPRS_PLUSINFINITY,2,3,4,5,6,1 };    /* Upper bounds - note macro for infinity */
    char sColName[] = "t\0x\0y\0z\0v\0w\0="; /* Column names */

    /* Note the existence of the "=" column fixed to 1 and used to add nonlinear terms that are not coefficients to the problem */
    /* Future releases of XSLP (XSLP with optimizer version 21.00 on onwards) will not require this to be done manually */

    /* Matrix data */
    int nColStart[] = { 0,1,1,1,1,1,1,1,1 };  /* Start positions of the columns in dMatElem
                                               - note there are nCol+1 entries, the last
                                               indicating where the next column would
                                               start if there was one */
    int* nColElem = NULL;               /* Number of elements in each column - not
                                               needed thanks to the last (optional) entry
                                               in nColStart */
    int nRowInd[] = { 0 };   /* Row indices for the matrix elements */
    double dMatElem[] = { -1 };  /* Matrix elements - arranged by column */

    /* Load the matrix into Optimizer */
    CHECK_XPRSCALL(XPRSloadlp(xprob, "UserFunctionExample_tracking", nCol, nRow, sRowType, dRHS, dRange, dObj, nColStart, nColElem, nRowInd, dMatElem, dLowerBd, dUpperBd));
    /* add names */
    CHECK_XPRSCALL(XPRSaddnames(xprob, 1, sRowName, 0, 0));
    CHECK_XPRSCALL(XPRSaddnames(xprob, 2, sColName, 0, 6));
  }

  // Add the user functions
  CHECK_XPRSCALL(XPRSnlpadduserfunction(xprob, "MyObj", XPRS_USERFUNCTION_VECMAPDELTA, 4, 1, 0, (XPRSfunctionptr)MyObj, NULL, &idMyObj));
  CHECK_XPRSCALL(XPRSnlpadduserfunction(xprob, "Ball", XPRS_USERFUNCTION_VECMAP, 2, 1, 0, (XPRSfunctionptr)Ball, NULL, &idBall));

  // add the nonlinear functions to the slp problem
  {
    int Col;
    int nToken = 0;
    int nCoef = 0;
    int RowIndex[10];
    int Type[100];
    double Value[100];
    int FormulaStart[10];

    // MyObj(y,z,v,w) = y^2 + z - v + w^2 into row 0
    RowIndex[nCoef] = 0;
    FormulaStart[nCoef++] = nToken;

    Type[nToken] = XPRS_TOK_FUN; Value[nToken++] = idMyObj;
    Type[nToken] = XPRS_TOK_LB;  Value[nToken++] = 0;
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "y", &Col)); Type[nToken] = XPRS_TOK_COL; Value[nToken++] = Col;
    Type[nToken] = XPRS_TOK_DEL;  Value[nToken++] = XPRS_DEL_COMMA;
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "z", &Col)); Type[nToken] = XPRS_TOK_COL; Value[nToken++] = Col;
    Type[nToken] = XPRS_TOK_DEL;  Value[nToken++] = XPRS_DEL_COMMA;
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "v", &Col)); Type[nToken] = XPRS_TOK_COL; Value[nToken++] = Col;
    Type[nToken] = XPRS_TOK_DEL;  Value[nToken++] = XPRS_DEL_COMMA;
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "w", &Col)); Type[nToken] = XPRS_TOK_COL; Value[nToken++] = Col;
    Type[nToken] = XPRS_TOK_RB;  Value[nToken++] = 0;
    Type[nToken] = XPRS_TOK_EOF; Value[nToken++] = 0;

    // BALL(x,t) = ( x ^ 2 + t ^ 2 ) into row 1
    RowIndex[nCoef] = 1;
    FormulaStart[nCoef++] = nToken;

    Type[nToken] = XPRS_TOK_FUN; Value[nToken++] = 2;
    Type[nToken] = XPRS_TOK_LB;  Value[nToken++] = 0;
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "x", &Col)); Type[nToken] = XPRS_TOK_COL; Value[nToken++] = Col;
    Type[nToken] = XPRS_TOK_DEL;  Value[nToken++] = XPRS_DEL_COMMA;
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "t", &Col)); Type[nToken] = XPRS_TOK_COL; Value[nToken++] = Col;
    Type[nToken] = XPRS_TOK_RB;  Value[nToken++] = 0;
    Type[nToken] = XPRS_TOK_EOF; Value[nToken++] = 0;
    FormulaStart[nCoef] = nToken;
    CHECK_XPRSCALL(XPRSnlpaddformulas(xprob, nCoef, RowIndex, FormulaStart, 0, Type, Value));
  }

  // Solve problem
  CHECK_XPRSCALL(XPRSnlpoptimize(xprob, "")); // 'c' as in continue

  // retrive and report the optimal solution
  {
    int nCol, Col;
    double* dsol;
    CHECK_XPRSCALL(XPRSgetintattrib(xprob, XPRS_COLS, &nCol));
    dsol = (double*)malloc(nCol * sizeof(double));
    CHECK_XPRSCALL(XPRSgetsolution(xprob, NULL, dsol, 0, nCol - 1));
    printf("Solution:\n");
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "x", &Col); printf("  x = %f\n", dsol[Col]));
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "y", &Col); printf("  y = %f\n", dsol[Col]));
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "z", &Col); printf("  z = %f\n", dsol[Col]));
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "v", &Col); printf("  v = %f\n", dsol[Col]));
    CHECK_XPRSCALL(XPRSgetindex(xprob, 2, "w", &Col); printf("  w = %f\n", dsol[Col]));
    free(dsol);
  }

  goto NormalReturn;
returnWithError:
  printf("\nError %d\n", ReturnValue);
  ReturnValue = -1;
NormalReturn:

  // Retrieve error from Xpress
  if (ReturnValue) {
    fprintf(stderr, "An error was detected during execution.\n");
    if (xprob) {
      int errorcode;
      char errorMessage[512];
      XPRSgetintattrib(xprob, XPRS_ERRORCODE, &errorcode);
      XPRSgetlasterror(xprob, errorMessage);
      fprintf(stderr, "Optimizer returned error code '%i' with message:\n%s\n", errorcode, errorMessage);
    }
  }

  /* Destroy problem object and free environment */
  XPRSdestroyprob(xprob);
  XPRSfree();

  return(ReturnValue);
}

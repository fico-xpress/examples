/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file Polygon_userfunc.c
   ```````````````````````
   Implement the polygon example using a user function.

   Maximize the area of polygon of N vertices and diameter of 1
   The position of vertices is indicated as (rho,theta) coordinates
   where rho denotes the distance to the base point
   (vertex with number N) and theta the angle from the x-axis.
   A user (black box) function is used to implement the problem.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "xprs.h"

#define MAXROW 20
#define MAXCOL 20
#define MAXELT 50
#define MAXTOKEN 200
#define MAXFORMULA 20

#define PI 3.14159

void Message(XPRSprob my_prob, void* my_object, const char* msg, int len, int msgtype);

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

double MyFunc(double* Values, void* Context);

/* Add a name to a buffer.
 * This is used to incrementally build the argument for XPRSaddnames.
 */
#define AddName(buffer,len,...) do {                                    \
    unsigned newlen = len + 1 /* NUL terminator for string .*/          \
      + snprintf(buffer + len, sizeof(buffer) - len, __VA_ARGS__);      \
    if ( newlen > sizeof(buffer) ) {                                    \
      fprintf(stderr, "buffer " #buffer " too small\n");                \
      abort();                                                          \
    }                                                                   \
    len = newlen;                                                       \
  } while (0)

int main(void) {
  XPRSprob xprob = NULL;

  int nRow, nCol, nSide, nElement, nToken, nformula, nRowName, nColName;
  int iRow;
  char RowType[MAXROW];
  double RHS[MAXROW], OBJ[MAXCOL], Element[MAXELT], Lower[MAXCOL], Upper[MAXCOL];
  int ColStart[MAXCOL + 1], RowIndex[MAXELT], ColIndex[MAXCOL];
  int FormulaStart[MAXFORMULA + 1];
  int Type[MAXTOKEN];
  double Value[MAXTOKEN];

  double InitialValue[MAXCOL];

  int functionID;

  int ReturnValue = 0;
  int i, j;
  char RowNames[500], ColNames[500];

  int nMyFuncArguments;

  /* Initialisation */
  CHECK_XPRSCALL(XPRSinit(NULL));
  CHECK_XPRSCALL(XPRScreateprob(&xprob));

  /* Message callback */
  CHECK_XPRSCALL(XPRSaddcbmessage(xprob, Message, NULL, 0));

  nSide = 5;
  nRowName = 0;

  /* Rows */
  nRow = nSide - 2 + (nSide - 1) * (nSide - 2) / 2 + 1;
  for (i = 0; i < nRow; i++) RHS[i] = 0;

  nRow = 0;
  RowType[nRow++] = 'E'; /* OBJEQ */
  AddName(RowNames, nRowName, "OBJEQ");
  for (i = 1; i < nSide - 1; i++) {
    RowType[nRow++] = 'G'; /* T2T1 .. T4T3 */
    RHS[i] = 0.001;
    AddName(RowNames, nRowName, "T%dT%d", i + 1, i);
  }

  for (i = 1; i < nSide - 1; i++) {
    for (j = i + 1; j < nSide; j++) {
      RowType[nRow] = 'L';
      RHS[nRow++] = 1.0;
      AddName(RowNames, nRowName, "V%dV%d", i, j);
    }
  }
  RowType[nRow] = '\0';

  /* Columns */
  nColName = 0;
  nCol = (nSide - 1) * 2 + 2;
  nElement = 0;
  for (i = 0; i < nCol; i++) {
    OBJ[i] = 0;					  /* objective function */
    Lower[i] = 0;				  /* lower bound normally zero */
    Upper[i] = XPRS_PLUSINFINITY; /* upper bound infinity */
  }

  /* OBJX */
  nCol = 0;
  ColStart[nCol] = nElement;
  OBJ[nCol] = 1.0;
  Lower[nCol++] = XPRS_MINUSINFINITY; /* free column */
  Element[nElement] = -1.0;
  RowIndex[nElement++] = 0;
  AddName(ColNames, nColName, "OBJX");

  /* THETA1 - THETA 4 */
  iRow = 0;
  for (i = 1; i < nSide; i++) {
    AddName(ColNames, nColName, "THETA%d", i);
    InitialValue[nCol] = PI * ((double)(i)) / ((double)(nSide));
    ColStart[nCol++] = nElement;
    if (i < nSide - 1) {
      Element[nElement] = -1;
      RowIndex[nElement++] = iRow + 1;
    }
    if (i > 1) {
      Element[nElement] = 1;
      RowIndex[nElement++] = iRow;
    }
    iRow++;
  }

  Upper[nCol - 1] = PI;

  /* Remaining columns come later */
  for (i = 1; i < nSide; i++) {
    Lower[nCol] = 0.01;	  /* lower bound */
    Upper[nCol] = 1;
    InitialValue[nCol] = 1;
    ColStart[nCol++] = nElement;
    AddName(ColNames, nColName, "RHO%d", i);
  }
  ColStart[nCol] = nElement;

  CHECK_XPRSCALL(XPRSsetintcontrol(xprob, XPRS_MPSNAMELENGTH, 16));

  CHECK_XPRSCALL(XPRSloadlp(xprob, "Polygon", nCol, nRow, RowType, RHS, NULL, OBJ,
    ColStart, NULL, RowIndex, Element, Lower, Upper));
  CHECK_XPRSCALL(XPRSaddnames(xprob, 1, RowNames, 0, nRow - 1));
  CHECK_XPRSCALL(XPRSaddnames(xprob, 2, ColNames, 0, nCol - 1));

  /* Define user function */
  nToken = 0;

  nMyFuncArguments = 2 * (nSide - 1);
  CHECK_XPRSCALL(XPRSnlpadduserfunction(xprob, "MyArea", XPRS_USERFUNCTION_VECMAP, 2 * (nSide - 1), 1, 0, (XPRSfunctionptr)&MyFunc, &nMyFuncArguments, &functionID));

  /* Build up nonlinear coefficients */
  /* Area */
  nToken = 0;
  nformula = 0;
  RowIndex[nformula] = 0;
  FormulaStart[nformula++] = nToken;
  Type[nToken] = XPRS_TOK_RB;
  Value[nToken++] = 0;
  for (i = nSide - 1; i > 0; i--) {
    Type[nToken] = XPRS_TOK_COL;
    Value[nToken++] = i;
    Type[nToken] = XPRS_TOK_COL;
    Value[nToken++] = nSide + i - 1;
  }
  Type[nToken] = XPRS_TOK_FUN;
  Value[nToken++] = functionID;
  Type[nToken] = XPRS_TOK_EOF;
  Value[nToken++] = 0;

  /* Distances */
  for (i = 1; i < nSide - 1; i++) {
    for (j = i + 1; j < nSide; j++) {
      RowIndex[nformula] = iRow++;
      FormulaStart[nformula++] = nToken;

      Type[nToken] = XPRS_TOK_COL;
      Value[nToken++] = nSide + i - 1;
      Type[nToken] = XPRS_TOK_CON;
      Value[nToken++] = 2;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_EXPONENT;
      Type[nToken] = XPRS_TOK_COL;
      Value[nToken++] = nSide + j - 1;
      Type[nToken] = XPRS_TOK_CON;
      Value[nToken++] = 2;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_EXPONENT;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_PLUS;
      Type[nToken] = XPRS_TOK_CON;
      Value[nToken++] = 2;
      Type[nToken] = XPRS_TOK_COL;
      Value[nToken++] = nSide + i - 1;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_MULTIPLY;
      Type[nToken] = XPRS_TOK_COL;
      Value[nToken++] = nSide + j - 1;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_MULTIPLY;
      Type[nToken] = XPRS_TOK_RB;
      Value[nToken++] = 0;
      Type[nToken] = XPRS_TOK_COL;
      Value[nToken++] = j;
      Type[nToken] = XPRS_TOK_COL;
      Value[nToken++] = i;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_MINUS;
      Type[nToken] = XPRS_TOK_IFUN;
      Value[nToken++] = XPRS_IFUN_COS;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_MULTIPLY;
      Type[nToken] = XPRS_TOK_OP;
      Value[nToken++] = XPRS_OP_MINUS;
      Type[nToken] = XPRS_TOK_EOF;
      Value[nToken++] = 0;
    }
  }
  FormulaStart[nformula] = nToken;

  CHECK_XPRSCALL(XPRSnlploadformulas(xprob, nformula, RowIndex, FormulaStart, 1, Type, Value));

  for (i = 0; i < nCol; i++) {
    ColIndex[i] = i;
  }

  CHECK_XPRSCALL(XPRSnlpsetinitval(xprob, nCol - 1, &ColIndex[1], &InitialValue[1]));

  CHECK_XPRSCALL(XPRSchgobjsense(xprob, XPRS_OBJ_MAXIMIZE));

  CHECK_XPRSCALL(XPRSoptimize(xprob, "s", NULL, NULL));

  goto NormalReturn;
returnWithError:
  printf("\nError %d", ReturnValue);
  ReturnValue = -1;
NormalReturn:

  // Retrieve error from Xpress
  if (ReturnValue) {
    fprintf(stderr, "An error was detected during execution.\n");
    if (xprob) {
      int errorcode = 0;
      char errorMessage[512];
      XPRSgetintattrib(xprob, XPRS_ERRORCODE, &errorcode);
      if (errorcode) {
        XPRSgetlasterror(xprob, errorMessage);
        fprintf(stderr, "Optimizer returned error code '%i' with message:\n%s\n", errorcode, errorMessage);
      }
    }
  }

  XPRSdestroyprob(xprob);
  XPRSfree();
  return(ReturnValue);
}

void Message(XPRSprob my_prob, void* my_object, const char* msg, int len, int msgtype) {
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

double MyFunc(double* Values, void* Context) {
  int i;
  double Area;
  int* nMyFuncArguments = (int*)Context;
  Area = 0;
  for (i = 3; i < (*nMyFuncArguments); i = i + 2) {
    Area = Area + 0.5 * Values[i - 3] * Values[i - 1] * sin(Values[i] - Values[i - 2]);
  }
  return Area;
}

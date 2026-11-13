/***********************************************************************
   Xpress Optimizer Examples
   =========================

      file Polygon_multimap.c
   ``````````````````````````

   Maximize the area of polygon of N vertices and diameter of 1
   The position of vertices is indicated as (rho,theta) coordinates
   where rho denotes the distance to the base point
   (vertex with number N) and theta the angle from the x-axis.
   The nonlinear expressions are described text based input.

   (c) 2021-2026 Fair Isaac Corporation

***********************************************************************

   Polygon example: maximise the area of an N sided polygon

   *** Demonstrating using a multi map (R^k->R^l) userfunction ***

   Variables:

    rho   : 0..N-1   ! Distance of vertex from the base point
    theta : 0..N-1   ! Angle from x-axis

   Objective:
     (sum (i in 1..N-2) (rho(i)*rho(i-1)*sin(theta(i)-theta(i-1)))) * 0.5

   Constraints:
     Vertices in increasing degree order:
        theta(i) >= theta(i-1) +.01  : i = 1..N-2
     Boundary conditions:
        theta(N-1) <= Pi
        0.1 <= rho(i) <= 1   : i = 0..N-2
     Third side of all triangles <= 1
        rho(i)^2 + rho(j)^2 - rho(i)*rho(j)*2*cos(theta(j)-theta(i)) <= 1    : i in 0..N-3, j in i..N-2

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "xprs.h"

#define PI 3.14159

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

// userfunction of type "multimapdelta", implementing (x0,x1) -> (sin(x0-x1),cos(x0-x1))
int myTrigonometrics(double* x, double* results, void* context)
{
  (void)context;
  results[0] = sin(x[0] - x[1]);
  results[1] = cos(x[0] - x[1]);
  return (0);
}

// Message callback
void message(XPRSprob prob, void* my_object, const char* msg, int len, int msgtype) {
  (void)prob;
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

int main(void) {
  XPRSprob xprob = NULL;

  // Number of side of the Polygon
  int nSide = 5;
  int maxNameLength = 256;
  int ReturnValue = 0;
  int nameCursor = 0;
  int nameBufferSizeRows = 0;
  int nameBufferSizeCols = 0;

  double* RHS = NULL;
  char* RowType = NULL;
  char* RowNames = NULL;

  double* OBJ = NULL;
  double* Lower = NULL;
  double* Upper = NULL;

  int* ColStart = NULL;
  double* Element = NULL;
  int* RowIndex = NULL;
  char* ColNames = NULL;

  char* CoefBuffer = NULL;
  int coefBufferCursor = 0;
  int coefBufferSize = 0;

  int i, j;

  /* Initialisation */
  CHECK_XPRSCALL(XPRSinit(NULL));
  CHECK_XPRSCALL(XPRScreateprob(&xprob));

  /* Messaging */
  CHECK_XPRSCALL(XPRSaddcbmessage(xprob, message, NULL, 0));

  printf("####################\n");
  printf("# Polygon example  #\n");
  printf("####################\n");

  /* Create rows */
  int nRow = nSide - 2 + (nSide - 1) * (nSide - 2) / 2 + 1;

  RHS = (double*)malloc(nRow * sizeof(double));
  if (!RHS) goto MemoryReturn;

  for (i = 0; i < nRow; i++) RHS[i] = 0;

  RowType = (char*)malloc((nRow + 1) * sizeof(char));
  if (!RowType) goto MemoryReturn;
  nameBufferSizeRows = maxNameLength * (nRow + 1);
  RowNames = (char*)malloc(nameBufferSizeRows * sizeof(char));
  if (!RowNames) goto MemoryReturn;

  nRow = 0;

  // Objective constraint
  RowType[nRow] = 'E'; /* OBJEQ */
  nameCursor += snprintf(RowNames + nameCursor, nameBufferSizeRows - nameCursor, "OBJEQ");
  nameCursor++;
  if (nameCursor + 1 > nameBufferSizeRows) goto BufferReturn;
  nRow++;

  // Vertices in increasing degree order
  for (i = 1; i < nSide - 1; i++)
  {
    RowType[nRow] = 'G'; /* T2T1 .. T4T3 */
    nameCursor += snprintf(RowNames + nameCursor, nameBufferSizeRows - nameCursor, "T%iT%i", i + 1, i);
    nameCursor++;
    if (nameCursor + 1 > nameBufferSizeRows) goto BufferReturn;
    nRow++;
    RHS[i] = 0.001;
  }

  // Third side of all triangles <= 1
  for (i = 1; i < nSide - 1; i++)
  {
    for (j = i + 1; j < nSide; j++)
    {
      RowType[nRow] = 'L';
      RHS[nRow] = 1.0;
      nameCursor += snprintf(RowNames + nameCursor, nameBufferSizeRows - nameCursor, "V%iV%i", i, j);
      nameCursor++;
      if (nameCursor + 1 > nameBufferSizeRows) goto BufferReturn;
      nRow++;
    }
  }
  RowType[nRow] = '\0';

  /* Columns and linear coefficients*/
  int nCol = (nSide - 1) * 2 + 2;
  int nElement = 0;

  OBJ = (double*)malloc((nCol + 1) * sizeof(double));
  if (!OBJ) goto MemoryReturn;
  Lower = (double*)malloc((nCol + 1) * sizeof(double));
  if (!Lower) goto MemoryReturn;
  Upper = (double*)malloc((nCol + 1) * sizeof(double));
  if (!Upper) goto MemoryReturn;

  for (i = 0; i < nCol; i++)
  {
    OBJ[i] = 0;           /* objective function */
    Lower[i] = 0;         /* lower bound normally zero */
    Upper[i] = XPRS_PLUSINFINITY; /* upper bound infinity */
  }

  // Arrays to load linear coefficients
  ColStart = (int*)malloc((nRow + 1) * sizeof(int));
  if (!ColStart) goto MemoryReturn;
  Element = (double*)malloc((10 * nCol) * sizeof(double));
  if (!Element) goto MemoryReturn;
  RowIndex = (int*)malloc((10 * nCol) * sizeof(int));
  if (!RowIndex) goto MemoryReturn;
  nameBufferSizeCols = maxNameLength * (nCol);
  ColNames = (char*)malloc(nameBufferSizeCols * sizeof(char));
  if (!ColNames) goto MemoryReturn;

  nameCursor = 0;

  /* OBJX */
  nCol = 0;
  ColStart[nCol] = nElement;
  OBJ[nCol] = 1.0;
  nameCursor += snprintf(ColNames + nameCursor, nameBufferSizeCols - nameCursor, "OBJX");
  nameCursor++;
  if (nameCursor + 1 > nameBufferSizeCols) goto BufferReturn;
  Lower[nCol++] = XPRS_MINUSINFINITY; /* free column */
  Element[nElement] = -1.0;
  RowIndex[nElement++] = 0;

  /* THETA1 - THETA 4 */
  int iRow = 0;
  for (i = 1; i < nSide; i++)
  {
    nameCursor += snprintf(ColNames + nameCursor, nameBufferSizeCols - nameCursor, "THETA%i", i);
    nameCursor++;
    if (nameCursor + 1 > nameBufferSizeCols) goto BufferReturn;
    ColStart[nCol++] = nElement;
    if (i < nSide - 1)
    {
      Element[nElement] = -1;
      RowIndex[nElement++] = iRow + 1;
    }
    if (i > 1)
    {
      Element[nElement] = 1;
      RowIndex[nElement++] = iRow;
    }
    iRow++;
  }

  Upper[nCol - 1] = PI;

  /* Rho's */
  for (i = 1; i < nSide; i++)
  {
    Lower[nCol] = 0.01;   /* lower bound */
    Upper[nCol] = 1;
    nameCursor += snprintf(ColNames + nameCursor, nameBufferSizeCols - nameCursor, "RHO%i", i);
    nameCursor++;
    if (nameCursor + 1 > nameBufferSizeCols) goto BufferReturn;
    ColStart[nCol++] = nElement;
  }
  ColStart[nCol] = nElement;

  CHECK_XPRSCALL(XPRSloadlp(xprob, "Polygon", nCol, nRow, RowType, RHS, NULL, OBJ, ColStart, NULL, RowIndex, Element, Lower, Upper));

  CHECK_XPRSCALL(XPRSaddnames(xprob, 1, RowNames, 0, nRow - 1));
  CHECK_XPRSCALL(XPRSaddnames(xprob, 2, ColNames, 0, nCol - 1));

  /* Add the user function*/
  CHECK_XPRSCALL(XPRSnlpadduserfunction(xprob, "myTrigonometrics", XPRS_USERFUNCTION_MULTIMAP, 2 /* #inputs arguments */, 2 /* #output arguments*/, 0, (XPRSfunctionptr)myTrigonometrics, NULL, NULL));

  /* Build up nonlinear coefficients */
  coefBufferSize = 1000 * maxNameLength;
  CoefBuffer = (char*)malloc(coefBufferSize);
  if (!CoefBuffer) goto MemoryReturn;

  /* Area */
  coefBufferCursor += snprintf(CoefBuffer + coefBufferCursor, coefBufferSize - coefBufferCursor,
    "0.5 * ( ");
  if (coefBufferCursor + 1 > coefBufferSize) goto BufferReturn;
  for (i = 1; i < nSide - 1; i++)
  {
    if (i > 1)
    {
      coefBufferCursor += snprintf(CoefBuffer + coefBufferCursor, coefBufferSize - coefBufferCursor,
        " + ");
      if (coefBufferCursor + 1 > coefBufferSize) goto BufferReturn;
    }
    coefBufferCursor += snprintf(CoefBuffer + coefBufferCursor, coefBufferSize - coefBufferCursor,
      "RHO%i * RHO%i * myTrigonometrics ( THETA%i , THETA%i : 1 )", i + 1, i, i + 1, i);
    if (coefBufferCursor + 1 > coefBufferSize) goto BufferReturn;
  }
  coefBufferCursor += snprintf(CoefBuffer + coefBufferCursor, coefBufferSize - coefBufferCursor,
    " )");
  if (coefBufferCursor + 1 > coefBufferSize) goto BufferReturn;
  CHECK_XPRSCALL(XPRSnlpchgformulastr(xprob, 0, CoefBuffer));

  /* Distances */
  for (i = 1; i < nSide - 1; i++)
  {
    for (j = i + 1; j < nSide; j++)
    {
      coefBufferCursor = 0;
      coefBufferCursor += snprintf(CoefBuffer + coefBufferCursor, coefBufferSize - coefBufferCursor,
        "RHO%i ^ 2 + RHO%i ^ 2 - 2 * RHO%i * RHO%i * myTrigonometrics ( THETA%i , THETA%i : 2 )", j, i, j, i, j, i);
      CHECK_XPRSCALL(XPRSnlpchgformulastr(xprob, iRow, CoefBuffer));
      iRow++;
    }
  }

  CHECK_XPRSCALL(XPRSchgobjsense(xprob, XPRS_OBJ_MAXIMIZE));

  CHECK_XPRSCALL(XPRSoptimize(xprob, "s", NULL, NULL));

  goto NormalReturn;
BufferReturn:
  printf("\nWorking buffer too short\n");
  goto NormalReturn;
MemoryReturn:
  printf("\nOut of memory\n");
  goto NormalReturn;
returnWithError:
  ReturnValue = -1;
NormalReturn:

  // Retrieve error from Xpress
  if (ReturnValue) {
    fprintf(stderr, "An error was detected during execution.\n");
    if (xprob) {
      int errorcode = 0;
      char errorMessage[512];
      XPRSgetintattrib(xprob, XPRS_ERRORCODE, &errorcode);
      XPRSgetlasterror(xprob, errorMessage);
      fprintf(stderr, "Optimizer returned error code '%i' with message:\n%s\n", errorcode, errorMessage);
    }
  }

  XPRSdestroyprob(xprob);
  XPRSfree();

  free(RHS);
  free(RowType);
  free(RowNames);
  free(OBJ);
  free(Lower);
  free(Upper);
  free(ColStart);
  free(Element);
  free(RowIndex);
  free(ColNames);
  free(CoefBuffer);

  return(ReturnValue);
}

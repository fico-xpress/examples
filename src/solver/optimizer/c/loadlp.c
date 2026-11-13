/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file loadlp.c
   ``````````````
   Load an LP problem directly into Optimizer and modify it by adding an
   extra constraint.

   The problem
     Maximise
            2x + y
     subject to
       c1:  x + 4y <= 24
            c2:       y <=  5
            c3: 3x +  y <= 20
            c4:  x +  y <=  9
    and
            0 <= x, y <= +infinity
    and the extra constraint
            c5: 6x + y <= 20
   are first stored in the user's data structures. The LP is then loaded
   into Optimizer, using loadprob, and solved using the primal simplex
   algorithm. Next, the extra constraint is added to the problem matrix,
   using addrows, and the revised problem solved using the dual algorithm.
   In each case, the problem matrix is output to a file, the objective
   function value displayed on screen, and the problem statistics are
   are stored in a log file.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"                       /* Optimizer header file */

/* Calls an Xpress optimizer function and checks the return code.
 * If the call fails then the function
 * - prints a short error message to stderr,
 * - sets variable 'returnCode' to the error,
 * - and branches to label 'cleanup'.
 */
#define CHECK_RETURN(call) do {                         \
    int result_ = call;                                 \
    if ( result_ != 0 ) {                               \
      fprintf(stderr, "Line %d: %s failed with %d\n",   \
              __LINE__, #call, result_);                \
      returnCode = result_;                             \
      goto cleanup;                                     \
    }                                                   \
  } while (0)

static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);

int main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;                   /* The problem instance */
  char sProblem1[]  = "lp";               /* First problem name */
  char sProblem2[]  = "revised";          /* Second problem name */
  char sLogFile[]   = "loadlp.log";       /* Log file name */
  double dObjValue;                       /* Objective value */
  /* Store the problem */

  /* Row data */
  int nRow          = 4;                  /* Number of rows */
  char sRowType[]   = {'L','L','L','L'};  /* Row types */
  double dRHS[]     = {24, 5, 20, 9};        /* RHS values */
  double *dRange    = NULL;               /* Range values - none in this
                                             example */
  char sRowName[]   = "c1\0c2\0c3\0c4";   /* Row names */

  /* Column data */
  int nCol          = 2;                  /* Number of columns */
  double dObj[]     = {2, 1};             /* Objective function coefficients */
  double dLowerBd[] = {0, 0};             /* Lower bounds on the columns */
  double dUpperBd[] = {XPRS_PLUSINFINITY, XPRS_PLUSINFINITY}; /* Upper bounds -
                                                                 note macro for
                                                                 infinity */
  char sColName[]   = "x\0y";             /* Column names */

  /* Matrix data */
  int nColStart[]   = {0, 3, 7};          /* Start positions of the columns
                                             in dMatElem - note there are
                                             nCol+1 entries, the last
                                             indicating where the next column
                                             would start if there was one */
  int *nColElem     = NULL;               /* Number of elements in each column
                                             - not needed thanks to the last
                                             (optional) entry in nColStart */
  int nRowInd[]     = {0, 2, 3,  0, 1, 2, 3};  /* Row indices for the matrix
                                                  elements */
  double dMatElem[] = {1, 3, 1,  4, 1, 1, 1};  /* Matrix elements - arranged by
                                                  column */

  /* Store extra constraint */
  int nNewRow           = 1;              /* Number of new rows */
  int nNewElem          = 2;              /* Number of elements in the new row */
  char sNewRowType[1]   = {'L'};          /* New row type */
  char sNewRowName[]    = "c5";           /* New row name */
  double dNewRHS[1]     = {20};           /* New RHS value */
  double dNewRowElem[2] = {6, 1};         /* New row elements */
  int nNewRowStart[2]   = {0, 2};         /* Start positions of the rows in
                                             dNewRowElem */
  int nNewColInd[2]    = {0, 1};          /* Column indices for the new row
                                             elements */

  /* Initialize the optimizer. */
  if ( XPRSinit("") != 0 ) {
    char message[512];
    XPRSgetlicerrmsg(message, sizeof(message));
    fprintf(stderr, "Licensing error: %s\n", message);
    return 1;
  }

  /* Create a new problem and immediately register a message handler.
   * Once we have a message handler installed, errors will produce verbose
   * error messages on the console and we can limit ourselves to minimal
   * error handling in the code here.
   */
  CHECK_RETURN( XPRScreateprob(&prob) );
  CHECK_RETURN( XPRSaddcbmessage(prob, messagecb, NULL, 0) );

  /* Delete and define log file */
  remove(sLogFile);
  CHECK_RETURN( XPRSsetlogfile(prob, sLogFile) );

  /*** Load and solve the problem ***/

  /* Load the matrix into Optimizer */
  CHECK_RETURN( XPRSloadlp(prob, sProblem1, nCol, nRow, sRowType, dRHS, dRange,
                           dObj, nColStart, nColElem,
                           nRowInd, dMatElem, dLowerBd, dUpperBd) );

  /* Add row names */
  CHECK_RETURN( XPRSaddnames(prob, 1, sRowName, 0, nRow-1) );

  /* Add column names */
  CHECK_RETURN( XPRSaddnames(prob, 2, sColName, 0, nCol-1) );

  /* Output the matrix */
  CHECK_RETURN( XPRSwriteprob(prob, sProblem1,"") );
  printf("Matrix file %s.mat has been created.\n", sProblem1);

  /* Solve the LP problem */
  CHECK_RETURN( XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE) );
  CHECK_RETURN( XPRSlpoptimize(prob,"") );

  /* Get and display the value of the objective function */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjValue) );
  printf("The optimal objective value is %g.\n\n", dObjValue);

  /*** Add the extra constraint and solve again ***/

  /* Add new row */
  CHECK_RETURN( XPRSaddrows(prob, nNewRow, nNewElem, sNewRowType, dNewRHS,
                            NULL, nNewRowStart, nNewColInd,
                            dNewRowElem) );

  /* Add new row name */
  CHECK_RETURN( XPRSaddnames(prob, 1, sNewRowName, nRow, nRow) );

  /* Output the revised matrix */
  CHECK_RETURN( XPRSwriteprob(prob, sProblem2,"") );
  printf("Matrix file %s.mat has been created.\n", sProblem2);

  /* Solve with dual - since the revised problem inherits dual feasibility
     from the original */
  CHECK_RETURN( XPRSlpoptimize(prob,"d") );

  /* Get and display the value of the objective function */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjValue) );
  printf("The revised optimal objective value is %g.\n\n", dObjValue);

 cleanup:
  if (returnCode > 0) {
    /* There was an error with the solver. Get the error code and error message.
     * If prob is still NULL then the error was in XPRScreateprob() and
     * we cannot find more detailed error information.
     */
    if (prob != NULL) {
      int errorCode = -1;
      char errorMessage[512] = {0};
      XPRSgetintattrib(prob, XPRS_ERRORCODE, &errorCode);
      XPRSgetlasterror(prob, errorMessage);
      fprintf(stderr, "Error %d: %s\n", errorCode, errorMessage);
    }
  }

  /* Free the resources (variables are initialized so that this is valid
   * even in case of error).
   */
  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}


/* XPRS optimizer message callback */
void messagecb(XPRSprob cbprob, void* cbdata,
                       const char *msg, int len, int msgtype)
{
  (void)cbprob;   /* unused (the problem for which the message is issued) */
  (void)cbdata;   /* unused (the data passed to XPRSaddcbmessage()) */
  switch(msgtype)
  {
  case 4:  /* error */
  case 3:  /* warning */
  case 2:  /* not used */
  case 1:  /* information */
    printf("%*s\n", len, msg);
    break;
  default: /* exiting - buffers need flushing */
    fflush(stdout);
    break;
  }
}

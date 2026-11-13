/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file trimloss.c
   ```````````````
   Modify a problem by adding an extra variable within an additional
   constraint.

   We take a trimloss problem in which each integer variable x(p)
   represents the number of rolls cut to pattern p.
   We define a new integer variable y=SUM(p)x(p) and add the associated
   constraint
                           x(1)+x(2)+...+x(N)-y = 0
   We do this by first adding a row containing the (unitary)
   coefficients of the x(p), and then a column corresponding to y.
   We output the revised matrix to a file and then solve the revised
   MIP, giving y the highest branching priority. Finally, we output
   the solution, both to the screen and to an ASCII solution file.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprs.h"                 /* Optimizer header file */

/* Define were data is located. */
#ifndef DATADIR
#  define DATADIR "../data"
#endif

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
  XPRSprob prob = NULL;                  /* The problem instance */
  char   sProblem[]=DATADIR"/trimloss";  /* Problem name */
  char   sLogFile[]="trimloss.log";      /* Log file name */

  int    nRow;            /* Number of rows */
  int    nCol;            /* Number of columns */
  int    i;               /* Loop counter */

  /* New row variables */
  int    nNewRow = 1;     /* Number of new rows */
  int    nRowElem;        /* Number of non-zero elements in the new row */
  double *pRowVal = NULL; /* Values of the new row elements */
  int    *pColInd = NULL; /* Column indices of the new row elements */
  int    nRowStart[1];    /* Start position of the row in pRowVal */
  char   sRowType[1];     /* New row type */
  double dRHS[1];         /* New RHS value */

  /* New column variables */
  int    nNewCol = 1;     /* Number of new columns */
  int    nColElem;        /* Number of non-zero elements in the new column */
  double *pColVal = NULL; /* Values of the new column elements */
  int    *pRowInd = NULL; /* Row indices of the new column elements */
  int    nColStart[1];    /* Start position of the column in pColVal */
  double dObjCoef[1];     /* Objective coefficient of the new column */
  double dUpperBnd[1];    /* Upper bound on the new column */
  double dLowerBnd[1];    /* Lower bound on the new column */
  char   sColType[1];     /* New column type */
  int    nNewColInd[1];   /* New column index */
  int    nTypeChg = 1;    /* Number of column types to change - to make y
                             an integer variable */
  int    nColPrior[1];    /* Branching priority of the new column */

  /* Solution variables */
  double dObjVal;         /* Objective value of the best integer solution found
                           */
  double *x = NULL;       /* Primal solution values */

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

  /* Remove and define log file */
  remove(sLogFile);
  CHECK_RETURN( XPRSsetlogfile(prob, sLogFile) );

  /* Turn off presolve and permit no cuts */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_PRESOLVE, 0) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY, 0) );

  /* Read the problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /* Get the number of rows and columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_ROWS, &nRow) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &nCol) );

  /*** Construct and add the new row ***/

  /* Assign the number of new row elements*/
  nRowElem = nCol;

  /* Allocate memory for the new row arrays */
  pRowVal=malloc(nRowElem * sizeof(*pRowVal));
  pColInd=malloc(nRowElem * sizeof(*pColInd));
  if (!pRowVal || !pColInd) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Construct the new row */
  dRHS[0]       = 0.0;
  nRowStart[0]  = 0;
  sRowType[0]   = 'E';
  /* Store the coefficents of the x(p) */
  for (i = 0; i < nCol; i++) {
    pRowVal[i] = 1.0;
    pColInd[i] = i;
  }

  /* Add the new row */
  CHECK_RETURN( XPRSaddrows(prob, nNewRow, nRowElem, sRowType, dRHS, NULL,
                            nRowStart, pColInd, pRowVal) );

  /* Update the number of rows */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_ROWS, &nRow) );

  /* Add new row name */
  CHECK_RETURN( XPRSaddnames(prob, XPRS_NAMES_ROW, "NewRow", nRow-1, nRow-1) );

  /*** Construct and add the new column ***/

  /* Assign the number of new column elements*/
  nColElem = 1;

  /* Allocate memory for the new column arrays */
  pColVal=malloc(nColElem  * sizeof(*pColVal));
  pRowInd=malloc(nColElem  * sizeof(*pRowInd));
  if (!pColVal || !pRowInd ) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Construct the new column */
  dObjCoef[0]  = 0.0;
  nColStart[0] = 0;
  pColVal[0]   = -1.0; /* the coefficent of -y */
  pRowInd[0]   = nRow-1;
  dLowerBnd[0] = 0.0;
  dUpperBnd[0] = 90;   /* the sum of the bounds on the x(p) */

  /* Add the new column */
  CHECK_RETURN( XPRSaddcols(prob, nNewCol, nColElem, dObjCoef, nColStart,
                            pRowInd, pColVal, dLowerBnd, dUpperBnd) );

  /* Update the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &nCol) );

  /* Make the new column an integer variable */
  nTypeChg=1;
  nNewColInd[0]=nCol-1;
  sColType[0]= 'I';

  CHECK_RETURN( XPRSchgcoltype(prob, nTypeChg, nNewColInd, sColType) );

  /* Add new column name */
  CHECK_RETURN( XPRSaddnames(prob, XPRS_NAMES_COLUMN, "y", nCol-1, nCol-1) );

  /*** Output the revised matrix ***/
  CHECK_RETURN( XPRSwriteprob(prob,"revised","") );

  /*** Solve the revised MIP ***/

  /* Assign the y variable a high branching priority (1)
     - by default the original x(p) variables have low priority (500) */
  nColPrior[0] = 1;

  /* Load this directive */
  CHECK_RETURN( XPRSloaddirs(prob, 1, nNewColInd, nColPrior, NULL, NULL, NULL)
    );

  /* Solve the MIP */
  printf("Solving problem %s:\n\n", sProblem);
  CHECK_RETURN( XPRSmipoptimize(prob,"") );

  /* Get the objective value of the best integer solution found */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_MIPOBJVAL, &dObjVal) );

  /* Allocate memory to the solution array */
  x = malloc(nCol*sizeof(*x));
  if (!x) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Get the primal solution values */
  CHECK_RETURN( XPRSgetsolution(prob, NULL, x, 0, nCol - 1) );

  /* Display the objective and solution values */
  printf("   Minimum loss is %g\n\n", dObjVal);
  printf("   The solution values are\n");
  for(i = 0; i < nCol-1; i++)
    printf("        x(%d)=%g\n", i+1, x[i]);
  printf("        y = %g\n\n", x[nCol-1]);

  /* Save the solution to an ASCII file */
  CHECK_RETURN( XPRSwritesol(prob,"revised","") );

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
  free(pRowVal);
  free(pColInd);
  free(pColVal);
  free(pRowInd);
  free(x);
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

/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file tableau.c
   ```````````````
   Read in a user-specified LP and solve it, displaying both the
   initial and optimal tableau.

   Inputs an MPS matrix file and required optimisation sense, and
   proceeds to solve the problem with lpoptimize. The simplex
   algorithm is interrupted to get its intial basis, and a tableau is
   requested with a call to function showtab. Once the solution is
   found, a second call produces the optimal tableau.
   Function showtab retrieves the pivot order of the basic variables,
   along with other problem information, and then constructs (and
   displays) the tableau row-by-row using the backwards transformation,
   btran.

   Given A*x = b with
                 - A being the problem matrix
                 - b its right hand side
                 - x all structural columns and slack variables

   The Simplex tableau for a given basis B is defined as

   (A_B^-1 * A) * x = A_B^-1 * b

   where A_B^-1 is the inverse of the basis matrix.
   Writing A = (A_B A_N) with sorted basic (B) and nonbasic (N) variables gives

   (I     A_B^-1 * A_N) * x   = A_B^-1 * b or

   x_B +  A_B^-1 * A_N  * x_N = A_B^-1 * b

   with I being the identity matrix.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprs.h"


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

#define MIN 1                        /* Sense values */
#define MAX 0

static int showtab(XPRSprob prob, const char *sState, const char *sProblem);
static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);

int main(int argc, char *argv[])
{
  int returnCode = 0;
  XPRSprob prob = NULL;             /* The problem instance */
  char const *sProblem = DATADIR"/tablo.mps";  /* Problem name */
  char   sLogFile[]="tableau.log";  /* Optimizer log file */
  int    nSense = MIN;              /* Optimisation sense: 1 if min, otherwise
                                       max */
  int    nIteration;                /* Number of simplex iterations */
  double dObjValue;                 /* Objective function value */

  /* Check the number of arguments is correct */
  if (argc > 3) {
    printf("Usage: tableau [matrix] [min|max]");
    return 1;
  }

  /* Set the problem name */
  if (argc >= 2)
    sProblem = argv[1];

  /* Set the optimisation sense (default is minim) */
  if (argc == 3) {
    if (strcmp(argv[2],"max") == 0)
      nSense=MAX;
    else if (strcmp(argv[2],"min") != 0) {
      printf("Usage: 'min' or 'max' expected as second argument\n\n");
      return 1;
    }
  }

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

  /* Delete and define the Optimizer log file */
  remove(sLogFile);
  CHECK_RETURN( XPRSsetlogfile(prob, sLogFile) );

  /* Input the matrix */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /* Turn presolve off as we want to display the initial tableau */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_PRESOLVE, 0) );

  /* Set the number of simplex iterations to zero */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_LPITERLIMIT, 0) );

  /* Perform the first step in the simplex algorithm */
  if (nSense == MAX) {
    CHECK_RETURN( XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE) );
  }
  CHECK_RETURN( XPRSlpoptimize(prob,"") );

  /* Display the initial tableau */
  CHECK_RETURN( showtab(prob, "Initial", sProblem) );

  /* Continue with the simplex algorithm */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_LPITERLIMIT, 1000000) );

  CHECK_RETURN( XPRSlpoptimize(prob,"") );

  /* Get and display the objective function value and the iteration count */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjValue) );

  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_SIMPLEXITER, &nIteration) );

  printf("M%simal solution with value %g found in %d iterations.\n",
         (nSense) ? "in" : "ax", dObjValue, nIteration);

  /* Display the optimal tableau */
  CHECK_RETURN( showtab(prob, "Optimal", sProblem) );

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


/*****************************************************************************\
 * Name:         showtab
 *
 * Purpose:      Display tableau on screen
 *
 * Arguments:    XPRSprob prob            The problem
 *
 *               const char *sState       Problem state
 *
 *               const char *sProblem     Problem name
 *
 * Return Value: None.
 *
 *
\*****************************************************************************/

int showtab(XPRSprob prob, const char *sState, const char *sProblem)
{
  int    returnCode = 0;
  int    nRow;                  /* Number of rows */
  int    nCol;                  /* Number of columns */
  int    nSprR;
  int    namlen;                /* Maximum length of row or column name */
  char   *sVectorName = NULL;   /* Row or column name */
  int    nElem;                 /* Number of non-zero matrix elements */
  int    *pRowInd = NULL;       /* Row indices of the matrix elements */
  int    iRow;                  /* Row index holder */
  double *pMatElem = NULL;      /* Values of the matrix elements */
  int    pColStart[2];          /* Start positions of the columns in pMatElem */
  double *pRhs = NULL;          /* Values of the right hand side  */
  int    *pPivot = NULL;        /* Pivot order of the basic variables */
  int    i, j;                  /* Loop counters */
  double *y = NULL;             /* Tableau computation: row of the basis inverse */
  double *z = NULL;             /* Tableau computation: tableau coeffs and rhs */
  double scalarProd;            /* Tableau computation: intermediate scalar products */


  /*** Get problem information ***/

  /* Get the number of rows */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_ROWS, &nRow) );

  /* Get the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &nCol) );

  /* Get the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_SPAREROWS, &nSprR) );

  /* Get the maximum name length */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_NAMELENGTH, &namlen) );
  namlen *= 8;  /* Because name length is returned in 8-byte units */

  /* Allocate memory to the arrays */
  y           = malloc(nRow          * sizeof(*y));
  z           = malloc((nRow+nCol+1) * sizeof(*z)); // tableau including rhs
  pRowInd     = malloc(nRow          * sizeof(*pRowInd));
  pMatElem    = malloc(nRow          * sizeof(*pMatElem));
  pRhs        = malloc(nRow          * sizeof(*pRhs));
  pPivot      = malloc(nRow          * sizeof(*pPivot));
  sVectorName = malloc((namlen+1)    * sizeof(*sVectorName));


  /* Check that the memory has been successfully allocated */
  if (!y || !z || !pRowInd || !pMatElem || !sVectorName || !pPivot) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Retrieve the pivot order of the basic variables */
  CHECK_RETURN( XPRSgetpivotorder(prob, pPivot) );

  /*** Construct and display the tableau ***/

  /* Printer header of matrix names */
  printf("\n%s tableau of Problem %s.\n", sState, sProblem);
  printf("Note: slacks on G type rows range from -infinity to 0\n\n     ");
  for (i=0; i<nRow; i++) {
    /* Get and print the individual row names */
    CHECK_RETURN( XPRSgetnamelist(prob, 1, sVectorName, namlen+1, NULL, i, i) );
    printf(" %-3.3s   ", sVectorName);
  }
  for (i=0; i<nCol; i++) {
    /* Get and print the individual column names */
    CHECK_RETURN( XPRSgetnamelist(prob, 2, sVectorName, namlen+1, NULL, i, i) );
    printf(" %-3.3s   ", sVectorName);
  }
  printf(" RHS\n");

  /* Retrieve the problem right hand side */
  CHECK_RETURN( XPRSgetrhs(prob, pRhs, 0, nRow-1) );

  /* Get the tableau row-by-row using the backwards transformation, btran */

  /* For each row iRow in turn, calculate z = e_irow * B^-1 * A */
  for (iRow=0; iRow<nRow; iRow++) {

    /* Set up the unit vector e_irow */
    for (i=0; i<nRow; i++) y[i]=0.0;
    y[iRow]=1.0;

    /* y = e_irow * B^-1, i.e., get iRow-th row of the basis inverse */
    CHECK_RETURN( XPRSbtran(prob, y) );

    /* Form z = y * A for each slack column of A */
    for (i=0; i<nRow; i++) z[i]=y[i];

    /* Form z = y * A, for each structural column of A */
    for (j=0; j<nCol; j++) {

      CHECK_RETURN( XPRSgetcols(prob, pColStart, pRowInd, pMatElem, nRow,
                                &nElem, j, j) );
      scalarProd =0;
      for (i=0; i<nElem; i++)
        scalarProd += y[pRowInd[i]] * pMatElem[i];
      z[nRow + j] = scalarProd;
    }

    /* Form z = y * b (right hand side) */
    scalarProd = 0;
    for (i=0; i<nRow; i++) {
      scalarProd += y[i] * pRhs[i];
    }
    z[nRow + nCol] = scalarProd;

    /* Display single tableau row */
    if (pPivot[iRow]<nRow) {
      /* Pivot is a row */
      CHECK_RETURN( XPRSgetnamelist(prob, 1, sVectorName, namlen+1, NULL,
                                    pPivot[iRow], pPivot[iRow]) );
    }
    else {
      /* Pivot is a column */
      CHECK_RETURN( XPRSgetnamelist(prob, 2, sVectorName, namlen+1, NULL,
                                    pPivot[iRow]-nRow-nSprR,
                                    pPivot[iRow]-nRow-nSprR) );
    }
    printf("%-3.3s:", sVectorName);
    for (j=0; j<=nRow + nCol; j++) {
      if (z[j]>=0.1 || z[j]<=-0.1)
        printf("%5.1f  ", z[j]);
      else
        printf("       ");
    }
    printf("\n");
  }
  printf("\n");

 cleanup:
  free(y);
  free(z);
  free(pRowInd);
  free(pMatElem);
  free(pRhs);
  free(pPivot);
  free(sVectorName);
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

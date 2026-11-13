/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file globjpar.c
   ```````````````
   Perform objective function parametrics on a MIP problem.

   We take a production plan model and observe how the optimal
   value of the objective function changes as we vary
   BEN(3), the benefit per month from finishing Project 3.
   The program increments BEN(3) from 8 to 15, and for each of these
   values revises the objective coefficients of the variables x(3, t), t=1:2
   and finds the best integer solution. Note that, for each t, the
   coefficient of x(3, t) is BEN(3)*(3-t) = BEN(3)*(6-t-4+1).
   The results are displayed on screen and the problem statistics stored
   in a log file.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"                      /* Optimizer header file */

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
  XPRSprob prob = NULL;               /* The problem instance */
  char sProblem[]=DATADIR"/pplan";    /* Problem name */
  char sLogFile[]="globjpar.log";     /* Log file name */
  int nRow;                           /* Number of rows */
  int nCol;                           /* Number of columns */
  int nColInd_x3[2];                  /* Column indices for x(3, t), t=1:2 */
  int *pRowStatus = NULL;             /* Basis status of the slack, surplus or
                                         artificial variable associated with
                                         each row */
  int *pColStatus = NULL;             /* Basis status of the columns */
  int i;                              /* Loop counter */
  double dBen3;                       /* Value of BEN(3) */
  double dObjCoeff_x3[2];             /* Objective coefficients of x(3, t),
                                         t=1:2 */
  double dObjValue;                   /* Objective value of the best integer
                                         solution */
  XPRSprob probCopy = NULL;

  /* Delete and define log file */
  remove(sLogFile);


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

  CHECK_RETURN( XPRScreateprob(&probCopy) );
  CHECK_RETURN( XPRSsetlogfile(prob, sLogFile) );

  /* Read the problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /* Set the objective sense */
  CHECK_RETURN( XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE) );

  /* Get the number of rows */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_ROWS, &nRow) );

  /* Get the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &nCol) );

  /* Get the column indices for x(3, t), t=1:2  */
  CHECK_RETURN( XPRSgetindex(prob, 2, "x___0301", &nColInd_x3[0]) );
  CHECK_RETURN( XPRSgetindex(prob, 2, "x___0302", &nColInd_x3[1]) );

  /* Allocate memory for the basis status arrays  */
  pRowStatus=malloc(nRow*sizeof(*pRowStatus));
  pColStatus=malloc(nCol*sizeof(*pColStatus));
  if (!pRowStatus || !pColStatus) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  printf("The results of the parameter changes on pplan are:\n\n");

  /* Increment BEN(3) from 8 to 15 */
  for (i=8; i<=15; i++) {
    dBen3 = (double) i;

    /* Revise the objective coefficients of x(3, t), t=1:2 */
    dObjCoeff_x3[0] = dBen3*(3.0-1.0);
    dObjCoeff_x3[1] = dBen3*(3.0-2.0);

    /* Change the objective function */
    CHECK_RETURN( XPRSchgobj(prob, 2, nColInd_x3, dObjCoeff_x3) );

    /* Store the current matrix - as it will be changed */
    CHECK_RETURN( XPRScopyprob(probCopy, prob,"temp") );

    /* Restore the previous optimal basis - for efficiency */
    if (i>8) {
      CHECK_RETURN( XPRSloadbasis(probCopy, pRowStatus, pColStatus) );
    }

    /* Solve the root node relaxation */
    CHECK_RETURN( XPRSsetintcontrol(probCopy, XPRS_MIPSTOPSTAGE, XPRS_MIPSTOPSTAGE_INITIALRELAXATION) );
    CHECK_RETURN( XPRSmipoptimize(probCopy,"") );
    CHECK_RETURN( XPRSsetdefaultcontrol(probCopy, XPRS_MIPSTOPSTAGE) );

    /* Get the optimal basis */
    CHECK_RETURN( XPRSgetbasis(probCopy, pRowStatus, pColStatus) );

    /* Search for an integer solution */
    CHECK_RETURN( XPRSmipoptimize(probCopy,"") );

    /* Get, and then print, the objective value of the best integer solution */
    CHECK_RETURN( XPRSgetdblattrib(probCopy, XPRS_MIPOBJVAL, &dObjValue) );
    printf("   BEN(3) = %2.0f; dObjValue = %4.1f\n", dBen3, dObjValue);
  }

  printf(" \n");

 cleanup:
  if (returnCode > 0) {
    /* There was an error. Get the error code and error message.
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
  free(pRowStatus);
  free(pColStatus);
  XPRSdestroyprob(probCopy);
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

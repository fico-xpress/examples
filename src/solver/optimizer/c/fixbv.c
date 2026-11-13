/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file fixbv.c
   ````````````
   Apply a binary fixing heuristic to an unpresolved MIP problem.

   We take a production plan model and solve its LP relaxation.
   Next we fix those binary variables that are very
   near zero to 0.0, and those that are almost one to 1.0, by changing
   their respective upper and lower bounds. Finally, we solve the
   modified problem as a MIP.
   This heuristic will speed up solution - though may fail to optimise
   the problem.
   The results are displayed on screen and the problem statistics
   stored in a log file.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"                   /* Optimizer header file */

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

#define TOL 0.0005                  /* Tolerance on binary variables */


int main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;            /* The problem instance. */
  char sProblem[]=DATADIR"/coco";  /* Problem name */
  char sLogFile[]="fixbv.log";     /* Log file name */

  int nCol;                        /* Number of columns */

  /* MIP problem information */
  int nDiscreteCols;               /* Number of discrete columns: binary,
                                      integer, semi-continuous and partial
                                      integer variables */
  int nSet;                        /* Number of S1 and S2 sets */
  int *pEntInd = NULL;             /* Column indices of the MIP entities (discrete columns) */
  char *pEntType = NULL;           /* MIP entity types */

  /* Bound changes */
  int *pBndInd = NULL;             /* Column indices of the bounds to be
                                      changed */
  char *pBndType = NULL;           /* New bound types */
  double *pBndVal = NULL;          /* New bound values */
  int nBnd;                        /* Bound counter */
  int i;                           /* Loop counter */
  int j;                           /* Holder for the bound indices */

  /* Solution information */
  double *x = NULL;                /* LP solution values */
  int nGlStatus;                   /* MIP status */
  int nNodes;                      /* Number of nodes solved so far in the
                                      branch-and-bound search */
  double dObjVal;                  /* Objective value of the best integer
                                      solution */

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

  /* Turn off presolve and permit no cuts - to slow down solution and allow
     the effect of the heuristic to be be seen */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_PRESOLVE, 0) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY, 0) );

  /* Read the problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /*** Solve the LP relaxation ***/

  /* Get the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &nCol) );

  /* Allocate memory for solution array and check for memory shortage */
  x = malloc(nCol * sizeof(*x));
  if (!x) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Solve the root node relaxation */
  CHECK_RETURN( XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPSTOPSTAGE, XPRS_MIPSTOPSTAGE_INITIALRELAXATION) );
  CHECK_RETURN( XPRSmipoptimize(prob,"") );
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_MIPSTOPSTAGE) );

  CHECK_RETURN( XPRSgetlpsol(prob, x, NULL, NULL, NULL) );
  /*** Fix the binary variables that are at their bounds ***/

  /* Allocate memory for MIP entity arrays and bound arrays */
  pEntInd=malloc(nCol * sizeof(*pEntInd));
  pEntType=malloc(nCol * sizeof(*pEntType));
  if (!pEntInd || !pEntType) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }
  CHECK_RETURN( XPRSgetmipentities(prob, &nDiscreteCols, &nSet, pEntType, pEntInd, NULL,
                              NULL, NULL, NULL, NULL) );
  pBndInd=malloc(nDiscreteCols * sizeof(*pBndInd));
  pBndVal=malloc(nDiscreteCols * sizeof(*pBndVal));
  pBndType=malloc(nDiscreteCols * sizeof(*pBndType));
  if (!pBndInd || !pBndVal || !pBndType) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Initialise bound counter */
  nBnd=0;

  /* Go through the MIP entities */
  for(i=0; i<nDiscreteCols; i++) {

    /* Test whether each is a binary variable */
    if (pEntType[i] == 'B') {

      /* Hold the index of the BV */
      j=pEntInd[i];

      /* If the value of the BV is within TOL of zero, store its index,
         set its upper bound to 0, and increment the bound counter */
      if (x[j]<=TOL) {
        pBndInd[nBnd]=j;
        pBndType[nBnd]='U';
        pBndVal[nBnd]=0.0;
        nBnd++;

        /* If the value of the BV is within TOL of one, store its index,
           set its lower bound to 1, and increment the bound counter */
      } else if ((1-x[j])<=TOL) {
        pBndInd[nBnd]=j;
        pBndType[nBnd]='L';
        pBndVal[nBnd]=1.0;
        nBnd++;
      }
    }
  }
  /* Instruct Optimizer to change the bounds of the appropriate BVs,
     and tell the user how many have been fixed */
  CHECK_RETURN( XPRSchgbounds(prob, nBnd, pBndInd, pBndType, pBndVal) );
  printf("Solving problem %s with a binary fixing heuristic:\n\n", sProblem);
  printf("   After the LP optimisation %d binary variables were fixed\n\n",
         nBnd);

  /*** Solve the modified problem as a MIP ***/

  /* Search for an integer solution */
  CHECK_RETURN( XPRSmipoptimize(prob,"") );

  /* Get the number of nodes solved in the branch-and-bound search */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_NODES, &nNodes) );

  /* Get the objective value of the best integer solution */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_MIPOBJVAL, &dObjVal) );

  /* Check the MIP status and display the results of the branch-and-bound search */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPSTATUS, &nGlStatus) );

  switch (nGlStatus) {
  case XPRS_MIP_NOT_LOADED:
    printf("   Problem has not been loaded");
    break;
  case XPRS_MIP_LP_NOT_OPTIMAL:
    printf("   Search has not begun - LP has not been optimised");
    break;
  case XPRS_MIP_LP_OPTIMAL:
    printf("   Search has not begun - LP has been optimised");
    break;
  case XPRS_MIP_NO_SOL_FOUND:
    printf("   Search interrupted - No integer solution was found");
    break;
  case XPRS_MIP_SOLUTION:
    printf("   Search interrupted - Integer solution found: %g", dObjVal);
    break;
  case XPRS_MIP_INFEAS:
    printf("   No integer solution was found");
    break;
  case XPRS_MIP_OPTIMAL:
    printf("   Integer solution found: %g", dObjVal);
    break;
  }
  printf("\n\n   The MIP optimisation took %d nodes\n\n", nNodes);

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
  free(x);
  free(pEntInd);
  free(pEntType);
  free(pBndInd);
  free(pBndVal);
  free(pBndType);
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

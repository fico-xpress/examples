/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file savesol.c
   ```````````````
   Show how to save a postsolved solution to memory and access the
   results of a MIP tree search. The program demonstrates the use of the
   integer solution callback.

   We take the knapsack problem in burglar.mps and instigate a MIP
   search. Whenever an integer solution is found it is postsolved,
   stored in memory, and printed to stdout. The best and final
   solution values, and other MIP search information, are displayed
   on screen. A log file is also created.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xprs.h"                       /* Optimizer header file */

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

static void printsol(XPRSprob, void* data);
static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);


static int gnCol;                      /* Number of columns in the problem
                                          matrix */
static double *gpIntSol = NULL;        /* Integer solution values */
static int callbackError = 0;          /* Indicates an error in a callback. */

int
main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;               /* The problem instance */
  char sProblem[]=DATADIR"/burglar";  /* Problem name */
  int nSol;                           /* Number of integer solutions found */
  double dBestObj;                    /* Best objective value found */
  int nNodes;                         /* Number of nodes solved in the MIP
                                         search */
  int nActNodes;                      /* Number of active nodes ignored by the
                                         search */
  int nLastNode;                      /* Node at which the last integer
                                         solution was found */
  double dBestBnd;                    /* Best bound found in the MIP search
                                       */
  int nGlStatus;                      /* MIP search status - complete,
                                         incomplete, etc */
  int i;                              /* Loop counter*/

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

  /* Alow no cuts - so the problem does not solve too quickly */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY, 0) );

  /* Read the problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /*** Tell Optimizer to postsolve every integer solution found, save it to
       memory and print the solution values to the output file ***/

  /* Call function printsol whenever an integer solution is found */
  CHECK_RETURN( XPRSaddcbintsol(prob, printsol, NULL, 0) );

  /* Get the number of columns and allocate space for the solution array */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &gnCol) );

  gpIntSol = malloc(gnCol*sizeof(*gpIntSol));
  if (!gpIntSol) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /*** Search for integer solutions ***/
  printf("Solving problem %s:\n\n", sProblem);
  CHECK_RETURN( XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE) );
  CHECK_RETURN( XPRSmipoptimize(prob,"") );
  if (callbackError) {
    returnCode = -3;
    goto cleanup;
  }

  /*** Retrieve the results of the MIP search ***/

  /* Get the number of integer solutions found */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPSOLS, &nSol) );

  /* Get the objective value of the best integer solution found */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_MIPOBJVAL, &dBestObj) );

  /* Get the number of outstanding nodes */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_ACTIVENODES, &nActNodes) );

  /* Get the node at which the last feasible integer solution was found */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPSOLNODE, &nLastNode) );

  /* Get the number of nodes solved */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_NODES, &nNodes) );

  /* Get the value of the best bound */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_BESTBOUND, &dBestBnd) );

  /* Get the MIP status */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPSTATUS, &nGlStatus) );

  /*** Display the results of the MIP search ***/

  switch (nGlStatus) {
  case 3:
    printf("MIP search incomplete\n\n");
    printf("   No integer solution found\n");
    printf("   %d nodes searched\n", nNodes);
    printf("   %d nodes remaining in search\n", nActNodes);
    printf("   Best bound %g\n\n", dBestBnd);
    break;
  case 4:
    printf("MIP search incomplete\n\n");
    printf("   %d integer solution%s found\n", nSol, (nSol==1) ? "" : "s");
    printf("   %d nodes searched\n", nNodes);
    printf("   %d nodes remaining in search\n", nActNodes);
    printf("   Best bound %g\n\n", dBestBnd);
    printf("   Best integer solution at node %d\n\n", nLastNode);
    printf("      Objective value %g\n\n", dBestObj);
    printf("      Solution values\n");
    for (i=0; i<gnCol; i++)
      printf("          x[%d]=%g\n", i, gpIntSol[i]);
    printf("\n");
    break;
  case 5:
    printf("MIP search complete\n\n");
    printf("   No integer solution found\n");
    printf("   %d nodes searched\n", nNodes);
    printf("   Best bound %g\n", dBestBnd);
    break;
  case 6:
    printf("MIP search complete\n\n");
    printf("   %d nodes searched\n", nNodes);
    printf("   %d integer solution%s found\n\n", nSol, (nSol==1) ? "" : "s");
    printf("   Best integer solution at node %d\n\n", nLastNode);
    printf("      Objective value %g\n\n", dBestObj);
    printf("      Solution values\n");
    for (i=0; i<gnCol; i++)
      printf("          x[%d]=%g\n", i, gpIntSol[i]);
    printf("\n");
    break;
  default:
    printf("MIP search did not take place\n\n");
    break;
  }

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
  free(gpIntSol);
  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

/*****************************************************************************\
 * Name:         printsol
 *
 * Purpose:      Save the postsolved nodal integer solution to memory, retrieve
 *               it and print the solution values to the output file.
 *
 *               The function is called when an integer solution is found at a
 *               node in the branch and bound tree.
 *
 * Arguments:    None
 *
 * Return Value: None
 *
\*****************************************************************************/
void printsol(XPRSprob prob, void* data)
{
  int returnCode = 0;
  int nNodeNum;            /* Current node number */
  int nCol;                /* Number of columns in the original problem */
  double dObjVal;          /* Objective value of the nodal integer solution */
  int i;                   /* Loop counter */

  (void)prob; /* unused */
  (void)data; /* unused */

  /* Get the current node number */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_CURRENTNODE, &nNodeNum) );

  /* Get the objective value of the current integer solution */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjVal) );

  /* Retrieve the postsolved solution values from memory */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTCOLS, &nCol) );
  CHECK_RETURN( XPRSgetcallbacksolution(prob, NULL, gpIntSol, 0, nCol - 1) );

  /* Print the solution to the output file */
  printf("Node %d\n", nNodeNum);
  printf("   Integer solution has objective value %g\n", dObjVal);
  printf("   Postsolved solution values are:\n");
  for (i=0; i<gnCol; i++)
    printf("      x[%d]=%g\n", i, gpIntSol[i]);
  printf("\n");

 cleanup:
  if (returnCode) {
    /* There was an error in the callback. We have to stop the optimizer
     * and also set a global flag that indicates that failure.
     */
    int errorCode = -1;
    char errorMessage[512];
    XPRSgetintattrib(prob, XPRS_ERRORCODE, &errorCode);
    XPRSgetlasterror(prob, errorMessage);
    fprintf(stderr, "Error %d in callback: %s\n", errorCode, errorMessage);
    callbackError = 1;
    XPRSinterrupt(prob, XPRS_STOP_USER);
  }
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

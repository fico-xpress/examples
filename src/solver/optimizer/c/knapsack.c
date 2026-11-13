/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file knapsack.c
   ````````````````
   Apply a primal heuristic to a knapsack problem.
   The program demonstrates the use of the MIP log callback.

   We take the knapsack problem stored in burglar.mps and instigate a
   MIP search. At each node, so long as the current solution is
   both LP optimal and integer infeasible, we truncate the solution
   values to create a feasible integer solution. We then update the
   cutoff, if the new objective value has improved it, and continue
   the search.

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

static void truncsol(XPRSprob prob, void* data, int *feas);
static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);

/* Global variables */
static double *x = NULL;            /* Nodal LP solution values */
static double *gpObjCoef = NULL;    /* Objective function coefficients */
static double gdIntTol;             /* Integer feasibility tolerance */
static int gnCol;                   /* Number of columns */
static int callbackError = 0;       /* Indicates an error in a callback. */

int main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;               /* The problem instance */
  char sProblem[]=DATADIR"/burglar";  /* Problem name */
  char sLogFile[]="knapsack.log";     /* Log file name */

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

  /* Turn off presolve and disallow cuts - to slow solution and allow the
     effect of the heuristic to be seen */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_PRESOLVE, 0) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY, 0) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_HEUREMPHASIS, 0) );

  /* Read the problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /*** Prepare to apply the heuristic ***/

  /* Get the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &gnCol) );

  /* Allocate memory to the coefficient and solution arrays */
  gpObjCoef=malloc(gnCol*sizeof(*gpObjCoef));
  x=malloc(gnCol*sizeof(*x));
  if (!gpObjCoef || !x) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Get the objective function coefficients */
  CHECK_RETURN( XPRSgetobj(prob, gpObjCoef, 0, gnCol-1) );

  /* Get integer feasibility tolerance */
  CHECK_RETURN( XPRSgetdblcontrol(prob, XPRS_MIPTOL, &gdIntTol) );

  /* Tell Optimizer to print MIP information to the log file at each node */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPLOG, 3) );

  /* Tell Optimizer to call truncsol at each node and apply the heuristic */
  CHECK_RETURN( XPRSaddcboptnode(prob, &truncsol, NULL, 0) );

  /*** Perform the MIP search - in the course of which the heuristic will
       be applied ***/
  CHECK_RETURN( XPRSchgobjsense(prob, XPRS_OBJ_MAXIMIZE) );
  printf("Applying a primal heuristic to problem %s:-\n\n", sProblem);
  CHECK_RETURN( XPRSmipoptimize(prob,"") );
  if (callbackError) {
    returnCode = -3;
    goto cleanup;
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
  free(gpObjCoef);
  free(x);

  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

/*****************************************************************************\
 * Name:         truncsol
 *
 * Purpose:      Truncate the LP nodal solution values to create a feasible
 *               integer solution, and update the cutoff value if the new
 *               objective value has improved it. The heuristic is only applied
 *               when the nodal solution is both LP optimal and integer
 *               infeasible.
 *
 *               This function is called at each node in the MIP search.
 *
 * Arguments:    None
 *
 * Return Value: 0
 *
\*****************************************************************************/

void truncsol(XPRSprob prob, void* data, int *feas)
{
  int returnCode = 0;
  int nNodNum;               /* Number of nodes solved */
  int nCol;                  /* Number of columns in the original problem */
  double dObjVal;            /* Objective value */
  double dCutoff;            /* Cutoff value */
  int nLPStatus;             /* LP solution status */
  int nIntInf;               /* Number of integer infeasibilities */
  int i;                     /* Loop counter */
  double dHeurObj;           /* Objective value after the solution values
                                have been truncated*/
  char *sLPStatus[]=         /* LP solution status - literal version */
    {"Optimal","Infeasible","Objective worse than cutoff",
     "Unfinished","Unbounded","Cutoff in dual"};

  (void)data; /* unused */
  (void)feas; /* unnused */

  /* Get the current node number  */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_CURRENTNODE, &nNodNum) );

  /* Get objective value at the current node */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjVal) );

  /* Get the current cutoff value */
  CHECK_RETURN( XPRSgetdblcontrol(prob, XPRS_MIPABSCUTOFF, &dCutoff) );

  /* Get LP solution status and the number of integer infeasibilities */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_LPSTATUS, &nLPStatus) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPINFEAS, &nIntInf) );

  /* Apply heuristic if nodal solution is LP optimal and integer infeasible */
  if (nLPStatus == 1 && nIntInf>0) {

    /* Get LP solution */
    CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTCOLS, &nCol) );
    CHECK_RETURN( XPRSgetcallbacksolution(prob, NULL, x, 0, nCol - 1) );

    /* Truncate each solution value - making allowance for the integer
       tolerance - and calculate the new "heuristic" objective value */
    for (dHeurObj=0, i=0; i<gnCol; i++) {
      dHeurObj += gpObjCoef[i] * (int) (x[i] + gdIntTol);
    }
    printf("   Node %d: Objective Value: ORIGINAL %g;  HEURISTIC %g\n\n",
           nNodNum, dObjVal, dHeurObj);

    /* If the "heuristic" objective value is better, update the cutoff -
       we assume that all the objective coefficents are integers */
    if( dHeurObj > dCutoff) {
      CHECK_RETURN( XPRSsetdblcontrol(prob, XPRS_MIPABSCUTOFF, dHeurObj +
                                      0.9999) );
      printf("              ** Cutoff updated to %g **\n\n", dHeurObj+1.0);
    }
  }

  /* If the nodal solution is not LP optimal do not apply the heuristic */
  else if (nLPStatus != 1) {
    printf("   Node %d: LP solution not optimal, not applying heuristic\n",
           nNodNum);
    printf("           (%s)\n\n", sLPStatus[nLPStatus-1]);
  }

  /* If the nodal solution is integer feasible print the objective value */
  else if (nIntInf == 0)
    printf("   Node %d: Integer solution found: Objective Value %g\n\n",
           nNodNum, dObjVal);
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

/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file roundint.c
   ```````````````
   Apply an integer fixing heuristic to a MIP problem. The program
   also demonstrates the use of the integer solution callback.

   Description: We take the power generation problem stored in hpw15.mps which
   seeks to optimise the operating pattern of a group of electricity
   generators. We solve this MIP with a very loose integer tolerance
   and then fix all binary and integer variables to their rounded
   integer values, changing both their lower and upper bounds. We then
   solve the resulting LP.
   The results are displayed on screen and the problem statistics
   stored in a logfile.

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

static int callbackError = 0;          /* Indicates an error in a callback. */

#define TOL 0.3                        /* Integer tolerance */

static void intsol(XPRSprob, void* data);
static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);


int main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;               /* The problem instance */
  char sProblem[]=DATADIR"/hpw15";    /* Problem name */

  int nCol;                           /* Number of columns */

  /* MIP problem information */
  int nDiscreteCols;                  /* Number of MIP entities: binary,
                                         integer,
                                         semi-continuous and partial integer
                                         variables */
  int nSet;                           /* Number of S1 and S2 sets */
  int *pEntInd = NULL;                 /* Column indices of the MIP entities
                                       */
  char *pEntType = NULL;               /* MIP entity types */

  /* Bound changes */
  int *pBndInd = NULL;                /* Column indices of the bounds to be
                                         changed */
  char *pBndType = NULL;              /* New bound types - always 'B', since
                                         both the upper
                                         and lower bounds are to be changed */
  double *pBndVal = NULL;             /* New bound values */
  int nBnd;                           /* Bound counter */
  int i;                              /* Loop counter */
  int j;                              /* Holder for the bound indices */

  /* Solution information */
  double *x = NULL,*y = NULL;         /* MIP and LP solution values */
  double dObjVal;                     /* Objective function value */

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

  /* Disable cuts - or the optimal solution will be found at the top node
     and the integer solution callback will hardly be used */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY, 0) );

  /* Read the problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem,"") );

  /*** Solve MIP with loose integer tolerance ***/

  /* Set integer feasibility tolerance */
  CHECK_RETURN( XPRSsetdblcontrol(prob, XPRS_MIPTOL, TOL) );

  /* Tell Optimizer to print out MIP information at each node and call
     intsol whenever an integer solution is found */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPLOG, 3) );
  CHECK_RETURN( XPRSaddcbintsol(prob, &intsol, NULL, 0) );

  /* Get the number of columns */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &nCol) );

  /* Allocate memory for MIP solution array and check for memory shortage */
  x = malloc(nCol * sizeof(*x));
  if (!x) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Solve the MIP problem and get the solution values */
  printf("Applying an integer fixing heuristic to problem %s:-\n\n", sProblem);
  printf("Solving MIP:\n\n");
  CHECK_RETURN( XPRSmipoptimize(prob,"") );
  if (callbackError) {
    returnCode = -3;
    goto cleanup;
  }
  CHECK_RETURN (XPRSgetsolution(prob, NULL, x, 0, nCol - 1) );

  /*** Round off the values of all binary and integer variables  ***/

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
  nBnd = 0;

  /* Go through MIP entities */
  for(i=0; i<nDiscreteCols; i++) {

    /* Test whether each is a binary or integer variable */
    if (pEntType[i] == 'B' || pEntType[i] == 'I') {

      /* Hold the index of the variable */
      j = pEntInd[i];

      /* Store the index, round the variable's value to the nearest integer,
         and reset both its upper and lower bounds */
      pBndInd[nBnd]=j;
      pBndVal[nBnd]=(double) (int)(x[j]+0.5);
      pBndType[nBnd]='B';
      nBnd++;
    }
  }

  /* Instruct Optimizer to change the bounds */
  CHECK_RETURN( XPRSchgbounds(prob, nBnd, pBndInd, pBndType, pBndVal) );
  printf("\nAfter MIP solve %d discrete variables were fixed\n\n",
         nBnd);

  /*** Solve the resulting LP ***/

  /* Allocate memory for LP solution array and check for memory shortage */
  y = malloc(nCol * sizeof(*y));
  if (!y) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Solve the LP and get the solution values */
  CHECK_RETURN( XPRSlpoptimize(prob,"") );
  CHECK_RETURN( XPRSgetsolution(prob, NULL, y, 0, nCol - 1) );

  /* Get and print the LP objective function value */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjVal) );
  printf("After solving the LP the final objective value was %g\n\n", dObjVal);

  /* Print the non-zero MIP and corresponding LP solution values for
     comparison:*/
  printf("The non-zero MIP and LP solution values are:\n\n");
  for(i=0; i<nCol; i++) {
    if(x[i])
      printf("   x[%2d] = %5.1f  %5.3g\n", i, x[i], y[i]);
  }
  printf("\n");

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
  free(y);

  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}


/*****************************************************************************\
 * Name:         intsol
 *
 * Purpose:      Displays the objective value associated with a node in the
 *               tree search. This function is called every time an integer
 *               solution has been found.
 *
 * Arguments:    None
 *
 * Return Value: 0
 *
\*****************************************************************************/

void intsol(XPRSprob prob, void* data)
{
  int returnCode = 0;
  int nNode;               /* Node number */
  double dObjVal;          /* Objective function value */

  (void)data; /* unused */

  /* Get the node number */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_CURRENTNODE, &nNode) );

  /* Get the objective function value */
  CHECK_RETURN( XPRSgetdblattrib(prob, XPRS_LPOBJVAL, &dObjVal) );

  /* Print the node number and the associated objective value */
  printf("   Node %d (Objective value = %g)\n", nNode, dObjVal);

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

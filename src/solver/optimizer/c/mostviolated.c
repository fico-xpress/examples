/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file mostviolated.c
   ```````````````````
   A Branching rule branching on the most violated Integer/Binary

   Demonstrate the Xpress-MP change branch callbacks.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

static int callbackError = 0;       /* Indicates an error in a callback. */

typedef struct {
  double* dSolution;
  double dTolerance;
  int    nMipEntities;
  int   *entityColIndex;
  char  *entityColType;
  int    nColumns;
} solutionData;

static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);

/* Branch on the most fractional integer */

static void variableSelection(XPRSprob prob,
  void* data, XPRSbranchobject branch, XPRSbranchobject* p_newbranch)
{
  int returnCode = 0;
  double dDist, dUpDist, dDownDist,  dGreatestDist;
  int iEntity;
  int branchCol = -1;
  solutionData *nodeData = (solutionData*)data;
  XPRSbranchobject newbranch = NULL;

  (void)branch; /* Not interested in what Xpress has planned. */

  /* Get solution in the presolved space. */
  CHECK_RETURN( XPRSgetcallbackpresolvesolution(prob,
    NULL, nodeData->dSolution, 0, nodeData->nColumns - 1) );
  /* Find the most fractional column. */
  dGreatestDist = 0;

  for( iEntity = 0; iEntity < nodeData->nMipEntities; iEntity++ ) {
    int colIndex = nodeData->entityColIndex[iEntity];
    double solVal = nodeData->dSolution[colIndex];
    char entityType = nodeData->entityColType[iEntity];

    /* filter other entity types such as 'S'emi-continuous columns, for
       which the violation is not the fractionality of the LP solution value */
    if ( entityType != 'B' && entityType != 'I')
      continue;

    /* compute the fractionality as the smaller of the up and down fractionality */
    dUpDist = ceil(solVal) - solVal;
    dDownDist = solVal - floor(solVal);
    dDist = fmin(dUpDist, dDownDist);

    /* update the most violated column */
    if( dDist > nodeData->dTolerance && dDist > dGreatestDist ) {
      branchCol = colIndex;
      dGreatestDist = dDist;
    }
  }

  /* If we found a column to branch on then create a branching object
   * (in the presolved space) that reflects branching on that column and
   * return it to the caller.
   */
  if (branchCol >= 0) {
    double bndU = floor(nodeData->dSolution[branchCol]);
    double bndL = ceil(nodeData->dSolution[branchCol]);
    if ( XPRS_bo_create(&newbranch, prob, 0) ||
         XPRS_bo_addbranches(newbranch, 2) ||
         XPRS_bo_addbounds(newbranch, 0, 1, "U", &branchCol, &bndU) ||
         XPRS_bo_addbounds(newbranch, 1, 1, "L", &branchCol, &bndL) )
      goto cleanup;
    *p_newbranch = newbranch;
    newbranch = NULL;
  }

 cleanup:
  if (newbranch != NULL)
    XPRS_bo_destroy(newbranch);
  if (returnCode) {
    /* There was an error in the callback. We have to stop the optimizer
     * and also set a global flag that indicates that failure.
     */
    int errorCode = -1;
    char errorMessage[512] = {0};
    XPRSgetintattrib(prob, XPRS_ERRORCODE, &errorCode);
    XPRSgetlasterror(prob, errorMessage);
    fprintf(stderr, "Error %d in callback: %s\n", errorCode, errorMessage);
    callbackError = 1;
    XPRSinterrupt(prob, XPRS_STOP_USER);
  }
}


int main( int argc, char *argv[] )
{
  int returnCode = 0;
  XPRSprob prob = NULL;
  solutionData nodeData = {0};
  char message[512];                         /* For early error messages */
  char const *modelfile = DATADIR"/burglar"; /* Default model file. */

  /* Check number of arguments and tell user to add an input file otherwise. */
  if (argc != 1 && argc != 2) {
    printf("syntax: mostviolated [filename] \n");
    return 1;
  }
  if (argc == 2)
    modelfile = argv[1];

  /* Initialize the optimizer. */
  if ( XPRSinit("") != 0 ) {
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

  CHECK_RETURN( XPRSreadprob(prob, modelfile, "") );

  /* Turn off dual presolve reductions for this example, otherwise the problem
     is reduced to an empty problem.
   */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPDUALREDUCTIONS, 0) );

  /* solve the LP relaxation of the problem to bring it into presolved state */
  printf("Start solving problem '%s'.\n", modelfile);
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPSTOPSTAGE, XPRS_MIPSTOPSTAGE_INITIALRELAXATION) );
  CHECK_RETURN( XPRSmipoptimize(prob,"") );
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_MIPSTOPSTAGE) );
  if (callbackError) {
    returnCode = -3;
    goto cleanup;
  }

  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPLOG, 3) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY, 0) );

  /* setup user data for branching calback */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPENTS, &(nodeData.nMipEntities)) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS,  &(nodeData.nColumns)) );
  CHECK_RETURN( XPRSgetdblcontrol(prob, XPRS_MIPTOL, &(nodeData.dTolerance)) );
  nodeData.dSolution = malloc(sizeof(double)*nodeData.nColumns);
  nodeData.entityColType = malloc(sizeof(char)*nodeData.nMipEntities);
  nodeData.entityColIndex = malloc(sizeof(int)*nodeData.nMipEntities);
  if (!nodeData.dSolution || !nodeData.entityColIndex || ! nodeData.entityColType) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* store MIP entitites of the presolved problem */
  CHECK_RETURN( XPRSgetmipentities(prob, NULL, NULL,
    nodeData.entityColType, nodeData.entityColIndex, NULL, NULL, NULL, NULL, NULL) );

  CHECK_RETURN( XPRSaddcbchgbranchobject(prob, variableSelection, &nodeData, 0) );

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
      char errorMessage[512];
      XPRSgetintattrib(prob, XPRS_ERRORCODE, &errorCode);
      XPRSgetlasterror(prob, errorMessage);
      fprintf(stderr, "Error %d: %s\n", errorCode, errorMessage);
    }
  }

  /* Free the resources (variables are initialized so that this is valid
   * even in case of error).
   */

  free(nodeData.dSolution);
  free(nodeData.entityColIndex);
  free(nodeData.entityColType);

  XPRSfree();
  printf("Solving complete.\n");
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

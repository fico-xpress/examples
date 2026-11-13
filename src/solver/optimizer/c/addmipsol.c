/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file addmipsol.c
   ````````````````
   Implement a rounding type heuristic in the optnode callback
   to demonstrate how to load solutions into the optimizer.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
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

typedef struct {
  char sProbName[256];
  int nCol;                           /* Number of columns */

  /* Global problem information */
  int nGlEnt;                         /* Number of global entities: binary,
                                         integer, semi-continuous and partial
                                         integer variables */
  int nSet;                           /* Number of S1 and S2 sets */
  int *pGlInd;                        /* Column indices of the global entities
                                       */
  char *pGlType;                      /* Global entity types */
} FixedSearchMainContext;


typedef struct {
  char sProbName[256];
  int nCol;                           /* Number of columns */

  /* Global problem information */
  int nGlEnt;                         /* Number of global entities: binary,
                                         integer, semi-continuous and partial
                                         integer variables */
  int nSet;                           /* Number of S1 and S2 sets */
  int *pGlInd;                        /* Column indices of the global entities
                                       */
  char *pGlType;                      /* Global entity types */

  /* Bound changes */
  int *pBndInd;                       /* Column indices of the bounds to be
                                         changed */
  char *pBndType;                     /* New bound types - always 'B', since
                                         both the upper and lower bounds are to
                                         be changed */
  double *pBndVal;                    /* New bound values */

  /* Solution information */
  double *x;                          /* MIP and LP solution values */
} FixedSearchLocalContext;


/* XPRS optimizer message callback */
static void messagecb(XPRSprob cbprob, void* cbdata,
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

int AllocateLocalContext(FixedSearchMainContext *MainContext,
                          FixedSearchLocalContext *LocalContext)
{
  int returnCode = 0;
  LocalContext->pGlInd = MainContext->pGlInd;
  LocalContext->pGlType = MainContext->pGlType;

  memcpy(LocalContext->sProbName, MainContext->sProbName,
         sizeof(LocalContext->sProbName));
  LocalContext->nCol = MainContext->nCol;
  LocalContext->nGlEnt = MainContext->nGlEnt;
  LocalContext->nSet = MainContext->nSet;

  /* Allocate memory for MIP solution array and bound arrays */
  LocalContext->x=malloc(LocalContext->nCol * sizeof(*LocalContext->x));
  LocalContext->pBndInd=malloc(LocalContext->nGlEnt *
                               sizeof(*LocalContext->pBndInd));
  LocalContext->pBndVal=malloc(LocalContext->nGlEnt *
                               sizeof(*LocalContext->pBndVal));
  LocalContext->pBndType=malloc(LocalContext->nGlEnt *
                                sizeof(*LocalContext->pBndType));
  if (!LocalContext->x || !LocalContext->pBndInd ||
      !LocalContext->pBndVal || !LocalContext->pBndType)
  {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }
 cleanup:
  return returnCode;
}

void FreeLocalContext(FixedSearchLocalContext *LocalContext)
{
  free(LocalContext->x);
  free(LocalContext->pBndInd);
  free(LocalContext->pBndVal);
  free(LocalContext->pBndType);
}

int AllocateMainContext(XPRSprob prob, FixedSearchMainContext *Context)
{
  int returnCode = 0;

  CHECK_RETURN( XPRSgetprobname(prob, Context->sProbName) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_COLS, &Context->nCol) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPENTS, &Context->nGlEnt) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_SETS, &Context->nSet) );

  /* Allocate memory for global entity arrays */
  Context->pGlInd=malloc(Context->nCol * sizeof(*Context->pGlInd));
  Context->pGlType=malloc(Context->nCol * sizeof(*Context->pGlType));
  if (!Context->pGlInd || !Context->pGlType) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Retrieve global entity information */
  CHECK_RETURN( XPRSgetmipentities(prob, &Context->nGlEnt, &Context->nSet,
                                   Context->pGlType, Context->pGlInd,
                                   NULL, NULL, NULL, NULL, NULL) );
 cleanup:
  return returnCode;
}

void FreeMainContext(FixedSearchMainContext *Context)
{
  free(Context->pGlInd);
  free(Context->pGlType);
}

int FixedSearch(XPRSprob parent, FixedSearchMainContext *MainContext)
{
  int returnCode = 0;
  int nBnd, i, j, iMipInfeas, iMipSols, bHasMipIncumbent, nOptnodeCalled;
  XPRSprob probg = NULL, emptyprob = NULL;
  double dRounded, dMipTol, dMipIncumbent;
  char sTempProbName[256 + 32 + 1];
  FixedSearchLocalContext LocalContext;
  int mipThreadID;

  /* Only run once per node (since adding a solution will call the callback to
     be fired again) */
  CHECK_RETURN( XPRSgetintattrib(parent, XPRS_CALLBACKCOUNT_OPTNODE,
                                 &nOptnodeCalled) );
  if(nOptnodeCalled > 1) {
    goto cleanup;
  }

  /* Get the id of the current thread. */
  CHECK_RETURN( XPRSgetintattrib(parent, XPRS_MIPTHREADID, &mipThreadID) );

  /* Only run the search for solutions with small numbers of integer
     infeasibilities */
  CHECK_RETURN( XPRSgetintattrib(parent, XPRS_MIPINFEAS, &iMipInfeas) );
  if(iMipInfeas > 0.25*MainContext->nGlEnt) {
    goto cleanup;
  }

  /* Create resources we need */
  CHECK_RETURN( AllocateLocalContext(MainContext, &LocalContext) );

  /* Create a problem containing the original problem */
  CHECK_RETURN( XPRScreateprob(&probg) );
  CHECK_RETURN( XPRScopyprob(probg, parent, LocalContext.sProbName) );
  CHECK_RETURN( XPRSpostsolve(probg) );

  /* Reset the controls of the problem and restore the name of the problem */
  CHECK_RETURN( XPRScreateprob(&emptyprob) );
  CHECK_RETURN( XPRScopycontrols(probg, emptyprob) );
  XPRSdestroyprob(emptyprob);

  /* Rename the problem so we don't get collisions with temporary files
     produced by the optimizer */
  snprintf(sTempProbName, sizeof(sTempProbName),
           "%s_%d", LocalContext.sProbName, mipThreadID);
  CHECK_RETURN( XPRSsetprobname(probg, sTempProbName) );

  /* Tell Optimizer to call Message whenever a message is output */
  CHECK_RETURN( XPRSaddcbmessage(probg, messagecb, &LocalContext, 0) );

  /* Get information from the solution at the current node of 'parent' problem
   */
  CHECK_RETURN( XPRSgetintattrib(parent, XPRS_MIPSOLS, &bHasMipIncumbent) );
  CHECK_RETURN( XPRSgetdblattrib(parent, XPRS_MIPOBJVAL, &dMipIncumbent) );

  CHECK_RETURN( XPRSgetcallbacksolution(parent, NULL, LocalContext.x, 0, LocalContext.nCol - 1) );

  /* Initialise bound counter */
  nBnd = 0;

  CHECK_RETURN( XPRSgetdblcontrol(parent, XPRS_MIPTOL, &dMipTol) );

  /* Go through global entities */
  for(i=0; i<LocalContext.nGlEnt; i++) {
    /* Test whether each is a binary or integer variable */
    if (LocalContext.pGlType[i] == 'B' || LocalContext.pGlType[i] == 'I') {
      j = LocalContext.pGlInd[i];

      /* Fix variables that are fractional */
      dRounded = (double) (int)(LocalContext.x[j]+0.5);
      if(fabs(dRounded - LocalContext.x[j]) <= dMipTol) {
        continue;
      }
      LocalContext.pBndInd[nBnd] = j;
      LocalContext.pBndVal[nBnd] = dRounded;
      LocalContext.pBndType[nBnd] = 'B';
      nBnd++;
    }
  }

  /* Instruct Optimizer to change the bounds */
  CHECK_RETURN( XPRSchgbounds(probg, nBnd, LocalContext.pBndInd,
                              LocalContext.pBndType, LocalContext.pBndVal) );
  printf("%d:After global the %d binary and integer variables were fixed\n",
         mipThreadID, nBnd);

  /*
    Ensure we only do a small search in the fixed problem and
    use the current incumbent objective from the main problem
  */
  CHECK_RETURN( XPRSsetintcontrol(probg, XPRS_MAXNODE, 100) );
  CHECK_RETURN( XPRSsetintcontrol(probg, XPRS_MAXMIPSOL, 1) );
  if(bHasMipIncumbent) {
    /* Set a cut-off to help the fixed search */
    CHECK_RETURN( XPRSsetdblcontrol(probg, XPRS_MIPABSCUTOFF, dMipIncumbent) );
  }
  CHECK_RETURN( XPRSsetintcontrol(probg, XPRS_MIPLOG, 3) );

  /* Read the basis for the LP relaxation to reduce run time */
  CHECK_RETURN( XPRSreadbasis(probg, LocalContext.sProbName,"") );

  /* Run the optimizer */
  CHECK_RETURN( XPRSmipoptimize(probg,"") );

  /* If we found a solution then load it into the main problem */
  CHECK_RETURN( XPRSgetintattrib(probg, XPRS_MIPSOLS, &iMipSols) );
  if(iMipSols > 0) {
    CHECK_RETURN( XPRSgetsolution(probg, NULL, LocalContext.x, 0, LocalContext.nCol - 1) );
    CHECK_RETURN( XPRSaddmipsol(parent, LocalContext.nCol, LocalContext.x,
                                NULL, NULL) );
  }

  /* Free the resources we have created */
  XPRSdestroyprob(probg);
  FreeLocalContext(&LocalContext);

 cleanup:
  return returnCode;
}

void OptNodeCb(XPRSprob my_prob, void *my_object, int *feas)
{
  FixedSearchMainContext *Context = my_object;
  (void)feas; /* unused */
  FixedSearch(my_prob, Context);
}

int main(void) {
  int returnCode = 0;
  XPRSprob prob = NULL;
  const char *sProbName = DATADIR"/addmipsol";
  FixedSearchMainContext Context;

  memset(&Context, 0, sizeof(Context));

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
  CHECK_RETURN( XPRSaddcbmessage(prob, messagecb, &Context, 0) );

  CHECK_RETURN( XPRSreadprob(prob, sProbName,"") );


  /* Register the fixed search routine to the optimal node callback */
  CHECK_RETURN( XPRSaddcboptnode(prob, OptNodeCb, &Context, 0) );

  /* Make the problem difficult for the optimizer to solve */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_CUTSTRATEGY,
                                  XPRS_CUTSTRATEGY_NONE) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_HEUREMPHASIS, 0) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_SBBEST, 0) );
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPTHREADS, 20) );

  /* Log every node in the branch-and-bound search */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPLOG, 3) );

  /* Setup some resources we need in the callbacks */
  CHECK_RETURN( AllocateMainContext(prob, &Context) );

  /* Solve the root node relaxation and write out the basis (to reduce solve
     time of the fixed problems) */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPSTOPSTAGE, XPRS_MIPSTOPSTAGE_INITIALRELAXATION) );
  CHECK_RETURN( XPRSmipoptimize(prob,"") );
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_MIPSTOPSTAGE) );
  CHECK_RETURN( XPRSwritebasis(prob,"","") );

  /* Run the MIP search */
  CHECK_RETURN( XPRSmipoptimize(prob, "") );

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
  FreeMainContext(&Context);
  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

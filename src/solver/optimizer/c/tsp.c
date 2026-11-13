/***************************************************************

  Travelling Salesman Problem
  ===========================

  Solves the classic travelling salesman problem as a MIP,
  where sub-tour elimination constraints are added
  only as needed during the branch-and-bound search.

  Demonstrates:
  - Loading of an initial, feasible MIP solution.
  - Adding external constraints during branch-and-bound.

  (c) 2021-2026 Fair Isaac Corporation
*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include "xprs.h"                 /* Optimizer header file */

/* Fixed constants. */
#define NUM_CITIES    10        /* Number of cities to visit. Make sure this
                                   is small when SubtourTypes is 0 or 1.
                                */
#define MAX_COORD     100       /* Maximum magnitude of each coordinate.
                                   The coordinates for the nodes in our TSP
                                   are drawn at random from this range. */
static int SubtourTypes = 2;    /* 0 - normal rows,
                                   1 - delayed rows,
                                   2 - cuts.*/

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

/* XPRS optimizer message callback */
static void cbMessage(XPRSprob cbprob, void* cbdata,
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

/* Create a feasible tour and add this as initial MIP solution. */
static int CreateInitialTour(XPRSprob prob)
{
  int returnCode = 0;
  int i;
  double tour[NUM_CITIES*NUM_CITIES];

  /* Create a tour that visits all cities in order. */
  memset(tour, 0, sizeof(tour));
  /* Travel from each city i to city i+1... */
  for (i = 0; i < NUM_CITIES - 1; i++)
    tour[i*NUM_CITIES + i+1] = 1.0;
  /* ... and complete the tour. */
  tour[(NUM_CITIES-1)*NUM_CITIES + 0] = 1.0;

  /* Load the starting solution. */
  CHECK_RETURN( XPRSaddmipsol(prob, sizeof(tour) / sizeof(tour[0]), tour,
                              NULL, "init_tour") );
 cleanup:
  return returnCode;
}

/* Load the TSP problem formulation into the Xpress problem structure.
   This only creates the constraints that make sure each city is entered
   and left exactly once.
   The constraints that ensure there is no subtour are created either in
   AddSubtourEliminationConstraints() or are dynamically separated in
   the cbOptNode() callback.
*/
static int CreateTSPProblem(XPRSprob prob)
{
  int returnCode = 0;
  int i, j;
  double dist[NUM_CITIES*NUM_CITIES];  /* distance matrix */
  double xLoc[NUM_CITIES];             /* x-coordinate of nodes */
  double yLoc[NUM_CITIES];             /* y-coordinate of nodes */
  double visitCoef[NUM_CITIES];        /* buffer for a row's non-zero values */
  int visitIdx[NUM_CITIES];            /* buffer for a row's non-zero indices */

  /* Sprinkle cities randomly in the plane.
     We use the C library's rand() function which returns a random integer
     in [0,RAND_MAX] */
  for (i = 0; i < NUM_CITIES; i++) {
    xLoc[i] = MAX_COORD * ((double)rand() / RAND_MAX);
    yLoc[i] = MAX_COORD * ((double)rand() / RAND_MAX);
  }
  /* Calculate the distance matrix.
     Our problem is symmetric, so the distance (i,j) is the same as (j,i) */
  for (i = 0; i < NUM_CITIES; i++) {
    dist[i*NUM_CITIES + i] = 0.0;
    for (j = 0; j < i; j++) {
      double d = sqrt((xLoc[i] - xLoc[j])*(xLoc[i] - xLoc[j]) +
                      (yLoc[i] - yLoc[j])*(yLoc[i] - yLoc[j]));
      dist[i*NUM_CITIES + j] = d;
      dist[j*NUM_CITIES + i] = d;
    }
  }

  /* Create the variables, which are binaries that indicate if we travel from
     city i to city j in our tour.
     The trip (i->j) will have column index i*(NUM_CITIES-1)+j. */
  for (i = 0; i < NUM_CITIES; i++) {
    for (j = 0; j < NUM_CITIES; j++) {
      /* Add the binary representing the trip (i->j) with an objective of
         minimizing total distance. */
      double lb = 0.0;
      double ub = 1.0;
      char colType = 'B';
      int colIdx = i*NUM_CITIES + j;
      char name[64];
      if (j == i) {
        /* self-loops are forbidden, they are non-optimal anyway */
        ub = 0.0;
      }
      CHECK_RETURN( XPRSaddcols(prob, 1, 0, &dist[colIdx], NULL, NULL, NULL,
                                &lb, &ub) );
      if (j != i)
        CHECK_RETURN( XPRSchgcoltype(prob, 1, &colIdx, &colType) );
      snprintf(name, sizeof(name), "travel(%i,%i)", i, j);
      CHECK_RETURN( XPRSaddnames(prob, 2/*col names*/, name, colIdx, colIdx) );
    }
  }

  /* Create constraints to ensure that each city is visited only once. */
  for (i = 0; i < NUM_CITIES; i++) {
    int nCoef;
    int const visitStart = 0;
    double const visitRhs = 1.0;
    char const visitSense = 'E';
    /* Create a constraint to ensure that we leave each city exactly once. */
    nCoef = 0;
    for (j = 0; j < NUM_CITIES; j++) {
      if (j == i)
        continue;
      visitCoef[nCoef] = 1.0;
      visitIdx[nCoef] = i*NUM_CITIES + j;
      nCoef += 1;
    }
    CHECK_RETURN( XPRSaddrows(prob, 1, nCoef, &visitSense, &visitRhs, NULL,
                              &visitStart, visitIdx, visitCoef) );
    /* Create a constraint to ensure that we enter each city exactly once. */
    nCoef = 0;
    for (j = 0; j < NUM_CITIES; j++) {
      if (j == i)
        continue;
      visitCoef[nCoef] = 1.0;
      visitIdx[nCoef] = j*NUM_CITIES + i;
      nCoef += 1;
    }
    CHECK_RETURN( XPRSaddrows(prob, 1, nCoef, &visitSense, &visitRhs, NULL,
                              &visitStart, visitIdx, visitCoef) );
  }
 cleanup:
  return returnCode;
}

/*
 * Adds the exponential number of subtour elimination constraints.
 *
 * We split the set of all cities S into two disjoint sets (T, S\T) and create
 * constraints that require that we travel at least once from the set T to the
 * set S\T.
 *
 * This simple implementation has complexity O(n^2*2^n), where n=NUM_CITIES,
 * so make sure to run it for tiny values of NUM_CITIES only.
 */
static int AddSubtourEliminationConstraints(XPRSprob prob, int useDelayedRows)
{
  int returnCode = 0;
  int isMember[NUM_CITIES];
  double visitCoef[NUM_CITIES*NUM_CITIES];
  int visitIdx[NUM_CITIES*NUM_CITIES];

#if NUM_CITIES == 0
  return 0;
#endif

  /* Enumerate all possible subtours. */
  memset(isMember, 0, sizeof(isMember));
  while (1) {
    int i, j;
    int k;
    int numMembers;
    int nCoef;
    char visitType;
    double visitRhs;
    int visitStart;

    k = 0;
    isMember[0] += 1;
    while (isMember[k] > 1) {
      isMember[k] = 0;
      isMember[k + 1] += 1;
      k++;
    }

    /* Check that we haven't enumerated all possibilities. */
    numMembers = 0;
    for (i = 0; i < NUM_CITIES; i++)
      numMembers += isMember[i];
    if (numMembers == NUM_CITIES)
      break;
    /* We don't need to create constraints for subsets containing more than
       half the cities, due to symmetries. */
    if (numMembers > NUM_CITIES / 2)
      continue;

    /* Create the sub-tour elimination constraint. */
    nCoef = 0;
    for (i = 0; i < NUM_CITIES; i++) {
      if (!isMember[i])
        continue;
      for (j = 0; j < NUM_CITIES; j++) {
        if (isMember[j])
          continue;
        visitCoef[nCoef] = 1.0;
        visitIdx[nCoef] = i*NUM_CITIES + j;
        nCoef += 1;
      }
    }
    visitType = 'G';
    visitRhs = 1.0;
    visitStart = 0;
    CHECK_RETURN( XPRSaddrows(prob, 1, nCoef, &visitType, &visitRhs, NULL,
                              &visitStart, visitIdx, visitCoef) );
    /* If delayed rows are requested then mark the row we just created
       as delayed row. */
    if (useDelayedRows) {
      int xprs_rows;
      int rowIdx;
      CHECK_RETURN( XPRSgetintattrib(prob, XPRS_ROWS, &xprs_rows) );
      rowIdx = xprs_rows - 1;
      CHECK_RETURN( XPRSloaddelayedrows(prob, 1, &rowIdx) );
    }
  }
 cleanup:
  return returnCode;
}

/*
 * Callback function used for catching heuristic solutions that contain
 * invalid subtours.
 * This callback is invoked before an integer solution is accepted. It gives
 * us a chance to reject the solution by setting *p_ifReject to a non-zero
 * value.
 */
static void cbPreIntSol(XPRSprob prob, void *data, int solType,
                                int *p_ifReject, double *p_newCutoff)
{
  int returnCode = 0;
  int i, j;
  int numCities;
  int nextCity[NUM_CITIES];
  double mipSol[NUM_CITIES*NUM_CITIES];

  (void)data;        /* unused */
  (void)p_newCutoff; /* unused: we don't find improving solutions */

  /* Get the current binary solution and translate it into a tour. */
  CHECK_RETURN( XPRSgetcallbacksolution(prob, NULL, mipSol, 0, NUM_CITIES*NUM_CITIES-1) );
  for (i = 0; i < NUM_CITIES; i++) nextCity[i] = -1;
  for (i = 0; i < NUM_CITIES; i++) {
    for (j = 0; j < NUM_CITIES; j++) {
      if (mipSol[i*NUM_CITIES + j] > 0.5) {
        nextCity[i] = j;
      }
    }
  }

  /* Count the number of cities in the first tour. */
  numCities = 1;
  printf("Check tour 0");
  for (i = 0; nextCity[i] != 0; i = nextCity[i]) {
    printf(" -> %d", nextCity[i]);
    numCities += 1;
  }
  printf(" -> 0");
  if (numCities < NUM_CITIES) {
    printf(" too short (%d vs %d citites): ", numCities, NUM_CITIES);
    /* We don't have a full tour so reject the solution.
       If solType is non-zero then we reject by setting *p_ifReject=1.
       If instead solType is zero then the solution came from an integral
       node. In this case we can reject by adding a cut that cuts off that
       solution. Note that we must NOT set *p_ifReject=1 in that case because
       that would result in just dropping the node, no matter whether we add
       cuts or not.
     */
    if (solType != 0) {
      printf("reject\n");
      *p_ifReject = 1;
    }
    else {
      /* This solution was reported for an intergral node. So we can provide
         a cut that cuts off that solution and prevents us from finding that
         solution again.
       */
      int isTour[NUM_CITIES];
      int numCoef;
      int cutType;
      char cutSense;
      int cutStart[2];
      double cutCoef[NUM_CITIES*NUM_CITIES];
      int cutIdx[NUM_CITIES*NUM_CITIES];
      int numCoefPresolved;
      double cutRhsPresolved;
      double cutCoefPresolved[NUM_CITIES*NUM_CITIES];
      int cutIdxPresolved[NUM_CITIES*NUM_CITIES];
      int status;

      printf("cut\n");

      /* Identify which cities are part of the subtour. */
      memset(isTour, 0, NUM_CITIES * sizeof(*isTour));
      numCities = 0;
      j = 0;
      do {
        i = nextCity[j];
        isTour[j] = 1;
        numCities += 1;
        nextCity[j] = -1;
        j = i;
      } while (nextCity[j] >= 0);

      /* Create a subtour elimination cut. */
      numCoef = 0;
      for (i = 0; i < NUM_CITIES; i++) {
        if (!isTour[i]) continue;
        for (j = 0; j < NUM_CITIES; j++) {
          if (isTour[j]) continue;
          cutCoef[numCoef] = 1.0;
          cutIdx[numCoef] = i*NUM_CITIES + j;
          numCoef += 1;
        }
      }

      /* Before adding the cut, we must translate it to the presolved model.
         If this translation fails then we cannot continue. The translation
         can only fail if we have presolve operations enabled that should be
         disabled in case of dynamically separated constraints. */
      cutSense = 'G';
      cutType = 1; /* arbitrary number that can be used to identify our cuts */
      CHECK_RETURN( XPRSpresolverow(prob, cutSense, numCoef, cutIdx, cutCoef,
                                    1.0, NUM_CITIES*NUM_CITIES,
                                    &numCoefPresolved, cutIdxPresolved,
                                    cutCoefPresolved, &cutRhsPresolved,
                                    &status) );
      if (status != 0) {
        fprintf(stderr,
                "Possible presolve operation prevented the proper translation of a subtour constraint, with status %i",
                status);
        abort();
      }
      cutStart[0] = 0;
      cutStart[1] = numCoefPresolved;
      CHECK_RETURN( XPRSaddcuts(prob, 1, &cutType, &cutSense, &cutRhsPresolved,
                                cutStart, cutIdxPresolved, cutCoefPresolved) );
    }
  }
  else {
    printf(" accept\n");
  }

 cleanup:
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


/* Print the current MIP solution */
static int PrintSolution(XPRSprob prob)
{
  int returnCode = 0;
  int i, j;
  int nextCity[NUM_CITIES];
  double mipSol[NUM_CITIES*NUM_CITIES];

  CHECK_RETURN( XPRSgetsolution(prob, NULL, mipSol, 0, NUM_CITIES*NUM_CITIES-1) );
  for (i = 0; i < NUM_CITIES; i++)
    nextCity[i] = -1;
  for (i = 0; i < NUM_CITIES; i++) {
    for (j = 0; j < NUM_CITIES; j++) {
      if (mipSol[i*NUM_CITIES + j] > 0.5) {
        nextCity[i] = j;
      }
    }
  }

  printf("Tour: %i", 0);
  for (i = 0; nextCity[i] != 0; i = nextCity[i]) printf(" -> %i", nextCity[i]);
  printf(" -> 0\n");
 cleanup:
  return returnCode;
}

int main(void)
{
  int returnCode = 0;
  int xprs_mipstatus;
  XPRSprob prob = NULL;

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
  CHECK_RETURN( XPRSaddcbmessage(prob, cbMessage, NULL, 0) );

  /* Create the basic TSP problem. */
  CHECK_RETURN( CreateTSPProblem(prob) );

  if ( SubtourTypes <= 1 ) {
    /* Add all subtour elimination constraints from the start. */
    CHECK_RETURN( AddSubtourEliminationConstraints(prob, (SubtourTypes == 1)) );
  }
  else {
    /* We are going to create our subtour elimination constraints dynamically
       during the solve, so ... */
    /* ... disable any presolve operations that conflict with not having the
       entire problem definition present ... */
    CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_MIPDUALREDUCTIONS, 0) );
    /* ... add a callback for filtering invalid solutions. In case of invalid
       solutions from integral nodes, it will also add a cut to avoid
       finding that solution again. */
    CHECK_RETURN( XPRSaddcbpreintsol(prob, cbPreIntSol, NULL, 0) );
  }

  /* Let's write out what we have created so far. */
  CHECK_RETURN( XPRSwriteprob(prob, "problem", "l") );

  /* Create a feasible starting tour. */
  CHECK_RETURN( CreateInitialTour(prob) );

  /* Solve the TSP! */
  CHECK_RETURN( XPRSmipoptimize(prob, "") );
  if (callbackError) {
    returnCode = -3;
    goto cleanup;
  }

  /* Check if we managed to solve our problem. */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_MIPSTATUS, &xprs_mipstatus) );
  switch (xprs_mipstatus) {
  case XPRS_MIP_OPTIMAL:
    printf("Optimal tour found:\n");
    CHECK_RETURN( PrintSolution(prob) );
    break;
  case XPRS_MIP_SOLUTION:
    printf("Solve was interrupted. Best tour found:\n");
    CHECK_RETURN( PrintSolution(prob) );
    break;
  case XPRS_MIP_INFEAS:
    printf("Problem unexpectedly found to be infeasible.\n");
    break;
  case XPRS_MIP_LP_NOT_OPTIMAL:
  case XPRS_MIP_NO_SOL_FOUND:
    printf("Solve was interrupted without a solution found.\n");
    break;
  default:
    printf("Unexpected solution status (%i).\n", xprs_mipstatus);
  }

 cleanup:
  if (returnCode != 0) {
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
  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

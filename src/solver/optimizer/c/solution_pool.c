/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file solution_pool.c
   ````````````````````
   Capacitated Facility Location Problem (CFLP) demonstrating the
   solution pool and solution enumeration.

   25 customers, 10 potential facility sites with integer coordinates.
   Single-sourcing: each customer is assigned to exactly one open
   facility. Service cost equals the Euclidean distance from the
   customer to its assigned facility. Fixed opening cost is correlated
   with facility capacity.

   Demonstrates four uses of the built-in solution pool and enumeration
   controls on the CFLP model:
     - collecting up to ten solutions found during a normal MIP solve
       (POOLLIMIT);
     - warm-starting a re-solve from the pool after fixing a facility
       open (POOLKEEPSOLS);
     - enumerating the five best solutions overall (POOLENUMSTRATEGY,
       POOLRELGAP);
     - enumerating the five best facility assignments only, by
       restricting enumeration to the facility variables
       (XPRSloadbranchdirs).
   After each solve, the objective value, number of open facilities,
   and maximum service distance are printed for each collected solution.

   Model
   -----
   Sets
       I = {0 ... N_CUST-1}   customers
       J = {0 ... N_FAC-1}    potential facility sites

   Parameters
       d_i      demand of customer i
       f_j      fixed cost of opening facility j
       Q_j      capacity of facility j
       c_ij     Euclidean distance from customer i to facility j

   Decision variables
       y_j  in {0,1}   1 iff facility j is opened
       x_ij in {0,1}   1 iff customer i is served by facility j

   Objective
       min  sum_j f_j y_j  +  sum_i sum_j c_ij x_ij

   Constraints
       (1)  sum_j x_ij = 1               for all i  (demand satisfaction)
       (2)  sum_i d_i x_ij <= Q_j y_j    for all j  (capacity + activation)

   (c) 2026 Fair Isaac Corporation
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xprs.h"

/* XPRS optimizer message callback */
static void XPRS_CC messagecb(XPRSprob cbprob, void *cbdata,
  const char *msg, int len, int msgtype)
{
  (void)cbprob;  /* unused (the problem for which the message is issued) */
  (void)cbdata;  /* unused (the data passed to XPRSaddcbmessage()) */
  switch (msgtype)
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

/* ---- Instance data ---------------------------------------------------- */

#define N_CUST  25
#define N_FAC   10

static const int cust_xy[N_CUST][2] = {
  {20,  9}, {22, 15}, { 5, 13}, { 6, 26}, {13, 14}, /* cluster A -- south-west */
  {15, 19}, {20, 19}, {11,  7}, { 3, 21},
  {70, 27}, {69, 33}, {82, 26}, {74, 30}, {77, 20}, /* cluster B -- south-east */
  {82, 11}, {72, 25}, {70, 24},
  {56, 82}, {53, 74}, {50, 84}, {39, 81}, {55, 80},  /* cluster C -- north */
  {43, 84},
  {46, 47}, {30, 55}                                 /* outliers  -- central */
};

static const int demands[N_CUST] = {
  26, 25, 37, 22, 35, 22, 43, 21, 41,   /* cluster A */
  29, 61, 32, 41, 40, 39, 19, 72,       /* cluster B */
  23, 40, 59, 27, 45, 25,               /* cluster C */
  21, 40                                /* outliers  */
};

static const int fac_xy[N_FAC][2] = {
  { 7, 15}, {12, 19},  /* j=0,1 -- near cluster A */
  {76, 19}, {80, 21},  /* j=2,3 -- near cluster B */
  {56, 88}, {52, 79},  /* j=4,5 -- near cluster C */
  {45, 50}, {63, 48},  /* j=6,7 -- central bridge */
  { 3, 93}, { 3, 88}   /* j=8,9 -- peripheral    */
};

static const int capacities[N_FAC]  = {307, 98, 86, 298, 211, 190, 70, 205, 140, 165};
static const int fixed_costs[N_FAC] = {787, 233, 257, 756, 567, 517, 227, 535, 367, 445};


/* Struct for collecting model data, such as mapping of variables to column indices. */
struct model_data_s {
  int y[N_FAC];
  int x[N_CUST][N_FAC];
  double dist[N_CUST][N_FAC];
};


/* Builds the CFLP model on prob and fills modelData with the mappings
 * from variables to column indices and the distance service costs.
 * Returns 0 on success, non-zero on failure. */
static int build_model(XPRSprob prob, struct model_data_s *modelData)
{
  int returnCode = 0;

  int cols, firstCol, nextCol;

  int     rowLength;
  int    *rowIndices = NULL;
  double *rowCoefs   = NULL;

  /* ---- Compute service costs ---- */
  for (int i = 0; i < N_CUST; i++) {
    for (int j = 0; j < N_FAC; j++) {
      double dx             = (double)(cust_xy[i][0] - fac_xy[j][0]);
      double dy             = (double)(cust_xy[i][1] - fac_xy[j][1]);
      modelData->dist[i][j] = sqrt(dx * dx + dy * dy);
    }
  }

  /* ---- Start with an empty problem ---- */
  CHECK_RETURN(XPRSloadlp(prob, "CFLP", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

  /* ---- Build the variables ---- */
  /* Add the N_FAC facility binary variables. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &firstCol));
  nextCol = firstCol;
  for (int j = 0; j < N_FAC; j++) {
    modelData->y[j] = nextCol++;
  }
  CHECK_RETURN(XPRSaddcols(prob, nextCol - firstCol, 0, NULL, NULL, NULL, NULL, NULL, NULL));
  for (int j = 0; j < N_FAC; j++) {
    char colType = 'B';
    CHECK_RETURN(XPRSchgcoltype(prob, 1, &modelData->y[j], &colType));
    char name[32];
    snprintf(name, sizeof(name), "y(%i)", j);
    CHECK_RETURN(XPRSaddnames(prob, XPRS_NAMES_COLUMN, name, modelData->y[j], modelData->y[j]));
  }
  /* Add the N_CUST*N_FAC service binary variables. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &firstCol));
  nextCol = firstCol;
  for (int i = 0; i < N_CUST; i++) {
    for (int j = 0; j < N_FAC; j++) {
      modelData->x[i][j] = nextCol++;
    }
  }
  CHECK_RETURN(XPRSaddcols(prob, nextCol - firstCol, 0, NULL, NULL, NULL, NULL, NULL, NULL));
  for (int i = 0; i < N_CUST; i++) {
    for (int j = 0; j < N_FAC; j++) {
      char colType = 'B';
      CHECK_RETURN(XPRSchgcoltype(prob, 1, &modelData->x[i][j], &colType));
      char name[32];
      snprintf(name, sizeof(name), "x(%i,%i)", i, j);
      CHECK_RETURN(XPRSaddnames(prob, XPRS_NAMES_COLUMN, name, modelData->x[i][j], modelData->x[i][j]));
    }
  }

  /* ---- Build the constraints ---- */
  /* Allocate space for building a single constraint. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &cols));
  rowIndices = malloc(cols * sizeof(int));
  rowCoefs   = malloc(cols * sizeof(double));
  if ((rowCoefs == NULL) || (rowIndices == NULL)) {
    fprintf(stderr, "malloc failure");
    returnCode = -1;
    goto cleanup;
  }
  /* Demand satisfaction constraints. */
  /*    sum_j x_ij = 1 for all i */
  for (int i = 0; i < N_CUST; i++) {
    int rowStart = 0;
    rowLength    = 0;
    for (int j = 0; j < N_FAC; j++) {
      rowIndices[rowLength] = modelData->x[i][j];
      rowCoefs[rowLength]   = 1.0;
      rowLength++;
    }
    char   rowType = 'E';
    double rowRHS  = 1.0;
    CHECK_RETURN(XPRSaddrows(prob, 1, rowLength, &rowType, &rowRHS, NULL, &rowStart, rowIndices, rowCoefs));
  }
  /* Capacity and activation constraints. */
  /*    sum_i d_i x_ij <= Q_j y_j for all j */
  for (int j = 0; j < N_FAC; j++) {
    int rowStart = 0;
    rowLength    = 0;
    for (int i = 0; i < N_CUST; i++) {
      rowIndices[rowLength] = modelData->x[i][j];
      rowCoefs[rowLength]   = demands[i];
      rowLength++;
    }
    rowIndices[rowLength] = modelData->y[j];
    rowCoefs[rowLength]   = -capacities[j];
    rowLength++;
    char   rowType = 'L';
    double rowRHS  = 0.0;
    CHECK_RETURN(XPRSaddrows(prob, 1, rowLength, &rowType, &rowRHS, NULL, &rowStart, rowIndices, rowCoefs));
  }

  /* ---- Build the objective function ---- */
  /*    sum_j f_j y_j  +  sum_i sum_j c_ij x_ij */
  rowLength = 0;
  for (int j = 0; j < N_FAC; j++) {
    rowIndices[rowLength] = modelData->y[j];
    rowCoefs[rowLength]   = fixed_costs[j];
    rowLength++;
  }
  for (int i = 0; i < N_CUST; i++) {
    for (int j = 0; j < N_FAC; j++) {
      rowCoefs[rowLength]   = modelData->dist[i][j];
      rowIndices[rowLength] = modelData->x[i][j];
      rowLength++;
    }
  }
  CHECK_RETURN(XPRSchgobj(prob, rowLength, rowIndices, rowCoefs));

cleanup:

  if (rowIndices)
    free(rowIndices);
  if (rowCoefs)
    free(rowCoefs);

  return returnCode;
}


/* Print the list of collected solutions. We list the following information:
 *   - Objective function value
 *   - Number of facilities opened
 *   - Maximum distance between a facility and a customer it serves.
 */
static int print_solutions(XPRSprob prob, struct model_data_s *modelData)
{
  int returnCode = 0;

  int    inputCols;
  int    nOpen;
  int    poolSize, poolStatus;
  double poolObj, maxDist;

  double *sol = NULL;

  /* ---- Solution array ---- */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_INPUTCOLS, &inputCols));
  sol = malloc(inputCols * sizeof(double));
  if (sol == NULL) {
    fprintf(stderr, "malloc failure");
    returnCode = -1;
    goto cleanup;
  }

  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_POOLSIZE, &poolSize));
  printf("\nCollected %d solution(s):\n", poolSize);
  printf("  %4s  %12s  %11s  %9s\n", "Sol#", "Objective", "Facilities", "Max dist");
  for (int p = 0; p < poolSize; p++) {
    CHECK_RETURN(XPRSgetpoolsolobjval(prob, p, &poolStatus, &poolObj));
    CHECK_RETURN(XPRSgetpoolsolution(prob, p, &poolStatus, sol, 0, inputCols-1));
    nOpen = 0;
    maxDist = 0.0;
    for (int j = 0; j < N_FAC; j++)
      nOpen += (int)(sol[modelData->y[j]] + 0.5);
    for (int i = 0; i < N_CUST; i++)
      for (int j = 0; j < N_FAC; j++)
        if ((int)(sol[modelData->x[i][j]] + 0.5)) {
          if (modelData->dist[i][j] > maxDist) maxDist = modelData->dist[i][j];
          break;
        }
    printf("  %4d  %12.2f  %11d  %9.2f\n", p, poolObj, nOpen, maxDist);
  }
  printf("\n");

cleanup:

  if (sol) free(sol);

  return returnCode;
}

/* Solve our model and collect up to 10 solutions into the pool.
 */
int solve_collect_solutions(XPRSprob prob, struct model_data_s *modelData)
{
  int returnCode = 0;

  /* Collect up to ten solutions during the MIP search. Solutions are
   * stored from best to worst; index 0 always holds the best solution. */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_POOLLIMIT, 10) );

  printf("\nSolving problem to collect solutions\n\n");
  int solveStatus, solStatus;
  CHECK_RETURN( XPRSoptimize(prob, "", &solveStatus, &solStatus) );
  if ((solveStatus == XPRS_SOLVESTATUS_COMPLETED) && (solStatus == XPRS_SOLSTATUS_OPTIMAL)) {
    printf("Solved problem to optimality.\n");
  } else {
    printf("Failed to solve problem with solvestatus %i and solstatus %i\n", solveStatus, solStatus);
    goto cleanup;
  }

  /* ---- Print the solution pool ---- */
  CHECK_RETURN( print_solutions(prob, modelData) );

cleanup:

  return returnCode;
}

/* Assumes that we have already solved the problem and that the pool contains
 * a set of solutions.
 *
 * This function forces a single facility to be open and uses the pooled
 * solutions to warm-start the next MIP solve.
 */
int solve_warm_start_from_pool(XPRSprob prob, struct model_data_s *modelData)
{
  int returnCode = 0;

  /* Make sure the problem is in its original state before we change it. */
  /* If the problem is already in its original state, this call will simply */
  /* return without doing anything. */
  CHECK_RETURN( XPRSpostsolve(prob) );

  /* We need to set the control POOLKEEPSOLS to transfer any existing */
  /* solutions to the warm-start pool before we fix a variable, since any */
  /* modifications will invalidate all collected selections. */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_POOLKEEPSOLS, -1) );

  /* Force a facility to be open. If we open an additional facility, all */
  /* existing solution should remain feasible - the presolver should take */
  /* care of satisfying the fixed binary. The warm-start solutions will not */
  /* be optimal since no customers are served by the new facility. */
  int bndIdx = 2;       /* Pick a facility that should not be open in our current optimal solution. */
  char bndType = 'B';   /* Fix the variable by specifying 'B'oth bound should be set. */
  double bndValue = 1.0;
  CHECK_RETURN( XPRSchgbounds(prob, 1, &bndIdx, &bndType, &bndValue) );

  /* ---- Re-solve ---- */
  printf("\nReoptimizing with fixed facility %i.\n\n", bndIdx);
  int solveStatus, solStatus;
  CHECK_RETURN( XPRSoptimize(prob, "", &solveStatus, &solStatus) );
  if ((solveStatus == XPRS_SOLVESTATUS_COMPLETED) && (solStatus == XPRS_SOLSTATUS_OPTIMAL)) {
    printf("Resolved problem to optimality.\n");
  } else {
    printf("Failed to resolve problem with solvestatus %i and solstatus %i\n", solveStatus, solStatus);
    goto cleanup;
  }

  /* ---- Print the new solution pool ---- */
  CHECK_RETURN( print_solutions(prob, modelData) );

  /* Reset controls. We should do this before modifying the problem */
  /* to avoid automatically transferring the solutions when restoring */
  /* the bound below. */
  CHECK_RETURN(XPRSsetdefaultcontrol(prob, XPRS_POOLKEEPSOLS));

  /* Undo the fixed facility by returning the lower bound to 0.0. */
  bndType = 'L';
  bndValue = 0.0;
  CHECK_RETURN( XPRSchgbounds(prob, 1, &bndIdx, &bndType, &bndValue) );

cleanup:

  return returnCode;
}

/* Use the built-in solution enumeration to find the 5 best solutions to
 * our problems. The solver, by default, considers two solutions as being
 * distinct if they differ in any of our binary restricted variables.
 * To help speed up the solve, we limit the search to solutions whose
 * objective value is within 50% of the best solution.
 */
int enumerate_all_binaries(XPRSprob prob, struct model_data_s *modelData)
{
  int returnCode = 0;

  /* Collect up to 5 best solutions. */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_POOLLIMIT, 5) );

  /* Enable automatic solution enumeration. */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_POOLENUMSTRATEGY, XPRS_POOLENUMSTRATEGY_FULL) );

  /* We limit collection to solutions that are within 50% of the best */
  /* solution. */
  CHECK_RETURN( XPRSsetdblcontrol(prob, XPRS_POOLRELGAP, 0.50) );

  /* Solve the problem, which will automatically trigger an extended */
  /* branch-and-bound search to find all 5 best solutions. */
  printf("\nSolving problem to find 5 best solutions.\n\n");
  int solveStatus, solStatus;
  CHECK_RETURN( XPRSoptimize(prob, "", &solveStatus, &solStatus) );
  if ((solveStatus == XPRS_SOLVESTATUS_COMPLETED) && (solStatus == XPRS_SOLSTATUS_OPTIMAL)) {
    printf("Solved problem to optimality.\n");
  } else {
    printf("Failed to solve problem with solvestatus %i and solstatus %i\n", solveStatus, solStatus);
    goto cleanup;
  }

  /* Print the collected solutions. */
  CHECK_RETURN( print_solutions(prob, modelData) );

  /* Reset controls. */
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_POOLLIMIT) );
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_POOLENUMSTRATEGY) );
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_POOLRELGAP) );

cleanup:

  return returnCode;
}

/* We want to find the 5 best combinations of opening facilities but the
 * demand sourcing should be optimal in each solution. To achieve this,
 * we need to tell the solver to enumerate only through solutions that
 * differ in facility variables. We can do this through solution enumeration
 * by specifying branching directives only on the facility variables.
 * The solver will automatically treat two solutions with the same
 * facility assignment as duplicates and only keep the best one, which
 * should result in a solution with the optimal demand assignment for each
 * facility assignment.
 */
int enumerate_facilities(XPRSprob prob, struct model_data_s *modelData)
{
  int returnCode = 0;

  /* Make sure the problem is in its original state before we change it. */
  /* If the problem is already in its original state, this call will simply */
  /* return without doing anything. */
  CHECK_RETURN( XPRSpostsolve(prob) );

  /* Collect up to 5 best solutions. */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_POOLLIMIT, 5) );

  /* Enable automatic solution enumeration. */
  CHECK_RETURN( XPRSsetintcontrol(prob, XPRS_POOLENUMSTRATEGY, XPRS_POOLENUMSTRATEGY_FULL) );

  /* Apply branching directives to the facility binaries. */
  /* Binaries without a directive will not be enumerated. */
  int directivesOn[N_FAC] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  CHECK_RETURN( XPRSloadbranchdirs(prob, N_FAC, modelData->y, directivesOn) );

  /* Solve the problem, which will automatically trigger an extended */
  /* branch-and-bound search to find the 5 best facility assignments. */
  printf("\nSolving problem to find 5 best facility assignments.\n\n");
  int solveStatus, solStatus;
  CHECK_RETURN( XPRSoptimize(prob, "", &solveStatus, &solStatus) );
  if ((solveStatus == XPRS_SOLVESTATUS_COMPLETED) && (solStatus == XPRS_SOLSTATUS_OPTIMAL)) {
    printf("Resolved problem to optimality.\n");
  } else {
    printf("Failed to solve problem with solvestatus %i and solstatus %i\n", solveStatus, solStatus);
    goto cleanup;
  }

  /* Print the collected solutions. */
  CHECK_RETURN( print_solutions(prob, modelData) );

  /* Reset controls. */
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_POOLLIMIT) );
  CHECK_RETURN( XPRSsetdefaultcontrol(prob, XPRS_POOLENUMSTRATEGY) );

  /* Clear branching directives. */
  int directivesOff[N_FAC] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  CHECK_RETURN( XPRSloadbranchdirs(prob, N_FAC, modelData->y, directivesOff) );

cleanup:

  return returnCode;
}

int main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;

  struct model_data_s modelData = { 0 };

  /* ---- Initialize the optimizer ---- */
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

  /* ---- Build the model ---- */
  CHECK_RETURN( build_model(prob, &modelData) );

  /* Solve the model and collect solutions. */
  CHECK_RETURN( solve_collect_solutions(prob, &modelData) );

  /* Try forcing a facility to be opened and reoptimize. We carry the */
  /* existing solutions through as warm-start solutions for the next */
  /* solve. */
  CHECK_RETURN( solve_warm_start_from_pool(prob, &modelData) );

  /* Run an enumeration search to find the 5 best solutions. */
  CHECK_RETURN( enumerate_all_binaries(prob, &modelData) );

  /* Run an enumeration search to find the 5 best facility assignments. */
  CHECK_RETURN( enumerate_facilities(prob, &modelData) );

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

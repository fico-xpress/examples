/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file solenum.c
   ``````````````
   Example of enumerating the n-best solutions when solving a MIP.

   The program reads a problem from a file and collects solutions into a pool.
   Depending on the setting of a searchMethod parameter, it will enumerate
   additional solutions. There is an optional parameter to specify the
   maximum number of solutions to collect.

   The parameter searchMethod can be 0, 1 or 2. For the value of 0 it just
   collects solutions. For a value of 1 it continues the search until it has
   found the n best solutions that are reachable through the branch-and-bound
   process. The value of 2 ensures the n-best solutions are returned.

   The example implements its own solution pool with solutions stored in order
   of objective value, and implements duplication checks. Almost all the
   interesting work happens within the preintsol callback. It collects
   solutions, checks for duplicates and adjusts the cutoff for the remaining
   search. The adjustment of the cutoff is crucial to ensure additional
   solutions are found.

   To guarantee that we find the n best solutions, XPRSloadbranchdirs is
   used to force the Optimizer into exhaustive branching. This function is used
   to specify the subset of integer variables that should be different in the
   collected solutions. Everything else is treated as a duplicate.

   (c) 2025-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "xprs.h"


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

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

static int callbackError = 0;       /* Indicates an error in a callback. */

// Data structures for holding a single solution.
struct solution_s {

  double *x;          // Primal solution vector
  double objValue;    // Objective value of the solution vector

  unsigned int hash;  // A computed hash value for the solution.
                      // Used for detecting duplicates.

  struct solution_s *next;    // Next solution in the ordered list.
  struct solution_s *prev;    // Previous solution in the ordered list.

};

// Data structure for holding a sorted list of solutions.
//
// Solutions are assumed to be sorted by objective value.
struct solutionPool_s {
  struct solution_s *first;
  struct solution_s *last;

  int nSolutions;               // Number of solutions stored in the pool.
  int ncols;                    // Number of columns of solutions stored in this pool.
  int isSortOrderAscending;     // Whether to maintain solutions in ascending or descending order.
  int maxSolutions;             // Maximum number of solutions to store.
};

unsigned int HashInteger(unsigned int i)
{
  i = ((i >> 16) ^ i) * 0x45d9f3b;
  i = ((i >> 16) ^ i) * 0x45d9f3b;
  i = (i >> 16) ^ i;
  return i;
}

// Calculates a simple hash across the solution values of enumerated columns.
unsigned int HashSolution(
  const double *x,
  const int *colIsEnumerated,
  const int ncols
) {
  unsigned int hash = 0;

  for (int col = 0; col < ncols; col++) {
    if (!colIsEnumerated[col]) continue;

    int intx = (int) round(x[col]);
    hash = HashInteger(hash ^ HashInteger(col) ^ HashInteger((unsigned int)intx));
  }

  return hash;
}

struct solution_s *AllocateSolution(const int ncols)
{
  struct solution_s *solution = NULL;

  solution = malloc(sizeof(*solution));
  if (solution == NULL) {
    perror("malloc");
    return NULL;
  }
  memset(solution, 0, sizeof(*solution));
  solution->x = malloc(ncols * sizeof(*solution->x));
  if (solution->x == NULL) {
    perror("malloc");
    free(solution);
    return NULL;
  }

  return solution;
}

void FreeSolution(struct solution_s **p_solution)
{
  if (*p_solution == NULL) return;

  free((*p_solution)->x);
  free(*p_solution);

  *p_solution = NULL;
}

void InitializePool(struct solutionPool_s *pool, int ncols, int ifSortAscending, int maxSolutions)
{
  pool->first = NULL;
  pool->last = NULL;
  pool->ncols = ncols;
  pool->nSolutions = 0;
  pool->isSortOrderAscending = ifSortAscending;
  pool->maxSolutions = maxSolutions;
}

// Insert a new solution into a pool in sorted order.
void AddSolution(
  struct solutionPool_s *pool,
  struct solution_s **p_newSolution,
  const int *colIsEnumerated          // Selects which columns to check for duplicates.
) {

  struct solution_s *newSolution = *p_newSolution;
  *p_newSolution = NULL;

  double sortMult = pool->isSortOrderAscending ? +1.0 : -1.0;

  // Hash the new solution for duplication checks.
  newSolution->hash = HashSolution(newSolution->x, colIsEnumerated, pool->ncols);

  // Search through the pool and find the right place to insert it.
  struct solution_s *duplicateSolution = NULL;
  struct solution_s *insertBeforeSolution = NULL;
  struct solution_s *nextSolution = pool->first;
  int ifDropNewSolution = 0;
  while (nextSolution) {

    // Check if our solution is better than the list solution.
    if ((insertBeforeSolution == NULL) && (sortMult * (nextSolution->objValue - newSolution->objValue) > 0.0)) {
      insertBeforeSolution = nextSolution;
    }

    if (newSolution->hash == nextSolution->hash) {
      int isDuplicate = 1;
      // We have a potentially duplicate column. Check if it is a duplicate.
      for (int col = 0; col < pool->ncols; col++) {
        if (!colIsEnumerated[col]) continue;
        if (fabs(nextSolution->x[col] - newSolution->x[col]) > 0.5) {
          // The two solutions must have different integer values.
          isDuplicate = 0;
          break;
        }
      }

      if (isDuplicate) {
        if (insertBeforeSolution) {
          // The new solution is better than the duplicate, so remove the
          // duplicate.
          duplicateSolution = nextSolution;
        } else {
          // The new solution is no better than the duplicate, so we don't insert
          // it into the list.
          ifDropNewSolution = 1;
          break;
        }
      }
    }

    nextSolution = nextSolution->next;
  }

  if (ifDropNewSolution) {
    FreeSolution(&newSolution);
  } else {
    if (insertBeforeSolution) {
      // Link the new solution into the list before the given solution.
      newSolution->next = insertBeforeSolution;
      newSolution->prev = insertBeforeSolution->prev;
      insertBeforeSolution->prev = newSolution;
    } else {
      // Add the new solution to the end of the list.
      newSolution->next = NULL;
      newSolution->prev = pool->last;
      pool->last = newSolution;
    }
    if (newSolution->prev) {
      newSolution->prev->next = newSolution;
    } else {
      pool->first = newSolution;
    }
    pool->nSolutions += 1;
  }

  if (duplicateSolution) {
    // Remove the duplicate solution from the list.
    if (duplicateSolution->prev) {
      duplicateSolution->prev->next = duplicateSolution->next;
    } else {
      pool->first = duplicateSolution->next;
    }
    if (duplicateSolution->next) {
      duplicateSolution->next->prev = duplicateSolution->prev;
    } else {
      pool->last = duplicateSolution->prev;
    }
    pool->nSolutions -= 1;
    FreeSolution(&duplicateSolution);
  }

  while (pool->nSolutions > pool->maxSolutions) {
    // We have too many solutions, so drop the last (worst) one.
    struct solution_s *dropSolution = pool->last;
    pool->last = dropSolution->prev;
    if (pool->last) {
      pool->last->next = NULL;
    } else {
      pool->first = NULL;
    }
    FreeSolution(&dropSolution);
    pool->nSolutions -= 1;
  }
}

// Select which integer columns to enumerate across.
//
// This could be adjusted to enumerate across a subset of integer columns.
int SelectEnumerationColumns(
  XPRSprob prob,
  int ncols,
  int *colIsEnumerated
) {
  int returnCode = 0;

  char *colType = NULL;

  // Get the type of each column so we can identify the integer columns.
  colType = malloc(ncols * sizeof(*colType));
  if (colType == NULL) {
    perror("malloc");
    returnCode = -1;
    goto cleanup;
  }
  CHECK_RETURN(XPRSgetcoltype(prob, colType, 0, ncols - 1));

  // Identify the integer restricted columns.
  for (int col = 0; col < ncols; col++) {
    if ((colType[col] == 'B') || (colType[col] == 'I')) {
      colIsEnumerated[col] = 1;
    } else {
      colIsEnumerated[col] = 0;
    }
  }

cleanup:

  free(colType);
  return returnCode;
}

// Enforce enumeration of the selected columns.
int EnforceEnumerationColumns(
  XPRSprob prob,
  int ncols,
  int *colIsEnumerated
) {
  int returnCode = 0;

  // We use the Xpress branch directives to force Xpress to continue branching
  // on any column that we want enumerated.
  int *colSelect = NULL;
  colSelect = malloc(ncols * sizeof(*colSelect));
  if (colSelect == NULL) {
    perror("malloc");
    returnCode = -1;
    goto cleanup;
  }
  int nSelect = 0;
  for (int col = 0; col < ncols; col++) {
    if (colIsEnumerated[col]) {
      colSelect[nSelect++] = col;
    }
  }
  CHECK_RETURN(XPRSloadbranchdirs(prob, nSelect, colSelect, NULL));

cleanup:

  free(colSelect);
  return returnCode;
}


// XPRS optimizer message callback
void cbMessage(XPRSprob cbprob, void* cbdata,
                       const char *msg, int len, int msgtype)
{
  (void)cbprob;   // unused (the problem for which the message is issued)
  (void)cbdata;   // unused (the data passed to XPRSaddcbmessage())
  switch(msgtype)
  {
  case 4:  // error
  case 3:  // warning
  case 2:  // not used
  case 1:  // information
    printf("%*s\n", len, msg);
    break;
  default: // exiting - buffers need flushing
    fflush(stdout);
    break;
  }
}


// Callback data for collecting solutions.
struct cbData_s {

  struct solutionPool_s pool;     // Collected solutions.

  int *colIsEnumerated;   // O or 1 for each column.
                          // 1 means that we want to enumerate different values for this column.
                          // Only integer columns should be flagged with 1.

  int isMinimization;     // 1 if minimization problem, 0 if maximization.

  int searchMethod;       // If >0, the search will continue until we have the required number of best solutions.

};

// The preintsol callback function that will be responsible for managing the
// search and solution pool.
//
// If searchMethod==0 we just collect any solution encountered.
//
// If searchmethod>0 we adjust the cutoff value to ensure that the search
// continues until we have found the n best solutions that the current search
// can reach.
//
// If heuristics are enabled, we need to check for duplicates, since heuristics
// can return a previously discovered solution.
//
// In a normal, multi-threaded solve, the preintsol callback can fire concurrently
// on multiple threads and we would need to mutex protect access. But by setting
// SERIALIZEPREINTSOL=1, we ensure that the preintsol callbacks are always
// fired sequentially and deterministically.
void cbPreIntSol(XPRSprob cbprob, void *cbDataPointer, int solType, int *p_reject, double *p_cutoff)
{
  int returnCode = 0;
  struct cbData_s *cbData = (struct cbData_s *)cbDataPointer;
  (void)solType;  // Unused: type of solution, e.g. heuristic or node
  (void)p_reject; // Unused: set to 1 to reject this solution

  // Collect the new solution.
  struct solution_s *newSolution = AllocateSolution(cbData->pool.ncols);
  if (newSolution == NULL) {
    returnCode = -1;
    goto cleanup;
  }
  // We have to use XPRSgetcallbacksolution() to retrieve the solution since
  // it has not been installed as the incumbent yet.
  CHECK_RETURN(XPRSgetcallbacksolution(cbprob, NULL, newSolution->x, 0, cbData->pool.ncols - 1));
  CHECK_RETURN(XPRSgetdblattrib(cbprob, XPRS_LPOBJVAL, &newSolution->objValue));

  // Add the new solution to our pool.
  AddSolution(&cbData->pool, &newSolution, cbData->colIsEnumerated);

  if (cbData->searchMethod == 0) {
    // We just collect the solutions and don't adjust the search.
    return;
  }

  // Adjust the cutoff so we continue finding solutions.
  if (cbData->pool.nSolutions >= cbData->pool.maxSolutions) {
    // We already have the required number of solutions, so set the
    // cutoff such that we only search for improving solutions.
    // We will ask for something slightly better than the worst we
    // have collected.
    *p_cutoff = cbData->pool.last->objValue + ((cbData->isMinimization) ? -1e-6 : +1e-6);
  } else {
    // We don't have enough solutions yet, so any solution is acceptable.
    *p_cutoff = (cbData->isMinimization) ? +1e+40 : -1e+40;
  }

cleanup:

  if (returnCode) {
    /* There was an error in the callback. Stop the Optimizer and remember the error. */
    XPRSinterrupt(cbprob, XPRS_STOP_USER);
    callbackError = 1;
  }
}

int main(int argc, char *argv[]) {
  int returnCode = 0;

  XPRSprob prob = NULL;

  struct cbData_s cbData = { 0 };

  const char *filename = NULL;
  int searchMethod = 0;
  int maxSolutions = 10;

  if ((argc < 3) || (argc > 4)) {
    printf("Syntax:\n");
    printf("   solenum <filename> <method> [<n>]\n");
    printf("Description:\n");
    printf("   Enumerates and collects multiple solutions to a MIP.\n");
    printf("Options:\n");
    printf("   <filename>  MPS or LP file to solve\n");
    printf("   <method>    How to collect solutions (0-2).\n");
    printf("                  0 - normal solve\n");
    printf("                  1 - extended search\n");
    printf("                  2 - n-best search\n");
    printf("   <n>         Maximum number of solutions to collect\n");
    return 0;
  }

  filename = argv[1];
  searchMethod = atoi(argv[2]);
  if ((searchMethod < 0) || (searchMethod > 2)) {
    printf("Invalid value (%i) given for search method. Must be 0, 1 or 2.\n", searchMethod);
    return 0;
  }
  if (argc == 4) {
    maxSolutions = max(1, atoi(argv[3]));
  }

  // Create our Xpress work problem.
  CHECK_RETURN(XPRSinit(NULL));
  CHECK_RETURN(XPRScreateprob(&prob));

  // Set a messaging callback so we can follow progress.
  CHECK_RETURN(XPRSaddcbmessage(prob, cbMessage, NULL, 0));

  // Load the given problem.
  if (XPRSreadprob(prob, filename, "")) {
    printf("Failed to load `%s`\n", filename);
    return 0;
  }

  // Extract the information we are going to need from the problem.
  int ncols;
  double objectiveSense;
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &ncols));
  CHECK_RETURN(XPRSgetdblattrib(prob, XPRS_OBJSENSE, &objectiveSense));
  cbData.isMinimization = (objectiveSense > 0.0);
  InitializePool(&cbData.pool, ncols, cbData.isMinimization, maxSolutions);

  // Select which columns we want to enumerate over.
  cbData.searchMethod = searchMethod;
  cbData.colIsEnumerated = malloc(ncols * sizeof(*cbData.colIsEnumerated));
  if (cbData.colIsEnumerated == NULL) {
    perror("malloc");
    returnCode = -1;
    goto cleanup;
  }
  CHECK_RETURN(SelectEnumerationColumns(prob, ncols, cbData.colIsEnumerated));

  if (searchMethod >= 2) {
    // Force branching on integer valued columns to guarantee we find
    // the n-best solutions.
    CHECK_RETURN(EnforceEnumerationColumns(prob, ncols, cbData.colIsEnumerated));
  }

  // Set the preintsol callback for collecting integer solutions.
  CHECK_RETURN(XPRSaddcbpreintsol(prob, cbPreIntSol, &cbData, 0));
  // Serialize the triggering of the callback so we don't have to worry about
  // concurrency.
  CHECK_RETURN(XPRSsetintcontrol(prob, XPRS_SERIALIZEPREINTSOL, 1));
  if (searchMethod >= 2) {
    // Disable dual reductions so we don't cut off dominated solutions.
    CHECK_RETURN(XPRSsetintcontrol(prob, XPRS_MIPDUALREDUCTIONS, 0));
    // Do not stop on a zero gap as we need to continue beyond the
    // optimal solution to get all the best solutions.
    CHECK_RETURN(XPRSsetdblcontrol(prob, XPRS_MIPRELSTOP, 0));
  }

  // Solve the problem.
  int solvestatus, solstatus;
  CHECK_RETURN(XPRSoptimize(prob, "", &solvestatus, &solstatus));

  if (callbackError) {
    returnCode = -3;
    goto cleanup;
  }

  // Print the collected solutions.
  printf("\n");
  if (cbData.pool.nSolutions == 0) {
    printf("No solutions collected!\n");
  } else {
    printf("Collected %i solutions with objective values:\n", cbData.pool.nSolutions);
    struct solution_s *solution = cbData.pool.first;
    while (solution) {
      printf("   %15.4f\n", solution->objValue);
      solution = solution->next;
    }
  }

cleanup:

  // Free up everything.
  while (cbData.pool.first) {
    struct solution_s *solution = cbData.pool.first;
    cbData.pool.first = solution->next;
    FreeSolution(&solution);
  }
  free(cbData.colIsEnumerated);

  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

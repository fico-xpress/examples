/***********************************************************************
  Xpress Optimizer Examples
  =========================

  file els_managedcuts.c
  ``````````
  Demonstrates how to implement cutting planes as part of a MIP branch-and=
  bound search using the cutround callback.

  Cuts are added as Optimizer managed cuts using XPRSaddmanagedcuts.

  Economic lot sizing problem. Solved by adding (l,S)-inequalities
  in a branch-and-cut heuristic (using the cutround callback).

  Problem description:
  ````````````````````

  ELS considers production planning over a horizon of T periods. In period t,
  t=1,...,T, there is a given demand DEMAND[p,t] that must be satisfied by
  production produce[p,t] in period t and by inventory carried over from previous
  periods. There is a set-up cost SETUPCOST[t] associated with production in
  period t and the total production capacity per period is limited. The unit
  production cost in period t is PRODCOST[p,t]. There is no
  inventory or stock-holding cost.

  Cutting planes:
  ```````````````

  A well-known class of valid inequalities for ELS are the
  (l,S)-inequalities.  Let D(p,q) denote the demand in periods p
  to q and y(t) be a binary variable indicating whether there is any
  production in period t.  For each period l and each subset of periods S
  of 1 to l, the (l,S)-inequality is

      sum (t=1:l | t in S) x(t) + sum (t=1:l | t not in S) D(t,l) * y(t)
          >= D(1,l)

  It says that actual production x(t) in periods included S plus maximum
  potential production D(t,l)*y(t) in the remaining periods (those not in
  S) must at least equal total demand in periods 1 to l.  Note that in
  period t at most D(t,l) production is required to meet demand up to
  period l.

  Based on the observation that

      sum (t=1:l | t in S) x(t) + sum (t=1:l | t not in S) D(t,l) * y(t)
          >= sum (t=1:l) min(x(t), D(t,l) * y(t))
          >= D(1,l)

  it is easy to develop a separation algorithm and thus a cutting plane
  algorithm based on these (l,S)-inequalities.

  (c) 2025-2026 Fair Isaac Corporation

*****************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "xprs.h"

/*
 * Calls an Xpress optimizer function and checks the return code.
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
static void cbMessage(XPRSprob cbprob, void *cbdata, const char *msg, int len, int msgtype)
{
  (void)cbprob; /* unused */
  (void)cbdata; /* unused */
  if (msgtype > 0) printf("%*s\n", len, msg);
}

/* Tolerance for satisfiability */
#define EPS 1e-6

/* Number of time periods. Reduce to create an easier problem. */
#define DIM_TIMES 15
/* Number of products to produce. */
#define DIM_PRODUCTS 4

static int DEMAND[DIM_PRODUCTS][DIM_TIMES] = {
  {2, 3, 5, 3, 4, 2, 5, 4, 1, 3, 4, 2, 3, 5, 2},
  {3, 1, 2, 3, 5, 3, 1, 2, 3, 3, 4, 5, 1, 4, 1},
  {3, 5, 2, 1, 2, 1, 3, 3, 5, 2, 2, 1, 3, 2, 3},
  {2, 2, 1, 3, 2, 1, 2, 2, 3, 3, 2, 2, 3, 1, 2}
};

static int SETUPCOST[DIM_TIMES] = {
  17, 14, 11, 6, 9, 6, 15, 10, 8, 7, 12, 9, 10, 8, 12
};

static int PRODCOST[DIM_PRODUCTS][DIM_TIMES] = {
  {5, 3, 2, 1, 3, 1, 4, 3, 2, 2, 3, 1, 2, 3, 2},
  {1, 4, 2, 3, 1, 3, 1, 2, 3, 3, 3, 4, 4, 2, 2},
  {3, 3, 3, 4, 4, 3, 3, 3, 2, 2, 1, 1, 3, 3, 3},
  {2, 2, 2, 3, 3, 3, 4, 4, 4, 3, 3, 2, 2, 2, 3}
};

static int CAP[DIM_TIMES] = {
  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};

struct prob_data_s {
  int produce[DIM_PRODUCTS][DIM_TIMES];
  int setup[DIM_PRODUCTS][DIM_TIMES];
  double D[DIM_PRODUCTS][DIM_TIMES][DIM_TIMES];
};

/* Creates an economic lot sizing problem. */
static int LoadELSProblem(XPRSprob prob, struct prob_data_s *probData)
{
  int returnCode = 0;

  int p, s, t;
  int col_next;

  int xprs_cols;

  int *row_inds = NULL;
  int row_start = 0;
  double *row_coefs = NULL;
  double row_rhs;
  char row_type;
  int row_length;

  /* Calculate demand D(p, s, t) as the demand for product p from time s to time t(inclusive). */
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (s = 0; s < DIM_TIMES; s++) {
      double sum_demand = 0.0;
      for (t = s; t < DIM_TIMES; t++) {
        sum_demand += DEMAND[p][t];
        probData->D[p][s][t] = sum_demand;
      }
    }
  }

  /* Start with an empty problem. */
  CHECK_RETURN(XPRSloadlp(prob, "ELS", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

  /* Define the variables. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &xprs_cols));
  col_next = xprs_cols;
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (t = 0; t < DIM_TIMES; t++) {
      probData->produce[p][t] = col_next++;
    }
  }
  CHECK_RETURN(XPRSaddcols(prob, col_next - xprs_cols, 0, NULL, NULL, NULL, NULL, NULL, NULL));
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &xprs_cols));
  col_next = xprs_cols;
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (t = 0; t < DIM_TIMES; t++) {
      probData->setup[p][t] = col_next++;
    }
  }
  CHECK_RETURN(XPRSaddcols(prob, col_next - xprs_cols, 0, NULL, NULL, NULL, NULL, NULL, NULL));

  /* Allocate space to define constraints. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_COLS, &xprs_cols));
  row_coefs = malloc(xprs_cols * sizeof(double));
  row_inds = malloc(xprs_cols * sizeof(int));
  if ((row_coefs == NULL) || (row_inds == NULL)) {
    fprintf(stderr, "malloc failure");
    returnCode = -1;
    goto cleanup;
  }

  /* Add the objective function :
   *  MinCost:= sum(t in TIMES) (SETUPCOST(t) * sum(p in PRODUCTS) setup(p,t) +
   *                            sum(p in PRODUCTS) PRODCOST(p, t) *produce(p, t) )
   */
  row_length = 0;
  for (t = 0; t < DIM_TIMES; t++) {
    for (p = 0; p < DIM_PRODUCTS; p++) {
      row_inds[row_length] = probData->setup[p][t]; row_coefs[row_length] = SETUPCOST[t]; row_length++;
      row_inds[row_length] = probData->produce[p][t]; row_coefs[row_length] = PRODCOST[p][t]; row_length++;
    }
  }
  CHECK_RETURN(XPRSchgobj(prob, row_length, row_inds, row_coefs));

  /* Add constraints. */

  /* Production in periods 0 to t must satisfy the total demand
   * during this period of time, for all t in [0,T[
   *    forall(p in PRODUCTS, t in TIMES)
   *      Dem(t) : = sum(s in 1..t) produce(p,s) >= sum(s in 1..t) DEMAND(s)
   */
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (t = 0; t < DIM_TIMES; t++) {
      row_length = 0;
      for (s = 0; s <= t; s++) {
        row_inds[row_length] = probData->produce[p][s]; row_coefs[row_length] = 1.0; row_length++;
      }
      row_rhs = 0.0;
      for (s = 0; s <= t; s++) row_rhs += DEMAND[p][s];
      row_type = 'G';
      CHECK_RETURN(XPRSaddrows(prob, 1, row_length, &row_type, &row_rhs, NULL, &row_start, row_inds, row_coefs));
    }
  }

  /* If there is production during t then there is a setup in t :
   *    forall(p in PRODUCTS, t in TIMES)
   *       ProdSetup(t) : = produce(t) <= D(t,TIMES) * setup(t)
   */
  for (p = 0; p < DIM_PRODUCTS; p++) {
    double sum_demand = 0.0;
    for (t = DIM_TIMES - 1; t >= 0; t--) {
      sum_demand += DEMAND[p][t];
      row_length = 0;
      row_inds[row_length] = probData->produce[p][t]; row_coefs[row_length] = 1.0; row_length++;
      row_inds[row_length] = probData->setup[p][t]; row_coefs[row_length] = -sum_demand; row_length++;
      row_rhs = 0.0;
      row_type = 'L';
      CHECK_RETURN(XPRSaddrows(prob, 1, row_length, &row_type, &row_rhs, NULL, &row_start, row_inds, row_coefs));
    }
  }

  /* Capacity limits :
   *    forall(t in TIMES) Capacity(t) : = sum(p in PRODUCTS) produce(p, t) <= CAP(t)
   */
  for (t = 0; t < DIM_TIMES; t++) {
    row_length = 0;
    for (p = 0; p < DIM_PRODUCTS; p++) {
      row_inds[row_length] = probData->produce[p][t]; row_coefs[row_length] = 1.0; row_length++;
    }
    row_rhs = CAP[t];
    row_type = 'L';
    CHECK_RETURN(XPRSaddrows(prob, 1, row_length, &row_type, &row_rhs, NULL, &row_start, row_inds, row_coefs));
  }

  /* setup variables are binary:
   *   forall(p in PRODUCTS, t in TIMES) setup(p,t) is_binary
   */
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (t = 0; t < DIM_TIMES; t++) {
      char col_type = 'B';
      CHECK_RETURN(XPRSchgcoltype(prob, 1, &probData->setup[p][t], &col_type));
    }
  }

  /* Add names for the variables. */
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (t = 0; t < DIM_TIMES; t++) {
      char name[32];
      snprintf(name, sizeof(name), "setup(%i,%i)", p, t);
      CHECK_RETURN(XPRSaddnames(prob, XPRS_NAMES_COLUMN, name, probData->setup[p][t], probData->setup[p][t]));
      snprintf(name, sizeof(name), "produce(%i,%i)", p, t);
      CHECK_RETURN(XPRSaddnames(prob, XPRS_NAMES_COLUMN, name, probData->produce[p][t], probData->produce[p][t]));
    }
  }

cleanup:

  if (row_inds) free(row_inds);
  if (row_coefs) free(row_coefs);

  return returnCode;
}

/* Cut round callback for separating our cutting planes. */
void cbCutRound(XPRSprob prob, void *cbdata, int ifxpresscuts, int *p_action)
{
  (void)ifxpresscuts; /* unused */
  (void)p_action; /* unused */

  int returnCode = 0;

  struct prob_data_s *probData = (struct prob_data_s *)cbdata;

  int p, t, l;

  int xprs_inputcols;
  int xprs_cutrounds;

  double *sol = NULL;
  int isAvailable;

  char    cutSense;
  int     cutLen;
  int    *cutInds = NULL;
  double *cutCoefs = NULL;
  double  cutRhs;
  int cutsStart[2];
  int nCutsAdded = 0;

  /* Apply only one round of cutting on each node. */
  /* Because the CutRound callback is fired *before* a round of cutting, */
  /* the CUTROUNDS attribute will start from 0 on the first invocation of */
  /* the callback. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_CUTROUNDS, &xprs_cutrounds));
  if (xprs_cutrounds >= 1) {
    goto cleanup;
  }

  /* We need the dimension of the solution vector. */
  CHECK_RETURN(XPRSgetintattrib(prob, XPRS_INPUTCOLS, &xprs_inputcols));

  /* Get the solution vector. */
  sol = malloc(xprs_inputcols * sizeof(*sol));
  if (!sol) {
    fprintf(stderr, "malloc failure");
    returnCode = -1;
    goto cleanup;
  }
  CHECK_RETURN(XPRSgetcallbacksolution(prob, &isAvailable, sol, 0, xprs_inputcols-1));
  /* Xpress will only fire the CutRound callback when a solution is available, */
  /* so there is no need to check the isAvailable return argument. */

  /* Search for violated constraints : the minimum of the actual production
   * produce[p][t] and the maximum potential production D[p][t][l]*setup[p][t]
   * in periods 0 to l must at least equal the total demand in periods
   * 0 to l.
   *    sum(t=1:l) min(prod[p][t], D[p][t][l]*setup[p][t]) >= D[p][0][l]
   */
  for (p = 0; p < DIM_PRODUCTS; p++) {
    for (l = 0; l < DIM_TIMES; l++) {
      double sum = 0.0;
      for (t = 0; t <= l; t++) {
        if (sol[probData->produce[p][t]] < probData->D[p][t][l] * sol[probData->setup[p][t]] + EPS) {
          sum += sol[probData->produce[p][t]];
        } else {
          sum += probData->D[p][t][l] * sol[probData->setup[p][t]];
        }
      }
      if (sum < probData->D[p][0][l] - EPS) {
        // Create the violated cut.
        if (!cutInds || !cutCoefs) {
          cutInds = malloc(xprs_inputcols * sizeof(*cutInds));
          cutCoefs = malloc(xprs_inputcols * sizeof(*cutCoefs));
          if (!cutInds || !cutCoefs) {
            fprintf(stderr, "malloc failure");
            returnCode = -1;
            goto cleanup;
          }
        }
        cutLen = 0;
        cutSense = 'G';
        for (t = 0; t <= l; t++) {
          if (sol[probData->produce[p][t]] < probData->D[p][t][l] * sol[probData->setup[p][t]] + EPS) {
            cutCoefs[cutLen] = 1.0;
            cutInds[cutLen] = probData->produce[p][t];
            cutLen += 1;
          } else {
            cutCoefs[cutLen] = probData->D[p][t][l];
            cutInds[cutLen] = probData->setup[p][t];
            cutLen += 1;
          }
        }
        cutRhs = probData->D[p][0][l];

        /* Give the cut to Xpress to manage. It will automatically be presolved. */
        cutsStart[0] = 0;
        cutsStart[1] = cutLen;
        CHECK_RETURN(XPRSaddmanagedcuts(prob, 1 /*globalvalid*/, 1, &cutSense, &cutRhs, cutsStart, cutInds, cutCoefs));
        nCutsAdded += 1;
        /* If we add any cuts in the callback, Xpress will automatically trigger another roound of cuts, */
        /* so there is no need to set the p_action return argument. */
      }
    }
  }

  if (nCutsAdded > 0) {
    printf("Cuts added: %i\n", nCutsAdded);
  }

cleanup:

  if (sol) free(sol);
  if (cutInds) free(cutInds);
  if (cutCoefs) free(cutCoefs);

  if (returnCode) {
    /* There was an error in the callback.Stop the Optimizer and remember the error. */
    XPRSinterrupt(prob, XPRS_STOP_USER);
    callbackError = 1;
  }
}

int main(void) {

  int returnCode = 0;

  XPRSprob prob = NULL;

  int solvestatus, solstatus;

  struct prob_data_s probData = { 0 };

  /* Initialize the Optimizer. */
  if (XPRSinit("") != 0) {
    char message[512];
    XPRSgetlicerrmsg(message, sizeof(message));
    fprintf(stderr, "Licensing error: %s\n", message);
    return 1;
  }

  /* Create a problem with a message handler so we can see what is going on. */
  CHECK_RETURN(XPRScreateprob(&prob));
  CHECK_RETURN(XPRSaddcbmessage(prob, cbMessage, NULL, 0));

  /* Create our Economic Lot Sizing problem. */
  CHECK_RETURN(LoadELSProblem(prob, &probData));
  CHECK_RETURN(XPRSwriteprob(prob, "els_managedcuts", "l"));

  /* Add a CutRound callback for separating our cuts. */
  CHECK_RETURN(XPRSaddcbcutround(prob, cbCutRound, &probData, 0));

  /* Solve the ELS problem. */
  CHECK_RETURN(XPRSoptimize(prob, "", &solvestatus, &solstatus));
  if (callbackError) {
    returnCode = -3;
    goto cleanup;
  }

  if ((solvestatus == XPRS_SOLVESTATUS_COMPLETED) && (solstatus == XPRS_SOLSTATUS_OPTIMAL)) {
    printf("Solved problem to optimality.\n");
  } else {
    printf("Failed to solve problem with solvestatus %i and solstatus %i\n", solvestatus, solstatus);
  }

cleanup:

  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file repair.c
   `````````````
   Demonstrates the FICO repairinfeas utility.

   Prints a relaxation summary and creates the relaxed subproblem.

   (c) 2017-2026 Fair Isaac Corporation
***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

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

static void messagecb(XPRSprob cbprob, void* cbdata,
                              const char *msg, int len, int msgtype);

static int Getinfeasibilitybreakers(XPRSprob prob, const double *x,
                                    const double *slacks, double constraints[],
                                    double bounds[], const double tolarence);
static int Generatefeasibilityreport(XPRSprob prob,
                                     const double *constraintviolations,
                                     const double *boundviolations);
static int Getrowbounds(XPRSprob prob, const int row, double *lb, double *ub);
static int Applyrepairinfeasresulttoproblem(XPRSprob prob,
                                            const double *constraintviolations,
                                            const double *boundviolations);
static int  Setrowbounds(XPRSprob prob, const int i, const double lb, const
                         double ub);

int main(int argc, char **argv) {
  int returnCode = 0;
  XPRSprob prob = NULL;                   /* The problem instance */
  int i, j;
  int Status;                             /* Status of Optimizer subroutines */
  char const *sProblem = DATADIR"/iisexample";  /* Problem name */
  int nrows, ncols;
  double *x = NULL, *slacks = NULL;
  double *constraintviolations = NULL, *boundviolations = NULL;
  int ndiscretecols, nsets;

  double *lrp_array = NULL;   /* Array of size ROWS containing the preferences
                                 for relaxing the less or equal side of row. */
  double *grp_array = NULL;   /* Array of size ROWS containing the preferences
                                 for relaxing the greater or equal side of a row. */
  double *lbp_array = NULL;   /* Array of size COLS containing the preferences
                                 for relaxing lower bounds. */
  double *ubp_array = NULL;   /* Array of size COLS containing preferences for
                                 relaxing upper bounds. */

  /* Parse the arguments to the program. */
  if (argc != 1 && argc != 2) {
    printf("syntax: repair [filename] \n");
    return(1);
  }
  if (argc == 2)
    sProblem = argv[1];

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

  /* Load problem file */
  CHECK_RETURN( XPRSreadprob(prob, sProblem, "") );

  /* Get problem size */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTROWS, &nrows) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTCOLS, &ncols) );

  /* Allocate memory for the preference arrays, the solution and the
   * infeasibility breakers
   */
  lrp_array = malloc(nrows*sizeof(*lrp_array));
  grp_array = malloc(nrows*sizeof(*grp_array));
  lbp_array = malloc(ncols*sizeof(*lbp_array));
  ubp_array = malloc(ncols*sizeof(*ubp_array));
  x                    = malloc(ncols*sizeof(*x));
  slacks               = malloc(nrows*sizeof(*slacks));
  boundviolations      = malloc(ncols*sizeof(*boundviolations));
  constraintviolations = malloc(nrows*sizeof(*constraintviolations));
  if (!lrp_array || !grp_array || !lbp_array || !ubp_array ||
      !x || !slacks || !boundviolations || !constraintviolations )
  {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  /* Set relaxation values */
  for (i=0; i<nrows; i++) {
    lrp_array[i] = 1;
    grp_array[i] = 1;
  }
  for (j=0; j<ncols; j++) {
    lbp_array[j] = 1;
    ubp_array[j] = 1;
  }

  /* Call repairinfeas */
  Status = -1; /* set a dummy value */
  CHECK_RETURN( XPRSrepairweightedinfeas(prob, &Status, lrp_array, grp_array,
                                         lbp_array, ubp_array, 'n', 0.001, "") );

  printf("Repairinfeas return code (%i): ", Status);
  switch(Status) {
  case 0: printf("relaxed optimum found\n"); break;
  case 1: printf("relaxed problem is infeasible\n"); break;
  case 2: printf("relaxed problem is unbounded\n"); break;
  case 3: printf("solution of the relaxed problem regarding the original objective is nonoptimal\n"); break;
  case 4: printf("error\n"); break;
  case 5: printf("numerical instability\n"); break;
  }

  XPRSgetmipentities(prob, &ndiscretecols, &nsets, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL);

  /* Get solution */
  XPRSgetsolution(prob, NULL, x, 0, ncols - 1);
  XPRSgetslacks(prob, NULL, slacks, 0, nrows - 1);

  /* Get the values of the breaker variables */
  CHECK_RETURN( Getinfeasibilitybreakers(prob, x, slacks, constraintviolations,
                                         boundviolations, 1.0e-6) );

  /* Print report */
  CHECK_RETURN( Generatefeasibilityreport(prob, constraintviolations,
                                          boundviolations) );

  /* Repair and save problem */
  CHECK_RETURN( Applyrepairinfeasresulttoproblem(prob, constraintviolations,
                                                 boundviolations) );
  CHECK_RETURN( XPRSwriteprob(prob, "repaired.lp","lp") );
  printf("Repaired problem is written as repaired.lp.\n");
  printf("(Please note that this matrix might show slightly different behaviour then the repairinfeas problem)\n\n");

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
  free(lrp_array);
  free(grp_array);
  free(lbp_array);
  free(ubp_array);

  free(x);
  free(slacks);
  free(boundviolations);
  free(constraintviolations);

  /* Destroy problem object and free environment */
  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

/*
  This function returns the lower and upper bounds for a row.
*/
int Getrowbounds(XPRSprob prob, const int i, double *lb, double *ub) {
  int returnCode = 0;
  char rowtype;
  double rhs, range;

  /* the bounds are calculated using the rhs, the range and the type of the row
   */
  CHECK_RETURN( XPRSgetrhs(prob, &rhs, i, i) );
  CHECK_RETURN( XPRSgetrhsrange(prob, &range, i, i) );
  CHECK_RETURN( XPRSgetrowtype(prob, &rowtype, i, i) );

  switch (rowtype) {
  case 'L': {
    *lb = XPRS_MINUSINFINITY;
    *ub = rhs;
  } break;
  case 'E': {
    *lb = rhs;
    *ub = rhs;
  } break;
  case 'G': {
    *lb = rhs;
    *ub = XPRS_PLUSINFINITY;
  } break;
  case 'R': {
    *lb = rhs-range;
    *ub = rhs;
  } break;
  default: {
    *lb = XPRS_MINUSINFINITY;
    *ub = XPRS_PLUSINFINITY;
  }
  }

 cleanup:
  return returnCode;
}

/*
  This function sets new lower and upper bounds for a row.
*/
int Setrowbounds(XPRSprob prob, const int i, const double dlb, const double
                 dub) {
  int returnCode = 0;
  char rowtype;
  double lb, ub, range;

  if (dlb < dub) {
    lb = dlb;
    ub = dub;
  } else {
    lb = dub;
    ub = dlb;
  }

  /* determine row type and set bounds */
  if (lb <= XPRS_MINUSINFINITY && ub >= XPRS_PLUSINFINITY) {
    rowtype = 'N';
    CHECK_RETURN( XPRSchgrowtype(prob, 1, &i, &rowtype) );
  } else {
    if (lb <= XPRS_MINUSINFINITY) {
      rowtype = 'L';
      CHECK_RETURN( XPRSchgrowtype(prob, 1, &i, &rowtype) );
      CHECK_RETURN( XPRSchgrhs(prob, 1, &i, &ub) );
    } else {
      if (ub >= XPRS_PLUSINFINITY) {
        rowtype = 'G';
        CHECK_RETURN( XPRSchgrowtype(prob, 1, &i, &rowtype) );
        CHECK_RETURN( XPRSchgrhs(prob, 1, &i, &lb) );
      } else {
        if (lb == ub) {
          rowtype = 'E';
          CHECK_RETURN( XPRSchgrowtype(prob, 1, &i, &rowtype) );
          CHECK_RETURN( XPRSchgrhs(prob, 1, &i, &ub) );
        } else {
          rowtype = 'L';
          CHECK_RETURN( XPRSchgrowtype(prob, 1, &i, &rowtype) );
          CHECK_RETURN( XPRSchgrhs(prob, 1, &i, &ub) );
          range = ub-lb;
          CHECK_RETURN( XPRSchgrhsrange(prob, 1, &i, &range) );
        }
      }
    }
  }
 cleanup:
  return returnCode;
}

/*
  This function calculates the infeasibilities of a solution (i.e. the values
  of the feasibility breakers after a repairinfeas call)
*/
int Getinfeasibilitybreakers(XPRSprob prob, const double *x,
                             const double *slacks, double constraints[],
                             double bounds[], const double tolarence) {
  int returnCode = 0;
  int i, j, ncols, nrows;
  double rhs, activity;
  double lb, ub;

  /* get problem dimensions */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTROWS, &nrows) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTCOLS, &ncols) );

  /* check constraints */
  for (i=0; i<nrows; i++) {

    CHECK_RETURN( XPRSgetrhs(prob, &rhs, i, i) );
    activity = rhs - slacks[i];
    CHECK_RETURN( Getrowbounds(prob, i, &lb, &ub) );

    if (activity < lb-tolarence) {
      constraints[i] = activity-lb;  /* a negative value indicates that the
                                        lower bound is relaxed */
    } else
      if (activity > ub+tolarence) {
        constraints[i] = activity-ub;  /* a positive value indicates that the
                                          upper bound is relaxed */
      } else {
        constraints[i] = 0.0;
      }
  }

  /* check bounds */
  for (j=0; j<ncols; j++) {
    CHECK_RETURN( XPRSgetlb(prob, &lb, j, j) );
    CHECK_RETURN( XPRSgetub(prob, &ub, j, j) );

    if (x[j] < lb-tolarence) {
      bounds[j] = x[j]-lb;  /* a negative value indicates that the lower bound
                               is relaxed */
    } else {
      if (x[j] > ub+tolarence) {
        bounds[j] = x[j]-ub;  /* a positive value indicates that the upper
                                 bound is relaxed */
      } else {
        bounds[j] = 0.0;
      }
    }
  }
 cleanup:
  return returnCode;
}

#define BOUNDCHARS 25 /* Length of buffers passed to Bound_to_string() */
/* This function just converts a double to a string for reporting, converting
   infinity values as "NONE" */
void Bound_to_string(double bound, char string[BOUNDCHARS]) {
  if (bound > XPRS_MINUSINFINITY && bound < XPRS_PLUSINFINITY) {
    if (bound >= 0)
      snprintf(string, BOUNDCHARS, " %.5e", bound);
    else
      snprintf(string, BOUNDCHARS, "%.5e", bound);
  } else {
    memcpy(string,"    NONE     \0", 14);
  }
}

/*
  Prints a summary of the infeasibily breakers using the already calculated
  infeasibities
*/
int Generatefeasibilityreport(XPRSprob prob, const double
                              *constraintviolations,
                              const double *boundviolations) {
  int returnCode = 0;
  int i, j;
  int nrows, ncols;
  double suminf = 0;                          /* Sum if infeasibility */
  int    ninf = 0;                            /* Number of rows and columns
                                                 reapired */
  int    len, NameLength;                     /* Name lengths in the LP file */
  char  *name = NULL;
  double lb, ub;
  char   slb[BOUNDCHARS], sub[BOUNDCHARS];    /* Fixed length name buffers */
  char   slbshift[BOUNDCHARS], subshift[BOUNDCHARS];

  /* Get name lengths */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTROWS, &nrows) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTCOLS, &ncols) );

  /* Get the maximum name length */
  NameLength = 0;
  for(i=0;i<nrows;i++) {
    CHECK_RETURN( XPRSgetnamelist(prob, 1, NULL, 0, &len, i, i) );
    if (len>NameLength)
      NameLength=len;
  }
  for(i=0;i<ncols;i++) {
    CHECK_RETURN( XPRSgetnamelist(prob, 2, NULL, 0, &len, i, i) );
    if (len>NameLength)
      NameLength=len;
  }

  /* Allocate name buffer */
  name = malloc(NameLength);
  if (!name) {
    perror("malloc");
    returnCode = -2;
    goto cleanup;
  }

  printf("\n");
  printf("\n");
  printf("XPRESS-MP FEASIBILITY REPAIR REPORT.\n");
  printf("\n");
  printf("The following constraints were repaired.\n");
  printf("\n");

  printf("Index     Name%*s  Lower bound    Repaired        Upper bound    Repaired \n", NameLength-4,"");

  for (i=0; i<nrows; i++) {
    if (constraintviolations[i] != 0) {

      /* get constraint name */
      CHECK_RETURN( XPRSgetnamelist(prob, 1, name, NameLength, NULL, i, i) );
      CHECK_RETURN( Getrowbounds(prob, i, &lb, &ub) );
      Bound_to_string(lb, slb); Bound_to_string(ub, sub);

      suminf += fabs(constraintviolations[i]);
      ninf++;

      if (constraintviolations[i] < 0) {
        Bound_to_string(lb+constraintviolations[i], slbshift);
        Bound_to_string(ub, subshift);
      } else {
        Bound_to_string(lb, slbshift);
        Bound_to_string(ub+constraintviolations[i], subshift);
      }

      printf("%.5i  %*s    %*s  %*s   %*s  %*s\n", i, NameLength, name, 7, slb,
             7, slbshift, 7, sub, 7, subshift);
    }
  }

  printf("\n");
  printf("The following bound constraints were repaired.\n");
  printf("\n");

  printf("Index   Name%*s   Lower bound      Repaired     Upper bound      Repaired \n", NameLength-4,"");

  for (j=0; j<ncols; j++) {
    if (boundviolations[j] != 0) {

      /* get constraint name */
      CHECK_RETURN( XPRSgetnamelist(prob, 2, name, NameLength, NULL, j, j) );
      CHECK_RETURN( XPRSgetlb(prob, &lb, j, j) );
      CHECK_RETURN( XPRSgetub(prob, &ub, j, j) );
      Bound_to_string(lb, slb); Bound_to_string(ub, sub);

      suminf += fabs(boundviolations[j]);
      ninf++;

      if (constraintviolations[j] < 0) {
        Bound_to_string(boundviolations[j], slbshift);
        Bound_to_string(ub, subshift);
      } else {
        Bound_to_string(lb, slbshift);
        Bound_to_string(boundviolations[j], subshift);
      }

      printf("%.5i  %*s    %*s  %*s   %*s  %*s\n", j, NameLength, name, 7, slb,
             7, slbshift, 7, sub, 7, subshift);
    }
  }

  printf("\n");
  printf("Sum of Violations %f\n", suminf);
  printf("Number of corrections: %i\n", ninf);
  printf("\n");

 cleanup:
  free(name);
  return returnCode;
}

/*
  Relaxes the problem given the values of the feasibility breakers
*/
int Applyrepairinfeasresulttoproblem(XPRSprob prob,
                                     const double *constraintviolations,
                                     const double *boundviolations)
{
  int returnCode = 0;
  int i, j;
  int nrows, ncols;
  char boundtype;
  double lb, ub;

  /* Get name lengths */
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTROWS, &nrows) );
  CHECK_RETURN( XPRSgetintattrib(prob, XPRS_INPUTCOLS, &ncols) );

  /* apply changes to rows */
  for (i=0; i<nrows; i++) {
    if (constraintviolations[i] != 0) {

      /* get constraint name */
      CHECK_RETURN( Getrowbounds(prob, i, &lb, &ub) );

      /* apply relaxation */
      if (constraintviolations[i] > 0) {
        ub += constraintviolations[i];
      } else {
        lb += constraintviolations[i];
      }

      CHECK_RETURN( Setrowbounds(prob, i, lb, ub) );
    }
  }

  /* apply changes to columns */
  for (j=0; j<ncols; j++) {
    if (boundviolations[j] != 0) {

      /* get constraint name */
      CHECK_RETURN( XPRSgetlb(prob, &lb, j, j) );
      CHECK_RETURN( XPRSgetub(prob, &ub, j, j) );

      if (boundviolations[j] > 0) {
        boundtype = 'U';
        ub += boundviolations[j];
        CHECK_RETURN( XPRSchgbounds(prob, 1, &j, &boundtype, &ub) );
      } else {
        boundtype = 'L';
        lb += boundviolations[j];
        CHECK_RETURN( XPRSchgbounds(prob, 1, &j, &boundtype, &lb) );
      }
    }
  }

  printf("\n");
  printf("Problem repaired.");
  printf("\n");

 cleanup:
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

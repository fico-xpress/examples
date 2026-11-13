/***************************************************************

  Goal programming example
  ========================

  An example of lexicographic goal programming using the Xpress
  multi-objective API.

  A company produces two electrical products, A and B. Both require
  two stages of production: wiring and assembly. The production plan
  must meet several goals:
  1. A profit of $200
  2. A contractual requirement of 40 units of product B
  3. To fully utilize the available wiring department hours
  4. To avoid overtime in the assembly department

  (c) 2022-2026 Fair Isaac Corporation
*****************************************************************/

#include <stdio.h>
#include "xprs.h"

#define COLS     8    /* Number of columns */
#define ROWS     4    /* Number of rows */
#define ENTITIES 2    /* Number of entities */

/* Column indices */
#define COL_PRODUCE_A       0
#define COL_PRODUCE_B       1
#define COL_SURPLUS_WIRING  2
#define COL_DEFICIT_WIRING  3
#define COL_SURPLUS_ASSEM   4
#define COL_DEFICIT_ASSEM   5
#define COL_DEFICIT_PROFIT  6
#define COL_DEFICIT_PROD_B  7

/* Calls an Xpress optimizer function and checks the return code.
 * If the call fails then the macro
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

int main(void)
{
  int returnCode = 0;
  XPRSprob prob = NULL;
  int i, solvestatus, solstatus;

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

  /* Define columns */
  {
    double objcoef[COLS];
    double lb[COLS];
    double ub[COLS];
    int start[COLS + 1];

    for (i = 0; i < COLS; i++) {
      objcoef[i] = 0;
      lb[i] = 0;
      ub[i] = XPRS_PLUSINFINITY;
      start[i] = 0;
    }
    start[i] = 0;

    CHECK_RETURN( XPRSaddcols(prob, COLS, 0, objcoef, start, NULL, NULL, lb, ub) );
  }

  /* First two columns are integer */
  {
    int entind[ENTITIES] = {COL_PRODUCE_A, COL_PRODUCE_B};
    char coltype[ENTITIES] = {'I', 'I'};

    CHECK_RETURN( XPRSchgcoltype(prob, 2, entind, coltype) );
  }

  /* Add one row per goal */
  {
    double rhs[] = {
      200,  /* Profit goal is $200 */
      40,   /* Production goal for product B is 40 units */
      120,  /* 120 wiring hours are available */
      300   /* 300 assembly hours are available */
    };
    char rowtype[] = {
      'G', 'G',   /* Profit and production goals should be met or exceeded */
      'E', 'E'    /* Workforce utilization goals should be met */
    };
    int start[] = {0, 2, 4, 8, 12};
    double rowcoef[] = {
      7, 6, 1,      /* Profit for products A and B + deficit in profit */
      1, 1,         /* Units of product B produced + deficit in units produced */
      2, 3, -1, 1,  /* Products A/B require 2/3 hours of wiring, - surplus + deficit */
      6, 5, -1, 1   /* Products A/B require 6/5 hours of wiring, - surplus + deficit */
    };
    int colind[] = {
      COL_PRODUCE_A, COL_PRODUCE_B, COL_DEFICIT_PROFIT,
      COL_PRODUCE_B, COL_DEFICIT_PROD_B,
      COL_PRODUCE_A, COL_PRODUCE_B, COL_SURPLUS_WIRING, COL_DEFICIT_WIRING,
      COL_PRODUCE_A, COL_PRODUCE_B, COL_SURPLUS_ASSEM, COL_DEFICIT_ASSEM
    };

    CHECK_RETURN( XPRSaddrows(prob, ROWS, sizeof(colind) / sizeof(int),
                              rowtype, rhs, NULL, start, colind, rowcoef) );
  }

  /* Define objectives to minimize deviations, in priority order */
  {
    int colind[2];
    double objcoef[2] = {1, 1};
    colind[0] = COL_DEFICIT_PROFIT;   /* Goal 1: minimize profit deficit */
    CHECK_RETURN (XPRSchgobj(prob, 1, colind, objcoef) );
    colind[0] = COL_DEFICIT_PROD_B;   /* Goal 2: minimize production deficit */
    CHECK_RETURN (XPRSchgobjn(prob, 1, 1, colind, objcoef) );
    colind[0] = COL_SURPLUS_WIRING;   /* Goal 3: minimize deviation from wiring hours target */
    colind[1] = COL_DEFICIT_WIRING;
    CHECK_RETURN (XPRSchgobjn(prob, 2, 2, colind, objcoef) );
    colind[0] = COL_SURPLUS_ASSEM;    /* Goal 4: minimize deviation from assembly hours target */
    colind[1] = COL_DEFICIT_ASSEM;
    CHECK_RETURN (XPRSchgobjn(prob, 3, 2, colind, objcoef) );

    /* Set up objective priorities and tolerances */
    for (i = 0; i < 4; i++) {
      CHECK_RETURN (XPRSsetobjintcontrol(prob, i, XPRS_OBJECTIVE_PRIORITY, 4 - i) );
      CHECK_RETURN (XPRSsetobjdblcontrol(prob, i, XPRS_OBJECTIVE_ABSTOL, 0) );
      CHECK_RETURN (XPRSsetobjdblcontrol(prob, i, XPRS_OBJECTIVE_RELTOL, 0) );
    }
  }

  /* Solve the problem */
  CHECK_RETURN( XPRSoptimize(prob, "", &solvestatus, &solstatus) );

  /* Print the result */
  if (solvestatus == XPRS_SOLVESTATUS_COMPLETED && solstatus == XPRS_SOLSTATUS_OPTIMAL) {
    double sol[COLS];
    CHECK_RETURN (XPRSgetsolution(prob, NULL, sol, 0, COLS - 1) );
    printf("Production plan:\n");
    printf("Product A: %d units\n", (int) sol[COL_PRODUCE_A]);
    printf("Product B: %d units\n", (int) sol[COL_PRODUCE_B]);
    printf("Profit: $%d\n", (int) (7 * sol[COL_PRODUCE_A] + 6 * sol[COL_PRODUCE_B]));
    if (sol[COL_DEFICIT_PROFIT] > 0) {
      printf("Profit goal missed by $%d\n", (int) sol[COL_DEFICIT_PROFIT]);
    }
    if (sol[COL_DEFICIT_PROD_B] > 0) {
      printf("Contractual goal for product B missed by %d units\n", (int) sol[COL_DEFICIT_PROD_B]);
    }
    if (sol[COL_SURPLUS_WIRING] > 0) {
      printf("Unused wiring department hours: %d\n", (int) sol[COL_SURPLUS_WIRING]);
    }
    if (sol[COL_DEFICIT_WIRING] > 0) {
      printf("Wiring department overtime: %d\n", (int) sol[COL_DEFICIT_WIRING]);
    }
    if (sol[COL_SURPLUS_ASSEM] > 0) {
      printf("Unused assembly department hours: %d\n", (int) sol[COL_SURPLUS_ASSEM]);
    }
    if (sol[COL_DEFICIT_ASSEM] > 0) {
      printf("Assembly department overtime: %d\n", (int) sol[COL_DEFICIT_ASSEM]);
    }
  } else {
    printf("Problem could not be solved\n");
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
  XPRSdestroyprob(prob);
  XPRSfree();

  return returnCode;
}

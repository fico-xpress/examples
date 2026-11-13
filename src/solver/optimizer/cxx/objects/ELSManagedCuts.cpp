// (c) 2023-2026 Fair Isaac Corporation
/** Demonstrates how to implement cutting planes as part of a MIP
 * branch-and-bound search using the cutround callback.
 *
 * Cuts are added as user cuts using XpressProblem.addManagedCut().
 *
 * Economic lot sizing problem. Solved by adding (l,S)-inequalities
 * in a branch-and-cut heuristic (using the cutround callback).
 *
 * ELS considers production planning over a horizon of T periods. In period t,
 * t=1,...,T, there is a given demand DEMAND[p,t] that must be satisfied by
 * production produce[p,t] in period t and by inventory carried over
 * from previous periods.
 * There is a set-up cost SETUPCOST[t] associated with production in
 * period t and the total production capacity per period is limited. The unit
 * production cost in period t is PRODCOST[p,t]. There is no
 * inventory or stock-holding cost.
 *
 * A well-known class of valid inequalities for ELS are the
 * (l,S)-inequalities.  Let D(p,q) denote the demand in periods p
 * to q and y(t) be a binary variable indicating whether there is any
 * production in period t.  For each period l and each subset of periods S
 * of 1 to l, the (l,S)-inequality is
 * <pre>
 *      sum (t=1:l | t in S) x(t) + sum (t=1:l | t not in S) D(t,l) * y(t)
 *          &gt;= D(1,l)
 * </pre>
 *
 * It says that actual production x(t) in periods included S plus maximum
 * potential production D(t,l)*y(t) in the remaining periods (those not in
 * S) must at least equal total demand in periods 1 to l.  Note that in
 * period t at most D(t,l) production is required to meet demand up to
 * period l.
 *
 * Based on the observation that
 * <pre>
 *      sum (t=1:l | t in S) x(t) + sum (t=1:l | t not in S) D(t,l) * y(t)
 *          &gt;= sum (t=1:l) min(x(t), D(t,l) * y(t))
 *          &gt;= D(1,l)
 * </pre>
 * it is easy to develop a separation algorithm and thus a cutting plane
 * algorithm based on these (l,S)-inequalities.
 */

#include <xpress.hpp>
#include <xpress_objects.hpp>
#include <iostream>

using namespace xpress;
using namespace xpress::objects;
using namespace xpress::objects::utils;

/** Tolerance for satisfiability. */
#define EPS 1e-6

/** Number of time periods. */
#define DIM_TIMES 15
/** Number of products to produce. */
#define DIM_PRODUCTS  4

/** Demand per product (first dim) and time period (second dim). */
static int const DEMAND[DIM_PRODUCTS][DIM_TIMES] = {
  {2, 3, 5, 3, 4, 2, 5, 4, 1, 3, 4, 2, 3, 5, 2},
  {3, 1, 2, 3, 5, 3, 1, 2, 3, 3, 4, 5, 1, 4, 1},
  {3, 5, 2, 1, 2, 1, 3, 3, 5, 2, 2, 1, 3, 2, 3},
  {2, 2, 1, 3, 2, 1, 2, 2, 3, 3, 2, 2, 3, 1, 2}
};
/** Setup cost. */
static double const SETUPCOST[DIM_TIMES] = {
  17, 14, 11, 6, 9, 6, 15, 10, 8, 7, 12, 9, 10, 8, 12
};
/** Production cost per product (first dim) and time period (second dim). */
static double const PRODCOST[DIM_PRODUCTS][DIM_TIMES] = {
  {5, 3, 2, 1, 3, 1, 4, 3, 2, 2, 3, 1, 2, 3, 2},
  {1, 4, 2, 3, 1, 3, 1, 2, 3, 3, 3, 4, 4, 2, 2},
  {3, 3, 3, 4, 4, 3, 3, 3, 2, 2, 1, 1, 3, 3, 3},
  {2, 2, 2, 3, 3, 3, 4, 4, 4, 3, 3, 2, 2, 2, 3}
};
/** Capacity. */
static const int CAP[DIM_TIMES] = {
  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};

/** Cut round callback for separating our cutting planes. */
class Callback {
  std::vector<std::vector<Variable>> const &produce;
  std::vector<std::vector<Variable>> const &setup;
  double const (&sumDemand)[DIM_PRODUCTS][DIM_TIMES][DIM_TIMES];
 public:
  Callback(std::vector<std::vector<Variable>> const &p,
           std::vector<std::vector<Variable>> const &s,
           double const (&d)[DIM_PRODUCTS][DIM_TIMES][DIM_TIMES])
  : produce(p)
  , setup(s)
  , sumDemand(d)
  {
  }
  void operator()(XpressProblem &prob, int, int *) const {
    // Apply only one round of cutting on each node.
    // Because the CutRound callback is fired *before* a round of
    // cutting, the CUTROUNDS attribute will start from 0 on the first
    // invocation of the callback.
    if (prob.attributes.getCutRounds() >= 1)
      return;

    // Get the solution vector.
    // Xpress will only fire the CutRound callback when a solution is
    // available, so there is no need to check whether a solution is
    // available.
    auto sol = prob.getCallbackSolution();

    // Search for violated constraints : the minimum of the actual
    // production produce[p][t] and the maximum potential production
    // D[p][t][l]*setup[p][t] in periods 0 to l must at least equal
    // the total demand in periods 0 to l.
    //    sum(t=1:l) min(prod[p][t], D[p][t][l]*setup[p][t]) >= D[p][0][l]
    int nCutsAdded = 0;
    for (int p = 0; p < DIM_PRODUCTS; p++) {
      for (int l = 0; l < DIM_TIMES; l++) {
        double sum = 0.0;
        for (int t = 0; t <= l; t++) {
          if (produce[p][t].getValue(sol) < sumDemand[p][t][l] * setup[p][t].getValue(sol) + EPS) {
            sum += produce[p][t].getValue(sol);
          }
          else {
            sum += sumDemand[p][t][l] * setup[p][t].getValue(sol);
          }
        }
        if (sum < sumDemand[p][0][l] - EPS) {
          // Create the violated cut.
          LinExpression cut = LinExpression::create();
          for (int t = 0; t <= l; t++) {
            if (produce[p][t].getValue(sol) < sumDemand[p][t][l] * setup[p][t].getValue(sol)) {
              cut.addTerm(1.0, produce[p][t]);
            }
            else {
              cut.addTerm(sumDemand[p][t][l], setup[p][t]);
            }
          }

          // Give the cut to Xpress to manage.
          // It will automatically be presolved.
          prob.addManagedCut(true, cut >= sumDemand[p][0][l]);
          ++nCutsAdded;

          // If we modified the problem in the callback, Xpress
          // will automatically trigger another roound of cuts,
          // so there is no need to set the action return
          // argument.
        }
      }
    }
    if (nCutsAdded > 0) {
      std::cout << "Cuts added: " << nCutsAdded << std::endl;
    }
  }
};

int main(void) {
  XpressProblem prob;
  prob.callbacks.addMessageCallback(XpressProblem::console);

  // Create an economic lot sizing problem.
  // Calculate demand D(p, s, t) as the demand for product p from
  // time s to time t(inclusive).
  double sumDemand[DIM_PRODUCTS][DIM_TIMES][DIM_TIMES];
  for (int p = 0; p < DIM_PRODUCTS; p++) {
    for (int s = 0; s < DIM_TIMES; s++) {
      double thisSumDemand = 0.0;
      for (int t = s; t < DIM_TIMES; t++) {
        thisSumDemand += DEMAND[p][t];
        sumDemand[p][s][t] = thisSumDemand;
      }
    }
  }

  auto produce = prob.addVariables(DIM_PRODUCTS, DIM_TIMES)
    .withName([](auto p, auto t){ return xpress::format("produce(%d,%d)", p, t); })
    .toArray();
  auto setup = prob.addVariables(DIM_PRODUCTS, DIM_TIMES)
    .withType(ColumnType::Binary)
    .withName([](auto p,auto t){ return xpress::format("setup(%d,%d)", p, t); })
    .toArray();

  // Add the objective function :
  // MinCost:= sum(t in TIMES) (SETUPCOST(t) * sum(p in PRODUCTS) setup(p,t) +
  //           sum(p in PRODUCTS) PRODCOST(p, t) *produce(p, t) )
  prob.setObjective(sum(DIM_TIMES, DIM_PRODUCTS,
                        [&](auto t, auto p){
                          return SETUPCOST[t] * setup[p][t] + PRODCOST[p][t] * produce[p][t];
                        }));

  // Add constraints.
  // Production in periods 0 to t must satisfy the total demand
  // during this period of time, for all t in [0,T[
  //    forall(p in PRODUCTS, t in TIMES)
  //      Dem(t) : = sum(s in 1..t) produce(p,s) >= sum(s in 1..t) DEMAND(s)
  prob.addConstraints(DIM_PRODUCTS, DIM_TIMES,
                      [&](auto p, auto t){
                        return sum(t + 1,
                                   [&](auto s){
                                     return produce[p][s];
                                   })
                          >= sumDemand[p][0][t];
                      });

  // If there is production during t then there is a setup in t :
  //    forall(p in PRODUCTS, t in TIMES)
  //       ProdSetup(t) : = produce(t) <= D(t,TIMES) * setup(t)
  for (int p = 0; p < DIM_PRODUCTS; ++p) {
    for (int t = DIM_TIMES - 1; t >= 0; --t) {
      prob.addConstraint(produce[p][t] <= sumDemand[p][t][DIM_TIMES - 1] * setup[p][t]);
    }
  }

  // Capacity limits :
  //    forall(t in TIMES) Capacity(t) : = sum(p in PRODUCTS) produce(p, t) <= CAP(t)
  prob.addConstraints(DIM_TIMES,
                      [&](auto t){
                        return sum(DIM_PRODUCTS,
                                   [&](auto p) {
                                     return produce[p][t];
                                   })
                          <= CAP[t];
                      });

  // Add a CutRound callback for separating our cuts.
  Callback cb(produce, setup, sumDemand);
  prob.callbacks.addCutRoundCallback(cb);

  prob.writeProb("elsmanagedcuts.lp");

  prob.optimize();

  if (prob.attributes.getSolveStatus() == SolveStatus::Completed &&
      prob.attributes.getSolStatus() == SolStatus::Optimal) {
    std::cout << "Solved problem to optimality." << std::endl;
  }
  else {
    std::cout << "Failed to solve problem with solvestatus "
              << to_string(prob.attributes.getSolveStatus())
              << " and solstatus "
              << to_string(prob.attributes.getSolStatus())
              << std::endl;
  }

  return 0;
}

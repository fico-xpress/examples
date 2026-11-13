// (c) 2024-2026 Fair Isaac Corporation

/**
 * Holy Grail. When the Sangraal (Holy Grail) is almost won the hero arrives at
 * a castle where he finds 8 imprisoned knights. He is facing the task to bring
 * the largest possible number of knights for the arrival of the Sangraal in
 * twenty minutes' time. The time required for freeing a knight depends on his
 * state of binding. A freed knight then needs a given amount of time to wash
 * and recover himself physically.
 *
 * Model formulation using indicator constraints.
 *
 * Description and original model by M. Chlond:
 * http://ite.pubs.informs.org/Vol4No3/Chlond
 */

#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/** A knight. */
struct Knight {
  /** The name of the knight. */
  std::string const name;
  /** How long it takes to free the knight. */
  double const freeTime;
  /** How long it takes the knight to wash and prepare himself. */
  double const washTime;

  Knight(std::string const &name, double freeTime, double washTime)
      : name(name), freeTime(freeTime), washTime(washTime) {}
};

/** The knights used in this example. */
Knight const knightArray[] = {
    Knight{"Agravain", 1, 15}, Knight{"Bors", 1, 5},
    Knight{"Caradoc", 2, 15},  Knight{"Dagonet", 2, 5},
    Knight{"Ector", 3, 10},    Knight{"Feirefiz", 4, 15},
    Knight{"Gareth", 5, 10},   Knight{"Harry", 6, 5}};
/** The number of nights in this example. */
int const knightCount = sizeof(knightArray) / sizeof(knightArray[0]);

/** The resource constraints used in this example. */
double const maxTime = 20.0;

int main() {
  XpressProblem prob;

  // Output all messages.
  prob.callbacks.addMessageCallback(XpressProblem::console);

  /**** VARIABLES ****/

  // Whether to free knight i as the j-th knight
  auto order = prob.addVariables(knightCount, knightCount)
                   .withName([](auto i, auto j) {
                     return xpress::format("order_%d_%d", i, j);
                   })
                   .withType(ColumnType::Binary)
                   .toArray();

  // Whether the knight who was freed as the i-th knight will be ready in time
  auto onTime =
      prob.addVariables(knightCount)
          .withName([](auto i) { return xpress::format("onTime_%d", i); })
          .withType(ColumnType::Binary)
          .toArray();

  // At what time the i-th freed knight will be ready
  auto readyTime =
      prob.addVariables(knightCount)
          .withName([](auto i) { return xpress::format("readyTime_%d", i); })
          .toArray();

  // Maximize the number of knights that are ready after 20 minutes
  Expression totalReady = sum(onTime);

  prob.setObjective(totalReady, ObjSense::Maximize);

  // Exactly one knight should be freed as the i-th knight
  prob.addConstraints(knightCount, [&](auto i) {
    return sum(knightCount, [&](auto j) { return order[i][j]; }) == 1.0;
  });

  // Each knight should be freed exactly once
  prob.addConstraints(knightCount, [&](auto i) {
    return sum(knightCount, [&](auto j) { return order[j][i]; }) == 1.0;
  });

  // The time each knight is ready, computed as the sum of times it took to free
  // all
  // previous knights, plus the time it took to free this knight plus the time
  // it takes the knight to wash and get ready

  // loop over all positions
  for (int p = 0; p < knightCount; p++) {
    LinExpression timeBeforeFree = LinExpression::create();
    // loop over all knights
    for (int k = 0; k < knightCount; k++) {
      // loop over all positions before the current one
      for (int q = 0; q < p; q++) {
        // If this knight was freed before the current position, add the time it
        // took to free them
        timeBeforeFree.addTerm(order[k][q] * knightArray[k].freeTime);
      }
      // if knight k was freed in this position, add the time it took to free
      // them and for them to prepare
      timeBeforeFree.addTerm(
          order[k][p] * (knightArray[k].freeTime + knightArray[k].washTime));
    }
    // add the actual constraint
    prob.addConstraint(timeBeforeFree == readyTime[p]);
  }

  // The i-th freed knight will be ready iff they were ready by time 20
  for (int p = 0; p < knightCount; p++) {
    Inequality isReady = prob.addConstraint(readyTime[p] <= maxTime);
    prob.setIndicator(onTime[p], true, isReady);
  }

  // Dump the problem to disk so that we can inspect it.
  prob.writeProb("sangraalind.lp", "l");

  // Solve
  prob.optimize();
  if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
      prob.attributes.getSolStatus() != SolStatus::Feasible)
    throw std::runtime_error("optimization failed with status " +
                             to_string(prob.attributes.getSolStatus()));
  auto sol = prob.getSolution();

  // Print the solution
  std::cout << static_cast<int>(round(prob.attributes.getObjVal()))
            << " knights ready in time:" << std::endl;
  for (int p = 0; p < knightCount; p++) {
    for (int k = 0; k < knightCount; k++) {
      if (order[k][p].getValue(sol) > 0.5) {
        std::string pos;
        if (p == 0) {
          pos = "1st";
        } else if (p == 1) {
          pos = "2nd";
        } else if (p == 2) {
          pos = "3rd";
        } else {
          pos = xpress::format("%dth", p + 1);
        }
        std::cout << pos << " freed knight: " << knightArray[k].name
                  << " ready at time " << readyTime[p].getValue(sol)
                  << std::endl;
      }
    }
  }

  return 0;
}

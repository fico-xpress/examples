// (c) 2024-2026 Fair Isaac Corporation

/**
 * A very simple multiple knapsack problem. The problem asks for assigning a set
 * of items to multiple knapsacks. The objective is to maximize profit while
 * respecting the per-knapsack weight restrictions. Each knapsack has a capacity
 * that limits the weight of items that can be put into it. The capacity can be
 * extended by a constant at a fixed cost.
 *
 * In this example, data for the knapsacks and items is given by arrays and
 * variables are arranged in arrays as well. They are indexed by knapsack and
 * item id.
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/** An item. */
struct Item {
  /** The weight of this item. */
  double weight;
  /** The profit (objective value) of this item. */
  double profit;
  /** The name of this item. */
  std::string name;

  Item(double weight = 0.0, double profit = 0.0, std::string const &name = "")
      : weight(weight), profit(profit), name(name) {}
};

/** A knapsack. */
struct Knapsack {
  /** Capacity of this item. */
  double capacity;
  /**
   * Extra capacity that can be bought for this item at cost
   * <code>extraCost</code>.
   */
  double extraCapacity;
  /**
   * The cost for increasing this knapsack's capacity by
   * <code>extraCapacity<code>.
   */
  double extraCost;
  /** The name of this knapsack. */
  std::string name;

  Knapsack(double capacity = 0.0, double extraCapacity = 0.0,
           double extraCost = 0.0, std::string const &name = "")
      : capacity(capacity), extraCapacity(extraCapacity), extraCost(extraCost),
        name(name) {}
};

/** The items to assign in this multiple knapsack instance. */
std::vector<Item> items{Item(2, 2, "item1"), Item(3, 3, "item2"),
                        Item(4, 4, "item3"), Item(5, 5, "item4"),
                        Item(6, 6, "item5"), Item(7, 7, "item6"),
                        Item(8, 8, "item7"), Item(9, 9, "item8")};

/** The knapsacks in this multiple knapsack instance. */
std::vector<Knapsack> knapsacks{Knapsack(15, 5, 2, "knapsack1"),
                                Knapsack(16, 4, 3, "knapsack2")};

/**
 * In this multiple knapsack instance we allow for buying some extra capacity in
 * each knapsack. There are different ways to model this extra capacity. This
 * enumeration indicates how this should be modeled.
 */
typedef enum {
  /** Do not allow any extra capacity. */
  NoExtraCapacity,
  /** Use binary variables to model the extra capacity. */
  BinaryVariables,
  /** Use indicator constraints to model the extra capacity. */
  IndicatorConstraints,
  /** Use a piecewise linear function to model extra capacity. */
  PiecewiseLinear,
  /** Use SOS constraints to model extra capacity. */
  SOSConstraint
} ExtendedCapacityModeling;

ExtendedCapacityModeling extendedCapacityModeling = NoExtraCapacity;

/**
 * Model and solve the multiple knapsack problem using the data that is provided
 * as arrays.
 */
int main(int argc = 0, char **argv = nullptr) {
  // If an argument was given on the command line then we assume that
  // this argument selects the way in which we are supposed to model
  // the extra capacity constraint.
  if (argc > 1) {
    std::string arg(argv[1]);
    if (arg == "NoExtraCapacity")
      extendedCapacityModeling = NoExtraCapacity;
    else if (arg == "BinaryVariables")
      extendedCapacityModeling = BinaryVariables;
    else if (arg == "IndicatorConstraints")
      extendedCapacityModeling = IndicatorConstraints;
    else if (arg == "PiecewiseLinear")
      extendedCapacityModeling = PiecewiseLinear;
    else if (arg == "SOS")
      extendedCapacityModeling = SOSConstraint;
    else
      throw std::runtime_error("unknown argument " + arg);
  }

  XpressProblem prob;
  // Create the variables.
  // This creates a two-dimensional array of variables.
  // The first index is the item index, the second index is the
  // the knapsack index.
  // All variables are binary and the name for variable x[i][j]
  // is "i@j".
  std::vector<std::vector<Variable>> x =
      prob.addVariables(items.size(), knapsacks.size())
          .withType(ColumnType::Binary)
          .withName(
              [](auto i, auto k) { return xpress::format("%d@%d", i, k); })
          .toArray();

  // Create the expression for the objective function.
  // sum(i in items) sum(k in knapsacks) i.profit * x[i][k]
  Expression profit = sum(items.size(), knapsacks.size(), [&](auto i, auto k) {
                        return items[i].profit * x[i][k];
                      });

  // Right-hand side for capacity constraint.
  std::vector<Expression> capacityRhs(knapsacks.size());
  switch (extendedCapacityModeling) {
  case NoExtraCapacity:
    // No extra capacity at all.
    for (unsigned k = 0; k < knapsacks.size(); ++k)
      capacityRhs[k] = ConstantExpression(knapsacks[k].capacity);
    break;
  case BinaryVariables:
    // Model extra capacity using an additional binary variable y[k]
    // for each knapsack k.
    // These variables are added to the objective with the extra
    // cost for the knapsack.
    // They are also added to capacityRhs[k] with the knapsack's
    // extra capacity as factor.
    // This way we get extra capacity and the cost for it if and
    // only if the binary variable is set to 1.
    {
      std::vector<Variable> y = prob.addVariables(knapsacks.size())
                                    .withType(ColumnType::Binary)
                                    .withName("y_%d")
                                    .toArray();
      profit = profit + sum(knapsacks.size(), [&](auto k) {
                 return -knapsacks[k].extraCost * y[k];
               });

      for (unsigned k = 0; k < knapsacks.size(); ++k)
        capacityRhs[k] =
            knapsacks[k].capacity + knapsacks[k].extraCapacity * y[k];
    }
    break;
  case IndicatorConstraints:
    // We model the additional capacity as follows:
    // - unconditionally add the extra capacity to each knapsack
    // - add an additional row that limits the capacity to the original capacity
    // - activate this new row only if a newly introduced indicator variable
    //   y is zero
    // - add the indicator variable to the objective function.
    {
      std::vector<Variable> y = prob.addVariables(knapsacks.size())
                                    .withType(ColumnType::Binary)
                                    .withName("y_%d")
                                    .toArray();
      profit = profit + sum(knapsacks.size(), [&](auto k) {
                 return -knapsacks[k].extraCost * y[k];
               });
      for (unsigned k = 0; k < knapsacks.size(); ++k) {
        int kIdx = k; // Effectively final for lambda
        Knapsack const &knapsack = knapsacks[kIdx];
        capacityRhs[k] =
            ConstantExpression(knapsack.capacity + knapsack.extraCapacity);
        prob.addConstraint(y[kIdx].ifNotThen(sum(items.size(), [&](auto i) {
                                               return items[i].weight *
                                                      x[i][kIdx];
                                             }) <= knapsack.capacity));
      }
    }
    break;
  case PiecewiseLinear:
    // Model the extra capacity using a piecewise linear function:
    // - Unconditionally add the extra capacity to knapsack k
    // - Create a new variable z[k] that is set to the weight in knapsack k
    // - Create a variable y[k] that is added to the objective with
    //   with the cost for extra capacity
    // - Create a piecewise linear
    //      y = f(z)
    //   where f is 0 on [0,capacity] and
    //   1 on [capacity+1,capacity+extraCapacity]
    {
      for (unsigned k = 0; k < knapsacks.size(); ++k)
        capacityRhs[k] = ConstantExpression(knapsacks[k].capacity +
                                            knapsacks[k].extraCapacity);
      std::vector<Variable> y =
          prob.addVariables(knapsacks.size())
              .withName("y_%d")
              .toArray();
      std::vector<Variable> z =
          prob.addVariables(knapsacks.size())
              .withName("z_%d")
              .toArray();
      profit = profit + sum(knapsacks.size(), [&](auto k) {
                 return -knapsacks[k].extraCost * y[k];
               });
      prob.addConstraints(knapsacks.size(), [&](auto k) {
        return sum(items.size(),
                   [&](auto i) { return items[i].weight * x[i][k]; }) == z[k];
      });
      prob.addConstraints(knapsacks.size(), [&](auto k) {
        return y[k].pwlOf(
            z[k], PwlBreakpoint(0, 0), PwlBreakpoint(knapsacks[k].capacity, 0),
            PwlBreakpoint(knapsacks[k].capacity + 1, 1),
            PwlBreakpoint(knapsacks[k].capacity + knapsacks[k].extraCapacity,
                          1));
      });
    }
    break;
  case SOSConstraint:
    // Model extra capacity using an SOS1 constraint:
    // - Create two binary variables y[k][0] and y[k][1] for each knapsack k.
    // - Set the right-hand side of the capacity constraint to
    //     k.capacity * y[k][0] +
    //     (k.capacity + k.extraCapacity) * y[k][1]
    // - Charge k.extraCost on y[k][1]
    // - Add an SOS1 constraint (y[k][0], y[k][1])
    {
      std::vector<std::vector<Variable>> y =
          prob.addVariables(knapsacks.size(), 2)
              .withType(ColumnType::Binary)
              .withName([](auto k, auto i) {
                return xpress::format("y(%d)(%d)", k, i);
              })
              .toArray();
      profit = profit + sum(knapsacks.size(), [&](auto k) {
                 return -knapsacks[k].extraCost * y[k][1];
               });
      for (unsigned k = 0; k < knapsacks.size(); ++k) {
        capacityRhs[k] =
            knapsacks[k].capacity * y[k][0] +
            (knapsacks[k].capacity + knapsacks[k].extraCapacity) * y[k][1];
        prob.addConstraint(
            SOS::sos1(y[k], nullptr, xpress::format("sos%d", k)));
      }
    }
    break;
  }
  // Set the objective function.
  // We want to maximize the profit, so we set the sense to
  // MAXIMIZE.
  prob.setObjective(profit, ObjSense::Maximize);

  // Create the capacity restriction.
  // There is one restriction for each knapsack.
  // The left-hand side is always the sum of the x variables for
  // this knapsack multiplied by the item weights.
  // The right hand side depends on the way how extra capacity
  // is modeled and contains the initial capacity plus any terms
  // that allow for extra capacity.
  prob.addConstraints(knapsacks.size(), [&](auto k) {
    return sum(items.size(), [&](auto i) {
             return items[i].weight * x[i][k];
           }) <= capacityRhs[k];
  });

  // Add a constraint that each item should be put in at most one knapsack
  prob.addConstraints(items.size(), [&](auto i) {
    return sum(knapsacks.size(),
               [&](auto k) { return x[i][k]; }) <= 1;
  });

  // Dump the problem to disk so that we can inspect it.
  prob.writeProb("Array.lp");

  // Finally solve, check the solution status and report.
  prob.optimize();
  if (prob.attributes.getSolStatus() != SolStatus::Optimal)
    throw std::runtime_error("no optimal solution found");
  std::cout << "Maximum profit: " << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (unsigned i = 0; i < items.size(); ++i) {
    int target = -1;
    for (unsigned k = 0; k < knapsacks.size(); ++k) {
      if (x[i][k].getValue(sol) > 0.5) {
        target = k;
        break;
      }
    }
    std::cout << "  item " << items[i].name << " "
              << (target < 0 ? "not picked"
                             : xpress::format("into %s",
                                              knapsacks[target].name.c_str()))
              << std::endl;
  }
  return 0;
}

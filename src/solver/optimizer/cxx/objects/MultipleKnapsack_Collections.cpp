// (c) 2024-2026 Fair Isaac Corporation

/**
 * A very simple multiple knapsack problem. The problem asks for assigning a set
 * of items to multiple knapsacks. The objective is to maximize profit while
 * respecting the per-knapsack weight restrictions. Each knapsack has a capacity
 * that limits the weight of items that can be put into it. The capacity can be
 * extended by a constant at a fixed cost.
 *
 * In this example, data for the knapsacks and items is given by collections and
 * variables are arranged in maps. They are indexed by knapsack and item
 * objects.
 */

#include <iostream>
#include <list>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::maps;
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
  bool operator==(Item const &other) const { return other.name == name; }
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
  bool operator==(Knapsack const &other) const { return other.name == name; }
};

// Specialize std::hash for Item and Knapsack so that we can use the
// classes as keys in hash maps.
namespace std {
template <> struct hash<Item> {
  size_t operator()(Item const &i) const { return hash<std::string>()(i.name); }
};
template <> struct hash<Knapsack> {
  size_t operator()(Knapsack const &k) const {
    return hash<std::string>()(k.name);
  }
};
} // namespace std

/** The items to assign in this multiple knapsack instance. */
std::list<Item> items{Item(2, 2, "item1"), Item(3, 3, "item2"),
                      Item(4, 4, "item3"), Item(5, 5, "item4"),
                      Item(6, 6, "item5"), Item(7, 7, "item6"),
                      Item(8, 8, "item7"), Item(9, 9, "item8")};

/** The knapsacks in this multiple knapsack instance. */
std::list<Knapsack> knapsacks{Knapsack(15, 5, 2, "knapsack1"),
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
 * as collections.
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
  // This creates a two-dimensional map of variables.
  // The first index is the item object, the second index is the
  // the knapsack object.
  // All variables are binary and the name for variable x[i][j]
  // is "i@j".
  HashMap2<Item, Knapsack, Variable> x =
      prob.addVariables(items, knapsacks)
          .withType(ColumnType::Binary)
          .withName([](Item const &i, Knapsack const &k) {
            return xpress::format("%s@%s", i.name.c_str(), k.name.c_str());
          })
          .toMap();

  // Create the expression for the objective function.
  // sum(i in items) sum(k in knapsacks) i.profit * x[i][k]
  Expression profit = sum(items, [&](Item const &i) {
    return sum(knapsacks,
               [&](Knapsack const &k) { return i.profit * x(i, k); });
  });

  // Right-hand side for capacity constraint.
  std::unordered_map<Knapsack, Expression> capacityRhs;
  switch (extendedCapacityModeling) {
  case NoExtraCapacity:
    // No extra capacity at all.
    for (Knapsack const &k : knapsacks)
      capacityRhs[k] = ConstantExpression(k.capacity);
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
      std::unordered_map<Knapsack, Variable> y =
          prob.addVariables(knapsacks)
              .withType(ColumnType::Binary)
              .withName([](Knapsack const &k) { return "y_" + k.name; })
              .toMap();
      profit = profit + sum(knapsacks, [&](Knapsack const &k) {
                 return -k.extraCost * y[k];
               });

      for (Knapsack const &k : knapsacks)
        capacityRhs[k] = k.capacity + k.extraCapacity * y[k];
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
      std::unordered_map<Knapsack, Variable> y =
          prob.addVariables(knapsacks)
              .withType(ColumnType::Binary)
              .withName([](Knapsack const &k) { return "y_" + k.name; })
              .toMap();
      profit = profit + sum(knapsacks, [&](Knapsack const &k) {
                 return -k.extraCost * y[k];
               });
      for (Knapsack const &k : knapsacks) {
        capacityRhs[k] = ConstantExpression(k.capacity + k.extraCapacity);
        prob.addConstraint(y[k].ifNotThen(sum(items, [&](Item const &i) {
                                            return i.weight * x(i, k);
                                          }) <= k.capacity));
      }
    }
    break;
  case PiecewiseLinear:
    // Model the extra capacity using a piecewise linear function:
    // - Unconditionally add the extra capacity to knapsack k
    // - Create a new variable z[k] that is set to the weight in knapsack k
    // - Create a variable y[k] that is added to the objective with the cost
    //   for extra capacity
    // - Create a piecewise linear
    //     y = f(z)
    //   where f is 0 on [0,capacity] and
    //   1 on [capacity+1,capacity+extraCapacity]
    {
      for (Knapsack const &k : knapsacks)
        capacityRhs[k] = ConstantExpression(k.capacity + k.extraCapacity);
      std::unordered_map<Knapsack, Variable> y =
          prob.addVariables(knapsacks)
              .withName([](Knapsack const &k) { return "y_" + k.name; })
              .toMap();
      std::unordered_map<Knapsack, Variable> z =
          prob.addVariables(knapsacks)
              .withName([](Knapsack const &k) { return "z_" + k.name; })
              .toMap();
      profit = profit + sum(knapsacks, [&](Knapsack const &k) {
                 return -k.extraCost * y[k];
               });
      prob.addConstraints(knapsacks, [&](Knapsack const &k) {
        return sum(items, [&](Item const &i) { return i.weight * x(i, k); }) ==
               z[k];
      });
      prob.addConstraints(knapsacks, [&](Knapsack const &k) {
        return y[k].pwlOf(z[k], PwlBreakpoint(0, 0),
                          PwlBreakpoint(k.capacity, 0),
                          PwlBreakpoint(k.capacity + 1, 1),
                          PwlBreakpoint(k.capacity + k.extraCapacity, 1));
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
      std::unordered_map<Knapsack, std::vector<Variable>> y;
      for (Knapsack const &k : knapsacks)
        y[k] = prob.addVariables(2)
                   .withType(ColumnType::Binary)
                   .withName([&](auto i) {
                     return xpress::format("y(%s)(%d)", k.name.c_str(), i);
                   })
                   .toArray();
      profit = profit + sum(knapsacks, [&](Knapsack const &k) {
                 return -k.extraCost + y[k][1];
               });
      for (Knapsack const &k : knapsacks) {
        capacityRhs[k] = k.capacity + y[k][0] + k.extraCapacity * y[k][1];
        prob.addConstraint(
            SOS::sos1(y[k], nullptr, xpress::format("sos%s", k.name.c_str())));
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
  prob.addConstraints(knapsacks, [&](Knapsack const &k) {
    return sum(items, [&](Item const &i) { return i.weight * x(i, k); }) <=
           capacityRhs[k];
  });

  // Add a constraint that each item should be put in at most one knapsack
  prob.addConstraints(items, [&](Item const &i) {
    return sum(knapsacks, [&](Knapsack const &k) { return x(i, k); }) <= 1.0;
  });

  // Dump the problem to disk so that we can inspect it.
  prob.writeProb("Collection.lp");

  // Finally solve, check the solution status and report.
  prob.optimize();
  if (prob.attributes.getSolStatus() != SolStatus::Optimal)
    throw std::runtime_error("no optimal solution found");
  std::cout << "Maximum profit: " << prob.attributes.getObjVal() << std::endl;
  auto sol = prob.getSolution();
  for (Item const &i : items) {
    std::optional<Knapsack> target = std::nullopt;
    for (Knapsack const &k : knapsacks) {
      if (x(i, k).getValue(sol) > 0.5) {
        target = k;
        break;
      }
    }
    std::cout << "  item " << i.name << " "
              << (target
                      ? xpress::format("into %s", target.value().name.c_str())
                      : "not picked")
              << std::endl;
  }
  return 0;
}

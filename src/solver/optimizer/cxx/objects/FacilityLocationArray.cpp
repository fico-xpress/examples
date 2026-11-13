// (c) 2024-2026 Fair Isaac Corporation

/**
 * This example demonstrates some modeling devices. We model a very simple
 * facility location problem: We have customers and facilities. The constraints
 * are: - each customer must be served from exactly one facility - customers can
 * only be served from open facilities - customer demand must be satisfied -
 * facility capacity must not be exceeded We minimize the sum of transport cost
 * (between customer and facility) and the cost for opening a facility. In this
 * example data is kept in arrays.
 */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/** Customer description. */
struct Customer {
  /** Name of customer. */
  std::string name;
  /** Demand for customer. */
  double demand;

  Customer(std::string const &name, double demand)
      : name(name), demand(demand) {}
};

/** Facility descriptor. */
struct Facility {
  /** Name of facility. */
  std::string name;
  /** Capacity of facility. */
  double capacity;
  /** Cost for opening this facility. */
  double cost;

  Facility(std::string const &name, double capacity, double cost)
      : name(name), capacity(capacity), cost(cost) {}
};

/** Customers in this example. */
std::vector<Customer> const customers{Customer{"Customer 1", 80},
                                      Customer{"Customer 2", 270},
                                      Customer{"Customer 3", 250}};
/** Facilities in this example. */
std::vector<Facility> const facilities{Facility{"Facility 1", 500, 1000},
                                       Facility{"Facility 2", 500, 1000},
                                       Facility{"Facility 3", 500, 1000}};
/** Cost for transporting one unit between customer and facility. */
std::vector<std::vector<double>> transportCost{std::vector<double>{4, 5, 6},
                                               std::vector<double>{6, 4, 3},
                                               std::vector<double>{9, 7, 4}};

int main() {
  XpressProblem prob;
  std::vector<Variable> y =
      prob.addVariables(facilities.size())
          .withType(ColumnType::Binary)
          .withName([&](auto f) { return facilities[f].name; })
          .toArray();
  std::vector<std::vector<Variable>> x =
      prob.addVariables(facilities.size(),
                        customers.size())
          .withName([&](auto f, auto c) {
            return xpress::format("x[%s,%s]", facilities[f].name.c_str(),
                                  customers[c].name.c_str());
          })
          .toArray();

  // for each customer c
  // sum(f=1..m) x[f,c] = d
  prob.addConstraints(customers.size(), [&](auto c) {
    return sum(facilities.size(), [&](auto f) { return x[f][c]; }) ==
           customers[c].demand;
  });

  // for each facility f
  // sum(c=1..n) x[f,c] <= capacity[j] * y[f]
  prob.addConstraints(facilities.size(), [&](auto f) {
    return sum(x[f]) <= facilities[f].capacity * y[f];
  });

  // minimize sum(f=1..m) cost[f] * y[f] +
  // sum(c=1..n) sum(f=1..m) cost[f,c] * x[f,c]
  prob.setObjective(sum(facilities.size(),
                        [&](auto f) { return facilities[f].cost * y[f]; }) +
                    sum(customers.size(), [&](auto c) {
                      return sum(facilities.size(),
                                 [&](auto f) {
                        return transportCost[f][c] * x[f][c];
                      });
                    }));

  prob.writeProb("facilitylocationarray.lp", "l");

  prob.optimize();
  if (prob.attributes.getSolStatus() != SolStatus::Optimal)
    throw std::runtime_error("failed to optimize with status " +
                             to_string(prob.attributes.getSolStatus()));
  auto sol = prob.getSolution();
  for (unsigned f = 0; f < facilities.size(); ++f) {
    if (y[f].getValue(sol) > 0.5) {
      std::cout << "Facility " << facilities[f].name << " is open, serves"
                << std::endl;
      for (unsigned c = 0; c < customers.size(); ++c) {
        if (x[f][c].getValue(sol) > 0.0)
          std::cout << "  " << customers[c].name << ": "
                    << x[f][c].getValue(sol) << std::endl;
      }
    }
  }
  return 0;
}

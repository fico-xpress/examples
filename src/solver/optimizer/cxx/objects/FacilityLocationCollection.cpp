// (c) 2024-2026 Fair Isaac Corporation

/**
 * This example demonstrates some modeling devices. We model a very simple
 * facility location problem: We have customers and facilities. The constraints
 * are: - each customer must be served from exactly one facility - customers can
 * only be served from open facilities - customer demand must be satisfied -
 * facility capacity must not be exceeded We minimize the sum of transport cost
 * (between customer and facility) and the cost for opening a facility. In this
 * example data is kept in collections.
 */

#include <iostream>
#include <list>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using namespace xpress::maps;
using xpress::objects::utils::sum;

/** Customer description. */
struct Customer {
  /** Name of customer. */
  std::string name;
  /** Demand for customer. */
  double demand;

  Customer(std::string const &name = "", double demand = 0.0)
      : name(name), demand(demand) {}

  bool operator==(Customer const &other) const { return name == other.name; }
};

/** Facility descriptor. */
struct Facility {
  /** Name of facility. */
  std::string name;
  /** Capacity of facility. */
  double capacity;
  /** Cost for opening this facility. */
  double cost;

  Facility(std::string const &name = "", double capacity = 0.0,
           double cost = 0.0)
      : name(name), capacity(capacity), cost(cost) {}

  bool operator==(Facility const &other) const { return name == other.name; }
};

// Specialize std::hash for Customer and Facility so that we can use the
// classes as keys in hash maps.
namespace std {
template <> struct hash<Customer> {
  size_t operator()(Customer const &c) const {
    return hash<std::string>()(c.name);
  }
};
template <> struct hash<Facility> {
  size_t operator()(Facility const &f) const {
    return hash<std::string>()(f.name);
  }
};
} // namespace std

/** Customers in this example. */
std::list<Customer> customers{Customer("Customer 1", 80),
                              Customer("Customer 2", 270),
                              Customer("Customer 3", 250)};
/** Facilities in this example. */
std::list<Facility> facilities{Facility("Facility 1", 500, 1000),
                               Facility("Facility 2", 500, 1000),
                               Facility("Facility 3", 500, 1000)};

/** Cost for transporting one unit between customer and facility. */
HashMap2<Facility, Customer, double> transportCost;
void initTransportCost() {
  std::vector<Customer> c(customers.begin(), customers.end());
  std::vector<Facility> f(facilities.begin(), facilities.end());
  transportCost.put(f[0], c[0], 4.0);
  transportCost.put(f[0], c[1], 5.0);
  transportCost.put(f[0], c[2], 6.0);
  transportCost.put(f[1], c[0], 6.0);
  transportCost.put(f[1], c[1], 4.0);
  transportCost.put(f[1], c[2], 3.0);
  transportCost.put(f[2], c[0], 9.0);
  transportCost.put(f[2], c[1], 7.0);
  transportCost.put(f[2], c[2], 4.0);
}

int main() {
  initTransportCost();
  XpressProblem prob;
  // Variables that indicate whether a facility is open or not.
  // This is a one-dimensional hash map that is indexed with Facility instances.
  // For indexing we can use operator[] if we know the key exists.
  std::unordered_map<Facility, Variable> y =
      prob.addVariables(facilities)
          .withType(ColumnType::Binary)
          .withName([](Facility f) { return f.name; })
          .toMap();
  // Variables that indicate how much of a customer is served from a facility.
  // This is a two-dimensional hash map that is indexed with (Facility,Customer)
  // pairs.
  // For indexing we can either use operator[](MapKey2<Facility,Customer>) or
  // the custom lookup operator()(Facility,Customer).
  // In this example we use the latter.
  HashMap2<Facility, Customer, Variable> x =
      prob.addVariables(facilities, customers)
          .withName([](auto f, auto c) {
            return xpress::format("x[%s,%s]", f.name.c_str(), c.name.c_str());
          })
          .toMap();

  // for each customer c
  // sum(f=1..m) x[f,c] = d
  prob.addConstraints(customers, [&](Customer const &c) {
    return sum(facilities, [&](Facility const &f) { return x(f, c); }) ==
           c.demand;
  });

  // for each facility f
  // sum(c=1..n) x[f,c] <= capacity[j] * y[f]
  prob.addConstraints(facilities, [&](Facility const &f) {
    return sum(customers, [&](Customer const &c) { return x(f, c); }) <=
           f.capacity * y[f];
  });

  // minimize sum(j=1..m) cost[j] * y[j] +
  // sum(i=1..n) sum(j=1..m) cost[f,c] * x[f,c]
  prob.setObjective(
      sum(facilities, [&](Facility const &f) { return f.cost * y[f]; }) +
      sum(customers, [&](Customer const &c) {
        return sum(facilities, [&](Facility const &f) {
          return transportCost(f, c) * x(f, c);
        });
      }));

  prob.writeProb("facilitylocationcollection.lp", "l");

  prob.optimize();
  if (prob.attributes.getSolStatus() != SolStatus::Optimal)
    throw std::runtime_error("failed to optimize with status " +
                             to_string(prob.attributes.getSolStatus()));
  auto sol = prob.getSolution();
  for (Facility const &f : facilities) {
    if (y[f].getValue(sol) > 0.5) {
      std::cout << "Facility " << f.name << " is open, serves" << std::endl;
      for (Customer const &c : customers) {
        if (x(f, c).getValue(sol) > 0.0)
          std::cout << "  " << c.name << ": " << x(f, c).getValue(sol)
                    << std::endl;
      }
    }
  }
  return 0;
}

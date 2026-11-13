// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::pow;

/*
 * QCQP problem (linear objective, convex quadratic constraints) Based on AMPL
 * model catenary.mod (Source:
 * http://www.orfe.princeton.edu/~rvdb/ampl/nlmodels/) This model finds the
 * shape of a hanging chain by minimizing its potential energy.
 */

const int N = 100;            // Number of chainlinks
const int L = 1;              // Difference in x-coordinates of endlinks
const double H = 2.0 * L / N; // Length of each link

int main() {
  try {
    XpressProblem prob;

    /* VARIABLES */

    // Continuous variables to denote x and y positions of each chainlink
    std::vector<Variable> x =
        prob.addVariables(N + 1)
            .withName("x(%d)")
            .withLB(XPRS_MINUSINFINITY) // Overwrite default lowerbound from 0
                                        // to minus infinity
            .toArray();

    std::vector<Variable> y =
        prob.addVariables(N + 1)
            .withName("y(%d)")
            .withLB(XPRS_MINUSINFINITY) // Overwrite default lowerbound from 0
                                        // to minus infinity
            .toArray();

    /* CONSTRAINTS */

    // Position of left endpoint / anchor
    x[0].fix(0);
    y[0].fix(0);
    // Position of right endpoint / anchor
    x[N].fix(L);
    y[N].fix(0);

    // Positions of chainlinks: forall(j in 1..N) (x(j)-x(j-1))^2 +
    // (y(j)-y(j-1))^2 <= H^2
    for (int j = 1; j <= N; j++) {
      Expression link_j = pow(x[j] - x[j - 1], 2.0) + pow(y[j] - y[j - 1], 2.0);
      prob.addConstraint(link_j <= H * H).setName(xpress::format("link_%d", j));
    }

    /* OBJECTIVE */

    // Minimise the potential energy: sum(j in 1..N) (y(j-1) + y(j)) / 2
    LinTermList obj;
    for (int i = 1; i <= N; i++) {
      obj.addTerm(0.5, y[i - 1])
          .addTerm(0.5, y[i]); // Add ( y[j-1] + y[j] ) / 2
    }
    obj.compress(); // Add coefficients of the same variable together: 0.5*y[1]
                    // + 0.5*y[1] ---> 1*y[1]
    prob.setObjective(obj, ObjSense::Minimize);

    /* SOLVE & PRINT */

    prob.writeProb("Catenary.lp");
    prob.optimize("");

    // Check the solution status
    if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
        prob.attributes.getSolStatus() != SolStatus::Feasible) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Optimization failed with status " + oss.str());
    }

    std::vector<double> sol = prob.getSolution();

    std::cout << "*** Solution ***" << std::endl;
    std::cout << "Objective value: " << prob.attributes.getObjVal()
              << std::endl;
    for (int i = 0; i <= N; i++) {
      std::cout << i << ": " << x[i].getValue(sol) << ", " << y[i].getValue(sol)
                << std::endl;
    }
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

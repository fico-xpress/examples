// (c) 2024-2026 Fair Isaac Corporation

#include <cstdlib>
#include <numeric>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/**
 * Example for solving a MIP using lazily separated cuts/constraints.
 *
 * We solve a random instance of the symmetric TSP using lazily separated
 * cuts/constraints.
 *
 * <p>
 * The model is based on a directed graph G = (V,E).
 * We have one binary variable x[e] for each edge e in E. That variable
 * is set to 1 if edge e is selected in the optimal tour and 0 otherwise.
 * </p>
 * <p>
 * The model contains only two explicit constraints:
 * <pre>
  for each v in V: sum(u in V : u != v) x[uv] == 1
  for each v in V: sum(u in V : u != v) x[vu] == 1
 </pre>
 * These state that node u should have exactly one outgoing and exactly
 * one incoming edge in a tour.
 * </p>
 * <p>
 * The above constraints ensures that the selected edges form tours. However,
 * it allows multiple tours, also known as subtours. So we need a constraint
 * that requires that there is only one tour (which then necessarily hits
 * all the nodes). This constraint is known as a subtour elimination constraint
 * and is
 * <pre>
    sum(e in S) x[e] <= |S|-1  for each subtour S
 * </pre>
 *
 * Since there are exponentially many subtours in a graph, this constraint is
 * not stated explicitly. Instead we check for any solution that the optimizer
 * finds, whether it satisfies the subtour elimination constraints. If it does
 * then we accept the solution. Otherwise we reject the solution and augment the
 * model by the violated subtours eliminiation constraint.
 * </p>
 * <p>
 * This lazy addition of constraints is implemented using a preintsol callback
 * that rejects any solution that violates a subtour elimination constraint and
 * injects a violated subtour elimination constraint in case the solution
 * candidate came from an integral node.
 * </p>
 * <p>
 * An important thing to note about this strategy is that dual reductions have
 * to be disabled. Since the optimizer does not see the whole model (subtour
 * elimination constraints are only generated on the fly), dual reductions may
 * cut off the optimal solution.
 * </p>
 */
class TravelingSalesPerson {
  /** Number of nodes in the instance. */
  unsigned const nodes;
  /** X coordinate of nodes. */
  std::vector<double> nodeX;
  /** Y coordinate of nodes. */
  std::vector<double> nodeY;
  /** Variables the edges. */
  std::vector<std::vector<Variable>> x;

public:
  /**
   * Construct a new random instance.
   *
   * @param nodes The number of nodes in the instance.
   * @param seed  Random number seed.
   */
  TravelingSalesPerson(unsigned nodes, unsigned seed = 0)
      : nodes(nodes), nodeX(nodes), nodeY(nodes) {
    // Fill coordinates with random values.
    std::srand(seed);
    auto randCoord = []() {
      return 4.0 * static_cast<double>(rand()) / RAND_MAX;
    };
    std::generate(nodeX.begin(), nodeX.end(), randCoord);
    std::generate(nodeY.begin(), nodeY.end(), randCoord);
  }

  /**
   * Get the distance between two nodes.
   *
   * @param u First node.
   * @param v Second node.
   * @return The distance between <code>u</code> and <code>v</code>. The
   * distance is symmetric.
   */
  double distance(int u, int v) const {
    return sqrt((nodeX[u] - nodeX[v]) * (nodeX[u] - nodeX[v]) +
                (nodeY[u] - nodeY[v]) * (nodeY[u] - nodeY[v]));
  }

private:
  /**
   * Find the tour rooted at 0 in a solution. As a side effect, the tour is
   * printed to the console.
   *
   * @param sol  The current solution.
   * @param from Stores the tour. <code>from[u]</code> yields the predecessor of
   *             <code>u</code> in the tour. If <code>from[u]</code> is negative
   *             then <code>u</code> is not in the tour. This parameter can be
   *             <code>null</code>.
   * @return The length of the tour.
   */
  unsigned findTour(std::vector<double> const &sol, std::vector<int> *from) {
    std::unique_ptr<std::vector<int>> tmpFrom(nullptr);
    if (!from) {
      tmpFrom = std::make_unique<std::vector<int>>();
      from = tmpFrom.get();
    }
    from->assign(nodes, -1);

    unsigned node = 0;
    unsigned used = 0;
    std::cout << "0";
    while (node != 0 || used == 0) {
      // Find the edge leaving node
      Variable edge = XpressProblem::NULL_VARIABLE;
      for (unsigned i = 0; i < nodes; ++i) {
        if (i != node && x[node][i].getValue(sol) > 0.5) {
          std::cout << " -> " << i;
          edge = x[node][i];
          (*from)[i] = node;
          node = i;
          ++used;
          break;
        }
      }
      if (!edge)
        break;
    }

    std::cout << std::endl;

    return used;
  }

  /**
   * Integer solution check callback.
   */
  void preIntsol(XpressProblem &prob, int soltype, int *p_reject,
                 double * /*p_cutoff*/) {
    std::cout << "Checking candidate solution ..." << std::endl;

    // Get current solution and check whether it is feasible
    std::vector<double> sol(nodes * nodes);
    prob.getLpSol(sol, nullptr, nullptr, nullptr);
    std::vector<int> from(nodes);
    unsigned used = findTour(sol, &from);
    std::cout << "Solution is ";
    if (used < nodes) {
      // The tour given by the current solution does not pass through
      // all the nodes and is thus infeasible.
      // If soltype is non-zero then we reject by setting
      // p_reject.value=1.
      // If instead soltype is zero then the solution came from an
      // integral node. In this case we can reject by adding a cut
      // that cuts off that solution. Note that we must NOT reject
      // in that case because that would result in just dropping
      // the node, no matter whether we add cuts or not.
      std::cout << "infeasible (" << used << " edges)" << std::endl;
      if (soltype != 0) {
        *p_reject = 1;
      } else {
        // The tour is too short. Get the edges on the tour and
        // add a subtour elimination constraint
        LinExpression subtour = LinExpression::create();
        for (unsigned u = 0; u < nodes; ++u) {
          if (from[u] >= 0)
            subtour.addTerm(x[from[u]][u]);
        }
        // We add the constraint. The solver must translate the
        // constraint from the original space into the presolved
        // space. This may fail (in theory). In that case the
        // addCut() function will return non-zero.
        if (prob.addCut(1, subtour.leq(used - 1)) != 0)
          throw std::runtime_error(
              "failed to presolve subtour elimination constraint");
      }
    } else {
      std::cout << "feasible" << std::endl;
    }
  }

  /** Create a feasible tour and add this as initial MIP solution. */
  void createInitialTour(XpressProblem &prob) {
    std::vector<Variable> variable(nodes);
    std::vector<double> value(nodes);
    // Create a tour that just visits each node in order.
    for (unsigned i = 0; i < nodes; ++i) {
      variable[i] = x[i][(i + 1) % nodes];
      value[i] = 1.0;
    }
    prob.addMipSol(value, variable, "init");
  }

public:
  /**
   * Solve the TSP represented by this instance.
   */
  void solve() {
    XpressProblem prob;
    // Create variables. We create one variable for each edge in
    // the (complete) graph. That is, we create variables from each
    // node u to all other nodes v. We even create a variable for
    // the self-loop from u to u, but that variable is fixed to 0.
    // x[u][v] gives the variable that represents edge uv.
    // All variables are binary.
    x = prob.addVariables(nodes, nodes)
            .withType(ColumnType::Binary)
            .withName([](auto i, auto j) { return format("x_%d_%d", i, j); })
            .withUB([](auto i, auto j) { return (i == j) ? 0.0 : 1.0; })
            .toArray();

    // Objective. All variables are in the objective and their
    // respective coefficient is the distance between the two nodes.
    prob.setObjective(sum(nodes,
                          [&](auto u) {
                            return sum(nodes, [&](auto v) {
                              return x[u][v] * distance(u, v);
                            });
                          }),
                      ObjSense::Minimize);

    // Create a vector with indices 0, ..., nodes-1
    std::vector<unsigned> indices(nodes);
    std::iota(indices.begin(), indices.end(), 0);

    // Constraint: In the graph that is induced by the selected
    //             edges, each node should have exactly one outgoing
    //             and exactly one incoming edge.
    //             These are the only constraints we add explicitly.
    //             Subtour elimination constraints are added
    //             dynamically via a callback.
    prob.addConstraints(nodes, [&](auto u) {
      return sum(indices,
                 [&](auto v) { return (u == v ? 0.0 : 1.0) * x[u][v]; }) == 1.0;
    });
    prob.addConstraints(nodes, [&](auto u) {
      return sum(indices,
                 [&](auto v) { return (u == v ? 0.0 : 1.0) * x[v][u]; }) == 1.0;
    });

    // Create a starting solution.
    // This is optional but having a feasible solution available right
    // from the beginning can improve optimizer performance.
    createInitialTour(prob);

    // Write out the model in case we want to look at it.
    prob.writeProb("travelingsalesperson.lp", "l");

    // We don't have all constraints explicitly in the matrix, hence
    // we must disable dual reductions. Otherwise MIP presolve may
    // cut off the optimal solution.
    prob.controls.setMipDualReductions(0);

    // Add a callback that rejects solutions that do not satisfy
    // the subtour constraints.
    prob.callbacks.addPreIntsolCallback(std::bind(
        &TravelingSalesPerson::preIntsol, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    // Add a message listener to display log information.
    prob.callbacks.addMessageCallback(XpressProblem::console);

    prob.optimize();
    if (prob.attributes.getSolStatus() != SolStatus::Optimal)
      throw std::runtime_error("failed to solve");

    auto sol = prob.getSolution();

    // Print the optimal tour.
    std::cout << "Tour with length " << prob.attributes.getMipObjVal()
              << std::endl;
    findTour(sol, nullptr);

    x.clear(); // We are done with the variables
  }
};

int main() {
  try {
    TravelingSalesPerson(10).solve();
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

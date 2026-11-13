/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file txp.cc
   ```````````
   Solve a MIP using cuts/constraints that are lazily separated.

   We take a random instance of the symmetric TSP and solve that using
   lazily separated constraints.

   (c) 2024-2026 Fair Isaac Corporation
***********************************************************************/

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <xpress.hpp>

using xpress::XPRSProblem;

/** Example for solving a MIP using lazily separated cuts/constraints.
 *
 * We solve a random instance of the symmetric TSP using lazily separated
 * cuts/constraints.
 *
 * <p>
 * The model is based on a graph G = (V,E).
 * We have one binary variable x[e] for each edge e in E. That variable
 * is set to 1 if edge e is selected in the optimal tour and 0 otherwise.
 * </p>
 * <p>
 * The model contains only one explicit set of constraints:
 * <pre>
  for each v in V: sum(u in V : u != v) x[uv] == 2
 </pre>
 * This states that from all the edges incident to a node u, exactly two
 * must be selected in the optimal tour.
 * </p>
 * <p>
 * The above constraints ensures that the selected edges form tours. However,
 * it allows multiple tours, also known as subtours. So we need a constraint
 * that requires that there is only one tour (which then necessarily hits
 * all the nodes). This constraint is known as subtour elimination constraint
 * and is
 * <pre>
    sum(e in S) x[e] <= |S|-1  for each subtour S
 </pre>
 * Since there are exponentially many subtours in a graph, this constraint
 * is not stated explicitly. Instead we check for any solution that the
 * optimizer finds, whether it satisfies the subtour elimination constraints.
 * If it does then we accept the solution. Otherwise we reject the solution
 * and augment the model by the violated subtour eliminiation constraints.
 * </p>
 * <p>
 * This lazy addition of constraints is implemented using
 * a preintsol callback that rejects any solution that violates a
 * subtour elimination constraint and injects a violated subtour elimination
 * constraint in case the solution candidate came from an integral node.
 * </p>
 * <p>
 * An important thing to note about this strategy is that dual reductions
 * have to be disabled. Since the optimizer does not see the whole model
 * (subtour elimination constraints are only generated on the fly), dual
 * reductions may cut off the optimal solution.
 * </p>
 */
class TSP {
public:
  /** Number of nodes in the instance. */
  unsigned const nodes;
  /** Number of edges in the instance. */
  unsigned const edges;
  /** X coordinate of nodes. */
  std::vector<double> nodeX;
  /** Y coordinate of nodes. */
  std::vector<double> nodeY;
  /** Variable indices for the edges. */
  std::vector<std::vector<int>> x;

public:
  /** Construct a new random instance.
   * @param nodes The number of nodes in the instance.
   * @param seed  Random number seed.
   */
  TSP(unsigned n, int seed = 0)
      : nodes(n), edges((n * (n - 1)) / 2), nodeX(n), nodeY(n),
        x(n, std::vector<int>(n)) {
    // Fill coordinates with random values.
    std::srand(seed);
    auto randCoord = []() {
      return 4.0 * static_cast<double>(rand()) / RAND_MAX;
    };
    std::generate(nodeX.begin(), nodeX.end(), randCoord);
    std::generate(nodeY.begin(), nodeY.end(), randCoord);
  }

  /** Get the distance between two nodes.
   * @param u First node.
   * @param v Second node.
   * @return The distance between <code>u</code> and <code>v</code>.
   *         The distance is symmetric.
   */
  double distance(unsigned u, unsigned v) const {
    return sqrt((nodeX[u] - nodeX[v]) * (nodeX[u] - nodeX[v]) +
                (nodeY[u] - nodeY[v]) * (nodeY[u] - nodeY[v]));
  }

  /** Find the tour rooted at 0 in a solution.
   * As a side effect, the tour is printed to the console.
   * @param sol  The current solution.
   * @param from Stores the tour. <code>from[u]</code> yields the
   *             predecessor of <code>u</code> in the tour. If
   *             <code>from[u]</code> is negative then <code>u</code>
   *             is not in the tour.
   *             This parameter can be <code>null</code>.
   * @return The length of the tour.
   */
  unsigned findTour(std::vector<double> const &sol,
                    std::vector<int> *from) const {
    std::unique_ptr<std::vector<int>> allocatedFrom;
    if (!from) {
      allocatedFrom.reset(new std::vector<int>(nodes));
      from = allocatedFrom.get();
    }
    from->assign(nodes, -1);
    std::vector<bool> inTour(edges); // Marks edges on the subtour

    unsigned u = 0;
    unsigned used = 0;
    std::cout << "0";
    do {
      for (unsigned v = 0; v < nodes; ++v) {
        if (u == v) // no self-loops
          continue;
        else if ((*from)[v] != -1) // node already on tour
          continue;
        else if (sol[x[u][v]] < 0.5) // edge not selected in solution
          continue;
        else if (inTour[x[u][v]]) // edge already on tour
          continue;
        else {
          std::cout << " -> " << v;
          inTour[x[u][v]] = true;
          (*from)[v] = u;
          used += 1;
          u = v;
          break;
        }
      }
    } while (u != 0);
    std::cout << std::endl;

    return used;
  }

  /** Integer solution check callback.
   */
  class PreIntsolCallback {
    TSP const &data;

  public:
    PreIntsolCallback(TSP const &d) : data(d) {}
    void operator()(XPRSProblem &prob, int soltype, int *p_reject, double *) {
      std::cout << "Checking candidate solution ..." << std::endl;

      // Get current solution and check whether it is feasible
      std::vector<double> sol(data.edges);
      prob.getLpSol(sol, nullptr, nullptr, nullptr);
      std::vector<int> from(data.nodes);
      unsigned used = data.findTour(sol, &from);
      std::cout << "Solution is " << std::flush;
      if (used < data.nodes) {
        // The tour given by the current solution does not pass through
        // all the nodes and is thus infeasible.
        // If soltype is non-zero then we reject.
        // If instead soltype is zero then the solution came from an
        // integral node. In this case we can reject by adding a cut
        // that cuts off that solution. Note that we must NOT return
        // null in that case because that would result in just dropping
        // the node, no matter whether we add cuts or not.
        std::cout << "infeasible (" << used << " edges)" << std::endl;
        if (soltype != 0) {
          *p_reject = 1;
        } else {
          // The tour is too short. Get the edges on the tour and
          // add a subtour elimination constraint
          std::vector<int> ind(used);
          for (unsigned u = 0, next = 0; u < data.nodes; ++u) {
            if (from[u] >= 0)
              ind[next++] = data.x[u][from[u]];
          }
          std::vector<double> val(used, 1.0);

          // Since we created the constraint in the original space,
          // we must crush it to the presolved space before we can
          // add it.
          int status = -1;
          double prerhs = 0.0;
          int maxcoefs = prob.attributes.getCols();
          int coefs = -1;
          std::vector<int> preind(maxcoefs);
          std::vector<double> preval(maxcoefs);
          prob.presolveRow('L', used, ind, val, used - 1, maxcoefs, &coefs,
                           preind, preval, &prerhs, &status);
          if (status == 0)
            prob.addCuts(1, std::vector<int>{0}, "L", &prerhs,
                         std::vector<int>{0, coefs}, preind, preval);
          else
            throw std::runtime_error(
                "failed to presolve subtour elimination constraint");
        }
      } else {
        std::cout << "feasible" << std::endl;
        // We accept the solution, so nothing to do
      }
    }
  };

  /** Create a feasible tour and add this as initial MIP solution. */
  void createInitialTour(XPRSProblem &prob) {
    std::vector<int> ind(nodes);
    std::vector<double> val(nodes);
    // Create a tour that just visits each node in order.
    for (unsigned i = 0; i < nodes; ++i) {
      ind[i] = x[i][(i + 1) % nodes];
      val[i] = 1.0;
    }
    prob.addMipSol(nodes, val, ind, "initial");
  }

  /** Solve the TSP represented by this instance.
   */
  void solve() {
    XPRSProblem prob;
    // Create variables. We create one variable for each edge in
    // the (complete) graph. x[u][v] gives the index of the variable
    // that represents edge uv. Since edges are undirected, x[v][u]
    // gives the same variable.
    // All variables are binary.
    for (unsigned i = 0; i < nodes; ++i) {
      for (unsigned j = i + 1; j < nodes; ++j) {
        x[j][i] = x[i][j] = prob.attributes.getCols();
        prob.addCols(1, 0, nullptr, nullptr, nullptr, nullptr, nullptr,
                     nullptr);
        prob.chgColType(1, &x[i][j], "B");
      }
    }

    // Objective. All variables are in the objective and their
    // respective coefficient is the distance between the two nodes.
    std::vector<int> objInd(edges);
    std::vector<double> objVal(edges);
    for (unsigned i = 0, nz = 0; i < nodes; ++i) {
      for (unsigned j = i + 1; j < nodes; ++j) {
        objInd[nz] = x[i][j];
        objVal[nz] = distance(i, j);
        ++nz;
      }
    }
    prob.chgObj(edges, objInd, objVal);

    // Constraint: In the graph that is induced by the selected
    //             edges, each node should have degree 2.
    //             This is the only constraint we add explicitly.
    //             Subtour elimination constraints are added
    //             dynamically via a callback.
    for (unsigned u = 0; u < nodes; ++u) {
      std::vector<int> ind(nodes - 1);

      for (unsigned v = 0, next = 0; v < nodes; ++v) {
        if (u != v)
          ind[next++] = x[u][v];
      }
      std::vector<double> val(nodes - 1, 1.0);
      prob.addRows(1, nodes - 1, "E", std::vector<double>({2}), nullptr,
                   std::vector<int>({0}), ind, val);
    }

    // Create a starting solution.
    // This is optional but having a feasible solution available right
    // from the beginning can improve optimizer performance.
    createInitialTour(prob);

    // Write out the model in case we want to look at it.
    prob.writeProb("tsp.lp", "l");

    // We don't have all constraints explicitly in the matrix, hence
    // we must disable dual reductions. Otherwise MIP presolve may
    // cut off the optimal solution.
    prob.controls.setMipDualReductions(0);

    // Add a callback that rejects solutions that do not satisfy
    // the subtour constraints.
    PreIntsolCallback cb(*this);
    prob.addPreIntsolCallback(cb);

    // Add a message listener to display log information.
    prob.addMessageCallback(XPRSProblem::console);

    prob.optimize("", nullptr, nullptr);

    auto sol = prob.getSolution();

    // Print the optimal tour.
    std::cout << "Tour with length " << prob.attributes.getObjVal()
              << std::endl;
    findTour(sol, nullptr);

  }
};

int main(void) {
  try {
    TSP(10).solve();
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}

/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file TSP.java
   ````````````
   Solve a MIP using cuts/constraints that are lazily separated.

   We take a random instance of the symmetric TSP and solve that using
   lazily separated constraints.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.XPRSprob;
import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.AbstractPreIntsolListener;

import static com.dashoptimization.XPRSenumerations.ObjSense;
import static com.dashoptimization.XPRSprob.RowInfo;

import java.util.Arrays;
import java.util.Random;

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
 * The model contains only one explicit constraint:
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
 * optimizer finds, whether it satisfies the subtour elimination constraint.
 * If it does then we accept the solution. Otherwise we reject the solution
 * and augment the model by the violated subtour eliminiation constraint.
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
public final class TSP {
    /** Number of nodes in the instance. */
    private final int nodes;
    /** Number of edges in the instance. */
    private final int edges;
    /** X coordinate of nodes. */
    private final double[] nodeX;
    /** Y coordinate of nodes. */
    private final double[] nodeY;
    /** Variable indices for the edges. */
    private final int[][] x;

    /** Construct a new random instance with random seed 0.
     * @param nodes The number of nodes in the instance.
     */
    public TSP(int nodes) { this(nodes, 0); }
    /** Construct a new random instance.
     * @param nodes The number of nodes in the instance.
     * @param seed  Random number seed.
     */
    public TSP(int nodes, int seed) {
        this.nodes = nodes;
        edges = (nodes * (nodes - 1)) / 2;
        nodeX = new double[nodes];
        nodeY = new double[nodes];
        Random rand = new Random(seed);
        for (int i = 0; i < nodes; ++i) {
            nodeX[i] = 4.0 * rand.nextDouble();
            nodeY[i] = 4.0 * rand.nextDouble();
        }
        x = new int[nodes][];
        for (int i = 0; i < nodes; ++i)
            x[i] = new int[nodes];
    }

    /** Get the distance between two nodes.
     * @param u First node.
     * @param v Second node.
     * @return The distance between <code>u</code> and <code>v</code>.
     *         The distance is symmetric.
     */
    public double distance(int u, int v) {
        return Math.sqrt((nodeX[u] - nodeX[v]) * (nodeX[u] - nodeX[v]) +
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
    private int findTour(double[] sol, int[] from) {
        if (from == null)
            from = new int[nodes];
        Arrays.fill(from, -1);
        boolean[] inTour = new boolean[edges]; // Marks edges on the subtour

        int u = 0;
        int used = 0;
        System.out.print("0");
        do {
            for (int v = 0; v < nodes; ++v) {
                if (u == v)                    // no self-loops
                    continue;
                else if (from[v] != -1)        // node already on tour
                    continue;
                else if (sol[x[u][v]] < 0.5)   // edge not selected in solution
                    continue;
                else if (inTour[x[u][v]])      // edge already on tour
                    continue;
                else {
                    System.out.print(" -> " + v);
                    inTour[x[u][v]] = true;
                    from[v] = u;
                    used += 1;
                    u = v;
                    break;
                }
            }
        } while (u != 0);
        System.out.println();

        return used;
    }

    /** Integer solution check callback.
     */
    private final class PreIntsolListener extends AbstractPreIntsolListener {
        @Override
        public Double preIntsolEvent(XPRSprob prob, int soltype, double cutoff) {
            System.out.println("Checking candidate solution ...");

            // Get current solution and check whether it is feasible
            double[] sol = prob.getCallbackSolution();
            int[] from = new int[nodes];
            int used = findTour(sol, from);
            System.out.print("Solution is ");
            if (used < nodes) {
                // The tour given by the current solution does not pass through
                // all the nodes and is thus infeasible.
                // If soltype is non-zero then we reject by returning null.
                // If instead soltype is zero then the solution came from an
                // integral node. In this case we can reject by adding a cut
                // that cuts off that solution. Note that we must NOT return
                // null in that case because that would result in just dropping
                // the node, no matter whether we add cuts or not.
                System.out.println("infeasible (" + used + " edges)");
                if (soltype != 0) {
                    return null;
                }
                else {
                    // The tour is too short. Get the edges on the tour and
                    // add a subtour elimination constraint
                    int[] ind = new int[used];
                    double[] val = new double[used];
                    for (int u = 0, next = 0; u < nodes; ++u) {
                        if (from[u] >= 0)
                            ind[next++] = x[u][from[u]];
                    }
                    Arrays.fill(val, 1);
                    // Since we created the constraint in the original space,
                    // we must crush it to the presolved space before we can
                    // add it.
                    RowInfo r = prob.presolveRow(ind, val, 'L', used - 1);
                    if (r != null)
                        prob.addCut(1, r);
                    else
                        throw new RuntimeException("failed to presolve subtour elimination constraint");
                }
                return cutoff; // Do not return null as that would drop the node
            }
            else {
                System.out.println("feasible");
                return cutoff;
            }
        }
    }

    /** Create a feasible tour and add this as initial MIP solution. */
    private void createInitialTour(XPRSprob prob) {
        int[] ind = new int[nodes];
        double[] val = new double[nodes];
        // Create a tour that just visits each node in order.
        for (int i = 0; i < nodes; ++i) {
            ind[i] = x[i][(i + 1) % nodes];
            val[i] = 1.0;
        }
        prob.addMipSol(val, ind);
    }

    /** Solve the TSP represented by this instance.
     */
    public void solve() {
        try (XPRSprob prob = new XPRSprob(null)) {
            // Create variables. We create one variable for each edge in
            // the (complete) graph. x[u][v] gives the index of the variable
            // that represents edge uv. Since edges are undirected, x[v][u]
            // gives the same variable.
            // All variables are binary.
            for (int i = 0; i < nodes; ++i) {
                for (int j = i + 1; j < nodes; ++j) {
                    x[j][i] = x[i][j] = prob.addColumn(0, 1, ColumnType.Binary, "x_" + i + "_" + j);
                }
            }

            // Objective. All variables are in the objective and their
            // respective coefficient is the distance between the two nodes.
            int[] objInd = new int[edges];
            double[] objVal = new double[edges];
            for (int i = 0, nz = 0; i < nodes; ++i) {
                for (int j = i + 1; j < nodes; ++j) {
                    objInd[nz] = x[i][j];
                    objVal[nz] = distance(i, j);
                    ++nz;
                }
            }
            prob.setObjective(objInd, objVal, ObjSense.MINIMIZE);

            // Constraint: In the graph that is induced by the selected
            //             edges, each node should have degree 2.
            //             This is the only constraint we add explicitly.
            //             Subtour elimination constraints are added
            //             dynamically via a callback.
            for (int u = 0; u < nodes; ++u) {
                int[] ind = new int[nodes - 1];
                double[] val = new double[nodes - 1];
                for (int v = 0, next = 0; v < nodes; ++v) {
                    if (u != v)
                        ind[next++] = x[u][v];
                }
                Arrays.fill(val, 1.0);
                prob.addRow(ind, val, 'E', 2);
            }
            /* Note: with Java 1.8 this could also be written differently:
            IntStream.range(0, nodes).forEach(
                  u -> prob.addRow(IntStream.range(0, nodes).filter(v -> v != u).map(v -> x[u][v]).toArray(),
                                   DoubleStream.generate(() -> 1).limit(nodes - 1).toArray(),
                                   'E', 2));
            */

            // Create a starting solution.
            // This is optional but having a feasible solution available right
            // from the beginning can improve optimizer performance.
            createInitialTour(prob);

            // Write out the model in case we want to look at it.
            prob.writeProb("tsp.lp", "l");

            // We don't have all constraints explicitly in the matrix, hence
            // we must disable dual reductions. Otherwise MIP presolve may
            // cut off the optimal solution.
            prob.controls().setMIPDualReductions(0);

            // Add a callback that rejects solutions that do not satisfy
            // the subtour constraints.
            prob.addPreIntsolListener(new PreIntsolListener());

            // Add a message listener to display log information.
            prob.addMessageListener(DefaultMessageListener::console);

            prob.optimize();

            double[] sol = prob.getSolution();

            // Print the optimal tour.
            System.out.println("Tour with length " + prob.attributes().getMIPObjVal());
            findTour(sol, null);

        }
    }

    public static void main(String[] args) {
        new TSP(10).solve();
    }
}

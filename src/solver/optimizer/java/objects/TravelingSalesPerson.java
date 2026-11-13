// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import java.util.Arrays;
import java.util.Random;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.DoubleHolder;
import com.dashoptimization.IntHolder;
import com.dashoptimization.XPRSenumerations.ObjSense;
import com.dashoptimization.XPRSenumerations.SolStatus;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

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
 * finds, whether it satisfies the subtour elimination constraint. If it does
 * then we accept the solution. Otherwise we reject the solution and augment the
 * model by the violated subtour eliminiation constraint.
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
public final class TravelingSalesPerson {
    /** Number of nodes in the instance. */
    private final int nodes;
    /** X coordinate of nodes. */
    private final double[] nodeX;
    /** Y coordinate of nodes. */
    private final double[] nodeY;
    /** Variables the edges. */
    private Variable[][] x;

    /**
     * Construct a new random instance with random seed 0.
     *
     * @param nodes The number of nodes in the instance.
     */
    public TravelingSalesPerson(int nodes) {
        this(nodes, 0);
    }

    /**
     * Construct a new random instance.
     *
     * @param nodes The number of nodes in the instance.
     * @param seed  Random number seed.
     */
    public TravelingSalesPerson(int nodes, int seed) {
        this.nodes = nodes;
        nodeX = new double[nodes];
        nodeY = new double[nodes];
        Random rand = new Random(seed);
        for (int i = 0; i < nodes; ++i) {
            nodeX[i] = 4.0 * rand.nextDouble();
            nodeY[i] = 4.0 * rand.nextDouble();
        }
    }

    /**
     * Get the distance between two nodes.
     *
     * @param u First node.
     * @param v Second node.
     * @return The distance between <code>u</code> and <code>v</code>. The distance
     *         is symmetric.
     */
    public double distance(int u, int v) {
        return Math.sqrt((nodeX[u] - nodeX[v]) * (nodeX[u] - nodeX[v]) + (nodeY[u] - nodeY[v]) * (nodeY[u] - nodeY[v]));
    }

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
    private int findTour(double[] sol, int[] from) {
        if (from == null)
            from = new int[nodes];
        Arrays.fill(from, -1);

        int node = 0;
        int used = 0;
        System.out.print("0");
        while (node != 0 || used == 0) {
            // Find the edge leaving node
            Variable edge = null;
            for (int i = 0; i < nodes; ++i) {
                if (i != node && x[node][i].getValue(sol) > 0.5) {
                    System.out.printf(" -> %d", i);
                    edge = x[node][i];
                    from[i] = node;
                    node = i;
                    ++used;
                    break;
                }
            }
            if (edge == null)
                break;
        }

        System.out.println();

        return used;
    }

    /**
     * Integer solution check callback.
     */
    private final class PreIntsolCallback implements XpressProblem.CallbackAPI.PreIntsolCallback {
        @Override
        public void preIntsol(XpressProblem prob, int soltype, IntHolder p_reject, DoubleHolder p_cutoff) {
            System.out.println("Checking candidate solution ...");

            // Get current solution and check whether it is feasible
            double[] sol = prob.getCallbackSolution();
            int[] from = new int[nodes];
            int used = findTour(sol, from);
            System.out.print("Solution is ");
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
                System.out.println("infeasible (" + used + " edges)");
                if (soltype != 0) {
                    p_reject.value = 1;
                } else {
                    // The tour is too short. Get the edges on the tour and
                    // add a subtour elimination constraint
                    LinExpression subtour = LinExpression.create();
                    for (int u = 0; u < nodes; ++u) {
                        if (from[u] >= 0)
                            subtour.addTerm(x[from[u]][u]);
                    }
                    // We add the constraint. The solver must translate the
                    // constraint from the original space into the presolved
                    // space. This may fail (in theory). In that case the
                    // addCut() function will return non-zero.
                    if (prob.addCut(1, subtour.leq(used - 1)) != 0)
                        throw new RuntimeException("failed to presolve subtour elimination constraint");
                }
            } else {
                System.out.println("feasible");
            }
        }
    }

    /** Create a feasible tour and add this as initial MIP solution. */
    private void createInitialTour(XpressProblem prob) {
        Variable[] variable = new Variable[nodes];
        double[] value = new double[nodes];
        // Create a tour that just visits each node in order.
        for (int i = 0; i < nodes; ++i) {
            variable[i] = x[i][(i + 1) % nodes];
            value[i] = 1.0;
        }
        prob.addMipSol(value, variable, "init");
    }

    /**
     * Solve the TSP represented by this instance.
     */
    public void solve() {
        try (XpressProblem prob = new XpressProblem(null)) {
            // Create variables. We create one variable for each edge in
            // the (complete) graph. That is, we create variables from each
            // node u to all other nodes v. We even create a variable for
            // the self-loop from u to u, but that variable is fixed to 0.
            // x[u][v] gives the variable that represents edge uv.
            // All variables are binary.
            x = prob.addVariables(nodes, nodes)
                .withType(ColumnType.Binary)
                .withName((i,j) -> String.format("x_%d_%d", i, j))
                .withUB((i,j) -> (i == j) ? 0.0 : 1.0)
                .toArray();

            // Objective. All variables are in the objective and their
            // respective coefficient is the distance between the two nodes.
            prob.setObjective(sum(nodes,
                                  u -> sum(nodes,
                                           v -> x[u][v].mul(distance(u, v)))),
                              ObjSense.MINIMIZE);

            // Constraint: In the graph that is induced by the selected
            //             edges, each node should have exactly one outgoing
            //             and exactly one incoming edge.
            //             These are the only constraints we add explicitly.
            //             Subtour elimination constraints are added
            //             dynamically via a callback.
            prob.addConstraints(nodes,
                                u -> sum(range(0, nodes)
                                         .filter(v -> v != u)
                                         .mapToObj(v -> x[u][v]))
                                     .eq(1));
            prob.addConstraints(nodes,
                                u -> sum(range(0, nodes)
                                         .filter(v -> v != u)
                                         .mapToObj(v -> x[v][u]))
                                     .eq(1));

            // Create a starting solution.
            // This is optional but having a feasible solution available right
            // from the beginning can improve optimizer performance.
            createInitialTour(prob);

            // Write out the model in case we want to look at it.
            prob.writeProb("travelingsalesperson.lp", "l");

            // We don't have all constraints explicitly in the matrix, hence
            // we must disable dual reductions. Otherwise MIP presolve may
            // cut off the optimal solution.
            prob.controls().setMIPDualReductions(0);

            // Add a callback that rejects solutions that do not satisfy
            // the subtour constraints.
            prob.callbacks.addPreIntsolCallback(new PreIntsolCallback());

            // Add a message listener to display log information.
            prob.addMessageListener(DefaultMessageListener::console);

            prob.optimize();
            if (prob.attributes().getSolStatus() != SolStatus.OPTIMAL)
                throw new RuntimeException("failed to solve");

            double[] sol = prob.getSolution();

            // Print the optimal tour.
            System.out.println("Tour with length " + prob.attributes().getMIPObjVal());
            findTour(sol, null);

            x = null; // We are done with the variables
        }
    }

    public static void main(String[] args) {
        new TravelingSalesPerson(10).solve();
    }
}

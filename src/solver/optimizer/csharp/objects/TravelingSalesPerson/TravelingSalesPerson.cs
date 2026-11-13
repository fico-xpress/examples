// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Collections.Generic;
using System.Linq;
using Optimizer;
using Optimizer.Objects;

namespace XPRSexamples
{
    /// <summary>Example for solving a MIP using lazily separated cuts/constraints.</summary>
    /// <remarks>
    /// We solve a random instance of the symmetric TSP using lazily separated
    /// cuts/constraints.
    ///
    /// <p>
    /// The model is based on a directed graph G = (V,E).
    /// We have one binary variable x[e] for each edge e in E. That variable
    /// is set to 1 if edge e is selected in the optimal tour and 0 otherwise.
    /// </p>
    /// <p>
    /// The model contains only two explicit constraints:
    /// <code>
    /// for each v in V: sum(u in V : u != v) x[uv] == 1
    /// for each v in V: sum(u in V : u != v) x[vu] == 1
    /// </code>
    /// These state that node u should have exactly one outgoing and exactly
    /// one incoming edge in a tour.
    /// </p>
    /// <p>
    /// The above constraints ensures that the selected edges form tours. However,
    /// it allows multiple tours, also known as subtours. So we need a constraint
    /// that requires that there is only one tour (which then necessarily hits
    /// all the nodes). This constraint is known as a subtour elimination constraint
    /// and is
    /// <code>
    ///   sum(e in S) x[e] <= |S|-1  for each subtour S
    /// </code>
    /// Since there are exponentially many subtours in a graph, this constraint
    /// is not stated explicitly. Instead we check for any solution that the
    /// optimizer finds, whether it satisfies the subtour elimination constraint.
    /// If it does then we accept the solution. Otherwise we reject the solution
    /// and augment the model by the violated subtour eliminiation constraint.
    /// </p>
    /// <p>
    /// This lazy addition of constraints is implemented using a preintsol callback
    /// that rejects any solution that violates a subtour elimination constraint.
    /// In case the solution came from an integral node, the callback injects a
    /// subtour elimination constraint that cuts off that solution.
    /// </p>
    /// <p>
    /// An important thing to note about this strategy is that dual reductions
    /// have to be disabled. Since the optimizer does not see the whole model
    /// (subtour elimination constraints are only generated on the fly), dual
    /// reductions may cut off the optimal solution.
    /// </p>
    /// </remarks>
    public class TravelingSalesPerson
    {
        /// <summary>
        /// A point in the plane.
        /// </summary>
        public class Point
        {
            public Point(double x, double y)
            {
                X = x;
                Y = y;
            }

            public double X { get; set; }
            public double Y { get; set; }
        }

        /** Number of nodes in the instance. */
        private readonly int nodes;
        /** Number of edges in the instance. */
        private readonly int edges;

        private readonly Point[] coordinates;
        /** Variable indices for the edges. */
        private Variable[,] x;

        /// <summary>Construct a new random instance with random seed 0 and 10 nodes.</summary>
        public TravelingSalesPerson() : this(10, 0) { }

        /// <summary>Construct a new random instance with random seed 0.</summary>
        /// <param name="nodes">The number of nodes in the instance.</param>
        public TravelingSalesPerson(int nodes) : this(nodes, 0) { }

        /// <summary>Construct a new random instance.</summary>
        /// <param name="nodes">The number of nodes in the instance.</param>
        /// <param name="seed">Random number seed.</param>
        public TravelingSalesPerson(int nodes, int seed)
        {
            this.nodes = nodes;
            edges = (nodes * (nodes - 1)) / 2;
            Random rand = new Random(seed);
            coordinates = Enumerable
                .Range(0, nodes)
                .Select((i) => new Point(4.0 * rand.NextDouble(), 4.0 * rand.NextDouble()))
                .ToArray();
        }

        /// <summary>Get the distance between two nodes.</summary>
        /// <param name="u">First node.</param>
        /// <param name="v">Second node.</param>
        /// <returns>The distance between <c>u</c> and <c>v</c>. The distance is symmetric.</returns>
        public double Distance(int u, int v)
        {
            double deltaX = coordinates[u].X - coordinates[v].X;
            double deltaY = coordinates[u].Y - coordinates[v].Y;
            return Math.Sqrt(deltaX * deltaX + deltaY * deltaY);
        }

        /// <summary>Find the tour rooted at 0 in a solution.</summary>
        /// <remarks>As a side effect, the tour is printed to the console.</remarks>
        /// <param name="sol">The current solution.</param>
        /// <param name="from">Stores the tour. <c>from[u]</c> yields the predecessor of <c>u</c> in the tour.
        /// If <c>from[u]</c> is negative then <c>u</c> is not in the tour. This parameter can be <c>null</c>.</param>
        /// <returns>The length of the tour.</returns>
        private int FindTour(double[] sol, int[] from)
        {
            if (from == null)
                from = new int[nodes];
            for (int i = 0; i < from.Length; ++i)
                from[i] = -1;

            int node = 0;
            int used = 0;
            Console.Write("0");
            while (node != 0 || used == 0)
            {
                // Find the edge leaving node
                Variable edge = null;
                for (int i = 0; i < nodes; ++i)
                {
                    if (i != node && x[node,i].GetValue(sol) > 0.5)
                    {
                        Console.Write(" -> {0}", i);
                        edge = x[node,i];
                        from[i] = node;
                        node = i;
                        ++used;
                        break;
                    }
                }
                if (object.ReferenceEquals(edge, null))
                    break;
            }
            Console.WriteLine();

            return used;
        }

        /// <summary>Integer solution check callback.</summary>
        private void PreIntsolCallback(XpressProblem prob, int soltype, ref int p_reject, ref double p_cutoff)
        {
            Console.WriteLine("Checking feasible solution ...");

            // Get current solution and check whether it is feasible
            double[] sol = prob.GetCallbackSolution();
            int[] from = new int[nodes];
            int used = FindTour(sol, from);
            Console.Write("Solution is ");
            if (used < nodes)
            {
                // The tour given by the current solution does not pass through
                // all the nodes and is thus infeasible.
                // If soltype is non-zero then we reject by setting p_reject=1.
                // If instead soltype is zero then the solution came from an
                // integral node. In this case we can reject by adding a cut
                // that cuts off that solution. Note that we must NOT set
                // reject in that case because that would result in just
                // dropping the node, no matter whether we add cuts or not.
                Console.WriteLine("infeasible (" + used + " edges)");
                if (soltype != 0)
                {
                    p_reject = 1;
                }
                else
                {
                    // The solution came from an integral node. Get the edges on the tour and add a
                    // subtour elimination constraint
                    LinExpression subtour = LinExpression.Create();
                    for (int u = 0, next = 0; u < nodes; ++u)
                    {
                        if (from[u] >= 0)
                            subtour.AddTerm(x[from[u],u]);
                    }
                    // We add the constraint. The solver must translate the
                    // constraint from the original space into the presolved
                    // space. This may fail (in theory). In that case the
                    // addCut() function will return non-zero.
                    if (prob.AddCut(1, subtour <= used - 1) != 0)
                        throw new Exception("failed to presolve subtour elimination constraint");
                }
            }
            else
            {
                Console.WriteLine("feasible");
            }
        }

        /// <summary>Create a feasible tour and add this as initial MIP solution.</summary>
        private void CreateInitialTour(XpressProblem prob)
        {
            // Create a tour that just visits each node in order, i.e., goes from u to u+1.
            prob.AddMipSol(Enumerable.Repeat(1.0, nodes).ToArray(),
                Enumerable.Range(0, nodes).Select((u) => x[u, (u + 1) % nodes]).ToArray(),
                "init");
        }

        /// <summary>Solve the TSP represented by this instance.</summary>
        public void Solve()
        {
            using (XpressProblem prob = new XpressProblem(""))
            {
                // Create variables. We create one variable for each edge in
                // the (complete) graph. That is, we create variables from each
                // node u to all other nodes v. We even create a variable for
                // the self-loop from u to u, but that variable is fixed to 0.
                // x[u][v] gives the variable that represents edge uv.
                // All variables are binary.
                x = prob.AddVariables(nodes, nodes)
                        .WithType(ColumnType.Binary)
                        .WithName((i,j) => $"x_{i}_{j}")
                        .WithUB((i,j) => (i == j) ? 0.0 : 1.0)
                        .ToArray();

                // Objective. All variables are in the objective and their
                // respective coefficient is the distance between the two nodes.
                prob.SetObjective(Utils.Sum(nodes,
                                  u => Utils.Sum(nodes,
                                           v => x[u, v] * Distance(u, v))),
                                  ObjSense.Minimize);

                // Constraint: In the graph that is induced by the selected
                //             edges, each node should have exactly one outgoing.
                //             and exactly one incoming edge.
                //             These are the only constraints we add explicitly.
                //             Subtour elimination constraints are added
                //             dynamically via a callback.
                prob.AddConstraints(nodes,
                                    u => Utils.Sum(Enumerable.Range(0, nodes)
                                              .Where(v => v != u)
                                              .Select(v => x[u, v]))
                                         == 1.0);
                prob.AddConstraints(nodes,
                                    u => Utils.Sum(Enumerable.Range(0, nodes)
                                              .Where(v => v != u)
                                              .Select(v => x[v, u]))
                                         == 1.0);

                // Create a starting solution.
                // This is optional but having a feasible solution available right
                // from the beginning can improve optimizer performance.
                CreateInitialTour(prob);

                // Write out the model in case we want to look at it.
                prob.WriteProb("travelingsalesperson.lp", "l");

                // We don't have all constraints explicitly in the matrix, hence
                // we must disable dual reductions. Otherwise MIP presolve may
                // cut off the optimal solution.
                prob.MIPDualReductions = 0;

                // Add a callback that rejects solutions that do not satisfy
                // the subtour constraints and adds subtour elimination constraints
                // if possible.
                prob.callbacks.AddPreIntsolCallback(PreIntsolCallback);

                // Add a message listener to display log information.
                prob.AddMessageCallback(DefaultMessageListener.Console);

                prob.Optimize();

                double[] sol = prob.GetSolution();
                if (prob.SolStatus != Optimizer.SolStatus.Optimal)
                    throw new Exception("failed to solve");


                // Print the optimal tour.
                Console.WriteLine("Tour with length " + prob.MIPObjVal);
                FindTour(sol, null);
            }
        }

        public static void Main(String[] args)
        {
            new TravelingSalesPerson(10).Solve();
        }
    }
}

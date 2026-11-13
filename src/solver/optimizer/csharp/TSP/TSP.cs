/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file TSP.cs
   ````````````
   Solve a MIP using cuts/constraints that are lazily separated.

   We take a random instance of the symmetric TSP and solve that using
   lazily separated constraints.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

using System;
using System.Linq;
using Optimizer;

namespace XPRSexamples
{
    /// <summary>Example for solving a MIP using lazily separated cuts/constraints.</summary>
    /// <remarks>
    /// We solve a random instance of the symmetric TSP using lazily separated
    /// cuts/constraints.
    ///
    /// <p>
    /// The model is based on a graph G = (V,E).
    /// We have one binary variable x[e] for each edge e in E. That variable
    /// is set to 1 if edge e is selected in the optimal tour and 0 otherwise.
    /// </p>
    /// <p>
    /// The model contains only one explicit constraint:
    /// <code>
    /// for each v in V: sum(u in V : u != v) x[uv] == 2
    /// </code>
    /// This states that from all the edges incident to a node u, exactly two
    /// must be selected in the optimal tour.
    /// </p>
    /// <p>
    /// The above constraints ensures that the selected edges form tours. However,
    /// it allows multiple tours, also known as subtours. So we need a constraint
    /// that requires that there is only one tour (which then necessarily hits
    /// all the nodes). This constraint is known as subtour elimination constraint
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
    public class TSP
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
        private readonly int[,] x;

        /// <summary>Construct a new random instance with random seed 0 and 10 nodes.</summary>
        public TSP() : this(10, 0) {}

        /// <summary>Construct a new random instance with random seed 0.</summary>
        /// <param name="nodes">The number of nodes in the instance.</param>
        public TSP(int nodes) : this(nodes, 0) {}

        /// <summary>Construct a new random instance.</summary>
        /// <param name="nodes">The number of nodes in the instance.</param>
        /// <param name="seed">Random number seed.</param>
        public TSP(int nodes, int seed)
        {
            this.nodes = nodes;
            edges = (nodes * (nodes - 1)) / 2;
            Random rand = new Random(seed);
            coordinates = Enumerable
                .Range(0, nodes)
                .Select((i) => new Point(4.0 * rand.NextDouble(), 4.0 * rand.NextDouble()))
                .ToArray();
            x = new int[nodes, nodes];
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
            bool[] inTour = new bool[edges]; // Marks edges on the subtour

            int u = 0;
            int used = 0;
            Console.Write("0");
            do
            {
                for (int v = 0; v < nodes; ++v)
                {
                    if (u == v)                    // no self-loops
                        continue;
                    else if (from[v] != -1)        // node already on tour
                        continue;
                    else if (sol[x[u, v]] < 0.5)   // edge not selected in solution
                        continue;
                    else if (inTour[x[u, v]])      // edge already on tour
                        continue;
                    else
                    {
                        Console.Write(" -> " + v);
                        inTour[x[u, v]] = true;
                        from[v] = u;
                        used += 1;
                        u = v;
                        break;
                    }
                }
            } while (u != 0);
            Console.WriteLine();

            return used;
        }

        /// <summary>Integer solution check callback.</summary>
        private void PreIntsolCallback(XPRSprob prob, object cbdata, int soltype, ref int p_reject, ref double p_cutoff) {
            Console.WriteLine("Checking feasible solution ...");

            // Get current solution and check whether it is feasible
            double[] sol = prob.GetCallbackSolution();
            int[] from = new int[nodes];
            int used = FindTour(sol, from);
            Console.Write("Solution is ");
            if (used<nodes)
            {
                // The tour given by the current solution does not pass through
                // all the nodes and is thus infeasible.
                // If soltype is non-zero then we reject by setting p_reject=1.
                // If instead soltype is zero then the solution came from an
                // integral node. In this case we can reject by adding a cut
                // that cuts off that solution. Note that we must NOT set
                // p_reject=1 in that case because that would result in just
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
                    int[] ind = new int[used];
                    double[] val = Enumerable.Repeat(1.0, used).ToArray();
                    for (int u = 0, next = 0; u < nodes; ++u)
                    {
                        if (from[u] >= 0)
                            ind[next++] = x[u, from[u]];
                    }
                    // Since we created the constraint in the original space, we must
                    // crush it to the presolved space before we can add it.
                    XPRSprob.RowInfo r = prob.PresolveRow(ind, val, 'L', used - 1);
                    if (r != null)
                        prob.AddCut(1, r);
                    else
                        throw new Exception("failed to presolve subtour elimination constraint");
                }
            }
            else
            {
                Console.WriteLine("feasible");
            }
        }

        /// <summary>Create a feasible tour and add this as initial MIP solution.</summary>
        private void CreateInitialTour(XPRSprob prob)
        {
            int[] ind = new int[nodes];
            double[] val = new double[nodes];
            // Create a tour that just visits each node in order, i.e., goes from u to u+1.
            prob.AddMipSol(Enumerable.Repeat(1.0, nodes).ToArray(),
                Enumerable.Range(0, nodes).Select((u) => x[u, (u + 1) % nodes]).ToArray());
        }

        /// <summary>Solve the TSP represented by this instance.</summary>
        public void Solve()
        {
            using (XPRSprob prob = new XPRSprob("")) {
                // Create variables. We create one variable for each edge in
                // the (complete) graph. x[u][v] gives the index of the variable
                // that represents edge uv. Since edges are undirected, x[v][u]
                // gives the same variable.
                // All variables are binary.
                for (int i = 0; i < nodes; ++i)
                {
                    for (int j = i + 1; j < nodes; ++j)
                    {
                        x[j, i] = x[i, j] = prob.BinVar("x_" + i + "_" + j);
                    }
                }

                // Objective. All variables are in the objective and their
                // respective coefficient is the distance between the two nodes.
                int[] objInd = new int[edges];
                double[] objVal = new double[edges];
                for (int u = 0, nz = 0; u < nodes; ++u)
                {
                    for (int v = u + 1; v < nodes; ++v)
                    {
                        objInd[nz] = x[u, v];
                        objVal[nz] = Distance(u, v);
                        ++nz;
                    }
                }
                prob.SetObjective(objInd, objVal, ObjSense.Minimize);

                // Constraint: In the graph that is induced by the selected
                //             edges, each node should have degree 2.
                //             This is the only constraint we add explicitly.
                //             Subtour elimination constraints are added
                //             dynamically via a callback.
                for (int u = 0; u < nodes; ++u)
                {
                    int[] ind = Enumerable.Range(0, nodes)
                        .Where((v) => v != u)
                        .Select((v) => x[u,v])
                        .ToArray();
                    double[] val = Enumerable.Repeat(1.0, ind.Length).ToArray();
                    prob.AddRow(ind, val, 'E', 2);
                }

                // Create a starting solution.
                // This is optional but having a feasible solution available right
                // from the beginning can improve optimizer performance.
                CreateInitialTour(prob);

                // Write out the model in case we want to look at it.
                prob.WriteProb("tsp.lp", "l");

                // We don't have all constraints explicitly in the matrix, hence
                // we must disable dual reductions. Otherwise MIP presolve may
                // cut off the optimal solution.
                prob.MIPDualReductions = 0;

                // Add a callback that rejects solutions that do not satisfy
                // the subtour constraints and adds subtour elimination constraints
                // if possible.
                prob.AddPreIntsolCallback(PreIntsolCallback);

                // Add a message listener to display log information.
                prob.AddMessageCallback(DefaultMessageListener.Console);

                prob.Optimize();

                double[] sol = prob.GetSolution();


                // Print the optimal tour.
                Console.WriteLine("Tour with length " + prob.MIPObjVal);
                FindTour(sol, null);
            }
        }

        public static void Main(String[] args)
        {
            new TSP(10).Solve();
        }
    }
}

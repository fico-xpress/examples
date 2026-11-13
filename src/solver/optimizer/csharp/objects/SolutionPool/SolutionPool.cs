//   Xpress Optimizer Examples
//   =========================
//
//   file SolutionPool.cs
//   ````````````````````
//   Capacitated Facility Location Problem (CFLP) demonstrating the
//   solution pool and solution enumeration.
//
//   25 customers, 10 potential facility sites with integer coordinates.
//   Single-sourcing: each customer is assigned to exactly one open
//   facility. Service cost equals the Euclidean distance from the
//   customer to its assigned facility. Fixed opening cost is correlated
//   with facility capacity.
//
//   Demonstrates four uses of the built-in solution pool and enumeration
//   controls on the CFLP model:
//   - collecting up to ten solutions found during a normal MIP solve
//     (POOLLIMIT);
//   - warm-starting a re-solve from the pool after fixing a facility
//     open (POOLKEEPSOLS);
//   - enumerating the five best solutions overall (POOLENUMSTRATEGY,
//     POOLRELGAP);
//   - enumerating the five best facility assignments only, by
//     restricting enumeration to the facility variables
//     (XpressProblem.LoadBranchDirs()).
//   After each solve, the objective value, number of open facilities,
//   and maximum service distance are printed for each collected solution.
//
//   Model
//   -----
//   Sets
//   I = {0 ... N_CUST-1}   customers
//   J = {0 ... N_FAC-1}    potential facility sites
//
//   Parameters
//   d_i      demand of customer i
//   f_j      fixed cost of opening facility j
//   Q_j      capacity of facility j
//   c_ij     Euclidean distance from customer i to facility j
//
//   Decision variables
//   y_j  in {0,1}   1 iff facility j is opened
//   x_ij in {0,1}   1 iff customer i is served by facility j
//
//   Objective
//   min  sum_j f_j y_j  +  sum_i sum_j c_ij x_ij
//
//   Constraints
//   (1)  sum_j x_ij = 1               for all i  (demand satisfaction)
//   (2)  sum_i d_i x_ij <= Q_j y_j    for all j  (capacity + activation)
//
//   (c) 2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;


namespace XpressExamples
{
    public class SolutionPool
    {
        const int N_CUST = 25;
        const int N_FAC = 10;

        static readonly int[,] custXY = {
            {20,  9}, {22, 15}, { 5, 13}, { 6, 26}, {13, 14}, // cluster A -- south-west
            {15, 19}, {20, 19}, {11,  7}, { 3, 21},
            {70, 27}, {69, 33}, {82, 26}, {74, 30}, {77, 20}, // cluster B -- south-east
            {82, 11}, {72, 25}, {70, 24},
            {56, 82}, {53, 74}, {50, 84}, {39, 81}, {55, 80}, // cluster C -- north
            {43, 84},
            {46, 47}, {30, 55}                                // outliers  -- central
        };

        static readonly int[] demands = {
            26, 25, 37, 22, 35, 22, 43, 21, 41,   // cluster A
            29, 61, 32, 41, 40, 39, 19, 72,       // cluster B
            23, 40, 59, 27, 45, 25,               // cluster C
            21, 40                                // outliers
        };

        static readonly int[,] facXY = {
            { 7, 15}, {12, 19},  // j=0,1 -- near cluster A
            {76, 19}, {80, 21},  // j=2,3 -- near cluster B
            {56, 88}, {52, 79},  // j=4,5 -- near cluster C
            {45, 50}, {63, 48},  // j=6,7 -- central bridge
            { 3, 93}, { 3, 88}   // j=8,9 -- peripheral
        };

        static readonly int[] capacities = { 307, 98, 86, 298, 211, 190, 70, 205, 140, 165 };
        static readonly int[] fixedCosts = { 787, 233, 257, 756, 567, 517, 227, 535, 367, 445 };


        /// <summary>Struct for collecting model data.</summary>
        public class ModelData
        {
            public readonly Variable[,] x;
            public readonly Variable[] y;
            public readonly double[,] dist;
            public ModelData(Variable[,] x, Variable[] y, double[,] dist)
            {
                this.x = x;
                this.y = y;
                this.dist = dist;
            }
        }


        /// <summary>Builds the CFLP model on prob.</summary>
        static ModelData BuildModel(XpressProblem prob)
        {
            ModelData modelData;

            // ---- Compute service costs ----
            double[,] dist = new double[N_CUST, N_FAC];
            for (int i = 0; i < N_CUST; i++)
            {
                for (int j = 0; j < N_FAC; j++)
                {
                    double dx = (double)(custXY[i, 0] - facXY[j, 0]);
                    double dy = (double)(custXY[i, 1] - facXY[j, 1]);
                    dist[i, j] = Math.Sqrt(dx * dx + dy * dy);
                }
            }

            // ---- Build the variables ----
            // Add the N_FAC facility binary variables.
            Variable[] y = prob.AddVariables(N_FAC)
              .WithType(ColumnType.Binary)
              .WithName("y({0})")
              .ToArray();
            // Add the N_CUST*N_FAC service binary variables.
            Variable[,] x = prob.AddVariables(N_CUST, N_FAC)
              .WithType(ColumnType.Binary)
              .WithName("x({0},{1})")
              .ToArray();

            // ---- Build the constraints ----
            // Demand satisfaction constraints.
            //    sum_j x_ij = 1 for all i
            prob.AddConstraints(N_CUST, i => Sum(N_FAC, j => x[i, j]) == 1.0);
            // Capacity and activation constraints.
            //    sum_i d_i x_ij <= Q_j y_j for all j
            prob.AddConstraints(N_FAC, j => Sum(N_CUST, i => demands[i] * x[i, j]) <= capacities[j] * y[j]);
            // ---- Build the objective function ----
            //    sum_j f_j y_j  +  sum_i sum_j c_ij x_ij
            prob.SetObjective(Sum(N_FAC, j => fixedCosts[j] * y[j]) + Sum(N_CUST, N_FAC, (i, j) => dist[i, j] * x[i, j]));

            return new ModelData(x, y, dist);
        }

        /// <summary>Print the list of collected solutions. We list the following information.</summary>
        /// <remarks>
        ///   - Objective function value
        ///   - Number of facilities opened
        ///   - Maximum distance between a facility and a customer it serves.
        /// </remarks>
        static void PrintSolutions(XpressProblem prob, ModelData modelData)
        {
            int poolSize = prob.PoolSize;

            Console.WriteLine($"Collected ${poolSize} solution(s):");
            Console.WriteLine("{0,4}  {1,12}  {2,11}  {3,9}",
                              "Sol#", "Objective", "Facilities", "Max dist");
            for (int p = 0; p < poolSize; p++)
            {
                double poolObj = prob.GetPoolSolObjVal(p);
                double[] sol = prob.GetPoolSolution(p);

                int nOpen = 0;
                double maxDist = 0.0;
                for (int j = 0; j < N_FAC; j++)
                    nOpen += (modelData.y[j].GetValue(sol) > 0.1) ? 1 : 0;
                for (int i = 0; i < N_CUST; i++)
                    for (int j = 0; j < N_FAC; j++)
                        if (modelData.x[i, j].GetValue(sol) > 0.1)
                        {
                            maxDist = Math.Max(maxDist, modelData.dist[i, j]);
                            break;
                        }
                Console.WriteLine("{0,4}  {1,12}  {2,11}  {3,9}",
                                  p, poolObj, nOpen, maxDist);
            }
            Console.WriteLine();
        }

        /// <summary>Solve our model and collect up to 10 solutions into the pool.</summary>
        static void SolveCollectSolutions(XpressProblem prob, ModelData modelData)
        {
            // Collect up to ten solutions during the MIP search. Solutions are
            // stored from best to worst; index 0 always holds the best solution.
            prob.PoolLimit = 10;

            Console.WriteLine("Solving problem to collect solutions");
            prob.Optimize();
            SolveStatus solveStatus = prob.SolveStatus;
            SolStatus solStatus = prob.SolStatus;
            if (solveStatus == SolveStatus.Completed && solStatus == SolStatus.Optimal)
            {
                Console.WriteLine("Solved problem to optimality.");
            }
            else
            {
                Console.WriteLine($"Failed to solve problem with solvestatus {solveStatus} and solstatus {solStatus}");
                return;
            }

            // ---- Print the solution pool ----
            PrintSolutions(prob, modelData);
        }

        /// <summary
        /// Assumes that we have already solved the problem and that the pool contains
        /// a set of solutions.
        /// </summary>
        /// <remarks>
        /// This function forces a single facility to be open and uses the pooled
        /// solutions to warm-start the next MIP solve.
        /// </remarks>
        static void SolveWarmStartFromPool(XpressProblem prob, ModelData modelData)
        {
            // Make sure the problem is in its original state before we change it.
            // If the problem is already in its original state, this call will
            // simply return without doing anything.
            prob.PostSolve();

            // We need to set the control POOLKEEPSOLS to transfer any existing
            // solutions to the warm-start pool before we fix a variable, since any
            // modifications will invalidate all collected selections.
            prob.PoolKeepSols = -1;

            // Force a facility to be open. If we open an additional facility, all
            // existing solution should remain feasible - the presolver should take
            // care of satisfying the fixed binary. The warm-start solutions will
            // not be optimal since no customers are served by the new facility.
            int yIdx = 2; // Pick a facility that should not be open in our current optimal solution.
            modelData.y[yIdx].Fix(1.0);

            // ---- Re-solve ----
            Console.WriteLine($"Reoptimizing with fixed facility {yIdx}");
            prob.Optimize();
            SolveStatus solveStatus = prob.SolveStatus;
            SolStatus solStatus = prob.SolStatus;
            if (solveStatus == SolveStatus.Completed && solStatus == SolStatus.Optimal)
            {
                Console.WriteLine("Resolved problem to optimality.");
            }
            else
            {
                Console.WriteLine($"Failed to resolve problem with solvestatus {solveStatus} and solstatus {solStatus}");
                return;
            }

            // ---- Print the new solution pool ----
            PrintSolutions(prob, modelData);

            // Reset controls. We should do this before modifying the problem
            // to avoid automatically transferring the solutions when restoring
            // the bound below.
            prob.SetDefaultControl(XPRScontrol.PoolKeepSols);

            // Undo the fixed facility by returning the lower bound to 0.0.
            modelData.y[yIdx].SetLB(0.0);
        }

        /// <summary>
        /// Use the built-in solution enumeration to find the 5 best solutions to
        /// our problems.
        /// </summary>
        /// <remarks>
        /// The solver, by default, considers two solutions as being
        /// distinct if they differ in any of our binary restricted variables.
        /// To help speed up the solve, we limit the search to solutions whose
        /// objective value is within 50% of the best solution.
        /// </remarks>
        static void EnumerateAllBinaries(XpressProblem prob, ModelData modelData)
        {
            // Collect up to 5 best solutions.
            prob.PoolLimit = 5;

            // Enable automatic solution enumeration.
            prob.PoolEnumStrategy = Optimizer.PoolEnumStrategy.Full;

            // We limit collection to solutions that are within 50% of the best
            // solution.
            prob.PoolRelGap = 0.50;

            // Solve the problem, which will automatically trigger an extended
            // branch-and-bound search to find all 5 best solutions.
            Console.WriteLine("Solving problem to find 5 best solutions.");
            prob.Optimize();
            SolveStatus solveStatus = prob.SolveStatus;
            SolStatus solStatus = prob.SolStatus;

            if (solveStatus == SolveStatus.Completed && solStatus == SolStatus.Optimal)
            {
                Console.WriteLine("Solved problem to optimality.");
            }
            else
            {
                Console.WriteLine($"Failed to solve problem with solvestatus {solveStatus} and solstatus {solStatus}");
                return;
            }

            // Print the collected solutions.
            PrintSolutions(prob, modelData);
        }

        /// <summary>
        /// We want to find the 5 best combinations of opening facilities but
        /// the demand sourcing should be optimal in each solution.
        /// </summar>
        /// <remarks>
        /// To achieve this, we need to tell the solver to enumerate only
        /// through solutions that differ in facility variables. We can do
        /// this through solution enumeration by specifying branching directives
        /// only on the facility variables.
        /// The solver will automatically treat two solutions with the same
        /// facility assignment as duplicates and only keep the best one, which
        /// should result in a solution with the optimal demand assignment for
        /// each facility assignment.
        /// </remarks>
        static void EnumerateFacilities(XpressProblem prob, ModelData modelData)
        {
            // Make sure the problem is in its original state before we change it.
            // If the problem is already in its original state, this call will
            // simply return without doing anything.
            prob.PostSolve();

            // Collect up to 5 best solutions.
            prob.PoolLimit = 5;

            // Enable automatic solution enumeration.
            prob.PoolEnumStrategy = Optimizer.PoolEnumStrategy.Full;

            // Apply branching directives to the facility binaries.
            // Binaries without a directive will not be enumerated.
            int[] directivesOn = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
            prob.LoadBranchDirs(modelData.y, directivesOn);

            // Solve the problem, which will automatically trigger an extended
            // branch-and-bound search to find the 5 best facility assignments.
            Console.WriteLine("Solving problem to find 5 best facility assignments.");
            prob.Optimize();
            SolveStatus solveStatus = prob.SolveStatus;
            SolStatus solStatus = prob.SolStatus;
            if (solveStatus == SolveStatus.Completed && solStatus == SolStatus.Optimal)
            {
                Console.WriteLine("Resolved problem to optimality.");
            }
            else
            {
                Console.WriteLine($"Failed to solve problem with solvestatus {solveStatus} and solstatus {solStatus}");
                return;
            }

            // Print the collected solutions.
            PrintSolutions(prob, modelData);

            // Reset controls.
            prob.SetDefaultControl(XPRScontrol.PoolLimit);
            prob.SetDefaultControl(XPRScontrol.PoolEnumStrategy);

            // Clear branching directives.
            int[] directivesOff = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            prob.LoadBranchDirs(modelData.y, directivesOff);
        }

        public static void Main(string[] args)
        {
            // Create a new problem and immediately register a message handler.
            // Once we have a message handler installed, errors will produce verbose
            // error messages on the console and we can limit ourselves to minimal
            // error handling in the code here.
            using (XpressProblem prob = new XpressProblem())
            {
                prob.AddMessageCallback(DefaultMessageListener.Console);

                // ---- Build the model ----
                ModelData modelData = BuildModel(prob);

                // Solve the model and collect solutions.
                SolveCollectSolutions(prob, modelData);

                // Try forcing a facility to be opened and reoptimize. We carry the
                // existing solutions through as warm-start solutions for the next
                // solve.
                SolveWarmStartFromPool(prob, modelData);

                // Run an enumeration search to find the 5 best solutions.
                EnumerateAllBinaries(prob, modelData);

                // Run an enumeration search to find the 5 best facility assignments.
                EnumerateFacilities(prob, modelData);
            }
        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using Optimizer.Objects;
using ColumnType = Optimizer.ColumnType;
using static Optimizer.Objects.Utils;
using static Optimizer.Objects.LinExpression;

namespace XpressExamples
{
    /// <summary>Load balancing of train wagons.</summary>
    /// <remarks>
    ///   second version, using heuristic solution as start solution for MIP
    /// </remarks>
    class Wagon
    {
        /* Box weights */
        private static readonly int[] WEIGHT = { 34, 6, 8, 17, 16, 5, 13, 21, 25, 31, 14, 13, 33, 9, 25, 25 };
        private static readonly int NBOXES = WEIGHT.Length;       /* Number of boxes                       */
        private static readonly int NWAGONS = 3;                  /* Number of wagons                      */
        private static readonly int WMAX = 100;                   /* Weight limit of the wagons            */
        private static readonly int[] HeurSol = new int[NBOXES];  /* Heuristic solution: for each box      */

        /* Variables */
        private static Variable[,] load;
        private static Variable maxweight;

        private static void model(XpressProblem p)
        {
            // Create load[box,wagon] (binary)
            load = p.AddVariables(NBOXES, NWAGONS)
                .WithType(ColumnType.Binary)
                .WithName((b, w) => $"load_{b + 1}_{w + 1}")
                .ToArray();

            // Create maxweight (continuous with lb=ceil((sum(b in BOXES) WEIGHT(b))/NBOXES)
            maxweight = p.AddVariable(
                Math.Ceiling(WEIGHT.Sum() / (double)NBOXES),
                Double.PositiveInfinity,
                ColumnType.Continuous,
                "maxweight"
            );

            // Every box into one wagon: forall(b in BOXES) sum(w in WAGONS) load(b,w) = 1
            p.AddConstraints(NBOXES,
                    b => Sum(NWAGONS, w => load[b, w]) == 1.0);

            // Limit the weight loaded into every wagon: forall(w in WAGONS) sum(b in BOXES) WEIGHT(b)*load(b,w) <= maxweight
            p.AddConstraints(NWAGONS,
                    w => Sum(NBOXES, b => WEIGHT[b] * load[b, w])
                    .Leq(maxweight).SetName($"wmilit_{w}")
                    );

            // Minimize maxweight
            p.SetObjective(maxweight, Optimizer.ObjSense.Minimize);
            p.WriteProb("Wagon.lp", "l");
        }

        /** LPT (Longest processing time) heuristic:
         * One at a time, place the heaviest unassigned
         * box onto the wagon with the least load
         * @return A dense vector with the heuristic solution.
         */
        private static double heuristic()
        {
            int[] ORDERW = new int[NBOXES];   // Box indices sorted in decreasing weight order
            int[] CurNum = new int[NWAGONS];          // For each wagon w, this is the number of boxes currently loaded
            int[] CurWeight = new int[NWAGONS];       // For each wagon w, this is the current weight, i.e. the sum of weights of loaded boxes
            int[,] Load = new int[NWAGONS, NBOXES];  // Load[w][i] (for i=0..CurNum[w]-1) contains the box index of the i-th box loaded on wagon w

            // Copy the box indices into array ORDERW and sort them in decreasing
            // order of box weights (the sorted indices are returned in array ORDERW)
            for (int b = 0; b < NBOXES; b++) ORDERW[b] = b;
            Array.Sort(ORDERW, (i1, i2) => WEIGHT[i2].CompareTo(WEIGHT[i1]));

            // Distribute the loads to the wagons using the LPT heuristic
            for (int b = 0; b < NBOXES; b++)
            {
                int v = 0;                          /* Find wagon v with the smallest load */
                for (int w = 0; w < NWAGONS; w++) if (CurWeight[w] <= CurWeight[v]) v = w;
                Load[v, CurNum[v]] = ORDERW[b];     /* Add current box to wagon v */
                CurNum[v]++;                        /* Increase the counter of boxes on v */
                CurWeight[v] += WEIGHT[ORDERW[b]];  /* Update current weight of the wagon */
            }

            // Calculate the solution value
            double heurobj = 0;                   /* heuristic solution objective value (max wagon weight) */
            for (int w = 0; w < NWAGONS; w++) if (CurWeight[w] > heurobj) heurobj = CurWeight[w];

            // Solution printing
            Console.WriteLine("Heuristic solution:");
            Console.WriteLine("Max weight: ", heurobj);

            for (int w = 0; w < NWAGONS; w++)
            {
                Console.Write($" {w + 1}:");
                for (int i = 0; i < CurNum[w]; i++) Console.Write(" " + (Load[w, i] + 1));
                Console.WriteLine($" (total weight: {CurWeight[w]:%d})");
            }

            // Save the heuristic solution into the HeurSol array
            for (int w = 0; w < NWAGONS; w++) for (int i = 0; i < CurNum[w]; i++) HeurSol[Load[w, i]] = w;
            return heurobj;
        }

        private static void optimization(XpressProblem p)
        {
            // Get the solution from the heuristic solution we have found
            // Send the solution to the optimizer
            Variable[] heurmipvar = new Variable[NBOXES];
            double[] heurmipsol = new double[NBOXES];
            for (int b = 0; b < NBOXES; b++)
            {
                heurmipvar[b] = load[b, HeurSol[b]];
                heurmipsol[b] = 1.0;
            }
            p.AddMipSol(heurmipsol, heurmipvar, "heuristic");
            p.Optimize();
            if (p.SolStatus != Optimizer.SolStatus.Optimal &&
                    p.SolStatus != Optimizer.SolStatus.Feasible)
                throw new Exception("failed to optimize with status " + p.SolStatus);

            Console.WriteLine(
                    $"Problem status:\n\tSolve status: {p.SolveStatus}\n\tLP status: {p.LPStatus}\n\tMIP status: {p.MIPStatus}\n\tSol status: {p.SolStatus}");

            // An integer solution has been found
            if (p.MIPStatus == Optimizer.MIPStatus.Optimal
                    || p.MIPStatus == Optimizer.MIPStatus.Solution)
            {
                double[] sol = p.GetSolution();
                Console.WriteLine($"Optimal solution:\n Max weight: {p.ObjVal}");
                for (int w = 0; w < NWAGONS; w++)
                {
                    double tot_weight = 0;
                    Console.Write($" {w + 1}:");
                    for (int b = 0; b < NBOXES; b++) if (load[b, w].GetValue(sol) * WEIGHT[b] > .5)
                        {
                            Console.Write(" {0:f}", load[b, w].GetValue(sol) * WEIGHT[b]);
                            tot_weight += load[b, w].GetValue(sol) * WEIGHT[b];
                        }
                    Console.WriteLine(" (total weight: {0:f}", tot_weight);
                }
            }
        }

        public static void Main(string[] args)
        {
            if (heuristic() <= WMAX)
            {
                Console.WriteLine("Heuristic solution fits capacity limits");
            }
            else
            {
                using (XpressProblem prob = new XpressProblem())
                {
                    /* Suppress outputs */
                    model(prob);
                    optimization(prob);
                }
            }
        }
    }
}

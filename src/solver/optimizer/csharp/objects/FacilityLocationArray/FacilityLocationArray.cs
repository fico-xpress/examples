using System;
using System.Collections.Generic;
using System.Linq;
using Optimizer;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;

namespace XPRSexamples
{
    /// <summary>This example demonstrates some modeling devices.</summary>
    /// <remarks>
    /// We model a very simple facility location problem: We have customers
    /// and facilities. The constraints are:
    /// - each customer must be served from exactly one facility
    /// - customers can only be served from open facilities
    /// - customer demand must be satisfied
    /// - facility capacity must not be exceeded
    ///
    /// We minimize the sum of transport cost (between customer and facility)
    /// and the cost for opening a facility.
    /// In this example data is kept in arrays.
    /// </remarks>
    internal class FacilityLocationArray
    {
        /// <summary>Customer description.</summary>
        private sealed class Customer
        {
            /// <summary>Name of customer.</summary>
            public readonly string Name;
            /// <summary>Demand for customer.</summary>
            public readonly double Demand;

            public Customer(string name, double demand)
            {
                this.Demand = demand;
                this.Name = name;
            }
        }

        /// <summary>Facility descriptor.</summary>
        private sealed class Facility
        {
            /// <summary>Name of facility.</summary>
            public readonly string Name;
            /// <summary>Capacity of facility.</summary>
            public readonly double Capacity;
            /// <summary>Cost for opening this facility.</summary>
            public readonly double Cost;

            public Facility(string name, double capacity, double cost)
            {
                this.Name = name;
                this.Capacity = capacity;
                this.Cost = cost;
            }
        }

        /// <summary>Customers in this example.</summary>
        private static readonly Customer[] customers = new Customer[] { new Customer("Customer 1", 80),
            new Customer("Customer 2", 270), new Customer("Customer 3", 250) };
        /// <summary>Facilities in this example.</summary>
        private static readonly Facility[] facilities = new Facility[] { new Facility("Facility 1", 500, 1000),
            new Facility("Facility 2", 500, 1000), new Facility("Facility 3", 500, 1000) };
        /// <summary>Cost for transporting one unit between customer and facility.</summary>
        private static readonly double[][] transportCost = new double[][] { new double[] { 4, 5, 6 }, new double[] { 6, 4, 3 },
            new double[] { 9, 7, 4 } };

        public static void Main(string[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                Variable[] y = prob.AddVariables(facilities.Length)
                    .WithType(ColumnType.Binary)
                        .WithName(f => facilities[f].Name)
                        .ToArray();
                Variable[,] x = prob.AddVariables(facilities.Length, customers.Length)
                        .WithUB(Double.PositiveInfinity)
                        .WithName((f, c) => $"x[{facilities[f].Name},{customers[c].Name}")
                        .ToArray();

                // for each customer c
                // sum(f=1..m) x[f,c] = d
                prob.AddConstraints(customers.Length, c => Sum(facilities.Length, f => x[f, c]) == customers[c].Demand);

                // for each facility f
                // sum(c=1..n) x[f,c] <= capacity[j] * y[f]
                prob.AddConstraints(facilities.Length, f => Sum(customers.Length, c => x[f, c]) <= y[f] * facilities[f].Capacity);

                // minimize sum(f=1..m) cost[f] * y[f] +
                // sum(c=1..n) sum(f=1..m) cost[f,c] * x[f,c]
                prob.SetObjective(Sum(Sum(facilities.Length, f => y[f] * facilities[f].Cost),
                        Sum(customers.Length, c => Sum(facilities.Length, f => x[f, c] * transportCost[f][c]))));

                prob.WriteProb("facilitylocationarray.lp");

                prob.Optimize();
                if (prob.SolStatus != SolStatus.Optimal)
                    throw new Exception($"failed to optimize with status {prob.SolStatus}");
                double[] sol = prob.GetSolution();
                for (int f = 0; f < facilities.Length; ++f)
                {
                    if (y[f].GetValue(sol) > 0.5)
                    {
                        Console.WriteLine($"Facility {facilities[f].Name} is open, serves");
                        for (int c = 0; c < customers.Length; ++c)
                        {
                            if (x[f, c].GetValue(sol) > 0.0)
                                Console.WriteLine($"  {customers[c].Name}: {x[f, c].GetValue(sol)}");
                        }
                    }
                }
            }
        }
    }
}

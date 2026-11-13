// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using Optimizer;
using static Optimizer.Objects.Utils;
using Variable = Optimizer.Objects.Variable;
using XpressProblem = Optimizer.Objects.XpressProblem;
using Optimizer.Maps;
using System.Collections.Generic;

namespace XPRSexamples
{
    /// <summary>Solve a facility location problem for which data is given as collections.</summary>
    public class FacilityLocationCollection
    {
        /// <summary>Customer descriptor.</summary>
        private sealed class Customer
        {
            /// <summary>Customer name.</summary>
            public readonly String name;
            /// <summary>Demand for this customer.</summary>
            public readonly double demand;
            public Customer(String name, double demand)
            {
                this.demand = demand;
                this.name = name;
            }
        }

        /// <summary>Facility descriptor.</summary>
        private sealed class Facility
        {
            /// <summary>Facility name.</summary>
            public readonly String name;
            /// <summary>Capacity of this facility.</summary>
            public readonly double capacity;
            /// <summary>Cost for opening this facility.</summary>
            public readonly double cost;
            public Facility(String name, double capacity, double cost)
            {
                this.name = name;
                this.capacity = capacity;
                this.cost = cost;
            }
        }

        /// <summary>The customers in this example.</summary>
        private static readonly Customer[] customers = new Customer[]{
            new Customer("Customer 1", 80),
            new Customer("Customer 2", 270),
            new Customer("Customer 3", 250)
        };
        /// <summary>The facilityies in this example.</summary>
        private static readonly Facility[] facilities = new Facility[]{
            new Facility("Facility 1", 500, 1000),
            new Facility("Facility 2", 500, 1000),
            new Facility("Facility 3", 500, 1000)
        };

        /// <summary>Cost for transporting one unit between a customer and a facility.</summary>
        private static readonly HashMap2<Facility, Customer, double> transportCost = new HashMap2<Facility, Customer, double>();
        static FacilityLocationCollection()
        {
            transportCost.Add(facilities[0], customers[0], 4.0);
            transportCost.Add(facilities[0], customers[1], 5.0);
            transportCost.Add(facilities[0], customers[2], 6.0);
            transportCost.Add(facilities[1], customers[0], 6.0);
            transportCost.Add(facilities[1], customers[1], 4.0);
            transportCost.Add(facilities[1], customers[2], 3.0);
            transportCost.Add(facilities[2], customers[0], 9.0);
            transportCost.Add(facilities[2], customers[1], 7.0);
            transportCost.Add(facilities[2], customers[2], 4.0);
        }

        public static void Main(String[] args)
        {
            using (XpressProblem prob = new XpressProblem())
            {
                // Create a 1-dimensional map of y variables, indexed by Facility instances.
                var y = prob.AddVariables(facilities)
                            .WithType(ColumnType.Binary)
                            .WithName(f => f.name)
                            .ToMap();
                // Create a 2-dimensional map of x variables, indexed by (Facility,Customer) pairs
                var x = prob.AddVariables(facilities, customers)
                            .WithName((f, c) => String.Format("x[{0},{1}]", f.name, c.name))
                            .ToMap();

                // for each customer c
                //    sum(f=1..m) x[f,c] = d
                prob.AddConstraints(customers,
                    c => Sum(facilities, (Facility f) => x[f, c]).Eq(c.demand)
                    );

                // for each facility f
                //    sum(c=1..n) x[f,c] <= capacity[j] * y[f]
                prob.AddConstraints(facilities,
                    f => Sum(customers, (Customer c) => x[f, c]).Leq(y[f] * f.capacity)
                    );

                // minimize sum(j=1..m) cost[j] * y[j] +
                //          sum(i=1..n) sum(j=1..m) cost[f,c] * x[f,c]
                prob.SetObjective(Sum(Sum(facilities, (Facility f) => y[f] * f.cost),
                                      Sum(customers,
                                          (Customer c) => Sum(facilities, (Facility f) => x[f, c] * transportCost[f, c]))));


                prob.WriteProb("facilitylocationcollection.lp", "l");

                prob.Optimize("");
                double[] sol = prob.GetSolution();
                foreach (var f in facilities)
                {
                    if (y[f].GetValue(sol) > 0.5)
                    {
                        Console.WriteLine("Facility " + f.name + " is open, serves");
                        foreach (var c in customers)
                        {
                            if (x[f, c].GetValue(sol) > 0.0)
                                Console.WriteLine("  " + c.name + ": " +
                                                   x[f, c].GetValue(sol));
                        }
                    }
                }
            }
        }
    }
}

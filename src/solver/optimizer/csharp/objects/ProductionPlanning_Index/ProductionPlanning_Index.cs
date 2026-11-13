// (c) 2023-2026 Fair Isaac Corporation

using System;
using System.Linq;
using System.Collections.Generic;
using Optimizer.Maps;
using Optimizer.Objects;
using ColumnType = Optimizer.ColumnType;
using static Optimizer.Objects.Utils;
using static Optimizer.XPRSprob;
using static Optimizer.Objects.ConstantExpression;

namespace XpressExamples
{
    /// <summary>A production planning example.</summary>
    /// <remarks>
    /// <p>
    ///   There is a set F of factories at which products can be produced from
    ///   raw materials.
    /// </p>
    /// <p>
    ///   Products are sold (at a certain price) out of the factories.
    ///   Products can be produced (at a certain cost) at the factories from raw
    ///   materials. Each product has raw material requirements.
    ///   Raw materials can be bought at the factories.
    ///   Products and raw materials can be stocked.
    ///   There is an initial stock of products and raw materials.
    ///   A factory can produce only if it is open. It can buy raw material or
    ///   sell products even it it is closed.
    /// </p>
    ///
    /// <p>
    ///   <b>Objective</b>
    ///   Maximize the total profit which comprises of
    ///   <list type='bullet'>
    ///     <item><description>
    ///   the revenue from selling products,
    ///     </description></item>
    ///     <item><description>
    ///   the production cost,
    ///     </description></item>
    ///     <item><description>
    ///   the cost for keeping products on stock,
    ///     </description></item>
    ///     <item><description>
    ///   the cost for keeping raw materials on stock,
    ///     </description></item>
    ///     <item><description>
    ///   the cost for buying raw material,
    ///     </description></item>
    ///     <item><description>
    ///   the cost for keeping a factory open
    ///     </description></item>
    ///   </list>
    /// </p>
    /// </remarks>
    public class ProductionPlanning_Index
    {
        const int PMAX = 2; // number of products
        const int FMAX = 2; // number of factories
        const int RMAX = 2; // number of raw material
        const int TMAX = 4; // number of time periods


        const double CPSTOCK = 2.0; // unit cost to store a product
        const double CRSTOCK = 2.0; // unit cost to store a raw material
        const double MAXRSTOCK = 300; // maximum number of raw material that can be stocked in a factory

        static readonly double[,] REV = // REV[p, t] equals unit price for selling product p in period t
            new double[PMAX, TMAX] {
                {400, 380, 405, 350},
                {410, 397, 412, 397}};

        static readonly double[,] CMAK = // CMAK[p, f] unit cost for producing product p at factory f
            new double[PMAX, FMAX] {
                {150, 153},
                { 75,  68}};

        static readonly double[,] CBUY = // CBUY[r, t] unit cost to buy raw material r in period t
          new double[RMAX, TMAX] {
                {100,  98,  97, 100},
                {200, 195, 198, 200}};

        static readonly double[] COPEN = // COPEN[f] fixed cost for factory f being open for one period
          new double[FMAX] { 50000, 63000 };

        static readonly double[,] REQ = // REQ[p, r]      raw material requirement (in units) of r to make one unit of p
          new double[PMAX, RMAX] {
                {1.0, 0.5},
                {1.3, 0.4}};

        static readonly double[,] MAXSELL = // MAXSELL[p, t]   maximum number of units that can be sold of product p in period t
          new double[PMAX, TMAX] {
                {650, 600, 500, 400},
                {600, 500, 300, 250}};

        static readonly double[] MAXMAKE = // MAXMAKE[f]   maximum number of units (over all products) a factory can produce per period
          new double[FMAX] { 400, 500 };

        static readonly double[,] PSTOCK0 = // PSTOCK0[p, f]  initial stock of product p at factory f
            new double[PMAX, FMAX] {
                {50, 100},
                {50,  50}};

        static readonly double[,] RSTOCK0 = // RSTOCK0[r, f]  initial stock of raw material r at factor f
            new double[RMAX, FMAX] {
                {100, 150},
                { 50, 100}};

        public static void Main(string[] args)
        {
            Console.WriteLine("Formulating the production planning problem");

            using (XpressProblem prob = new XpressProblem())
            {
                // make[p, f, t]: Amount of product p to make at factory f in period t
                Variable[,,] make = prob.AddVariables(PMAX, FMAX, TMAX)
                  .WithName("make_p{0}_f{1}_t{2}")
                  .ToArray();

                // sell[p, f, t]: Amount of product p to sell from factory f in period t
                Variable[,,] sell = prob.AddVariables(PMAX, FMAX, TMAX)
                  .WithName("sell_p{0}_f{1}_t{2}")
                  .ToArray();

                // pstock[p, f, t]: Stock level of product p at factor f at start of period t
                Variable[,,] pstock = prob.AddVariables(PMAX, FMAX, TMAX)
                  .WithName("pstock_p{0}_f{1}_t{2}")
                  .ToArray();

                // buy[r, f, t]: Amount of raw material r bought for factory f in period t
                Variable[,,] buy = prob.AddVariables(RMAX, FMAX, TMAX)
                  .WithName("buy_r{0}_f{1}_t{2}")
                  .ToArray();

                // rstock[r, f, t]: Stock level of raw material r at factory f at start of period t
                Variable[,,] rstock = prob.AddVariables(RMAX, FMAX, TMAX)
                  .WithName("rstock_r{0}_f{1}_t{2}")
                  .ToArray();

                // openm[f, t]: If factory f is open in period t
                Variable[,] openm = prob.AddVariables(FMAX, TMAX)
                  .WithType(ColumnType.Binary)
                  .WithUB(1)
                  .WithName("openm_f{0}_t{1}")
                  .ToArray();



                // ## Objective:
                // Maximize total profit
                // revenue from selling products
                // + REV[p, t] * sell[p, f, t]
                LinExpression revenue = LinExpression.Create();
                foreach (Index idx in LoopThrough3D(PMAX, FMAX, TMAX))
                {
                    int p = idx.i1, f = idx.i2, t = idx.i3;
                    revenue.AddTerm(REV[p, t] * sell[p, f, t]);
                }

                // cost for making products (must be subtracted from profit)
                // - CMAK[p, f] * make[p, f, t]
                LinExpression prodCost = LinExpression.Create();
                foreach (Index idx in LoopThrough3D(PMAX, FMAX, TMAX))
                {
                    int p = idx.i1, f = idx.i2, t = idx.i3;
                    prodCost.AddTerm(-CMAK[p, f] * make[p, f, t]);
                }

                // cost for storing products (must be subtracted from profit)
                // - CPSTOCK * pstock[p, f, t]
                LinExpression prodStorageCost = LinExpression.Create();
                foreach (Index idx in LoopThrough3D(PMAX, FMAX, TMAX))
                {
                    int p = idx.i1, f = idx.i2, t = idx.i3;
                    prodStorageCost.AddTerm(-CPSTOCK * pstock[p, f, t]);
                }


                // cost for opening a factory in a time period
                // - openm[f, t] * COPEN[f]
                LinExpression factoryCost = LinExpression.Create();
                foreach (Index idx in LoopThrough2D(FMAX, TMAX))
                {
                    int f = idx.i1, t = idx.i2;
                    factoryCost.AddTerm(-COPEN[f] * openm[f, t]);
                }

                // cost for buying raw material in time period t
                // - buy[r, f, t] * CBUY[r, t]
                LinExpression rawMaterialBuyCost = LinExpression.Create();
                foreach (Index idx in LoopThrough3D(RMAX, FMAX, TMAX))
                {
                    int r = idx.i1, f = idx.i2, t = idx.i3;
                    rawMaterialBuyCost.AddTerm(-CBUY[r, t] * buy[r, f, t]);
                }

                // cost for storing raw material (must be subtracted from profit)
                // - rstock[r, f, t] * CRSTOCK
                // an alternative way of setting an objective Expression is the below nested sum expression
                // this construction does not use an index loop
                Expression rawMaterialStorageCost = Sum(FMAX, RMAX, TMAX,
                  (f, r, t) => rstock[r, f, t] * -CRSTOCK
                );

                // sum up the 6 individual contributions to the overall profit
                Expression profit = Sum(revenue, prodCost, prodStorageCost, factoryCost, rawMaterialStorageCost, rawMaterialBuyCost);

                // set maximization of profit as objective function
                prob.SetObjective(profit, Optimizer.ObjSense.Maximize);

                // constraints
                // Product stock balance
                foreach (Index idx in LoopThrough3D(PMAX, FMAX, TMAX))
                {
                    int p = idx.i1, f = idx.i2, t = idx.i3;

                    // for each time period except the last time period, surplus is available as stock for the next time period
                    if (t < TMAX - 1)
                    {
                        prob.AddConstraint(
                          pstock[p, f, t] + make[p, f, t] == sell[p, f, t] + pstock[p, f, t + 1])
                          .SetName($"prod_stock_balance_p{p}_f{f}_t{t}");
                    }
                    else
                    {
                        prob.AddConstraint(
                          pstock[p, f, t] + make[p, f, t] >= sell[p, f, t])
                          .SetName($"prod_stock_balance_p{p}_f{f}_t{t}");
                    }
                }

                // Raw material stock balance
                foreach (Index idx in LoopThrough3D(RMAX, FMAX, TMAX))
                {
                    int r = idx.i1, f = idx.i2, t = idx.i3;
                    if (t < TMAX - 1)
                    {
                        prob.AddConstraint(
                          rstock[r, f, t] + buy[r, f, t] == rstock[r, f, t + 1] + Sum(PMAX, p => make[p, f, t] * REQ[p, r]))
                          .SetName($"raw_material_stock_balance_r{r}_f{f}_t{t}");
                    }
                    else
                    {
                        prob.AddConstraint(
                          rstock[r, f, t] + buy[r, f, t] >= Sum(PMAX, p => make[p, f, t] * REQ[p, r]))
                          .SetName($"raw_material_stock_balance_r{r}_f{f}_t{t}");
                    }
                }

                // Limit on the amount of product p to be sold
                // exemplifies how to loop through multiple ranges
                prob.AddConstraints(PMAX, TMAX,
                  (p, t) =>
                    (Sum(FMAX, f => sell[p, f, t]) <= MAXSELL[p, t])
                    .SetName($"maxsell_p{p}_t{t}")
                );


                // Capacity limit at factory f
                // exemplifies how to loop through multiple ranges
                prob.AddConstraints(FMAX, TMAX,
                  (f, t) =>
                    (Sum(PMAX, p => make[p, f, t]) <= openm[f, t].Mul(MAXMAKE[f]))
                    .SetName($"capacity_f{f}_t{t}")
                );

                // Raw material stock limit
                // exemplifies how to loop through multiple ranges
                prob.AddConstraints(FMAX, TMAX,
                  (f, t) =>
                    (Sum(RMAX, r => rstock[r, f, t]) <= MAXRSTOCK)
                    .SetName($"raw_material_stock_limit_f{f}_t{t}")
                );

                // Initial product storage
                // loop through a custom IEnumerable, in our case our custom index object.
                prob.AddConstraints(LoopThrough2D(PMAX, FMAX),
                  idx =>
                    // pstock is indexed (p, f, t), PSTOCK0 is indexed (p, f)
                    (pstock[idx.i1, idx.i2, 0] == PSTOCK0[idx.i1, idx.i2])
                    .SetName($"initial_product_stock_p{idx.i1}_f{idx.i2}")
                );

                // Initial raw material storage
                // classic for loop
                foreach (Index idx in LoopThrough2D(RMAX, FMAX))
                {
                    int r = idx.i1, f = idx.i2;
                    prob.AddConstraint((rstock[r, f, 0] == RSTOCK0[r, f])
                    .SetName($"initial_raw_material_stock_r{r}_f{f}"));
                }

                // write the problem in LP format for manual inspection
                Console.WriteLine("Writing the problem to 'ProductionPlanning.lp'");
                prob.WriteProb("ProductionPlanning.lp", "l");

                // Solve the problem
                Console.WriteLine("Solving the problem");
                prob.Optimize();

                Console.WriteLine("Problem finished with SolStatus {0}", prob.SolStatus);

                if (prob.SolStatus != Optimizer.SolStatus.Optimal)
                {
                    throw new Exception("Problem not solved to optimality");
                }

                Console.WriteLine("Solution has objective value (profit) of {0}", prob.ObjVal);

                Console.WriteLine("");
                Console.WriteLine("*** Solution ***");
                double[] sol = prob.GetSolution();

                // Is factory f open at time period t?
                for (int f = 0; f < FMAX; f++)
                {
                    for (int t = 0; t < TMAX; t++)
                    {
                        Console.WriteLine($"Factory {f}\tTime Period {t} : Open = {openm[f, t].GetValue(sol)}");
                    }
                }
                Console.WriteLine("");

                // Production plan for producing
                WriteSol3D(sol, make, PMAX, FMAX, TMAX, new string[] { "Product", "Factory", "Time Period" }, "Make");
                Console.WriteLine("");

                // Production plan for selling products
                WriteSol3D(sol, sell, PMAX, FMAX, TMAX, new string[] { "Product", "Factory", "Time Period" }, "Sell");
                Console.WriteLine("");

                // Production plan for keeping products in stock
                WriteSol3D(sol, pstock, PMAX, FMAX, TMAX, new string[] { "Product", "Factory", "Time Period" }, "Pstock");
                Console.WriteLine("");

                // Production plan for keeping raw material in stock
                WriteSol3D(sol, rstock, RMAX, FMAX, TMAX, new string[] { "Material", "Factory", "Time Period" }, "Rstock");
                Console.WriteLine("");

                // Buying plan for raw material
                WriteSol3D(sol, buy, RMAX, FMAX, TMAX, new string[] { "Material", "Factory", "Time Period" }, "Buy");
                Console.WriteLine("");
            }
        }

        /// <summary>Helper class for convenient index iteration</summary>
        public sealed class Index
        {
            public readonly int i1;
            public readonly int i2;
            public readonly int i3;

            public Index(int i1, int i2, int i3)
            {
                this.i1 = i1;
                this.i2 = i2;
                this.i3 = i3;
            }
            public override string ToString() { return $"Index {this.i1} {this.i2} {this.i3}"; }
        }
        /// <summary>
        /// Loop through a 3-dimensional range, always starting at 0.
        /// </summary>
        /// <param name="max1">First index varies between 0 and max1</param>
        /// <param name="max2">Second index varies between 0 and max2</param>
        /// <param name="max3">Third index varies between 0 and max3</param>
        /// <returns>an Enumerable that can be used to loop over the cross product of the three ranges</returns>
        public static IEnumerable<Index> LoopThrough3D(int max1, int max2, int max3)
        {
            for (int i1 = 0; i1 < max1; i1++)
            {
                for (int i2 = 0; i2 < max2; i2++)
                {
                    if (max3 > 0)
                    {
                        for (int i3 = 0; i3 < max3; i3++)
                        {
                            Index idx = new Index(i1, i2, i3);
                            yield return idx;
                        }
                    }
                    else
                    {
                        yield return new Index(i1, i2, -1);
                    }
                }
            }
        }
        /// <summary>
        /// Loop through a 2-dimensional range, starting at 0
        /// </summary>
        /// <param name="max1">First index varies between 0 and max1</param>
        /// <param name="max2">Second index varies between 0 and max2</param>
        /// <returns>an Enumerable that can be used to loop over the cross product of the two ranges. The third
        /// entry of the returned Index object will always be -1</returns>
        public static IEnumerable<Index> LoopThrough2D(int max1, int max2)
        {
            return LoopThrough3D(max1, max2, 0);
        }

        /// <summary>
        /// Convenience function for printing solution values stored in a 3-dimensional Variable array
        /// </summary>
        /// <param name="sol">solution object, obtained via prob.getSolution()</param>
        /// <param name="array">3-dimensional array of Xpress Variables</param>
        /// <param name="max1">First index varies between 0 and max1</param>
        /// <param name="max2">Second index varies between 0 and max2</param>
        /// <param name="max3">Third index varies between 0 and max3</param>
        /// <param name="dimNames">An array with a name for every dimension</param>
        /// <param name="name">The name of the array</param>
        public static void WriteSol3D(double[] sol, Variable[,,] array, int max1, int max2, int max3, string[] dimNames, string name)
        {
            foreach (Index idx in LoopThrough3D(max1, max2, max3))
            {
                int i1 = idx.i1, i2 = idx.i2, i3 = idx.i3;
                Console.WriteLine($"{dimNames[0]} {i1}\t{dimNames[1]} {i2}\t{dimNames[2]} {i3} : {name} = {array[i1, i2, i3].GetValue(sol)}");
            }
        }

    }
}

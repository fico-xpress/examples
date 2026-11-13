// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import java.util.Locale;
import java.util.function.Function;
import java.util.stream.IntStream;
import java.util.stream.Stream;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Expression;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/** Production planning problem. */
public class ProductionPlanning_Index {
    static final int PMAX = 2; // number of products
    static final int FMAX = 2; // number of factories
    static final int RMAX = 2; // number of raw material
    static final int TMAX = 4; // number of time periods

    static final double CPSTOCK = 2.0; // unit cost to store a product
    static final double CRSTOCK = 2.0; // unit cost to store a raw material
    static final double MAXRSTOCK = 300; // maximum number of raw material that can be stocked in a factory

    static final double[][] REV = // REV[p][t] equals unit price for selling product p in period t
            { { 400, 380, 405, 350 }, { 410, 397, 412, 397 } };

    static final double[][] CMAK = // CMAK[p][f] unit cost for producing product p at factory f
            { { 150, 153 }, { 75, 68 } };

    static final double[][] CBUY = // CBUY[r][t] unit cost to buy raw material r in period t
            { { 100, 98, 97, 100 }, { 200, 195, 198, 200 } };

    static final double[] COPEN = // COPEN[f] fixed cost for factory f being open for one period
            { 50000, 63000 };

    static final double[][] REQ = // REQ[p][r] raw material requirement (in units) of r to make one unit of p
            { { 1.0, 0.5 }, { 1.3, 0.4 } };

    static final double[][] MAXSELL = // MAXSELL[p][t] maximum number of units that can be sold of product p in period
                                      // t
            { { 650, 600, 500, 400 }, { 600, 500, 300, 250 } };

    static final double[] MAXMAKE = // MAXMAKE[f] maximum number of units (over all products) a factory can produce
                                    // per period
            { 400, 500 };

    static final double[][] PSTOCK0 = // PSTOCK0[p][f] initial stock of product p at factory f
            { { 50, 100 }, { 50, 50 } };

    static final double[][] RSTOCK0 = // RSTOCK0[r][f] initial stock of raw material r at factor f
            { { 100, 150 }, { 50, 100 } };

    public static class Index2 {
        public final int i1;
        public final int i2;

        public Index2(int i1, int i2) {
            this.i1 = i1;
            this.i2 = i2;
        }

        @Override
        public String toString() {
            return String.format("Index %d %d", i1, i2);
        }
    }

    /** Helper class for convenient index iteration */
    public static class Index3 {
        public final int i1;
        public final int i2;
        public final int i3;

        public Index3(int i1, int i2, int i3) {
            this.i1 = i1;
            this.i2 = i2;
            this.i3 = i3;
        }

        @Override
        public String toString() {
            return String.format("Index %d %d %d", i1, i2, i3);
        }
    }

    /**
     * Loop through a 2-dimensional range, starting at 0
     *
     * @param max1 First index varies between 0 and max1
     * @param max2 Second index varies between 0 and max2
     * @return a stream that can be used to loop over the cross product of the two
     *         ranges.
     */
    public static Stream<Index2> loopThrough2D(int max1, int max2) {
        return IntStream.range(0, max1)
                .mapToObj(i1 -> Stream.iterate(new Index2(i1, 0), ind -> new Index2(ind.i1, ind.i2 + 1)).limit(max2))
                .flatMap(Function.identity());

    }

    /**
     * Loop through a 3-dimensional range, starting at 0
     *
     * @param max1 First index varies between 0 and max1
     * @param max2 Second index varies between 0 and max2
     * @param max3 Third index varies between 0 and max2
     * @return a stream that can be used to loop over the cross product of the three
     *         ranges.
     */
    public static Stream<Index3> loopThrough3D(int max1, int max2, int max3) {
        return loopThrough2D(max1, max2).flatMap(ind2 -> Stream
                .iterate(new Index3(ind2.i1, ind2.i2, 0), ind3 -> new Index3(ind3.i1, ind3.i2, ind3.i3 + 1))
                .limit(max3));
    }

    /**
     * Convenience function for printing solution values stored in a 3-dimensional
     * Variable array
     *
     * @param sol      solution object, obtained via prob.getSolution()
     * @param array    3-dimensional array of Xpress Variables
     * @param max1     First index varies between 0 and max1
     * @param max2     Second index varies between 0 and max2
     * @param max3     Third index varies between 0 and max3
     * @param dimNames An array with a name for every dimension
     * @param name     The name of the array
     */
    public static void writeSol3D(double[] sol, Variable[][][] array, int max1, int max2, int max3, String[] dimNames,
            String name) {
        loopThrough3D(max1, max2, max3).forEach(idx -> {
            int i1 = idx.i1, i2 = idx.i2, i3 = idx.i3;
            System.out.printf(Locale.US, "%s %d\t%s %d\t%s %d : %s = %g%n", dimNames[0], i1, dimNames[1], i2,
                    dimNames[2], i3, name, array[i1][i2][i3].getValue(sol));
        });
    }

    public static void main(String[] args) {
        System.out.println("Formulating the production planning problem");

        try (XpressProblem prob = new XpressProblem()) {
            // make[p][f][t]: Amount of product p to make at factory f in period t
            Variable[][][] make = prob.addVariables(PMAX, FMAX, TMAX).withName("make_p%d_f%d_t%d").toArray();

            // sell[p][f][t]: Amount of product p to sell from factory f in period t
            Variable[][][] sell = prob.addVariables(PMAX, FMAX, TMAX).withName("sell_p%d_f%d_t%d").toArray();

            // pstock[p][f][t]: Stock level of product p at factor f at start of period t
            Variable[][][] pstock = prob.addVariables(PMAX, FMAX, TMAX).withName("pstock_p%d_f%d_t%d").toArray();

            // buy[r][f][t]: Amount of raw material r bought for factory f in period t
            Variable[][][] buy = prob.addVariables(RMAX, FMAX, TMAX).withName("buy_p%d_f%d_t%d").toArray();
            // rstock[r][f][t]: Stock level of raw material r at factory f at start of
            // period t
            Variable[][][] rstock = prob.addVariables(RMAX, FMAX, TMAX).withName("rstock_p%d_f%d_t%d").toArray();

            // openm[f][t]: If factory f is open in period t
            Variable[][] openm = prob.addVariables(FMAX, TMAX).withType(ColumnType.Binary).withUB(1)
                    .withName("openm_f%d_t%d").toArray();

            // ## Objective:
            // Maximize total profit
            // revenue from selling products
            // + REV[p][t] * sell[p][f][t]
            LinExpression revenue = LinExpression.create();

            loopThrough3D(PMAX, FMAX, TMAX).forEach(idx -> {
                int p = idx.i1, f = idx.i2, t = idx.i3;
                revenue.addTerm(sell[p][f][t].mul(REV[p][t]));
            });

            // cost for making products (must be subtracted from profit)
            // - CMAK[p, f] * make[p][f][t]
            LinExpression prodCost = LinExpression.create();

            loopThrough3D(PMAX, FMAX, TMAX).forEach(idx -> {
                int p = idx.i1, f = idx.i2, t = idx.i3;
                prodCost.addTerm(make[p][f][t].mul(-CMAK[p][f]));
            });

            // cost for storing products (must be subtracted from profit)
            // - CPSTOCK * pstock[p][f][t]
            LinExpression prodStorageCost = LinExpression.create();

            loopThrough3D(PMAX, FMAX, TMAX).forEach(idx -> {
                int p = idx.i1, f = idx.i2, t = idx.i3;
                prodStorageCost.addTerm(pstock[p][f][t].mul(-CPSTOCK));
            });

            // cost for opening a factory in a time period
            // - openm[f][t] * COPEN[f]
            LinExpression factoryCost = LinExpression.create();

            loopThrough2D(FMAX, TMAX).forEach(idx -> {
                int f = idx.i1, t = idx.i2;
                factoryCost.addTerm(openm[f][t].mul(-COPEN[f]));
            });

            // cost for buying raw material in time period t
            // - buy[r][f][t] * CBUY[r, t]
            LinExpression rawMaterialBuyCost = LinExpression.create();

            loopThrough3D(PMAX, FMAX, TMAX).forEach(idx -> {
                int r = idx.i1, f = idx.i2, t = idx.i3;
                rawMaterialBuyCost.addTerm(buy[r][f][t].mul(-CBUY[r][t]));
            });

            // cost for storing raw material (must be subtracted from profit)
            // - rstock[r][f][t] * CRSTOCK
            // an alternative way of setting an objective Expression is to pass
            // the stream directly to the sum function
            Expression rawMaterialStorageCost = sum(FMAX, RMAX, TMAX,
                    (f, r, t) -> rstock[r][f][t].mul(-CRSTOCK));

            // sum up the 6 individual contributions to the overall profit
            Expression profit = sum(revenue, prodCost, prodStorageCost, factoryCost, rawMaterialStorageCost,
                    rawMaterialBuyCost);

            // set maximization of profit as objective function
            prob.setObjective(profit, XPRSenumerations.ObjSense.MAXIMIZE);

            // constraints
            // Product stock balance
            loopThrough3D(PMAX, FMAX, TMAX).forEach(idx -> {
                int p = idx.i1, f = idx.i2, t = idx.i3;

                // for each time period except the last time period, surplus is available as
                // stock for the next time period
                if (t < TMAX - 1) {
                    prob.addConstraint(pstock[p][f][t].plus(make[p][f][t]).eq(sell[p][f][t].plus(pstock[p][f][t + 1]))
                            .setName(String.format("prod_stock_balance_p%d_f%d_t%d", p, f, t)));
                } else {
                    prob.addConstraint(pstock[p][f][t].plus(make[p][f][t]).geq(sell[p][f][t])
                            .setName(String.format("prod_stock_balance_p%d_f%d_t%d", p, f, t)));
                }
            });

            // Raw material stock balance
            loopThrough3D(PMAX, FMAX, TMAX).forEach(idx -> {
                int r = idx.i1, f = idx.i2, t = idx.i3;
                if (t < TMAX - 1) {
                    prob.addConstraint(rstock[r][f][t].plus(buy[r][f][t])
                            .eq(rstock[r][f][t + 1].plus(sum(PMAX, p -> make[p][f][t].mul(REQ[p][r]))))
                            .setName(String.format("raw_material_stock_balance_r%d_f%d_t%d", r, f, t)));
                } else {
                    prob.addConstraint(
                            rstock[r][f][t].plus(buy[r][f][t]).geq(sum(PMAX, p -> make[p][f][t].mul(REQ[p][r])))
                                    .setName(String.format("raw_material_stock_balance_r%d_f%d_t%d", r, f, t)));
                }
            });

            // Limit on the amount of product p to be sold
            // exemplifies how to loop through multiple ranges
            prob.addConstraints(PMAX, TMAX, (p, t) -> (sum(FMAX, f -> sell[p][f][t]).leq(MAXSELL[p][t]))
                    .setName(String.format("maxsell_p%d_t%d", p, t)));

            // Capacity limit at factory f
            // exemplifies how to loop through multiple ranges
            prob.addConstraints(FMAX, TMAX, (f, t) -> sum(PMAX, p -> make[p][f][t]).leq(openm[f][t].mul(MAXMAKE[f]))
                    .setName(String.format("capacity_f%d_t%d", f, t)));

            // Raw material stock limit
            // exemplifies how to loop through multiple ranges
            prob.addConstraints(FMAX, TMAX, (f, t) -> sum(RMAX, r -> rstock[r][f][t]).leq(MAXRSTOCK)
                    .setName(String.format("raw_material_stock_limit_f%d_t%d", f, t)));

            // Initial product storage
            // loop through a custom IEnumerable, in our case our custom index object.
            prob.addConstraints(loopThrough2D(PMAX, FMAX), idx ->
            // pstock is indexed (p, f, t), PSTOCK0 is indexed (p, f)
            (pstock[idx.i1][idx.i2][0].eq(PSTOCK0[idx.i1][idx.i2]))
                    .setName(String.format("initial_product_stock_p%d_f%d", idx.i1, idx.i2)));

            // Initial raw material storage
            // classic for loop
            loopThrough2D(PMAX, FMAX).forEach(idx -> {
                int r = idx.i1, f = idx.i2;
                prob.addConstraint((rstock[r][f][0].eq(RSTOCK0[r][f]))
                        .setName(String.format("initial_raw_material_stock_r%d_f%d", r, f)));
            });

            // write the problem in LP format for manual inspection
            System.out.println("Writing the problem to 'ProductionPlanning.lp'");
            prob.writeProb("ProductionPlanning.lp", "l");

            // Solve the problem
            System.out.println("Solving the problem");
            prob.optimize();

            System.out.println("Problem finished with SolStatus " + prob.attributes().getSolStatus());

            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL) {
                throw new RuntimeException("Problem not solved to optimality");
            }

            System.out.println("Solution has objective value (profit) of " + prob.attributes().getObjVal());

            System.out.println("");
            System.out.println("*** Solution ***");
            double[] sol = prob.getSolution();

            // Is factory f open at time period t?
            for (int f = 0; f < FMAX; f++) {
                for (int t = 0; t < TMAX; t++) {
                    System.out.printf(Locale.US, "Factory %d\tTime Period %d : Open = %g%n", f, t,
                            openm[f][t].getValue(sol));
                }
            }
            System.out.println("");

            // Production plan for producing
            writeSol3D(sol, make, PMAX, FMAX, TMAX, new String[] { "Product", "Factory", "Time Period" }, "Make");
            System.out.println("");

            // Production plan for selling products
            writeSol3D(sol, sell, PMAX, FMAX, TMAX, new String[] { "Product", "Factory", "Time Period" }, "Sell");
            System.out.println("");

            // Production plan for keeping products in stock
            writeSol3D(sol, pstock, PMAX, FMAX, TMAX, new String[] { "Product", "Factory", "Time Period" }, "Pstock");
            System.out.println("");

            // Production plan for keeping raw material in stock
            writeSol3D(sol, rstock, RMAX, FMAX, TMAX, new String[] { "Material", "Factory", "Time Period" }, "Rstock");
            System.out.println("");

            // Buying plan for raw material
            writeSol3D(sol, buy, RMAX, FMAX, TMAX, new String[] { "Material", "Factory", "Time Period" }, "Buy");
            System.out.println("");

        }
    }
}

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * This example demonstrates some modeling devices. We model a very simple
 * facility location problem: We have customers and facilities. The constraints
 * are:
 * - each customer must be served from exactly one facility
 * - customers can only be served from open facilities
 * - customer demand must be satisfied
 * - facility capacity must not be exceeded We minimize the sum of transport cost
 *   (between customer and facility) and the cost for opening a facility.
 * In this example data is kept in arrays.
 */
public class FacilityLocationArray {
    /** Customer description. */
    private static final class Customer {
        /** Name of customer. */
        public final String name;
        /** Demand for customer. */
        public final double demand;

        public Customer(String name, double demand) {
            this.demand = demand;
            this.name = name;
        }
    }

    /** Facility descriptor. */
    private static final class Facility {
        /** Name of facility. */
        public final String name;
        /** Capacity of facility. */
        public final double capacity;
        /** Cost for opening this facility. */
        public final double cost;

        public Facility(String name, double capacity, double cost) {
            this.name = name;
            this.capacity = capacity;
            this.cost = cost;
        }
    }

    /** Customers in this example. */
    private static final Customer[] customers = new Customer[] { new Customer("Customer 1", 80),
            new Customer("Customer 2", 270), new Customer("Customer 3", 250) };
    /** Facilities in this example. */
    private static final Facility[] facilities = new Facility[] { new Facility("Facility 1", 500, 1000),
            new Facility("Facility 2", 500, 1000), new Facility("Facility 3", 500, 1000) };
    /** Cost for transporting one unit between customer and facility. */
    private static final double[][] transportCost = new double[][] { new double[] { 4, 5, 6 }, new double[] { 6, 4, 3 },
            new double[] { 9, 7, 4 } };

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            Variable[] y = prob.addVariables(facilities.length).withType(ColumnType.Binary)
                    .withName(f -> facilities[f].name).toArray();
            Variable[][] x = prob.addVariables(facilities.length, customers.length).withLB(0.0)
                    .withUB(Double.POSITIVE_INFINITY)
                    .withName((f, c) -> String.format("x[%s,%s]", facilities[f].name, customers[c].name)).toArray();

            // for each customer c
            // sum(f=1..m) x[f,c] = d
            prob.addConstraints(customers.length, c -> sum(facilities.length, f -> x[f][c]).eq(customers[c].demand));

            // for each facility f
            // sum(c=1..n) x[f,c] <= capacity[j] * y[f]
            prob.addConstraints(facilities.length, f -> sum(x[f]).leq(y[f].mul(facilities[f].capacity)));

            // minimize sum(f=1..m) cost[f] * y[f] +
            // sum(c=1..n) sum(f=1..m) cost[f,c] * x[f,c]
            prob.setObjective(sum(sum(facilities.length, f -> y[f].mul(facilities[f].cost)),
                    sum(customers.length, c -> sum(facilities.length, f -> x[f][c].mul(transportCost[f][c])))));

            prob.writeProb("facilitylocationarray.lp", "l");

            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("failed to optimize with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();
            for (int f = 0; f < facilities.length; ++f) {
                if (y[f].getValue(sol) > 0.5) {
                    System.out.println("Facility " + facilities[f].name + " is open, serves");
                    for (int c = 0; c < customers.length; ++c) {
                        if (x[f][c].getValue(sol) > 0.0)
                            System.out.println("  " + customers[c].name + ": " + x[f][c].getValue(sol));
                    }
                }
            }
        }
    }
}

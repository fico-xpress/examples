// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;
import static java.util.Arrays.asList;

import java.util.List;
import java.util.Map;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.maps.HashMap2;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * This example demonstrates some modeling devices. We model a very simple
 * facility location problem: We have customers and facilities. The constraints
 * are: - each customer must be served from exactly one facility - customers can
 * only be served from open facilities - customer demand must be satisfied -
 * facility capacity must not be exceeded We minimize the sum of transport cost
 * (between customer and facility) and the cost for opening a facility. In this
 * example data is kept in collections.
 */
public class FacilityLocationCollection {
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

    private static final List<Customer> customers = asList(new Customer("Customer 1", 80),
            new Customer("Customer 2", 270), new Customer("Customer 3", 250));
    private static final List<Facility> facilities = asList(new Facility("Facility 1", 500, 1000),
            new Facility("Facility 2", 500, 1000), new Facility("Facility 3", 500, 1000));

    private static final HashMap2<Facility, Customer, Double> transportCost = new HashMap2<Facility, Customer, Double>();
    static {
        transportCost.put(facilities.get(0), customers.get(0), 4.0);
        transportCost.put(facilities.get(0), customers.get(1), 5.0);
        transportCost.put(facilities.get(0), customers.get(2), 6.0);
        transportCost.put(facilities.get(1), customers.get(0), 6.0);
        transportCost.put(facilities.get(1), customers.get(1), 4.0);
        transportCost.put(facilities.get(1), customers.get(2), 3.0);
        transportCost.put(facilities.get(2), customers.get(0), 9.0);
        transportCost.put(facilities.get(2), customers.get(1), 7.0);
        transportCost.put(facilities.get(2), customers.get(2), 4.0);
    }

    public static void main(String[] args) {
        try (XpressProblem prob = new XpressProblem()) {
            Map<Facility, Variable> y = prob.addVariables(facilities).withType(ColumnType.Binary).withName(f -> f.name)
                    .toMap();
            HashMap2<Facility, Customer, Variable> x = prob.addVariables(facilities, customers).withLB(0.0)
                    .withUB(Double.POSITIVE_INFINITY).withName((f, c) -> String.format("x[%s,%s]", f.name, c.name))
                    .toMap();

            // for each customer c
            // sum(f=1..m) x[f,c] = d
            prob.addConstraints(customers, c -> sum(facilities, f -> x.get(f, c)).eq(c.demand));

            // for each facility f
            // sum(c=1..n) x[f,c] <= capacity[j] * y[f]
            prob.addConstraints(facilities, f -> sum(customers, c -> x.get(f, c)).leq(y.get(f).mul(f.capacity)));

            // minimize sum(j=1..m) cost[j] * y[j] +
            // sum(i=1..n) sum(j=1..m) cost[f,c] * x[f,c]
            prob.setObjective(sum(sum(facilities, f -> y.get(f).mul(f.cost)),
                    sum(customers.stream(), c -> sum(facilities, f -> x.get(f, c).mul(transportCost.get(f, c))))));

            prob.writeProb("facilitylocationcollection.lp", "l");

            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL)
                throw new RuntimeException("failed to optimize with status " + prob.attributes().getSolStatus());
            double[] sol = prob.getSolution();
            for (Facility f : facilities) {
                if (y.get(f).getValue(sol) > 0.5) {
                    System.out.println("Facility " + f.name + " is open, serves");
                    for (Customer c : customers) {
                        if (x.get(f, c).getValue(sol) > 0.0)
                            System.out.println("  " + c.name + ": " + x.get(f, c).getValue(sol));
                    }
                }
            }
        }
    }
}

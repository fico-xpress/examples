// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

//These imports are only for the parser.
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.stream.Stream;

import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Purchasing problem with price breaks using PieceWise Linear constraints.
 * There are three suppliers of a good, and they have quoted various prices for
 * various quantities of product. We want to buy at least total cost, yet not
 * buy too much from any one supplier. Each supplier offers decreasing prices
 * for increased lot size, in the form of incremental discounts.
 */
public class PurchasePWL {

    static final String DATAFILE = System.getenv().getOrDefault("EXAMPLE_DATA_DIR", "../../data") + "/purchase.cdat";

    static final int NB = 4; /* Number of breakpoints */
    static double REQ; /* Total quantity required */
    static int[] SUPPLIERS; /* Suppliers */
    static double[] MAXPERC; /* Maximum percentages for each supplier */
    static double[][] COST; /* Unit cost */
    static double[][] BREAKP; /* Breakpoints (quantities at which unit cost changes) */

    public static void main(String[] args) throws IOException {
        readData(); // Read data from file

        try (XpressProblem prob = new XpressProblem()) {

            // Create the decision variables
            // Quantity to purchase from supplier s
            Variable[] buy = prob.addVariables(SUPPLIERS.length).withUB(i -> MAXPERC[i] * REQ / 100.0)
                    .withName("buy %d").toArray();

            // Cost incurred from supplier s
            Variable[] cost = prob.addVariables(SUPPLIERS.length).withName("cost %d").toArray();

            // The minimum quantity that must be bought
            prob.addConstraint(sum(buy).geq(REQ));

            // Function to calculate cost at breakpoints
            double[][] COSTBREAK = new double[SUPPLIERS.length][NB];
            for (int s = 0; s < SUPPLIERS.length; s++) {
                COSTBREAK[s][0] = 0;
                for (int b = 1; b < NB; b++)
                    COSTBREAK[s][b] = COSTBREAK[s][b - 1] + COST[s][b] * (BREAKP[s][b] - BREAKP[s][b - 1]);
            }

            // Define relation between bought quantities and price paid per supplier
            for (int s = 0; s < SUPPLIERS.length; s++) {
                prob.addConstraint(cost[s].pwlOf(buy[s], BREAKP[s], COSTBREAK[s]));
            }

            // Objective: sum of costs
            prob.setObjective(sum(cost));

            // Solve the problem
            prob.optimize();

            System.out.println("Problem status: " + prob.attributes().getMIPStatus());
            if (prob.attributes().getMIPStatus() != XPRSenumerations.MIPStatus.SOLUTION
                    && prob.attributes().getMIPStatus() != XPRSenumerations.MIPStatus.OPTIMAL)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getMIPStatus());

            // Solution printing
            System.out.println("Total cost: " + prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            for (int s = 0; s < SUPPLIERS.length; s++)
                System.out.println(
                        "Supp. " + SUPPLIERS[s] + ": buy:" + buy[s].getValue(sol) + ": cost:" + cost[s].getValue(sol));
        }
    }

    /**
     * Read a list of strings. Iterates <code>tokens</code> until a semicolon is
     * encountered or the iterator ends.
     *
     * @param tokens The token sequence to read.
     * @return A stream of all tokens before the first semicolon.
     */
    private static <T> Stream<String> readStrings(Iterator<String> tokens) {
        ArrayList<String> result = new ArrayList<String>();
        while (tokens.hasNext()) {
            String token = tokens.next();
            if (token.equals(";"))
                break;
            result.add(token);
        }
        return result.stream();
    }

    /**
     * Read a table of doubles. Allocates a <code>nrow</code> by <code>ncol</code>
     * double table and fills it by the sparse data from the token sequence.
     * <code>tokens</code> is assumed to hold <code>nrow</code> sequences of
     * indices, each of which is terminated by a semicolon. The indices in those
     * vectors specify the <code>true</code> entries in the corresponding row of the
     * table.
     *
     * @param tokens Token sequence.
     * @param nrow   Number of rows in the table.
     * @param ncol   Number of columns in the table.
     * @return The double table.
     */
    private static double[][] readTable(Iterator<String> tokens, int nrow, int ncol) throws IOException {
        double[][] tbl = new double[nrow][ncol];
        for (int r = 0; r < nrow; r++) {
            tbl[r] = readStrings(tokens).mapToDouble(Double::valueOf).toArray();
        }
        return tbl;
    }

    private static void readData() throws IOException {
        // Convert the input file into a sequence of tokens that are
        // separated by whitespace.
        Iterator<String> tokens = Files.lines(new File(DATAFILE).toPath()).map(s -> Arrays.stream(s.split("\\s+")))
                .flatMap(s -> s)
                // Split semicolon off its token.
                .map(s -> (s.length() > 0 && s.endsWith(";")) ? Stream.of(s.substring(0, s.length() - 1), ";")
                        : Stream.of(s))
                .flatMap(s -> s)
                // Remove empty tokens.
                .filter(s -> s.length() > 0).iterator();

        while (tokens.hasNext()) {
            String token = tokens.next();
            if (token.equals("SUPPLIERS:"))
                SUPPLIERS = readStrings(tokens).mapToInt(Integer::valueOf).toArray();
            else if (token.equals("MAXPERC:"))
                MAXPERC = readStrings(tokens).mapToDouble(Double::valueOf).toArray();
            else if (token.equals("REQ:"))
                REQ = Double.valueOf(tokens.next());
            else if (token.equals("BREAKP:")) {
                BREAKP = readTable(tokens, SUPPLIERS.length, NB);
            } else if (token.equals("COST:"))
                COST = readTable(tokens, SUPPLIERS.length, NB);
        }
    }
}

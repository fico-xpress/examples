// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

// These imports are only for the parser.
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.stream.Stream;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Modeling a MIP problem to perform portfolio optimization. <br>
 * There are a number of shares available in which to invest. The problem is to
 * split the available capital between these shares to maximize return on
 * investment, while satisfying certain constraints on the portfolio:
 * <ul>
 * <li>A maximum of 7 distinct shares can be invest in.</li>
 * <li>Each share has an associated industry sector and geographic region, and
 * the capital may not be overly concentrated in any one sector or region.</li>
 * <li>Some of the shares are considered to be high-risk, and the maximum
 * investment in these shares is limited.</li>
 * </ul>
 */
public class Folio {
    static final String DATAFILE = System.getenv().getOrDefault("EXAMPLE_DATA_DIR", "../../data") + "/folio10.cdat";

    static final int MAXNUM = 7; /* Max. number of different assets */
    static final double MAXRISK = 1.0 / 3; /* Max. investment into high-risk values */
    static final double MINREG = 0.2; /* Min. investment per geogr. region */
    static final double MAXREG = 0.5; /* Max. investment per geogr. region */
    static final double MAXSEC = 0.25; /* Max. investment per ind. sector */
    static final double MAXVAL = 0.2; /* Max. investment per share */
    static final double MINVAL = 0.1; /* Min. investment per share */

    static double[] RET; /* Estimated return in investment */
    static int[] RISK; /* High-risk values among shares */
    static boolean[][] LOC; /* Geogr. region of shares */
    static boolean[][] SEC; /* Industry sector of shares */

    static String[] SHARES;
    static String[] REGIONS;
    static String[] TYPES;

    public static void main(String[] args) throws IOException {
        readData(); // Read data from file

        try (XpressProblem prob = new XpressProblem()) {

            // Create the decision variables
            // Fraction of capital used per share
            Variable[] frac = prob.addVariables(SHARES.length).withUB(MAXVAL).withName("frac %d").toArray();
            // 1 if asset is in portfolio, 0 otherwise
            Variable[] buy = prob.addVariables(SHARES.length).withType(ColumnType.Binary).withName("buy %d").toArray();

            // Objective: total return
            prob.setObjective(scalarProduct(frac, RET), XPRSenumerations.ObjSense.MAXIMIZE);

            // Limit the percentage of high-risk values
            prob.addConstraint(sum(RISK, v -> frac[v]).leq(MAXRISK));

            // Limits on geographical distribution
            prob.addConstraints(REGIONS.length,
                    r -> sum(range(0, SHARES.length).filter(s -> LOC[r][s]).mapToObj(s -> frac[s])).in(MINREG, MAXREG));

            // Diversification across industry sectors
            prob.addConstraints(TYPES.length,
                    t -> sum(range(0, SHARES.length).filter(s -> SEC[t][s]).mapToObj(s -> frac[s])).leq(MAXSEC));

            // Spend all the capital
            prob.addConstraint(sum(frac).eq(1));

            // Limit the total number of assets
            prob.addConstraint(sum(buy).leq(MAXNUM));

            // Linking the variables
            for (int s = 0; s < SHARES.length; s++) {
                prob.addConstraint(frac[s].leq(buy[s].mul(MAXVAL)));
                prob.addConstraint(frac[s].geq(buy[s].mul(MINVAL)));
            }

            // Set a time limit of 10 seconds
            prob.controls().setTimeLimit(10.0);

            // Solve the problem
            prob.optimize("");

            System.out.println("Problem status: " + prob.attributes().getMIPStatus());
            if (prob.attributes().getMIPStatus() != XPRSenumerations.MIPStatus.SOLUTION
                    && prob.attributes().getMIPStatus() != XPRSenumerations.MIPStatus.OPTIMAL)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getMIPStatus());

            // Solution printing
            System.out.println("Total return: " + prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            for (int s = 0; s < SHARES.length; s++)
                if (buy[s].getValue(sol) > 0.5)
                    System.out.println(
                            "  " + s + ": " + frac[s].getValue(sol) * 100 + "% (" + buy[s].getValue(sol) + ")");
// ifndef RELEASE
            assert Math.abs(prob.attributes().getObjVal() - 21.4193) <= 1e-4;
// #endif
        }
    }

    /**
     * Read a list of strings. Iterates <code>tokens</code> until a semicolon is
     * encountered or the iterator ends.
     *
     * @param tokens The token sequence to read.
     * @return A stream of all tokens before the first semiconlon.
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
     * Read a sparse table of booleans. Allocates a <code>nrow</code> by
     * <code>ncol</code> boolean table and fills it by the sparse data from the
     * token sequence. <code>tokens</code> is assumed to hold <code>nrow</code>
     * sequences of indices, each of which is terminated by a semicolon. The indices
     * in those vectors specify the <code>true</code> entries in the corresponding
     * row of the table.
     *
     * @param tokens Token sequence.
     * @param nrow   Number of rows in the table.
     * @param ncol   Number of columns in the table.
     * @return The boolean table.
     */
    private static boolean[][] readBoolTable(Iterator<String> tokens, int nrow, int ncol) throws IOException {
        boolean[][] tbl = new boolean[nrow][ncol];

        for (int r = 0; r < nrow; r++) {
            while (tokens.hasNext()) {
                String token = tokens.next();
                if (token.equals(";"))
                    break;
                tbl[r][Integer.valueOf(token)] = true;
            }
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
            if (token.equals("SHARES:"))
                SHARES = readStrings(tokens).toArray(String[]::new);
            else if (token.equals("REGIONS:"))
                REGIONS = readStrings(tokens).toArray(String[]::new);
            else if (token.equals("TYPES:"))
                TYPES = readStrings(tokens).toArray(String[]::new);
            else if (token.equals("RISK:"))
                RISK = readStrings(tokens).mapToInt(Integer::valueOf).toArray();
            else if (token.equals("RET:"))
                RET = readStrings(tokens).mapToDouble(Double::valueOf).toArray();
            else if (token.equals("LOC:"))
                LOC = readBoolTable(tokens, REGIONS.length, SHARES.length);
            else if (token.equals("SEC:"))
                SEC = readBoolTable(tokens, TYPES.length, SHARES.length);
        }
    }
}

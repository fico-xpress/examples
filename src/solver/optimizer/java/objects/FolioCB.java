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
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Modeling a MIP problem to perform portfolio optimization. -- Defining an
 * integer solution callback --
 */
public class FolioCB {
    /* Path to Data file */
    private static final String DATAFILE = System.getenv().getOrDefault("EXAMPLE_DATA_DIR", "../../data")
            + "/folio10.cdat";
    private static final int MAXNUM = 15; /* Max. number of different assets */
    private static final double MAXRISK = 1.0 / 3; /* Max. investment into high-risk values */
    private static final double MINREG = 0.2; /* Min. investment per geogr. region */
    private static final double MAXREG = 0.5; /* Max. investment per geogr. region */
    private static final double MAXSEC = 0.25; /* Max. investment per ind. sector */
    private static final double MAXVAL = 0.2; /* Max. investment per share */
    private static final double MINVAL = 0.1; /* Min. investment per share */

    private static double[] RET; /* Estimated return in investment */
    private static int[] RISK; /* High-risk values among shares */
    private static boolean[][] LOC; /* Geogr. region of shares */
    private static boolean[][] SEC; /* Industry sector of shares */

    private static String[] SHARES;
    private static String[] REGIONS;
    private static String[] TYPES;

    /* Fraction of capital used per share */
    private static Variable[] frac;
    /* 1 if asset is in portfolio, 0 otherwise */
    private static Variable[] buy;

    private static void printProblemStatus(XpressProblem prob) {
        System.out.println(String.format("Problem status:%n\tSolve status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getSolStatus()));
    }

    private static void printProblemSolution(XpressProblem prob, boolean isCallback) {
        double[] sol = isCallback ? prob.getCallbackSolution() : prob.getSolution();
        System.out.println("Total return: " + (isCallback ? prob.attributes().getLPObjVal() : prob.attributes().getObjVal()));
        range(0, SHARES.length).forEach(i -> {
            if (buy[i].getValue(sol) > 0.5)
                System.out.println(String.format("%d: %.2f%s (%.1f)", i, 100.0 * frac[i].getValue(sol), "%",
                        buy[i].getValue(sol)));
        });
    }

    public static void main(String[] args) throws IOException {
        readData();
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/
            frac = prob.addVariables(SHARES.length)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("frac_%d", i))
                    /* Upper bounds on the investment per share */
                    .withUB(MAXVAL).toArray();

            buy = prob.addVariables(SHARES.length).withName(i -> String.format("buy_%d", i)).withType(ColumnType.Binary)
                    .toArray();

            /**** CONSTRAINTS ****/
            /* Limit the percentage of high-risk values */
            prob.addConstraint(sum(RISK.length, i -> frac[RISK[i]]).leq(MAXRISK).setName("Risk"));

            /* Limits on geographical distribution */
            prob.addConstraints(REGIONS.length,
                    r -> sum(range(0, SHARES.length).filter(s -> LOC[r][s]).mapToObj(s -> frac[s])).in(MINREG, MAXREG));

            /* Diversification across industry sectors */
            prob.addConstraints(TYPES.length,
                    t -> sum(range(0, SHARES.length).filter(s -> SEC[t][s]).mapToObj(s -> frac[s])).leq(MAXSEC));

            /* Spend all the capital */
            prob.addConstraint(sum(frac).eq(1.0).setName("Cap"));

            /* Limit the total number of assets */
            prob.addConstraint(sum(buy).leq(MAXNUM).setName("MaxAssets"));

            /* Linking the variables */
            prob.addConstraints(SHARES.length,
                    i -> frac[i].geq(buy[i].mul(MINVAL)).setName(String.format("link_lb_%d", i)));
            prob.addConstraints(SHARES.length,
                    i -> frac[i].leq(buy[i].mul(MAXVAL)).setName(String.format("link_ub_%d", i)));

            /* Objective: maximize total return */
            prob.setObjective(scalarProduct(frac, RET), XPRSenumerations.ObjSense.MAXIMIZE);

            /* Callback for each new integer solution found */
            prob.callbacks.addIntSolCallback(p -> {
                    printProblemSolution(prob, true);
            });

            /* Solve */
            prob.optimize();

            /* Solution printing */
            printProblemStatus(prob);
            printProblemSolution(prob, false);
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

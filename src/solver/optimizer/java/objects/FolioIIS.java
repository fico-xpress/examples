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
import com.dashoptimization.XPRSprob.IISStatusInfo;
import com.dashoptimization.objects.IIS;
import com.dashoptimization.objects.IISConstraint;
import com.dashoptimization.objects.IISVariable;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Modeling a MIP problem to perform portfolio optimization.
 * Used infeasible model parameter values and illustrates retrieving IIS.
 */
public class FolioIIS {
    /* Path to Data file */
    private static final String DATAFILE = System.getenv().getOrDefault("EXAMPLE_DATA_DIR", "../../data")
            + "/folio10.cdat";
    private static final int MAXNUM = 5; /* Max. number of different assets */
    private static final double MAXRISK = 1.0 / 3; /* Max. investment into high-risk values */
    private static final double MINREG = 0.1; /* Min. investment per geogr. region */
    private static final double MAXREG = 0.2; /* Max. investment per geogr. region */
    private static final double MAXSEC = 0.1; /* Max. investment per ind. sector */
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
        System.out.println(String.format("Problem status:%n\tSolve status: %s%n\tLP status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getLPStatus(), prob.attributes().getSolStatus()));
    }

    public static void main(String[] args) throws IOException {
        readData();
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/
            frac = prob.addVariables(SHARES.length)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("frac_%s", SHARES[i]))
                    /* Upper bounds on the investment per share */
                    .withUB(MAXVAL).toArray();

            buy = prob.addVariables(SHARES.length).withName(i -> String.format("buy_%s", SHARES[i]))
                    .withType(ColumnType.Binary).toArray();

            /**** CONSTRAINTS ****/
            /* Limit the percentage of high-risk values */
            prob.addConstraint(sum(RISK.length, i -> frac[RISK[i]]).leq(MAXRISK).setName("Risk"));

            /* Limits on geographical distribution */
            range(0, REGIONS.length).forEach(r -> {
                LinExpression MinReg = LinExpression.create();
                LinExpression MaxReg = LinExpression.create();
                range(0, SHARES.length).filter(s -> LOC[r][s]).forEach(s -> MinReg.addTerm(frac[s]));
                range(0, SHARES.length).filter(s -> LOC[r][s]).forEach(s -> MaxReg.addTerm(frac[s]));
                prob.addConstraint(MinReg.geq(MINREG).setName("MinReg(" + REGIONS[r] + ")"));
                prob.addConstraint(MaxReg.leq(MAXREG).setName("MaxReg(" + REGIONS[r] + ")"));
            });

            /* Diversification across industry sectors */
            range(0, TYPES.length).forEach(t -> {
                LinExpression LimSec = LinExpression.create();
                range(0, SHARES.length).filter(s -> SEC[t][s]).forEach(s -> LimSec.addTerm(frac[s]));
                prob.addConstraint(LimSec.leq(MAXSEC).setName("LimSec(" + TYPES[t] + ")"));
            });

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

            /* Solve */
            prob.optimize();

            /* Solution printing */
            printProblemStatus(prob);
            if (prob.attributes().getSolStatus() == XPRSenumerations.SolStatus.INFEASIBLE) {
                System.out.println("LP infeasible. Retrieving IIS.");
                // Check there is at least one IIS
                int status = prob.firstIIS(1);
                if (status != 0)
                    throw new RuntimeException("firstIIS() failed with status " + status);
                int iisIndex = 1; // First IIS has index 1
                do {
                    IISStatusInfo info = prob.IISStatus();
                    System.out.println(String.format(
                            "IIS has %d constraints and %d columns, %d infeasibilities with an infeasibility of %f%n",
                            info.rowsizes[iisIndex], info.colsizes[iisIndex], info.numinfeas[iisIndex],
                            info.suminfeas[iisIndex]));
                    IIS data = prob.getIIS(iisIndex);
                    System.out.println("Variables in IIS:");
                    for (IISVariable v : data.getVariables()) {
                        // Note that the IISVariable class has more fields than
                        // we print here. See the reference documentation for
                        // details.
                        System.out.printf("\t%s %s%n", v.getVariable().getName(), v.getDomain());
                    }
                    System.out.println("Constraints in IIS:");
                    for (IISConstraint c : data.getConstraints()) {
                        // Note that the IISVariable class has more fields than
                        // we print here. See the reference documentation for
                        // details.
                        System.out.printf("\t%s%n", c.getConstraint().getName());
                    }
                    ++iisIndex; // Prepare for next IIS (if any)
                } while (prob.nextIIS() == 0);
            }
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

// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import java.io.FileReader;
import java.io.IOException;
import java.io.StreamTokenizer;

import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.QuadExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Modeling a small QCQP problem to perform portfolio optimization. -- Maximize
 * return with limit on variance ---
 */
public class FolioQC {
    /* Path to Data file */
    private static final String DATAFILE = System.getenv().getOrDefault("EXAMPLE_DATA_DIR", "../../data")
            + "/foliocppqp.dat";
    /* Max. allowed variance */
    private static final double MAXVAR = 0.55;
    /* Number of shares */
    private static final int NSHARES = 10;
    /* Number of North-American shares */
    private static final int NNA = 4;
    /* Estimated return in investment */
    private static final double[] RET = new double[] { 5, 17, 26, 12, 8, 9, 7, 6, 31, 21 };
    /* Shares issued in N.-America */
    private static final int[] NA = new int[] { 0, 1, 2, 3 };
    /* Variance/covariance matrix of estimated returns */
    private static double[][] VAR;

    private static void readData() throws IOException {
        int s, t;
        FileReader datafile = null;
        StreamTokenizer st = null;

        VAR = new double[NSHARES][NSHARES];

        /* Read `VAR' data from file */
        datafile = new FileReader(DATAFILE); /* Open the data file */
        st = new StreamTokenizer(datafile); /* Initialize the stream tokenizer */
        st.commentChar('!'); /* Use the character '!' for comments */
        st.eolIsSignificant(true); /* Return end-of-line character */
        st.parseNumbers(); /* Read numbers as numbers (not strings) */

        for (s = 0; s < NSHARES; s++) {
            do {
                st.nextToken();
            } while (st.ttype == StreamTokenizer.TT_EOL); /* Skip empty lines and comment lines */
            for (t = 0; t < NSHARES; t++) {
                if (st.ttype != StreamTokenizer.TT_NUMBER)
                    break;
                VAR[s][t] = st.nval;
                st.nextToken();
            }
        }
        datafile.close();
    }

    private static void printProblemStatus(XpressProblem prob) {
        System.out.println(String.format(
                "Problem status:%n\tSolve status: %s%n\tLP status: %s%n\tMIP status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getLPStatus(), prob.attributes().getMIPStatus(),
                prob.attributes().getSolStatus()));
    }

    public static void main(String[] args) throws IOException {
        readData();
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/
            Variable[] frac = prob.addVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("frac_%d", i))
                    /* Upper bounds on the investment per share */
                    .withUB(0.3).toArray();

            /**** CONSTRAINTS ****/
            /* Limit variance */
            QuadExpression variance = QuadExpression.create();
            range(0, NSHARES).forEach(s -> range(0, NSHARES).forEach(
                    /* v * fs * ft */
                    t -> variance.addTerm(frac[s], frac[t], VAR[s][t])));
            prob.addConstraint(variance.leq(MAXVAR).setName("Variance"));

            /* Minimum amount of North-American values */
            prob.addConstraint(sum(NNA, i -> frac[NA[i]]).geq(0.5).setName("NA"));

            /* Spend all the capital */
            prob.addConstraint(sum(frac).eq(1.0).setName("Cap"));

            /* Objective: maximize total return */
            prob.setObjective(scalarProduct(frac, RET), XPRSenumerations.ObjSense.MAXIMIZE);

            /* Solve */
            prob.optimize();

            /* Solution printing */
            printProblemStatus(prob);
            System.out
                    .println("With of max variance of " + MAXVAR + " total return is " + prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            range(0, NSHARES).forEach(i -> System.out
                    .println(String.format("%s : %.2f%s", frac[i].getName(), 100.0 * frac[i].getValue(sol), "%")));
        }
    }
}

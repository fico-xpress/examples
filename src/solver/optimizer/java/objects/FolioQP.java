// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.scalarProduct;
import static com.dashoptimization.objects.Utils.sum;
import static java.util.stream.IntStream.range;

import java.io.FileReader;
import java.io.IOException;
import java.io.StreamTokenizer;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.QuadExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Modeling a small QP problem to perform portfolio optimization. -- 1. QP:
 * minimize variance 2. MIQP: limited number of assets ---
 */
public class FolioQP {
    /* Path to Data file */
    private static final String DATAFILE = System.getenv().getOrDefault("EXAMPLE_DATA_DIR", "../../data")
            + "/foliocppqp.dat";
    /* Target yield */
    private static final int TARGET = 9;
    /* Max. number of different assets */
    private static final int MAXNUM = 4;
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
        System.out.println(String.format("Problem status:%n\tSolve status: %s%n\tSol status: %s",
                prob.attributes().getSolveStatus(), prob.attributes().getSolStatus()));
    }

    public static void main(String[] args) throws IOException {
        readData();
        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /***** First problem: unlimited number of assets *****/

            /**** VARIABLES ****/
            Variable[] frac = prob.addVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("frac_%d", i))
                    /* Upper bounds on the investment per share */
                    .withUB(0.3).toArray();

            /**** CONSTRAINTS ****/
            /* Minimum amount of North-American values */
            prob.addConstraint(sum(NNA, i -> frac[NA[i]]).geq(0.5).setName("NA"));

            /* Spend all the capital */
            prob.addConstraint(sum(frac).eq(1.0).setName("Cap"));

            /* Target yield */
            prob.addConstraint(scalarProduct(frac, RET).geq(TARGET).setName("TargetYield"));

            /* Objective: minimize mean variance */
            QuadExpression variance = QuadExpression.create();
            range(0, NSHARES).forEach(s -> range(0, NSHARES).forEach(
                    /* v * fs * ft */
                    t -> variance.addTerm(frac[s], frac[t], VAR[s][t])));
            prob.setObjective(variance, XPRSenumerations.ObjSense.MINIMIZE);

            /* Solve */
            prob.optimize();

            /* Solution printing */
            printProblemStatus(prob);
            System.out.println("With a target of " + TARGET + " minimum variance is " + prob.attributes().getObjVal());
            double[] sollp = prob.getSolution();
            range(0, NSHARES).forEach(i -> System.out
                    .println(String.format("%s : %.2f%s", frac[i].getName(), 100.0 * frac[i].getValue(sollp), "%")));

            /***** Second problem: limit total number of assets *****/
            Variable[] buy = prob.addVariables(NSHARES)
                    /* Fraction of capital used per share */
                    .withName(i -> String.format("buy_%d", i)).withType(ColumnType.Binary).toArray();

            /* Limit the total number of assets */
            prob.addConstraint(sum(buy).leq(MAXNUM).setName("MaxAssets"));

            /* Linking the variables */
            /* frac .<= buy */
            prob.addConstraints(NSHARES, i -> frac[i].leq(buy[i]).setName(String.format("link_%d", i)));

            /* Solve */
            prob.optimize();

            /* Solution printing */
            printProblemStatus(prob);
            System.out.println("With a target of " + TARGET + " and at most " + MAXNUM + " assets, minimum variance is "
                    + prob.attributes().getObjVal());
            double[] solmip = prob.getSolution();
            range(0, NSHARES).forEach(i -> System.out.println(String.format("%s : %.2f%s (%.1f)", frac[i].getName(),
                    100.0 * frac[i].getValue(solmip), "%", buy[i].getValue(solmip))));
        }
    }
}

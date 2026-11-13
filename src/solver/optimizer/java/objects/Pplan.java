// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

//These imports are only for the parser.
import java.io.IOException;
import java.util.stream.IntStream;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.XPRSenumerations.ObjSense;
import com.dashoptimization.objects.LinExpression;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Project planning: MIP example. A company has several projects that it must
 * undertake in the next few months. Each project lasts for a given time (its
 * duration) and uses up one resource as soon as it starts. The resource profile
 * is the amount of the resource that is used in the months following the start
 * of the project. For instance, project 1 uses up 3 units of resource in the
 * month it starts, 4 units in its second month, and 2 units in its last month.
 * The problem is to decide when to start each project, subject to not using
 * more of any resource in a given month than is available. The benefit from the
 * project only starts to accrue when the project has been completed, and then
 * it accrues at BEN[p] per month for project p, up to the end of the time
 * horizon.
 */
public class Pplan {

    static final String[] PROJ = { "A", "B", "C" }; /* Set of projects */
    static final int NM = 6; /* Time horizon (months) */
    static final int[] MONTHS = IntStream.range(1, NM + 1).toArray(); /* Set of time periods (months) to plan for */
    static final int[] DUR = { 3, 3, 4 }; /* Duration of project p */
    static final int[] RESMAX = { 5, 6, 7, 7, 6, 6 }; /* Resource available in month m */
    static final double[] BEN = { 10.2, 12.3, 11.2 }; /* Benefit per month once project finished */
    static final int[][] RESUSE = /* Res. usage of proj. p in its t'th month */
            { { 3, 4, 2, 0, 0, 0 }, { 4, 1, 5, 0, 0, 0 }, { 3, 2, 1, 2, 0, 0 } };

    public static void main(String[] args) throws IOException {

        try (XpressProblem prob = new XpressProblem()) {

            // Create the decision variables
            // 1 if proj p starts in month m, else 0
            Variable[][] start = prob.addVariables(PROJ.length, NM).withType(ColumnType.Binary)
                    .withName((p, m) -> String.format("start(%s,%d)", PROJ[p], MONTHS[m])).toArray();

            // Each project starts once and only once
            prob.addConstraints(PROJ.length, p -> sum(start[p]).eq(1));

            // Resource availability. A project starting in month m is in its k-m month in
            // month k:
            // sum(p in PROJ, m in 0..k) RESUSE[p,k-m]*start[p,m] <= RESMAX[k]
            prob.addConstraints(NM,
                                k -> sum(PROJ.length, k + 1,
                                         (p, m) -> start[p][m].mul(RESUSE[p][k - m]))
                                     .leq(RESMAX[k]));

            /*
             * Objective: Maximize Benefit If project p starts in month m, it finishes in
             * month m+DUR[p]-1 and contributes a benefit of BEN[p] for the remaining
             * NM-[m+DUR[p]-1] months:
             */
            LinExpression obj = LinExpression.create();
            for (int p = 0; p < PROJ.length; p++) {
                for (int m = 0; m < NM - DUR[p]; m++) {
                    obj.addTerm(start[p][m], BEN[p] * (NM - m - DUR[p]));
                }
            }
            prob.setObjective(obj, ObjSense.MAXIMIZE);

            // Solve the problem
            prob.optimize("");

            System.out.println("Problem status: " + prob.attributes().getMIPStatus());
            if (prob.attributes().getMIPStatus() != XPRSenumerations.MIPStatus.SOLUTION
                    && prob.attributes().getMIPStatus() != XPRSenumerations.MIPStatus.OPTIMAL)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getMIPStatus());

            // Solution printing
            System.out.println("Solution value is: " + prob.attributes().getObjVal());
            double[] sol = prob.getSolution();
            for (int p = 0; p < PROJ.length; p++)
                for (int m = 0; m < NM; m++) {
                    if (start[p][m].getValue(sol) > 0.5)
                        System.out.println("Project " + PROJ[p] + " starts in month " + MONTHS[m]);
                }
        }
    }
}
